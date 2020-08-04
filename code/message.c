#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "touch.h"
#include "lcd.h"
#include "parkingnum.h"
#include "rfid.h"
#include "message.h"
#include "beep.h"


static int time_10s_falg = 0;
static int *p_menu_nt = NULL;
static 	int card_value;
extern Rfid_card rfidcard[CARDNUM];
extern int time_1s_flag;
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
Rfid_card* card_message(void)
{
	pthread_t pt1,pt2,pt4,pt_beep;
	int ret;
	int count1 = 0,count2 = 0;
	int time = 10;
	int beep_true = 1;

	unsigned int cardid = 0;
	struct ts_xy ts_data,ts_data2;
	Rfid_card* card_mesg = NULL;
	char str[30];
	char pswd[7];
	char str1[23];
	ret = pthread_create(&pt1,NULL,timer,(void*)&time);//检测是否超时线程
	if(ret != 0){							
		printf("超时建立线程失败\n");
		return NULL;							
	}
	ret = pthread_create(&pt2,NULL,read_card,(void*)&cardid);//检测是否刷卡线程
	if(ret != 0){							
		printf("读卡建立线程失败\n");
		return NULL;							
	}
	ret = pthread_create(&pt4,NULL,thread_getxy,(void*)&ts_data);//触摸
	if(ret != 0){							
		printf("读卡建立线程失败\n");
		return NULL;							
	}
	usleep(2);
	lcd_show_bmp("picture/card_msg_t_208_100.bmp",0,59,193,208,100);
	usleep(1000*100);
	lcd_show_bmp("picture/card_msg_208_100.bmp",0,59,193,208,100);
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
				sleep(3);
				lcd_recover(p_menu_nt,223,102,360,280);
				return NULL;
			}
			if(cardid){//读卡成功处理
				ret = pthread_create(&pt_beep,NULL,beep,(void*)&beep_true);//检测是否超时线程
				if(ret != 0){							
					printf("超时建立线程失败\n");
					return NULL;							
				}
				pthread_cancel(pt1);
				pthread_cancel(pt4);
				if(cardid == rfidcard[yueqian].id) card_value = 0; 
				if(cardid == rfidcard[school].id)  card_value = 1; 
				if(cardid == rfidcard[rural].id)  card_value = 2; 
				if(cardid == rfidcard[abc].id)  card_value = 3; 
				int errornum = 2;
				while(1){
					ret =  input_password(pswd);
					if(ret == 0){//取消输入
						lcd_recover(p_menu_nt,223,102,360,280);
						return NULL;
					}
					if(strcmp(pswd,rfidcard[card_value].password) != 0){//密码错误
						if(errornum == 0){
							lcd_show_bmp("picture/password_errorlast_360_280.bmp",0,223,102,360,280);
							sleep(3);
							lcd_recover(p_menu_nt,223,102,360,280);
							return NULL;
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
								return NULL;
							}
						}
					}
					else{//密码正确
						break;
					}
					errornum--;		
				}
				if(cardid== rfidcard[yueqian].id){
					card_mesg =  &rfidcard[yueqian];
				}
				if(cardid == rfidcard[school].id){
					card_mesg =  &rfidcard[school];
				}
					
				if(cardid == rfidcard[rural].id){
					card_mesg =  &rfidcard[rural];
				}
					
				if(cardid == rfidcard[abc].id){
					card_mesg =  &rfidcard[abc];
				}
				return card_mesg;
				
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
				return NULL;
			}
			usleep(1000*500);
		}

    }				
}
void show_message(void)
{
	pthread_t pt2;
	struct ts_xy ts_data,ts_data2;
	char str[23];
	char oldpswd[7];
	char new1pswd[7];
	char new2pswd[7];
	Rfid_card* ret;
	int ret1;
	ret = card_message();
	if(ret == NULL) return;
	ret1 = pthread_create(&pt2,NULL,thread_getxy,(void*)&ts_data);//触摸
	if(ret1 != 0){							
		printf("读卡建立线程失败\n");
		return;							
	}
	/***************************调试**************************/
	printf("id = %x\n",ret->id);
	printf("overagr = %d\n",ret->overage);
	printf("stopnum = %d\n",ret->stopflag);
	printf("reservenum = %d\n",ret->reserveflag);
	printf("usetime = %d\n",ret->usetime);
	/***************************调试**************************/	
	lcd_show_bmp("picture/card_msg_360_280.bmp",0,223,102,360,280);
	switch(ret->id){//显示卡号
		case 0x7d78e9f7://yueqian
			lcd_show_bmp("picture/yueqian_card_104_36.bmp",0,344,134,104,36);
		break;
			
		case 0x2f0efd0f://school
			lcd_show_bmp("picture/school_card_104_36.bmp",0,344,134,104,36);
		break;
			
		case 0xb85851a1://rural
			lcd_show_bmp("picture/rural_card_104_36.bmp",0,344,134,104,36);
		break;
			
		case 0x6aa1c18f://abc
			lcd_show_bmp("picture/abc_card_104_36.bmp",0,344,134,104,36);
		break;
	}
	if(ret->stopflag){//显示停车状态
		sprintf(str,"%s%d%s","picture/num",ret->stopflag,"_24_36.bmp");
		lcd_show_bmp(str,0,344,178,24,36);
		lcd_show_bmp("picture/numcarpark_72_36.bmp",0,369,178,72,36);
	}
	else lcd_show_bmp("picture/wu_24_36.bmp",0,344,178,24,36);
	if(ret->reserveflag){//显示预约状态
		sprintf(str,"%s%d%s","picture/num",ret->reserveflag,"_24_36.bmp");
		lcd_show_bmp(str,0,344,222,24,36);
		lcd_show_bmp("picture/numcarpark_72_36.bmp",0,369,222,72,36);
	}
	else lcd_show_bmp("picture/wu_24_36.bmp",0,344,222,24,36);
	//显示余额
	
	int overage_g = ret->overage%10;
	int overage_s = (ret->overage/10)%10;
	int overage_b = ret->overage/100;
	sprintf(str,"%s%d%s","picture/num",overage_b,"_24_36.bmp");
	lcd_show_bmp(str,0,344,263,24,36);
	sprintf(str,"%s%d%s","picture/num",overage_s,"_24_36.bmp");
	lcd_show_bmp(str,0,369,263,24,36);
	sprintf(str,"%s%d%s","picture/num",overage_g,"_24_36.bmp");
	lcd_show_bmp(str,0,394,263,24,36);
	//使用显示时间
	char s = 0;
	char m = 0;
	char h = 0;
	int temp = 0;
	lcd_show_bmp("picture/num0_24_36.bmp",0,379,309,24,36);
	lcd_show_bmp("picture/num0_24_36.bmp",0,404,309,24,36);
	lcd_show_bmp("picture/num0_24_36.bmp",0,435,309,24,36);
	lcd_show_bmp("picture/num0_24_36.bmp",0,460,309,24,36);
	lcd_show_bmp("picture/num0_24_36.bmp",0,490,309,24,36);
	lcd_show_bmp("picture/num0_24_36.bmp",0,515,309,24,36);
	while(1){
		if(time_1s_flag){
			temp = ret->usetime;//每隔1s获取时间
			time_1s_flag = 0;
			h = temp/3600;
			m = temp/60 - h*60;
			s = temp - m*60 - h*3600;
			//printf("%d %d %d",h,m,s);
			sprintf(str,"%s%d%s","picture/num",h/10,"_24_36.bmp");
			lcd_show_bmp(str,0,379,309,24,36);
			sprintf(str,"%s%d%s","picture/num",h%10,"_24_36.bmp");
			lcd_show_bmp(str,0,404,309,24,36);
			sprintf(str,"%s%d%s","picture/num",m/10,"_24_36.bmp");
			lcd_show_bmp(str,0,435,309,24,36);
			sprintf(str,"%s%d%s","picture/num",m%10,"_24_36.bmp");
			lcd_show_bmp(str,0,460,309,24,36);
			sprintf(str,"%s%d%s","picture/num",s/10,"_24_36.bmp");
			lcd_show_bmp(str,0,490,309,24,36);
			sprintf(str,"%s%d%s","picture/num",s%10,"_24_36.bmp");
			lcd_show_bmp(str,0,515,309,24,36);
		}

		if(ts_data.x>511&&ts_data.x<565&&ts_data.y>119&&ts_data.y<169){
			pthread_cancel(pt2);
			lcd_show_bmp("picture/cancel_64_60.bmp",0,506,114,64,60);
			usleep(1000*100);
			break;
		}
		if(ts_data.x>511&&ts_data.x<565&&ts_data.y>205&&ts_data.y<255){//修改密码
			int *messagegui = NULL;
			lcd_show_bmp("picture/changepassword_64_60.bmp",0,506,200,64,60);
			usleep(1000*100);
			lcd_show_bmp("picture/changepassword_void_64_60.bmp",0,506,200,64,60);
			messagegui = lcd_show_bmp("picture/changepassword1_360_280.bmp",1,223,102,360,280);
			sleep(1);
			int errornum = 2;
			while(1){
				ret1 = input_password(oldpswd);
				if(ret1 == 0){//取消输入
					lcd_recover(messagegui,223,102,360,280);
					ts_data.x = 0;
					ts_data.y = 0;
					break;
				}
				if(strcmp(oldpswd,rfidcard[card_value].password) != 0){//密码错误
					if(errornum == 0){
						lcd_show_bmp("picture/password_errorlast_360_280.bmp",0,223,102,360,280);
						sleep(3);
						lcd_recover(messagegui,223,102,360,280);
						break;
					}
					lcd_show_bmp("picture/password_error_360_280.bmp",0,223,102,360,280);
					sprintf(str,"%s%d%s","picture/num",errornum,"_24_36.bmp");
					lcd_show_bmp(str,0,442,250,24,36);
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
							lcd_recover(messagegui,223,102,360,280);
							break;
						}
					}
				}
				else{//密码正确
					lcd_show_bmp("picture/changepassword2_360_280.bmp",0,223,102,360,280);
					sleep(1);
					ret1 = input_password(new1pswd);
					if(ret1 == 0){//取消输入
						lcd_recover(messagegui,223,102,360,280);
						ts_data.x = 0;
						ts_data.y = 0;
						break;
					}
					lcd_show_bmp("picture/changepassword3_360_280.bmp",0,223,102,360,280);
					sleep(1);
					while(1){
						ret1 = input_password(new2pswd);
						if(ret1 == 0){//取消输入
							lcd_recover(messagegui,223,102,360,280);
							ts_data.x = 0;
							ts_data.y = 0;
							break;
						}
						if(strcmp(new1pswd,new2pswd) != 0){
							lcd_show_bmp("picture/password_changeerror_360_280.bmp",0,223,102,360,280);
							sleep(1);
							continue;
						}
						else{
							strcpy(rfidcard[card_value].password,new2pswd);
							lcd_show_bmp("picture/changepwssword_success_360_280.bmp",0,223,102,360,280);
							sleep(2);
							lcd_recover(messagegui,223,102,360,280);
							break;
						}
					}
				break;
				}
				errornum--;		
			}
		}
	}
	lcd_recover(p_menu_nt,223,102,360,280);	
}
