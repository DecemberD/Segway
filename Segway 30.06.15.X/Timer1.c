/**********************************************************************c
* Author: Marcin Dec
* Date: 17.02.2015
* FileName:        Timer1.c
* Dependencies:    Header Timer1.h
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® C30 v3.00 or higher
*
************************************************************************/
#include <p30f6013A.h>

#define F_XT 7372800L
#define PLL 4L
#define POST 1L
#define F_OSC F_XT*PLL/POST
#define F_CY F_OSC/4L

#define T1_LOOP 100L
#define T1_POST 256L
#define T1_PR1 (T1_LOOP)*(F_CY/T1_POST)/1000L

#if (T1_POST == 256) 
	#define T1_TCKPS 3
#elif (T1_POST == 64)
	#define T1_TCKPS 2
#elif (T1_POST == 8)
	#define T1_TCKPS 1
#elif (T1_POST == 1)
	#define T1_TCKPS 0
#else 
	#error "T1_POST must be selected from set {1, 8, 64, 256} "
#endif

int pr1 = 2880;
int dir = 1;
unsigned int pordtD = 1;
unsigned int tempPordtD;
unsigned char MotorL[8] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09};
signed int MotorLIndex = 0;
void Timer1Init(void) 
{
/////////////// Timer1  Setup  ////////////////////////////////////////////////////////////////////
	T1CON = 0;					// Stops the Timer1 and reset control register
	TMR1 = 0;					// Clear contents of the timer register
	T1CONbits.TCKPS = T1_TCKPS;	// Timer Input Clock Prescale Select bits
								// 11 = 1:256 prescale value
								// 10 = 1:64 prescale value
								// 01 = 1:8 prescale value
								// 00 = 1:1 prescale value
	PR1 = pr1;				// Load the Period register
	IPC0bits.T1IP = 0x3;		// Set Timer1 interrupt priority to 3 (1-lowest, 7-highiest)
	IFS0bits.T1IF = 0;			// Clear the Timer1 interrupt status flag
	IEC0bits.T1IE = 1;			// Enable Timer1 interrupts
	T1CONbits.TON = 1;			// Start Timer1 with prescaler settings 1:1

}


///////// Timer1 Interrupt routine //////////////////////
void __attribute__((__interrupt__,no_auto_psv)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0;					// Clear the Timer1 interrupt status flag
	if(dir)
	{
		tempPordtD = PORTD;
		tempPordtD &= 0xFFF0;
		tempPordtD |= pordtD;
		PORTD = tempPordtD;
		pordtD = MotorL[MotorLIndex++];
		if(MotorLIndex == 8) MotorLIndex = 0;
	}
	else
	{
		tempPordtD = PORTD;
		tempPordtD &= 0xFFF0;
		tempPordtD |= pordtD;
		PORTD = tempPordtD;
		pordtD = MotorL[MotorLIndex--];
		if(MotorLIndex == -1) MotorLIndex = 7;
	}
}
