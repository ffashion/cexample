#include <hs/hs.h>
#include <string.h>
#include <stdio.h>
int 
event_handler(unsigned id,unsigned long long from,unsigned long long to,
unsigned int flags,void *ctx);

int main(int argc, char const *argv[]){
    hs_database_t *database = NULL;
    hs_compile_error_t  *compile_err = NULL;
    hs_scratch_t *scratch = NULL;

    const char *pattern = ".*";
    const char *data = "hello world";
    size_t len = strlen(data);
    int flags;

    //只支持单正则
    //编译正则表达式 用于生成database
    //mode确定了生成database的格式，主要有BLOCK，STREAM和VECTOR三种 每一种模式的database只能由相应的scan接口使用
    //platform用来指定此database的目标平台 为NULL表示与当前平台一致
    //HS_FLAG_ALLOWEMPTY 支持使用.*
    if( hs_compile(pattern,HS_FLAG_ALLOWEMPTY,HS_MODE_BLOCK,NULL,&database,&compile_err) != HS_SUCCESS){
        hs_free_compile_error(compile_err);
    }
   

    // hs_compile_multi(pattern,HS_FLAG_MULTILINE,NULL,0,HS_MODE_BLOCK,NULL,&database,&compile_err);
    
    //临时数据 scratch
    if (hs_alloc_scratch(database,&scratch) != HS_SUCCESS) {
        hs_free_database(database);
    }
    


    // hs_compile_multi()
    //flags 目前未使用
    if (hs_scan(database,data,len,0,scratch,event_handler,(void *)pattern) != HS_SUCCESS) {
        hs_free_database(database);
    }
    



    hs_free_scratch(scratch);
    hs_free_database(database);

    return 0;
}
int 
event_handler(unsigned id,unsigned long long from,unsigned long long to,
unsigned int flags,void *ctx){
    printf("id %d from %llu :Match for pattern \"%s\" at offset %llu\n", id,from,(char *)ctx, to);
    return 0;

}