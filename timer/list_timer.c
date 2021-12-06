#include <stdio.h>
#include <time.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

typedef void* (*timer_cb)(void *);

typedef struct list_node_timer { 
    struct list_node_timer *next;
    time_t time;
    timer_cb cb;
}list_node_timer_t;


typedef struct list {
    list_node_timer_t *first;
}list_t;


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
    //比某个节点小 插入到他的前面
    for (; node ;) {
        if (time < node->time) {
            if (node = list->first) {
                list->first = new;
                new->next = node;
                return 0;
            }else {
                //由于使用单链表 不能获得前一个节点
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
int main(int argc, char const *argv[])
{
    int rc = 0;
    list_t list;
    list.first = NULL;

    rc = list_timer_insert(&list, time(NULL) + 1, test_cb);
    rc = list_timer_insert(&list, time(NULL) + 2, test_cb);
    rc = list_timer_insert(&list, time(NULL) + 3, test_cb);
    rc = list_timer_insert(&list, time(NULL) + 4, test_cb);

    list_traveverse(&list);
    while(1) {
        list_node_timer_t *node = list_timer_find_latest(&list);
        if (node) {
            printf("find node->time: %ld\n", node->time);
            if (node->cb) {
                node->cb(NULL);
            }
            free(node);
        }
    }
    return 0;
}
