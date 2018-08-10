/*
 * main.cpp - calculator main programm 
 *
 * Created: 6/19/2018 7:31:02 AM
 * Author : tleonard
 *
 *
 * History
 *
 *	Date		Version		Comments
 *	-----		-------		--------
 *	6-19-18		0.1			Initial made from dds5
 *	6-29-18		0.2			Changed touch functions to direct access 
 *
 */ 

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "clock.h"
#include "uart.h"
#include "led.h"
#include "lcd.h"
#include "graphics.h"
#include "sw.h"
#include "ElapsedTime.h"


using namespace std;

FILE uart_str = FDEV_SETUP_STREAM_CPP(uart_putc, uart_getc, _FDEV_SETUP_RW);
FILE lcd_str  = FDEV_SETUP_STREAM_CPP(lcd_putc, NULL, _FDEV_SETUP_WRITE);
FILE *lcdfp = &lcd_str;


int main(void) {

Clock *cl = new Clock();

Led *led = new Led();
Lcd *l = new Lcd();
Graphics *g = new Graphics();

int pass = 0;

pixColor  f = GREEN;
pixColor pix_green = GREEN;
pixColor  b = LTGREEN;
pixColor  fg_grey = GRAY;
pixColor pix_blk = BLACK;

char buf[16];

	uart_init();
	stdout = stdin = &uart_str;
	stderr = &uart_str;
	printf("main: Starting loop\n");
	
	ElapsedTime *et = new ElapsedTime(ONE_SEC);

	while(1) {
		cursor_t t;
		if (l->getTouch(&t)) {
			g->gotoxy(3,0);
			fprintf(lcdfp, "Touch  pass: %d", pass);
//printf("Touch pass: %d\n", pass);
        	g->gotoxy(15,0);
        	fprintf(lcdfp," x: %i  y: %i     ", t.col, t.line);        
//printf(" x: %i  y: %i\n", t.col, t.line);
		} else {
        	g->gotoxy(3,0);
        	fprintf(lcdfp, "No Touch         ");     
		}

		if (et->expired()) {

			if (l->detectTouch()) {
				g->gotoxy(2,0);
				fprintf(lcdfp, "Touch  pass: %d", pass);
			} else {
				g->gotoxy(2,0);
				fprintf(lcdfp, "No Touch        ");
			}

			g->gotoxy(19,0);
			fprintf(lcdfp, "Line 19  pass: %d", pass);
    
			sprintf(buf, "Pass %d\n", pass++);

			for (size_t i = 0; i < strlen(buf); i++) {
				g->draw2xChar(buf[i], 62, i*2*CHAR_WIDTH, f, b);
			}
		}
	}

#if 0
	delete l;
	delete cl;
	delete et;
	delete led;
#endif
}


