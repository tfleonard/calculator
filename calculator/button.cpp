/*
 * button.cpp
 *
 * Created: 7/2/2018 12:34:44 PM
 *  Author: tleonard
 */ 

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "ST7735.h"
#include "graphics.h"
#include "button.h"

button::button(pixpsn_t ul, pixpsn_t lr, pixColor f, pixColor b, char *text, uint8_t size) {

	upperLeft = ul;
	lowerRight = lr;
	fg = f;
	bg = b;
	title = text;
	mag = size;
	xlen = lr.col - ul.col+1;
	ylen = lr.line - ul.line+1;

}



void button::draw(void) {

	uint16_t ys;
	uint16_t ye;
	uint16_t xs;
	uint16_t xe;
	uint8_t buf[CHAR_WIDTH * 3];
	uint8_t bufIndx;
	uint16_t len;

	ys = upperLeft.line;
	xs = upperLeft.col;
	ye = lowerRight.line;
	xe = lowerRight.col;
	
	cli();
	
	buf[0] = (xs >> 8) & 0xff;
	buf[1] = (xs & 0xff);
	buf[2] = (xe >> 8) & 0xff;
	buf[3] = (xe & 0xff);
	sendCmd(CMD_COL_ADDR_SET);
	sendData(buf, 4);
	cmpltCmd();
	
	buf[0] = (ys >> 8) & 0xff;
	buf[1] = (ys & 0xff);
	buf[2] = (ye >> 8) & 0xff;
	buf[3] = (ye & 0xff);
	sendCmd(CMD_ROW_ADDR_SET);
	sendData(buf, 4);
	cmpltCmd();

	sendCmd(CMD_MEM_WRITE);

	bufIndx = 0;
	//	initialize the line buffer
	for (int i=0; i< CHAR_WIDTH; i++) {
		buf[bufIndx++] = bg.r;
		buf[bufIndx++] = bg.g;
		buf[bufIndx++] = bg.b;
	}
	
	for (uint16_t i=0; i< ylen; i++) {
		len = xlen * 3;
		while(len) {
			if (len > bufIndx) {
				sendData(buf,bufIndx);
				len -= bufIndx;
				} else {
				sendData(buf,len);
				break;
			}
		}
	}
	cmpltCmd();
	sei();
}

