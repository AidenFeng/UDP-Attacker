//awl_pool.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <linux/sockios.h>

#include <awl_pool.h>
#include <awl_rand.h>
#include <awl_checksum.h>
#include <awl_sig.h>

extern unsigned int sendtime;
extern int quit_id;
extern unsigned char buf_len;
extern unsigned char randbits;
extern unsigned char flag;
/* init POOL_MAX's packages for send */
int pool_init(char *head[],struct awl_data *to, struct awl_data *sr){
	int i = 0;
	struct ether_header *eth;
	struct iphdr *iph;
	struct udphdr *udph;
	int eth_len = sizeof(struct ether_header);
	int iph_len = sizeof(struct iphdr),udph_len = sizeof(struct udphdr);
	buf_len = eth_len+iph_len+udph_len;
		
	while(i<POOL_MAX){
		head[i] = (char *)malloc(buf_len);
		if(head[i] == NULL){
			return(0);
		}
		eth = (struct ether_header *)head[i];
		iph = (struct iphdr *)(head[i]+eth_len);
		udph = (struct udphdr *)(head[i]+eth_len+iph_len);
		
		/* copy dest mac */	
		memcpy(eth->ether_dhost,to->eth_dst,ETH_ALEN);	
		if(flag&0x04) {
			memcpy(eth->ether_shost, sr->eth_dst, ETH_ALEN);
		}
		else {
			/* write rand source mac */
			eth->ether_shost[0] = get_rand8();
			eth->ether_shost[1] = get_rand8();
			eth->ether_shost[2] = get_rand8();
			eth->ether_shost[3] = get_rand8();
			eth->ether_shost[4] = get_rand8();
			eth->ether_shost[5] = get_rand8();	
		}
		eth->ether_type = htons(ETHERTYPE_IP);
		
		iph->ihl=5;
    		iph->version=4;
    		iph->tos=0;
    		iph->tot_len=htons(iph_len+udph_len);
		iph->id=htons(get_rand16());
    		iph->frag_off=0;
    		iph->ttl=get_rand8();
    		iph->protocol=IPPROTO_UDP;
    		iph->check=0;
		if(flag&0x08) {
			unsigned int tmpip=sr->d_addr;
			unsigned char *tmpptr=(unsigned char *)&tmpip;
			unsigned char i=0x01, index=3;
			while(i<=0x08) {
				if(randbits&i) {
					tmpptr[index]=get_rand8();
				}
				i <<= 1;
				--index;
			}
			iph->saddr=tmpip;
		}
		else {
    			iph->saddr=get_rand32();
		}
		iph->daddr=to->d_addr;
	
		if(flag&0x10) {
			udph->source = sr->d_port;
		}
		else {
			udph->source = get_rand16();
		}
		udph->dest = to->d_port;
		udph->len = htons(sizeof(struct udphdr));
		udph->check = 0;
		udph->check = udp_checksum(iph->saddr,iph->daddr, (unsigned short *)udph, udph_len);
		iph->check  = ip_checksum((unsigned short *)iph,iph->ihl*4);
		i++;
	}
	
	return(1);
}

int pool_clean(char *head[]){
	int i=0;
	while(i<POOL_MAX){
		free(head[i]);
		i++;
	}	
	return(1);
}

int pool_show(char *head[]){
	int i = 0;
	struct iphdr *iph;
	struct in_addr in;
	while(i<POOL_MAX){
		iph =(struct iphdr*)head[i];
		in.s_addr = iph->saddr;
		printf("\nNo.%d,",i);
		printf("src address:%s",inet_ntoa(in));
						
		i++;
	}
	
	return(1);
}

int pool_renew(char *head[]){
	int i = 0;
	struct ether_header *eth;
	struct iphdr *iph;
	struct udphdr *udph;
	int eth_len = sizeof(struct ether_header);
	int iph_len = sizeof(struct iphdr),udph_len = sizeof(struct udphdr);
	printf("renew the pool begin!\n");
	if(flag&0x20) {
		sleep(6000*sendtime);
        }

	while(1) {
		eth = (struct ether_header *)head[i];
		iph = (struct iphdr *)(head[i]+eth_len);
		udph = (struct udphdr *)(head[i]+eth_len+iph_len);

		if(~flag&0x04) {
			eth->ether_shost[0] = get_rand8();
			eth->ether_shost[1] = get_rand8();
			eth->ether_shost[2] = get_rand8();
			eth->ether_shost[3] = get_rand8();
			eth->ether_shost[4] = get_rand8();
			eth->ether_shost[5] = get_rand8();
		}
		iph->id=htons(get_rand16());
    		iph->ttl=get_rand8();
    		iph->check=0;
    		if(~flag&0x08) {
			iph->saddr=get_rand32();
    		}
		
		if(~flag&0x10) {
			udph->source = get_rand16();
		}
		udph->check = 0;
		
		udph->check = udp_checksum(iph->saddr,iph->daddr,(unsigned short *)udph,udph_len);
		iph->check  = ip_checksum((unsigned short *)iph,iph->ihl*4);
		if(quit_id == 1){
			printf("pool_renew success\n");
			break;
		}
		
		if(++i >= POOL_MAX){
			i = 0;
			sleep(2);
			printf("turn\n");
			if(flag&0x20) {
        	                sleep(6000*sendtime);
	                }
		}

	//	sleep(60);
	}
		return(1);
}

void getmac(unsigned char* dest)
{  
	char *device="eth0"; //eth0是网卡设备名
	struct ifreq req;
	
	int s=socket(AF_INET,SOCK_DGRAM,0); //internet协议族的数据报类型套接口
	strcpy(req.ifr_name,device); //将设备名作为输入参数传入
	int err=ioctl(s,SIOCGIFHWADDR,&req); //执行取MAC地址操作
	close(s); 
	
	if(err != -1)
	{  
		memcpy(dest,req.ifr_hwaddr.sa_data,ETH_ALEN); //取输出的MAC地址
	}
}

void getip(unsigned int *dest)
{
}









