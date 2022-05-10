/* base */
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
/* netinet*/
#include <netinet/in.h>
#include <netinet/ip_icmp.h>	
#include <netinet/udp.h>	
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netdb.h>
/* linux*/
#include <linux/if_packet.h>
#include <linux/if_tun.h>
#include <linux/if_ether.h>
#include <linux/if.h>

/*sys*/
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/eventfd.h>

/*thread*/
#include <pthread.h>

#define READ_EVENT     (EPOLLIN|EPOLLET)
#define WRITE_EVENT    EPOLLOUT
#define CLOSE_EVENT    1
#define ECHO_OK        0
#define ECHO_ERROR     -1
#define ECHO_AGAIN      -2

typedef struct cycle{
    pthread_t pid;
    int ep;
    int efd;
    struct epoll_event *event_list;
    int nevent;
}cycle_t;

cycle_t cycle;



static int 
echo_epoll_init(cycle_t *cycle, unsigned timer) {

    if (cycle->ep == -1) {
        cycle->ep = epoll_create(1024);
    }
    
    if (cycle->event_list) {
        free(cycle->event_list);
    }
    cycle->nevent = cycle->nevent ? cycle->nevent:1024;
    
    cycle->event_list = malloc(sizeof(struct epoll_event) * cycle->nevent);
    if (cycle->event_list == NULL) {
        return -1;
    }
    return 0;
}
int epoll_process_event(cycle_t *cycle, unsigned timer) {
    int nevent;
    //timer == -1 epoll_wait block
    //timer == 0, epoll_wait return immediately
    nevent = epoll_wait(cycle->ep, cycle->event_list, cycle->nevent, timer);
    struct epoll_event *e;

    if (nevent == 0) {
        if (timer == -1) {
            return 0;
        }
        perror("epoll wait\n");
        return -1;
    }
    for (int i=0; i < nevent; i++) {
        e = &cycle->event_list[i];
        printf("the fd is %d\n", e->data.fd);
        eventfd_t value;
        eventfd_read(e->data.fd, &value);
        printf("the data is %d\n", (int)value);
    }
    return 0;
}
static int 
echo_epoll_done(cycle_t *cycle) {
    if(cycle->event_list){
        free(cycle->event_list);
        cycle->nevent = 0;
    }
    if (cycle->ep != -1) {
        close(cycle->ep);
        cycle->ep = -1;
    }
    return 0;
}

void *received_routine(void *data) {
    (void)data;
    int *start_flag = data;

    int rc;
    struct epoll_event ee;
    cycle.ep = -1;
    rc = echo_epoll_init(&cycle, -1);
    if (rc < 0) {
        perror("init ");
        exit(EXIT_FAILURE);
    }

    cycle.efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    ee.data.fd = cycle.efd;
    ee.events = READ_EVENT;

    int n = epoll_ctl(cycle.ep, EPOLL_CTL_ADD, cycle.efd, &ee);
    if (n < 0) {
        perror("epool ctl");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        *start_flag = 1;
        rc = epoll_process_event(&cycle, -1);
        if (rc != 0) {
            break;
        }
    }
    echo_epoll_done(&cycle);
    return NULL;
}

int	main(int argc, char **argv) {

    int start_flag = 0;
    pthread_create(&cycle.pid, NULL, received_routine, &start_flag);

    for(;;) {
        if (start_flag) {
            uint64_t count = 1;
            int n = write(cycle.efd, &count, sizeof(uint64_t));
            if (n < 0) {
                perror("write\n");
                
            }
            sleep(2);
            // n = eventfd_write(cycle.efd, 2);
            
        }
    }

    return 0;
}
