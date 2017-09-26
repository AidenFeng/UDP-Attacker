#ifndef _AWL_CHECKSUM_H
#define _AWL_CHECKSUM_H

struct pseudo_header {
	unsigned long saddr;
	unsigned long daddr;
	unsigned char mbz;
	unsigned char proto;
	unsigned short header_len;
};
	
unsigned int inline tcp_checksum(unsigned long saddr, unsigned long daddr, const unsigned short *buffer, int size);
unsigned int inline ip_checksum(const unsigned short *buffer, int size);
unsigned int inline udp_checksum(unsigned long saddr, unsigned long daddr, const unsigned short *buffer, int size);
#endif /* _AWL_CHECKSUM_H */
