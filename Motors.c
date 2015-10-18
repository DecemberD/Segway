/**********************************************************************c
* Author: Marcin Dec
* Date: 11.08.2015
* FileName:        Motor.c
* Dependencies:    Header Motor.h
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® XC16 C Compiler
*
************************************************************************/
#include <p30f6013A.h>
#include <math.h>
#include "Globals.h"
#include "Motors.h"
#include "Pid.h"
#include "ADC.h"

//// Timer 2 period in microseconds ///////////////////////////////
#define T2_PERIOD 50L
//// Timer 2 postscaler factor ////////////////////////////////////
#define T2_POST 1L
//// Timer 2 reload value = 1474/////////////////////////////////////////
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




//unsigned int DecaySin = 0;
//unsigned int DecayCos = 10;

//int Rev = 32000;
//int Presc = 0;				// Revolution control prescaler
//int MicStep = 0;			// Sine-Cosine look-up array index (MicStepArray[])
//int MICSTEPS = 16;
//int StepIndx = 0;
//int StepIncLatch = 1;
//int StepInc = 0;
//int PrescVal = 0;
//int PrescValLatch = 1250;
//int SpeedAbs = 0;
//int Speed = 0;
//int Direction = 1;
//int Present = 0;
//int SpeedChange = 1;
//int Tick = 0;
//int PhaseAfwd = 1;
//int PhaseArev = 0;
//int PhaseBfwd = 1;
int debug = 0;
int DebugSet = 5;
int ADCon1_m;
int temp;

const int MICSTEPS = 16;
const int SPEED_SLEWRATE = 1;
extern __psv__ unsigned int SPEED_CURVE[251][3] __attribute__((space(psv)));
extern const    int __attribute__((space(auto_psv),aligned)) Sin90[17];
extern const    int __attribute__((space(auto_psv),aligned)) Cos90[17];
extern void Tick(void);
//unsigned int Sin90[17]; //MICSTEPS + 1
//unsigned int Cos90[17]; //MICSTEPS + 1

struct Motor
{
    int StepPresc;  // = 0;			// Micro step prescaler counter
    int MicStep;    // = 0;			// Micro step counter
    int StepIncLatch;   // = 1;                 // Micro step increment latch register
    int StepInc;    // = 0;                     // Micro step increment
    int StepPrescVal;   // = 0;                 // Micro step prescaler compare value
    int StepPrescValLatch;  // = 1250;          // Micro step prescaler compare value latch register
    int SpeedAbs;   // = 0;                     // Current motor speed absolute value
    int Speed;  // = 0;                         // Current motor speed
    int InputSpeed;  // = 0;                    // Input speed from external
    int Direction;  // = 1;                     // Direction auxiliary variable
    unsigned int DecaySin; // = 0;              // Decay % value for rising slope
    unsigned int DecayCos; // = 10;             // Decay % value for falling slope
    unsigned int Oc84rs; // = 0;                // Decay control auxiliary register
    unsigned int Oc73rs; // = 0;                // Decay control auxiliary register
    unsigned int Oc62rs; // = 0;                // Decay control auxiliary register
    unsigned int Oc51rs; // = 0;                // Decay control auxiliary register
    unsigned int Oc84r; // = 0;                 // Decay control auxiliary register
    unsigned int Oc73r; // = 0;                 // Decay control auxiliary register
    unsigned int Oc62r; // = 0;                 // Decay control auxiliary register
    unsigned int Oc51r; // = 0;                 // Decay control auxiliary register
}
MotorLeft = {0, 0, 1, 0, 0, 1250, 0, 0, 50, 1, 10,0,0,0,0,0,0,0,0},
MotorRight = {0, 0, 1, 0, 0, 1250, 0, 0, 0, 1, 10,0,0,0,0,0,0,0,0};

//unsigned long int MicStepArray[17]; //  [MICSTEPS+1]
//union
//{
//	struct
//	{
//		unsigned int Sin90;
//		unsigned int Cos90;
//	} MicStepStruct;
//	unsigned long int MicStep;
//} MicStepUnion;
//#define INP MicStepUnion.MicStepStruct
//#define MST MicStepUnion

void SpeedSlewRate(struct Motor* motor )
{
    if(motor->InputSpeed > motor->Speed)
    {
        motor->Speed += SPEED_SLEWRATE;
        if(motor->Speed > motor->InputSpeed)
            motor->Speed = motor->InputSpeed;
    }
    else if(motor->InputSpeed < motor->Speed)
    {
        motor->Speed -= SPEED_SLEWRATE;
        if(motor->Speed < motor->InputSpeed)
            motor->Speed = motor->InputSpeed;
    }
}
inline void MotorRightDecayLoad(void)
{
    if(MotorRight.MicStep/MICSTEPS == 0)		// sin >0, cos >0
    {
        OC7R = MotorRight.Oc73r;
        OC7RS = MotorRight.Oc73rs;
        OC5R = MotorRight.Oc51r;
        OC5RS = MotorRight.Oc51rs;
    }
    else if(MotorRight.MicStep/MICSTEPS == 1)		// sin >0, cos <0
    {
        OC7R = MotorRight.Oc73r;
        OC7RS = MotorRight.Oc73rs;
        OC6R = MotorRight.Oc62r;
        OC6RS = MotorRight.Oc62rs;
    }
    else if(MotorRight.MicStep/MICSTEPS == 2)		// sin <0, cos <0
    {
        OC8R = MotorRight.Oc84r;
        OC8RS = MotorRight.Oc84rs;
        OC6R = MotorRight.Oc62r;
        OC6RS = MotorRight.Oc62rs;
    }
    else if(MotorRight.MicStep/MICSTEPS == 3)		// sin <0, cos >0
    {
        OC8R = MotorRight.Oc84r;
        OC8RS = MotorRight.Oc84rs;
        OC5R = MotorRight.Oc51r;
        OC5RS = MotorRight.Oc51rs;
    }
}

inline void MotorLeftDecayLoad(void)
{
    if(MotorLeft.MicStep/MICSTEPS == 0)			// sin >0, cos >0
    {
        OC3R = MotorLeft.Oc73r;
        OC3RS = MotorLeft.Oc73rs;
        OC1R = MotorLeft.Oc51r;
        OC1RS = MotorLeft.Oc51rs;
    }
    else if(MotorLeft.MicStep/MICSTEPS == 1)		// sin >0, cos <0
    {
        OC3R = MotorLeft.Oc73r;
        OC3RS = MotorLeft.Oc73rs;
        OC2R = MotorLeft.Oc62r;
        OC2RS = MotorLeft.Oc62rs;
    }
    else if(MotorLeft.MicStep/MICSTEPS == 2)		// sin <0, cos <0
    {
        OC4R = MotorLeft.Oc84r;
        OC4RS = MotorLeft.Oc84rs;
        OC2R = MotorLeft.Oc62r;
        OC2RS = MotorLeft.Oc62rs;
    }
    else if(MotorLeft.MicStep/MICSTEPS == 3)		// sin <0, cos >0
    {
        OC4R = MotorLeft.Oc84r;
        OC4RS = MotorLeft.Oc84rs;
        OC1R = MotorLeft.Oc51r;
        OC1RS = MotorLeft.Oc51rs;
    }
}
inline void MotorDecayLoad(void)
{
    MotorRightDecayLoad();
    MotorLeftDecayLoad();
}
//inline PidIterate(void)
//{
//    PidExec(ADCCSARight, setp, &PidCSAR)
//}
inline void MotorRightDriver(void)
{
    

    if(MotorRight.MicStep/MICSTEPS == 0)			// sin >0, cos >0
    {
        // Phase A //////
        // Present Sin = 0 and loaded next value, DecaySin
        // Polarisation positive
        //MST.MicStep = MicStepArray[MotorRight.MicStep%MICSTEPS + MotorRight.StepIncLatch];
        if(MotorRight.Direction)
        {
             if(OC8CONbits.OCM != 0b110)
             OC8CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
             if(OC7CONbits.OCM != 0b101)
             OC7CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
        }
        else
        {
            if(OC6CONbits.OCM != 0b110)
            OC6CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
            if(OC5CONbits.OCM != 0b101)
            OC5CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
        }
        OC8RS = PidExec(ADCCSARight, Sin90[MotorRight.MicStep%MICSTEPS + MotorRight.StepIncLatch], &PidCSAR);
        MotorRight.Oc73r = OC8RS;
        if((MotorRight.Oc73r + 10) >= PR2)
            OC7CONbits.OCM = 0b000;                 // Disable fast deccay
        else OC7CONbits.OCM = 0b101;
        MotorRight.Oc73rs = 2 + MotorRight.Oc73r + (PR2 - MotorRight.Oc73r) * MotorRight.DecaySin / 100;
        // Phase B //////
        // Present Cos = 1 and loaded next value, DecayCos
        OC6RS = PidExec(ADCCSBRight,Cos90[MotorRight.MicStep%MICSTEPS + MotorRight.StepIncLatch], &PidCSBR);
        MotorRight.Oc51r = OC6RS;
        if((MotorRight.Oc51r + 10) >= PR2)
            OC5CONbits.OCM = 0b000;                 // Disable fast deccay
        else OC5CONbits.OCM = 0b101;
        MotorRight.Oc51rs = 2 + MotorRight.Oc51r + (PR2 - MotorRight.Oc51r) * MotorRight.DecayCos / 100;
    }
    else if(MotorRight.MicStep/MICSTEPS == 1)		// sin >0, cos <0
    {
        // Phase A //////
        // Present Cos = 1 and loaded next value, DecayCos
        //MST.MicStep = MicStepArray[MotorRight.MicStep%MICSTEPS + MotorRight.StepIncLatch];
        if(MotorRight.Direction)
        {
            if(OC6CONbits.OCM != 0b101)
            OC6CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
            if(OC5CONbits.OCM != 0b110)
            OC5CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
        }
        else
        {
             if(OC8CONbits.OCM != 0b110)
             OC8CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
             if(OC7CONbits.OCM != 0b101)
             OC7CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
        }
        OC8RS = PidExec(ADCCSARight,Cos90[MotorRight.MicStep%MICSTEPS + MotorRight.StepIncLatch], &PidCSAR);
        MotorRight.Oc73r = OC8RS;
        if((MotorRight.Oc73r + 10) >= PR2)
            OC7CONbits.OCM = 0b000;                 // Disable fast deccay
        else OC7CONbits.OCM = 0b101;
        MotorRight.Oc73rs = 2 + MotorRight.Oc73r + (PR2 - MotorRight.Oc73r) * MotorRight.DecayCos / 100;
        // Phase B //////
        // Present Sin = 0 and loaded next value, DecaySin
        // Polarisation negative

        OC5RS = PidExec(ADCCSBRight, Sin90[MotorRight.MicStep%MICSTEPS + MotorRight.StepIncLatch], &PidCSBR);
        MotorRight.Oc62r = OC5RS;
        if((MotorRight.Oc62r + 10) >= PR2)
            OC6CONbits.OCM = 0b000;                 // Disable fast deccay
        else OC6CONbits.OCM = 0b101;
        MotorRight.Oc62rs = 2 + MotorRight.Oc62r + (PR2 - MotorRight.Oc62r) * MotorRight.DecaySin / 100;
    }
    else if(MotorRight.MicStep/MICSTEPS == 2)		// sin <0, cos <0
    {
        // Phase A //////
        // Present Sin = 0 and loaded next value, DecaySin
        // Polarisation negative
        //MST.MicStep = MicStepArray[MotorRight.MicStep%MICSTEPS + MotorRight.StepIncLatch];
        if(MotorRight.Direction)
        {
            if(OC7CONbits.OCM != 0b110)
            OC7CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
            if(OC8CONbits.OCM != 0b101)
            OC8CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
        }
        else
        {
            if(OC6CONbits.OCM != 0b101)
            OC6CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
            if(OC5CONbits.OCM != 0b110)
            OC5CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
        }
        OC7RS = PidExec(ADCCSARight, Sin90[MotorRight.MicStep%MICSTEPS + MotorRight.StepIncLatch], &PidCSAR);
        MotorRight.Oc84r = OC7RS;
        if((MotorRight.Oc84r + 10) >= PR2)
            OC8CONbits.OCM = 0b000;                 // Disable fast deccay
        else OC8CONbits.OCM = 0b101;
        MotorRight.Oc84rs = 2 + MotorRight.Oc84r + (PR2 - MotorRight.Oc84r) * MotorRight.DecaySin / 100;
        // Phase B //////
        // Present Cos = 1 and loaded next value, DecayCos
        OC5RS = PidExec(ADCCSBRight, Cos90[MotorRight.MicStep%MICSTEPS + MotorRight.StepIncLatch], &PidCSBR);
        MotorRight.Oc62r = OC5RS;
        if((MotorRight.Oc62r + 10) >= PR2)
            OC6CONbits.OCM = 0b000;                 // Disable fast deccay
        else OC6CONbits.OCM = 0b101;
        MotorRight.Oc62rs = 2 + MotorRight.Oc62r + (PR2 - MotorRight.Oc62r) * MotorRight.DecayCos / 100;
    }
    else if(MotorRight.MicStep/MICSTEPS == 3)					// sin <0, cos >0
    {
        // Phase A //////
        // Present Cos = 1 and loaded next value, DecayCos
        //MST.MicStep = MicStepArray[MotorRight.MicStep%MICSTEPS + MotorRight.StepIncLatch];
        if(MotorRight.Direction)
        {
            if(OC6CONbits.OCM != 0b110)
            OC6CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
            if(OC5CONbits.OCM != 0b101)
            OC5CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
        }
        else
        {
            if(OC7CONbits.OCM != 0b110)
            OC7CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
            if(OC8CONbits.OCM != 0b101)
            OC8CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
        }
        OC7RS = PidExec(ADCCSARight, Cos90[MotorRight.MicStep%MICSTEPS + MotorRight.StepIncLatch], &PidCSAR);
        MotorRight.Oc84r = OC7RS;
        if((MotorRight.Oc84r + 10) >= PR2)
            OC8CONbits.OCM = 0b000;                 // Disable fast deccay
        else OC8CONbits.OCM = 0b101;
        MotorRight.Oc84rs = 2 + MotorRight.Oc84r + (PR2 - MotorRight.Oc84r) * MotorRight.DecayCos / 100;
        // Phase B //////
        // Present Sin = 0 and loaded next value, DecaySin
        // Polarisation positive

        OC6RS = PidExec(ADCCSBRight, Sin90[MotorRight.MicStep%MICSTEPS + MotorRight.StepIncLatch], &PidCSBR);
        MotorRight.Oc51r = OC6RS;
        if((MotorRight.Oc51r + 10) >= PR2)
            OC5CONbits.OCM = 0b000;                 // Disable fast deccay
        else OC5CONbits.OCM = 0b101;
        MotorRight.Oc51rs = 2 + MotorRight.Oc51r + (PR2 - MotorRight.Oc51r) *  MotorRight.DecaySin / 100;
    }
    MotorRight.StepPresc++;
    if(MotorRight.StepPresc >= MotorRight.StepPrescValLatch)
    {
            MotorRight.StepPresc = 0;
            if(MotorRight.Direction == 1)
            {
                MotorRight.MicStep += MotorRight.StepIncLatch;
                if(MotorRight.MicStep >= MICSTEPS * 4)
                {
                    MotorRight.MicStep = 0;
                    MotorRight.StepIncLatch = MotorRight.StepInc;
                    MotorRight.StepPrescValLatch = MotorRight.StepPrescVal;
                }
            }
            else
            {
                MotorRight.MicStep -= MotorRight.StepIncLatch;
                if(MotorRight.MicStep < 0)
                {
                    MotorRight.StepIncLatch = MotorRight.StepInc;
                    MotorRight.StepPrescValLatch = MotorRight.StepPrescVal;
                    MotorRight.MicStep = MICSTEPS * 4 - MotorRight.StepIncLatch;

                }
            }
    }
}
//inline void MotorLeftDriver(struct Motor* motor)
//{
//    static unsigned int Oc84rs = 0;
//    static unsigned int Oc73rs = 0;
//    static unsigned int Oc62rs = 0;
//    static unsigned int Oc51rs = 0;
//    static unsigned int Oc84r = 0;
//    static unsigned int Oc73r = 0;
//    static unsigned int Oc62r = 0;
//    static unsigned int Oc51r = 0;
//
//    if(MotorLeft.MicStep/MICSTEPS == 0)			// sin >0, cos >0
//    {
//        // Phase A //////
//        // Present Sin = 0 and loaded next value, DecaySin
//        // Polarisation positive
//        //MST.MicStep = MicStepArray[MotorLeft.MicStep%MICSTEPS + MotorLeft.StepIncLatch];
//        if(MotorLeft.Direction)
//        {
//             if(OC4CONbits.OCM != 0b110)
//             OC4CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
//             if(OC3CONbits.OCM != 0b101)
//             OC3CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
//        }
//        else
//        {
//            if(OC2CONbits.OCM != 0b110)
//            OC2CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
//            if(OC1CONbits.OCM != 0b101)
//            OC1CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
//        }
//        OC4RS = Sin90[MotorLeft.MicStep%MICSTEPS + MotorLeft.StepIncLatch];
//        OC3R = Oc73r;
//        OC3RS = Oc73rs;
//        Oc73r = OC4RS;
//        if((Oc73r + 10) >= PR2)
//            OC3CONbits.OCM = 0b000;                 // Disable fast deccay
//        else OC3CONbits.OCM = 0b101;
//        Oc73rs = 2 + Oc73r + (PR2 - Oc73r) * MotorLeft.DecaySin / 100;
//        // Phase B //////
//        // Present Cos = 1 and loaded next value, DecayCos
//        OC2RS = Cos90[MotorLeft.MicStep%MICSTEPS + MotorLeft.StepIncLatch];
//        OC1R = Oc51r;
//        OC1RS = Oc51rs;
//        Oc51r = OC2RS;
//        if((Oc51r + 10) >= PR2)
//            OC1CONbits.OCM = 0b000;                 // Disable fast deccay
//        else OC1CONbits.OCM = 0b101;
//        Oc51rs = 2 + Oc51r + (PR2 - Oc51r) * MotorLeft.DecayCos / 100;
//    }
//    else if(MotorLeft.MicStep/MICSTEPS == 1)		// sin >0, cos <0
//    {
//        // Phase A //////
//        // Present Cos = 1 and loaded next value, DecayCos
//        //MST.MicStep = MicStepArray[MotorLeft.MicStep%MICSTEPS + MotorLeft.StepIncLatch];
//        if(MotorLeft.Direction)
//        {
//            if(OC2CONbits.OCM != 0b101)
//            OC2CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
//            if(OC1CONbits.OCM != 0b110)
//            OC1CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
//        }
//        else
//        {
//             if(OC4CONbits.OCM != 0b110)
//             OC4CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
//             if(OC3CONbits.OCM != 0b101)
//             OC3CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
//        }
//        OC4RS = Cos90[MotorLeft.MicStep%MICSTEPS + MotorLeft.StepIncLatch];
//        OC3R = Oc73r;
//        OC3RS = Oc73rs;
//        Oc73r = OC4RS;
//        if((Oc73r + 10) >= PR2)
//            OC3CONbits.OCM = 0b000;                 // Disable fast deccay
//        else OC3CONbits.OCM = 0b101;
//        Oc73rs = 2 + Oc73r + (PR2 - Oc73r) * MotorLeft.DecayCos / 100;
//        // Phase B //////
//        // Present Sin = 0 and loaded next value, DecaySin
//        // Polarisation negative
//
//        OC1RS = Sin90[MotorLeft.MicStep%MICSTEPS + MotorLeft.StepIncLatch];
//        OC2R = Oc62r;
//        OC2RS = Oc62rs;
//        Oc62r = OC1RS;
//        if((Oc62r + 10) >= PR2)
//            OC2CONbits.OCM = 0b000;                 // Disable fast deccay
//        else OC2CONbits.OCM = 0b101;
//        Oc62rs = 2 + Oc62r + (PR2 - Oc62r) * MotorLeft.DecaySin / 100;
//    }
//    else if(MotorLeft.MicStep/MICSTEPS == 2)		// sin <0, cos <0
//    {
//        // Phase A //////
//        // Present Sin = 0 and loaded next value, DecaySin
//        // Polarisation negative
//        //MST.MicStep = MicStepArray[MotorLeft.MicStep%MICSTEPS + MotorLeft.StepIncLatch];
//        if(MotorLeft.Direction)
//        {
//            if(OC3CONbits.OCM != 0b110)
//            OC3CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
//            if(OC4CONbits.OCM != 0b101)
//            OC4CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
//        }
//        else
//        {
//            if(OC2CONbits.OCM != 0b101)
//            OC2CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
//            if(OC1CONbits.OCM != 0b110)
//            OC1CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
//        }
//        OC3RS = Sin90[MotorLeft.MicStep%MICSTEPS + MotorLeft.StepIncLatch];
//        OC4R = Oc84r;
//        OC4RS = Oc84rs;
//        Oc84r = OC3RS;
//        if((Oc84r + 10) >= PR2)
//            OC4CONbits.OCM = 0b000;                 // Disable fast deccay
//        else OC4CONbits.OCM = 0b101;
//        Oc84rs = 2 + Oc84r + (PR2 - Oc84r) * MotorLeft.DecaySin / 100;
//        // Phase B //////
//        // Present Cos = 1 and loaded next value, DecayCos
//        OC1RS = Cos90[MotorLeft.MicStep%MICSTEPS + MotorLeft.StepIncLatch];
//        OC2R = Oc62r;
//        OC2RS = Oc62rs;
//        Oc62r = OC1RS;
//        if((Oc62r + 10) >= PR2)
//            OC2CONbits.OCM = 0b000;                 // Disable fast deccay
//        else OC2CONbits.OCM = 0b101;
//        Oc62rs = 2 + Oc62r + (PR2 - Oc62r) * MotorLeft.DecayCos / 100;
//    }
//    else if(MotorLeft.MicStep/MICSTEPS == 3)					// sin <0, cos >0
//    {
//        // Phase A //////
//        // Present Cos = 1 and loaded next value, DecayCos
//        //MST.MicStep = MicStepArray[MotorLeft.MicStep%MICSTEPS + MotorLeft.StepIncLatch];
//        if(MotorLeft.Direction)
//        {
//            if(OC2CONbits.OCM != 0b110)
//            OC2CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
//            if(OC1CONbits.OCM != 0b101)
//            OC1CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
//        }
//        else
//        {
//            if(OC3CONbits.OCM != 0b110)
//            OC3CONbits.OCM = 0b110;		// "110" Set OC mode to Simple PMW without Fault input
//            if(OC4CONbits.OCM != 0b101)
//            OC4CONbits.OCM = 0b101;		// "101" Dual output match mode - continuous
//        }
//        OC3RS = Cos90[MotorLeft.MicStep%MICSTEPS + MotorLeft.StepIncLatch];
//        OC4R = Oc84r;
//        OC4RS = Oc84rs;
//        Oc84r = OC3RS;
//        if((Oc84r + 10) >= PR2)
//            OC4CONbits.OCM = 0b000;                 // Disable fast deccay
//        else OC4CONbits.OCM = 0b101;
//        Oc84rs = 2 + Oc84r + (PR2 - Oc84r) * MotorLeft.DecayCos / 100;
//        // Phase B //////
//        // Present Sin = 0 and loaded next value, DecaySin
//        // Polarisation positive
//
//        OC2RS = Sin90[MotorLeft.MicStep%MICSTEPS + MotorLeft.StepIncLatch];
//        OC1R = Oc51r;
//        OC1RS = Oc51rs;
//        Oc51r = OC2RS;
//        if((Oc51r + 10) >= PR2)
//            OC1CONbits.OCM = 0b000;                 // Disable fast deccay
//        else OC1CONbits.OCM = 0b101;
//        Oc51rs = 2 + Oc51r + (PR2 - Oc51r) *  MotorLeft.DecaySin / 100;
//    }
//    MotorLeft.StepPresc++;
//    if(MotorLeft.StepPresc >= MotorLeft.StepPrescValLatch)
//    {
//            MotorLeft.StepPresc = 0;
//            if(MotorLeft.Direction == 1)
//            {
//                MotorLeft.MicStep += MotorLeft.StepIncLatch;
//                if(MotorLeft.MicStep >= MICSTEPS * 4)
//                {
//                    MotorLeft.MicStep = 0;
//                    MotorLeft.StepIncLatch = MotorLeft.StepInc;
//                    MotorLeft.StepPrescValLatch = MotorLeft.StepPrescVal;
//                }
//            }
//            else
//            {
//                MotorLeft.MicStep -= MotorLeft.StepIncLatch;
//                if(MotorLeft.MicStep < 0)
//                {
//                    MotorLeft.StepIncLatch = MotorLeft.StepInc;
//                    MotorLeft.StepPrescValLatch = MotorLeft.StepPrescVal;
//                    MotorLeft.MicStep = MICSTEPS * 4 - MotorLeft.StepIncLatch;
//
//                }
//            }
//    }
//}
void MotorInputUpdate(struct Motor* motor)
{
    motor->StepInc = SPEED_CURVE[motor->SpeedAbs][1];
    motor->StepPrescVal = SPEED_CURVE[motor->SpeedAbs][2];

    SpeedSlewRate(motor);
    
    if(motor->Speed > 0)
    {
        if(motor->SpeedAbs == 0)
        {
            motor->StepIncLatch = SPEED_CURVE[1][1];
        }
        motor->SpeedAbs = motor->Speed;
        motor->Direction = 1;

    }
    else if(motor->Speed < 0)
    {
        if(motor->SpeedAbs == 0)
        {
            motor->StepIncLatch = SPEED_CURVE[1][1];
        }
        motor->SpeedAbs = -(motor->Speed);
        motor->Direction = 0;
     }
     else
     {
        motor->SpeedAbs = 0;
        motor->StepIncLatch = 0;
     }
}
void MotorsInputUpdate(void)
{
    MotorInputUpdate(&MotorRight);
    MotorInputUpdate(&MotorLeft);
}
void MotorsInit(void)
{
    //////////////// MicStepArray Init  /////////////////////////////////////////////////////////////////////////
//	int i;
//	for(i = 0; i <= MICSTEPS; i++)
//	{
//		Sin90[i] = (unsigned int)((float)T2_PR2 * sin((float)i/(float)(MICSTEPS) * (PI/2)));
//		Cos90[i] = (unsigned int)((float)T2_PR2 * cos((float)i/(float)(MICSTEPS) * (PI/2)));
//	}
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
       // PORTFbits.RF0 = 1;              //
        PORTF |= 3;
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

        //motor->StepInc = MCurve[motor->SpeedAbs][1];
        //motor->PrescVal = MCurve[motor->SpeedAbs][2];
}

///////// Timer2 Interrupt routine //////////////////////
void __attribute__((__interrupt__,no_auto_psv)) _T2Interrupt(void)
{
	IFS0bits.T2IF = 0;				// Clear Timer2 interrupt status flag
        ADCon1_m = ADCON1;
        MotorDecayLoad();                               // Load decay timers for motor drivers
        //MotorRightDecayLoad();
        //MotorRightDecayLoad();
        ADCUpdate();                                    // Update anologue values
        MotorRightDriver();
        MotorRightDriver();
        MotorsInputUpdate();
        
        Tick();

        if(debug == 1)
        {
            temp = 0;

        }
        if(debug == 2)
        {
            temp = 0;

        }
        if(debug == 3)
        {
            temp = 0;

        }
        if(debug == 4)
        {
            temp = 0;

        }
        if(debug == 5)
        {
            temp = 0;
            
        }
        if(debug == 6)
        {
            debug = 0;

        }
        debug++;

        
//        Present++;
//        if(Present == 800)
//        {
//            Present = 0;
//            if(Speed == 250)
//                SpeedChange = -1;
//            if(Speed == -250)
//                SpeedChange = 1;
//            Speed += SpeedChange;
//        }
//        Tick = 1;

}

//void MotorAcc(int dir,int rate, int inc )
//{
//    Direction = dir;
//    while(1)
//    {
//        while(Tick == 0);
//        Tick = 0;
//        Rev = Rev + inc;
//        if(Rev > rate)
//        {
//          Rev = rate;
//          break;
//        }
//        if(Rev < 0)
//        {
//           Rev = 0;
//          break;
//        }
//    }
//
//}
