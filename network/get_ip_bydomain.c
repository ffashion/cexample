#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
int get_ips_by_domain(const char * const domain){
    struct hostent *_hostent = NULL;
    _hostent =  gethostbyname(domain);
    
    for(int i = 0;_hostent->h_addr_list[i]; i++){
        char *ipaddr = inet_ntoa(*((struct in_addr *)_hostent->h_addr_list[i]));
        printf("ip addr%d: %s\n",i,ipaddr);
    }
    // int i = 0;
    // while(_hostent->h_addr_list[i] != NULL){
    //     char *ipaddr = inet_ntoa(*((struct in_addr *)_hostent->h_addr_list[i]));
    //     printf("ip addr%d: %s\n",i,ipaddr);
    //     i++;
    // }
    return 0;
}

int main(int argc, char const *argv[])
{
    get_ips_by_domain("baidu.com");
    for (;;) {
        // sleep(10);
    }
    return 0;
}