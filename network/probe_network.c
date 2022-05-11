#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

struct server_epoll_data {
    int  fd;
    char ip[sizeof("255.255.255.255:65535")];
};

typedef  struct server_in_cidr {
    in_addr_t                 addr;
    in_addr_t                 mask;
}server_in_cidr_t;

typedef struct server_cidr {
    server_in_cidr_t in;
}server_cidr_t;

/**
 * @brief 
 * 
 * @param addr 
 * @param addr2 如果addr 为NULL 使用这个地址 主机序 地址
 * @param port 
 * @return int  -1 erro 0 correct
 */

int connect2server(char *addr, uint32_t addr2, int port){
    int reuseport = 1 , reuseaddr = 1, rc;
    if(port <= 0){
        return -1;
    }
    int flags ;
    struct sockaddr_in client;
    int client_fd;
    client.sin_family = AF_INET;

    if (addr != NULL) {
        client.sin_addr.s_addr = inet_addr(addr);
    } else {
        if (addr2 == -1) {
            return -1;
        }
        client.sin_addr.s_addr = htonl(addr2);
    }
    
    client.sin_port = htons(port);
    client_fd = socket(AF_INET,SOCK_STREAM,0);

    flags = fcntl(client_fd, F_GETFL, 0);

    fcntl(client_fd, F_SETFL, O_NONBLOCK | flags);

    rc = setsockopt(client_fd, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(reuseport));
    if (rc == -1) 
        return -1;
    rc = setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
    if (rc == -1) 
        return -1;

    if(connect(client_fd,(struct sockaddr *)&client,sizeof(client)) == -1){
        if (errno == EINPROGRESS) {
            return client_fd;
        }
        return -1;
    }
    return client_fd;
}

int server_epoll_create() {

    return epoll_create(1);
}

int server_epoll_add_fd(int ep, int fd, struct server_epoll_data *data) {
    
    struct epoll_event event;

    // event->events = EPOLLIN  | EPOLLPRI ;

    event.events = EPOLLIN  | EPOLLPRI | EPOLLOUT | EPOLLET;
    if (data == NULL) {
        event.data.fd = fd;
    }else {
        data->fd = fd;
        event.data.ptr = data;
    }

    return epoll_ctl(ep, EPOLL_CTL_ADD , fd, &event);
}

int server_epoll_free_fd(int ep, int fd) {
    

}

int ptocidr(const char *text, server_cidr_t *cidr) {
    char *addr, *mask;
    int addr_len, shift;

    
    mask = strchr(text, '/');

    if (mask == NULL) 
        return -1;

    addr_len = mask - text;

    addr = malloc(addr_len +1);
    if (!addr) 
        return -1;
    
    (void)memcpy(addr, text, addr_len);
    addr[addr_len] = '\0';
    cidr->in.addr =  inet_addr(addr);
    free(addr);

    mask++;
    shift = atoi(mask);
    if(shift == -1) 
        return -1;


    if (shift > 32) 
        return -1;
    if (shift) {
        cidr->in.mask =  htonl((uint32_t) (0xffffffffu << (32 - shift)));
    }else {
        //shift is zero 
        cidr->in.mask = 0;
    }

    if (cidr->in.addr == (cidr->in.addr & cidr->in.mask)) 
        return 0;
    
    cidr->in.addr &= cidr->in.mask;
    
    
    return 1;
}
int get_port_range(const char *text, int *port1, int *port2) {
    const char *_port1, *_port2;
    _port1 = text;
    _port2 = strchr(text, ',');
    
    if (_port2 == NULL) {
        *port1 = atoi(_port1);
        *port2 = *port1;
        if(port1 <= 0) {
            return -1;
        }
    }else {
        _port2++;
        *port1 = atoi(_port1);
        *port2 = atoi(_port2);
        if (*port1<=0 || *port2 <=0 || *port2 < *port1) {
            return -1;
        }
    }
    return 0;
}
void hexToString(unsigned char *hex, char *str, int len, int output) {
  if (str == NULL) {
    str = malloc(2 * len) ;
  }
  for (size_t i = 0; i < len; i++) {
    sprintf(str + i * 2, "%02x", hex[i]);
  }
  if (output) {
    printf("%s\n", str);
  }
  free(str);
  return;
}

int main(int argc, char const *argv[]){
    char c ;
    int ep, epoll_num, tris = 7, port_min, port_max;
    struct epoll_event events[10000];
    struct server_epoll_data epoll_datas[10000];
    struct server_epoll_data *epoll_data;
    server_cidr_t cidr;
    uint32_t host_ip, host_mask, host_port, subnet_range;
    int fd[10000] = {0}, fd_index = 0 , i;

    //ip/mask port
    if (argc <= 2) {
        printf("usage ./probe_network [ip/mask] [port1 [,port2]]\n");
        exit(-1);
    }

    int rc = ptocidr(argv[1], &cidr);

    if (rc == -1) {
        return -1;
    }

    host_ip = ntohl(cidr.in.addr);
    host_mask = ntohl(cidr.in.mask);


    // port = atoi(argv[2]);
    rc = get_port_range(argv[2], &port_min, &port_max);
    if (rc == -1) {
        printf("port set error\n");
        return -1;
    }


    ep = server_epoll_create();
    if (ep == -1) {
        perror("epoll create");
        return -1;
    }
    
    
    subnet_range = (host_mask ^ 0xffffffffu) -1;
    // hexToString((unsigned char *)&host_ip, NULL, 4, 1);
    // hexToString((unsigned char *)&host_mask, NULL, 4, 1);

    // // hexToString((unsigned char *)&subnet_range, NULL, 4, 1);

    // printf("subnet_range :%d\n",subnet_range);
    
    if (subnet_range <= 0) {
        printf("ip or mask set error\n");
        return -1;
    }
        
    
    for (i = 1 ; i <= subnet_range; i++) {
        
        for (host_port = port_min; host_port <= port_max ; host_port++) {
            fd_index ++;
            fd[fd_index] = connect2server(NULL, host_ip + i, host_port);
            if (fd[fd_index] == -1) {
                perror("");
                goto error_close_fd;
            }

            struct in_addr in_addr;
            in_addr.s_addr = (in_addr_t)htonl(host_ip + i);

            strcpy(epoll_datas[i].ip, inet_ntoa(in_addr));

            sprintf(epoll_datas[i].ip + strlen(inet_ntoa(in_addr)) , ":%d", host_port);
            

            // printf("try %s\n",epoll_datas[i].ip);

            if (server_epoll_add_fd(ep, fd[fd_index], &epoll_datas[i]) == -1)
                return -1;
            
        }

    }

    for (; tris; tris--) {
        epoll_num =  epoll_wait(ep, events, sizeof(events)/sizeof(events[0]), 5000); //延时5000毫秒
        // printf("epoll num :%d\n",epoll_num);
        if (epoll_num == 0) {
            break;
        }
        for (int i=0 ; i <= epoll_num-1 ;i++) {
            epoll_data = events[i].data.ptr;
            int data;
            if (read(epoll_data->fd, &data,sizeof(data)) <= 0) {
                //reset or close
                continue;
            }
            printf("%s\n", epoll_data->ip);
        }

    }

    // printf("close %d\n",fd_index);
    for(i = 1; i <= fd_index; i++) {
        close(fd[i]);
    }
    return 0;
error_close_fd:
    // printf("close %d\n",fd_index-1);
    for(i = 1; i <= fd_index -1; i++) {
        close(fd[i]);
    }
    return -1;
}
