#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

uint32_t ceil_to_power_of_2(uint32_t x)
  { return (UINT32_MAX >> __builtin_clz(x - 1)) + 1;  }

#if 0
static inline int align_power2(int n) {
    n |= n  >> 1;
    n |= n  >> 2;
    n |= n  >> 4;
    n |= n  >> 8;
    n |= n  >> 16;
    return n + 1;
}
#endif
typedef struct map_node{
    char *key;
    char *value;
    struct map_node *next;
}map_node_t;

typedef struct {
    int bucket;
    map_node_t *map_node; //this is a array, dynamic alloc
}map_t;
//djb hash function
static unsigned map_hash(const char *str) {
    unsigned hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) ^ *str++;
    }
    return hash;
}

int map_insert(map_t *map,const char *key,const char *value) {
    
    unsigned unsign_key = map_hash(key);
    int bucket = map->bucket;
    //第一个节点无法判断是否存了值(因为在init的时候 就已经分配了空间 head就已经有了值)
    map_node_t *current = &map->map_node[unsign_key & (bucket - 1)];
    map_node_t *node = current;
    
    if (!node->value) {
        node->key   =  strdup(key);
        node->value =  strdup(value);
        node->next = NULL;
        return 0;
    }

    for( ; node->next; node = node->next){
        //we alrady have a same key-val, just update it.
        if(strcmp(node->next->key,key) == 0) {
            free(node->next->value);
            node->next->value = strdup(value);
            return 0;
        }
        
    }

    //we not have a same key-val, we need insert a new key-value to the map
    map_node_t *new_node = (map_node_t *)malloc(sizeof(map_node_t));
    node->next = new_node;
    new_node->key   =  strdup(key);
    new_node->value =  strdup(value);
    new_node->next = NULL;
    return 0;
}

map_node_t *map_search(map_t *map,const char *key) {
    unsigned unsign_key = map_hash(key);
    int bucket = map->bucket;
    map_node_t *current = &map->map_node[unsign_key & (bucket - 1)];
    map_node_t *node = current;
    
    
    for( ; node && node->key; node = node->next) {
        if(strcmp(key,node->key) == 0) {
            return node;
        }
    }
    return NULL;
}

int map_delete(map_t *map,const char *key) {
    unsigned unsign_key = map_hash(key);
    int bucket = map->bucket;
    map_node_t *current = &map->map_node[unsign_key & (bucket - 1)];
    map_node_t *node = current;

    for( ; node->next; node = node->next){
        if(strcmp(node->next->key,key) == 0) {
            //now we should delete node->next
            map_node_t *save = node->next;
            node->next = node->next->next;
            free(save->key);
            free(save->value);
            free(save);
            return 0;
        }
    }
    //we not find this key
    return -1;
}
int map_free(map_t *map){
    for(int i=0 ; i < map->bucket ; i++) {
        map_node_t *current = &map->map_node[i];
        map_node_t *node = current;
        
        free(node->key);
        free(node->value);
        node = node->next;
        
        for( ; node ;) {
            map_node_t *next = node->next;
            free(node->key);
            free(node->value);
            free(node);
            node = next;
        }
    }
    free(map->map_node);
    return 0;
}

int map_init(map_t *map,int bucket){
    map->bucket = ceil_to_power_of_2(bucket);
    map->map_node = (map_node_t *)malloc(map->bucket * sizeof(map_node_t));
    memset(map->map_node,0,map->bucket * sizeof(map_node_t));
    return 0;
}

int main(int argc, char const *argv[])
{   
    map_t map;
    map_node_t *node;
    char *key_friend = "friend";
    char *key_father = "father";
    char *value;
    int rc;


    map_init(&map,1000);
    map_insert(&map,key_friend,"kity");
    map_insert(&map,key_father, "zhang");
    map_insert(&map,key_father,"whang");


    node = map_search(&map,key_friend);
    if (node) {
        printf("%s:%s\n",key_father,node->value);
    }else {
        printf("we dont find %s node\n", key_father);
    }

    node  = map_search(&map,key_father);
    if (value) {
        printf("%s:%s\n",key_father,node->value);
    }else {
        printf("we dont find %s node\n", key_father);
    }
    
    if(map_delete(&map, key_father) == 0) {
        printf("delete node %s ok\n",key_father);
    }else {
        printf("we dont find %s node\n", key_father);
    }
    
    node  = map_search(&map,key_father);
    if (node) {
        printf("%s:%s\n",key_father,node->value);
    }else {
        printf("we dont find %s node\n", key_father);
    }

    map_free(&map);

    return 0;
}
