#include <stdio.h>
#include <time.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
typedef void* (*timer_cb)(void *);

typedef struct list_node_timer { 
    struct list_node_timer *next;
    time_t time;
    timer_cb cb;
}list_node_timer_t;


typedef struct list {
    list_node_timer_t *first;
    pthread_mutex_t mutex;
}list_t;

int random_usleep(int range_ms) {
    int sleep_time;
    srand(time(NULL));
    sleep_time =  rand() % range_ms;
    
    usleep(sleep_time);
    return  0;;
}

int list_timer_init(list_t *list) {
    list->first = NULL;
    pthread_mutex_init(&list->mutex, NULL);
} 
int list_timer_insert(list_t *list, time_t time, timer_cb cb) {
    list_node_timer_t *node;
    list_node_timer_t *new;
    if (list == NULL)
        return -1;
    if(list->first == NULL) {
        list->first = malloc(sizeof(list_node_timer_t));
        node = list->first;
        node->next = NULL;
        node->time = time;
        node->cb = cb;
        return 0;
    }
    node = list->first;
    new =  malloc(sizeof(list_node_timer_t));
    new->next = NULL;
    new->time = time;
    new->cb = cb;
    //比某个节点小 插入到他的前面 相同的时间 会插入到之前的那个后面
    //时间从低到高排列
    for (; node ;) {
        if (new->time < node->time) {
            if (node == list->first) {
                list->first = new;
                new->next = node;
                return 0;
            }else {
                //由于使用单链表 不能获得前一个节点 使用交换的方法
                
                //换回调
                new->cb = node->cb; 
                node->cb = cb;
                //换时间
                new->time = node->time; //新的节点换成node->time
                node->time = time; //老的节点换成time


                list_node_timer_t *tmp;
                tmp = node->next;
                node->next = new;
                new->next = tmp;
                return 0;
            }
        }
        if (node->next) 
            node = node->next;
        else {
            // printf("t\n");
            node->next = new;
            return 0;
        }
    }
    return 0;
}


list_node_timer_t *list_timer_find_latest(list_t *list) {
    list_node_timer_t *node;
    if (list == NULL || list->first == NULL) 
        return NULL;
    
    //只寻找首节点
    node = list->first;
    //没有任何定时器到时
    if (node->time > time(NULL))
        return NULL;
    else {
        list->first = node->next;
        return node;
    }
        
}

int list_traveverse(list_t *list) {
    list_node_timer_t *node;
    if (!list)
        return -1;
    node = list->first;
    for (; node ;) {
        printf("node->time :%ld\n", node->time);
        node = node->next;
    }
    return 1;
}

void *test_cb(void * argc) {
    static int i = 0;
    printf("this is test_cb %d\n",i++);
}


void *add_timer_handler(void *argc) {
    int rc = 0;
    list_t *list = argc;
    for(;;) {

        pthread_mutex_lock(&list->mutex);
        printf("insert node->time %ld\n", time(NULL) + 1);
        rc =  list_timer_insert(list, time(NULL) + 1, test_cb);

        pthread_mutex_unlock(&list->mutex);
        
        random_usleep(1000 * 1000); //1s
    }

}

void *read_timer_handler(void *argc) {
    int rc = 0;
    list_t *list = argc;

    for(;;) {
        //这步 实际上是删除。
        pthread_mutex_lock(&list->mutex);
        list_node_timer_t *node = list_timer_find_latest(list);
        pthread_mutex_unlock(&list->mutex);
        if (node) {
            printf("find node->time: %ld\n", node->time);
            if (node->cb) {
                node->cb(NULL);
            }
            free(node);
        }else {
            printf("timer list is null\n");
        }
        
        random_usleep(1000 * 1000 ); //1s
    }

}


int main(int argc, char const *argv[])
{
    int rc = 0;
    list_t list;

    list_timer_init(&list);

    pthread_t add_timer_handler_tid1;
    pthread_t add_timer_handler_tid2;
    
    pthread_t read_timer_handler_tid1;
    pthread_t read_timer_handler_tid2;


    //别看 开了4个线程 实际上 就是单线程的性能
    rc = pthread_create(&add_timer_handler_tid1, NULL, add_timer_handler, &list);

    rc = pthread_create(&add_timer_handler_tid2, NULL, add_timer_handler, &list);

    rc = pthread_create(&read_timer_handler_tid1, NULL, read_timer_handler, &list);

    rc = pthread_create(&read_timer_handler_tid2, NULL, read_timer_handler, &list);

    // list_traveverse(&list);


    
    for(;;);
   
    return 0;
}
