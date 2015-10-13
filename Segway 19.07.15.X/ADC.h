/**********************************************************************c
* Author: Marcin Dec
* Date: 11.08.2015
* FileName:        ADC.c
* Dependencies:    ADC.h
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® XC16 C Compiler
*
************************************************************************/
#include <p30f6013A.h>
/////////////// Motor Right ///////////////////
        //  CSA - AN8/RB8
        //  CSB - AN9/RB9
/////////////// Motor Left ///////////////////
        //  CSA - AN6/OCFA/RB6
        //  CSB - AN7/RB7
/////////////// Battery //////////////////////
        // Current - AN10/RB10
        // Voltage - AN11/RB11

#define ADC_BUFF_LENGTH 5
#define ADC_INPUTS  6
extern int* ADC16Ptr;
extern int ADCBuff[ADC_BUFF_LENGTH];
extern const int ADCInputs[ADC_INPUTS];
extern int ADCCSARight;
extern int ADCCSBRight;
extern int ADCCSALeft;
extern int ADCCSBLeft;
extern int ADCCurrent;
extern int ADCVoltage;

extern int ADCInpChange(void);
extern void ADCUpdate(void);
extern void ADCInit(void);

