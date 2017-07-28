/*
 * PID.h
 *
 * Created: 11/22/2016 7:53:47 AM
 *  Author: Rajesh
 */ 


#ifndef PID_H_
#define PID_H_




#ifndef F_CPU
#define F_CPU	16000000UL

#endif

#define SETPOINTSTEPPING 10

#include "headers.h"
#include <math.h>

//extern char buff[20];

class PID
{
	private:
	
	
	char a;
	
	int pTerm, iTerm, dTerm,lastRPM;
	int error;
	bool LowSpeed;
	

	public:
	uint8_t timer;
	int setpoint,lastOutput;
	float kp,ki,kd;
	void Initialise(void);
	void Inc_KP(void){kp+=0.01;};
	void Dcr_KP(void){kp-=0.01;};
	void Inc_KI(void){ki+=0.0001;};
	void Dcr_KI(void){ki-=0.0001;};
	void Inc_KD(void){kd+=0.05;};
	void Dcr_KD(void){kd-=0.05;};
	void Inc_Setpoint(void){setpoint += SETPOINTSTEPPING;};
	void Dcr_Setpoint(void){setpoint -= SETPOINTSTEPPING;};
	void Set_Setpoint(int val);
	void Set_PID(float KP,float KI, float KD);
	float Get_Kp(void);
	float Get_Ki(void);
	float Get_Kd(void);
	
	int Get_Setpoint(void);
	int Get_Pterm(void)	{return pTerm;};
	int Get_Iterm(void)	{return iTerm;};
	int Get_dTerm(void)	{return dTerm;};
	int Compute_PID(int input, bool LowFlag);
};

void inline limit (int &value, int minValue,int maxValue);
#endif /*PID_H_*/