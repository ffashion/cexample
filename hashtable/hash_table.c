#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct map_node{
    char *key;
    int value;
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

int map_insert(map_t *map,char *key,int value) {
    
    unsigned unsign_key = map_hash(key);
    int bucket = map->bucket;
    //第一个节点无法判断是否存了值 所以永远不存值
    map_node_t *current = &map->map_node[unsign_key % bucket];
    map_node_t *node = current;
    
    for(;node->next ; node = node->next){
        if(strcmp(node->next->key,key) == 0) {
            node->next->value = value;
            return 0;
        }
        
    }

    //以下此时node 为最后一个节点
    map_node_t *new_node = (map_node_t *)malloc(sizeof(map_node_t));
    node->next = new_node;

    new_node->key =  strdup(key);
    new_node->value = value;
    new_node->next = NULL;
    return 0;
}

int map_search(map_t *map,char *key) {
    unsigned unsign_key = map_hash(key);
    int bucket = map->bucket;
    map_node_t *current = &map->map_node[unsign_key % bucket];
    map_node_t *node = current->next;
    for( ;node ; node = node->next){
        if(strcmp(key,node->key) == 0) {
            return node->value;
        }
    }
    return 0;
}
int map_free(map_t *map){
    for(int i=0 ; i<map->bucket ; i++) {
        map_node_t *current = &map->map_node[i];
        map_node_t *node = current->next;
        
        for( ; node ;) {
            map_node_t *next = node->next;
            free(node->key);
            free(node);
            node = next;
        }
    }
    free(map->map_node);
    return 0;
}

int map_init(map_t *map,int bucket){
    map->bucket = bucket;
    map->map_node = (map_node_t *)malloc(bucket * sizeof(map_node_t));
    memset(map->map_node,0,bucket * sizeof(map_node_t));
    return 0;
}

int main(int argc, char const *argv[])
{   
    map_t map;

    map_init(&map,1000);
    map_insert(&map,"hello",4);
    map_insert(&map,"hello",2);
    map_insert(&map,"what",20);
    int rc  = map_search(&map,"hello");
    printf("%d\n",rc);
    rc  = map_search(&map,"what");
    printf("%d\n",rc);

    map_free(&map);

    return 0;
}
