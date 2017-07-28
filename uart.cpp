/*
 *  uart.cpp
 *
 *  Created:2017/05/06
 *  Edited By :Bibek Shrestha
 */ 
/*************************************************************************
Title:    Interrupt UART library with receive/transmit circular buffers
Author:   Peter Fleury <pfleury@gmx.ch>   http://jump.to/fleury
File:     $Id: uart.c,v 1.6.2.1 2007/07/01 11:14:38 peter Exp $
Software: AVR-GCC 4.1, AVR Libc 1.4.6 or higher
Hardware: any AVR with built-in UART, 
License:  GNU General Public License 
          
DESCRIPTION:
    An interrupt is generated when the UART has finished transmitting or
    receiving a byte. The interrupt handling routines use circular buffers
    for buffering received and transmitted data.
    
    The UART_RX_BUFFER_SIZE and UART_TX_BUFFER_SIZE variables define
    the buffer size in bytes. Note that these variables must be a 
    power of 2.
    
USAGE:
    Refere to the header file uart.h for a description of the routines. 
    See also example test_uart.c.

NOTES:
    Based on Atmel Application Note AVR306
                    
LICENSE:
    Copyright (C) 2006 Peter Fleury

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
                        
*************************************************************************/

/************************************************************************
uart_available, uart_flush, uart1_available, and uart1_flush functions
were adapted from the Arduino HardwareSerial.h library by Tim Sharpe on 
11 Jan 2009.  The license info for HardwareSerial.h is as follows:

  HardwareSerial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  Modified 23 November 2006 by David A. Mellis
************************************************************************/

/************************************************************************
Changelog for modifications made by Tim Sharpe, starting with the current
  library version on his Web site as of 05/01/2009. 

Date        Description
=========================================================================
05/11/2009  Changed all existing UARTx_RECEIVE_INTERRUPT and UARTx_TRANSMIT_INTERRUPT
              macros to use the "_vect" format introduced in AVR-Libc
			  v1.4.0.  Had to split the 3290 and 6490 out of their existing
			  macro due to an inconsistency in the UART0_RECEIVE_INTERRUPT 
			  vector name (seems like a typo: USART_RX_vect for the 3290/6490
			  vice USART0_RX_vect for the others in the macro).
			Verified all existing macro register names against the device
			  header files in AVR-Libc v1.6.6 to catch any inconsistencies.
05/12/2009  Added support for 48P, 88P, 168P, and 328P by adding them to the
               existing 48/88/168 macro.
			Added Arduino-style available() and flush() functions for both
			supported UARTs.  Really wanted to keep them out of the library, so
			that it would be as close as possible to Peter Fleury's original
			library, but has scoping issues accessing internal variables from
			another program.  Go C!
05/13/2009  Changed Interrupt Service Routine label from the old "SIGNAL" to
               the "ISR" format introduced in AVR-Libc v1.4.0.

************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "communication.h"
#include "uart.h"


/*
 *  constants and macros
 */

/* size of RX/TX buffers */
#define UART_RX_BUFFER_MASK ( UART_RX_BUFFER_SIZE - 1)
#define UART_TX_BUFFER_MASK ( UART_TX_BUFFER_SIZE - 1)

#if ( UART_RX_BUFFER_SIZE & UART_RX_BUFFER_MASK )
#error RX buffer size is not a power of 2
#endif
#if ( UART_TX_BUFFER_SIZE & UART_TX_BUFFER_MASK )
#error TX buffer size is not a power of 2
#endif

#if defined(__AVR_AT90S2313__) \
 || defined(__AVR_AT90S4414__) || defined(__AVR_AT90S4434__) \
 || defined(__AVR_AT90S8515__) || defined(__AVR_AT90S8535__) \
 || defined(__AVR_ATmega103__)
 /* old AVR classic or ATmega103 with one UART */
 #define AT90_UART
 #define UART0_RECEIVE_INTERRUPT   UART_RX_vect
 #define UART0_TRANSMIT_INTERRUPT  UART_UDRE_vect
 #define UART0_STATUS   USR
 #define UART0_CONTROL  UCR
 #define UART0_DATA     UDR  
 #define UART0_UDRIE    UDRIE
#elif defined(__AVR_AT90S2333__) || defined(__AVR_AT90S4433__)
 /* old AVR classic with one UART */
 #define AT90_UART
 #define UART0_RECEIVE_INTERRUPT   UART_RX_vect
 #define UART0_TRANSMIT_INTERRUPT  UART_UDRE_vect
 #define UART0_STATUS   UCSRA
 #define UART0_CONTROL  UCSRB
 #define UART0_DATA     UDR 
 #define UART0_UDRIE    UDRIE
#elif  defined(__AVR_ATmega8__)  || defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__) \
  || defined(__AVR_ATmega323__)
  /* ATmega with one USART */
 #define ATMEGA_USART
 #define UART0_RECEIVE_INTERRUPT   USART_RXC_vect
 #define UART0_TRANSMIT_INTERRUPT  USART_UDRE_vect
 #define UART0_STATUS   UCSRA
 #define UART0_CONTROL  UCSRB
 #define UART0_DATA     UDR
 #define UART0_UDRIE    UDRIE
#elif  defined(__AVR_ATmega8515__) || defined(__AVR_ATmega8535__)
  /* ATmega with one USART */
 #define ATMEGA_USART
 #define UART0_RECEIVE_INTERRUPT   USART_RX_vect
 #define UART0_TRANSMIT_INTERRUPT  USART_UDRE_vect
 #define UART0_STATUS   UCSRA
 #define UART0_CONTROL  UCSRB
 #define UART0_DATA     UDR
 #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega163__) 
  /* ATmega163 with one UART */
 #define ATMEGA_UART
 #define UART0_RECEIVE_INTERRUPT   UART_RX_vect
 #define UART0_TRANSMIT_INTERRUPT  UART_UDRE_vect
 #define UART0_STATUS   UCSRA
 #define UART0_CONTROL  UCSRB
 #define UART0_DATA     UDR
 #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega162__) 
 /* ATmega with two USART */
 #define ATMEGA_USART0
 #define ATMEGA_USART1
 #define UART0_RECEIVE_INTERRUPT   USART0_RXC_vect
 #define UART1_RECEIVE_INTERRUPT   USART1_RXC_vect
 #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
 #define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect
 #define UART0_STATUS   UCSR0A
 #define UART0_CONTROL  UCSR0B
 #define UART0_DATA     UDR0
 #define UART0_UDRIE    UDRIE0
 #define UART1_STATUS   UCSR1A
 #define UART1_CONTROL  UCSR1B
 #define UART1_DATA     UDR1
 #define UART1_UDRIE    UDRIE1
#elif defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__) 
 /* ATmega with two USART */
 #define ATMEGA_USART0
 #define ATMEGA_USART1
 #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
 #define UART1_RECEIVE_INTERRUPT   USART1_RX_vect
 #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
 #define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect
 #define UART0_STATUS   UCSR0A
 #define UART0_CONTROL  UCSR0B
 #define UART0_DATA     UDR0
 #define UART0_UDRIE    UDRIE0
 #define UART1_STATUS   UCSR1A
 #define UART1_CONTROL  UCSR1B
 #define UART1_DATA     UDR1
 #define UART1_UDRIE    UDRIE1
#elif defined(__AVR_ATmega161__)
 /* ATmega with UART */
 #error "AVR ATmega161 currently not supported by this libaray !"
#elif defined(__AVR_ATmega169__) 
 /* ATmega with one USART */
 #define ATMEGA_USART
 #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
 #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
 #define UART0_STATUS   UCSRA
 #define UART0_CONTROL  UCSRB
 #define UART0_DATA     UDR
 #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega48__) ||defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__) || \
      defined(__AVR_ATmega48P__) ||defined(__AVR_ATmega88P__) || defined(__AVR_ATmega168P__) || \
      defined(__AVR_ATmega328P__) 
 /* TLS-Added 48P/88P/168P/328P */
 /* ATmega with one USART */
 #define ATMEGA_USART0
 #define UART0_RECEIVE_INTERRUPT   USART_RX_vect
 #define UART0_TRANSMIT_INTERRUPT  USART_UDRE_vect
 #define UART0_STATUS   UCSR0A
 #define UART0_CONTROL  UCSR0B
 #define UART0_DATA     UDR0
 #define UART0_UDRIE    UDRIE0
#elif defined(__AVR_ATtiny2313__)
 #define ATMEGA_USART
 #define UART0_RECEIVE_INTERRUPT   USART_RX_vect 
 #define UART0_TRANSMIT_INTERRUPT  USART_UDRE_vect
 #define UART0_STATUS   UCSRA
 #define UART0_CONTROL  UCSRB
 #define UART0_DATA     UDR
 #define UART0_UDRIE    UDRIE
#elif defined(__AVR_ATmega329__) ||\
      defined(__AVR_ATmega649__) ||\
      defined(__AVR_ATmega325__) ||defined(__AVR_ATmega3250__) ||\
      defined(__AVR_ATmega645__) ||defined(__AVR_ATmega6450__)
  /* ATmega with one USART */
  #define ATMEGA_USART0
  #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
  #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
  #define UART0_STATUS   UCSR0A
  #define UART0_CONTROL  UCSR0B
  #define UART0_DATA     UDR0
  #define UART0_UDRIE    UDRIE0
#elif defined(__AVR_ATmega3290__) ||\
      defined(__AVR_ATmega6490__) 
  /* TLS-Separated these two from the previous group because of inconsistency in the USART_RX */
  /* ATmega with one USART */
  #define ATMEGA_USART0
  #define UART0_RECEIVE_INTERRUPT   USART_RX_vect
  #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
  #define UART0_STATUS   UCSR0A
  #define UART0_CONTROL  UCSR0B
  #define UART0_DATA     UDR0
  #define UART0_UDRIE    UDRIE0
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega640__)
#define ATMEGA_USART0
#define ATMEGA_USART1

#define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
#define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
#define UART0_STATUS   UCSR0A
#define UART0_CONTROL  UCSR0B
#define UART0_DATA     UDR0
#define UART0_UDRIE    UDRIE0


#define UART1_RECEIVE_INTERRUPT   USART1_RX_vect
#define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect
#define UART1_STATUS   UCSR1A
#define UART1_CONTROL  UCSR1B
#define UART1_DATA     UDR1
#define UART1_UDRIE    UDRIE1



#elif defined(__AVR_ATmega2560__)
/* ATmega with four USART */

#define ATMEGA_USART0
#define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
#define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
#define UART0_STATUS   UCSR0A
#define UART0_CONTROL  UCSR0B
#define UART0_DATA     UDR0
#define UART0_UDRIE    UDRIE0


#define ATMEGA_USART1
#define UART1_RECEIVE_INTERRUPT   USART1_RX_vect
#define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect
#define UART1_STATUS   UCSR1A
#define UART1_CONTROL  UCSR1B
#define UART1_DATA     UDR1
#define UART1_UDRIE    UDRIE1


#define ATMEGA_USART2
#define UART2_RECEIVE_INTERRUPT   USART2_RX_vect
#define UART2_TRANSMIT_INTERRUPT  USART2_UDRE_vect
#define UART2_STATUS   UCSR2A
#define UART2_CONTROL  UCSR2B
#define UART2_DATA     UDR2
#define UART2_UDRIE    UDRIE2


#define ATMEGA_USART3
#define UART3_RECEIVE_INTERRUPT   USART3_RX_vect
#define UART3_TRANSMIT_INTERRUPT  USART3_UDRE_vect
#define UART3_STATUS   UCSR3A
#define UART3_CONTROL  UCSR3B
#define UART3_DATA     UDR3
#define UART3_UDRIE    UDRIE3


#elif defined(__AVR_ATmega644__)
/* ATmega with one USART */

#define ATMEGA_USART0
#define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
#define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
#define UART0_STATUS   UCSR0A
#define UART0_CONTROL  UCSR0B
#define UART0_DATA     UDR0
#define UART0_UDRIE    UDRIE0


#elif defined(__AVR_ATmega644__)
 /* ATmega with one USART */
 #define ATMEGA_USART0
 #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
 #define UART0_TRANSMIT_INTERRUPT  USART0_UDRE_vect
 #define UART0_STATUS   UCSR0A
 #define UART0_CONTROL  UCSR0B
 #define UART0_DATA     UDR0
 #define UART0_UDRIE    UDRIE0
#elif defined(__AVR_ATmega164P__) || defined(__AVR_ATmega324P__) || defined(__AVR_ATmega644P__)
 /* ATmega with two USART */
 #define ATMEGA_USART0
 #define ATMEGA_USART1
 #define UART0_RECEIVE_INTERRUPT   USART0_RX_vect
 #define UART1_RECEIVE_INTERRUPT   USART0_UDRE_vect
 #define UART0_TRANSMIT_INTERRUPT  USART1_RX_vect
 #define UART1_TRANSMIT_INTERRUPT  USART1_UDRE_vect
 #define UART0_STATUS   UCSR0A
 #define UART0_CONTROL  UCSR0B
 #define UART0_DATA     UDR0
 #define UART0_UDRIE    UDRIE0
 #define UART1_STATUS   UCSR1A
 #define UART1_CONTROL  UCSR1B
 #define UART1_DATA     UDR1
 #define UART1_UDRIE    UDRIE1
#else
 #error "no UART definition for MCU available"
#endif


/*
 *  module global variables
 */
static volatile unsigned char UART0_TxBuf[UART_TX_BUFFER_SIZE];
static volatile unsigned char UART0_RxBuf[UART_RX_BUFFER_SIZE];
static volatile unsigned char UART0_TxHead;
static volatile unsigned char UART0_TxTail;
static volatile unsigned char UART0_RxHead;
static volatile unsigned char UART0_RxTail;
static volatile unsigned char UART0_LastRxError;

#if defined( ATMEGA_USART1 ) && defined(USING_UART1)
static volatile unsigned char UART1_TxBuf[UART_TX_BUFFER_SIZE];
static volatile unsigned char UART1_RxBuf[UART_RX_BUFFER_SIZE];
static volatile unsigned char UART1_TxHead;
static volatile unsigned char UART1_TxTail;
static volatile unsigned char UART1_RxHead;
static volatile unsigned char UART1_RxTail;
static volatile unsigned char UART1_LastRxError;
#endif

#if defined( ATMEGA_USART2 ) && defined( USING_UART2)
static volatile unsigned char UART2_TxBuf[UART_TX_BUFFER_SIZE];
static volatile unsigned char UART2_RxBuf[UART_RX_BUFFER_SIZE];
static volatile unsigned char UART2_TxHead;
static volatile unsigned char UART2_TxTail;
static volatile unsigned char UART2_RxHead;
static volatile unsigned char UART2_RxTail;
static volatile unsigned char UART2_LastRxError;
#endif


#if defined( ATMEGA_USART3 ) && defined(USING_UART3)
static volatile unsigned char UART3_TxBuf[UART_TX_BUFFER_SIZE];
static volatile unsigned char UART3_RxBuf[UART_RX_BUFFER_SIZE];
static volatile unsigned char UART3_TxHead;
static volatile unsigned char UART3_TxTail;
static volatile unsigned char UART3_RxHead;
static volatile unsigned char UART3_RxTail;
static volatile unsigned char UART3_LastRxError;
#endif



ISR(UART0_RECEIVE_INTERRUPT)
/*************************************************************************
Function: UART Receive Complete interrupt
Purpose:  called when the UART has received a character
**************************************************************************/
{
	unsigned char tmphead;
	unsigned char data;
	unsigned char usr;
	unsigned char lastRxError;


	/* read UART status register and UART data register */
	usr = UART0_STATUS;
	data = UART0_DATA;

	/* */
#if defined( AT90_UART )
	lastRxError = (usr & (_BV(FE) | _BV(DOR)));
#elif defined( ATMEGA_USART )
	lastRxError = (usr & (_BV(FE) | _BV(DOR)));
#elif defined( ATMEGA_USART0 )
	lastRxError = (usr & (_BV(FE0) | _BV(DOR0)));
#elif defined ( ATMEGA_UART )
	lastRxError = (usr & (_BV(FE) | _BV(DOR)));
#endif

	/* calculate buffer index */
	tmphead = (UART0_RxHead + 1) & UART_RX_BUFFER_MASK;

	if (tmphead == UART0_RxTail) {
		/* error: receive buffer overflow */
		lastRxError = UART_BUFFER_OVERFLOW >> 8;
	}
	else {
		/* store new index */
		UART0_RxHead = tmphead;
		/* store received data in buffer */
		UART0_RxBuf[tmphead] = data;
	}
	UART0_LastRxError = lastRxError;
}


ISR(UART0_TRANSMIT_INTERRUPT)
/*************************************************************************
Function: UART Data Register Empty interrupt
Purpose:  called when the UART is ready to transmit the next byte
**************************************************************************/
{
	unsigned char tmptail;


	if (UART0_TxHead != UART0_TxTail) {
		/* calculate and store new buffer index */
		tmptail = (UART0_TxTail + 1) & UART_TX_BUFFER_MASK;
		UART0_TxTail = tmptail;
		/* get one byte from buffer and write it to UART */
		UART0_DATA = UART0_TxBuf[tmptail];  /* start transmission */
	}
	else {
		/* tx buffer empty, disable UDRE interrupt */
		UART0_CONTROL &= ~_BV(UART0_UDRIE);
	}
}


/*************************************************************************
Function: uart_init()
Purpose:  initialize UART and set baudrate
Input:    baudrate using macro UART_BAUD_SELECT()
Returns:  none
**************************************************************************/
void uart0_init(unsigned int baudrate)
{
	UART0_TxHead = 0;
	UART0_TxTail = 0;
	UART0_RxHead = 0;
	UART0_RxTail = 0;

#if defined( AT90_UART )
	/* set baud rate */
	UBRR = (unsigned char)baudrate;

	/* enable UART receiver and transmmitter and receive complete interrupt */
	UART0_CONTROL = _BV(RXCIE) | _BV(RXEN) | _BV(TXEN);

#elif defined (ATMEGA_USART)
	/* Set baud rate */
	if (baudrate & 0x8000)
	{
		UART0_STATUS = (1 << U2X0);  //Enable 2x speed 
		baudrate &= ~0x8000;
	}
	UBRRH = (unsigned char)(baudrate >> 8);
	UBRRL = (unsigned char)baudrate;

	/* Enable USART receiver and transmitter and receive complete interrupt */
	UART0_CONTROL = _BV(RXCIE) | (1 << RXEN) | (1 << TXEN);

	/* Set frame format: asynchronous, 8data, no parity, 1stop bit */
#ifdef URSEL
	UCSRC = (1 << URSEL) | (1 << UCSZ0) | ( 1 << UCSZ1);
#else
	UCSRC = (1 << UCSZ0) | ( 1 << UCSZ1);
#endif 

#elif defined (ATMEGA_USART0 )
	/* Set baud rate */
	if (baudrate & 0x8000)
	{
		UART0_STATUS = (1 << U2X0);  //Enable 2x speed 
		baudrate &= ~0x8000;
	}
	UBRR0H = (unsigned char)(baudrate >> 8);
	UBRR0L = (unsigned char)baudrate;

	/* Enable USART receiver and transmitter and receive complete interrupt */
	UART0_CONTROL = _BV(RXCIE0) | (1 << RXEN0) | (1 << TXEN0);

	/* Set frame format: asynchronous, 8data, no parity, 1stop bit */
#ifdef URSEL0
	UCSR0C = (1 << URSEL0) | (1 << UCSZ00) | ( 1 << UCSZ01);
#else
	UCSR0C = (1 << UCSZ00) | ( 1 << UCSZ01);
#endif 

#elif defined ( ATMEGA_UART )
	/* set baud rate */
	if (baudrate & 0x8000)
	{
		UART0_STATUS = (1 << U2X);  //Enable 2x speed 
		baudrate &= ~0x8000;
	}
	UBRRH = (unsigned char)(baudrate >> 8);
	UBRRL = (unsigned char)baudrate;

	/* Enable UART receiver and transmitter and receive complete interrupt */
	UART0_CONTROL = _BV(RXCIE) | (1 << RXEN) | (1 << TXEN);

#endif

}/* uart_init */


 /*************************************************************************
 Function: uart_getc()
 Purpose:  return byte from ringbuffer
 Returns:  lower byte:  received byte from ringbuffer
 higher byte: last receive error
 **************************************************************************/
unsigned int uart0_getc(void)
{
	unsigned char tmptail;
	unsigned char data;


	if (UART0_RxHead == UART0_RxTail) {
		return UART_NO_DATA;   /* no data available */
	}

	/* calculate /store buffer index */
	tmptail = (UART0_RxTail + 1) & UART_RX_BUFFER_MASK;
	UART0_RxTail = tmptail;

	/* get data from receive buffer */
	data = UART0_RxBuf[tmptail];

	return (UART0_LastRxError << 8) + data;

}/* uart_getc */


 /*************************************************************************
 Function: uart_putc()
 Purpose:  write byte to ringbuffer for transmitting via UART
 Input:    byte to be transmitted
 Returns:  none
 **************************************************************************/
void uart0_putc(unsigned char data)
{
	unsigned char tmphead;


	tmphead = (UART0_TxHead + 1) & UART_TX_BUFFER_MASK;

	while (tmphead == UART0_TxTail) {
		;/* wait for free space in buffer */
	}

	UART0_TxBuf[tmphead] = data;
	UART0_TxHead = tmphead;

	/* enable UDRE interrupt */
	UART0_CONTROL |= _BV(UART0_UDRIE);

}/* uart_putc */

/*************************************************************************
 Function: uart0_putint()
 Purpose:  transmit integer to UART0
 Input:    integer to be transmitted
 Returns:  none
 **************************************************************************/
void uart0_putint(int input)
{
	char buffer[15];
	itoa(input,buffer,10);
	char * s = buffer;
	while (*s)
	uart0_putc(*s++);

	}/* uart0_putint */

 /*************************************************************************
 Function: uart_puts()
 Purpose:  transmit string to UART
 Input:    string to be transmitted
 Returns:  none
 **************************************************************************/
void uart0_puts(const char *s)
{
	while (*s)
		uart0_putc(*s++);

}/* uart_puts */


 /*************************************************************************
 Function: uart_puts_p()
 Purpose:  transmit string from program memory to UART
 Input:    program memory string to be transmitted
 Returns:  none
 **************************************************************************/
void uart0_puts_p(const char *progmem_s)
{
	register char c;

	while ((c = pgm_read_byte(progmem_s++)))
		uart0_putc(c);

}/* uart_puts_p */



 /*************************************************************************
 Function: uart_available()
 Purpose:  Determine the number of bytes waiting in the receive buffer
 Input:    None
 Returns:  Integer number of bytes in the receive buffer
 **************************************************************************/
int uart0_available(void)
{
	return (UART_RX_BUFFER_MASK + UART0_RxHead - UART0_RxTail) % UART_RX_BUFFER_MASK;
}/* uart_available */



 /*************************************************************************
 Function: uart_flush()
 Purpose:  Flush bytes waiting the receive buffer.  Acutally ignores them.
 Input:    None
 Returns:  None
 **************************************************************************/
void uart0_flush(void)
{
	UART0_RxHead = UART0_RxTail;
}/* uart_flush */


 /*
 * these functions are only for ATmegas with two USART
 */
#if defined( ATMEGA_USART1 ) && defined (USING_UART1)

ISR(UART1_RECEIVE_INTERRUPT)
/*************************************************************************
Function: UART1 Receive Complete interrupt
Purpose:  called when the UART1 has received a character
**************************************************************************/
{
	unsigned char tmphead;
	unsigned char data;
	unsigned char usr;
	unsigned char lastRxError;


	/* read UART status register and UART data register */
	usr = UART1_STATUS;
	data = UART1_DATA;

	/* */
	lastRxError = (usr & (_BV(FE1) | _BV(DOR1)));

	/* calculate buffer index */
	tmphead = (UART1_RxHead + 1) & UART_RX_BUFFER_MASK;

	if (tmphead == UART1_RxTail) {
		/* error: receive buffer overflow */
		lastRxError = UART_BUFFER_OVERFLOW >> 8;
	}
	else {
		/* store new index */
		UART1_RxHead = tmphead;
		/* store received data in buffer */
		UART1_RxBuf[tmphead] = data;
	}
	UART1_LastRxError = lastRxError;
}


ISR(UART1_TRANSMIT_INTERRUPT)
/*************************************************************************
Function: UART1 Data Register Empty interrupt
Purpose:  called when the UART1 is ready to transmit the next byte
**************************************************************************/
{
	unsigned char tmptail;


	if (UART1_TxHead != UART1_TxTail) {
		/* calculate and store new buffer index */
		tmptail = (UART1_TxTail + 1) & UART_TX_BUFFER_MASK;
		UART1_TxTail = tmptail;
		/* get one byte from buffer and write it to UART */
		UART1_DATA = UART1_TxBuf[tmptail];  /* start transmission */
	}
	else {
		/* tx buffer empty, disable UDRE interrupt */
		UART1_CONTROL &= ~_BV(UART1_UDRIE);
	}
}


/*************************************************************************
Function: uart1_init()
Purpose:  initialize UART1 and set baudrate
Input:    baudrate using macro UART_BAUD_SELECT()
Returns:  none
**************************************************************************/
void uart1_init(unsigned int baudrate)
{
	UART1_TxHead = 0;
	UART1_TxTail = 0;
	UART1_RxHead = 0;
	UART1_RxTail = 0;


	/* Set baud rate */
	if (baudrate & 0x8000)
	{
		UART1_STATUS = (1 << U2X1);  //Enable 2x speed 
		baudrate &= ~0x8000;
	}
	UBRR1H = (unsigned char)(baudrate >> 8);
	UBRR1L = (unsigned char)baudrate;

	/* Enable USART receiver and transmitter and receive complete interrupt */
	UART1_CONTROL = _BV(RXCIE1) | (1 << RXEN1) | (1 << TXEN1);

	/* Set frame format: asynchronous, 8data, no parity, 1stop bit */
#ifdef URSEL1
	UCSR1C = (1 << URSEL1) | (1 << UCSZ10) | ( 1 << UCSZ11);
#else
	UCSR1C = (1 << UCSZ10) | ( 1 << UCSZ11);
#endif 
}/* uart_init */


 /*************************************************************************
 Function: uart1_getc()
 Purpose:  return byte from ringbuffer
 Returns:  lower byte:  received byte from ringbuffer
 higher byte: last receive error
 **************************************************************************/
unsigned int uart1_getc(void)
{
	unsigned char tmptail;
	unsigned char data;


	if (UART1_RxHead == UART1_RxTail) {
		return UART_NO_DATA;   /* no data available */
	}

	/* calculate /store buffer index */
	tmptail = (UART1_RxTail + 1) & UART_RX_BUFFER_MASK;
	UART1_RxTail = tmptail;

	/* get data from receive buffer */
	data = UART1_RxBuf[tmptail];

	return (UART1_LastRxError << 8) + data;

}/* uart1_getc */


 /*************************************************************************
 Function: uart1_putc()
 Purpose:  write byte to ringbuffer for transmitting via UART
 Input:    byte to be transmitted
 Returns:  none
 **************************************************************************/
void uart1_putc(unsigned char data)
{
	unsigned char tmphead;


	tmphead = (UART1_TxHead + 1) & UART_TX_BUFFER_MASK;

	while (tmphead == UART1_TxTail) {
		;/* wait for free space in buffer */
	}

	UART1_TxBuf[tmphead] = data;
	UART1_TxHead = tmphead;

	/* enable UDRE interrupt */
	UART1_CONTROL |= _BV(UART1_UDRIE);

}/* uart1_putc */


/*************************************************************************
 Function: uart1_putint()
 Purpose:  transmit integer to UART1
 Input:    integer to be transmitted
 Returns:  none
 **************************************************************************/
void uart1_putint(int input)
{
	char buffer[15];
	itoa(input,buffer,10);
	char * s = buffer;
	while (*s)
	uart1_putc(*s++);

	}/* uart1_putint */

 /*************************************************************************
 Function: uart1_puts()
 Purpose:  transmit string to UART1
 Input:    string to be transmitted
 Returns:  none
 **************************************************************************/
void uart1_puts(const char *s)
{
	while (*s)
		uart1_putc(*s++);

}/* uart1_puts */


 /*************************************************************************
 Function: uart1_puts_p()
 Purpose:  transmit string from program memory to UART1
 Input:    program memory string to be transmitted
 Returns:  none
 **************************************************************************/
void uart1_puts_p(const char *progmem_s)
{
	register char c;

	while ((c = pgm_read_byte(progmem_s++)))
		uart1_putc(c);

}/* uart1_puts_p */



 /*************************************************************************
 Function: uart1_available()
 Purpose:  Determine the number of bytes waiting in the receive buffer
 Input:    None
 Returns:  Integer number of bytes in the receive buffer
 **************************************************************************/
int uart1_available(void)
{
	return (UART_RX_BUFFER_MASK + UART1_RxHead - UART1_RxTail) % UART_RX_BUFFER_MASK;
}/* uart1_available */



 /*************************************************************************
 Function: uart1_flush()
 Purpose:  Flush bytes waiting the receive buffer.  Acutally ignores them.
 Input:    None
 Returns:  None
 **************************************************************************/
void uart1_flush(void)
{
	UART1_RxHead = UART1_RxTail;
}/* uart1_flush */

#endif

#if defined( ATMEGA_USART2 ) && defined(USING_UART2)

ISR(UART2_RECEIVE_INTERRUPT)
/*************************************************************************
Function: UART2 Receive Complete interrupt
Purpose:  called when the UART2 has received a character
**************************************************************************/
{
	unsigned char tmphead;
	unsigned char data;
	unsigned char usr;
	unsigned char lastRxError;


	/* read UART status register and UART data register */
	usr = UART2_STATUS;
	data = UART2_DATA;

	/* */
	lastRxError = (usr & (_BV(FE2) | _BV(DOR2)));

	/* calculate buffer index */
	tmphead = (UART2_RxHead + 1) & UART_RX_BUFFER_MASK;

	if (tmphead == UART2_RxTail) {
		/* error: receive buffer overflow */
		lastRxError = UART_BUFFER_OVERFLOW >> 8;
	}
	else {
		/* store new index */
		UART2_RxHead = tmphead;
		/* store received data in buffer */
		UART2_RxBuf[tmphead] = data;
	}
	UART2_LastRxError = lastRxError;
}


ISR(UART2_TRANSMIT_INTERRUPT)
/*************************************************************************
Function: UART2 Data Register Empty interrupt
Purpose:  called when the UART2 is ready to transmit the next byte
**************************************************************************/
{
	unsigned char tmptail;


	if (UART2_TxHead != UART2_TxTail) {
		/* calculate and store new buffer index */
		tmptail = (UART2_TxTail + 1) & UART_TX_BUFFER_MASK;
		UART2_TxTail = tmptail;
		/* get one byte from buffer and write it to UART */
		UART2_DATA = UART2_TxBuf[tmptail];  /* start transmission */
	}
	else {
		/* tx buffer empty, disable UDRE interrupt */
		UART2_CONTROL &= ~_BV(UART2_UDRIE);
	}
}


/*************************************************************************
Function: uart2_init()
Purpose:  initialize UART2 and set baudrate
Input:    baudrate using macro UART_BAUD_SELECT()
Returns:  none
**************************************************************************/
void uart2_init(unsigned int baudrate)
{
	UART2_TxHead = 0;
	UART2_TxTail = 0;
	UART2_RxHead = 0;
	UART2_RxTail = 0;


	/* Set baud rate */
	if (baudrate & 0x8000)
	{
		UART2_STATUS = (1 << U2X2);  //Enable 2x speed 
		baudrate &= ~0x8000;
	}
	UBRR2H = (unsigned char)(baudrate >> 8);
	UBRR2L = (unsigned char)baudrate;

	/* Enable USART receiver and transmitter and receive complete interrupt */
	UART2_CONTROL = _BV(RXCIE2) | (1 << RXEN2) | (1 << TXEN2);

	/* Set frame format: asynchronous, 8data, no parity, 2stop bit */
#ifdef URSEL2
	UCSR2C = (1 << URSEL2) | (1 << UCSZ20) | ( 1 << UCSZ21);
#else
	UCSR2C = (1 << UCSZ20) | ( 1 << UCSZ21);
#endif 
}/* uart_init */


 /*************************************************************************
 Function: uart2_getc()
 Purpose:  return byte from ringbuffer
 Returns:  lower byte:  received byte from ringbuffer
 higher byte: last receive error
 **************************************************************************/
unsigned int uart2_getc(void)
{
	unsigned char tmptail;
	unsigned char data;


	if (UART2_RxHead == UART2_RxTail) {
		return UART_NO_DATA;   /* no data available */
	}

	/* calculate /store buffer index */
	tmptail = (UART2_RxTail + 1) & UART_RX_BUFFER_MASK;
	UART2_RxTail = tmptail;

	/* get data from receive buffer */
	data = UART2_RxBuf[tmptail];

	return (UART2_LastRxError << 8) + data;

}/* uart2_getc */


 /*************************************************************************
 Function: uart2_putc()
 Purpose:  write byte to ringbuffer for transmitting via UART
 Input:    byte to be transmitted
 Returns:  none
 **************************************************************************/
void uart2_putc(unsigned char data)
{
	unsigned char tmphead;


	tmphead = (UART2_TxHead + 1) & UART_TX_BUFFER_MASK;

	while (tmphead == UART2_TxTail) {
		;/* wait for free space in buffer */
	}

	UART2_TxBuf[tmphead] = data;
	UART2_TxHead = tmphead;

	/* enable UDRE interrupt */
	UART2_CONTROL |= _BV(UART2_UDRIE);

}/* uart2_putc */

/*************************************************************************
 Function: uart2_putint()
 Purpose:  transmit integer to UART2
 Input:    integer to be transmitted
 Returns:  none
 **************************************************************************/
void uart2_putint(int input)
{
	char buffer[15];
	itoa(input,buffer,10);
	char * s = buffer;
	while (*s)
	uart2_putc(*s++);

	}/* uart2_putint */

 /*************************************************************************
 Function: uart2_puts()
 Purpose:  transmit string to UART2
 Input:    string to be transmitted
 Returns:  none
 **************************************************************************/
void uart2_puts(const char *s)
{
	while (*s)
		uart2_putc(*s++);

}/* uart2_puts */


 /*************************************************************************
 Function: uart2_puts_p()
 Purpose:  transmit string from program memory to UART2
 Input:    program memory string to be transmitted
 Returns:  none
 **************************************************************************/
void uart2_puts_p(const char *progmem_s)
{
	register char c;

	while ((c = pgm_read_byte(progmem_s++)))
		uart2_putc(c);

}/* uart2_puts_p */



 /*************************************************************************
 Function: uart2_available()
 Purpose:  Determine the number of bytes waiting in the receive buffer
 Input:    None
 Returns:  Integer number of bytes in the receive buffer
 **************************************************************************/
int uart2_available(void)
{
	return (UART_RX_BUFFER_MASK + UART2_RxHead - UART2_RxTail) % UART_RX_BUFFER_MASK;
}/* uart2_available */



 /*************************************************************************
 Function: uart2_flush()
 Purpose:  Flush bytes waiting the receive buffer.  Acutally ignores them.
 Input:    None
 Returns:  None
 **************************************************************************/
void uart2_flush(void)
{
	UART2_RxHead = UART2_RxTail;
}/* uart2_flush */

#endif


#if defined( ATMEGA_USART3 ) && defined(USING_UART3)

extern volatile unsigned char Rx_Buffer;

ISR(UART3_RECEIVE_INTERRUPT)
/*************************************************************************
Function: UART3 Receive Complete interrupt
Purpose:  called when the UART3 has received a character
**************************************************************************/
{
	unsigned char tmphead;
	unsigned char data;
	unsigned char usr;
	unsigned char lastRxError;


	/* read UART status register and UART data register */
	usr = UART3_STATUS;
	data = UART3_DATA;
	

	/* */
	lastRxError = (usr & (_BV(FE3) | _BV(DOR3)));

	/* calculate buffer index */
	tmphead = (UART3_RxHead + 1) & UART_RX_BUFFER_MASK;

	if (tmphead == UART3_RxTail) {
		/* error: receive buffer overflow */
		lastRxError = UART_BUFFER_OVERFLOW >> 8;
	}
	else {
		/* store new index */
		UART3_RxHead = tmphead;
		/* store received data in buffer */
		UART3_RxBuf[tmphead] = data;
	}
	UART3_LastRxError = lastRxError;
}


ISR(UART3_TRANSMIT_INTERRUPT)
/*************************************************************************
Function: UART3 Data Register Empty interrupt
Purpose:  called when the UART3 is ready to transmit the next byte
**************************************************************************/
{
	unsigned char tmptail;


	if (UART3_TxHead != UART3_TxTail) {
		/* calculate and store new buffer index */
		tmptail = (UART3_TxTail + 1) & UART_TX_BUFFER_MASK;
		UART3_TxTail = tmptail;
		/* get one byte from buffer and write it to UART */
		UART3_DATA = UART3_TxBuf[tmptail];  /* start transmission */
	}
	else {
		/* tx buffer empty, disable UDRE interrupt */
		UART3_CONTROL &= ~_BV(UART3_UDRIE);
	}
}


/*************************************************************************
Function: uart3_init()
Purpose:  initialize UART3 and set baudrate
Input:    baudrate using macro UART_BAUD_SELECT()
Returns:  none
**************************************************************************/
void uart3_init(unsigned int baudrate)
{
	UART3_TxHead = 0;
	UART3_TxTail = 0;
	UART3_RxHead = 0;
	UART3_RxTail = 0;


	/* Set baud rate */
	if (baudrate & 0x8000)
	{
		UART3_STATUS = (3 << U2X3);  //Enable 2x speed 
		baudrate &= ~0x8000;
	}
	UBRR3H = (unsigned char)(baudrate >> 8);
	UBRR3L = (unsigned char)baudrate;

	/* Enable USART receiver and transmitter and receive complete interrupt */
	UART3_CONTROL = _BV(RXCIE3) | (1 << RXEN3) | (1 << TXEN3);

	/* Set frame format: asynchronous, 8data, no parity, 1-stop bit */
#ifdef URSEL3
	UCSR3C = (1 << URSEL3) | (1 << UCSZ30) | ( 1 << UCSZ31);
#else
	UCSR3C = (1 << UCSZ30) | ( 1 << UCSZ31);
#endif 
}/* uart_init */


 /*************************************************************************
 Function: uart3_getc()
 Purpose:  return byte from ring-buffer
 Returns:  lower byte:  received byte from ring-buffer
 higher byte: last receive error
 **************************************************************************/
unsigned int uart3_getc(void)
{
	unsigned char tmptail;
	unsigned char data;


	if (UART3_RxHead == UART3_RxTail) {
		 return UART_NO_DATA;   /* no data available */
	}

	/* calculate /store buffer index */
	tmptail = (UART3_RxTail + 1) & UART_RX_BUFFER_MASK;
	UART3_RxTail = tmptail;

	/* get data from receive buffer */
	data = UART3_RxBuf[tmptail];

	return (UART3_LastRxError << 8) + data;

}/* uart3_getc */


 /*************************************************************************
 Function: uart3_putc()
 Purpose:  write byte to ringbuffer for transmitting via UART
 Input:    byte to be transmitted
 Returns:  none
 **************************************************************************/
void uart3_putc(unsigned char data)
{
	unsigned char tmphead;


	tmphead = (UART3_TxHead + 1) & UART_TX_BUFFER_MASK;

	while (tmphead == UART3_TxTail) {
		;/* wait for free space in buffer */
	}

	UART3_TxBuf[tmphead] = data;
	UART3_TxHead = tmphead;

	/* enable UDRE interrupt */
	UART3_CONTROL |= _BV(UART3_UDRIE);

}/* uart3_putc */

/*************************************************************************
 Function: uart3_putint()
 Purpose:  transmit integer to UART3
 Input:    integer to be transmitted
 Returns:  none
 **************************************************************************/
void uart3_putint(int input)
{
	char buffer[15];
	itoa(input,buffer,10);
	char * s = buffer;
	while (*s)
	uart3_putc(*s++);

	}/* uart3_putint */


 /*************************************************************************
 Function: uart3_puts()
 Purpose:  transmit string to UART3
 Input:    string to be transmitted
 Returns:  none
 **************************************************************************/
void uart3_puts(const char *s)
{
	while (*s)
		uart3_putc(*s++);

}/* uart3_puts */


 /*************************************************************************
 Function: uart3_puts_p()
 Purpose:  transmit string from program memory to UART3
 Input:    program memory string to be transmitted
 Returns:  none
 **************************************************************************/
void uart3_puts_p(const char *progmem_s)
{
	register char c;

	while ((c = pgm_read_byte(progmem_s++)))
		uart3_putc(c);

}/* uart3_puts_p */



 /*************************************************************************
 Function: uart3_available()
 Purpose:  Determine the number of bytes waiting in the receive buffer
 Input:    None
 Returns:  Integer number of bytes in the receive buffer
 **************************************************************************/
int uart3_available(void)
{
	return (UART3_RxHead - UART3_RxTail) != 0;
}/* uart3_available */



 /*************************************************************************
 Function: uart3_flush()
 Purpose:  Flush bytes waiting the receive buffer.  Acutally ignores them.
 Input:    None
 Returns:  None
 **************************************************************************/
void uart3_flush(void)
{
	UART3_RxHead = UART3_RxTail;
}/* uart3_flush */

#endif

