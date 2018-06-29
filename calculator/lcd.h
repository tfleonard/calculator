/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   lcd.h
 * Author: tleonard
 *
 */

#ifndef LCD_H
#define LCD_H

#include <stdlib.h>
#include "colors.h"

//
// uncomment the next line for the Transistor Tester LCD
// comment it out for the 1.8 LCD
//
//#define LCD_18	1		// new LCD display to UNO test bed (Red)
//#define LCD_TT	1		// transistor tester PINS
//#define LCD_TB	1		// tt lcd to UNO test bed

#define XMIN  79
#define XMAX  828
#define YMIN  117
#define YMAX  817

//
// uncomment the next line to flip the display
//
#define FLIP_XY	1


#ifndef FLIP_XY
#define LCD_MEM_CTL (FLAG_RGB_BGR | FLAG_MY_REV )
#define NUM_PIX_LINES		320
#define	NUM_PIX_PER_LINE	240
#else
#define LCD_MEM_CTL (FLAG_RGB_BGR | FLAG_MV_REV )
#define NUM_PIX_LINES		240
#define	NUM_PIX_PER_LINE	320
#endif

#define	CHAR_WIDTH	8
#define	CHAR_HEIGHT	12

#define NUM_BYTES_PER_PIXEL	3
#define NUM_BITS_PER_PIXEL (NUM_BYTES_PER_PIXEL * 8)
#define NUM_BYTES_PER_CHAR (CHAR_WIDTH * CHAR_HEIGHT * NUM_BYTES_PER_PIXEL)

#define NUM_CHAR_PER_LINE	(NUM_PIX_PER_LINE/CHAR_WIDTH)
#define	NUM_LINES		(NUM_PIX_LINES / CHAR_HEIGHT)


#define USEC	16

#define RST_LOW	(10 * USEC)	// 10 usec delay in cpu clocks) 	

//
// lcd signals
//
#ifdef LCD_18

#define LCD_SCK	(1 << 2)			// PD2
#define LCD_SDA	(1 << 3)			// PD3
#define LCD_A0	(1 << 4)			// PD4
#define LCD_nRST	(1 << 5)		// PD5
#define LCD_nCS		(1 << 6)		// PD6
#define LCD_DDR	(0x7C)				// PD2-PD6 outputs
#define LCD_DFLT	(LCD_nRST | LCD_nCS)  // nRST and nCS high
#define LCD_RST_LOW	2		// 

#elif LCD_TT

#define LCD_SCK	(1 << 2)			// PD2
#define LCD_SDA	(1 << 3)			// PD3
#define LCD_A0	(1 << 1)			// PD1
#define LCD_nRST	(1 << 0)		// PD0
#define LCD_nCS		(1 << 5)		// PD5
#define LCD_DDR	(0x2F)				// PD0-PD3,PD5 outputs
#define LCD_DFLT	(LCD_nRST | LCD_nCS)  // nRST and nCS high

#elif LCD_TB

#define LCD_SCK	(1 << 5)			// PD5
#define LCD_SDA	(1 << 4)			// PD4
#define LCD_A0	(1 << 3)			// PD3
#define LCD_nRST	(1 << 2)		// PD2
#define LCD_nCS		(1 << 6)		// PD6
#define LCD_DDR	(0x7C)				// PD2-PD6 outputs
#define LCD_DFLT	(LCD_nRST | LCD_nCS)  // nRST and nCS high

// 2.4 lcd shield  IL9341, 8 bit parallel
#else
#define LCD_nRST	(1 << 4)		// PC4
#define LCD_nCS		(1 << 3)		// PC3
#define LCD_A0		(1 << 2)		// PC2
#define LCD_nWR		(1 << 1)		// PC1
#define LCD_nRD		(1 << 0)		// PC0
#define LCD_DDRC	(0x1F)			// PC0-PC4 outputs
// data bus is pd7-pd2,pb1,pb0
#define LCD_DDRB	( (1 << PB0) | (1 << PB1))		// PB0, PB1 outputs
#define LCD_DDRD	(0xfc)												// PD2-PD7 outputs

#define LCD_DFLT	(LCD_nRST | LCD_nCS | LCD_nWR | LCD_nRD)  // nRST and nCS high, rd, wr low


//
// touch screen shares these pins with lcd: 
#define X2 (1 << PB0)	//	X2	-	PB0		D8
#define Y2 (1 << PB1)	//  Y2	-	PB1		D9
#define X1 (1 << PC2)	//	X1	-	PC2		A2, digital out, analog in
#define Y1 (1 << PC3)	//	Y1	-	PC3		A3, digital out, analog in

// Touchscreen connection:
#define YY1 A3  // need two analog inputs
#define XX1 A2  //
#define YY2 9   //
#define XX2 8   //

#define Y1A	PC3
#define X1A PC2

// ADMUX defines
#define ADCREF	(0x1 << 6)			// AREF pin
#define ADCLKDIF	0x07				// divide by 128

#endif


typedef struct _cursor {
	uint16_t line;		// y
	uint16_t col;		// x
} cursor_t;


typedef struct _pixpsn {
	uint16_t line;		// y
	uint16_t col;		// x
} pixpsn_t;

typedef struct _pixColor {
	uint8_t	r;
	uint8_t g;
	uint8_t b;
}pixColor;

//
// wrapper for fprintf
//
int lcd_putc(char c, FILE *f);


class Lcd {

private:
	static pixColor fgColor;
	static pixColor bgColor;
	static cursor_t cursor;
	static uint8_t lcdInitialized;

	static void sendCmd(uint8_t cmd);
	static void sendData(uint8_t *buf, uint8_t cnt);
	static void cmpltCmd(void);
	static void sendByte(uint8_t b);

// control commands
	void on(void);
	void off(void);
	void sleep_out(void);
	void mem_access(void);
	void reset(void);
  void pump_reg(void);

public:
	Lcd(void);

// char display commands
	static void lcd_putchar(char p);
	void puts(const char *p);
	void puts(const char *p, pixColor c);
	void puts(const char *p, pixColor fgc, pixColor bgc);
	void clrEol(void);
	void drawCursor(uint8_t erase);

// pixel commands
	void drawChar(char c, uint8_t mode, uint8_t line, uint8_t col);
	void drawChar(char cc, uint8_t mode, uint8_t line, uint8_t col, pixColor fc, pixColor bc);
	void clrScrn(void);
	void gotoxy(uint8_t line, uint8_t col);
	void setFgColor(uint8_t r, uint8_t g, uint8_t b);
  void setFgColor(pixColor p); 
	void setBgColor(uint8_t r, uint8_t g, uint8_t b);
  void setBgColor(pixColor p);
  void set_display_brightness(uint8_t level);
  
	// touch screen
	bool detectTouch(void);
	bool getTouch(cursor_t *t);

	void *operator new(size_t size) { return malloc(size);}
	void operator delete(void *ptr) { if (ptr) free(ptr); }


	friend class Graphics;


};

//
// all functions implemented in assembler must be demangled
//
#if defined (__cplusplus)
extern "C" {
  #endif

void _getPixels(char c, char *buf);

#if defined (__cplusplus)
}
#endif





#endif /* LCD_H */

