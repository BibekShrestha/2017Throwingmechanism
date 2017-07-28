/*
 * PID.cpp
 *
 * Created: 11/22/2016 8:04:16 AM
 * Author: Rajesh
 */ 



   #define PID_TCCRA		TCCR0A
   #define PID_TCCRB		TCCR0B
   #define PID_TCCRC		TCCR0C
   #define PID_TIMSK		TIMSK0
   #define PID_TCNT			TCNT0
   #define PID_CS0			CS00
   #define PID_CS1			CS01
   #define PID_CS2			CS02
   #define PID_TOIE			TOIE0



  #define VG_KP			1.07
  #define VG_KI			0.0135
  #define VG_KD			23.87
  
  #define MAX_OUTPUT	1400
  #define MIN_OUTPUT	-1400

  #include "PID.h"
  #include <stdlib.h>
  #include "uart.h"

void PID::Initialise(void)
{	
	 
	REGISTER_SET1(PID_TIMSK,PID_TOIE);
	REGISTER_SET1(PID_TCCRB,PID_CS2);
	setpoint=1500;	
	LowSpeed = true;
}


void PID::Set_Setpoint(int val)
{
	setpoint=val;
	//errSum=0;
}

void PID::Set_PID(float KP,float KI, float KD)
{
	kp=KP;
	ki=KI;
	kd=KD;	
}

float PID::Get_Kp()
{
	return kp;
}

float PID::Get_Ki()
{
	return ki;
}

float PID::Get_Kd()
{
	return kd;
}
int PID::Get_Setpoint()
{
	return setpoint;
}

int PID::Compute_PID(int currentRPM,bool LowFlag)
{
	static int output;
	static uint16_t TimeLimit = 0;
	TimeLimit = 0;
	if(LowFlag)
		{
			TimeLimit = 5;

		}

	if(timer >= TimeLimit)
	{
		error = setpoint-currentRPM;//speed error
		
		//if(abs(error) > 10)
		{
			pTerm = kp * error;
			
			iTerm += ki*error;
			
			dTerm = kd*(currentRPM-lastRPM);
			
			output=pTerm+iTerm-dTerm;
			
			lastRPM = currentRPM;
			

			lastOutput = output;
			
			
		}
		timer = 0;
	}
	return lastOutput;
	
}

void inline limit (int &value, int minValue,int maxValue)
{
	if (value>maxValue)
		value=maxValue;
	else if (value<minValue)
		value=minValue;
}