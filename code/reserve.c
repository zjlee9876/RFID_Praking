#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "touch.h"
#include "lcd.h"
#include "parkingnum.h"
#include "rfid.h"
#include "reserve.h"
#include "beep.h"

static int time_10s_falg = 0;
extern Rfid_card rfidcard[CARDNUM];
extern Park park[CARDNUM];
extern int LOOP_NUM1;
extern int LOOP_NUM2;
extern int LOOP_NUM3;
extern int LOOP_NUM4;

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
void park_reserve(void)
{
	pthread_t pt1,pt2,pt3,pt4,pt_beep;
	int ret;
	int count1 = 0,count2 = 0;
	int time = 10;
	int beep_true = 1;
	int beep_fault = 2;
	int reserve_errornum = 0;
	char pswd[7];
	unsigned int cardid = 0;
	unsigned char card_value = 0xff;
	unsigned char parkstatus = 0;
	int *p_menu_nt = NULL;
	struct ts_xy ts_data,ts_data1,ts_data2;
	char str[30];
	char str1[23];
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
	lcd_show_bmp("picture/reserve_t_208_100.bmp",0,59,333,208,100);
	usleep(1000*100);
	lcd_show_bmp("picture/reserve_208_100.bmp",0,59,333,208,100);
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
			if(cardid){
				pthread_cancel(pt1);
				pthread_cancel(pt4);
				if(cardid == rfidcard[yueqian].id) card_value = 0; 
				if(cardid == rfidcard[school].id)  card_value = 1; 
				if(cardid == rfidcard[rural].id)  card_value = 2; 
				if(cardid == rfidcard[abc].id)  card_value = 3; 
				
				if(rfidcard[card_value].stopflag){//该卡已入库
					reserve_errornum  = 1;
					lcd_show_bmp("picture/carexist_360_280.bmp",0,223,102,360,280);
				}
				if(rfidcard[card_value].reserveflag){//该卡已预约
					reserve_errornum  = 1;
					lcd_show_bmp("picture/carreserve_360_280.bmp",0,223,102,360,280);
				}
				if(reserve_errornum == 1){
					ret = pthread_create(&pt_beep,NULL,beep,(void*)&beep_fault);
				    if(ret != 0){							
						printf("超时建立线程失败\n");
						return;							
					}
					sleep(2);
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
					lcd_show_bmp("picture/reserve_360_280.bmp",0,223,102,360,280);
					if(park[0].parkingnum||park[0].reserve){//车位已经停车或者已经预约
						lcd_show_bmp("picture/reserve_error_100_48.bmp",0,289,169,100,48);//显示红×
						parkstatus |= (1<<0);
					}
					if(park[1].parkingnum||park[1].reserve){
						lcd_show_bmp("picture/reserve_error_100_48.bmp",0,411,169,100,48);
						parkstatus |= (1<<1);
					} 
					if(park[2].parkingnum||park[2].reserve){
						 lcd_show_bmp("picture/reserve_error_100_48.bmp",0,289,226,100,48);
						 parkstatus |= (1<<2);
					}
					if(park[3].parkingnum||park[3].reserve){
						 lcd_show_bmp("picture/reserve_error_100_48.bmp",0,411,226,100,48);
						 parkstatus |= (1<<3);
					}
					ts_data1 = ts_get_xy();
				/*****************************************************************************************************/
					if(ts_data1.x>288&&ts_data1.x<390&&ts_data1.y>168&&ts_data1.y<218){//点击车位1
						if(((parkstatus>>0)&0x01) == 1){//已经停车或者已经预约
							lcd_show_bmp("picture/reserve_error_t_100_48.bmp",0,289,169,100,48);
							usleep(1000*100);
							lcd_show_bmp("picture/reserve_error_100_48.bmp",0,289,169,100,48);
							int *p_menu_nt1 = lcd_show_bmp("picture/stopalready_360_280.bmp",1,223,102,360,280);
							sleep(2);
							lcd_recover(p_menu_nt1,223,102,360,280);
							continue;
						}
						else{
							int parknum = num1;
							lcd_show_bmp("picture/reserve_select_100_48.bmp",0,289,169,100,48);
							usleep(1000*200);
							lcd_show_bmp("picture/reserve_void_100_48.bmp",0,289,169,100,48);
							while(1){
								lcd_show_bmp("picture/reserve_sure_360_280.bmp",1,223,102,360,280);
								lcd_show_bmp("picture/num1_24_36.bmp",0,394,221,24,36);
								ts_data2 = ts_get_xy();
								if(ts_data2.x>324&&ts_data2.x<378&&ts_data2.y>295&&ts_data2.y<345){//确定
									lcd_show_bmp("picture/yes_64_60.bmp",0,319,290,64,60);
									usleep(1000*100);
									lcd_show_bmp("picture/reserve_success_360_280.bmp",0,223,102,360,280);
									rfidcard[card_value].reserveflag = 1;
									park[0].reserve = 1;
									park[0].id = cardid;
									park[0].parkingnum = 0;
									/**************************调试*******************************/
									printf("-----------------------------------------------------\n");
									printf("id = %x\n",rfidcard[card_value].id);
									printf("overage = %d\n",rfidcard[card_value].overage);
									printf("reserveflag = %d\n",rfidcard[card_value].reserveflag);
									printf("stopflag = %d\n",rfidcard[card_value].stopflag);
									printf("-----------------------------------------------------\n");
									/**************************调试*******************************/
									sleep(2);
									lcd_recover(p_menu_nt,223,102,360,280);
									lcd_show_bmp("picture/stopreserve_164_88.bmp",0,368,74,164,88);
									LOOP_NUM1 = 1;
									ret = pthread_create(&pt3,NULL,stop_toll,(void*)&parknum);//计时收费线程
									if(ret != 0){							
										printf("超时建立线程失败\n");
										return;							
									}
									
									return;
								}
								
								if(ts_data2.x>431&&ts_data2.x<485&&ts_data2.y>295&&ts_data2.y<345){//取消
									lcd_show_bmp("picture/cancel_64_60.bmp",0,426,290,64,60);
									usleep(1000*100);
									break;
								}
							}
						}
					}
				/*****************************************************************************************************/
					if(ts_data1.x>410&&ts_data1.x<512&&ts_data1.y>168&&ts_data1.y<218){//点击车位2
						if(((parkstatus>>1)&0x01) == 1){//已经停车或者已经预约
							lcd_show_bmp("picture/reserve_error_t_100_48.bmp",0,411,169,100,48);
							usleep(1000*100);
							lcd_show_bmp("picture/reserve_error_100_48.bmp",0,411,169,100,48);
							int *p_menu_nt1 = lcd_show_bmp("picture/stopalready_360_280.bmp",1,223,102,360,280);
							sleep(2);
							lcd_recover(p_menu_nt1,223,102,360,280);
							continue;
						}
						else{
							int parknum = num2;
							lcd_show_bmp("picture/reserve_select_100_48.bmp",0,411,169,100,48);
							usleep(1000*200);
							lcd_show_bmp("picture/reserve_void_100_48.bmp",0,411,169,100,48);
							while(1){
								lcd_show_bmp("picture/reserve_sure_360_280.bmp",1,223,102,360,280);
								lcd_show_bmp("picture/num2_24_36.bmp",0,394,221,24,36);
								ts_data2 = ts_get_xy();
								if(ts_data2.x>324&&ts_data2.x<378&&ts_data2.y>295&&ts_data2.y<345){//确定
									lcd_show_bmp("picture/yes_64_60.bmp",0,319,290,64,60);
									usleep(1000*100);
									lcd_show_bmp("picture/reserve_success_360_280.bmp",0,223,102,360,280);
									rfidcard[card_value].reserveflag = 2;
									park[1].reserve = 1;
									park[1].id = cardid;
									park[1].parkingnum = 0;
									/**************************调试*******************************/
									printf("-----------------------------------------------------\n");
									printf("id = %x\n",rfidcard[card_value].id);
									printf("overage = %d\n",rfidcard[card_value].overage);
									printf("reserveflag = %d\n",rfidcard[card_value].reserveflag);
									printf("stopflag = %d\n",rfidcard[card_value].stopflag);
									printf("-----------------------------------------------------\n");
									/**************************调试*******************************/
									sleep(2);
									lcd_recover(p_menu_nt,223,102,360,280);
									lcd_show_bmp("picture/stopreserve_164_88.bmp",0,552,74,164,88);
									LOOP_NUM2 = 1;
									ret = pthread_create(&pt3,NULL,stop_toll,(void*)&parknum);//计时收费线程
									if(ret != 0){							
										printf("超时建立线程失败\n");
										return;							
									}
									
									return;
								}
								
								if(ts_data2.x>431&&ts_data2.x<485&&ts_data2.y>295&&ts_data2.y<345){//取消
									lcd_show_bmp("picture/cancel_64_60.bmp",0,426,290,64,60);
									usleep(1000*100);
									break;
								}
							}
						}
					}
				/*****************************************************************************************************/
					if(ts_data1.x>288&&ts_data1.x<390&&ts_data1.y>225&&ts_data1.y<275){//点击车位3
						if(((parkstatus>>2)&0x01) == 1){//已经停车或者已经预约
							lcd_show_bmp("picture/reserve_error_t_100_48.bmp",0,289,226,100,48);
							usleep(1000*100);
							lcd_show_bmp("picture/reserve_error_100_48.bmp",0,289,226,100,48);
							int *p_menu_nt1 = lcd_show_bmp("picture/stopalready_360_280.bmp",1,223,102,360,280);
							sleep(2);
							lcd_recover(p_menu_nt1,223,102,360,280);
							continue;
						}
						else{
							int parknum = num3;
							lcd_show_bmp("picture/reserve_select_100_48.bmp",0,289,226,100,48);
							usleep(1000*200);
							lcd_show_bmp("picture/reserve_void_100_48.bmp",0,289,226,100,48);
							while(1){
								lcd_show_bmp("picture/reserve_sure_360_280.bmp",1,223,102,360,280);
								lcd_show_bmp("picture/num3_24_36.bmp",0,394,221,24,36);
								ts_data2 = ts_get_xy();
								if(ts_data2.x>324&&ts_data2.x<378&&ts_data2.y>295&&ts_data2.y<345){//确定
									lcd_show_bmp("picture/yes_64_60.bmp",0,319,290,64,60);
									usleep(1000*100);
									lcd_show_bmp("picture/reserve_success_360_280.bmp",0,223,102,360,280);
									rfidcard[card_value].reserveflag = 3;
									park[2].reserve = 1;
									park[2].id = cardid;
									park[2].parkingnum = 0;
									/**************************调试*******************************/
									printf("-----------------------------------------------------\n");
									printf("id = %x\n",rfidcard[card_value].id);
									printf("overage = %d\n",rfidcard[card_value].overage);
									printf("reserveflag = %d\n",rfidcard[card_value].reserveflag);
									printf("stopflag = %d\n",rfidcard[card_value].stopflag);
									printf("-----------------------------------------------------\n");
									/**************************调试*******************************/
									sleep(2);
									lcd_recover(p_menu_nt,223,102,360,280);
									lcd_show_bmp("picture/stopreserve_164_88.bmp",0,368,177,164,88);
									LOOP_NUM3 = 1;
									ret = pthread_create(&pt3,NULL,stop_toll,(void*)&parknum);//计时收费线程
									if(ret != 0){							
										printf("超时建立线程失败\n");
										return;							
									}
									return;
								}
								
								if(ts_data2.x>431&&ts_data2.x<485&&ts_data2.y>295&&ts_data2.y<345){//取消
									lcd_show_bmp("picture/cancel_64_60.bmp",0,426,290,64,60);
									usleep(1000*100);
									break;
								}
							}
						}
					}
				/*****************************************************************************************************/
					if(ts_data1.x>410&&ts_data1.x<512&&ts_data1.y>225&&ts_data1.y<275){//点击车位4
						if(((parkstatus>>3)&0x01) == 1){//已经停车或者已经预约
							lcd_show_bmp("picture/reserve_error_t_100_48.bmp",0,411,226,100,48);
							usleep(1000*100);
							lcd_show_bmp("picture/reserve_error_100_48.bmp",0,411,226,100,48);
							int *p_menu_nt1 = lcd_show_bmp("picture/stopalready_360_280.bmp",1,223,102,360,280);
							sleep(2);
							lcd_recover(p_menu_nt1,223,102,360,280);
							continue;
						}
						else{
							int parknum = num4;
							lcd_show_bmp("picture/reserve_select_100_48.bmp",0,411,226,100,48);
							usleep(1000*200);
							lcd_show_bmp("picture/reserve_void_100_48.bmp",0,411,226,100,48);
							while(1){
								lcd_show_bmp("picture/reserve_sure_360_280.bmp",1,223,102,360,280);
								lcd_show_bmp("picture/num4_24_36.bmp",0,394,221,24,36);
								ts_data2 = ts_get_xy();
								if(ts_data2.x>324&&ts_data2.x<378&&ts_data2.y>295&&ts_data2.y<345){//确定
									lcd_show_bmp("picture/yes_64_60.bmp",0,319,290,64,60);
									usleep(1000*100);
									lcd_show_bmp("picture/reserve_success_360_280.bmp",0,223,102,360,280);
									rfidcard[card_value].reserveflag = 4;
									park[3].reserve = 1;
									park[3].id = cardid;
									park[3].parkingnum = 0;
									/**************************调试*******************************/
									printf("-----------------------------------------------------\n");
									printf("id = %x\n",rfidcard[card_value].id);
									printf("overage = %d\n",rfidcard[card_value].overage);
									printf("reserveflag = %d\n",rfidcard[card_value].reserveflag);
									printf("stopflag = %d\n",rfidcard[card_value].stopflag);
									printf("-----------------------------------------------------\n");
									/**************************调试*******************************/
									sleep(2);
									lcd_recover(p_menu_nt,223,102,360,280);
									lcd_show_bmp("picture/stopreserve_164_88.bmp",0,552,177,164,88);
									LOOP_NUM4 = 1;
									ret = pthread_create(&pt3,NULL,stop_toll,(void*)&parknum);//计时收费线程
									if(ret != 0){							
										printf("超时建立线程失败\n");
										return;							
									}
									return;
								}
								
								if(ts_data2.x>431&&ts_data2.x<485&&ts_data2.y>295&&ts_data2.y<345){//取消
									lcd_show_bmp("picture/cancel_64_60.bmp",0,426,290,64,60);
									usleep(1000*100);
									break;
								}
							}
						}
					}
					if(ts_data1.x>369&&ts_data1.x<423&&ts_data1.y>287&&ts_data1.y<337){//取消
						lcd_show_bmp("picture/cancel_64_60.bmp",0,364,282,64,60);
						usleep(1000*100);
						lcd_recover(p_menu_nt,223,102,360,280);
						return;
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
