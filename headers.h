/*
 * headers.h
 *
 * Created: 2/18/2017 4:33:57 PM
 *  Author: Bibek Shrestha
 */ 


#ifndef HEADERS_H_
#define HEADERS_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif


#ifndef __TMOTOR_LIMIT_STATUS__
#define __TMOTOR_LIMIT_STATUS__

#define MIDBACKPOSITION		5
#define HOMEPOSITION		6
#define MIDPOSITION			7
#define UPPOSITION			8
#define NONPOSITION			0


#define GOING_UP			1
#define GOING_MID			2
#define GOING_HOME			3
#define GOING_MID_BACK		4
#define NOTGOINGANYWHERE	0

#endif




#ifndef ENCODER_INTVECT_
#define ENCODER_INTVECT_


#define INT_ENCODERFRONTA_VECT	INT4_vect
#define INT_ENCODERBACKA_VECT	INT5_vect


#define ENCODERFRONTB	E,3
#define ENCODERBACKB	H,3

#endif  //ENCODER_INTVECT_




#ifndef __MOTOR_INTPIN__
#define __MOTOR_INTPIN__

#define MOTORFRONT_EICR		EICRA
#define MOTORFRONT_INT		INT3
#define MOTORFRONT_INTPIN	D,3
#define MOTORFRONT_ISC0		ISC30
#define MOTORFRONT_ISC1		ISC31
#define MOTORFRONT_INTF		INTF3
#define MOTORFRONT_INT_vect INT3_vect


#define MOTORBACK_EICR		EICRA
#define MOTORBACK_INT		INT2	
#define MOTORBACK_INTPIN	D,2
#define MOTORBACK_ISC0		ISC20
#define MOTORBACK_ISC1		ISC21
#define MOTORBACK_INTF		INTF2
#define MOTORBACK_INT_vect	INT2_vect



#define SIDEMOTOR_EICR		EICRA
#define SIDEMOTOR_INT		INT0
#define SIDEMOTOR_INTPIN	D,0
#define SIDEMOTOR_ISC0		ISC00
#define SIDEMOTOR_ISC1		ISC01
#define SIDEMOTOR_INTF		INTF0
#define SIDEMOTOR_INT_vect	INT0_vect


#define EN_FRONT_EICR		EICRB		
#define EN_FRONT_INT		INT5
#define EN_FRONT_INTPIN		E,5
#define EN_FRONT_ISC0		ISC50
#define EN_FRONT_ISC1		ISC51
#define EN_FRONT_INTF		INTF5
#define EN_FRONT_INT_vect	INT5_vect


#define EN_BACK_EICR			EICRB			
#define EN_BACK_INT				INT4	
#define EN_BACK_INTPIN			E,4		
#define EN_BACK_ISC0			ISC40	
#define EN_BACK_ISC1			ISC41	
#define EN_BACK_INTF			INTF4	
#define EN_BACK_INT_vect		INT4_vect


#endif  //__MOTOR_INTPIN__



#ifndef __LIMIT_INTERRUPTS__
#define __LIMIT_INTERRUPTS__
								
#define FL_EICR			EICRA
#define FL_INT			INT1
#define FL_INTPIN		D,1
#define FL_ISC0			ISC10
#define FL_ISC1			ISC11
#define FL_INTF			INTF1
#define FL_INT_vect 	INT1_vect


#endif  //__LIMIT_INTERRUPTS__






#ifndef __TIMER_OVERFLOW_VECTORS__
#define __TIMER_OVERFLOW_VECTORS__



#define SIDEMOTOR_TIMER_OVERFLOW_VECT	TIMER4_OVF_vect
#define SIDEMOTOR_TCNT					TCNT4


#define MOTORBACK_TIMER_OVERFLOW_VECT	TIMER3_OVF_vect
#define MOTORBACK_TCNT					TCNT3

#define MOTORFRONT_TIMER_OVERFLOW_VECT	TIMER1_OVF_vect
#define MOTORFRONT_TCNT					TCNT1

#define PID_TIMER						TIMER0
#define PID_TIMER_OVERFLOW_vect			TIMER0_OVF_vect
#define PID_TCNT						TCNT0


#endif	// __TIMER_OVERFLOW_VECTORS__




#define INPUT2(port,pin) DDR ## port &= ~_BV(pin)
#define OUTPUT2(port,pin) DDR ## port |= _BV(pin)
#define CLEAR2(port,pin) PORT ## port &= ~_BV(pin)
#define SET2(port,pin) PORT ## port |= _BV(pin)
#define TOGGLE2(port,pin) PORT ## port ^= _BV(pin)

#define READ2(port,pin) ((PIN ## port & _BV(pin))?1:0)

#define REGISTER_SET1( REGISTER, BIT1 ) REGISTER|=_BV( BIT1 )
#define REGISTER_SET2( REGISTER, BIT1, BIT2 ) REGISTER|=_BV( BIT1 )|_BV( BIT2 )
#define REGISTER_SET3( REGISTER, BIT1, BIT2, BIT3 ) REGISTER|=_BV( BIT1 )|_BV( BIT2 )|_BV( BIT3 )
#define REGISTER_SET4( REGISTER, BIT1, BIT2, BIT3, BIT4 ) REGISTER|=_BV( BIT1 )|_BV( BIT2 )|_BV( BIT3 )|_BV( BIT4 )
#define REGISTER_SET5( REGISTER, BIT1, BIT2, BIT3, BIT4, BIT5 ) REGISTER|=_BV( BIT1 )|_BV( BIT2 )|_BV( BIT3 )|_BV( BIT4 )|_BV( BIT5 )
#define REGISTER_SET6( REGISTER, BIT1, BIT2, BIT3, BIT4, BIT5, BIT6 ) REGISTER|=_BV( BIT1 )|_BV( BIT2 )|_BV( BIT3 )|_BV( BIT4 )|_BV( BIT5 )|_BV( BIT6 )
#define REGISTER_RESET( REGISTER,BIT ) REGISTER&=~_BV( BIT )
#define READVALUE(REGISTER) REGISTER
#define REGISTER_CLEAR(REGISTER) REGISTER = 0


#define INPUT(x) INPUT2(x)
#define OUTPUT(x) OUTPUT2(x)
#define CLEAR(x) CLEAR2(x)
#define SET(x) SET2(x)
#define TOGGLE(x) TOGGLE2(x)
#define READ(x) READ2(x)
#define PULLUP_ON(x) INPUT2(x); SET2(x)
#define PULLUP_OFF(x) INPUT2(x); CLEAR2(x)



#include <avr/io.h>
#include <avr/interrupt.h>

#include <avr/wdt.h>

#include "uart.h"
#include "Communication.h"
#include "lcd.h"












#endif /* HEADERS_H_ */