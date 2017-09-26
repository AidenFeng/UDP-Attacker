//main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#include <linux/if_ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#include <awl_rand.h>
#include <awl_checksum.h>
#include <awl_pool.h>
#include <awl_pth.h>
#include <awl_sig.h>
//#include <awl_getinfo.h>

int s;
int quit_id = 0;		// quit sig
char *head[POOL_MAX];		// pool make some packages
int toaddr_len = sizeof(struct sockaddr);
unsigned char buf_len=0;			// the packages size
struct sockaddr to_info;		// interface send packages
unsigned char flag=0x00;	// rand flag
unsigned int sendtime=0;
unsigned char randbits=0x00;
unsigned int pth_num=6;

int main(int argc, char *argv[])
{
	int ret,opt;
	void * pth_message;
	struct awl_data  to, sr;	// the mac and ip-port about dest
	
	signal(SIGINT,sig_process);  // ctrl+c:goto sig_process
		
	bzero(&to_info,sizeof(to_info));
	to_info.sa_family = AF_INET;
	strcpy(to_info.sa_data,"eth0");	// is a deafult ip-port-others
	
	/* set destination mac */
	bzero(&to,sizeof(to));
	to.eth_dst[0] = 0xff;
	to.eth_dst[1] = 0xff;
	to.eth_dst[2] = 0xff;
	to.eth_dst[3] = 0xff;
	to.eth_dst[4] = 0xff;
	to.eth_dst[5] = 0xff;
	
	/* get the rand opt */
	while((opt = getopt(argc,argv,"w:m:M:i:I:p:P:t:n:s:")) != -1){
		switch(opt){
			case 's':
				pth_num = atoi(optarg);
				break;
			case 'w':
				strcpy(to_info.sa_data,optarg);
				break;
			case 'm':
				to.eth_dst[0] = strtol(strtok(optarg,":"),NULL,16);
				to.eth_dst[1] = strtol(strtok(NULL,":"),NULL,16);
				to.eth_dst[2] = strtol(strtok(NULL,":"),NULL,16);
				to.eth_dst[3] = strtol(strtok(NULL,":"),NULL,16);
				to.eth_dst[4] = strtol(strtok(NULL,":"),NULL,16);
				to.eth_dst[5] = strtol(strtok(NULL,":"),NULL,16);
				break;
			case 'M':
				flag |= 0x04;
				if(strcmp(optarg, "host")==0)
				{
					getmac(sr.eth_dst);
					// memcpy(sr.eth_dst,getmac(),ETH_ALEN);
				}
				else
				{
					sr.eth_dst[0] = strtol(strtok(optarg,":"),NULL,16);
					sr.eth_dst[1] = strtol(strtok(NULL,":"),NULL,16);
					sr.eth_dst[2] = strtol(strtok(NULL,":"),NULL,16);
					sr.eth_dst[3] = strtol(strtok(NULL,":"),NULL,16);
					sr.eth_dst[4] = strtol(strtok(NULL,":"),NULL,16);
					sr.eth_dst[5] = strtol(strtok(NULL,":"),NULL,16);
			 	}
				break;		
			case 'i':
				flag |= 0x01;
				to.d_addr = inet_addr(optarg);
				if((to.d_addr <=0)) {
					printf("Format Error\n");
					exit(1);
				}
				break;	
			case 'I':
				flag |= 0x08;
				if(strcmp(optarg, "host")==0)
				{
					getip(&sr.d_addr);
					//memcpy(sr.d_addr,getip(),4);
				}
				else
				{
					sr.d_addr = inet_addr(optarg);
					if((sr.d_addr <=0)) {
						printf("Format Error\n");
						exit(1);
					}
				}
				break;
			case 'p':
				flag |= 0x02; 
				to.d_port = htons(atoi(optarg));
				if(to.d_port <=0) {
					printf("Format Error\n");
					exit(1);
				}
				break;;
			case 'P':
				flag |= 0x10;
				sr.d_port = htons(atoi(optarg));
				if(sr.d_port <=0) {
					printf("Format Error\n");
					exit(1);
				}
				break;
			case 't':
				flag |= 0x20;
				sendtime = atoi(optarg);
				break;
			case 'n':
				flag |= 0x40;
				randbits = atoi(optarg);
				break;
			case '?':
				printf("Format Error\n");
				exit(1);
			break;
		}
	} // end -while
	/* check the flag */
	pthread_t pth[pth_num];	// pth list
	if((flag&0x03)!=0x03)
	{
		printf("------------------\nyou must input the ip and port of destination!exit\n");
		exit(1);
	}

	rand_init();
		
	if((s=socket(AF_INET,SOCK_PACKET,ETH_P_IP))<0){
		perror("Socket:");
		exit(1);
	}
{	/* malloc a foo to be the args to allow broadcast*/
	int foo = 1;
	if((setsockopt(s,SOL_SOCKET,SO_BROADCAST,(void *)&foo,sizeof(foo)))<0){
		perror("setsockopt");
		exit(1);
	}
}	

	/* init POOL_MAX's packages for send */
	pool_init(head,&to,&sr);

	/* create pth_num's thread to send packages */	
	int i;
	for(i=0; i<pth_num; ++i) {
		ret = pthread_create(&pth[i],NULL,awl_send,NULL);
		// create error
		if(ret != 0){
			perror("pthread_create");
extern unsigned int pth_num;
			exit(1);
		}
	}

	/* wait some time renew the packages 
 	 * change some rand args  */
		pool_renew(head);
		sleep(2);
	/* wait pthread exit and free the resource */
	for(i=0;i<pth_num;i++){
		ret = pthread_join(pth[i],&pth_message);
		if(ret !=0){
			perror("pthread_join Error\n");
			exit(1);
		}
		printf("pthread join %d is %s !\n",i,(char *)pth_message);
	}
	
	/* free the malloc's space */
	pool_clean(head);
	
	/* free the socket */
	close(s);
	return (0);
}
