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
    char ip[sizeof("255.255.255.255")];
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
    int ep, epoll_num, tris = 7, port;
    uint32_t subnet_range;
    struct epoll_event events[255];
    struct server_epoll_data epoll_datas[255];
    struct server_epoll_data *epoll_data;
    server_cidr_t cidr;
    uint32_t host_ip;
    uint32_t host_mask;

    //ip/mask port
    if (argc <= 2) {
        printf("usage ./probe_network [ip/mask] [port]\n");
        exit(-1);
    }

    int rc = ptocidr(argv[1], &cidr);

    if (rc == -1) {
        return -1;
    }

    host_ip = ntohl(cidr.in.addr);
    host_mask = ntohl(cidr.in.mask);


    port = atoi(argv[2]);


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
        
    
    for (int i =1 ; i <= subnet_range; i++) {
        int fd = -1;
       
        fd = connect2server(NULL, host_ip + i, port);
        if (fd == -1) {
            perror("");
            return -1;
        }
        struct in_addr in_addr;
        in_addr.s_addr = (in_addr_t)htonl(host_ip + i);

        strcpy(epoll_datas[i].ip, inet_ntoa(in_addr));
        
        // printf("try %s\n",epoll_datas[i].ip);

        if (server_epoll_add_fd(ep, fd, &epoll_datas[i]) == -1) {
            return -1;
        }

    }

     for (; tris; tris--) {
        epoll_num =  epoll_wait(ep, events, sizeof(events)/sizeof(events[0]), 5000); //延时5000毫秒
        // printf("epoll num :%d\n",epoll_num);
        if (epoll_num == 0) {
            return 0;
        }
        for (int i=0 ; i <= epoll_num-1 ;i++) {
            epoll_data = events[i].data.ptr;
            printf("%s\n", epoll_data->ip);
        } 

    }

    return 0;
}
