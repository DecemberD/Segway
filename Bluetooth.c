/**********************************************************************c
* Author: Marcin Dec
* Date: 01.02.2015
* FileName:        Bluetooth.c
* Dependencies:    Header Bluetooth.h
* Processor:       dsPIC30F6013A
* Compiler:        MPLAB® C30 v3.00 or higher
*
************************************************************************/
#include <p30f6013A.h>
#include "Bluetooth.h"
#include "Segway.h"
#include "Globals.h"

//// Uart Boud Rate in bit/s  //////////////////////////////////////////
#define UART1_BOUD_RATE 9600L
//// Uart Boud Rate Generator register value  //////////////////////////
#define BRG F_CY/16/UART1_BOUD_RATE-1
//// Uart connection timeout in miliseconds  ///////////////////////////
#define TIMEOUT 2000L
//// Uart connection timeout normalized value //////////////////////////
#define ST_TIMEOUT TIMEOUT*UART1_BOUD_RATE/10000
//// Uart Transmiter Buffer Length /////////////////////////////////////
#define TX_BUFFER_LENGTH 64
//// Uart Receiver Buffer Length ///////////////////////////////////////
#define RX_BUFFER_LENGTH 64

static unsigned char TxFrameBuffer[TX_BUFFER_LENGTH];					// Uart Transmiter Buffer
static unsigned int TxFrameBufferIndex;									// Uart Transmiter Buffer index
static unsigned char RxFrameBuffer[TX_BUFFER_LENGTH];					// Uart Receiver Buffer
static unsigned int RxFrameBufferIndex;									// Uart Receiver Buffer index
static unsigned char AckFrame[] = {0x01, 0x01};
static unsigned char NackFrame[] = {0x01, 0x02};
static unsigned int AckFlag;
static unsigned int NackFlag;
static unsigned int RxStartFlag;
static unsigned int ConnectedFlag;
static unsigned int PingFlag;

///////// UART1 Transmitter Interrupt routine //////////////////////
void __attribute__((__interrupt__,no_auto_psv)) _U1TXInterrupt(void);

///////// UART1 Receiver Interrupt routine //////////////////////
void __attribute__((__interrupt__,no_auto_psv)) _U1RXInterrupt(void);



///////// UART1 Transmitter Interrupt routine //////////////////////
void __attribute__((__interrupt__,no_auto_psv)) _U1TXInterrupt(void)
{		
	IFS0bits.U1TXIF = 0; 			// Clear TX interrupt flag
	
}
///////// UART1 Receiver Interrupt routine //////////////////////
void __attribute__((__interrupt__,no_auto_psv)) _U1RXInterrupt(void)
{	
	unsigned char RxByte;
	static unsigned int Length = 0;
	IFS0bits.U1RXIF = 0; 			// Clear RX interrupt flag

	RxByte = U1RXREG;
	if(RxStartFlag == 1)								// enters here only when RxByte contains second byte of received frame with Length property
	{
		Length = (unsigned int)RxByte;					// retain Length property used to iterate through the RxFrameBuffer
		RxFrameBuffer[RxFrameBufferIndex++] = RxByte;	// write it also to first byte of the RxFrameBuffer (not used enywhere, but coppied just in case)
		RxStartFlag = 0;								// prevent entering to this block of code when reads rest of frame
	}
	else if(RxByte == 0x7E)								// enters here only at the begining of a frame
	{
		RxStartFlag = 1;								// prepares for next byte redout being Length property
	}
	else												// enters here when Lenght property is known to the end of the frame
	{
		CalculateRxCRC(Length, RxByte);					// Calculates CRC and decodes frame on the fly
	}
	if(U1STAbits.OERR == 1) U1STAbits.OERR = 0; // Clear buffer overflow Error if present
}
void BluetoothInit(void) {
/////////////// UART1 common for TX and RX Setup  ////////////////////////////////////////////////////////////////////
		U1MODEbits.PDSEL = 0;		// Parity and Data Selection bits
									// "11" - 9-bit data, no parity
									// "10" - 8-bit data, odd parity
									// "01" - 8-bit data, even parity
									// "00" - 8-bit data, no parity
	 	U1MODEbits.STSEL = 0;		// Stop Selection bit
									// "1" - 2 Stop bits
									// "0" - 1 Stop bit
		U1BRG = BRG;			// set Boud Rate Generator to macro BRG (see define list on the top of a file)
		U1MODEbits.UARTEN = 1;		// Enable UART1 (pins U1RX and U1TX)				
///////// Transmiter Setup /////////////////////////////////////////////////////
		U1STAbits.UTXISEL = 1;		// Transmission Interrupt Mode Selection bit
									// "1" - Interrupt when a character is transferred to the Transmit Shift register and as result, the transmit buffer
									// becomes empty
									// "0" - Interrupt when a character is transferred to the Transmit Shift register (this implies that there is at least
									// one character open in the transmit buffer)							
		IEC0bits.U1TXIE = 1;		// Transmiter Interrupt Enable
		IPC2bits.U1TXIP = 2;		// Set TX Interrupt priority (1-lowest, 7-highiest)
		U1STAbits.UTXEN = 1;		// Enable Transmiter
		U1TXREG = 0x00;				// Transmit first byte to trigger transmit interrupt
///////// Receiver Setup /////////////////////////////////////////////////////
		U1STAbits.URXISEL = 0;		// "0" or "1" - Receiver interrupt every one byte
									// "2" - Receiver interrupt every 3 bytes
									// "3" - Receiver interrupt every 4 bytes
		IEC0bits.U1RXIE = 1;		// Receiver Interrupt Enable
		IPC2bits.U1RXIP = 1;		// Set RX Interrupt priority (1-lowest, 7-highiest)
}



