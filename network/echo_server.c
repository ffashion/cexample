#include <asm-generic/errno-base.h>
#include <errno.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define READ_EVENT     (EPOLLIN|EPOLLRDHUP)
#define WRITE_EVENT    EPOLLOUT
#define ECHO_CLOSE_EVENT    1
#define ECHO_OK        0
#define ECHO_ERROR     -1
#define ECHO_AGAIN      -2


typedef struct echo_event_s echo_event_t;
typedef struct echo_listening_s echo_listening_t;
typedef struct echo_connection_s echo_connection_t;
typedef struct echo_cycle_s echo_cycle_t;
typedef void (*echo_event_handler_pt)(echo_event_t *ev);
typedef void (*echo_connection_handler_pt)(echo_connection_t *c);

typedef ssize_t (*echo_recv_pt)(echo_connection_t *c, char *buf, size_t size);
typedef ssize_t (*echo_send_pt)(echo_connection_t *c, char *buf, size_t size);
typedef void (*echo_connection_handler_pt)(echo_connection_t *c);

typedef unsigned long               echo_atomic_uint_t;


struct echo_event_s{
    void             *data;
    echo_event_handler_pt handler;
    int                 available;
    unsigned         write:1;
    unsigned         instance:1;
    unsigned         active:1; //如果这个事件处于epoll中 那么active为1 用于判断在epoll_ctl的时候 使用MOD还是ADD
    unsigned         ready:1; //表示是否有数据读，如果读取到EAGAIN 那么此位会被置0
    unsigned         closed;
    unsigned         accept:1;
    unsigned         eof:1;
    unsigned         error:1;
};

struct echo_connection_s{
    void *data; //在cycle上用于表示next connection, 在请求的时候又可以用于hc和r
    echo_event_t *read;
    echo_event_t *write;
    int fd;
    int type;
    int sent;
    echo_listening_t *listening;
    struct sockaddr *sockaddr; //客户端地址
    socklen_t        socklen; //客户端地址长度
    echo_recv_pt recv;
    echo_send_pt send;
    struct sockaddr    *local_sockaddr; //只在accepte返回的那个fd使用，listeing的服务器地址
    socklen_t           local_socklen;
    echo_atomic_uint_t number; 
    char                *buffer;
    int                 buffer_size;
};

struct echo_listening_s{  
    int fd;
    int ignore;
    int type;
    int backlog;
    struct sockaddr     *sockaddr;
    socklen_t           socklen;
    echo_connection_t   *connection;
    echo_listening_t    *previous;
    echo_connection_handler_pt handler;

    unsigned reuseport:1;
    unsigned add_reuseport:1;
    unsigned listen:1;
};

struct echo_cycle_s {
    int connections_n;
    echo_connection_t *connections;
    echo_event_t      *read_events;
    echo_event_t      *write_events;
    int free_connections_n;
    echo_connection_t *free_connections;
    
    echo_listening_t  *listening;
    echo_listening_t  *listening_next;
    int listening_n;
    int listening_used;

    int nevents;
};
static int ep = -1;
static struct epoll_event *event_list;
static int nevent;
static unsigned echo_timer_resolution;
static echo_cycle_t *echo_cycle;

int echo_nonblocking(int s);
int echo_open_listening_sockets(echo_cycle_t *cycle);
echo_connection_t* echo_get_connection(int fd);
static void echo_event_accept(echo_event_t *ev);
static void echo_event_recvmsg(echo_event_t *ev);
int echo_event_process_init(echo_cycle_t *cycle);
ssize_t ngx_unix_recv(echo_connection_t *c, char *buf, size_t size);
ssize_t ngx_unix_sned(echo_connection_t *c, char *buf, size_t size);
void echo_http_init_connection(echo_connection_t *c);
void echo_http_empty_handler(echo_event_t *wev);
static void echo_http_wait_request_handler(echo_event_t *rev);
void echo_close_connection(echo_connection_t *c);
int echo_handle_read_event(echo_event_t *rev, unsigned flags);
//base function
void *echo_palloc(size_t size) {
    void *addr;
    addr = malloc(size);
    if (addr == NULL) {
        return NULL;
    }
    memset(addr, 0, size);
    return addr;
}

int
echo_nonblocking(int s) {
    int  nb;

    nb = 1;

    return ioctl(s, FIONBIO, &nb);
}
ssize_t ngx_unix_recv(echo_connection_t *c, char *buf, size_t size){
    echo_event_t *rev;
    int n, err;
    rev = c->read;

    do {
        n = recv(c->fd, buf, size, 0);
        if (n == 0) {
            rev->ready = 0;
            rev->eof = 1;
            return 0;
        }
        if (n > 0) {
            return n;
        }

        if (err == EAGAIN || err == EINTR) {
            n = ECHO_AGAIN;
        }else {
            n = ECHO_ERROR;
            break;
        }

    }while(err == EINTR);

    rev->ready = 0;
    if (n == ECHO_ERROR) {
        rev->error = 1;
    }
    return n;
}

ssize_t ngx_unix_sned(echo_connection_t *c, char *buf, size_t size) {
    ssize_t       n;
    int err;
    echo_event_t *wev;

    wev = c->write;
    for (;;) {
        n = send(c->fd, buf, size, 0);
        if (n > 0) {
            if (n < size) {
                wev->ready = 0;
            }
            c->sent += n;
            return n;
        }
        err = errno;
        if (n == 0) {
            //log
            wev->ready = 0;
            return 0;
        }
        if (err == EAGAIN || err == EINTR) {
            wev->ready = 0;
            if (err == EAGAIN) {
                return ECHO_AGAIN;
            }

        }else {
            wev->error = 1;
            return ECHO_ERROR;
        }
    }

}

//connection part;
echo_connection_t*
echo_get_connection(int s) {
    echo_connection_t *c;
    echo_event_t *rev, *wev;
    int instance;
    c = echo_cycle->free_connections;
    if (c == NULL) {
        return NULL;
    }

    echo_cycle->free_connections = c->data;
    echo_cycle->free_connections_n--;

    // 保存 读写事件
    rev = c->read;
    wev = c->write;

    memset(c, 0, sizeof(echo_connection_t));
    //恢复读写 事件
    c->read = rev;
    c->write = wev;
    c->fd = s;

    instance = c->read->instance;

    memset(c->read, 0, sizeof(echo_event_t));
    memset(c->write, 0, sizeof(echo_event_t));

    c->read->instance = !instance;
    c->write->instance = !instance;

    c->read->data = c;
    c->write->data = c;
    c->write->write = 1;
    
    return c;
}
void echo_free_connection(echo_connection_t *c) {
    c->data = echo_cycle->free_connections;
    echo_cycle->free_connections = c;
    echo_cycle->free_connections_n++;
}


// listening part
// open all listening 
int 
echo_open_listening_sockets(echo_cycle_t *cycle) {
    echo_listening_t *ls;
    int s, err, faild;
    ls = cycle->listening;
    for(int i=0; i < cycle->listening_used; i++) {
        if (ls[i].ignore) {
            continue;
        }
        if (ls[i].add_reuseport) {
            int reuseport = 1;
            
            if(setsockopt(ls[i].fd, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(int)) == -1) {
                return ECHO_ERROR;
            }
            ls[i].add_reuseport = 0;
        }
        if (ls[i].fd == -1) {
            continue;
        }
        s = socket(ls[i].sockaddr->sa_family, ls[i].type, 0);
        if (s == -1) {
            return ECHO_ERROR;
        }

        if(ls[i].type != SOCK_DGRAM){
            int reuseaddr = 1;
            if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1){
                close(s);
                return ECHO_ERROR;
            }
        }
        if (ls[i].reuseport) {
            int reuseport = 1;
            if(setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(int)) == -1) {
                close(s);
                return ECHO_ERROR;
            }
        }
        //set nobloking
        if (echo_nonblocking(s) == -1) {
            close(s);
            return ECHO_ERROR;
        }
        if (bind(s, ls[i].sockaddr, ls[i].socklen) == -1) {
            err = errno;
            if (err != EADDRINUSE) {
                close(s);
                return ECHO_ERROR;
            }
            faild = 1;
            continue;
        }
        if (ls[i].type != SOCK_STREAM) {
            ls[i].fd = s;
            continue;
        }

        //SOCK_STREAM
        if(listen(s, ls[i].backlog) == -1) {
            err = errno;
            if (err != EADDRINUSE) {
                return ECHO_ERROR;
            }
            faild = 1;
            continue;
        }
        ls[i].listen = 1;
        ls[i].fd = s;
    }
    return ECHO_OK;
}



// event module part

static int 
echo_epoll_add_event(echo_event_t *ev, unsigned event, unsigned flags) {
    echo_connection_t *c;
    echo_event_t       *e;
    struct epoll_event ee;
    unsigned events, prev;
    int op;
    c = ev->data;
    events = event;
    //2个event 关联一个fd 根据另外一个事件, 判断connection关联的fd是否加入了epoll
    if (event == READ_EVENT) {
        e = c->write;
        prev = WRITE_EVENT;
    }else {
        e = c->read;
        prev = READ_EVENT;
    }
    //如果相对应事件是激活的(读对应写 写对应读)
    if (e->active) {
        op = EPOLL_CTL_MOD;
        events |= prev;
    }else {
        op = EPOLL_CTL_ADD;
    }

    ee.events |= events | flags;
    ee.data.ptr = (void *) ((uintptr_t) c | ev->instance);

    if (epoll_ctl(ep, op, c->fd, &ee) == -1) {
        return ECHO_ERROR;
    }
    ev->active = 1;

    return ECHO_OK;
}
static int 
echo_epoll_del_event(echo_event_t *ev, unsigned event, unsigned flags) {
    echo_event_t *e;
    echo_connection_t *c;
    struct epoll_event ee;
    int prev, op;
    if(flags & ECHO_CLOSE_EVENT) {
        ev->active = 0;
        return ECHO_OK;
    }
    c = ev->data;

    if (event == READ_EVENT) {
        e = c->write;
        prev = WRITE_EVENT;
    }else {
        e = c->read;
        prev = READ_EVENT;
    }

    if (e->active) {
        op = EPOLL_CTL_MOD;
        ee.events = prev | flags;
        ee.data.ptr = (void *) ((uintptr_t) c | ev->instance);
    }else {
        op = EPOLL_CTL_DEL;
        ee.events = 0;
        ee.data.ptr = NULL;
    }

    if (epoll_ctl(ep, op, c->fd, &ee) == -1) {
        return ECHO_ERROR;
    }
    ev->active = 0;
    return ECHO_OK;
}


static int 
echo_epoll_init(echo_cycle_t *cycle, unsigned timer) {
    if(ep == -1) {
        ep = epoll_create(cycle->connections_n / 2);
        if (ep == -1) 
            return ECHO_ERROR;
    }
    if (event_list) 
        free(event_list);
    
    nevent = cycle->nevents;
    if (nevent == 0)
        return ECHO_ERROR;
    
    event_list = echo_palloc(sizeof(struct epoll_event) * nevent);
    if (event_list == NULL) 
        return ECHO_ERROR;
    
    return ECHO_OK;
}

static int 
echo_epoll_done(echo_cycle_t *cycle) {
    if (event_list) {
        free(event_list);
        nevent = 0;
    }
    if (ep != -1) {
        close(ep);
        ep = -1;
    }
    return 0;
}

int echo_event_process_init(echo_cycle_t *cycle) {
    echo_event_t *rev, *wev;
    echo_connection_t *c, *old;
    echo_listening_t *ls;
    //for module in events_modules do module.init()
    echo_timer_resolution = 0;
    cycle->nevents = 512;
    echo_epoll_init(cycle, echo_timer_resolution);

    //set timer handler

    //set limit handler 

    // init cycle 
    cycle->connections = echo_palloc(cycle->connections_n * sizeof(echo_connection_t));
    if (cycle->connections == NULL) {
        return ECHO_ERROR;
    }
    c = cycle->connections;

    cycle->read_events = echo_palloc(cycle->connections_n * sizeof(echo_event_t));
    if (cycle->read_events == NULL) {
        return ECHO_ERROR;
    }
    rev = cycle->read_events;
    for (int i = 0; i < cycle->connections_n; i++) {
        rev[i].closed = 1;
        rev[i].instance = 1;
    }

    cycle->write_events = echo_palloc(cycle->connections_n * sizeof(echo_event_t));
    if (cycle->write_events == NULL) {
        return ECHO_ERROR;
    }

    wev = cycle->write_events;
    for (int i=0; i<cycle->connections_n; i++) {
        wev[i].closed = 1;
    }

    for (int i=0; i<cycle->connections_n; i++) {
        c[i].read  = &rev[i];
        c[i].write = &wev[i];
        if (i != cycle->connections_n -1) {
            c[i].data = &c[i + 1];
        }else {
            c[i].data = NULL;
        }
        c[i].fd = -1;
    }
    cycle->free_connections = cycle->connections;
    cycle->free_connections_n = cycle->connections_n;

    //process listening socket , 将所有的listeing都关联了一个connection
    ls = cycle->listening;
    for (int i=0; i<cycle->listening_used; i++) {
        c = echo_get_connection(ls[i].fd);
        if (c == NULL) {
            return ECHO_ERROR;
        }
        c->type = ls[i].type;
        c->listening = &ls[i];
        
        ls[i].connection = c;

        c->read->accept = 1;

        if (ls[i].previous) {
            old = ls[i].previous->connection;
            if (echo_epoll_del_event(old->read, READ_EVENT, ECHO_CLOSE_EVENT) == -1) {
                return ECHO_ERROR;
            }
            old->fd = -1;
        }
        c->read->handler = (c->type == SOCK_STREAM) ?
                    echo_event_accept: echo_event_recvmsg;

        if (echo_epoll_add_event(c->read, READ_EVENT, EPOLLET) == -1) {
            return ECHO_ERROR;
        }
    }
    return ECHO_OK;
}

static int 
echo_epoll_process_event(echo_cycle_t *cycle, unsigned timer) {
    echo_connection_t *c;
    int events, instance, err, revents;
    echo_event_t *rev, *wev;

    events =  epoll_wait(ep, event_list, nevent, timer);
    
    err = (events == -1) ? errno : 0;
    if (err) {
        if (err == EINTR) {
            return ECHO_OK;
        }else {
            return ECHO_ERROR;
        }
    }
    
    if (events == 0) {
        if (timer == -1) {
            return ECHO_OK;
        }

        return ECHO_ERROR;
    }

    for (int i = 0; i < events ; i++) {
        c = event_list[i].data.ptr;
        instance = (uintptr_t)c & 1;
        c = (echo_connection_t *) ((uintptr_t) c & (uintptr_t) ~1);

        revents = event_list[i].events;
        //if read event
        rev = c->read;
        if (c->fd == -1 || rev->instance != instance) {
            //fd 在迭代过程中已经被close
            continue;
        }
        
        if (revents & (EPOLLERR|EPOLLHUP)) {
            revents = EPOLLIN | EPOLLOUT;
        }
        if ((revents & EPOLLIN) && rev->active) {
            rev->ready = 1;
            rev->available = -1;
            rev->handler(rev);
        }
         //if write event
        wev = c->write;
        if (c->fd == -1 || wev->instance != instance) {
            continue;
        }
        if ((revents & EPOLLOUT) && wev->active) {
            wev->ready = 1;
            wev->handler(wev);
        }

       
    }
    
}
int 
echo_handle_read_event(echo_event_t *rev, unsigned flags) {

    if (!rev->active && !rev->ready) {
        if (echo_epoll_add_event(rev, READ_EVENT, EPOLLET)
            == ECHO_ERROR) {
            return ECHO_ERROR;
        }
    }
    return ECHO_OK;
}


static void
echo_close_accepted_connection(echo_connection_t *c) {

}

static void echo_event_recvmsg(echo_event_t *ev) {

}

static void
echo_http_wait_request_handler(echo_event_t *rev) {
    echo_connection_t *c;
    int n, buffer_size;
    c = rev->data;
    
    buffer_size = (c->buffer_size) ? c->buffer_size: 1024;

    c->buffer = echo_palloc(buffer_size);
    n = c->recv(c, c->buffer, buffer_size);
    if (n == EAGAIN) {
        free(c->buffer);
        return;
    }

    if (n == ECHO_ERROR) {
        echo_close_connection(c);
    }
    
    if (n == 0) {
        //peer close tcp
        echo_close_connection(c);
    }

    n = c->send(c, c->buffer, n);
    if (n == EAGAIN) {
        
    }

}

void echo_http_empty_handler(echo_event_t *wev) {
    return;
}
void
echo_close_connection(echo_connection_t *c) {
    if (c->buffer) {
        free(c->buffer);
    }
    if (c->read->active) {
        echo_epoll_del_event(c->read, READ_EVENT, 0);
    }

    if (c->write->active) {
        echo_epoll_del_event(c->write, READ_EVENT, 0);
    }
    echo_free_connection(c);
}

void
echo_http_init_connection(echo_connection_t *c) {

    c->read->handler = echo_http_wait_request_handler;
    c->write->handler = echo_http_empty_handler;
    
    if (echo_handle_read_event(c->read, 0) == ECHO_ERROR) {
        echo_close_connection(c);
        return;
    }
} 
static void 
echo_event_accept(echo_event_t *ev) {
    echo_connection_t *c, *lc;
    echo_listening_t *ls;
    socklen_t socklen;
    struct sockaddr sa;
    int s, err;
    echo_event_t *rev, *wev;

    lc = ev->data;
    ls = lc->listening;

    ev->ready = 0;

    socklen = sizeof(struct sockaddr);
    s = accept(lc->fd, &sa, &socklen);
    if (s == -1) {
        err = errno;
        if (err == EAGAIN) {
            return;
        }
        if (err == EMFILE || err == ENFILE) {
            //delete accept event
        }
        return;
    }
    c = echo_get_connection(s);
    if (c == NULL) {
        //无法创建新连接了 直接关闭
        if (close(s) == -1) {
            //log()
            return;
        }
        return;
    }
    c->type = SOCK_STREAM;
    c->sockaddr = echo_palloc(socklen);
    if (c->sockaddr == NULL) {
        echo_close_accepted_connection(c);
        return;
    }
    c->recv = ngx_unix_recv;
    c->send = ngx_unix_sned;

    c->socklen = socklen;
    //新的connection 继承之前的listening
    c->listening = ls;
    c->local_sockaddr = ls->sockaddr;
    c->local_socklen = ls->socklen;

    c->write->ready = 1;

    ls->handler(c);
    
}



int
echo_init_cycle(echo_cycle_t *cycle) {
    echo_listening_t *ls;
    struct sockaddr_in *sin;
    cycle->connections_n = 100;
    cycle->listening_n = 10;
    

    //read conf file and set cycle
    cycle->listening = echo_palloc(cycle->listening_n *  sizeof(echo_listening_t));
    ls = cycle->listening;

    sin = echo_palloc(sizeof(struct sockaddr_in));
    sin->sin_port = htons(8083);
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = inet_addr("127.0.0.1");
    ls[0].backlog = 10;
    ls[0].ignore = 0;
    ls[0].type = SOCK_STREAM;
    ls[0].sockaddr = (struct sockaddr *)sin;
    ls[0].socklen = sizeof(struct sockaddr_in);
    ls[0].handler = echo_http_init_connection;
    cycle->listening_used = 1;


    if(echo_open_listening_sockets(cycle) == -1) {
        return ECHO_ERROR;
    }

    //
    if (echo_event_process_init(cycle) == -1) {
        return ECHO_ERROR;
    }
    return ECHO_OK;
}



int	main(int argc, const char **argv) {

    echo_cycle = echo_palloc(sizeof(echo_cycle_t));
    
    if (echo_init_cycle(echo_cycle) == ECHO_ERROR) {
        return -1;
    }

    for (;;) {
        echo_epoll_process_event(echo_cycle, -1);
    }  
    return 0;
}
