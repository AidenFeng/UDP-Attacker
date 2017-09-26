//awl_pth.c
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/if_arp.h>

#include <awl_pth.h>
#include <awl_rand.h>
#include <awl_pool.h>

extern int s;			// socket
extern char *head[];		// pool
extern int toaddr_len;		// sockaddr
extern unsigned char buf_len;		// 0
extern int quit_id;		// 0
extern struct sockaddr to_info;	// sockaddr to
extern unsigned int sendtime;
extern unsigned char flag;
/* pthread_create in fun main */
void *awl_send(void *message){
	int i;
	while(1){
		i = get_randn(POOL_MAX);
		sendto(s,head[i],buf_len,0,&to_info,toaddr_len);
		// stop signal	
//		printf("sent success\n");
		if(quit_id == 1){
			goto over;
		}
		if(flag&0x20) {
			sleep(sendtime);
		}
	}
	over:
		pthread_exit("success");
}
