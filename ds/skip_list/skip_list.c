#include "skip_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include <assert.h>
void *malloc_fault(size_t size) {
    void *p;
    p = calloc(1, size);
    if(p == NULL) {
        abort();
    }
    return p;
}



skip_list_node_t *skip_list_node_create(int level, double score, char *ele) {
    skip_list_node_t *new;
    new = malloc_fault(sizeof(skip_list_node_t) + level * sizeof(zskip_list_level_t));
    new->score = score;
    if (ele) {
        new->ele = strdup(ele);
    }
    
    return new;
}

void skip_list_node_free(skip_list_node_t *node) {
    free(node->ele);
    free(node);
}

skip_list_t *skip_list_create(void) {
    skip_list_t *zsl;
    int i ;
    zsl = malloc_fault(sizeof(skip_list_t));
    zsl->level = 1;
    zsl->length = 0;
    // the first level have all node ...
    zsl->header = skip_list_node_create(ZSKIPLIST_MAXLEVEL, 0, NULL);

    for (i = 0; i < ZSKIPLIST_MAXLEVEL; i++) {
        zsl->header->level[i].next = NULL;
        zsl->header->level[i].next_step = 0;
    }
    zsl->header->prev = NULL;
    zsl->tail = NULL;
    return zsl;
}


skip_list_node_t *skip_list_insert(skip_list_t *zsl, double score, char *ele) {
    int level;
    skip_list_node_t *header, *new, *update[ZSKIPLIST_MAXLEVEL];
    level = random_level();
    
    //set update 
    header = zsl->header;
    for (int i= zsl->level -1; i >= 0; i--) {
        //find the correct header... use score...
        skip_list_node_t *next;
        next = header->level[i].next;
        while(next && (next->score < score ||
                (next->score == score && strcmp(next->ele, ele) < 0))) {
            header = next;
            next = header->level[i].next;
        }
        update[i] = header;
    }

    if (level > zsl->level) {
        //select less than level's node into update
        for (int i = zsl->level; i < level; i++) {
            //find the correct header... beacause now we dont have data in bigger than zsl->level header,
            // so we set the update[i] with zsl->header immediately
            update[i] = zsl->header;
        }
        zsl->level = level;
    }
    //the more level number bigger, the new node alloc mem bigger ...
    // the first level have all node... but the node have the smallest mem..
    new = skip_list_node_create(level, score, ele);

    //update next...
    for (int i =0; i < level; i++) {
        skip_list_node_t *next;
        new->level[i].next = update[i]->level[i].next;
        update[i]->level[i].next = new;
    }

    //update prev...
    new->prev = (update[0] == zsl->header) ? NULL : update[0];

    //update next node's prev 
    if (new->level[0].next) {
        new->level[0].next->prev = new;
    }else {
        zsl->tail = new;
    }
    zsl->length++;
    return new;
}

void _skip_list_del(skip_list_t *zsl, skip_list_node_t *node, skip_list_node_t **update) {

    //update next
    for (int i = 0; i < zsl->level; i++) {
        skip_list_node_t *next = update[i]->level[i].next;
        if (next == node) {
            next = node->level[i].next;
        }
    }

    //update prev
    if (node->level[0].next) {
        node->level[0].next->prev = node->prev;
    } {
        zsl->tail = node->prev;
    }
    while (zsl->level > 1 && zsl->header->level[zsl->level -1].next == NULL) {
        zsl->level--;
    }
    zsl->length--;
}

bool skip_list_del(skip_list_t *zsl, double score, char *ele, skip_list_node_t **node) {
    skip_list_node_t *header, *update[ZSKIPLIST_MAXLEVEL], *del;

    header = zsl->header;
    //use high level and -- ,for perf ...
    for (int i = zsl->level-1; i >= 0; i--) {
        //find the correct header... use score...
        skip_list_node_t *next;
        next = header->level[i].next;
        while(next && (next->score < score ||
                (next->score == score && strcmp(next->ele, ele) < 0))) {
            header = next;
            next = header->level[i].next;
        }
        update[i] = header;
    }

    assert(update[0] == header);
    del = update[0]->level[0].next;
    
    if (del && del->score == score && strcmp(del->ele, ele) ==0) {
        _skip_list_del(zsl, del, update);
        if (!node) {
            skip_list_node_free(del);
        }else {
            *node = del;
        }
        return true;
    }

    return false; /*not found */
}

bool skip_list_find(skip_list_t *zsl, double score, char *ele, skip_list_node_t **node) {
    skip_list_node_t *ret;
    ret = zsl->header;

    for (int i=0; i < zsl->level; i++) {
        //find the correct header... use score...
        skip_list_node_t *next;
        next = ret->level[i].next;
        while(next && (next->score < score ||
                (next->score == score && strcmp(next->ele, ele) <= 0))) {
            ret = next;
        }
        if(ret->ele && ret->score == score && strcmp(ret->ele, ele)== 0) {

        }
    }
    return 0;
}

static void time_dump(int ticks, clock_t start, clock_t stop, struct tms *start_tms, struct tms *stop_tms)
{
    printf("  real time: %lf\n", (stop - start) / (double)ticks);
    printf("  user time: %lf\n", (stop_tms->tms_utime - start_tms->tms_utime) / (double)ticks);
    printf("  kern time: %lf\n", (stop_tms->tms_stime - start_tms->tms_stime) / (double)ticks);
}


#define BTREE_DEBUG 0
#define TEST_DEPTH 32
#define TEST_LEN 10000



char *err_stat[] = {
    "del node err",
    "del node ok"
};
int	main(int argc, char **argv) {
    
    skip_list_t *list;
    skip_list_node_t *node;
    bool rc;


    struct tms start_tms, stop_tms;
    clock_t start, stop;
    unsigned int count = 0, ticks;

    ticks = sysconf(_SC_CLK_TCK);
    list = skip_list_create();

    printf("insert %u Node:\n", TEST_LEN);
    start = times(&start_tms);
    for (int i = 0; i < TEST_LEN; ++i) {
        node = skip_list_insert(list, count++, "hello world");
        if (node == NULL) {
            printf("oom\n");
            abort();
            //log ...some error
        }
    }
    stop = times(&stop_tms);
    printf("the max levele %d\n", list->level);
    time_dump(ticks, start, stop, &start_tms, &stop_tms);

    rc = skip_list_del(list, 2, "hello world", NULL);
    printf("%s\n", err_stat[rc]);
    

    rc = skip_list_del(list, 2, "hello world", NULL);
    printf("%s\n", err_stat[rc]);

    return 0;
}
