/**********************************************************************c
* Author: Marcin Dec
* Date: 14.02.2015
* FileName:        MPU6050.h
* Dependencies:    MPU6050.c
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® C30 v3.00 or higher
*
************************************************************************/

#define F_XT 7372800L
#define PLL 4L
#define POST 1L
#define F_OSC F_XT*PLL/POST
#define F_CY F_OSC/4

#define I2C_BOUD_RATE 380000L
#define I2C_BRG (F_CY/I2C_BOUD_RATE - F_CY/1111111) - 1
#define TIMEOUT 2000L
#define ST_TIMEOUT TIMEOUT*UART1_BOUD_RATE/10000

#define TX_BUFFER_LENGTH 64
#define RX_BUFFER_LENGTH 64