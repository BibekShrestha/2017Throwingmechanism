/*
 * Definitions.cpp
 *
 * Created: 2/18/2017 4:16:29 PM
 *  Author: Bibek Shrestha
 */ 


 #include "declarations.h"
 #include "uart.h"
 
 void wdt_init(void)
 {
	 MCUSR = 0;
	 wdt_disable();
	 return;
 }


 int Abs(int x)
 {
	if (x < 0)
		return 0 - x;
	return x;
 }

 void initialise()
 {
	
	uart0_init(UART_BAUD_SELECT(57600,F_CPU));
	//uart2_init(UART_BAUD_SELECT(38400,F_CPU));
	uart3_init(UART_BAUD_SELECT(57600,F_CPU));
	lcd_init();

 }



 void bluetooth_check()
 {
	uart0_puts("Why so serious?\n\rLet me put a smile on that face?\n\r");
 }

