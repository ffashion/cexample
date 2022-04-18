// only mac
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <linux/if.h>
void hexToString(unsigned char *hex, char *str, int len, int output) {
  if (str == NULL) {
    str = malloc(2 * len) ;
  }
  for (size_t i = 0; i < len; i++) {
    sprintf(str + i * 2, "%02x", hex[i]);
  }
  if (output) {
    printf("%s\n", str);
  }
  free(str);
  return;
}

int tap_alloc(int flag) {
    struct ifreq ifr;
    int fd ;
    fd = open("/dev/net/tun", O_RDWR); //必须是这个位置
    if (fd < 0) {
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    ifr.ifr_flags = flag;
    strcpy(ifr.ifr_name, "tun0");

    if (ioctl(fd, TUNSETIFF, &ifr)  < 0) {
        close(fd);
        return -1;
    }

    return fd;
}
// step 1: sudo ifconfig tun0 192.168.1.2/24
// step 2: ping or curl the other ips, example 192.168.1.3

int	main(int argc, char **argv) {
    int fd, n;
    char buf[48];
    fd = tap_alloc(IFF_TUN | IFF_NO_PI);
    if(fd < 0) {
        perror("tap alloc");
        return -1;;
    }
    //set ip address 
    system("sudo ifconfig tun0 192.168.1.2/24");
    for (;;) {
        n = read(fd, buf, sizeof(buf));
        printf("read %d byte\n", n);
        hexToString((unsigned char *)buf, NULL, n, 1);
    }
    close(fd);
    return 0;
}
