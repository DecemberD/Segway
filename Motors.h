/**********************************************************************c
* Author: Marcin Dec
* Date: 21.02.2015
* FileName:        Motors.h
* Dependencies:    Motors.c
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® C30 v3.00 or higher
*
************************************************************************/


struct Ticks
{
    int Tick_100: 1;
    int Tick_200: 1;
    int Tick_400: 1;
    int Tick_800: 1;
    int Tick_1600: 1;
    int Tick_3200: 1;
    int Tick_6400: 1;
    int Tick_12800: 1;
    int Tick_25600: 1;
    int Tick_51200: 1;
    int Tick_102400: 1;
    int Tick_204800: 1;
    int Tick_409600: 1;
    int Tick_819200: 1;
    int Tick_1638400: 1;
    int Tick_3276800: 1;
};
extern struct Ticks Tick;
extern void MotorsInit(void);