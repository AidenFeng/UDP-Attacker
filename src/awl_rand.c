//awl_rand.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

void rand_init(void){
	srand(time(0));
}

u_int8_t  get_rand8(void){
	return(rand() % 256);
}
u_int16_t get_rand16(void){
	return(rand() % 65536);
}
u_int32_t get_rand32(void){
	return(rand());
}
u_int32_t get_randn(u_int32_t n){
	return(rand() % n);
}
