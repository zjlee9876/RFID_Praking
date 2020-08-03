#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "key.h"
#include "lcd.h"
#include "rfid.h"
#include "parkingnum.h"
#include "touch.h"
#include "beep.h"

extern void *play_music(void *arg);
extern Park park[CARDNUM];
extern Rfid_card rfidcard[CARDNUM];
extern int timecounter;
extern int LOOP_NUM1;
extern int LOOP_NUM2;
extern int LOOP_NUM3;
extern int LOOP_NUM4;

static int time_10s_falg = 0;

static void* timer(void *arg)//超时
{
	pthread_detach(pthread_self());
	int *time = (int*)arg;
	while((*time)--) sleep(1);
	time_10s_falg = 1;
	
}

static void* read_card(void *arg)//读卡
{
	int *cardid = (int*)arg;
	pthread_detach(pthread_self());
	*cardid = get_rfid();
}
static void *thread_getxy(void *arg)
{
	pthread_detach(pthread_self());
	struct ts_xy *p = (struct ts_xy*)arg;
	while(1) *p = ts_get_xy(); 
}
char *read_key(void)
{
	int fd;
	static char key_flag[4];
	int ret,i;
	fd = open("/dev/key_drv", O_RDWR);
	if(fd == -1){
		perror("open key_drv");
		return NULL;
	}
	ret = read(fd, key_flag, 4);
	if(ret == -1){
		perror("read key_drv");
		return NULL;
	}
	close(fd);
	return key_flag;	
}

void exit_car(Parknum parknum)
{
	printf("exitcar\n");
	pthread_t pt1,pt2,pt3,pt4,pt_beep;
	int *p_menu_nt = NULL;
	int card_value;
	int ret;
	int count = 0;
	int time = 10;
	int cardid = 0;
	int overage_temp;
	int beep_true = 1;
	int beep_fault = 2;
	int errornum = 0;
	char str[30];
	char str1[23];
	char pswd[7];
	struct ts_xy ts_data,ts_data2;
	ret = pthread_create(&pt1,NULL,timer,(void*)&time);//检测是否超时线程
	if(ret != 0){							
		printf("超时建立线程失败\n");
		return;							
	}
	ret = pthread_create(&pt2,NULL,read_card,(void*)&cardid);//检测是否刷卡线程
	if(ret != 0){							
		printf("读卡建立线程失败\n");
		return;							
	}
	ret = pthread_create(&pt3,NULL,thread_getxy,(void*)&ts_data);//触摸
	if(ret != 0){							
		printf("读卡建立线程失败\n");
		return;							
	}
	usleep(2);
	while(1){
	/*****************************************************************************************************/
		if(park[parknum-1].parkingnum||park[parknum-1].reserve){
			for(int i = 1;i < 5;i++){
				sprintf(str,"%s%d%s","picture/readcard",i,"_360_280.bmp");
				if(++count==1)  p_menu_nt = lcd_show_bmp(str,1,223,102,360,280);
				lcd_show_bmp(str,1,223,102,360,280);
				if(time_10s_falg){
					time_10s_falg = 0;	
					pthread_cancel(pt2);
					pthread_cancel(pt3);
					lcd_show_bmp("picture/readcard_timeout_360_280.bmp",0,223,102,360,280);
					sleep(3);
					lcd_recover(p_menu_nt,223,102,360,280);
					return;
				}
				if(cardid){
					if(cardid != park[parknum-1].id){
						pthread_cancel(pt1);
						pthread_cancel(pt3);
						lcd_show_bmp("picture/readcard_error_360_280.bmp",0,223,102,360,280);
						ret = pthread_create(&pt_beep,NULL,beep,(void*)&beep_fault);
						if(ret != 0){							
							printf("超时建立线程失败\n");
							return;							
						}
						sleep(3);
						lcd_recover(p_menu_nt,223,102,360,280);
						return;
					}
					/*else{
						ret = pthread_create(&pt_beep,NULL,beep,(void*)&beep_true);
						if(ret != 0){							
							printf("超时建立线程失败\n");
							return;							
						}
					}*/
					pthread_cancel(pt1);
					pthread_cancel(pt3);
					if(cardid == rfidcard[yueqian].id) card_value = 0; 
					if(cardid == rfidcard[school].id)  card_value = 1; 
					if(cardid == rfidcard[rural].id)  card_value = 2; 
					if(cardid == rfidcard[abc].id)  card_value = 3; 
					overage_temp = rfidcard[card_value].overage - rfidcard[card_value].usetime;//扣费
					if(overage_temp<0){//扣费后预约小于0
						lcd_show_bmp("picture/overage_error_360_280.bmp",0,223,102,360,280);
						ret = pthread_create(&pt_beep,NULL,beep,(void*)&beep_fault);
						if(ret != 0){							
							printf("超时建立线程失败\n");
							return;							
						}
						sleep(3);
						lcd_recover(p_menu_nt,223,102,360,280);
						return;
					}
					else{
						ret = pthread_create(&pt_beep,NULL,beep,(void*)&beep_true);
						if(ret != 0){							
							printf("超时建立线程失败\n");
							return;							
						}
					}
					/*while(1){
						ret = input_password(pswd);
						if(ret == 0){//取消输入
							lcd_recover(p_menu_nt,223,102,360,280);
							return;
						}
						if(strcmp(pswd,rfidcard[card_value].password) != 0){//密码错误
							if(errornum == 0){
								lcd_show_bmp("picture/password_errorlast_360_280.bmp",0,223,102,360,280);
								sleep(3);
								lcd_recover(p_menu_nt,223,102,360,280);
								return;
							}
							lcd_show_bmp("picture/password_error_360_280.bmp",0,223,102,360,280);
							sprintf(str1,"%s%d%s","picture/num",errornum,"_24_36.bmp");
							lcd_show_bmp(str1,0,442,250,24,36);
							while(1){
								ts_data2 = ts_get_xy();
								if(ts_data2.x>324&&ts_data2.x<378&&ts_data2.y>295&&ts_data2.y<345){
									lcd_show_bmp("picture/yes_64_60.bmp",0,319,290,64,60);
									usleep(1000*100);
									break;
								}
								if(ts_data2.x>431&&ts_data2.x<485&&ts_data2.y>295&&ts_data2.y<345){
									lcd_show_bmp("picture/cancel_64_60.bmp",0,426,290,64,60);
									usleep(1000*100);
									lcd_recover(p_menu_nt,223,102,360,280);
									return;
								}
							}
						}
						else{//密码正确
							break;
						}
						errornum--;				
					}*/
					rfidcard[card_value].overage -= rfidcard[card_value].usetime;
					rfidcard[card_value].stopflag = 0;
					rfidcard[card_value].reserveflag = 0;
					rfidcard[card_value].usetime = 0;
					
					if(parknum==num1) LOOP_NUM1 = 0;
					if(parknum==num2) LOOP_NUM2 = 0;
					if(parknum==num3) LOOP_NUM3 = 0;
					if(parknum==num4) LOOP_NUM4 = 0;
					usleep(5);
					//车位信息全部清楚
					/*ret = pthread_create(&pt4, NULL,play_music,(void*)"use_again.mp3");
					if(ret != 0){
						printf("建立线程失败\n");
						return;
					}*/
					park[parknum-1].id = 0;
					park[parknum-1].parkingnum = 0;
					park[parknum-1].reserve = 0;
					lcd_show_bmp("picture/onceagain_360_280.bmp",0,223,102,360,280);
					sleep(3);
					lcd_recover(p_menu_nt,223,102,360,280);
					switch(parknum){
						case num1:
							lcd_show_bmp("picture/stopvoid_164_88.bmp",0,368,74,164,88);
							break;
						
						case num2:
							lcd_show_bmp("picture/stopvoid_164_88.bmp",0,552,74,164,88);
							break;
						
						case num3:
							lcd_show_bmp("picture/stopvoid_164_88.bmp",0,368,177,164,88);
							break;
						
						case num4:
							lcd_show_bmp("picture/stopvoid_164_88.bmp",0,552,177,164,88);
							break;
				    }
					/******************************调试**********************************/
					printf("yueqian id:%x\n",rfidcard[yueqian].id);
					printf("school id:%x\n",rfidcard[school].id);
					printf("rural id:%x\n",rfidcard[rural].id);
					printf("abc id:%x\n",rfidcard[abc].id);
					printf("yueqians overage:%d\n",rfidcard[yueqian].overage);
					printf("school overage:%d\n",rfidcard[school].overage);
					printf("rural overage:%d\n",rfidcard[rural].overage);
					printf("abc overage:%d\n",rfidcard[abc].overage);
					/******************************调试**********************************/
					return;
				}
				if(ts_data.x>369&&ts_data.x<423&&ts_data.y>287&&ts_data.y<337){//取消
					pthread_cancel(pt1);
					pthread_cancel(pt2);
					pthread_cancel(pt3);
					ts_data.x = 0;
					ts_data.y = 0;
					lcd_show_bmp("picture/cancel_64_60.bmp",0,364,282,64,60);
					usleep(1000*100);
					lcd_recover(p_menu_nt,223,102,360,280);
					return;
				}
				usleep(1000*500);
			}
		}
	/*****************************************************************************************************/
		else{//如果没有车位没有车
			pthread_cancel(pt1);
			pthread_cancel(pt2);
			pthread_cancel(pt3);
			p_menu_nt = lcd_show_bmp("picture/exit_error_360_280.bmp",1,223,102,360,280);
			sprintf(str1,"%s%d%s","picture/num",parknum,"_24_36.bmp");
			lcd_show_bmp(str1,0,283,287,24,36);
			ret = pthread_create(&pt_beep,NULL,beep,(void*)&beep_fault);
			if(ret != 0){							
				printf("超时建立线程失败\n");
				return;							
			}
			sleep(3);
			lcd_recover(p_menu_nt,223,102,360,280);
			return;
		}
	}
}
void key_ctrl_interface(void)
{
	char *p = NULL;
	while(1){
		p = read_key();
		if(p[3] == 0) //K4
			exit_car(num1);
		
		if(p[2] == 0) //K3
			exit_car(num2);
		
		if(p[0] == 0) //K2
			exit_car(num3);
		
		if(p[1] == 0) //K6
			exit_car(num4);	
	}
}
