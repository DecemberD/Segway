/**********************************************************************c
* Author: Marcin Dec
* Date: 02.05.2015
* FileName:        Monitor.cpp
* Dependencies:    ??
* Processor:       dsPIC30F6013A
* Compiler:        ??
*
************************************************************************/

#define MPU5060_LENGTH 21
#define MPU5060_COMMAND 4
#define PID_LENGTH 13
#define PID_COMMAND 5
#define KALMAN_LENGTH 7
#define KALMAN_COMMAND 8
#define MOTORS_LENGTH 7
#define MOTORS_COMMAND 10

class   Mpu5060
{

    unsigned char  Length;
    unsigned char  Command;
    int Acc_X;
    int Acc_Y;
    int Acc_Z;
    int Gyr_X;
    int Gyr_Y;
    int Gyr_Z;
    int X;
    int Y;
    int Z;
    int Temp_C;
}
union tagMPU5060
{
	unsigned char MPU5060Frame[MPU5060_LENGTH+1];
};
extern union tagMPU5060 Mpu5060;

union tagPID
{
	struct PID_
	{
		unsigned char Length;
		unsigned char Command;
		float P;
		float I;
		float D;
	}Pid_;
	unsigned char PidFrame[PID_LENGTH+1];
};
extern union tagPID Pid;

union tagKALMAN
{
	struct KALMAN_
	{
		unsigned char Length;
		unsigned char Command;
		int P;
		int I;
		int D;
	}Kalman_;
	unsigned char KalmanFrame[KALMAN_LENGTH+1];
};
extern union tagKALMAN Kalman;

union tagMOTORS
{
	struct MOTORS_
	{
		unsigned char Length;
		unsigned char Command;
		int P;
		int I;
		int D;
	}Motors_;
	unsigned char MotorsFrame[MOTORS_LENGTH+1];
};
extern union tagMOTORS Motors;

extern void Mpu5060();
extern void Pid();
extern void Kalman();
extern void Motors();
extern void Segway();
extern void SegwayUpdate(unsigned char* frame);

extern int BroadcastAppend(unsigned char* frame);
extern unsigned char* GetNextFrame(void);


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

