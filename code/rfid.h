#ifndef _RFID_H
#define _RFID_H

#define CARDNUM   4
typedef struct
{
	unsigned char parkingnum;//车位号
	unsigned char reserve;//是否预约   0否    1是
	unsigned int id;//卡号
}Park;

typedef struct
{
    const unsigned int id;//卡号
	int overage;//余额
    unsigned char stopflag;
	unsigned char reserveflag;
	unsigned short usetime;
	char password[7];
}Rfid_card;

typedef enum
{
	yueqian,
	school,
	rural,
	abc
}Card_value;

unsigned int get_rfid(void);
void init_tty(int fd);

#endif/*_RFID_H*/