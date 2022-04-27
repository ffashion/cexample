
#include <pcap/pcap.h>
/* base */
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
/* netinet*/
#include <netinet/in.h>
#include <netinet/ip_icmp.h>	
#include <netinet/udp.h>	
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netdb.h>
/* linux*/
#include <linux/if_packet.h>
#include <linux/if_tun.h>
#include <linux/if_ether.h>
#include <linux/if.h>

/*sys*/
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>


#define ETH_HDRLEN 14  // Ethernet header length
#define IP4_HDRLEN 20  // IPv4 header length
#define ICMP_HDRLEN 8  // ICMP header length for echo request, excludes data


void read_packet(u_char *user, const struct pcap_pkthdr *header, const u_char *packet) {
    struct ethhdr *ethhdr;
    struct ip *iph;
    struct icmp *icmphdr;

    ethhdr = (struct ethhdr *)packet;
    iph = (struct ip *) (packet + ETH_HDRLEN);
    icmphdr = (struct icmp *) (packet + ETH_HDRLEN + IP4_HDRLEN);
    

    if (ethhdr->h_proto == htons(ETH_P_IP)) {
        switch (iph->ip_p) {
            case IPPROTO_ICMP:
                printf("icmp: caplen %d, \n", header->caplen);
                break;
            case IPPROTO_IGMP:
                printf("igmp: caplen %d, \n", header->caplen);
                break;
            case IPPROTO_TCP:
                printf("tcp:, caplen %d,\n", header->caplen);
                break;
            case IPPROTO_UDP:
                printf("udp: caplen %d\n", header->caplen);
                break;
            default:
                break;
	    }
    }
}

int main(int argc, char *argv[])
{
	pcap_t *handle;			/* Session handle */
	char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
	struct bpf_program fp;		/* The compiled filter */
	char filter_exp[] = "port 23";	/* The filter expression */
	bpf_u_int32 mask;		/* Our netmask */
	bpf_u_int32 net;		/* Our IP */
	struct pcap_pkthdr header;	/* The header that pcap gives us */
	const u_char *packet;		/* The actual packet */
    pcap_if_t *devs, *device;
    char *ifname;
    int cnt;

    /* Init var */
    ifname = "ens3";
    cnt = 1;

	/* Define the device */
	if (pcap_findalldevs(&devs, errbuf) < 0) {
        printf("%s\n", errbuf);
        return -1;
    }
    device = devs;
    //find ens3
    while (strlen(ifname) != strlen(device->name)) {
        if (strcmp(ifname, device->name) == 0) {
            break;
        }
        device = devs->next;
    }
    
	/* Find the properties for the device */
	if (pcap_lookupnet(device->name, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Couldn't get netmask for device %s: %s\n", device->name, errbuf);
		net = 0;
		mask = 0;
	}
	/* Open the session in promiscuous mode */
	handle = pcap_open_live(device->name, BUFSIZ, 1, 1000, errbuf); //call pcap_create()
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", device->name, errbuf);
		return(2);
	}
	/* Grab a packet */
    for (;;) {
        if (pcap_loop(handle, cnt, read_packet, NULL) != 0) {
            break;
        }
    }
	/* And close the session */
	pcap_close(handle);
	return(0);
}