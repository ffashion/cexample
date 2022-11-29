#include  "list.h"
#include  <stdbool.h>
#include  <assert.h>
#include  <stdio.h>
#include  <time.h>
#include  <signal.h>
#include  <unistd.h>
#include  <memory.h>
#include  <termio.h>
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))


typedef struct game_cycle {
    int tfd; /*terminal fd*/
    struct termios save;
    struct termios raw;
    int need_exit;
}game_cycle_t;

typedef struct cli_opt {
    int gb_size;
}cli_opt_t;

typedef struct coord {
    int x;
    int y;
}coord_t;

typedef struct game_board_node {
    struct list_head horizon;
    struct list_head vertical;
    int num;
    coord_t coord;
}game_board_node_t;

typedef struct game_board {
    int size; //a square of size * size

    game_board_node_t *left_top;   //  ptr (0, 0)
    game_board_node_t *left_down;  //  ptr  (0, -size)
    game_board_node_t *right_top;  //  ptr (size, 0)
    game_board_node_t *right_down; //  ptr(-size, -size)
    
    int score;
}game_board_t;

typedef struct game_ops game_ops_t;

enum game_op {
    game_op_up,
    game_op_down,
    game_op_left,
    game_op_right
};

typedef bool (*gb_update_pt)(game_board_t *gb, game_ops_t *ops);
typedef game_board_node_t* (*merge_direction_pt)(game_board_node_t* node, game_board_node_t* head);


struct game_ops {
    enum game_op op;
    gb_update_pt gb_merge;
    unsigned       corner;
    unsigned       head;
    merge_direction_pt merge_dir;
    merge_direction_pt merge_edge_dir;
};

bool gb_merge(game_board_t *gb, game_ops_t *ops);

game_board_node_t *
gb_get_up_node(game_board_node_t *node, game_board_node_t *head);

game_board_node_t *
gb_get_down_node(game_board_node_t *node, game_board_node_t *head);

game_board_node_t *
gb_get_right_node(game_board_node_t *node, game_board_node_t *head);

game_board_node_t *
gb_get_left_node(game_board_node_t *node, game_board_node_t *head);

bool 
gb_node_is_null(game_board_node_t *node);

bool 
gb_merge_edge(game_board_t *gb, game_board_node_t *head, game_ops_t *ops);

bool 
gb_merge_part_edge(game_board_t *gb, game_board_node_t *node, game_board_node_t *head, game_ops_t *ops);

bool 
gb_merge_node(game_board_t *gb, game_board_node_t *node, game_board_node_t *second);

game_board_node_t *
gb_find_node_by_coordinate(game_board_t *gb, int x, int y);

struct game_ops gops_tables[]  = {
    {
        .op = game_op_up,
        .gb_merge = gb_merge,
        .corner = offsetof(game_board_t, left_top),
        .merge_dir = gb_get_right_node,
        .merge_edge_dir = gb_get_down_node,
    },
    {
        .op = game_op_down,
        .gb_merge = gb_merge,
        .corner = offsetof(game_board_t, left_down),
        .merge_dir = gb_get_right_node,
        .merge_edge_dir = gb_get_up_node,
    }, 
    {
        .op = game_op_left,
        .gb_merge = gb_merge,
        .corner = offsetof(game_board_t, left_top),
        .merge_dir = gb_get_down_node,
        .merge_edge_dir = gb_get_right_node,
    },
    {
        .op = game_op_right,
        .gb_merge = gb_merge,
        .corner = offsetof(game_board_t, right_top),
        .merge_dir = gb_get_down_node,
        .merge_edge_dir = gb_get_left_node,
    }
};

static game_cycle_t cycle;

// 0 stand for the node is null node
inline bool 
gb_node_is_null(game_board_node_t *node) {
    return node->num == 0;
}

inline game_board_node_t *
gb_get_up_node(game_board_node_t *node, game_board_node_t *head) {
    game_board_node_t *ret;
    ret = list_prev_entry(node, vertical);
#if 0
    printf("(%d, %d)up node is (%d, %d)\n", node->coord.x, node->coord.y, ret->coord.x, ret->coord.y);
#endif
    if (ret == head) {
        return NULL;
    }
    return ret;
}

inline game_board_node_t *
gb_get_down_node(game_board_node_t *node, game_board_node_t *head) {
    game_board_node_t *ret;
    ret = list_next_entry(node, vertical);
#if 0
    printf("(%d, %d) down node is (%d, %d)\n", node->coord.x, node->coord.y, ret->coord.x, ret->coord.y);
#endif
    if (ret == head) {
        return NULL;
    }
    return ret;
}

inline game_board_node_t *
gb_get_left_node(game_board_node_t *node, game_board_node_t *head) {
    game_board_node_t *ret;
    ret = list_prev_entry(node, horizon);
#if 0
    printf("(%d, %d)left node is (%d, %d)\n", node->coord.x, node->coord.y, ret->coord.x, ret->coord.y);
#endif
    if (ret == head) {
        return NULL;
    }
    return ret;
}

inline game_board_node_t *
gb_get_right_node(game_board_node_t *node, game_board_node_t *head) {
    game_board_node_t *ret;
    ret = list_next_entry(node, horizon);
#if 0
    printf("(%d, %d)right node is (%d, %d)\n", node->coord.x, node->coord.y, ret->coord.x, ret->coord.y);
#endif
    if (ret == head) {
        return NULL;
    }
    return ret;
}

/*
we need itrator the whole list... and if the first next node dont match(equal), just skip this node 
if the first next node matched, we should merge the  next node to this node. and we should get next next node. untile we dont match
 and we set node to next, if current node is zero.. we merge the next node.
*/

/*
    x x x 1       x x x 2       x x x 2     x x x 2
    x x x 1   ==> x x x 0   ==> x x x 1 ==> x x x 2
    x x x 1       x x x 1       x x x 0     x x x 0
    x x x 1       x x x 1       x x x 1     x x x 0
*/

inline bool 
gb_merge_node(game_board_t *gb, game_board_node_t *node, game_board_node_t *second) {
    if (gb_node_is_null(node) || node->num == second->num) {
        if (node->num == second->num) {
            gb->score += node->num * 2;
        }
        node->num += second->num;
        second->num = 0;
        return true;
    }
    return false;
}

inline bool 
gb_merge_part_edge(game_board_t *gb, game_board_node_t *node, game_board_node_t *head, game_ops_t *ops) {
    game_board_node_t *next;
    bool merge = false;
    next = node;
    for (;;) {
        if ((next = ops->merge_edge_dir(next, head)) == NULL) {
            return merge;
        }

        if (gb_node_is_null(next)) {
            continue;
        }

        if (gb_merge_node(gb, node, next)) {
            merge = true;
            continue;
        }

        return merge;
    }

    return merge;
}


inline bool 
gb_merge_edge(game_board_t *gb, game_board_node_t *head, game_ops_t *ops) {
    game_board_node_t *node;
    int i;
    bool merge = false;
    node = head;

    for (i = 0; i < gb->size; i++) {
        merge |= gb_merge_part_edge(gb, node, head, ops);
        node = ops->merge_edge_dir(node, head);
    }
 
    return merge;
}


inline bool 
gb_merge(game_board_t *gb, game_ops_t *ops) {
    game_board_node_t *node, **corner;
    char *p;
    int i;
    bool merge = false;
    p = (void *)gb;

    corner = (game_board_node_t **) (p + ops->corner);

    node = *corner;
    
    for (i = 0; i < gb->size; i++) {
        merge |= gb_merge_edge(gb, node, ops);
        node = ops->merge_dir(node, *corner);
    }
    return merge;
}

void gb_init_line_list(game_board_t *gb, game_board_node_t* head, int y) {
    int i;
    game_board_node_t *node, *prev;

    INIT_LIST_HEAD(&head->horizon);
    
    node = head;

    node++; /*skip head*/
    prev = head;

    node->coord = (coord_t){0, y};

    for (i = 0; i < gb->size -1; i++) {
        list_add(&node->horizon, &prev->horizon);
        node->coord = (coord_t){i + 1, y};
        prev = node;
        node++;
    }
}

void gb_init_vert_list(game_board_t *gb, game_board_node_t* head, int x) {
    int i;
    game_board_node_t* node, *prev;
    INIT_LIST_HEAD(&head->vertical);

    node = head;

    node += gb->size;
    prev = head;
    node->coord = (coord_t){x, 0};

    for (i = 0; i < gb->size -1; i++) {
        list_add(&node->vertical, &prev->vertical);
        node->coord = (coord_t){x, i+1};
        prev = node;
        node += gb->size;
    }
}

game_board_t *gb_create(int size) {
    game_board_t *gb;
    game_board_node_t *node, *start;
    int i;
    gb = malloc(sizeof(game_board_t));
    if (gb == NULL) {
        return NULL;
    }

    memset(gb, 0, sizeof(game_board_t));

    gb->size = size;

    node = malloc(sizeof(game_board_node_t) * (size * size));
    if (node == NULL) {
        //FIXME: free the gb 
        return NULL;
    }
    memset(node, 0, sizeof(game_board_node_t) * (size * size));

    gb->left_top =  node;
    gb->right_top = gb->left_top + (size -1);

    // printf("offset :%ld\n", gb->right_top - gb->left_top);

    gb->left_down = gb->left_top + (size * (size - 1));

    gb->right_down = gb->left_down + (size -1);

    // printf("offset :%ld\n", gb->right_down - gb->left_top);

    //init the list

    start = gb->left_top;
    //init the horizon direction
    for (i = 0; i < size ; i++) {
        gb_init_line_list(gb, start, i);
        start += size;
    }
    

    //init the vertical direction
    start = gb->left_top;
    for (i = 0 ; i < size; i++) {
        gb_init_vert_list(gb, start, i);
        start++;
    }

    return gb;
}

bool gb_destroy(game_board_t *gb) {
    free(gb->left_top);
    free(gb);
    return true;
}

int range_radom(int min, int max) {
    srand(time(NULL));
    return (rand() % (max - min + 1)) + min;
}

game_board_node_t *
gb_find_node_by_coordinate(game_board_t *gb, int x, int y) {
    game_board_node_t *node, *line, *verical;
    if (x > gb->size || y > gb->size) {
        return NULL;
    }

    line = gb->left_top;
    node = line;

    for (; x ; x--) {
        assert(node);
        node = gb_get_right_node(node, line);
    }
    
    verical = node;

    for (; y ; y--) {
        assert(node);
        node = gb_get_down_node(node, verical);
    }

    return node;
}

bool gb_init_with_random(game_board_t *gb) {
    int x0, y0, x1, y1;
    game_board_node_t *node;

    x0 = range_radom(0, gb->size - 1);
    y0 = range_radom(0, gb->size / 2 - 1);

    x1 = range_radom(0, gb->size - 1);
    y1 = range_radom(gb->size / 2, gb->size - 1);

#if 0
    printf("%d, %d, %d, %d\n", x0, y0, x1, y1);
#endif
    
    node = gb_find_node_by_coordinate(gb, x0, y0);
    if (node == NULL) {
        return false;
    }
    node->num = 2;

    node = gb_find_node_by_coordinate(gb, x1, y1);
    if (node == NULL) {
        return false;
    }
    node->num = 2;
    return true;
}

bool gb_init_with_nums(game_board_t *gb, int size, int *nums) {
    int i, j;
    game_board_node_t *node, *head, *line;
    if (gb->size * gb->size != size) {
        return false;
    }

    head = gb->left_top;
    line = head;
    node = line;

    for (i = 0; i < gb->size; i++) {

        for (j = 0; j < gb->size; j++) {
            node->num = nums[i * gb->size + j];
            node = gb_get_right_node(node, line);
        }
        line = gb_get_down_node(line, head);
        node = line;
    }

    return true;
}

game_board_node_t *
gb_find_null_node_by_iter_line(game_board_t *gb, game_board_node_t *line) {
    game_board_node_t *node;
    int i;

    node = line;
    for (i = 0; i < gb->size; i++) {
        if (gb_node_is_null(node)) {
            return node;
        }

        node = gb_get_right_node(node, line);
        
    }
    return NULL;
}

game_board_node_t 
*gb_find_null_node_by_iter(game_board_t *gb) {
    game_board_node_t *node, *line;
    int i;
    line = gb->left_top;

    for (i = 0 ; i < gb->size; i++) {

        if ((node = gb_find_null_node_by_iter_line(gb, line)) != NULL) {
            return node;
        }

        line = gb_get_down_node(line, gb->left_top);
    }

    return NULL;
}

game_board_node_t *gb_find_null_node(game_board_t *gb){
    int tries, i, x0, y0;
    game_board_node_t *node;
    tries = 3;

    for (i = 0; i < tries; i++) {
        x0 = range_radom(0, gb->size - 1);
        y0 = range_radom(0, gb->size - 1);

        node = gb_find_node_by_coordinate(gb, x0, y0);

        if (gb_node_is_null(node)) {
            return node;
        }
    }

#if 0
    printf("gb_find_null_node_by_iter \n");
#endif

    return gb_find_null_node_by_iter(gb);
}

bool gb_update(game_board_t *gb, enum game_op op) {
    int i;
    struct game_ops *ops;
    game_board_node_t *node;
    bool merge = false;

    for (i = 0; i < ARRAY_SIZE(gops_tables); i++) {
        ops = &gops_tables[i];
        if (ops->op == op) {
            merge = ops->gb_merge(gb, ops);
            break;
        }
    }

#if 0
    printf("merge %d\n", merge);
#endif

    if (!merge) {
        //dont add a new node
        return true;
    }

    //need add a 'num = 2' node
    node = gb_find_null_node(gb);
    if (node == NULL) {
        //dead
        return false;
    }
    node->num = 2;
    return true;
}

game_board_node_t *
gb_get_next_line(game_board_node_t *node, game_board_node_t *head) {
    

    return node;
}

void 
gb_draw_line(game_board_t *gb, game_board_node_t *line) {
    game_board_node_t *node;
    int i;

    node = line;
    
    for (i = 0; i < gb->size; i++) {
        printf("%4d ", node->num);
        node = gb_get_right_node(node, line);
    }
    printf("\n");
}

void gb_draw(game_board_t *gb) {
    game_board_node_t *line;
    int i;

    line = gb->left_top;

    printf("score: %d\n", gb->score);
    for (i = 0; i < gb->size; i++) {
        gb_draw_line(gb, line);
        line = gb_get_down_node(line, gb->left_top);
    }

    printf("\n");
#if 0
    line = gb->left_down;
    for (i = 0; i < gb->size; i++) {
        gb_draw_line(gb, line);
        line = gb_get_up_node(line, gb->left_top);
    }
#endif

#if 0
    game_board_node_t *node;
    node = gb->right_top;
    for (i = 0 ; i < gb->size -1; i++) {
        node = gb_get_left_node(node, gb->left_top);
    }

    assert(node->horizon.next == gb->left_top->horizon.next);
#endif

    return;
}
cli_opt_t read_cli_opt(int argc, char **argv) {  
    cli_opt_t opt;

    //set Default value
    opt.gb_size = 4;

    if (argc == 2) {
        opt.gb_size = atoi(argv[1]);
        if (opt.gb_size == 0) {
            printf("illegal gb size %d\n", opt.gb_size);
            exit(EXIT_FAILURE);
        }
    }
    return opt;
}

void process_signal() {
    
    if (cycle.need_exit) {
        if (tcsetattr(STDIN_FILENO, TCSANOW, &cycle.save) != 0) {
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    return;
}


void console_handler(int sig) {
    cycle.need_exit = 1;
}


#define KEYCODE_U 0x41
#define KEYCODE_D 0x42
#define KEYCODE_L 0x44
#define KEYCODE_R 0x43

enum game_op read_op_from_cli() {
    enum game_op op;
    char key_op;
    if (read(STDIN_FILENO, &key_op, 1) < 0) {
        return -1;
    }
    
    switch (key_op) {
        case KEYCODE_U:
        case 'w':
            op = game_op_up;
            break;
        case KEYCODE_D:
        case 's':
            op = game_op_down;
            break;
        case KEYCODE_L:
        case 'a':
            op = game_op_left;
            break;
        case 'd':
        case KEYCODE_R:
            op = game_op_right;
            break;
        case 'q':
            cycle.need_exit = 1;
        default:
            return -1;
    }

    return op;
}

int init_console(game_cycle_t *cycle) {
    struct termios raw;
    if (tcgetattr(STDIN_FILENO, &cycle->save) != 0) {
        return -1;
    }
    
    memcpy(&raw, &cycle->save, sizeof(struct termios));
    raw.c_lflag &=~ (ICANON | ECHO);
    raw.c_cc[VEOL] = 1;
    raw.c_cc[VEOF] = 2;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    cycle->raw = raw;

    signal(SIGINT, console_handler);

    return 0;
}


int	main(int argc, char **argv) {
    game_board_t *gb;
    cli_opt_t opt;
    enum game_op op = 0;
    
    init_console(&cycle);

    opt = read_cli_opt(argc, argv);
    
    if ((gb = gb_create(opt.gb_size)) == NULL) {
        return -1;
    }

    if (!gb_init_with_random(gb)) {
        return -1;
    }
#define DDEBUG 0
#if DDEBUG
    int nums[16] = {
    2, 4, 8, 16, 
    32, 64, 0, 0,
    0, 0, 0, 0, 
    0, 0, 0, 0
    };
    if (!gb_init_with_nums(gb, ARRAY_SIZE(nums), nums)) {
        gb_destroy(gb);
        return -1;
    }
#endif 

    gb_draw(gb);
    
    for (;;) {
        //process signal 
        process_signal();
        //read op from cli
        op =  read_op_from_cli();
        if (op == -1) {
            //skip ignal char
            continue;
        }
        if (!gb_update(gb, op)) {
            gb_destroy(gb);
            printf("You Dead!!\n");
            return -1;
        }
        gb_draw(gb);
    }

    gb_destroy(gb);
    return 0;
}
