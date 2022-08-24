#ifndef __SKIP_LIST__H__
#define __SKIP_LIST__H__
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#define ZSKIPLIST_MAXLEVEL 32
#define ZSKIPLIST_P 0.25      /* Skiplist P = 1/4 */
// https://www.cnblogs.com/wangzming/p/15322048.html
typedef struct zskip_list_level {
    struct skip_list_node *next;
    unsigned long next_step;
} zskip_list_level_t;

typedef struct skip_list_node {
    char *ele;
    double score;
    struct skip_list_node *prev;
    zskip_list_level_t level[0];
    // struct zskip_list_node *next;
    // struct zskip_list_node *low_node;
}skip_list_node_t;

typedef struct {
    skip_list_node_t *header, *tail;
    unsigned long length;
    int level;
}skip_list_t;

static inline int random_level(void) {
    static const int threshold = ZSKIPLIST_P*RAND_MAX;
    int level = 1;
    while (random() < threshold)
        level += 1;
    return (level<ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
}

#endif  //!__SKIP_LIST__H__
