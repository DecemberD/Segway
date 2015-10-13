/**********************************************************************c
* Author: Marcin Dec
* Date: 20.02.2015
* FileName:        Motors.c
* Dependencies:    Header Motors.h
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® C30 v3.00 or higher
*
************************************************************************/
#include <p30f6013A.h>
#include <math.h>
#include "Globals.h"
#include "Motors.h"

//// Timer 2 period in microseconds ///////////////////////////////
#define T2_PERIOD 50L
//// Timer 2 postscaler factor ////////////////////////////////////
#define T2_POST 1L
//// Timer 2 reload value /////////////////////////////////////////
#define T2_PR2 65535
//(T2_PERIOD)*(F_CY/T2_POST)/1000000L
//// Step Motor microsteps ///////////////////////////////////////
//#define MICSTEPS 64
int MICSTEPS = 64;
int Rev = 100;
#define PI 3.1415926535897932384626433832795F
//// Timer 2 postscaler register value ///////////////////////////
#if (T2_POST == 256) 
	#define T2_TCKPS 3
#elif (T2_POST == 64)
	#define T2_TCKPS 2
#elif (T2_POST == 8)
	#define T2_TCKPS 1
#elif (T2_POST == 1)
	#define T2_TCKPS 0
#else 
	#error "T2_POST must be selected from set {1, 8, 64, 256} "
#endif


unsigned long int MicStepArray[64]; // zamien na [MICSTEPS]
union 
{	
	struct 
	{
		unsigned int Sin90;
		unsigned int Cos90;
	} MicStepStruct;
	unsigned long int MicStep;
} MicStepUnion;
#define INP MicStepUnion.MicStepStruct 
#define MST MicStepUnion 
//int PWM = 2048;
int PERIOD = 4096;
void MotorsInit(void) 
{	
	//////////////// MicStepArray Init  /////////////////////////////////////////////////////////////////////////
	int i;
	for(i = 0; i < MICSTEPS; i++)
	{
		INP.Sin90 = (unsigned int)((float)T2_PR2 * sin((float)i/(float)(MICSTEPS) * (PI/2)));
		INP.Cos90 = (unsigned int)((float)T2_PR2 * cos((float)i/(float)(MICSTEPS) * (PI/2)));
		MicStepArray[i] = MST.MicStep;
	}
	/////////////// Motor Outputs ///////////////////////////////////////////////////////////////////////////////
	TRISDbits.TRISD4 = 0;		//	In1
	TRISDbits.TRISD13 = 0;		//  In2
	TRISDbits.TRISD6 = 0;		//	In3
	TRISDbits.TRISD5 = 0;		//	In4
	//PORTD = 1;	
	/////////////// Output Compare 1  Setup  ////////////////////////////////////////////////////////////////////
	OC1CON = 0;					// Turn off Output Compare 1 Module
	//OC1RS = PWM;					// Write duty cycle buffer register
	//OC1R = PWM;					// Write OC1R to initial duty cycle value
	OC1CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
								// "111" Set OC mode to Simple PMW with Fault input
	OC1CONbits.OCTSEL = 0;		// "0" Timer 2 as time base
								// "1" Timer 3 as time base
	/////////////// Output Compare 2  Setup  ////////////////////////////////////////////////////////////////////
	OC2CON = 0;					// Turn off Output Compare 2 Module
	OC2RS = 0;					// Write duty cycle buffer register
	OC2R = 0;					// Write OC2R to initial duty cycle value
	OC2CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
								// "111" Set OC mode to Simple PMW with Fault input
	OC2CONbits.OCTSEL = 0;		// "0" Timer 2 as time base
								// "1" Timer 3 as time base
	//////////// Timer 2  Setup  ////////////////////////////////////////////////////////////////////////////////
	T2CON = 0;					// Stops the Timer2 and reset control register
	TMR2 = 0;					// Clear contents of the timer register
	T2CONbits.TCKPS = T2_TCKPS;	// Timer Input Clock Prescale Select bits
								// 11 = 1:256 prescale value
								// 10 = 1:64 prescale value
								// 01 = 1:8 prescale value
								// 00 = 1:1 prescale value
	PR2 = PERIOD;				// Load the Period register
	IPC1bits.T2IP = 0x3;		// Set Timer2 interrupt priority to 3 (1-lowest, 7-highiest)
	IFS0bits.T2IF = 0;			// Clear the Timer2 interrupt status flag
	IEC0bits.T2IE = 0;			// Disable Timer2 interrupts
	T2CONbits.TON = 1;			// Start Timer2
	
}

/////////// Timer2 Interrupt routine //////////////////////
//void __attribute__((__interrupt__,no_auto_psv)) _T2Interrupt(void)
//{
//	static int Presc = 0;				// Revolution control prescaler
//	static int MicStep = 0;				// Sine-Cosine look-up array index (MicStepArray[])
//	static int tempPordtD = 0;			// Used to read-modify-write operation on PORTD
//	IFS0bits.T2IF = 0;					// Clear the Timer2 interrupt status flag
//	PORTD = tempPordtD;					// Set In1, In2, In3, In4 from previous iteration
//	tempPordtD = PORTD;					// Read current PORTD for calculation used for next iteration
//	if(MicStep/MICSTEPS == 0)			// sin >0, cos >0
//	{	
//		//				54321
//		//				1111119876543210			
//		tempPordtD &= 0b1101111110001111;
//		tempPordtD |= 0b0000000001010000;
//		MST.MicStep = MicStepArray[MicStep%MICSTEPS];		
//		OC1RS = T2_PR2 + 2;	//OC1RS = INP.Sin90;
//		OC2RS = INP.Cos90;
//	}
//	else if(MicStep/MICSTEPS == 1)		// sin >0, cos <0
//	{	
//		//				54321
//		//				1111119876543210
//		tempPordtD &= 0b1101111110001111;
//		tempPordtD |= 0b0000000000110000;
//		MST.MicStep = MicStepArray[MicStep%MICSTEPS];
//		OC1RS = T2_PR2 + 2;	//OC1RS = INP.Cos90;
//		OC2RS = INP.Sin90;
//	}
//	else if(MicStep/MICSTEPS == 2)		// sin <0, cos <0
//	{
//		//				54321
//		//				1111119876543210
//		tempPordtD &= 0b1101111110001111;
//		tempPordtD |= 0b0010000000100000;
//		MST.MicStep = MicStepArray[MicStep%MICSTEPS];
//		OC1RS = INP.Cos90;	// OC1RS = INP.Sin90;
//		OC2RS = T2_PR2 + 2;	//OC2RS = INP.Cos90;
//	}
//	else								// sin <0, cos >0
//	{
//		//				54321
//		//				1111119876543210
//		tempPordtD &= 0b1101111110001111;
//		tempPordtD |= 0b0000000000110000;
//		MST.MicStep = MicStepArray[MicStep%MICSTEPS];
//		OC1RS = INP.Sin90;	// OC1RS = INP.Cos90;
//		OC2RS = T2_PR2 + 2;	// OC2RS = INP.Sin90;
//	}
//	Presc++;								
//	if(Presc > Rev)
//	{
//		Presc = 0;
//		MicStep++;
//		if(MicStep == MICSTEPS * 4)
//			MicStep = 0;
//	}
//}

//void MotorAcc(int dir, )
//{
//	
//}
