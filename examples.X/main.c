#include <p30f6013A.h>
                             // watchdog disabled

//struct Pid
//{
//        // external
//	int kp;
//	int ki;
//	int kd;
//	int offset;
//	int min;
//	int max;
//        int inverse;
//        // internal
//	long long int i;
//        int prevIn;
//}
//PidCSAR = {10, 10, 10, 0, -100, 100, 1},
//PidCSBR = {10, 10, 10, 0, 0, 30000},
//PidCSAL = {10, 10, 10, 0, 0, 30000},
//PidCSBL = {10, 10, 10, 0, 0, 30000},
//PidSpeedR = {10, 10, 10, 0, -30000, 30000},
//PidSpeedL = {10, 10, 10, 0, -30000, 30000};
//
//extern int PidExec(int in, int setp, struct Pid* pid);
//extern __psv__ unsigned int table[2000] __attribute__((space(psv)));
//int i = 0;
//int in = 0;
//int setp = 0;
//int array[2][2][3]={2,2,2,2,2,2,2,2,2,2,2,2};
//int* pointer;
//
////int PidExecu(int in, struct Pid* pid)
////{
////	int out;
////
////	pid->i += in * (pid->ki);
////	pid->p = in * (pid->kp);
////	pid->d = (in - pid->prevIn) * (pid->kd);
////	pid->prevIn = in;
////	out = pid->p + pid->i + pid->d;
////	out = out << 2;
////        if(out > pid->max)
////            out = pid->max;
////        else if(out < pid->min)
////            out = pid->min;
////        else
////            pid->prevIn = in;
////	return out + pid->offset;
////}
//int main(void)
//{
//    i = table[1];
//    pointer = **array;
//    for(i=0; i<12; i++)
//    {
//        *(pointer+i)=3;
//    }
//    while(1)
//    {
//        i = PidExec(in, setp, &PidCSAR);
//        i++;
//    }
//    i++;
//    //i = PidExecu(5, &PidCSAR);
//    return i;
//}


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
//#include "Bluetooth.h"

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

_FOSC(CSW_FSCM_OFF & XT_PLL4);                      // no clock switching , primary clock XT with PLL x 4
_FBORPOR( MCLR_EN); //  MCLR as reset pin
_FWDT(WDT_OFF);                               // watchdog disabled



/* Allocate memory for buffers and drivers	*/



long k;
long h = 100000;
int dir = 1;
int rev = 32000;
int incr = 200;

int ADCValue;
int* ADC16Ptr;
int count;
#define ADC_BUFF_LENGTH 5
#define ADC_INPUTS  6
int* ADC16Ptr;
int ADCBuff[ADC_BUFF_LENGTH];
const int ADCInputs[ADC_INPUTS] = {8, 9, 6, 7, 10, 11};
int ADCCSARight;
int ADCCSBRight;
int ADCCSALeft;
int ADCCSBLeft;
int ADCCurrent;
int ADCVoltage;
int ADCCSARightArr[ADC_BUFF_LENGTH];
int ADCCSBRightArr[ADC_BUFF_LENGTH];
int ADCCSALeftArr[ADC_BUFF_LENGTH];
int ADCCSBLeftArr[ADC_BUFF_LENGTH];
int ADCCurrentArr[ADC_BUFF_LENGTH];
int ADCVoltageArr[ADC_BUFF_LENGTH];
int ADCon1;
int ADCon1_2;
void ADCBuffLoad(int* buff) {
    int i;
    for (i = 0; i < ADC_BUFF_LENGTH; i++) {
        *(buff + i) = *(ADC16Ptr + i);
    }
}
void ADCBuffCopy(int* buff, int* copy) {
    int i;
    for (i = 0; i < ADC_BUFF_LENGTH; i++) {
        *(copy + i) = *(buff + i);
    }
}
int ADCMax(int* buff) {
    int max = 0, i;
    for (i = 0; i < ADC_BUFF_LENGTH; i++) {
        if (*buff > max) {
            max = *buff++;
        } else buff++;
    }
    return max;
}

int ADCMean(int* buff) {
    int mean = 0, i;
    for (i = 0; i < ADC_BUFF_LENGTH; i++) {
        mean += *buff++;
    }
    mean >>= 2;
    return mean;
}

int ADCInpChange(void) {
    static int index = -1;
    index++;
    if (index >= ADC_INPUTS)
        index = 0;
    //ADCHS = (ADCHS & 0xFFF0) | ADCInputs[index];
    return index;
}

void ADCUpdate(void) {
    static int index = -1;
    int index_new;
    ADCon1_2 = ADCON1;
    ADCON1bits.ASAM = 0; // stop auto sampling
    ADCon1 = ADCON1;
    //ADCON1bits.SAMP = 0; // stop sampling if in progress
    ADCBuffLoad(ADCBuff); // retain ADCBUF content
    index_new = ADCInpChange(); // switch channel and return its index
    IFS0bits.ADIF = 0;
    ADCON1bits.ASAM = 1; // start next sampling cycle
    switch (index) {
        case 0:
            ADCBuffCopy(ADCBuff, ADCCSARightArr);
            //ADCCSARight = ADCMax(ADCBuff);
            break;
        case 1:
            ADCBuffCopy(ADCBuff, ADCCSBRightArr);
            //ADCCSBRight = ADCMax(ADCBuff);
            break;
        case 2:
            ADCBuffCopy(ADCBuff, ADCCSALeftArr);
            //ADCCSALeft = ADCMax(ADCBuff);
            break;
        case 3:
            ADCBuffCopy(ADCBuff, ADCCSBLeftArr);
            //ADCCSBLeft = ADCMax(ADCBuff);
            break;
        case 4:
            ADCBuffCopy(ADCBuff, ADCCurrentArr);
            //ADCCurrent = ADCMean(ADCBuff);
            break;
        case 5:
            ADCBuffCopy(ADCBuff, ADCVoltageArr);
            //ADCVoltage = ADCMean(ADCBuff);
            break;
    }
    index = index_new; // retain index for next cycle
}


int main(void)
{
    while(1)
    {
        ADCUpdate();
    }

}











