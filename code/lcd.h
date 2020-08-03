#ifndef _LCD_H
#define _LCD_H


typedef enum lcdcolor
{
	White = 0x00ffffff,
	Yellow = 0x00ffff00,
	Red = 0x00ff0000,
	Blue = 0x000000ff,
	Green = 0x0000ff00
}Color;

int lcd_show_color(Color color);
int *lcd_show_bmp(char *bmp_name,char copy_flag,short x_lcd,short y_lcd,short x_bmp,short y_bmp);
int lcd_recover(int *buf,short x_lcd,short y_lcd,short x_bmp,short y_bmp);



#endif /* _LCD_H_ */