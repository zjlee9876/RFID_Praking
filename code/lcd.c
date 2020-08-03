#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "lcd.h"

int lcd_show_color(Color color)
{
	int fd_bmp,fd_lcd,i,j;
	char bmp_buf[800*480*3];	
	int lcd_buf[800*480];
	int lcd_new[800*480];
	int *lcd_base = NULL;
	fd_lcd = open("/dev/fb0", O_RDWR);
	if(fd_lcd == -1){
		perror("open lcd");
		return -1;		
	}
	lcd_base = mmap(NULL, 800*480*4, PROT_READ|PROT_WRITE, MAP_SHARED, fd_lcd, 0);
	if(lcd_base == MAP_FAILED){
		perror("mmap failed");
		return -1;		
	}
	for(i=0;i<800*480;i++)
		*(lcd_base + i) = color;
	munmap(lcd_base, 800*480*4);
	close(fd_lcd);
	return 0;
}

/*
函数功能：显示bmp图片
返回值：成功--显示图片之前的lcd数据的首地址 失败--NULL
参数：bmp_name：图片的路径
	  x_lcd:开始显示的横坐标
	  y_lcd:开始显示的纵坐标
	  x_bmp：图片的横像素
	  y_bmp：图片的纵像素
*/
int *lcd_show_bmp(char *bmp_name,char copy_flag,short x_lcd,short y_lcd,short x_bmp,short y_bmp)
{
	
	int fd_bmp,fd_lcd,i,j,count = 0;
	char * const bmp_buf = (char*)malloc(x_bmp*y_bmp*sizeof(char)*3);
	int * const lcd_buf = (int*)malloc(x_bmp*y_bmp*sizeof(int));
	int *readlcd_buf = (int*)malloc(x_bmp*y_bmp*sizeof(int));
	int *lcd_base = NULL;
	fd_bmp = open(bmp_name, O_RDONLY);
	if(fd_bmp == -1){
		perror("open bmp");
		return NULL;
	}
	lseek(fd_bmp, 54, SEEK_SET);	
	read(fd_bmp, bmp_buf, x_bmp*y_bmp*3);
	close(fd_bmp);
	for(i=0;i<x_bmp*y_bmp;i++)
		lcd_buf[i] = (0x00<<24) + (bmp_buf[i*3+2]<<16) + (bmp_buf[i*3+1]<<8) + (bmp_buf[i*3]<<0);
	fd_lcd = open("/dev/fb0", O_RDWR);
	if(fd_lcd == -1){
		perror("oepn lcd");
		return NULL;		
	}
	lcd_base = mmap(NULL, 800*480*4, PROT_READ|PROT_WRITE, MAP_SHARED, fd_lcd, 0);
	if(lcd_base == MAP_FAILED){
		perror("mmap failed");
		return NULL;		
	}
	for(i=0;i<y_bmp;i++){
		for(j=0;j<x_bmp;j++){
			if(copy_flag) readlcd_buf[count] = *(lcd_base + ((y_lcd+y_bmp-1-i)*800+x_lcd)+j);
			*(lcd_base + ((y_lcd+y_bmp-1-i)*800+x_lcd)+j) = lcd_buf[count];
			count++;
		}
	}
	munmap(lcd_base, 800*480*4);
	free(bmp_buf);
	free(lcd_buf);
	close(fd_lcd);	
	return readlcd_buf;
}
/*
函数功能：恢复图片挖空区域
返回值：成功--0 失败-- -1
参数：buf：恢复数据的首地址
	  x_lcd:开始显示的横坐标
	  y_lcd:开始显示的纵坐标
	  x_bmp：图片的横像素
	  y_bmp：图片的纵像素
*/
int lcd_recover(int *buf,short x_lcd,short y_lcd,short x_bmp,short y_bmp)
{
	int fd_lcd,count = 0;
	int *lcd_base = NULL;
	fd_lcd = open("/dev/fb0", O_RDWR);
	if(fd_lcd == -1){
		perror("oepn lcd");
		return -1;		
	}
	lcd_base = mmap(NULL, 800*480*4, PROT_READ|PROT_WRITE, MAP_SHARED, fd_lcd, 0);
	if(lcd_base == MAP_FAILED){
		perror("mmap failed");
		return -1;		
	}
	for(int i=0;i<y_bmp;i++){
		for(int j=0;j<x_bmp;j++){
			*(lcd_base + ((y_lcd+y_bmp-1-i)*800+x_lcd)+j) = buf[count];
			count++;
		}
	}
	munmap(lcd_base, 800*480*4);
	close(fd_lcd);	
	return 0;
}

