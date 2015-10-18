/**********************************************************************c
* Author: Marcin Dec
* Date: 01.02.2015
* FileName:        CPU.c
* Dependencies:    
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® C30 v3.00 or higher
*
************************************************************************/
#include <p30f6013A.h>

_FOSC(CSW_FSCM_OFF & XT_PLL16);         // no clock switching , primary clock XT with PLL x 16
_FBORPOR( MCLR_EN);                     //  MCLR as reset pin
_FWDT(WDT_OFF);                         // Watchdog disabled
