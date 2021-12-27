#include "list.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 



#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#if 0
struct list_head {
    struct list_head *next, *prev;
};
//  ptr为member 的地址 , type 为整个结构体类型 member为结构体中的某个类型名称 返回结构体首地址
// 只有第一个参数比较正常 后面2个参数只有宏才能实现
#define container_of(ptr, type, member) ({                      \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)


#define list_for_each_entry(pos, head, member)  \
    for (pos = list_entry((head)->next, typeof(*pos), member);  \
         &pos->member != (head);    \
         pos = list_entry(pos->member.next, typeof(*pos), member))

static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}


 static inline void __list_add(struct list_head *_new,
                  struct list_head *prev,
                  struct list_head *next)
{
    next->prev = _new;
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is useful for implementing queues.
 */

static inline void
list_add(struct list_head *_new, struct list_head *head)
{
    __list_add(_new, head, head->next);
}

#endif 

struct list_head list;

struct data_node {
    struct list_head list_node;
    int num;
};


int main(int argc, char const *argv[]){

    // list_for_each_entry()

    INIT_LIST_HEAD(&list);

    struct data_node *new = malloc(sizeof(struct data_node));
    INIT_LIST_HEAD(&new->list_node);
    new->num = 10;

    //连接 list 和 list连接
    list_add(&new->list_node, &list);



    struct data_node * node = NULL;

    
    struct data_node a;
    a.num = 11;

    node =  container_of(&a.num, struct data_node, num);
    printf("%d\n",node->num);

    printf("%ld\n", offsetof(struct data_node, num));
    
    
    list_for_each_entry(node, &list, list_node){
        printf("node->num : %d\n",node->num);

    }

    // for (node = list_entry(list.next, typeof(*node), list);
    //     &node->num != &list;

    // ) {

    // }
}