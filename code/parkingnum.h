#ifndef _PARKINGNUM_H
#define _PARKINGNUM_H

typedef enum
{
	num1 = 1,
	num2 = 2,
	num3 = 3,
	num4 = 4,
}Parknum; 

void select_parkingnum(Parknum parknum);
void* stop_toll(void *arg);
int input_password(char *pswd);
#endif /*_PARKINGNUM_H*/