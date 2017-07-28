			/**************************************************************
 *	Hitachi HD44780 LCD routines for AVR
 *						-Er.Sapin Bajracharya, October 2012
 *                      -edited by Rabin Giri
 **************************************************************
 *	The DDR bits associated with the various lines
 *	(LCD_EN, LCD_RS, LCD_D7..LCD_D0, LCD_DATA, LCD_CLK)
 *	must be set (output mode) in the main program 
 *	prior to calling the lcd_init() function.
 **************************************************************
 *		Uses 6 lines:
 *		LCD_EN		:	STROBE 			:	Strobe for LCD/LCD_EN
 *		LCD_RS		:	RegisterSelect	:	R/S line 
 *		LCD_D7...D4	:	Data lines		:	D7 to D4 lines 
 ***************************************************************

	
	Available functions:
	lcd_init();
		LCD initialization function
		MUST be called first, after associated DDR bits
		have been configured as outputs
	lcd_putch(char c);
		Displays supplied character at the current position
	lcd_puts(const char *s);
		Displays supplied string(character array) starting
		at current position
	lcd_clear();
		Clears and homes the display
	lcd_goto(uint8_t pos)
		Sets cursor location to given address (0-127)
	lcd_unum(uint16_t num)
		Displays unsigned integer (0 - 65535)
	lcd_unum3(uint16_t num)
		Displays unsigned number, upto 3 digits(0-999)
	lcd_unum_hex(uint16_t num)
		Displays 16bit number in 4digit Hex
	lcd_cmd(uint8_t c);
		Writes supplied byte to LCD with RS = 0 (command)
	lcd_dat(uint8_t c);
		Writes supplied byte to LCD with RS = 1 (data)
 *************************************************************/

#ifndef	LCD_H_
#define LCD_H_ 1


#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//
#define LCD_EN_PORT	PORTC
#define LCD_EN_PIN	PINC7
	
#define	LCD_RS_PORT PORTA		//R/S line
#define	LCD_RS_PIN	PINA4

#define LCD_D7_PORT PORTG
#define LCD_D7_PIN	PING1
#define LCD_D6_PORT PORTD
#define LCD_D6_PIN	PIND7
#define LCD_D5_PORT PORTC
#define LCD_D5_PIN	PINC1
#define LCD_D4_PORT PORTC
#define LCD_D4_PIN	PINC3



/*************************************************/
/*****DO NOT MODIFY ANYTHING BELOW THIS POINT*****/
/*************************************************/


#include <util/delay.h>

#include <avr/pgmspace.h>

#include <stdarg.h>
#include <stdlib.h> 


#define DDR(x) (*(&x - 1))      /* address of data direction register of port x */

/* instruction register bit positions, see HD44780U data sheet */
#define LCD_CLR               0      /* DB0: clear display                  */

#define LCD_HOME              1      /* DB1: return to home position        */

#define LCD_ENTRY_MODE        2      /* DB2: set entry mode                 */
#define LCD_ENTRY_INC         1      /*   DB1: 1=increment, 0=decrement     */
#define LCD_ENTRY_SHIFT       0      /*   DB2: 1=display shift on           */

#define LCD_ON                3      /* DB3: turn lcd/cursor on             */
#define LCD_ON_DISPLAY        2      /*   DB2: turn display on              */
#define LCD_ON_CURSOR         1      /*   DB1: turn cursor on               */
#define LCD_ON_BLINK          0      /*     DB0: blinking cursor ?          */

#define LCD_MOVE              4      /* DB4: move cursor/display            */
#define LCD_MOVE_DISP         3      /*   DB3: move display (0-> cursor) ?  */
#define LCD_MOVE_RIGHT        2      /*   DB2: move right (0-> left) ?      */

#define LCD_FUNCTION          5      /* DB5: function set                   */
#define LCD_FUNCTION_8BIT     4      /*   DB4: set 8BIT mode (0->4BIT mode) */
#define LCD_FUNCTION_2LINES   3      /*   DB3: two lines (0->one line)      */
#define LCD_FUNCTION_10DOTS   2      /*   DB2: 5x10 font (0->5x7 font)      */

#define LCD_CGRAM             6      /* DB6: set CG RAM address             */

#define LCD_DDRAM             7      /* DB7: set DD RAM address             */

#define LCD_BUSY              7      /* DB7: LCD is busy                    */

/* set entry mode: display shift on/off, dec/inc cursor move direction */
#define LCD_ENTRY_DEC            0x04   /* display shift off, dec cursor move dir */ //((1<<LCD_ENTRY_MODE) | (0<<LCD_ENTRY_INC) | (0<<LCD_ENTRY_SHIFT))
#define LCD_ENTRY_DEC_SHIFT      0x05   /* display shift on,  dec cursor move dir */ //((1<<LCD_ENTRY_MODE) | (0<<LCD_ENTRY_INC) | (1<<LCD_ENTRY_SHIFT))
#define LCD_ENTRY_INC_           0x06   /* display shift off, inc cursor move dir */ //((1<<LCD_ENTRY_MODE) | (1<<LCD_ENTRY_INC) | (0<<LCD_ENTRY_SHIFT))
#define LCD_ENTRY_INC_SHIFT      0x07   /* display shift on,  inc cursor move dir */ //((1<<LCD_ENTRY_MODE) | (1<<LCD_ENTRY_INC) | (1<<LCD_ENTRY_SHIFT))

/* display on/off, cursor on/off, blinking char at cursor position */
#define LCD_DISP_OFF             0x08   /* display off                            */ //((1<<LCD_ON) | (0<<LCD_ON_DISPLAY) | (0<<LCD_ON_CURSOR) | (0<<LCD_ON_BLINK))
#define LCD_DISP_ON              0x0C   /* display on, cursor off                 */ //((1<<LCD_ON) | (1<<LCD_ON_DISPLAY) | (0<<LCD_ON_CURSOR) | (0<<LCD_ON_BLINK))
#define LCD_DISP_ON_BLINK        0x0D   /* display on, cursor off, blink char     */ //((1<<LCD_ON) | (1<<LCD_ON_DISPLAY) | (0<<LCD_ON_CURSOR) | (1<<LCD_ON_BLINK))
#define LCD_DISP_ON_CURSOR       0x0E   /* display on, cursor on                  */ //((1<<LCD_ON) | (1<<LCD_ON_DISPLAY) | (1<<LCD_ON_CURSOR) | (0<<LCD_ON_BLINK))
#define LCD_DISP_ON_CURSOR_BLINK 0x0F   /* display on, cursor on, blink char      */ //((1<<LCD_ON) | (1<<LCD_ON_DISPLAY) | (1<<LCD_ON_CURSOR) | (1<<LCD_ON_BLINK))

/* move cursor/shift display */
#define LCD_MOVE_CURSOR_LEFT     0x10   /* move cursor left  (decrement)          */ //((1<<LCD_MOVE) | (0<<LCD_MOVE_DISP) | (0<<LCD_MOVE_RIGHT))
#define LCD_MOVE_CURSOR_RIGHT    0x14   /* move cursor right (increment)          */ //((1<<LCD_MOVE) | (0<<LCD_MOVE_DISP) | (1<<LCD_MOVE_RIGHT))
#define LCD_MOVE_DISP_LEFT       0x18   /* shift display left                     */ //((1<<LCD_MOVE) | (1<<LCD_MOVE_DISP) | (0<<LCD_MOVE_RIGHT))
#define LCD_MOVE_DISP_RIGHT      0x1C   /* shift display right                    */ //((1<<LCD_MOVE) | (1<<LCD_MOVE_DISP) | (1<<LCD_MOVE_RIGHT))

/* function set: set interface data length and number of display lines */
#define LCD_FUNCTION_4BIT_1LINE  0x20   /* 4-bit interface, single line, 5x7 dots */ //((1<<LCD_FUNCTION) | (0<<LCD_FUNCTION_8BIT) | (0<<LCD_FUNCTION_2LINES) | (0<<LCD_FUNCTION_10DOTS))
#define LCD_FUNCTION_4BIT_2LINES 0x28   /* 4-bit interface, dual line,   5x7 dots */ //((1<<LCD_FUNCTION) | (0<<LCD_FUNCTION_8BIT) | (1<<LCD_FUNCTION_2LINES) | (0<<LCD_FUNCTION_10DOTS))
#define LCD_FUNCTION_8BIT_1LINE  0x30   /* 8-bit interface, single line, 5x7 dots */ //((1<<LCD_FUNCTION) | (1<<LCD_FUNCTION_8BIT) | (0<<LCD_FUNCTION_2LINES) | (0<<LCD_FUNCTION_10DOTS))
#define LCD_FUNCTION_8BIT_2LINES 0x38   /* 8-bit interface, dual line,   5x7 dots */ //((1<<LCD_FUNCTION) | (0<<LCD_FUNCTION_8BIT) | (1<<LCD_FUNCTION_2LINES) | (0<<LCD_FUNCTION_10DOTS))


#define LCD_MODE_DEFAULT     ((1<<LCD_ENTRY_MODE) | (1<<LCD_ENTRY_INC) )


#define	LCD_STROBE()	((LCD_EN_PORT |= (1 << LCD_EN_PIN)),(LCD_EN_PORT &= ~(1 << LCD_EN_PIN)))



void lcd_cmd(uint8_t cmd);
void lcd_dat(uint8_t dat);


void lcd_clear(void);
void lcd_puts(const char * s);
void lcd_putch(char c);
void lcd_goto(unsigned char pos);
void lcd_goto(unsigned char pos);
void lcd_init();
void lcd_unum_hex(uint16_t num);
void lcd_unum3(uint8_t num);
void lcd_unum(uint16_t);
void lcd_num(int num, int radix = 10);
void lcd_gotoxy(unsigned char x, unsigned char y);
void Printf(char *fmt,...);

#endif /*LCD_H_*/