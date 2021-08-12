#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
extern int optind; //opt index 保存argv 下一个未处理的索引
extern int opterr; //是否开启错误显示，默认为1
extern int optopt; //错误选项字符

extern char *optarg;//如果选项带有参数 则把全局遍历optarg设为指向这个参数

int main(int argc, char *const argv[]){
    int opt;
    //getopt的返回值为选项的字符
    while((opt = getopt(argc,argv,"p:x:")) != -1){
        printf("optind =%d opterr = %d optopt = %d\n",optind,opterr,optopt);
        switch (opt){
        case 'p':
            printf("%s\n",optarg);
            break;
        case 'x':
            printf("%s\n",optarg);
            break;
        default:
            break;
        }
    }
    // getopt_long()


    return 0;
}
