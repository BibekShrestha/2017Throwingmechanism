/*
 * declarations.h
 *
 * Created: 2/18/2017 4:17:11 PM
 *  Author: Bibek Shrestha
 */


#ifndef DECLARATIONS_H_
#define DECLARATIONS_H_




#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "headers.h"

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

int Abs(int);
void initialise();

void bluetooth_check();




#endif /* DECLARATIONS_H_ */