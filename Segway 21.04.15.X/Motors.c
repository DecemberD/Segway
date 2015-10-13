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
//#include "sin90.h"
//#include "cos90.h"
//// Timer 2 period in microseconds ///////////////////////////////
#define T2_PERIOD 50L
//// Timer 2 postscaler factor ////////////////////////////////////
#define T2_POST 1L
//// Timer 2 reload value /////////////////////////////////////////
#define T2_PR2 (T2_PERIOD)*(F_CY/T2_POST)/1000000L
//// Step Motor microsteps ///////////////////////////////////////
//#define MICSTEPS 64

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

unsigned long int MicStepArray[65]; // zamien na [MICSTEPS]
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
unsigned int DeccaySin = 0;
unsigned int DeccayCos = 10;
int Rev = 32000;
int Presc = 0;				// Revolution control prescaler
int MicStep = 0;			// Sine-Cosine look-up array index (MicStepArray[])
int MICSTEPS = 16;
int StepIndx = 0;
int StepIncLatch = 0;
int PrescValue = 100;
int Direction = 1;
int Tick = 0;

void MotorsInit(void) 
{
    //////////////// MicStepArray Init  /////////////////////////////////////////////////////////////////////////
	int i;
	for(i = 0; i <= MICSTEPS; i++)
	{
		INP.Sin90 = (unsigned int)((float)T2_PR2 * sin((float)i/(float)(MICSTEPS) * (PI/2)));
		INP.Cos90 = (unsigned int)((float)T2_PR2 * cos((float)i/(float)(MICSTEPS) * (PI/2)));
		MicStepArray[i] = MST.MicStep;
	}
    //////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////// Motor Control Pins ///////////////////////////////////////////////////////////////////////////////
	/////////////// Motor Right ///////////////////
        //  ENA -   RF1
        //  In1 -   OC8/CN16/RD7
        //  In2 -   OC7/CN15/RD6
        //  ENB -   RF0
        //  In3 -   OC6/CN14/RD5
        //  In4 -   OC5/CN13/RD4
        /////////////// Motor Left ///////////////////
        //  ENA -   RD13
        //  In1 -   OC4/RD3
        //  In2 -   OC3/RD2
        //  ENB -   RD12
        //  In3 -   EMUD2/OC2/RD1
        //  In4 -   EMUC2/OC1/RD0
        ///////////////////////////////////////////////

        TRISFbits.TRISF1 = 0;		// Enable Motor driver pins as outputs
	TRISFbits.TRISF0 = 0;		//
	TRISDbits.TRISD13 = 0;		//
	TRISDbits.TRISD12 = 0;		//

        PORTFbits.RF1 = 1;              // Enable Motor drivers
        PORTFbits.RF0 = 1;              //
        PORTDbits.RD13 = 1;             //
        PORTDbits.RD12 = 1;             //

        TRISDbits.TRISD7 = 0;		// Enable Motor driver pins as outputs
	TRISDbits.TRISD6 = 0;		//
	TRISDbits.TRISD5 = 0;		//
	TRISDbits.TRISD4 = 0;		//
        TRISDbits.TRISD3 = 0;		// Enable Motor driver pins as outputs
	TRISDbits.TRISD2 = 0;		//
	TRISDbits.TRISD1 = 0;		//
	TRISDbits.TRISD0 = 0;		//

        PORTDbits.RD7 = 0;              // Enable Motor drivers
        PORTDbits.RD6 = 0;              //
        PORTDbits.RD5 = 0;              //
        PORTDbits.RD4 = 0;              //
        PORTDbits.RD3 = 0;              // Enable Motor drivers
        PORTDbits.RD2 = 0;              //
        PORTDbits.RD1 = 0;              //
        PORTDbits.RD0 = 0;              //
	/////////////// Output Compare 8  Setup  ////////////////////////////////////////////////////////////////////
	OC8CON = 0;			// Turn off Output Compare Module
	OC8RS = 0;			// Write duty cycle buffer register
	OC8R = 0;			// Write OCR to initial duty cycle value
	OC8CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
					// "111" Set OC mode to Simple PMW with Fault input
	OC8CONbits.OCTSEL = 0;		// "0" Timer 2 as time base
                                        // "1" Timer 3 as time base
	/////////////// Output Compare 7  Setup  ////////////////////////////////////////////////////////////////////
	OC7CON = 0;			// Turn off Output Compare Module
	OC7RS = 0;			// Write duty cycle buffer register
	OC7R = 0xFFFF;			// Write OCR to initial duty cycle value
	OC7CONbits.OCM = 0b101;		// Dual output match mode - continuous
                                            // "110" Set OC mode to Simple PMW without Fault input
                                            // "111" Set OC mode to Simple PMW with Fault input
	OC7CONbits.OCTSEL = 0;		// "0" Timer 2 as time base
                                            // "1" Timer 3 as time base
        /////////////// Output Compare 6  Setup  ////////////////////////////////////////////////////////////////////
	OC6CON = 0;			// Turn off Output Compare Module
	OC6RS = 0;			// Write duty cycle buffer register
	OC6R = 0;			// Write OC2R to initial duty cycle value
	OC6CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
                                            // "111" Set OC mode to Simple PMW with Fault input
	OC6CONbits.OCTSEL = 0;		// "0" Timer 2 as time base
                                            // "1" Timer 3 as time base
        /////////////// Output Compare 5  Setup  ////////////////////////////////////////////////////////////////////
	OC5CON = 0;			// Turn off Output Compare Module
	OC5RS = 0;			// Write duty cycle buffer register
	OC5R = 0xFFFF;			// Write OC2R to initial duty cycle value
	OC5CONbits.OCM = 0b101;		// Dual output match mode - continuous
                                            // "110" Set OC mode to Simple PMW without Fault input
                                            // "111" Set OC mode to Simple PMW with Fault input
	OC5CONbits.OCTSEL = 0;		// "0" Timer 2 as time base
                                            // "1" Timer 3 as time base
        /////////////// Output Compare 4  Setup  ////////////////////////////////////////////////////////////////////
	OC4CON = 0;			// Turn off Output Compare Module
	OC4RS = 0;			// Write duty cycle buffer register
	OC4R = 0;			// Write OC2R to initial duty cycle value
	OC4CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
                                            // "111" Set OC mode to Simple PMW with Fault input
	OC4CONbits.OCTSEL = 0;		// "0" Timer 2 as time base
                                            // "1" Timer 3 as time base
        /////////////// Output Compare 3  Setup  ////////////////////////////////////////////////////////////////////
	OC3CON = 0;			// Turn off Output Compare Module
	OC3RS = 0;			// Write duty cycle buffer register
	OC3R = 0;			// Write OC2R to initial duty cycle value
	OC3CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
                                            // "111" Set OC mode to Simple PMW with Fault input
	OC3CONbits.OCTSEL = 0;		// "0" Timer 2 as time base
                                            // "1" Timer 3 as time base
        /////////////// Output Compare 2  Setup  ////////////////////////////////////////////////////////////////////
	OC2CON = 0;			// Turn off Output Compare Module
	OC2RS = 0;			// Write duty cycle buffer register
	OC2R = 0;			// Write OC2R to initial duty cycle value
	OC2CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
                                            // "111" Set OC mode to Simple PMW with Fault input
	OC2CONbits.OCTSEL = 0;		// "0" Timer 2 as time base
                                            // "1" Timer 3 as time base
        /////////////// Output Compare 1  Setup  ////////////////////////////////////////////////////////////////////
	OC1CON = 0;			// Turn off Output Compare Module
	OC1RS = 1;			// Write duty cycle buffer register
	OC1R = 0;			// Write OC2R to initial duty cycle value
	OC1CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
                                            // "111" Set OC mode to Simple PMW with Fault input
	OC1CONbits.OCTSEL = 0;		// "0" Timer 2 as time base
                                            // "1" Timer 3 as time base
	//////////// Timer 2  Setup  ////////////////////////////////////////////////////////////////////////////////
	T2CON = 0;			// Stops the Timer2 and reset control register
	TMR2 = 0;			// Clear contents of the timer register
	T2CONbits.TCKPS = T2_TCKPS;	// Timer Input Clock Prescale Select bits
                                            // 11 = 1:256 prescale value
                                            // 10 = 1:64 prescale value
                                            // 01 = 1:8 prescale value
                                            // 00 = 1:1 prescale value
	PR2 = T2_PR2;			// Load the Period register
	IPC1bits.T2IP = 0x3;		// Set Timer2 interrupt priority to 3 (1-lowest, 7-highiest)
	IFS0bits.T2IF = 0;		// Clear the Timer2 interrupt status flag
	IEC0bits.T2IE = 1;		// Enable Timer2 interrupts
	T2CONbits.TON = 1;		// Start Timer2
	
}

///////// Timer2 Interrupt routine //////////////////////
void __attribute__((__interrupt__,no_auto_psv)) _T2Interrupt(void)
{
        static unsigned int Oc8rs = 0;
        static unsigned int Oc7rs = 0;
        static unsigned int Oc6rs = 0;
        static unsigned int Oc5rs = 0;
        static unsigned int Oc8r = 0;
        static unsigned int Oc7r = 0;
        static unsigned int Oc6r = 0;
        static unsigned int Oc5r = 0;
	IFS0bits.T2IF = 0;				// Clear the Timer2 interrupt status flag
	
        
        
        if(MicStep == 0)
        {
            // Phase A //////
            // Present Sin = 0 and loaded next value, DeccaySin
            // Polarisation positive
//////            if((StepIndx = MicStep%MICSTEPS + StepIncLatch) >= MICSTEPS)
//////            {
//////                StepIndx = 64;
//////            }
            MST.MicStep = MicStepArray[MicStep%MICSTEPS + StepIncLatch];
            if(OC8CONbits.OCM != 0b110)
            OC8CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
            if(OC7CONbits.OCM != 0b101)
            OC7CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
            OC8RS = INP.Sin90;
            OC7R = Oc7r;
            OC7RS = Oc7rs;
            Oc7r = OC8RS;
            if((Oc7r + 10) >= PR2)
                OC7CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC7CONbits.OCM = 0b101;
            Oc7rs = 2 + Oc7r + (PR2 - Oc7r) * DeccaySin / 100;
            // Phase B //////
            // Present Cos = 1 and loaded next value, DeccayCos
            OC6RS = INP.Cos90;
            OC5R = Oc5r;
            OC5RS = Oc5rs;
            Oc5r = OC6RS;
            if((Oc5r + 10) >= PR2)
                OC5CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC5CONbits.OCM = 0b101;
            Oc5rs = 2 + Oc5r + (PR2 - Oc5r) * DeccayCos / 100;
        }
        else if(MicStep + 1 == MICSTEPS)
        {
            // Phase A //////
            // Present Sin -> 1 and loaded next value Cos = 1, DeccayCos
            MST.MicStep = MicStepArray[0];
            OC8RS = INP.Cos90;
            OC7R =  Oc7r;
            OC7RS = Oc7rs;
            Oc7r = OC8RS;
            if((Oc7r + 10) >= PR2)
                OC7CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC7CONbits.OCM = 0b101;
            Oc7rs = 2 + Oc7r + (PR2 - Oc7r) * DeccayCos / 100;
            // Phase B //////
            // Present Cos -> 0 and loaded next value Sin = 0, DeccaySin
            OC6RS = INP.Sin90;
            OC5R = Oc5r;
            OC5RS = Oc5rs;
            Oc5r = OC6RS;
            if((Oc5r + 10) >= PR2)
                OC5CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC5CONbits.OCM = 0b101;
            Oc5rs = 2 + Oc5r + (PR2 - Oc5r) * DeccaySin / 100;
        }
        else if(MicStep/MICSTEPS == 0)			// sin >0, cos >0
	{
            // Phase A //////
            // Present Sin and loaded next value Sin , DeccaySin
            MST.MicStep = MicStepArray[MicStep%MICSTEPS + 1];
            OC8RS = INP.Sin90;
            OC7R = Oc7r;
            OC7RS = Oc7rs;
            Oc7r = OC8RS;
            if((Oc7r + 10) >= PR2)
                OC7CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC7CONbits.OCM = 0b101;
            Oc7rs = 2 + Oc7r + (PR2 - Oc7r) * DeccaySin / 100;
            // Phase B //////
            // Present Cos and loaded next value Cos , DeccayCos
            OC6RS = INP.Cos90;
            OC5R = Oc5r;
            OC5RS = Oc5rs;
            Oc5r = OC6RS;
            if((Oc5r + 10) >= PR2)
                OC5CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC5CONbits.OCM = 0b101;
            Oc5rs = 2 + Oc5r + (PR2 - Oc5r) * DeccayCos / 100;
	}
        else if(MicStep == MICSTEPS)
        {
            // Phase A //////
            // Present Cos = 1 and loaded next value, DeccayCos
            MST.MicStep = MicStepArray[MicStep%MICSTEPS + 1];
            OC8RS = INP.Cos90;
            OC7R = Oc7r;
            OC7RS = Oc7rs;
            Oc7r = OC8RS;
            if((Oc7r + 10) >= PR2)
                OC7CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC7CONbits.OCM = 0b101;
            Oc7rs = 2 + Oc7r + (PR2 - Oc7r) * DeccayCos / 100;
            // Phase B //////
            // Present Sin = 0 and loaded next value, DeccaySin
            // Polarisation negative
            if(OC6CONbits.OCM != 0b101)
            OC6CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
            if(OC5CONbits.OCM != 0b110)
            OC5CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
            OC5RS = INP.Sin90;
            OC5RS = INP.Sin90;
            OC6R = Oc6r;
            OC6RS = Oc6rs;
            Oc6r = OC5RS;
            if((Oc6r + 10) >= PR2)
                OC6CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC6CONbits.OCM = 0b101;
            Oc6rs = 2 + Oc6r + (PR2 - Oc6r) * DeccaySin / 100;
        }
        else if(MicStep + 1 == 2 * MICSTEPS)
        {
            // Phase A //////
            // Present Cos -> 0 and loaded next value Sin = 0, DeccaySin
            MST.MicStep = MicStepArray[0];
            OC8RS = INP.Sin90;
            OC7R = Oc7r;
            OC7RS = Oc7rs;
            Oc7r = OC8RS;
            if((Oc7r + 10) >= PR2)
                OC7CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC7CONbits.OCM = 0b101;
            Oc7rs = 2 + Oc7r + (PR2 - Oc7r) * DeccayCos / 100;
            // Phase B //////
            // Present Sin -> 1 and loaded next value Cos = 1, DeccayCos
            OC5RS = INP.Cos90;
            OC6R = Oc6r;
            OC6RS = Oc6rs;
            Oc6r = OC5RS;
            if((Oc6r + 10) >= PR2)
                OC6CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC6CONbits.OCM = 0b101;
            Oc6rs = 2 + Oc6r + (PR2 - Oc6r) * DeccayCos / 100;
        }
	else if(MicStep/MICSTEPS == 1)		// sin >0, cos <0
	{
            // Phase A //////
            // Present Cos and loaded next value Cos , DeccayCos
            MST.MicStep = MicStepArray[MicStep%MICSTEPS + 1];
            OC8RS = INP.Cos90;
            OC7R = Oc7r;
            OC7RS = Oc7rs;
            Oc7r = OC8RS;
            if((Oc7r + 10) >= PR2)
                OC7CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC7CONbits.OCM = 0b101;
            Oc7rs = 2 + Oc7r + (PR2 - Oc7r) * DeccayCos / 100;
            // Phase B //////
            // Present Sin and loaded next value Sin , DeccaySin
            OC5RS = INP.Sin90;
            OC6R = Oc6r;
            OC6RS = Oc6rs;
            Oc6r = OC5RS;
            if((Oc6r + 10) >= PR2)
                OC6CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC6CONbits.OCM = 0b101;
            Oc6rs = 2 + Oc6r + (PR2 - Oc6r) * DeccaySin / 100;
	}
        else if(MicStep == 2 * MICSTEPS)
        {
            // Phase A //////
            // Present Sin = 0 and loaded next value, DeccaySin
            // Polarisation negative
            MST.MicStep = MicStepArray[MicStep%MICSTEPS + 1];
            if(OC7CONbits.OCM != 0b110)
            OC7CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
            if(OC8CONbits.OCM != 0b101)
            OC8CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
            OC7RS = INP.Sin90;
            OC7RS = INP.Sin90;
            OC8R = Oc8r;
            OC8RS = Oc8rs;
            Oc8r = OC7RS;
            if((Oc8r + 10) >= PR2)
                OC8CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC8CONbits.OCM = 0b101;
            Oc8rs = 2 + Oc8r + (PR2 - Oc8r) * DeccaySin / 100;
            // Phase B //////
            // Present Cos = 1 and loaded next value, DeccayCos
            OC5RS = INP.Cos90;
            OC6R = Oc6r;
            OC6RS = Oc6rs;
            Oc6r = OC5RS;
            if((Oc6r + 10) >= PR2)
                OC6CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC6CONbits.OCM = 0b101;
            Oc6rs = 2 + Oc6r + (PR2 - Oc6r) * DeccayCos / 100;
        }
        else if(MicStep + 1 == 3 * MICSTEPS)
        {
            // Phase A //////
            // Present Sin -> 1 and loaded next value Cos = 1, DeccayCos
            MST.MicStep = MicStepArray[0];
            OC7RS = INP.Cos90;
            OC8R = Oc8r;
            OC8RS = Oc8rs;
            Oc8r = OC7RS;
            if((Oc8r + 10) >= PR2)
                OC8CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC8CONbits.OCM = 0b101;
            Oc8rs = 2 + Oc8r + (PR2 - Oc8r) * DeccayCos / 100;
            // Phase B //////
            // Present Cos -> 0 and loaded next value Sin = 0, DeccaySin
            OC5RS = INP.Sin90;
            OC6R = Oc6r;
            OC6RS = Oc6rs;
            Oc6r = OC5RS;
            if((Oc6r + 10) >= PR2)
                OC6CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC6CONbits.OCM = 0b101;
            Oc6rs = 2 + Oc6r + (PR2 - Oc6r) * DeccaySin / 100;
        }
	else if(MicStep/MICSTEPS == 2)		// sin <0, cos <0
	{
            // Phase A //////
            // Present Sin and loaded next value Sin , DeccaySin
            MST.MicStep = MicStepArray[MicStep%MICSTEPS + 1];
            OC7RS = INP.Sin90;
            OC8R = Oc8r;
            OC8RS = Oc8rs;
            Oc8r = OC7RS;
            if((Oc8r + 10) >= PR2)
                OC8CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC8CONbits.OCM = 0b101;
            Oc8rs = 2 + Oc8r + (PR2 - Oc8r) * DeccaySin / 100;
            // Phase B //////
            // Present Cos and loaded next value Cos , DeccayCos
            OC5RS = INP.Cos90;
            OC6R = Oc6r;
            OC6RS = Oc6rs;
            Oc6r = OC5RS;
            if((Oc6r + 10) >= PR2)
                OC6CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC6CONbits.OCM = 0b101;
            Oc6rs = 2 + Oc6r + (PR2 - Oc6r) * DeccayCos / 100;
	}
        else if(MicStep == 3 * MICSTEPS)
        {
            // Phase A //////
            // Present Cos = 1 and loaded next value, DeccayCos
            MST.MicStep = MicStepArray[MicStep%MICSTEPS + 1];
            OC7RS = INP.Cos90;
            OC8R = Oc8r;
            OC8RS = Oc8rs;
            Oc8r = OC7RS;
            if((Oc8r + 10) >= PR2)
                OC8CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC8CONbits.OCM = 0b101;
            Oc8rs = 2 + Oc8r + (PR2 - Oc8r) * DeccayCos / 100;
            // Phase B //////
            // Present Sin = 0 and loaded next value, DeccaySin
            // Polarisation positive
            if(OC6CONbits.OCM != 0b110)
            OC6CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
            if(OC5CONbits.OCM != 0b101)
            OC5CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
            OC6RS = INP.Sin90;
            OC5R = Oc5r;
            OC5RS = Oc5rs;
            Oc5r = OC6RS;
            if((Oc5r + 10) >= PR2)
                OC5CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC5CONbits.OCM = 0b101;
            Oc5rs = 2 + Oc5r + (PR2 - Oc5r) * DeccaySin / 100;
        }
        else if(MicStep + 1 == 4 * MICSTEPS)
        {
            // Phase A //////
            // Present Cos -> 0 and loaded next value Sin = 0, DeccaySin
            MST.MicStep = MicStepArray[0];
            OC7RS = INP.Sin90;
            OC8R = Oc8r;
            OC8RS = Oc8rs;
            Oc8r = OC7RS;
            if((Oc8r + 10) >= PR2)
                OC8CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC8CONbits.OCM = 0b101;
            Oc8rs = 2 + Oc8r + (PR2 - Oc8r) * DeccaySin / 100;
            // Phase B //////
            // Present Sin -> 1 and loaded next value Cos = 1, DeccayCos
            OC6RS = INP.Cos90;
            OC5R = Oc5r;
            OC5RS = Oc5rs;
            Oc5r = OC6RS;
            if((Oc5r + 10) >= PR2)
                OC5CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC5CONbits.OCM = 0b101;
            Oc5rs = 2 + Oc5r + (PR2 - Oc5r) * DeccayCos / 100;
        }
	else if(MicStep/MICSTEPS == 3)					// sin <0, cos >0
	{
            // Phase A //////
            // Present Cos and loaded next value Cos , DeccayCos
            MST.MicStep = MicStepArray[MicStep%MICSTEPS + 1];
            OC7RS = INP.Cos90;
            OC8R =  Oc8r;
            OC8RS = Oc8rs;
            Oc8r = OC7RS;
            if((Oc8r + 10) >= PR2)
                OC8CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC8CONbits.OCM = 0b101;
            Oc8rs = 2 + Oc8r + (PR2 - Oc8r) * DeccayCos / 100;
            // Phase B //////
            // Present Sin and loaded next value Sin , DeccaySin
            OC6RS = INP.Sin90;
            OC5R = Oc5r;
            OC5RS = Oc5rs;
            Oc5r = OC6RS;
            if((Oc5r + 10) >= PR2)
                OC5CONbits.OCM = 0b000;                 // Disable fast deccay
            else OC5CONbits.OCM = 0b101;
            Oc5rs = 2 + Oc5r + (PR2 - Oc5r) * DeccaySin / 100;
	}
	Presc++;								
	if(Presc > Rev)
	{
		Presc = 0;
                if(Direction == 1)
                {
                    MicStep += StepIncLatch;
                    if(MicStep == MICSTEPS * 4)
			MicStep = 0;
                }
                else
                {
                    MicStep--;
                    if(MicStep == -1)
			MicStep = MICSTEPS * 4 - 1;
                }
	}
        Tick = 1;

}

void MotorAcc(int dir,int rate, int inc )
{
    Direction = dir;
    while(1)
    {
        while(Tick == 0);
        Tick = 0;
        Rev = Rev + inc;
        if(Rev > rate)
        {
          Rev = rate;
          break;
        }
        if(Rev < 0)
        {
           Rev = 0;
          break;
        }
    }
    
}
