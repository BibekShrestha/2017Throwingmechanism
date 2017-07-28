/*
 * lcd.cpp
 *
 * Created: 12/23/2016 9:47:53 PM
 *  Author: Bibek Shrestha
 */

#include "lcd.h" 
static void lcd_write(uint8_t c)
{
	_delay_us(40);
	//MS nibble
	if(c & 0x80)
		LCD_D7_PORT |=  (1 << LCD_D7_PIN);
	else
		LCD_D7_PORT &= ~(1 << LCD_D7_PIN);
	
	if(c & 0x40)
		LCD_D6_PORT |=  (1 << LCD_D6_PIN);
	else
		LCD_D6_PORT &= ~(1 << LCD_D6_PIN);
	
	if(c & 0x20)
		LCD_D5_PORT |=  (1 << LCD_D5_PIN);
	else
		LCD_D5_PORT &= ~(1 << LCD_D5_PIN);
		
	if(c & 0x10)
		LCD_D4_PORT |=  (1 << LCD_D4_PIN);
	else
		LCD_D4_PORT &= ~(1 << LCD_D4_PIN);
	
	LCD_STROBE();
	
	//LS nibble
	if(c & 0x08)
		LCD_D7_PORT |=  (1 << LCD_D7_PIN);
	else
		LCD_D7_PORT &= ~(1 << LCD_D7_PIN);
		
	if(c & 0x04)
		LCD_D6_PORT |=  (1 << LCD_D6_PIN);
	else
		LCD_D6_PORT &= ~(1 << LCD_D6_PIN);
		
	if(c & 0x02)
		LCD_D5_PORT |=  (1 << LCD_D5_PIN);
	else
		LCD_D5_PORT &= ~(1 << LCD_D5_PIN);
		
	if(c & 0x01)
		LCD_D4_PORT |=  (1 << LCD_D4_PIN);
	else
		LCD_D4_PORT &= ~(1 << LCD_D4_PIN);
	
	LCD_STROBE();
}

//Write command to LCD
void lcd_cmd(uint8_t cmd)
{
	LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
	lcd_write(cmd);
}

//Write character data to LCD
void lcd_dat(uint8_t dat)
{
	LCD_RS_PORT |= (1 << LCD_RS_PIN);
	lcd_write(dat);
}


/*
 * 	Clear and home the LCD
 */

void lcd_clear(void)
{
	LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
	lcd_write(1<<LCD_CLR);
	_delay_ms(2);
}

/* write a string of chars to the LCD */

void lcd_puts(const char * s)
{
	LCD_RS_PORT |= (1 << LCD_RS_PIN);	// write characters
	while(*s)
		lcd_write(*s++);
}

#define lcd_puts_P(__s)         lcd_puts_p(PSTR(__s))

/* print string from program memory on lcd (no auto linefeed) */

void lcd_puts_p(const char *progmem_s)
{
    register char c;
	
	LCD_RS_PORT |= (1 << LCD_RS_PIN);	// write characters
    while ( (c = pgm_read_byte(progmem_s++)) ) 
        lcd_write(c);
}
/* write one character to the LCD */

void lcd_putch(char c)
{
	LCD_RS_PORT |= (1 << LCD_RS_PIN);	// write characters
	lcd_write( c );
}

/*
 * Go to the specified position
 */

void lcd_goto(unsigned char pos)
{
	LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
	lcd_write(0x80|pos);
}
	


void lcd_init()
{
	DDR(LCD_RS_PORT) |= (1 << LCD_RS_PIN);
	DDR(LCD_EN_PORT) |= (1 << LCD_EN_PIN);
	
	DDR(LCD_D7_PORT) |= (1 << LCD_D7_PIN);
	DDR(LCD_D6_PORT) |= (1 << LCD_D6_PIN);
	DDR(LCD_D5_PORT) |= (1 << LCD_D5_PIN);
	DDR(LCD_D4_PORT) |= (1 << LCD_D4_PIN);
	
	LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
	LCD_EN_PORT &= ~(1 << LCD_EN_PIN);
	
	_delay_ms(15);	// wait 15mSec after power applied,

	LCD_D4_PORT |= (1 << LCD_D4_PIN);//0x3 & 0x01;				//bit0 000X
	LCD_D5_PORT |= (1 << LCD_D5_PIN);//(0x3>>1) & 0x01;		//bit1 00XY -> 000X
	LCD_D6_PORT &= ~(1 << LCD_D6_PIN);//(0x3>>2) & 0x01;		//bit2 0XYZ -> 000X
	LCD_D7_PORT &= ~(1 << LCD_D7_PIN);//(0x3>>3) & 0x01;		//bit3 XYZW -> 000X
	
	LCD_STROBE();
	_delay_ms(5);
	LCD_STROBE();
	_delay_us(200);
	LCD_STROBE();
	_delay_us(200);
	
	// Four bit mode 
	LCD_D4_PORT &= ~(1 << LCD_D4_PIN);	//2 & 0x01
	LCD_D5_PORT |=  (1 << LCD_D5_PIN);	//(2>>1) & 0x01
	LCD_D6_PORT &= ~(1 << LCD_D6_PIN);	//(2>>2) & 0x01
	LCD_D7_PORT &= ~(1 << LCD_D7_PIN);	//(2>>3) & 0x01
	
	LCD_STROBE();

	lcd_write(0x28);		// Set interface length: nibblemode, 2line, 5x7dot
	lcd_write(0b00001100);	// Display On, Cursor Off, Cursor Blink off
	lcd_clear();			// Clear screen
	lcd_write(0x6);			// Set entry Mode : increment, displayShiftOff
}




const char PROGMEM chex[] =			{	'0', '1', '2', '3', '4', '5', '6', '7',
									'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'  };
const char PROGMEM chex_wBlanking[] = {	' ', '1', '2', '3', '4', '5', '6', '7',
									'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'  };

//without blanking of leading zeros
void lcd_unum_hex(uint16_t num)
{
	lcd_putch(pgm_read_byte(&chex[(num>>12)&0x0F]));
	lcd_putch(pgm_read_byte(&chex[(num>>8)&0x0F]));
	lcd_putch(pgm_read_byte(&chex[(num>>4)&0x0F]));
	lcd_putch(pgm_read_byte(&chex[num&0x0F]));
}

void lcd_unum_hex_wBlanking(uint16_t num)
{
	lcd_putch(pgm_read_byte(&chex_wBlanking[(num>>12)&0x0F]));
	lcd_putch(pgm_read_byte(&chex_wBlanking[(num>>8)&0x0F]));
	lcd_putch(pgm_read_byte(&chex_wBlanking[(num>>4)&0x0F]));
	lcd_putch(pgm_read_byte(&chex[num&0x0F]));
}

void lcd_unum3(uint8_t num)
{
	lcd_putch(num/100 + '0');
	lcd_putch((num%100)/10 + '0');
	lcd_putch(num%10 + '0');
}

void lcd_unum(uint16_t num)
{
    uint16_t bcd = 0;
    uint8_t bcd4 = 0;
    for(uint8_t i=16; i!=0; --i)
    {
	    if((bcd&0x000F) >= 0x5)
            bcd+=0x3;
        if((bcd&0x00F0) >= 0x50)
            bcd+=0x30;
        if((bcd&0x0F00) >= 0x500)
            bcd+=0x300;
        if((bcd&0xF000) >= 0x5000)
            bcd+=0x3000;
            
        bcd4 <<= 1;
        bcd4 += ((bcd&0x8000)?1:0);
        bcd <<= 1;
        bcd += ((num&(1<<(i-1)))?1:0);
    }
	
    lcd_putch((bcd4)?(chex[bcd4]):' ');    
    lcd_putch((!bcd4 && bcd<0x1000)	?' ':(pgm_read_byte(&chex[bcd>>12])));
    lcd_putch((!bcd4 && bcd<0x100)	?' ':(pgm_read_byte(&chex[(bcd>>8)&0x000F])));
    lcd_putch((!bcd4 && bcd<0x10)	?' ':(pgm_read_byte(&chex[(bcd>>4)&0x000F])));
    lcd_putch(pgm_read_byte(&chex[bcd&0x000F]));	
}	

void lcd_num( int num, int radix )
{
	char str[6];
	itoa( num, str, radix );
	lcd_puts( str );
}

void lcd_gotoxy( unsigned char x, unsigned char y )
{
	lcd_goto( y*64+x );
}

void Printf( char *fmt,... )
{
	va_list aptr;
	char *p, *sval;
	int ival;
	float fval;
	va_start( aptr, fmt );
	
	for( p=fmt; *p ; ++p )
	if( *p=='%' )
	switch( * ++p )
	{
		case 'b':
		ival = va_arg( aptr, int );
		lcd_num( ival, 2 );
		break;
		
		case 'x':
		ival = va_arg( aptr, int );
		lcd_num( ival, 16 );
		break;
		
		case 'd':
		ival = va_arg( aptr, int );
		lcd_num( ival, 10 );
		break;
		
		case 's':
		for( sval=va_arg(aptr, char*); *sval; ++sval )
		lcd_putch( *sval );
		break;
		
		case 'f':
		fval=va_arg( aptr, double );
		{
			int d,m;
			m=10000*(fval=fval-(d=fval));
			if( m<0 )
			m*=-1;
			lcd_num( d, 10 );
			lcd_putch('.');
			lcd_num( m, 10 );
		}
		break;
		
		case 'o':
		ival = va_arg( aptr, int );
		lcd_num( ival, 8 );
		break;
		
		
		default:
		lcd_putch(*p);
		
		
	}
	else if( *p=='\t' )
	lcd_putch(' ');
	else if( *p=='\n' )
	lcd_gotoxy( 0,1 );
	else
	lcd_putch( *p );
}






