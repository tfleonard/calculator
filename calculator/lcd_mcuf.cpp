

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "clock.h"
#include "lcd.h"
#include "ST7735.h"

#include "lcd_mcuf.h"

Lcd_mcuf::Lcd_mcuf(void) {
	Lcd();
}

bool Lcd_mcuf::getTouch(cursor_t *t) {

	volatile uint8_t datd;
	volatile uint8_t ddrd;
	volatile uint8_t datc;
	volatile uint8_t ddrc;
	volatile int32_t adcval;
	int32_t deltax;
	int32_t deltay;

	#ifdef FLIPXY
	deltax = XMAX-XMIN;
	deltay = YMAX-YMIN;
	#else
	deltay = XMAX-XMIN;
	deltax = YMAX-YMIN;
	#endif

	if (!detectTouch()) {
		return false;
	}

	// save the current state of ports b and c
	datd = PORTD;
	ddrd = DDRD;
	datc = PORTC;
	ddrc = DDRC;
	
	cli();
	
	//	pinMode(XX1, OUTPUT);
	DDRC |= X1;
	//	digitalWrite(XX1, LOW);    // top of screen
	PORTC &= ~X1;
	
	//	pinMode(XX2, OUTPUT);
	DDRD |= X2;
	//	digitalWrite(XX2, HIGH);    // top of screen
	PORTD |= X2;
	
	//	digitalWrite(YY2,LOW);
	PORTD &= ~Y2;
	//	pinMode (YY2, INPUT);
	DDRD &= ~Y2;

	//	digitalWrite(YY1,LOW);
	PORTC &= ~Y1;
	//	pinMode(YY1,INPUT);
	DDRC &= ~Y1;

	//	adcval = analogRead(YY1);
	ADMUX = ADCREF | Y1A;		// AREF, select Y1
	ADCSRB = 0;
	ADCSRA = (1 << ADEN) | (1 << ADIF) | ADCLKDIF;	// enable a/d, clear int flag, div 128
	ADCSRA |= (1 << ADSC);
	while (!(ADCSRA & (1 << ADIF)));
	adcval = ADCW;

	//printf("adc col: %i\n", adcval);
	adcval -= XMIN;
	if (adcval < 0) {
		adcval = 0;
	}
	adcval = adcval * NUM_PIX_PER_LINE / (XMAX-XMIN);
	if (adcval > NUM_PIX_PER_LINE) {
		adcval = NUM_PIX_PER_LINE;
	}
	//printf("adj col: %i\n", adcval);
#ifdef TOUCH_00
	t->col = NUM_PIX_PER_LINE - (uint16_t)adcval; 
#else
	t->col = (uint16_t)adcval;
#endif

	//
	// now get y
	//
	//	pinMode(YY1, OUTPUT);
	DDRC |= Y1;
	//	digitalWrite(YY1, LOW);
	PORTC &= ~Y1;

	//	pinMode(YY2, OUTPUT);
	DDRD |= Y2;
	//	digitalWrite(YY2, HIGH);
	PORTD |= Y2;

	//	pinMode(XX1, INPUT);
	DDRC &= ~X1;

	//	digitalWrite(XX2,LOW);
	PORTD &= ~X2;

	//	pinMode(XX2, INPUT);
	DDRD &= ~X2;

	//	adcval = analogRead(XX1);
	ADMUX = ADCREF | X1A;		// AREF, select X1
	ADCSRA |= (1 << ADIF);	// clear interrupt flag
	ADCSRA |= (1 << ADSC);	// start conversion
	while (!(ADCSRA & (1 << ADIF)));
	adcval = ADCW;
	ADCSRA = 0;							// disable A/D

	//printf("raw row: %i\n", adcval);
	adcval -= YMIN;
	if (adcval < 0) {
		adcval = 0;
	}
	adcval = adcval * NUM_PIX_LINES / (YMAX-YMIN);
	if (adcval > NUM_PIX_LINES) {
		adcval = NUM_PIX_LINES;
	}
	//printf("adj row: %i\n", adcval);
	#if TOUCH_00
	t->line = NUM_PIX_LINES - (uint16_t)adcval;
	#else
	t->line = (uint16_t)adcval;
	#endif

	//	digitalWrite(YY2, LO);
	PORTD &= ~Y2;

	PORTD = datd;
	DDRD = ddrd;
	PORTC = datc;
	DDRC = ddrc;

	sei();
	return true;
}



//
// sequence:
//	1. set Y1 as input
//	2. set Y2 as input with pullup
//	3. set X1,X2 as outputs, LOW
//	4. read Y1 - 0 touch
//						 - 1 no touch
//	5. remove pullup on Y2
//	6. set Y1,Y2 as outputs
//	7. restore all registers to state on entry

bool Lcd_mcuf::detectTouch(void) {
	volatile uint8_t datd = PORTD;
	volatile uint8_t ddrd = DDRD;
	volatile uint8_t datc = PORTC;
	volatile uint8_t ddrc = DDRC;
	uint8_t result;

	cli();

	//
	// set Y1 as input
	//
	DDRC &= ~Y1;
	PORTC &= ~Y1;

	//
	// set Y2 as input, pullup
	DDRD &= ~Y2;
	PORTD |= Y2;

	// set X2 output
	DDRD |= (uint8_t)X2;
	// set X2 low
	PORTD &= ~X2;

	// set X1 output
	DDRC |= X1;
	// set x1 low
	PORTC &= ~X1;

	// give things time to settle down
	usecDly(USEC/2);

	// get the result
	result = PINC & Y1;

	PORTD &= ~Y2;				// remove pullup
	DDRD = ddrd;
	PORTD = datd;

	// now restore everything
	DDRC = ddrc;
	PORTC = datc;

	sei();
	if (result) {
		return false;
	}
	return true;
}


