//awl_pool.h
#ifndef _AWL_POOL_H
#define _AWL_POOL_H

#include <sys/socket.h>
#include <linux/if_ether.h>
#define  POOL_MAX 65535

struct awl_data{
	uint8_t eth_dst[ETH_ALEN];
	uint32_t d_addr;
	uint16_t d_port;
};


int pool_init(char *head[],struct awl_data *to, struct awl_data *rc);
int pool_clean(char *head[]);
int pool_show(char *head[]);
int pool_renew(char *head[]);
void getmac(unsigned char* dest);
void getip(unsigned int* dest);

#endif /* _AWL_POOL_H */
