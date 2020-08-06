#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "touch.h"
#include "lcd.h"
#include "rfid.h"
#include "recharge.h"
#include "beep.h"
#include "parkingnum.h"

static int time_10s_falg = 0;

extern Rfid_card rfidcard[CARDNUM];

static void* timer(void *arg)//超时
{
	
	pthread_detach(pthread_self());
	int *time = (int*)arg;
	while((*time)--) sleep(1);
	time_10s_falg = 1;
	
}
static void* read_card(void *arg)//读卡
{
	unsigned int *cardid = (unsigned int*)arg;
	pthread_detach(pthread_self());
	*cardid = get_rfid();
}

static void *thread_getxy(void *arg)
{
	pthread_detach(pthread_self());
	struct ts_xy *p = (struct ts_xy*)arg;
	while(1) *p = ts_get_xy(); 
}
void card_recharge(void)
{
	pthread_t pt1,pt2,pt4,pt_beep;
	int ret;
	int count1 = 0,count2 = 0,count3 = 0;
	int time = 10;
	int temp = 0;//找零值
	int value = 0;
	int beep_true = 1;
	int overage_temp;
	unsigned int cardid = 0;
	unsigned char card_value = 0xff;
	int *p_menu_nt = NULL;
	struct ts_xy ts_data,ts_data1,ts_data2;
	char str[30];
	char str1[23];
	char pswd[7];
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
	ret = pthread_create(&pt4,NULL,thread_getxy,(void*)&ts_data);//触摸
	if(ret != 0){							
		printf("读卡建立线程失败\n");
		return;							
	}
	usleep(2);
	lcd_show_bmp("picture/recharge_t_208_100.bmp",0,59,53,208,100);
	usleep(1000*100);
	lcd_show_bmp("picture/recharge_208_100.bmp",0,59,53,208,100);
	while(1){
	/*****************************************************************************************************/
		for(int i = 1;i < 5;i++){
			sprintf(str,"%s%d%s","picture/readcard",i,"_360_280.bmp");
			if(++count1==1)  p_menu_nt = lcd_show_bmp(str,1,223,102,360,280);
			lcd_show_bmp(str,1,223,102,360,280);
			if(time_10s_falg){//超时处理
				time_10s_falg = 0;	
				pthread_cancel(pt2);
				pthread_cancel(pt4);
				lcd_show_bmp("picture/readcard_timeout_360_280.bmp",0,223,102,360,280);
				sleep(2);
				lcd_recover(p_menu_nt,223,102,360,280);
				return;
			}
			if(cardid){//读卡成功处理
				ret = pthread_create(&pt_beep,NULL,beep,(void*)&beep_true);//检测是否超时线程
				if(ret != 0){							
					printf("超时建立线程失败\n");
					return;							
				}
				pthread_cancel(pt1);
				pthread_cancel(pt4);
				if(cardid== rfidcard[yueqian].id) card_value = 0; 
				if(cardid == rfidcard[school].id)  card_value = 1; 
				if(cardid == rfidcard[rural].id)  card_value = 2; 
				if(cardid == rfidcard[abc].id)  card_value = 3; 
				int errornum = 2;
				while(1){
					ret =  input_password(pswd);
					if(ret == 0){//取消输入
						lcd_recover(p_menu_nt,223,102,360,280);
						return;
					}
					if(strcmp(pswd,rfidcard[card_value].password) != 0){//密码错误
						if(errornum == 0){
							lcd_show_bmp("picture/password_errorlast_360_280.bmp",0,223,102,360,280);
							sleep(2);
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
					
				}
				while(1){
					lcd_show_bmp("picture/recharge_360_280.bmp",0,223,102,360,280);
					ts_data1 = ts_get_xy();
					if(ts_data1.x>247&&ts_data1.x<322&&ts_data1.y>168&&ts_data1.y<251){
						lcd_show_bmp("picture/recharge20_360_280.bmp",0,223,102,360,280);
						value = 20;
						usleep(1000*200);
						//break;
					}
					if(ts_data1.x>329&&ts_data1.x<401&&ts_data1.y>168&&ts_data1.y<251){
						lcd_show_bmp("picture/recharge50_360_280.bmp",0,223,102,360,280);
						value = 50;
						usleep(1000*200);
						//break;
					}
					if(ts_data1.x>406&&ts_data1.x<480&&ts_data1.y>168&&ts_data1.y<251){
						lcd_show_bmp("picture/recharge100_360_280.bmp",0,223,102,360,280);
						value = 100;
						usleep(1000*200);
						//break;
					}
					if(ts_data1.x>487&&ts_data1.x<562&&ts_data1.y>168&&ts_data1.y<251){
						lcd_show_bmp("picture/recharge200_360_280.bmp",0,223,102,360,280);
						value = 200;
						usleep(1000*200);
						//break;
					}
					if(ts_data1.x>369&&ts_data1.x<423&&ts_data1.y>287&&ts_data1.y<337){
						lcd_show_bmp("picture/cancel_64_60.bmp",0,364,282,64,60);
						usleep(1000*100);
						lcd_recover(p_menu_nt,223,102,360,280);
						return;
					}
					overage_temp = rfidcard[card_value].overage;
					rfidcard[card_value].overage += value;
					if(rfidcard[card_value].overage>999){
						temp = rfidcard[card_value].overage - 999;
						rfidcard[card_value].overage = 999;
				    }
					if(value){
						lcd_show_bmp("picture/readcard_sure_360_280.bmp",0,223,102,360,280);	
						int value_g = value % 10;
						int value_s = (value / 10) % 10;
						int value_b = value / 100;
						sprintf(str1,"%s%d%s","picture/num",value_g,"_24_36.bmp");
						lcd_show_bmp(str1,0,455,206,24,36);
						sprintf(str1,"%s%d%s","picture/num",value_s,"_24_36.bmp");
						lcd_show_bmp(str1,0,430,206,24,36);
						sprintf(str1,"%s%d%s","picture/num",value_b,"_24_36.bmp");
						lcd_show_bmp(str1,0,405,206,24,36);
						
						int temp_g = temp % 10;
						int temp_s = (temp / 10) % 10;
						int temp_b = temp / 100;
						sprintf(str1,"%s%d%s","picture/num",temp_g,"_24_36.bmp");
						lcd_show_bmp(str1,0,455,248,24,36);
						sprintf(str1,"%s%d%s","picture/num",temp_s,"_24_36.bmp");
						lcd_show_bmp(str1,0,430,248,24,36);
						sprintf(str1,"%s%d%s","picture/num",temp_b,"_24_36.bmp");
						lcd_show_bmp(str1,0,405,248,24,36);
						while(1){
							ts_data2 = ts_get_xy();
							if(ts_data2.x>324&&ts_data2.x<378&&ts_data2.y>295&&ts_data2.y<345){//确定
								lcd_show_bmp("picture/yes_64_60.bmp",0,319,290,64,60);
								usleep(1000*100);
								lcd_show_bmp("picture/recharge_success_360_280.bmp",0,223,102,360,280);
								sleep(2);
								lcd_recover(p_menu_nt,223,102,360,280);
								return;
							}
							
							if(ts_data2.x>431&&ts_data2.x<485&&ts_data2.y>295&&ts_data2.y<345){//取消
								count2 = 0;
								rfidcard[card_value].overage = overage_temp;
								lcd_show_bmp("picture/cancel_64_60.bmp",0,426,290,64,60);
								value = 0;
								usleep(1000*100);
								break;
							}
						}
					}
				}
			}
			if(ts_data.x>369&&ts_data.x<423&&ts_data.y>287&&ts_data.y<337){//读卡取消
				pthread_cancel(pt1);
				pthread_cancel(pt2);
				pthread_cancel(pt4);
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
}