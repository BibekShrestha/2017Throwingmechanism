
/*
 * DMTM New code.cpp
 *
 * Created: 6/5/2017 7:09:26 PM
 * Author : Bibek Shrestha
 */ 




#include "declarations.h"
#include "headers.h"
#include "MotorBack.h"
#include "MotorFront.h"
#include "TMotor.h"
#include "MzMotorBack.h"
#include "MzMotorFront.h"
#include "MotorSide.h"


#include <util/delay.h>


#define DD_MGZ_LIMIT	F,7
#define DD_THR_LIMIT	B,2


MotorBack		BackMotor;
MotorFront		FrontMotor;
MotorSide		SideMotor;




TMotor			ThrowMotor;
MzMotorFront	MagazineFront;
MzMotorBack		MagazineBack;

unsigned char rx = 0;
unsigned char Rx_Buffer = 0x00;

		bool gLimitFlag   = false;
		bool gChangeFlag  = false;
		
		bool TempReturn1  = false;
		bool TempReturn2  = false;
volatile bool StartFlag  = false;

long int RPM;

char gStatus;
char gPostion;


int main(void)
{
	
	PULLUP_ON(DD_MGZ_LIMIT);


	if(1)
	{
		char Count1;
		MzMotorBack  PreBackMagazine;
		MzMotorFront PreFrontMagazine;
		PreFrontMagazine.Initialise();
		PreBackMagazine.Initialise();
		ThrowMotor.Initialise();


		PreFrontMagazine.PreMoveD();
		PreBackMagazine.PreMoveD();
		ThrowMotor.Operate(1);

		while(1)
		{
			if(!READ(DD_MGZ_LIMIT))
			{
				++Count1;
			}
			if(Count1)
			{
				PreBackMagazine.StopMotor();
				PreFrontMagazine.StopMotor();
				break;
			}
			ThrowMotor.Operate(0);
		}
		
	}


	bool MotorA_Return = false;
	bool MotorB_Return = false;
	bool MotorS_Return = false;

	
	initialise();

	BackMotor.Initialise();
	FrontMotor.Initialise();
	


	

	
	MagazineFront.Initialise();
	MagazineBack.Initialise();
	SideMotor.Initialise();

	BackMotor.StopMotor();
	FrontMotor.StopMotor();
	SideMotor.StopMotor();

	

	ThrowMotor.StopMotor();

	sei();
	bluetooth_check();
	
	while(1)
	{
	
		
		gLimitFlag = ThrowMotor.LimitFlag;
		gChangeFlag = ThrowMotor.ChangeFlag;
	
		gStatus = ThrowMotor.Status;
		gPostion = ThrowMotor.Position; 

		

		if(uart3_available())
		{	
			Rx_Buffer = uart3_getc();
		}
		
			
			
		lcd_gotoxy(0,0);
		lcd_num(BackMotor.Controller.setpoint);
		lcd_putch(' ');
		
		lcd_num(BackMotor.RPM);

		lcd_gotoxy(0,1);
		lcd_num(ThrowMotor.Position);
		lcd_putch(' ');
		lcd_num(ThrowMotor.Status);
		lcd_putch(' ');
		lcd_num(SideMotor.OCR);
		lcd_putch(' ');

		if(Rx_Buffer)
		{
			lcd_putch(Rx_Buffer);
		}	
		else
		{
			lcd_putch('0');
		}
		lcd_putch(' ');
		if(StartFlag)
		{
			lcd_putch( 65 );
		}
		else
			lcd_putch( 67 );




		RPM = SideMotor.RPM;
			
		if( Rx_Buffer == 'g')
		{
			if(!StartFlag)
			{
				MagazineBack.VirginityFlag = true;
				MagazineFront.VirginityFlag = true;
			}
			StartFlag = true;

			if(ThrowMotor.Position == HOMEPOSITION)
			{
				TempReturn1 = MagazineFront.Operate(true);
				TempReturn2 = MagazineBack.Operate(true);
			}
			


		}
		else if( Rx_Buffer == 's' || rx == 'S')
		{
			StartFlag = false;
			MagazineBack.StopMotor();
			MagazineFront.StopMotor();
			
			
		}
		
	

		if(StartFlag == true)
		{
	
			if(uart0_available())
			{
				rx = uart0_getc();
				if(rx == '.')
				{
					StartFlag = false;
				}
			}

			MotorA_Return = BackMotor.Operate(rx, Rx_Buffer);
			FrontMotor.SetOcrValue(BackMotor.Ocr);

			
			
			MotorS_Return = SideMotor.Operate(rx, Rx_Buffer, 0);


			MagazineFront.Operate(false);
			MagazineBack.Operate(false);

			ThrowMotor.Operate( Rx_Buffer );
			
			Rx_Buffer = 0;
			rx = 0;
			
			uart0_putc('2');
			uart0_putc(' ');
			uart0_putint(BackMotor.RPM);
			uart0_putc(' ');
			uart0_putint(BackMotor.Controller.setpoint);

			uart0_putc(' ');
			uart0_putint(BackMotor.Ocr);

			
			uart0_putc(' ');
			uart0_putint(FrontMotor.RPM);
			uart0_putc(' ');
			uart0_putint(FrontMotor.Controller.setpoint);
			
			uart0_putc(' ');
			uart0_putint(FrontMotor.Ocr);


			uart0_putc('\n');
			uart0_putc('\r');
			
		
		}
		else
		{
				
			BackMotor.StopMotor();
			FrontMotor.StopMotor();
			SideMotor.StopMotor();

			
			ThrowMotor.Operate( 0 );
			

			if(Rx_Buffer ==  'd')
			{
				
				MagazineBack.StopMotor();
				MagazineFront.StopMotor();

			}
			else if(Rx_Buffer == 'D')			
			{
				MagazineBack.MoveD();
				MagazineFront.MoveD();

			}
			
			Rx_Buffer = 0;
			rx = 0;
			
			uart0_putc('1');
			uart0_putc(' ');
			uart0_putint(BackMotor.RPM);
			uart0_putc(' ');
			uart0_putint(BackMotor.Controller.setpoint);

			uart0_putc(' ');
			uart0_putint(BackMotor.Ocr);

			
			uart0_putc(' ');
			uart0_putint(FrontMotor.RPM);
			uart0_putc(' ');
			uart0_putint(FrontMotor.Controller.setpoint);
			
			uart0_putc(' ');
			uart0_putint(FrontMotor.Ocr);


			uart0_putc('\n');
			uart0_putc('\r');

			
		}

		Rx_Buffer = 0;		
		rx = 0;	
		
	}
	
	
}










ISR(MOTORBACK_INT_vect)
{
	BackMotor.Count   = READVALUE(MOTORBACK_TCNT);
	MOTORBACK_TCNT = 0;
	BackMotor.IntFlag = true;
}


ISR(MOTORBACK_TIMER_OVERFLOW_VECT)
{
	BackMotor.RPM = 0;
}

ISR(SIDEMOTOR_INT_vect)
{
	SideMotor.Count   = READVALUE(SIDEMOTOR_TCNT);
	SIDEMOTOR_TCNT = 0;
	SideMotor.IntFlag = true;
}


ISR(SIDEMOTOR_TIMER_OVERFLOW_VECT)
{
	//SideMotor.RPM = 0;
}



ISR(MOTORFRONT_TIMER_OVERFLOW_VECT)
{
	FrontMotor.RPM = 0;
}


ISR(MOTORFRONT_INT_vect)
{
	FrontMotor.Count   = READVALUE(MOTORFRONT_TCNT);
	MOTORFRONT_TCNT = 0;
	FrontMotor.IntFlag = true;
}


ISR(PID_TIMER_OVERFLOW_vect)
{
	++BackMotor.Controller.timer;
	++FrontMotor.Controller.timer;
}


/*
ISR(FL_INT_vect)
{	

	ThrowMotor.LimitFlag = READ(FL_INTPIN);
	ThrowMotor.ChangeFlag = true;

}
*/
/*
ISR(BL_INT_vect)
{	
	ThrowMotor.BackLimitFlag = !READ(BL_INTPIN);
}
*/


ISR(EN_FRONT_INT_vect)
{
	
	if(!READ(ENCODERFRONTB))
	{
		MagazineFront.Encoder.UpFlag = true;
		++MagazineFront.Encoder.Count;
	}
	else
	{
		MagazineFront.Encoder.UpFlag = false;
		--MagazineFront.Encoder.Count;
	}


}


ISR(EN_BACK_INT_vect)
{	
	if(READ(ENCODERBACKB))
	{
		MagazineBack.Encoder.UpFlag = true;
		++MagazineBack.Encoder.Count;
	}
	else
	{
		MagazineBack.Encoder.UpFlag = false;
		--MagazineBack.Encoder.Count;
	}

	

}







