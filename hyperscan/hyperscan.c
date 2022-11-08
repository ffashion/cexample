#include <hs/hs.h>
#include <hs/ch.h>
#include <string.h>
#include <stdio.h>
#include  <sys/stat.h>
#include  <assert.h>
#include  <stdlib.h>
#include  <stdlib.h>

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
    void *ctx;
};

struct dict {
    struct array *patterns;
    struct hs_db *hdb;
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
        struct pattern *pattern = (struct pattern *)array_push(dict->patterns);
        pattern->str.data  =  strdup(buf);
        assert(pattern->str.data);
        // memcpy(pattern->str, buf, strlen(buf) + 1);
        pattern->str.len = strlen(buf);
    }
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


int 
event_handler(unsigned id,unsigned long long from,unsigned long long to,
unsigned int flags,void *ctx);

int try_prepare_hs(const char **expressions, int npattern, struct dict *dict) {
    hs_compile_error_t *hs_cerr;
    hs_database_t *hsdb;
    hs_scratch_t  *hs_scratch;
    
    if (hs_compile_multi(expressions, 0, 0, npattern, HS_MODE_BLOCK, NULL, &hsdb, &hs_cerr) != HS_SUCCESS) {
        hs_free_compile_error(hs_cerr);
        return -1;
    }

    // we only need one hs_scratch in one program. FIXME: remove it ...
    if (hs_alloc_scratch(hsdb, &hs_scratch) != HS_SUCCESS) {
        hs_free_database(hsdb);
        return -1;
    }
    dict->hdb->db = hsdb;
    dict->hdb->scratch = hs_scratch;
    dict->hdb->ishs = 1;
    return 0;
}

int try_prepare_ch(const char **expressions, int npattern, struct dict *dict) {
    ch_compile_error_t *ch_cerr;
    ch_database_t      *chdb;
    ch_scratch_t       *ch_scratch;
    
    if (ch_compile_multi(expressions, 0, 0, npattern, HS_MODE_BLOCK, NULL, &chdb, &ch_cerr) != HS_SUCCESS) {
        ch_free_compile_error(ch_cerr);
        return -1;
    }
    if (ch_alloc_scratch(chdb, &ch_scratch) != HS_SUCCESS) {
        ch_free_database(chdb);
        return -1;
    }
    dict->hdb->db = chdb;
    dict->hdb->scratch = ch_scratch;
    dict->hdb->ishs = 1;
    return 0;
}


int main(int argc, char const *argv[]){
    int nrequest, i, npattern, j;
    struct dict * dict = read_dict((char *)"./pattern.txt");
    struct request *r = read_requests((char *)"./requests.txt");


    const char *expressions[1024];

    npattern =  dict->patterns->nelts;
    struct pattern *nds = (struct pattern *)dict->patterns->elts; //nd stand for needles

    for (i = 0; i < dict->patterns->nelts; i++) {
        expressions[i] = nds->str.data;
    }

    if (try_prepare_hs(expressions, npattern, dict) == -1) {
        printf("prepare hs error\n");
    }else if (try_prepare_ch(expressions, npattern, dict) == -1) {
        printf("prepare ch error exit\n");
        return -1;
    }
    

    return 0;
}


int 
event_handler(unsigned id,unsigned long long from,unsigned long long to,
unsigned int flags,void *ctx){
    printf("id %d from %llu :Match for pattern \"%s\" at offset %llu\n", id,from,(char *)ctx, to);
    return 0;

}