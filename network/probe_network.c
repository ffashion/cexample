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
    char ip[32];
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
 * @param addr2 如果addr 为NULL 使用这个地址
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

int main(int argc, char const *argv[]){
    char c ;
    int ep, epoll_num, tris = 7, port;
    struct epoll_event events[255];
    struct server_epoll_data epoll_datas[255];
    struct server_epoll_data *epoll_data;
    char ip_mask[sizeof("255.255.255.255/32")];

    //ip/mask port
    if (argc <= 2) {
        printf("usage ./probe_network [ip/mask] [port]\n");
        exit(-1);
    }

    strcpy(ip_mask,argv[1]);


    port = atoi(argv[2]);


    ep = server_epoll_create();
    if (ep == -1) {
        perror("epoll create");
        return -1;
    }
    


    // char addr[] = "nc -z -w 1 10.10.22.000 22";
    char addr[] = "10.10.22.000";

    for (int i =1 ; i <= 254 ; i++) {
        
        
        if (i < 10) {
            addr[9] = i + '0';
            addr[10] = ' ';
            addr[11] = ' ';
        }
 
        if (i >= 10) {
            addr[9] = i % 100 /10 + '0';
            addr[10] = i % 10 + '0';
            addr[11] = ' ';
        }
        if (i >= 100) {
            addr[9] = (i % 1000 /100) + '0';
            addr[10] = (i % 100 /10) + '0';
            addr[11] = (i % 10) + '0';
        }

        // epoll_datas->ip = addr;
        

        int fd = -1;
        fd = connect2server(addr, -1, port);
        

        if (fd == -1) {
            return -1;
        }

        strcpy(epoll_datas[i].ip, addr);
        
        // printf("try %s\n",epoll_datas[i].ip);

        if (server_epoll_add_fd(ep, fd, &epoll_datas[i]) == -1) {
            return -1;
        }
    }

    for (; tris; tris--) {
        epoll_num =  epoll_wait(ep, events, sizeof(events)/sizeof(events[0]), 5000); //延时200毫秒
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
