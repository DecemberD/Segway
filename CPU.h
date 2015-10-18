/**********************************************************************c
* Author: Marcin Dec
* Date: 01.02.2015
* FileName:        CPU.h
* Dependencies:    
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® C30 v3.00 or higher
*
************************************************************************/
#ifndef CPU.h
#define CPU.h

//// Crystal Frequency in Hz  /////////////////////////////////////
#define F_XT 7372800L
/// PLL gain                  /////////////////////////////////////
#define PLL 16L
/// Postscaler factor		  /////////////////////////////////////
#define POST 1L
/// System Clock Source frequency (Fosc) //////////////////////////
#define F_OSC F_XT*PLL/POST
/// Internal Instruction Cycle Clock frequency (Fcy) = 29491200 Hz //////////////
#define F_CY F_OSC/4L

#endif