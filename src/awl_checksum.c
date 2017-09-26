#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <awl_checksum.h>

static unsigned short inline checksum(const unsigned short *buffer,int size) {

	unsigned long cksum = 0;
	
	while(size>1) {
		cksum += *buffer++;
		size  -= sizeof(unsigned short);
	}
	if(size) {
		cksum += *(unsigned char *)buffer;
	}
	
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);		
	return((unsigned short )(~cksum));
}

unsigned int inline tcp_checksum(unsigned long saddr, unsigned long daddr, const unsigned short *buffer, int size) {
	unsigned long sum = 0;
	char *buf = (char *)malloc(sizeof(struct pseudo_header)+size);
	struct pseudo_header *tph;
	tph = (struct pseudo_header *)buf;
	
	tph->saddr = saddr;
	tph->daddr = daddr;
	tph->mbz = 0;
	tph->proto = IPPROTO_TCP;
	tph->header_len = htons(sizeof(struct tcphdr));
	memcpy(buf+sizeof(struct pseudo_header),buffer,size);

	sum = checksum((unsigned short*)buf,sizeof(struct pseudo_header)+size);
	free(buf);
	return sum;
}

unsigned int inline udp_checksum(unsigned long saddr, unsigned long daddr, const unsigned short *buffer, int size) {
	unsigned long sum = 0;
        char *buf = (char *)malloc(sizeof(struct pseudo_header)+size);
        struct pseudo_header *tph;
        tph = (struct pseudo_header *)buf;

        tph->saddr = saddr;
        tph->daddr = daddr;
        tph->mbz = 0;
        tph->proto = IPPROTO_UDP;
        tph->header_len = htons(sizeof(struct udphdr));
        memcpy(buf+sizeof(struct pseudo_header),buffer,size);

        sum = checksum((unsigned short*)buf,sizeof(struct pseudo_header)+size);
        free(buf);
        return sum;
}


unsigned int inline ip_checksum(const unsigned short *buffer, int size){
	return(checksum(buffer, size));
}
