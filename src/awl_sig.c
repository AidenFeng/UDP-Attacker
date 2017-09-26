//awl_sig.c
#include <stdio.h>
#include <signal.h>
#include <awl_sig.h>

extern int quit_id;

void sig_process(int sig)
{
	quit_id = 1;
	signal(SIGINT,sig_process);	
	// resignal let quit_id alaways be zero to terminate the process
}
