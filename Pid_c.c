/**********************************************************************c
* Author: Marcin Dec
* Date: 11.08.2015
* FileName:        Pid_c.c
* Dependencies:    
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® XC16 C Compiler
*
************************************************************************/
#include <p30f6013A.h>
#include "Pid.h"

struct Pid
{
        // external
	int kp;
	int ki;
	int kd;
	int offset;
	int min;
	int max;
        int inverse;
        // internal
	long long int i;
        int prevIn;
};
struct Pid PidCSAR = {512, 0, 0, 0, 0, 368, 0};
struct Pid PidCSBR = {512, 0, 0, 0, 0, 368, 0};
struct Pid PidCSAL = {10, 10, 10, 0, 0, 30000};
struct Pid PidCSBL = {10, 10, 10, 0, 0, 30000};
struct Pid PidSpeedR = {10, 10, 10, 0, -30000, 30000};
struct Pid PidSpeedL = {10, 10, 10, 0, -30000, 30000};


//int i = 0;
//int in = 0;
//int setp = 0;
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
//
//    while(1)
//    {
//        i = PidExec(in, setp, &PidCSAR);
//        i++;
//    }
//    i++;
//    //i = PidExecu(5, &PidCSAR);
//    return i;
//}
//






