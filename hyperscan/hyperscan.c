#include <hs/hs.h>
#include <hs/ch.h>
#include <string.h>
#include <stdio.h>
#include  <sys/stat.h>
#include  <assert.h>
#include  <stdlib.h>
#include  <stdlib.h>

static int 
ch_match_handler(unsigned int id,
                                unsigned long long from,
                                unsigned long long to,
                                unsigned int flags,
                                unsigned int size,
                                const ch_capture_t *captured,
                                void *ctx);
int
hs_event_handler(unsigned id,unsigned long long from,unsigned long long to,
unsigned int flags,void *ctx);

int hs_pattern_match(struct dict *dict, struct str *hys);
typedef int (*pattern_match)(struct dict *dict, struct str *hys);

struct array {
    void *elts;
    int  nelts;
    size_t size;
    int  nalloc;
};

struct hs_db {
    void *db;
    void *scratch;
    int ishs;
};

struct str {
    char *data;
    int  len;
};

struct pattern {
    struct str str;
    int id;
    void *ctx;
};

struct dict {
    struct array *patterns;
    struct hs_db hdb;
    pattern_match match;
};

struct request {
    struct array *strs;
};


struct array *array_create(int nalloc, size_t size) {
    struct array *array;
    array = (struct array *)malloc(sizeof(struct array));
    
    array->elts = malloc(size * nalloc);
    array->nelts = 0;
    array->size = size;
    array->nalloc = nalloc;

    return array;
}

void *array_push(struct array *a) {
    void        *elt, *mynew = NULL;
    size_t       size = 0;
    if (a->nelts == a->nalloc) {
        assert(size);
        // assert(mynew);
        printf("fuck you\n");
        exit(-1);
    }
    elt = (u_char *) a->elts + a->size * a->nelts;
    a->nelts++;
    return elt;
}


#define PATTERN_MAX_SIZE 1024
struct dict *read_dict(char *file) {
    FILE *fp = NULL;
    fp = fopen(file, "r");
    struct dict *dict = (struct dict *)malloc(sizeof(struct dict));
    dict->patterns = array_create(10240, sizeof(struct pattern));

    assert(dict);

    assert(fp);
    int id = 0;
    for (;;) {
        char buf[PATTERN_MAX_SIZE];
        int len;
        memset(buf, 0, sizeof(buf));
        if (fgets(buf, sizeof(buf), fp) == NULL) {
            if (feof(fp)) {
                break;
            }else {
                fclose(fp);
                return NULL;
            }
        }
        //remove last's LF
        len = strlen(buf);
        buf[len -1] = '\0';

        struct pattern *pattern = (struct pattern *)array_push(dict->patterns);
        pattern->str.data  =  strdup(buf); //need remove \n
        
        assert(pattern->str.data);
        // memcpy(pattern->str, buf, strlen(buf) + 1);
        pattern->str.len = len - 1;
        pattern->id = id++;
    }

    dict->match = hs_pattern_match;
    return dict;
}

#define PATTERN_MAX_SIZE 1024
struct request *read_requests(char *file) {
    FILE *fp = NULL;
    fp = fopen(file, "r");
    assert(fp);

    struct request *request = (struct request *)malloc(sizeof(struct request));
    request->strs = array_create(10240, sizeof(struct str));

    for (;;) {
        char buf[PATTERN_MAX_SIZE];
        memset(buf, 0, sizeof(buf));
        if (fgets(buf, sizeof(buf), fp) == NULL) {
            if (feof(fp)) {
                break;
            }else {
                fclose(fp);
                return NULL;
            }
        }
        struct str *master = (struct str *)array_push(request->strs);
        master->data =  strdup(buf);
        assert(master->data);
        // memcpy(str->str, buf, strlen(buf) + 1);
        master->len = strlen(buf);
    }
    return request;
}


int hs_pattern_match(struct dict *dict, struct str *hys) {
    if (dict->hdb.ishs) {
        //FIXME
    }else {
        // HS_SCAN_TERMINATED
        if (ch_scan((ch_database_t *)dict->hdb.db, hys->data, hys->len, 0, (ch_scratch_t *)dict->hdb.scratch, ch_match_handler, NULL, dict) != HS_SUCCESS) {
            printf("ch_scan error\n");
            return -1;
        }   
    }
    return 0;
}

int try_prepare_hs(const char **expressions, int npattern, unsigned *ids, struct dict *dict) {
    hs_compile_error_t *hs_cerr;
    hs_database_t *hsdb;
    hs_scratch_t  *hs_scratch;
    
    if (hs_compile_multi(expressions, 0, ids, npattern, HS_MODE_BLOCK, NULL, &hsdb, &hs_cerr) != HS_SUCCESS) {
        hs_free_compile_error(hs_cerr);
        return -1;
    }

    // we only need one hs_scratch in one program. FIXME: remove it ...
    if (hs_alloc_scratch(hsdb, &hs_scratch) != HS_SUCCESS) {
        hs_free_database(hsdb);
        return -1;
    }
    dict->hdb.db = hsdb;
    dict->hdb.scratch = hs_scratch;
    dict->hdb.ishs = 1;
    return 0;
}

int try_prepare_ch(const char **expressions, int npattern, unsigned *ids, struct dict *dict) {
    ch_compile_error_t *ch_cerr;
    ch_database_t      *chdb;
    ch_scratch_t       *ch_scratch;
    // http://intel.github.io/hyperscan/dev-reference/api_constants.html#pattern-flags
    if (ch_compile_multi(expressions, 0, ids, npattern, CH_MODE_GROUPS, NULL, &chdb, &ch_cerr) != HS_SUCCESS) {
        printf("%s\n", ch_cerr->message);
        ch_free_compile_error(ch_cerr);
        return -1;
    }
    if (ch_alloc_scratch(chdb, &ch_scratch) != HS_SUCCESS) {
        ch_free_database(chdb);
        return -1;
    }
    dict->hdb.db = chdb;
    dict->hdb.scratch = ch_scratch;
    dict->hdb.ishs = 0;
    return 0;
}


int main(int argc, char const *argv[]){
    int nrequest, i, npattern, j;
    struct dict * dict = read_dict((char *)"./pattern.txt");
    struct request *r = read_requests((char *)"./requests.txt");


    const char *expressions[10240];
    unsigned ids[10240] = {0};
    npattern =  dict->patterns->nelts;
    struct pattern *nds = (struct pattern *)dict->patterns->elts; //nd stand for needles

for (i = 0; i < npattern; i++) {
        expressions[i] = nds[i].str.data;
        ids[i] = nds[i].id;
    }

    if (try_prepare_hs(expressions, npattern, ids, dict) == 0) {
    }else if (try_prepare_ch(expressions,npattern, ids, dict) == -1) {
        printf("prepare ch error exit\n");
        return -1;
    }
    
    struct str *hys = (struct str *)r->strs->elts;
    nrequest = r->strs->nelts;

    for (i = 0; i < nrequest; i++) {
        if (dict->match(dict, &hys[i]) == 0) {
            // printf("%s match!!!\n", hys[i].data);
        }
    }

    return 0;
}


int 
hs_event_handler(unsigned id,unsigned long long from,unsigned long long to,
unsigned int flags,void *ctx){
    struct dict *dict = (struct dict *)ctx;

    printf("id %d from %llu to %llu\n", id, from, to);



    return true;
}

static int 
ch_match_handler(unsigned int id,
                                unsigned long long from,
                                unsigned long long to,
                                unsigned int flags,
                                unsigned int size,
                                const ch_capture_t *captured,
                                void *ctx) {
    // http://intel.github.io/hyperscan/dev-reference/api_files.html#c.match_event_handler
    struct dict *dict = (struct dict *)ctx;
    // [form, to)
    char *data = (char *)ctx;

    printf("id %d from %llu to %llu\n", id, from, to);
    //we can't memset data here...

    //0 stand for next match
    return 0;

}
