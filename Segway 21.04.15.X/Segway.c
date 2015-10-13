/**********************************************************************c
* Author: Marcin Dec
* Date: 01.02.2015
* FileName:        Segway.c
* Dependencies:    Header Segway.h
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® C30 v3.00 or higher
*
************************************************************************/

#include "Segway.h"

#define BROADCAST_CAPACITY 10

static unsigned char* Broadcast[BROADCAST_CAPACITY];
static int BroadcastCount = 0;
static int BroadcastIndex = 0;

union tagMPU5060 Mpu5060;
union tagPID Pid;
union tagKALMAN Kalman;
union tagMOTORS Motors;

void Mpu5060();
void Pid();
void Kalman();
void Motors();
void SegwayUpdate(unsigned char* frame);
static void ArrayCopy(unsigned char* from, unsigned char* to);

void Mpu5060()
{
	int i;
	Mpu5060.Mpu5060_.Length = MPU5060_LENGTH;
	Mpu5060.Mpu5060_.Command = MPU5060_COMMAND;
	for(i = 2; i <= MPU5060_LENGTH; i++)
	{
		Mpu5060.MPU5060Frame[i] = i;
	}
}
void Pid()
{	
	//int i;
	Pid.Pid_.Length = PID_LENGTH;
	Pid.Pid_.Command = PID_COMMAND;
	//for(i = 2; i <= PID_LENGTH; i++)
	//{
	//	Pid.PidFrame[i] = i + 30;
	//}
	Pid.Pid_.P = -4.23;
	Pid.Pid_.I = 3.447;
	Pid.Pid_.D = 12.633;
}
void Kalman()
{	
	int i;
	Kalman.Kalman_.Length = KALMAN_LENGTH;
	Kalman.Kalman_.Command = KALMAN_COMMAND;
	for(i = 2; i <= KALMAN_LENGTH; i++)
	{
		Kalman.KalmanFrame[i] = i + 30;
	}
}
void Motors()
{	
	int i;
	Motors.Motors_.Length = MOTORS_LENGTH;
	Motors.Motors_.Command = MOTORS_COMMAND;
	for(i = 2; i <= MOTORS_LENGTH; i++)
	{
		Motors.MotorsFrame[i] = i + 30;
	}
}
void Segway()
{
	Mpu5060();
	Pid();
}
void SegwayUpdate(unsigned char* frame)
{
	switch (frame[1])
	{
		case 0x06:
		ArrayCopy(frame, Pid.PidFrame);
		break;
		case 0x08:
		ArrayCopy(frame, Kalman.KalmanFrame);
		break;
		case 0x0A:
		ArrayCopy(frame, Motors.MotorsFrame);
		break;
	}
}

extern int BroadcastAppend(unsigned char* frame)
{
	if(BroadcastCount < BROADCAST_CAPACITY)
	{
		Broadcast[BroadcastCount] = frame;
		BroadcastCount++;
		return 1;
	}
	else
		return 0;
}
extern unsigned char* GetNextFrame(void)
{
	if(BroadcastIndex >= BroadcastCount)
		BroadcastIndex = 0;
	return Broadcast[BroadcastIndex++];
}
static void ArrayCopy(unsigned char* from, unsigned char* to)
{	
	unsigned char i;
	unsigned char Length = from[0];
	for(i = 2; i <= Length; i++)
	{
		to[i] = from[i];
	}
	
}
