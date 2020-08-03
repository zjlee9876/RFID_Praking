#ifndef _BEEP_H
#define _BEEP_H

typedef enum beepstatus
{
	OFF = 0,
	ON = 1
}Beepstatus;

void beep_ctrl_interface(void);
void* beep(void *arg);
#endif /* _BEEP_H */