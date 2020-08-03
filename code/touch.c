#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>

#include "touch.h"



struct ts_xy ts_get_xy(void)
{
	int fd_ts,i;
	struct ts_xy ts_temp;
	struct input_event ts_data;
	fd_ts = open("/dev/input/event0", O_RDONLY);
	if(fd_ts == -1){
		perror("open ts");
	}
	for(i=0;i<6;i++){	
		read(fd_ts, &ts_data, sizeof(struct input_event));
		
		if(ts_data.type == EV_ABS){
			if(ts_data.code == ABS_X)
				ts_temp.x = ts_data.value;
			else if(ts_data.code == ABS_Y)
				ts_temp.y = ts_data.value;		
		}
	}
	printf("x = %d,y = %d\n",ts_temp.x,ts_temp.y );
	close(fd_ts);
	return ts_temp;
}







