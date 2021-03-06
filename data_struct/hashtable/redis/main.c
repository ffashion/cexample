#include "dict.h"
#include "string.h"
#include <stdio.h>
uint64_t dictStrHash(const void *key) {
    return dictGenHashFunction((unsigned char*)key, strlen((char*)key));
}
int dictKeyCompare(dict *d, const void *key1,
        const void *key2)
{
    int l1,l2;
    (void)d;

    l1 = strlen((char *)key1);
    l2 = strlen((char *)key2);
    if (l1 != l2) return 0;
    return memcmp(key1, key2, l1) == 0;
}

void dictKeyDestructor(dict *d, void *val)
{
    (void)d, (void)val;
    free(val);
}

void dictValDestructor(dict *d, void *val)
{
    (void)d, (void)val;

    free(val);
}

int dictExpandAllowed(size_t moreMem, double usedRatio) {
    //Check the memory size to determine whether the hash table can be expanded
    (void)moreMem;
    (void)usedRatio;
    return 1;
}


size_t dictEntryMetadataSize(dict *d) {
   (void)d;
   //we can use metadata to store the expire time
   return 0;
}


dictType hashDictType = {
    dictStrHash,
    NULL,
    NULL,
    dictKeyCompare,
    dictKeyDestructor,
    dictValDestructor,
    dictExpandAllowed,
    dictEntryMetadataSize
};

int	main(int argc, char **argv) {
    (void)argc;
    (void)argv;


    dict *d;

    dictEntry *entry;

    d = dictCreate(&hashDictType);


    for (int i = 0; i <= 10; i++) {
        char *key = malloc(10);
        char *val = malloc(10);
        memset(key, 0, 10);
        sprintf(key, "key%d", i);
        sprintf(val, "value");
        //if dict have this key, we dont insert.
        dictAdd(d, key, val);
    }

    for (int i = 0; i <= 10; i++) {
        char key[10];
        memset(key, 0, 10);
        sprintf(key, "key%d", i);
        entry = dictFind(d, key);
        printf("%s->%s\n",(char *)entry->key,(char *)entry->v.val);
    }
    

    for (int i = 0; i <= 10; i++) {
        char key[10];
        memset(key, 0, 10);
        sprintf(key, "key%d", i);
        dictDelete(d, key);
    }


    char *key = malloc(10);
    for (int i = 0; i <= 10; i++) {
        char *val = malloc(10);
        sprintf(key, "key");
        sprintf(val, "value%d", i);

        if (dictReplace(d, key, val) == 1) {
            printf("scratch: %s->%s\n",(char *)key,val);
        }else {
            printf("already: %s->%s\n",(char *)key,(char *)val);
        }
    }
    entry = dictFind(d, "key");
    printf("%s->%s\n",(char *)entry->key,(char *)entry->v.val);


    dictRelease(d);

    // printf("%s\n", (char *)entry->v.val);
    return 0;
}
