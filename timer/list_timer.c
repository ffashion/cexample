#include <stdio.h>
#include <time.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
typedef struct list_node_timer { 
    struct list_node_timer *next;
    time_t time;
}list_node_timer_t;


typedef struct list {
    list_node_timer_t *first;
}list_t;


int list_timer_insert(list_t *list, time_t time) {
    list_node_timer_t *node;
    list_node_timer_t *new;
    if (list == NULL)
        return -1;
    if(list->first == NULL) {
        list->first = malloc(sizeof(list_node_timer_t));
        node = list->first;
        node->next = NULL;
        node->time = time;
        return 0;
    }
    node = list->first;
    new =  malloc(sizeof(list_node_timer_t));
    new->next = NULL;
    new->time = time;

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


time_t list_timer_find_latest(list_t *list) {
    list_node_timer_t *node;
    time_t ret;
    if (list == NULL || list->first == NULL) 
        return -1;
    
    //只寻找首节点
    node = list->first;
    //没有任何定时器到时
    if (node->time > time(NULL))
        return -1;
    else {
        list->first = node->next;
        ret = node->time;
        free(node);
        return ret;
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

int main(int argc, char const *argv[])
{
    int rc = 0;
    list_t list;
    list.first = NULL;

    rc = list_timer_insert(&list, time(NULL) + 1);
    rc = list_timer_insert(&list, time(NULL) + 2);
    rc = list_timer_insert(&list, time(NULL) + 3);
    rc = list_timer_insert(&list, time(NULL) + 4);

    list_traveverse(&list);

    while(1) {
        time_t latest_time = list_timer_find_latest(&list);
        if (latest_time != -1)
            printf("find timer %ld\n",latest_time);
    }
    return 0;
}
