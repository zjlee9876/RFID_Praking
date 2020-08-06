#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "touch.h"
#include "lcd.h"
#include "parkingnum.h"
#include "rfid.h"
#include "beep.h"

static int time_10s_falg = 0;
int LOOP_NUM1 = 0;
int LOOP_NUM2 = 0;
int LOOP_NUM3 = 0;
int LOOP_NUM4 = 0;
int time_1s_flag = 0;
Park park[CARDNUM];
extern Rfid_card rfidcard[CARDNUM];
//extern void *play_music(void *arg);



static void* timer(void *arg)//超时
{
	
	pthread_detach(pthread_self());
	int *time = (int*)arg;
	//printf("time = %d\n",*time);
	while((*time)--) sleep(1);
	time_10s_falg = 1;
	
}
static void* read_card(void *arg)//读卡
{
	unsigned int *cardid = (unsigned int*)arg;
	pthread_detach(pthread_self());
	*cardid = get_rfid();
}


void* stop_toll(void *arg)//计时
{
	pthread_detach(pthread_self());
	int num = *((int*)arg);
	switch(num){
		case 1:
			while(LOOP_NUM1){
				sleep(1);
				//park[num-1].stoptime++;
				//printf("parkingnum%d strat timing %d\n",num,park[num-1].stoptime);
				if(park[num-1].id == rfidcard[yueqian].id){
					rfidcard[yueqian].usetime++;
					printf("yuqian_card is strat timing %d\n",rfidcard[yueqian].usetime);
				} 
				if(park[num-1].id == rfidcard[school].id){
					rfidcard[school].usetime++;
					printf("school_card is strat timing %d\n",rfidcard[school].usetime);
				} 
				if(park[num-1].id == rfidcard[rural].id){
					rfidcard[rural].usetime++;
					printf("rural_card is strat timing %d\n",rfidcard[rural].usetime);
				}
				if(park[num-1].id == rfidcard[abc].id){
					rfidcard[abc].usetime++;
					printf("abc_card is strat timing %d\n",rfidcard[abc].usetime);
				}
				time_1s_flag = 1;
			}
		break;
		case 2:
			while(LOOP_NUM2){
				sleep(1);
				//park[num-1].stoptime++;
				//printf("parkingnum%d strat timing %d\n",num,park[num-1].stoptime);
				if(park[num-1].id == rfidcard[yueqian].id){
					rfidcard[yueqian].usetime++;
					printf("yuqian_card is strat timing %d\n",rfidcard[yueqian].usetime);
				} 
				if(park[num-1].id == rfidcard[school].id){
					rfidcard[school].usetime++;
					printf("school_card is strat timing %d\n",rfidcard[school].usetime);
				} 
				if(park[num-1].id == rfidcard[rural].id){
					rfidcard[rural].usetime++;
					printf("rural_card is strat timing %d\n",rfidcard[rural].usetime);
				}
				if(park[num-1].id == rfidcard[abc].id){
					rfidcard[abc].usetime++;
					printf("abc_card is strat timing %d\n",rfidcard[abc].usetime);
				}
				time_1s_flag = 1;
			}
		break;
		case 3:
			while(LOOP_NUM3){
				sleep(1);
				//park[num-1].stoptime++;
				//printf("parkingnum%d strat timing %d\n",num,park[num-1].stoptime);
				if(park[num-1].id == rfidcard[yueqian].id){
					rfidcard[yueqian].usetime++;
					printf("yuqian_card is strat timing %d\n",rfidcard[yueqian].usetime);
				} 
				if(park[num-1].id == rfidcard[school].id){
					rfidcard[school].usetime++;
					printf("school_card is strat timing %d\n",rfidcard[school].usetime);
				} 
				if(park[num-1].id == rfidcard[rural].id){
					rfidcard[rural].usetime++;
					printf("rural_card is strat timing %d\n",rfidcard[rural].usetime);
				}
				if(park[num-1].id == rfidcard[abc].id){
					rfidcard[abc].usetime++;
					printf("abc_card is strat timing %d\n",rfidcard[abc].usetime);
				}
				time_1s_flag = 1;
			}
		break;
		case 4:
			while(LOOP_NUM4){
				sleep(1);
				//park[num-1].stoptime++;
				//printf("parkingnum%d strat timing %d\n",num,park[num-1].stoptime);
				if(park[num-1].id == rfidcard[yueqian].id){
					rfidcard[yueqian].usetime++;
					printf("yuqian_card is strat timing %d\n",rfidcard[yueqian].usetime);
				} 
				if(park[num-1].id == rfidcard[school].id){
					rfidcard[school].usetime++;
					printf("school_card is strat timing %d\n",rfidcard[school].usetime);
				} 
				if(park[num-1].id == rfidcard[rural].id){
					rfidcard[rural].usetime++;
					printf("rural_card is strat timing %d\n",rfidcard[rural].usetime);
				}
				if(park[num-1].id == rfidcard[abc].id){
					rfidcard[abc].usetime++;
					printf("abc_card is strat timing %d\n",rfidcard[abc].usetime);
				}
				time_1s_flag = 1;
			}
		break;
	}
	
	printf("stop timing\n");
}

						
static void *thread_getxy(void *arg)
{
	pthread_detach(pthread_self());
	struct ts_xy *p = (struct ts_xy*)arg;
	while(1) *p = ts_get_xy(); 
}
/*
函数原型：int input_password(char *pswd)
函数功能：获取输入的密码
返回值：0取消操作
		1成功
		
		
参数：密码的首地址
*/
int input_password(char *pswd)
{
	int *pswd_nt = NULL;
	//char pswd[7];//卡密码
	int pswd_count = 0;
	struct ts_xy ts_data2;
	lcd_show_bmp("picture/password_360_280.bmp",0,223,102,360,280);
	while(pswd_count<6){
		ts_data2 = ts_get_xy();
		if(ts_data2.x>248 && ts_data2.x<300 && ts_data2.y>132 && ts_data2.y<184){//1
			pswd_nt = lcd_show_bmp("picture/password_t_52_52.bmp",1,248,132,52,52);
			usleep(1000*100);
			lcd_recover(pswd_nt,248,132,52,52);
			pswd[pswd_count] = '1';
			pswd_count++;
		}
		
		if(ts_data2.x>302 && ts_data2.x<354 && ts_data2.y>132 && ts_data2.y<184){//2
			pswd_nt = lcd_show_bmp("picture/password_t_52_52.bmp",1,302,132,52,52);
			usleep(1000*100);
			lcd_recover(pswd_nt,302,132,52,52);
			pswd[pswd_count] = '2';
			pswd_count++;
		}
		
		if(ts_data2.x>357 && ts_data2.x<409 && ts_data2.y>132 && ts_data2.y<184){//3
			pswd_nt = lcd_show_bmp("picture/password_t_52_52.bmp",1,357,132,52,52);
			usleep(1000*100);
			lcd_recover(pswd_nt,357,132,52,52);
			pswd[pswd_count] = '3';
			pswd_count++;
		}
		/*************************************************************************/
		if(ts_data2.x>248 && ts_data2.x<300 && ts_data2.y>186 && ts_data2.y<238){//4
			pswd_nt = lcd_show_bmp("picture/password_t_52_52.bmp",1,248,186,52,52);
			usleep(1000*100);
			lcd_recover(pswd_nt,248,186,52,52);
			pswd[pswd_count] = '4';
			pswd_count++;
		}
		if(ts_data2.x>302 && ts_data2.x<354 && ts_data2.y>186 && ts_data2.y<238){//5
			pswd_nt = lcd_show_bmp("picture/password_t_52_52.bmp",1,302,186,52,52);
			usleep(1000*100);
			lcd_recover(pswd_nt,302,186,52,52);
			pswd[pswd_count] = '5';
			pswd_count++;
		}
		if(ts_data2.x>357 && ts_data2.x<409 && ts_data2.y>186 && ts_data2.y<238){//6
			pswd_nt = lcd_show_bmp("picture/password_t_52_52.bmp",1,357,186,52,52);
			usleep(1000*100);
			lcd_recover(pswd_nt,357,186,52,52);
			pswd[pswd_count] = '6';
			pswd_count++;
		}
		/*************************************************************************/		
		if(ts_data2.x>248 && ts_data2.x<300 && ts_data2.y>241 && ts_data2.y<293){//7
			pswd_nt = lcd_show_bmp("picture/password_t_52_52.bmp",1,248,241,52,52);
			usleep(1000*100);
			lcd_recover(pswd_nt,248,241,52,52);
			pswd[pswd_count] = '7';
			pswd_count++;
		}
		
		if(ts_data2.x>302 && ts_data2.x<354 && ts_data2.y>241 && ts_data2.y<293){//8
			pswd_nt = lcd_show_bmp("picture/password_t_52_52.bmp",1,302,241,52,52);
			usleep(1000*100);
			lcd_recover(pswd_nt,302,241,52,52);
			pswd[pswd_count] = '8';
			pswd_count++;
		}
		
		if(ts_data2.x>357 && ts_data2.x<409 && ts_data2.y>241 && ts_data2.y<293){//9
			pswd_nt = lcd_show_bmp("picture/password_t_52_52.bmp",1,357,241,52,52);
			usleep(1000*100);
			lcd_recover(pswd_nt,357,241,52,52);
			pswd[pswd_count] = '9';
			pswd_count++;
		}
		/*************************************************************************/		
		if(ts_data2.x>248 && ts_data2.x<300 && ts_data2.y>296 && ts_data2.y<349){//清除
			pswd_nt = lcd_show_bmp("picture/password_t_52_52.bmp",1,248,296,52,52);
			usleep(1000*100);
			lcd_recover(pswd_nt,248,296,52,52);
			lcd_show_bmp("picture/pwclear_128_24.bmp",0,429,243,128,24);
			pswd_count = 0;
		}
		
		if(ts_data2.x>302 && ts_data2.x<354 && ts_data2.y>296 && ts_data2.y<349){//0
			pswd_nt = lcd_show_bmp("picture/password_t_52_52.bmp",1,302,296,52,52);
			usleep(1000*100);
			lcd_recover(pswd_nt,302,296,52,52);
			pswd[pswd_count] = '0';
			pswd_count++;
		}
		
		if(ts_data2.x>357 && ts_data2.x<409 && ts_data2.y>296 && ts_data2.y<349){//backspace
			pswd_nt = lcd_show_bmp("picture/password_t_52_52.bmp",1,357,296,52,52);
			usleep(1000*100);
			lcd_recover(pswd_nt,357,296,52,52);
			if(0<pswd_count&&pswd_count<6) lcd_show_bmp("picture/pwvoid_20_20.bmp",0,437+(pswd_count-1)*20,246,20,20);
			if(--pswd_count<0) pswd_count = 0;
			
		}
		if(ts_data2.x>511&&ts_data2.x<565&&ts_data2.y>119&&ts_data2.y<169){//取消
			lcd_show_bmp("picture/cancel_64_60.bmp",0,506,114,64,60);
			usleep(1000*100);
			return 0;
		}
		if(0<pswd_count&&pswd_count<6) lcd_show_bmp("picture/pw_20_20.bmp",0,437+(pswd_count-1)*20,246,20,20);
		
	}
	pswd[6] = '\0';
	printf("password = %s\n",pswd);
	return 1;
}

void select_parkingnum(Parknum parknum)
{
	pthread_t pt1,pt2,pt3,pt4,pt_beep;
	int *p_menu_nt = NULL;
	int ret;
	int count1 = 0,count2 = 0;
	int time = 10;
	int beep_ture = 1;
	int beep_fault = 2;
	int stop_errornum = 0;
	int stopnum = 0;
	int reservenum = 0;
	int existnum = 0;
	unsigned int cardid = 0;
	struct ts_xy ts_data,ts_data1,ts_data2;
	char str[30];
	char str1[23];
	char pswd[7];
	int card_value;//调试
	switch(parknum){
		case num1:
			if(park[parknum-1].reserve == 1){
				lcd_show_bmp("picture/stopreserve_t_164_88.bmp",0,368,74,164,88);
				usleep(1000*100);
				lcd_show_bmp("picture/stopreserve_164_88.bmp",0,368,74,164,88);
				reservenum = 1;
			}
			if(park[parknum-1].reserve == 0 && park[parknum-1].parkingnum == 0){
				lcd_show_bmp("picture/selectparknum_t_164_88.bmp",0,368,74,164,88);
				usleep(1000*100);
				lcd_show_bmp("picture/stopvoid_164_88.bmp",0,368,74,164,88);
				stopnum = 1;
			}
			if(park[parknum-1].parkingnum){
				lcd_show_bmp("picture/exitcar_t_164_88.bmp",0,368,74,164,88);
				usleep(1000*100);
				lcd_show_bmp("picture/stopcar_164_88.bmp",0,368,74,164,88);
				existnum = 1;
			}
			break;
		case num2:
			if(park[parknum-1].reserve == 1){
				lcd_show_bmp("picture/stopreserve_t_164_88.bmp",0,552,74,164,88);
				usleep(1000*100);
				lcd_show_bmp("picture/stopreserve_164_88.bmp",0,552,74,164,88);
				reservenum = 2;
			}
			if(park[parknum-1].reserve == 0 && park[parknum-1].parkingnum == 0){
				lcd_show_bmp("picture/selectparknum_t_164_88.bmp",0,552,74,164,88);
				usleep(1000*100);
				lcd_show_bmp("picture/stopvoid_164_88.bmp",0,552,74,164,88);
				stopnum = 2;
			}
			if(park[parknum-1].parkingnum){
				lcd_show_bmp("picture/exitcar_t_164_88.bmp",0,552,74,164,88);
				usleep(1000*100);
				lcd_show_bmp("picture/stopcar_164_88.bmp",0,552,74,164,88);
				existnum = 2;
			}
			break;
		
		case num3:
			if(park[parknum-1].reserve == 1){
				lcd_show_bmp("picture/stopreserve_t_164_88.bmp",0,368,177,164,88);
				usleep(1000*100);
				lcd_show_bmp("picture/stopreserve_164_88.bmp",0,368,177,164,88);
				reservenum = 3;
			}
			if(park[parknum-1].reserve == 0 && park[parknum-1].parkingnum == 0){
				lcd_show_bmp("picture/selectparknum_t_164_88.bmp",0,368,177,164,88);
				usleep(1000*100);
				lcd_show_bmp("picture/stopvoid_164_88.bmp",0,368,177,164,88);
				stopnum = 3;
			}
			if(park[parknum-1].parkingnum){
				lcd_show_bmp("picture/exitcar_t_164_88.bmp",0,368,177,164,88);
				usleep(1000*100);
				lcd_show_bmp("picture/stopcar_164_88.bmp",0,368,177,164,88);
				existnum = 3;
			}
			break;
		
		case num4:
			if(park[parknum-1].reserve == 1){
				lcd_show_bmp("picture/stopreserve_t_164_88.bmp",0,552,177,164,88);
				usleep(1000*100);
				lcd_show_bmp("picture/stopreserve_164_88.bmp",0,552,177,164,88);
				reservenum = 4;
			}
			if(park[parknum-1].reserve == 0 && park[parknum-1].parkingnum == 0){
				lcd_show_bmp("picture/selectparknum_t_164_88.bmp",0,552,177,164,88);
				usleep(1000*100);
				lcd_show_bmp("picture/stopvoid_164_88.bmp",0,552,177,164,88);
				stopnum = 4;
			}
			if(park[parknum-1].parkingnum){
				lcd_show_bmp("picture/exitcar_t_164_88.bmp",0,552,177,164,88);
				usleep(1000*100);
				lcd_show_bmp("picture/stopcar_164_88.bmp",0,552,177,164,88);
				existnum = 4;
			}
			break;
	}

	if(existnum){
		p_menu_nt = lcd_show_bmp("picture/stopalready_360_280.bmp",1,223,102,360,280);
		sprintf(str1,"%s%d%s","picture/num",existnum,"_24_36.bmp");
		lcd_show_bmp(str1,0,316,287,24,36);
		sleep(2);
		lcd_recover(p_menu_nt,223,102,360,280);
		return;
	}
	if(stopnum){
		p_menu_nt = lcd_show_bmp("picture/stop_sure_360_280.bmp",1,223,102,360,280);
		sprintf(str1,"%s%d%s","picture/num",stopnum,"_24_36.bmp");
		lcd_show_bmp(str1,0,394,221,24,36);
	}
	if(reservenum){
		p_menu_nt = lcd_show_bmp("picture/reservestop_sure_360_280.bmp",1,223,102,360,280);
		sprintf(str1,"%s%d%s","picture/num",reservenum,"_24_36.bmp");
		lcd_show_bmp(str1,0,394,221,24,36);
	}
	while(1){
		ts_data1 = ts_get_xy();
		if(ts_data1.x>324&&ts_data1.x<378&&ts_data1.y>295&&ts_data1.y<345){//确定
			lcd_show_bmp("picture/yes_64_60.bmp",0,319,290,64,60);
			usleep(1000*100);
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
			break;
		}
		if(ts_data1.x>431&&ts_data1.x<485&&ts_data1.y>295&&ts_data1.y<345){//取消
			lcd_show_bmp("picture/cancel_64_60.bmp",0,426,290,64,60);
			usleep(1000*100);
			lcd_recover(p_menu_nt,223,102,360,280);
			return;
		}
	}
	while(1){
		for(int i = 1;i < 5;i++){
			sprintf(str,"%s%d%s","picture/readcard",i,"_360_280.bmp");
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
				ret = pthread_create(&pt_beep,NULL,beep,(void*)&beep_ture);
				if(ret != 0){							
					printf("超时建立线程失败\n");
					return;							
				}
				//输入密码
			
				pthread_cancel(pt1);
				pthread_cancel(pt4);
				if(cardid == rfidcard[yueqian].id) card_value = 0; 
				if(cardid == rfidcard[school].id)  card_value = 1; 
				if(cardid == rfidcard[rural].id)  card_value = 2; 
				if(cardid == rfidcard[abc].id)  card_value = 3; 
				if(rfidcard[card_value].stopflag && rfidcard[card_value].reserveflag==0){//卡已经停车
					stop_errornum = 1;
					pthread_cancel(pt1);
					pthread_cancel(pt4);
					lcd_show_bmp("picture/carexist_360_280.bmp",0,223,102,360,280);
				}
				if(rfidcard[card_value].stopflag==0 && rfidcard[card_value].reserveflag){//卡已经预约,判断此车位是否为预约的车位
					if(park[parknum-1].reserve == 1){
						if(park[parknum-1].id != rfidcard[card_value].id){
							stop_errornum = 1;
							pthread_cancel(pt1);
							pthread_cancel(pt4);
							lcd_show_bmp("picture/reserver_no_360_280.bmp",0,223,102,360,280);
						}
					}
					else{
						stop_errornum = 1;
						pthread_cancel(pt1);
						pthread_cancel(pt4);
						lcd_show_bmp("picture/carreserve_360_280.bmp",0,223,102,360,280);
					}
				}
				if(rfidcard[card_value].stopflag==0 && rfidcard[card_value].reserveflag==0){//卡没有预约也没有停车，需要判断此车位有无预约
					if(park[parknum-1].reserve==1){
						stop_errornum = 1;
						pthread_cancel(pt1);
						pthread_cancel(pt4);
						lcd_show_bmp("picture/reservealready_360_280.bmp",0,223,102,360,280);
					}
				}
				printf("---------------stop_errornum = %d--------------\n",stop_errornum);
				if(stop_errornum == 1){
					ret = pthread_create(&pt_beep,NULL,beep,(void*)&beep_fault);
					if(ret != 0){							
						printf("超时建立线程失败\n");
						return;							
					}
					sleep(2);
					lcd_recover(p_menu_nt,223,102,360,280);
					return;
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
				park[parknum-1].id = cardid;//在车位的卡号
				park[parknum-1].parkingnum = 1;//所停的车位号，也是停车的标志
				if(park[parknum-1].id == rfidcard[yueqian].id){
					rfidcard[yueqian].stopflag = parknum;//该卡的车已经入库
					rfidcard[yueqian].reserveflag = 0;//既然已经停车了，也就不存在预约这个东西了
				}
				if(park[parknum-1].id == rfidcard[school].id){
					rfidcard[school].stopflag = parknum;//该卡的车已经入库
					rfidcard[school].reserveflag = 0;//既然已经停车了，也就不存在预约这个东西了
				}
				if(park[parknum-1].id == rfidcard[rural].id){
					rfidcard[rural].stopflag = parknum;//该卡的车已经入库
					rfidcard[rural].reserveflag = 0;//既然已经停车了，也就不存在预约这个东西了
				}
				if(park[parknum-1].id == rfidcard[abc].id){
					rfidcard[abc].stopflag = parknum;//该卡的车已经入库
					rfidcard[abc].reserveflag = 0;//既然已经停车了，也就不存在预约这个东西了
				}
				if(park[parknum-1].parkingnum && park[parknum-1].reserve==0){
					if(parknum==num1) LOOP_NUM1 = 1;
					if(parknum==num2) LOOP_NUM2 = 1;
					if(parknum==num3) LOOP_NUM3 = 1;
					if(parknum==num4) LOOP_NUM4 = 1;
					ret = pthread_create(&pt3,NULL,stop_toll,(void*)&parknum);//计时收费线程
					if(ret != 0){							
						printf("超时建立线程失败\n");
						return;							
					}
				}
				park[parknum-1].reserve = 0;//预约标志清0
				lcd_show_bmp("picture/readcard_success_360_280.bmp",0,223,102,360,280);
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
				switch(parknum){
					case num1:
						lcd_show_bmp("picture/stopcar_164_88.bmp",0,368,74,164,88);
						return;
					
					case num2:
						lcd_show_bmp("picture/stopcar_164_88.bmp",0,552,74,164,88);
						return;
					
					case num3:
						lcd_show_bmp("picture/stopcar_164_88.bmp",0,368,177,164,88);
						return;
					
					case num4:
						lcd_show_bmp("picture/stopcar_164_88.bmp",0,552,177,164,88);
						return;
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
	/*****************************************************************************************************/
	}				
}

