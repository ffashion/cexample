#include "mpool.h"
#include "token.h"
#include "parser.h"
#include <stdio.h>

int	main(int argc, char **argv) {
    Token *head;
    Node *node;
    mpool_t *pool;


    pool = mpool_create(MPOOL_DEFAULT_SIZE, NULL);
    if (pool == NULL) {
        return -1;
    }


    if (argc != 2) {
        return -1;
    }

    head = tokenize(argv[1], pool);
    if (head == NULL) {
        return -1;
    }
    
    //skip head 
    node = parser(list_next_entry(head, list), pool);

    if (node == NULL) {
        return -1;
    }

    printf("%d\n", compute(node));

    mpool_destroy(pool);

    return 0;
}
