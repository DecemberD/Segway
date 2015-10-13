/**********************************************************************c
* © 2007 Microchip Technology Inc.
*
* FileName:        main.c
* Dependencies:    Header (.h) files if applicable, see below
* Processor:       dsPIC33FJ256GP506
* Compiler:        MPLAB® C30 v3.00 or higher
*
*SOFTWARE LICENSE AGREEMENT:
*Except as otherwise expressly provided below, Microchip Technology Inc. 
*(Microchip) licenses this software to you solely for use with Microchip products.
*This software is owned by Microchip and/or its licensors, and is protected under 
*applicable copyright laws.  All rights reserved.
*
*This software and any accompanying information is for suggestion only.  
*It shall not be deemed to modify Microchip’s standard warranty for its products.
*It is your responsibility to ensure that this software meets your requirements.
*
*WARRANTY DISCLAIMER AND LIMIT ON LIABILITY:  SOFTWARE IS PROVIDED AS IS.  
*MICROCHIP AND ITS LICENSORS EXPRESSLY DISCLAIM ANY WARRANTY OF ANY KIND, 
*WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
*OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. 
*IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE FOR ANY INCIDENTAL, 
*SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, 
*HARM TO YOUR EQUIPMENT, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY 
*OR SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY 
*DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER SIMILAR 
*COSTS.  To the fullest extend allowed by law, Microchip and its licensors 
*liability shall not exceed the amount of fees, if any, that you have paid 
*directly to Microchip to use this software.
*
*THIRD PARTY SOFTWARE:  Notwithstanding anything to the contrary, any third 
*party software accompanying this software including but not limited to ITU 
*software is subject to the terms and conditions of the third party’s license
*agreement such as the General Public License.  To the extent required by third
*party licenses covering such third party software, the terms of such license 
*will apply in lieu of the terms provided herein.  To the extent the terms of 
*such third party licenses prohibit any of the restrictions described herein, 
*such restrictions will not apply to such third party software.  THIRD PARTY 
*SOFTWARE  IS SUBJECT TO THE FOREGOING WARRANTY DISCLAIMER AND LIMIT ON LIABILITY 
*PROVIDED IN THE PARAGRAPH ABOVE
*
*MICROCHIP PROVIDES THIS SOFTWARE (INCLUDING ACCOMPANYING THIRD PARTY SOFTWARE)
*CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE TERMS.
************************************************************************/

#include <p33FJ256GP506.h>
#include "..\h\sask.h"

#define PERIOD 256
#define KEYSTEP PERIOD/10
#define SAMPLE 128

_FGS(GWRP_OFF & GCP_OFF);                      // no code protection, no write protection
_FOSCSEL(FNOSC_FRC);                           // fast RC oscilator selected
_FOSC(FCKSM_CSECMD & OSCIOFNC_ON & POSCMD_NONE); // clock switching enabled, OSC2 pin as digital I/O, primary oscilator disabled
_FWDT(FWDTEN_OFF);                               // watchdog disabled



/* Allocate memory for buffers and drivers	*/




extern int sin_pwm[SAMPLE];
int* sin_sample;

unsigned int duty=0;
unsigned int duty1=0;
unsigned int sin_prescaler;
unsigned int sin_prescaler_val=0;
unsigned int gain=128;
unsigned int sin_index1=0;
unsigned int sin_index2=SAMPLE/2;
unsigned int sin_index3=SAMPLE/4;
unsigned int sin_index4=(SAMPLE/4)+(SAMPLE/2);

	/* Code for Timer 2 ISR*/
	void __attribute__((__interrupt__)) _T2Interrupt( void )
{
	/* Interrupt Service Routine code goes here */
	if(sin_prescaler==sin_prescaler_val) {
		sin_index1++;
		if (sin_index1==SAMPLE) {
			sin_index1=0;
		}	
		sin_index2++;
		if (sin_index2==SAMPLE){
			sin_index2=0;			
		}
		sin_index3++;
		if (sin_index3==SAMPLE){
			sin_index3=0;
		}
		sin_index4++;
		if (sin_index4==SAMPLE){
			sin_index4=0;
		}
		if (sin_index1==1) {
			SINK_A=1;
			SINK_B=0;
		}
		if (sin_index2==1){
			SINK_B=1;
			SINK_A=0;			
		}
		if (sin_index3==1){
			SINK_C=1;
			SINK_D=0;
		}
		if (sin_index4==1){
			SINK_D=1;
			SINK_C=0;
		}
		sin_prescaler=0;
	}
	else sin_prescaler++;

	OC3RS = (((*(sin_sample+sin_index1))*gain)>>8); // Write Duty Cycle value for next PWM cycle
	OC4RS = (((*(sin_sample+sin_index2))*gain)>>8);
	OC5RS = (((*(sin_sample+sin_index3))*gain)>>8);
	OC6RS = (((*(sin_sample+sin_index4))*gain)>>8);
	YELLOW_LED=~YELLOW_LED;	
	IFS0bits.T2IF = 0; // Clear Timer 2 interrupt flag

}

	/* Code for Timer 3 ISR*/
	void __attribute__((__interrupt__)) _T3Interrupt( void )
{
	T3CONbits.TON = 0; // Disable Timer
	TMR3 = 0x00; // Clear timer register
	PR3 = PERIOD; // Load the period value
	IEC0bits.T3IE = 0; // Disable Timer 3 interrupt
	T3CONbits.TON = 1; // Start Timer 3
	IFS0bits.T3IF = 0; // Clear Timer 3 interrupt flag
}
int main(void)
{
	/* Configure Oscillator to operate the device at 40MHz.
	 * Fosc= Fin*M/(N1*N2), Fcy=Fosc/2
	 * Fosc= 7.37M*40/(2*2)=80Mhz for 7.37M input clock */
 
	PLLFBD=41;				/* M=39	*/
	CLKDIVbits.PLLPOST=0;		/* N1=2	*/
	CLKDIVbits.PLLPRE=0;		/* N2=2	*/
	OSCTUN=0;			
	
	__builtin_write_OSCCONH(0x01);		/*	Initiate Clock Switch to FRC with PLL*/
	__builtin_write_OSCCONL(0x01);
	while (OSCCONbits.COSC != 0b01);	/*	Wait for Clock switch to occur	*/
	while(!OSCCONbits.LOCK);	


	/* Intialize the board and the drivers	*/
	SASKInit();
	duty=100;
	sin_sample=&sin_pwm;
	
// Initialize Output Compare Modules
	OC3CONbits.OCM = 0b000; // Disable Output Compare Module
	OC3R = 0; // Write the duty cycle for the first PWM pulse
	OC3RS = 0; // Write the duty cycle for the second PWM pulse
	OC3CONbits.OCTSEL = 0; // Select Timer 2 as output compare time base
	OC3R = 0; // Load the Compare Register Value
	OC3CONbits.OCM = 0b110; // Select the Output Compare mode

	OC4CONbits.OCM = 0b000; // Disable Output Compare Module
	OC4R = 100; // Write the duty cycle for the first PWM pulse
	OC4RS = 200; // Write the duty cycle for the second PWM pulse
	OC4CONbits.OCTSEL = 0; // Select Timer 2 as output compare time base
	OC4R = 100; // Load the Compare Register Value
	OC4CONbits.OCM = 0b110; // Select the Output Compare mode

	OC5CONbits.OCM = 0b000; // Disable Output Compare Module
	OC5R = 100; // Write the duty cycle for the first PWM pulse
	OC5RS = 200; // Write the duty cycle for the second PWM pulse
	OC5CONbits.OCTSEL = 1; // Select Timer 3 as output compare time base
	OC5R = 100; // Load the Compare Register Value
	OC5CONbits.OCM = 0b110; // Select the Output Compare mode

	OC6CONbits.OCM = 0b000; // Disable Output Compare Module
	OC6R = 100; // Write the duty cycle for the first PWM pulse
	OC6RS = 200; // Write the duty cycle for the second PWM pulse
	OC6CONbits.OCTSEL = 1; // Select Timer 3 as output compare time base
	OC6R = 100; // Load the Compare Register Value
	OC6CONbits.OCM = 0b110; // Select the Output Compare mode
	
// Initialize  Timer2
	T2CONbits.TON = 0; // Disable Timer
	T2CONbits.TCS = 0; // Select internal instruction cycle clock
	T2CONbits.TGATE = 0; // Disable Gated Timer mode
	T2CONbits.TCKPS = 0b01; // Select 1:8 Prescaler
	TMR2 = 0x00; // Clear timer register
	PR2 = PERIOD; // Load the period value
	IPC1bits.T2IP = 0x01; // Set Timer 2 Interrupt Priority Level
	IFS0bits.T2IF = 0; // Clear Timer 2 Interrupt Flag
	IEC0bits.T2IE = 1; // Enable Timer 2 interrupt

// Initialize Timer3
	T3CONbits.TON = 0; // Disable Timer
	T3CONbits.TCS = 0; // Select internal instruction cycle clock
	T3CONbits.TGATE = 0; // Disable Gated Timer mode
	T3CONbits.TCKPS = 0b01; // Select 1:8 Prescaler
	TMR3 = 0x00; // Clear timer register
	PR3 = PERIOD/2; // Load the period value
	IPC2bits.T3IP = 0x01; // Set Timer 3 Interrupt Priority Level
	IFS0bits.T3IF = 0; // Clear Timer 3 Interrupt Flag
	IEC0bits.T3IE = 1; // Enable Timer 3 interrupt

	T2CONbits.TON = 1; // Start Timer 2
	T3CONbits.TON = 1; // Start Timer 3



		



//remove comment in normal run//	FIRinit();
//remove comment in normal run//	Init_Timer1();


	while(1)
	{
			/* Obtaing the ADC samples	*/
			//while(WM8510IsReadBusy(codecHandle));
			//WM8510Read(codecHandle,samples,FRAME_SIZE);
					
		
					/* Stop the Audio input and output since this is a blocking
					 * operation. Also rewind record and playback pointers to
					 * start of the user flash area.*/
					 
					//WM8510Stop(codecHandle);
					
					
					/* Since erase is complete, the next time the loop is executed
					 * dont erase the flash. Start the audio input and output	*/
				
					//WM8510Start(codecHandle);
				
				
			//remove comment in normal run//if(trig)

			//remove comment in normal run//	trig=0;
				//	if(mute) 
					


			
			/* The CheckSwitch functions are defined in sask.c	*/
			
			if((CheckSwitchS1()) == 1)
			{
				/* Toggle the record function and Yellow led.
				 * Rewind the intro message playback pointer. 
				 * And if recording, disable playback.*/
				//mute=0; 
				//factor1+=10;
				duty+=KEYSTEP;
				//if(duty==PERIOD) duty=PERIOD-KEYSTEP;
				//sin_index1++;
				//if(sin_index1==128) sin_index1=127;
				//sin_prescaler_val++;	
				gain++;
				if(gain==256) gain=255;			
			}
			
			
			if((CheckSwitchS2()) == 1)
			{
				/* Toggle the record function and AMBER led.
				 * Rewind the intro message playback pointer. 
				 * And if recording, disable playback.*/
				//mute=1;  
				//factor1-=10;
				duty-=KEYSTEP;
			//	if(duty==0) duty=KEYSTEP;
				//sin_index1--;
				//if(sin_index1==65535) sin_index1=0;
				//sin_prescaler_val--;
				gain--;
				if(gain==65535) gain=0;	
			}
			
			
	}
	
	
	
}
