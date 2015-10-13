/**********************************************************************c
* Author: Marcin Dec
* Date: 31.01.2015
* FileName:        main.c
* Dependencies:    Header (.h) files if applicable, see below
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® C30 v3.00 or higher
*
************************************************************************/

#include <p30f6013A.h>
#include "Motors.h"
#include "Globals.h"
#include "Bluetooth.h"
#include <math.h>

// T2_PERIOD is PWM period in microseconds
//#define T2_PERIOD 50L
//#define T2_POST 1L
//#define T2_PR2 (T2_PERIOD)*(F_CY/T2_POST)/1000000L
//#define MICSTEPS 64
//#define PI 3.1415926535897932384626433832795F
//extern union MPU5060 Mpu5060;
//extern union PID Pid;

//extern int BroadcastAppend(unsigned char* frame);

_FOSC(CSW_FSCM_OFF & XT_PLL16);                      // no clock switching , primary clock XT with PLL x 16
_FBORPOR( MCLR_DIS); //  MCLR as reset pin
_FWDT(WDT_OFF);                               // watchdog disabled



/* Allocate memory for buffers and drivers	*/



long k;
long h = 10000;





int main(void)
{
	/* Configure Oscillator to operate the device at 40MHz.
	 * Fosc= Fin*M/(N1*N2), Fcy=Fosc/4
	 * Fosc= 7.37M*40/(2*2)=80Mhz for 7.37M input clock */
//OSCCONbits.POST
//POST<1:0>: Oscillator Postscaler Selection bits
//11 = Oscillator postscaler divides clock by 64
//10 = Oscillator postscaler divides clock by 16
//01 = Oscillator postscaler divides clock by 4
//00 = Oscillator postscaler does not alter clock

	MotorsInit();
	BluetoothInit();
	float a = sin(3.1415926535897932384626433832795/2);
	a=a+a;

	while(1)
	{
		for(k=0; k<h; k++)
		{
			float x, y, z;
			x = 3.6;
			y = 5.73;
			z = x / y;

		}
	}

}




