#include <libiptc/libiptc.h>
#include <iptables.h>
#include <stdio.h>
extern char *optarg;
int main(int argc, char const *argv[])
{   
    int c = 0;
    struct xtc_handle * handle = NULL;
    const char *chain = NULL;
    
    while( (c = getopt(argc,argv,"t:")) != -1) {
        switch(c) {
            case 't':
                if (!optarg) {
                    //参数错误
                    printf("error use-- ./get_chain -t mangle\n");
                    return -1;
                }  
            break;
        }
    }

    handle = iptc_init(optarg);
    if (!handle) {
        //名称错误
        printf("error use-- ./get_chain -t mangle\n");
        return -1;
    }
    for (chain = iptc_first_chain(handle); chain; chain = iptc_next_chain(handle))  {
        printf("%s\n", chain);
    }
    return 0;
}
