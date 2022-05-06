/* base */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

/* netinet*/
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
/* linux*/
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_tun.h>
#include <sys/mman.h>
/*sys*/
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
int page_size = 0;
struct jmp {
  uint32_t opcode : 8;
  int32_t offset : 32;
} __attribute__((packed));

#define JMP(off) ((struct jmp){0xe9, off - sizeof(struct jmp)})

static inline bool within_page(void *addr) {
    return (uintptr_t)addr % page_size + sizeof(struct jmp) <= page_size;
}

void DSU(void *old, void *new) {

    void *base = (void *)((uintptr_t)old & ~(page_size - 1));
    size_t len = page_size * (within_page(old) ? 1 : 2);
    int flags = PROT_WRITE | PROT_READ | PROT_EXEC;

    if (mprotect(base, len, flags) == 0) {
        *(struct jmp *)old = JMP((char *)new - (char *)old); // **PATCH**
        mprotect(base, len, flags & ~PROT_WRITE);
    } else {
        perror("DSU fail");
    }
}
void foo() { 
    printf("In %s();\n", __func__); 
}
void foo_new() { 
    printf("In updated %s();\n", __func__); 
}

int main() {
    page_size = (int)sysconf(_SC_PAGESIZE);
    foo();
    DSU(foo, foo_new);
    foo();
}