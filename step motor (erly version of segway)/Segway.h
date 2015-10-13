/**********************************************************************c
* Author: Marcin Dec
* Date: 01.02.2015
* FileName:        Segway.h
* Dependencies:    Segway.c
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® C30 v3.00 or higher
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

union tagMPU5060
{
	struct MPU5060_
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
	}Mpu5060_;
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

extern void Mpu5060Init();
extern void PidInit();
extern void KalmanInit();
extern void MotorsInit();
extern void SegwayInit();
extern void SegwayUpdate(unsigned char* frame);

extern int BroadcastAppend(unsigned char* frame);
extern unsigned char* GetNextFrame(void);

