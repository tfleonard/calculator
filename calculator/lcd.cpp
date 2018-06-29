/*
 * Notes:
 * This code uses the _getPixels assembly code in font.s to retrieve
 * a buffer containing the pixels for a specified character.  The FONT array
 * is stored in code space since this processor has very little ram.  This requires
 * the assembly code function which takes a character and a buffer and returns the pixels.
 *
 * 
 */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "clock.h"
#include "lcd.h"
#include "ST7735.h"
//#include <Arduino.h>
//#include "pins_arduino.h"

//
// static var instantiations
//
pixColor Lcd::fgColor;
pixColor Lcd::bgColor;
cursor_t Lcd::cursor;
uint8_t Lcd::lcdInitialized = 0;


Lcd::Lcd(void) {
	volatile uint8_t reg;

	if (lcdInitialized) {			// only init the lcd once
		return;
	}
	// set the LCD bits in PORTC to outputs
	reg = DDRD;
	reg |= LCD_DDRD;
	DDRD = reg;

	reg = DDRC;
	reg |= LCD_DDRC;
	DDRC = reg;

	reg = DDRB;
	reg |= LCD_DDRB;
	DDRB = reg;

	// set the LCD bits to default
	reg = PORTC;
	reg &= ~LCD_DDRC;		// turn off all lcd bits
	reg |= LCD_DFLT;
	PORTC = reg;
	fgColor = WHITE;
	bgColor = LTGREEN;
	cursor.col = 0;
	cursor.line = 0;
	reset();
	mem_access();
	Clock::delay(12);
  pump_reg();
  Clock::delay(12);
  sleep_out();
  Clock::delay(12);
	on();	
  Clock::delay(12);
  set_display_brightness(0xff); 
  Clock::delay(12); 
 	clrScrn();
	lcdInitialized = 1;
}


void Lcd::reset(void) {

	volatile uint8_t reg;

	reg = PORTC;
	reg &= ~LCD_nRST;		// set reset low
	PORTC = reg;
	usecDly(RST_LOW);		// reset pulse is 10 usec
	reg |= LCD_nRST;
	PORTC = reg;			// set reset high
	Clock::delay(13);		// wait more than 120 msec
}


void Lcd::sendCmd(uint8_t cmd) {

volatile uint8_t reg;

	reg = PORTC;
	reg = reg & ~(LCD_nCS | LCD_A0);
	PORTC = reg;				// CS, A0 low
	
	// first send cmd
	sendByte(cmd);
}


void Lcd::sendData(uint8_t *buf, uint8_t cnt) {

volatile uint8_t reg = PORTC;
  
  PORTC = reg | LCD_A0;   // A0 hi for data/param

  for (int i=0; i< cnt; i++) {
    sendByte(buf[i]);
  }
}


//
// assumes CS and A0 are set correctly before entry
//
void Lcd::sendByte(uint8_t b) {

volatile uint8_t regb = PORTB;
volatile uint8_t regd = PORTD;
volatile uint8_t regc = PORTC;
volatile uint8_t tregc = regc;

	regb &= ~LCD_DDRB;
	regb |= (b & LCD_DDRB);
	PORTB = regb;

	regd &= ~LCD_DDRD;
	regd |= (b & LCD_DDRD);
	PORTD = regd;

	tregc = regc & ~LCD_nWR;
	PORTC = tregc;
	PORTC = regc;				// restore PORTC
}


void Lcd::on(void) {

  cli();
	sendCmd(CMD_DISPLAY_ON);
  cmpltCmd();
  sei();
}

	
void Lcd::off(void) {
  
  cli();
	sendCmd(CMD_DISPLAY_OFF);
  cmpltCmd();
  sei();
}


void Lcd::sleep_out(void) {

  cli();
  sendCmd(CMD_SLEEP_OUT);
  cmpltCmd();
  sei();
}


#define CMD_PUMP_REG  0xF7

void Lcd::pump_reg(void) {
  uint8_t buf = 0x20;
  cli();
  sendCmd(0xF7);
  sendData(&buf,1);
  cmpltCmd();
  sei();
}


void Lcd::set_display_brightness(uint8_t level) {
  uint8_t buf = level;
  cli();
#if 0
  sendCmd(0x53);  // CTRL DIsplay CMD
  buf = 0x20;     // enable brightness control
  sendData(&buf,1);
  cmpltCmd();
#endif

  buf = level;
  sendCmd(0x51);  // Write Brightness CMD
  sendData(&buf,1);
  cmpltCmd();
  
  sei();
}


void Lcd::mem_access(void) {
	uint8_t buf = LCD_MEM_CTL;
	cli();

	//
	// flip xy if needed
	//
	sendCmd(CMD_MEM_CTRL);
	sendData(&buf, 1);
	cmpltCmd();

	//
	// set pixel format to 18 bits per pixel
	//
	sendCmd(CMD_PIX_FORMAT);
	buf = FLAG_IFPF_18;
	sendData(&buf, 1);
	cmpltCmd();
	sei();
}


void Lcd::setFgColor(uint8_t r, uint8_t g, uint8_t b) {
	
	fgColor.r = r;
	fgColor.g = g;
	fgColor.b = b;
}

void Lcd::setFgColor(pixColor p) {
  
  fgColor = p;
}


void Lcd::setBgColor(uint8_t r, uint8_t g, uint8_t b) {
	
	bgColor.r = r;
	bgColor.g = g;
	bgColor.b = b;
}

void Lcd::setBgColor(pixColor p) {
  
  bgColor = p;
}


void Lcd::lcd_putchar(char c) {

uint16_t ys = cursor.line * CHAR_HEIGHT;
uint16_t ye = ys + CHAR_HEIGHT - 1;
uint16_t xs = cursor.col * CHAR_WIDTH;
uint16_t xe = xs + CHAR_WIDTH -1;
char pixBuf[CHAR_HEIGHT];
uint8_t buf[CHAR_WIDTH * 3];
uint8_t bufIndx;
  
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

	_getPixels(c, pixBuf); 

  sendCmd(CMD_MEM_WRITE);
  
	for (int i=0; i< CHAR_HEIGHT; i++) {
    bufIndx = 0;
		uint8_t pixels = pixBuf[i];
		for (int j=0; j< CHAR_WIDTH; j++) {
			if (pixels & 0x01) {								// font table assumes right to left scan 
				buf[bufIndx++] = fgColor.r;
				buf[bufIndx++] = fgColor.g;
				buf[bufIndx++] = fgColor.b;
			} else {
				buf[bufIndx++] = bgColor.r;
				buf[bufIndx++] = bgColor.g;
				buf[bufIndx++] = bgColor.b;
			}
			pixels >>= 1;		
		}
    sendData(buf,bufIndx);
	}
  cmpltCmd();
  cursor.col++;
  if (cursor.col == NUM_CHAR_PER_LINE) {
    cursor.col = 0;
    cursor.line++;
    if (cursor.line == NUM_LINES) {
      cursor.line = 0;
    }  
  }
  sei();
}

void Lcd::cmpltCmd(void) {
volatile uint8_t reg;
  
  reg = PORTC & ~LCD_DDRC;
  PORTC = reg | LCD_DFLT;   // CS, RST, nWR, nRD hi
}


void Lcd::clrEol(void) {
	uint8_t col = cursor.col;
	while (col++ < NUM_CHAR_PER_LINE) {
		lcd_putchar(' ');
	}
}


/*
 Draw character c at pixel position (y, x) with foreground color fc,
 *  background color bc, 
 *  node = 1 interpret y,x as a text line, col; 
 *       = 0 interpret y,x as a pixel line, col
 */
void Lcd::drawChar(char cc, uint8_t mode, uint8_t line, uint8_t col, pixColor fc, pixColor bc) {

uint8_t ys; 
uint8_t ye; 
uint8_t xs; 
uint8_t xe; 
char pixBuf[CHAR_HEIGHT];
uint8_t buf[CHAR_WIDTH * 3];
uint8_t bufIndx;

  if (mode) { 
    ys = cursor.line * CHAR_HEIGHT;
    xs = cursor.col * CHAR_WIDTH;
  } else {
    ys = line;
    xs = col;
  }
  ye = ys + CHAR_HEIGHT - 1;
  xe = xs + CHAR_WIDTH -1; 
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

  _getPixels(cc, pixBuf); 

  sendCmd(CMD_MEM_WRITE);
  
  for (int i=0; i< CHAR_HEIGHT; i++) {
    bufIndx = 0;
    uint8_t pixels = pixBuf[i];
    for (int j=0; j< CHAR_WIDTH; j++) {
      if (pixels & 0x01) {                // font table assumes right to left scan 
        buf[bufIndx++] = fc.r;
        buf[bufIndx++] = fc.g;
        buf[bufIndx++] = fc.b;
      } else {
        buf[bufIndx++] = bc.r;
        buf[bufIndx++] = bc.g;
        buf[bufIndx++] = bc.b;
      }
      pixels >>= 1;   
    }
    sendData(buf,bufIndx);
  }
  cmpltCmd();
  if (mode) {
    cursor.col++;
    if (cursor.col == NUM_CHAR_PER_LINE) {
      cursor.col = 0;
      cursor.line++;
      if (cursor.line == NUM_LINES) {
        cursor.line = 0;
      }  
    }
  }
  sei();
}


/*
 Draw character c at pixel position (y, x) with default foreground and background colors
 */
void Lcd::drawChar(char cc, uint8_t mode, uint8_t line, uint8_t col ) {

	drawChar(cc, mode, line, col, fgColor, bgColor);
}


void Lcd::clrScrn(void) {

uint8_t pixBuf[3];
uint8_t buf[4];

  pixColor oldBg = bgColor;
  bgColor = LTGREEN;
  
  cli();
  
  buf[0] = 0;
  buf[1] = 0;
  buf[2] = (NUM_PIX_PER_LINE >> 8) & 0xff;
  buf[3] = (NUM_PIX_PER_LINE & 0xff);
  sendCmd(CMD_COL_ADDR_SET);
  sendData(buf, 4);
  cmpltCmd();
  
  buf[0] = 0;
  buf[1] = 0;
  buf[2] = (NUM_PIX_LINES >> 8) & 0xff;
  buf[3] = (NUM_PIX_LINES & 0xff);
  sendCmd(CMD_ROW_ADDR_SET);
  sendData(buf, 4);
  cmpltCmd();

  pixBuf[0] = bgColor.r;
  pixBuf[1] = bgColor.g;
  pixBuf[2] = bgColor.b; 

  sendCmd(CMD_MEM_WRITE);
  
  for (int i=0; i< NUM_PIX_LINES; i++) {
    for (int j=0; j< NUM_PIX_PER_LINE; j++) {
      sendData(pixBuf,3);
    }
  }
  sei();
	
	cursor.line = 0;
	cursor.col = 0;
  bgColor = oldBg;
}


void Lcd::puts(const char *p) {
	while (*p) {
		drawChar(*p++, 1, cursor.line, cursor.col);
	}	
}


void Lcd::puts(const char *p, pixColor c) {
	while (*p) {
		drawChar(*p++, 1, cursor.line, cursor.col, c, bgColor);
	}
}

void Lcd::puts(const char *p, pixColor fgc, pixColor bgc) {
	while (*p) {
		drawChar(*p++, 1, cursor.line, cursor.col, fgc, bgc);
	}
}


void Lcd::gotoxy(uint8_t line, uint8_t col) {	
	cursor.line = line;
	cursor.col = col;
}


//
// erase = 0, draw inverted space as cursor
// erase = 1, draw normal space as cursor
//
void Lcd::drawCursor(uint8_t erase) {
  if (erase) {
    drawChar(' ', 1, cursor.line, cursor.col, fgColor, bgColor);    
  } else {
  	drawChar(' ', 1, cursor.line, cursor.col, bgColor, fgColor);
  }
}




bool Lcd::getTouch(cursor_t *t) {

volatile uint8_t datb; 
volatile uint8_t ddrb;
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
	datb = PORTB;
	ddrb = DDRB;
	datc = PORTC;
	ddrc = DDRC;
  
  cli();
  
//	pinMode(XX1, OUTPUT);
	DDRC |= X1;
//	digitalWrite(XX1, LOW);    // top of screen
	PORTC &= ~X1;
  
//	pinMode(XX2, OUTPUT);
	DDRB |= X2;
//	digitalWrite(XX2, HIGH);    // top of screen
	PORTB |= X2;
 
//	digitalWrite(YY2,LOW);
	PORTB &= ~Y2;
//	pinMode (YY2, INPUT);
	DDRB &= ~Y2;

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
  t->col = (uint16_t)adcval; 

  // 
  // now get y
  // 
//	pinMode(YY1, OUTPUT);
	DDRC |= Y1;
//	digitalWrite(YY1, LOW);
	PORTC &= ~Y1;

//	pinMode(YY2, OUTPUT);
	DDRB |= Y2;
//	digitalWrite(YY2, HIGH);
	PORTB |= Y2;

//	pinMode(XX1, INPUT);
	DDRC &= ~X1;

//	digitalWrite(XX2,LOW);
	PORTB &= ~X2;

//	pinMode(XX2, INPUT);
	DDRB &= ~X2;

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
  t->line = (uint16_t)adcval;

//	digitalWrite(YY2, LO);
	PORTB &= ~Y2;

	PORTB = datb;
	DDRB = ddrb;
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

bool Lcd::detectTouch(void) {
volatile uint8_t datb = PORTB;
volatile uint8_t ddrb = DDRB; 
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
	DDRB &= ~Y2;
	PORTB |= Y2;		

	// set X2 output
	DDRB |= (uint8_t)X2;
	// set X2 low
	PORTB &= ~X2;

	// set X1 output
	DDRC |= X1;
	// set x1 low
	PORTC &= ~X1;

	// give things time to settle down
	usecDly(USEC/2);

	// get the result
	result = PINC & Y1;

	PORTB &= ~Y2;				// remove pullup
	DDRB = ddrb;
	PORTB = datb;

	// now restore everything
	DDRC = ddrc;
	PORTC = datc;

	sei();
	if (result) {
		return false;
	}
	return true;
}





//
// fprintf wrapper
//
int lcd_putc(char c, FILE *f) {
	Lcd::lcd_putchar(c);
	return 0;
}


