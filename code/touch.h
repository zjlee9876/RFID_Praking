#ifndef _TOUCH_H
#define _TOUCH_H


struct ts_xy {
	unsigned int x;
	unsigned int y;
};

struct ts_xy ts_get_xy(void);


#endif /*_TOUCH_H*/
