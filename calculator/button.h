/*
 * button.h
 *
 * Created: 7/2/2018 12:10:21 PM
 *  Author: tleonard
 */ 


#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdlib.h>
#include "lcd.h"

class button: public Lcd {



private:
	pixpsn_t start;
	uint16_t xlen;
	uint16_t ylen;
	pixColor fg;
	pixColor bg;
	char *title;
	uint8_t mag;

public:
	button(pixpsn_t psn, pixColor f, pixColor b, char *text, uint8_t size);
	void draw(void);

};


#endif /* BUTTON.H_H_ */