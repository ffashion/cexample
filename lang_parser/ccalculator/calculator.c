#include "mpool.h"
#include "token.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

int	main(int argc, char **argv) {
    Token *head;
    Node *node;
    mpool_t *pool;
    char *token;

    token = getenv("COMPUTE");

    if (argc == 2) {
        token = argv[1];
    }
    
    if (token == NULL) {
        return -1;
    }

    pool = mpool_create(MPOOL_DEFAULT_SIZE, NULL);
    if (pool == NULL) {
        return -1;
    }
    
    head = tokenize(token, pool);
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
