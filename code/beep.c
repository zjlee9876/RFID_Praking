#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "beep.h"


int beep_ctrl(Beepstatus value)
{
	int fd_beep,ret;
	Beepstatus beep_data;
	beep_data = value;
	
	fd_beep = open("/dev/beep_drv",O_RDWR);
	if(fd_beep == -1)
	{
		perror("open beep");
		return -1;
	}
	
		
	ret = write(fd_beep,&beep_data,1);
	if(ret == -1)
	{
		perror("write beep_drv");	
		return -1;			
	}
	close(fd_beep);
	return 0;
}

void* beep(void *arg)
{
	pthread_detach(pthread_self());
	if(*(int*)arg == 1){
		beep_ctrl(ON);
		usleep(1000*100);
		beep_ctrl(OFF);
	}
	if(*(int*)arg == 2){
		beep_ctrl(ON);
		usleep(1000*100);
		beep_ctrl(OFF);
		usleep(1000*50);
		beep_ctrl(ON);
		usleep(1000*100);
		beep_ctrl(OFF);
		usleep(1000*50);
		beep_ctrl(ON);
		usleep(1000*100);
		beep_ctrl(OFF);
	}
}