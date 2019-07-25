#ifndef LCD_MCUF_H
#define LCD_MCUF_H

#include <stdlib.h>
#include "colors.h"

#include "lcd.h"

class Lcd_mcuf : Lcd {

public:
	Lcd_mcuf(void);

	// touch screen
	bool detectTouch(void);
	bool getTouch(cursor_t *t);
	void *operator new(size_t size) { return malloc(size);}
	void operator delete(void *ptr) { if (ptr) free(ptr); }

};










#endif // LCD_MCUF_H