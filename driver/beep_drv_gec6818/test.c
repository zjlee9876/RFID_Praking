#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int main(void)
{
	int fd_beep,ret,beep_data;
	fd_beep = open("/dev/beep_drv",O_RDWR);
	if(fd_beep == -1)
	{
		perror("open beep");
		return -1;
	}
	while(1)
	{
		beep_data = 1;
		ret = write(fd_beep,&beep_data,1);
		if(ret == -1)
		{
			perror("write beep_drv");	
			return -1;			
		}
		sleep(1);//1s
		beep_data = 0;
		ret = write(fd_beep,&beep_data,1);
		if(ret == -1)
		{
			perror("write beep_drv");	
			return -1;
		}
		sleep(5);//1s
	}
	close(fd_beep);
	return 0;
}