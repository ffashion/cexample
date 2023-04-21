#include "token.h"
#include "parser.h"
#include <stdio.h>

int	main(int argc, char **argv) {
    Token *head;
    Node *node;

    if (argc != 2) {
        return -1;
    }

    head = tokenize(argv[1]);
    if (head == NULL) {
        return -1;
    }
    
    //skip head 
    node = parser(list_next_entry(head, list));

    if (node == NULL) {
        return -1;
    }


    printf("%d\n", compute(node));

    return 0;
}
