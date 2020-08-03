#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lcd.h"
#include "touch.h"
#include "parkingnum.h"
#include "rfid.h"
#include "key.h"
#include "reserve.h"
#include "message.h"
#include "recharge.h"
extern Park park[CARDNUM];
extern Rfid_card rfidcard[CARDNUM]; 

static int start_gif(int *touch_flag);
static void *getxy(void *arg);
static void *key(void *arg);
void *play_music(void *arg);
static void *thread_getxy(void *arg);
int main(void)
{
	int ret;
	pthread_t pt1,pt2,pt3,pt4;
	int touch_flag = 0;
	int loop = 1;
	struct ts_xy ts_data;
	/*int fd;
	fd = open("/dev/ttySAC1", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0)
	{
		fprintf(stderr, "Open Gec210_ttySAC1 fail!\n");
		return -1;
	}
	init_tty(fd);*/
	for(int i = 0;i < 4;i++){
		park[i].parkingnum = 0;
		park[i].reserve = 0;
		park[i].id = 0;
	}
	ret = pthread_create(&pt2, NULL, key, (void*)(&loop));
	if(ret != 0){
		printf("建立线程失败\n");
		return -1;
	}
	ret = pthread_create(&pt1, NULL, getxy, (void*)(&touch_flag));
	if(ret != 0){
		printf("建立线程失败\n");
		return -1;
	}
	while(!(start_gif((int*)&touch_flag)));
	lcd_show_bmp("picture/menu_nt_800_480.bmp",0,0,0,800,480);
	ret = pthread_create(&pt3, NULL,play_music,(void*)"welcome_to_use.mp3");
	if(ret != 0){
		printf("建立线程失败\n");
		return -1;
	}
	lcd_show_bmp("picture/menu_nt_800_480.bmp",0,0,0,800,480);
	/*ret = pthread_create(&pt4, NULL,thread_getxy,(void*)&ts_data);
	if(ret != 0){
		printf("建立线程失败\n");
		return -1;
	}*/
	//lcd_show_bmp("picture/menu_nt_800_480.bmp",0,0,0,800,480);
	while(1){
		ts_data = ts_get_xy();
		if(ts_data.x>365&&ts_data.x<535&&ts_data.y>71&&ts_data.y<165)//选择车位1
			select_parkingnum(num1);
		if(ts_data.x>549&&ts_data.x<719&&ts_data.y>71&&ts_data.y<165)//选择车位2
			select_parkingnum(num2);
		if(ts_data.x>365&&ts_data.x<535&&ts_data.y>174&&ts_data.y<268)//选择车位3
			select_parkingnum(num3);
		if(ts_data.x>549&&ts_data.x<719&&ts_data.y>174&&ts_data.y<268)//选择车位4
			select_parkingnum(num4);
		if(ts_data.x>67&&ts_data.x<259&&ts_data.y>61&&ts_data.y<143)//用户充值
			card_recharge();
		if(ts_data.x>67&&ts_data.x<259&&ts_data.y>201&&ts_data.y<283)//余额查询
			show_message();
		if(ts_data.x>67&&ts_data.x<259&&ts_data.y>341&&ts_data.y<423)//提前预约
			park_reserve();			
	}
	return 0;
}

static void *thread_getxy(void *arg)
{
	pthread_detach(pthread_self());
	struct ts_xy *p = (struct ts_xy*)arg;
	while(1) *p = ts_get_xy(); 
}
static void *getxy(void *arg)
{
	int *p = (int*)arg;
	pthread_detach(pthread_self());
	struct ts_xy ts_data;
	ts_data = ts_get_xy();
	*p = 1; 
	
}
static void *key(void *arg)
{
	pthread_detach(pthread_self());
	while(*((int*)arg))
		key_ctrl_interface();

}
static int start_gif(int *touch_flag)
{
	lcd_show_bmp("picture/start1_800_480.bmp",0,0,0,800,480);
	if(*touch_flag) return 1;
	usleep(500*1000);
	lcd_show_bmp("picture/start2_800_480.bmp",0,0,0,800,480);
	if(*touch_flag) return 1;
	usleep(500*1000);
	lcd_show_bmp("picture/start3_800_480.bmp",0,0,0,800,480);
	if(*touch_flag) return 1;
	usleep(500*1000);
	lcd_show_bmp("picture/start4_800_480.bmp",0,0,0,800,480);
	if(*touch_flag) return 1;
	usleep(500*1000);
	
}
void *play_music(void *arg)
{
	pthread_detach(pthread_self());
	char str[50];
	sprintf(str,"%s%s","madplay mp3/",(char*)arg);
	printf("statr play\n");
	system(str);
	system("killall -9 madplay");
	printf("stop play\n");
}