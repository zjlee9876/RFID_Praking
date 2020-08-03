#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
	int fd;
	char led_flag[2]={1,3};//led3 on
	int ret;
	// /dev/led_drv---》设备文件。手动创建和自动创建
	fd = open("/dev/led_drv", O_RDWR);
	if(fd == -1)
	{
		perror("open led_drv");
		return -1;
	}
	while(1)
	{
		int i,j;
		for(i=1;i<5;i++)
		{
			led_flag[1] = i;
			for(j=0;j<2;j++)
			{
				led_flag[0]=j;
				ret = write(fd, led_flag, sizeof(led_flag));
				if(ret == -1)
				{
					perror("write led_drv");		
				}
				//sleep(1); //1s
				usleep(200*1000);//200ms
			}
		}
	}

	close(fd);
	
	return 0;	
}
