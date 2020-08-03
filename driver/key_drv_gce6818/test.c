#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
	int fd;
	char key_flag[4];
	int ret,i;
	fd = open("/dev/key_drv", O_RDWR);
	if(fd == -1)
	{
		perror("open key_drv");
		return -1;
	}
	while(1)
	{
		ret = read(fd, key_flag, 4);
		if(ret == -1)
		{
			perror("read key_drv");
		}
		for(i=0;i<4;i++)
			printf("key%d value = %d\n", i, key_flag[i]);
		usleep(100*1000);
	}

	close(fd);
	
	return 0;	
}
