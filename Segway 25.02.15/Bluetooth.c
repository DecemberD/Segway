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

///////// UART1 Ping function //////////////////////
static void Ping(unsigned int incrementValue);

///////// Calculates CRC over Tx frame and prepares it to transmision //////////////////////////
static int CalculateTxCRC(unsigned char* hexData);
///////// Removes staffed bytes and Calculates CRC over Rx frame  //////////////////////////
static void CalculateRxCRC(unsigned int length, unsigned char byte);


///////// UART1 Transmitter Interrupt routine //////////////////////
void __attribute__((__interrupt__,no_auto_psv)) _U1TXInterrupt(void)
{	
	static int ReteinedIndex = 0;
	unsigned int i;
	IFS0bits.U1TXIF = 0; 			// Clear TX interrupt flag
	if(ReteinedIndex != -1)
	{
		for(i = 0; i < 4; i++)
		{
			U1TXREG = TxFrameBuffer[TxFrameBufferIndex++];
			if(TxFrameBufferIndex > ReteinedIndex)
			{
				break;
			}
		}
	}
	else
	{
		// for the future, if there is nothing to broadcast
	}
	Ping(i);
	if(TxFrameBufferIndex > ReteinedIndex)
	{
		if(AckFlag == 1)
		{
			ReteinedIndex = CalculateTxCRC(AckFrame);
			TxFrameBufferIndex = 0;
			AckFlag = 0;
		}
		else if(NackFlag == 1)
		{
			ReteinedIndex = CalculateTxCRC(NackFrame);
			TxFrameBufferIndex = 0;
			NackFlag = 0;
		}
		else
		{
			ReteinedIndex = CalculateTxCRC(GetNextFrame());
			TxFrameBufferIndex = 0;
		}
	}
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
		Length = (unsigned int)RxByte;					// retein Length property used to iterate through the RxFrameBuffer
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
int CalculateTxCRC(unsigned char* hexData)
        {
			unsigned int i;
			if(hexData == 0)
			{
				return -1;
			}
            unsigned char Length = hexData[0] + 1;
            unsigned char Sum1 = 0x00;
            unsigned char Sum2 = 0x00;
            for (i = 1; i < Length; i++)
            {
                if ((0xFF - Sum1) < hexData[i])
                {
                    Sum1++;
                }
                Sum1 = (Sum1 + hexData[i]);

                if (Sum1 == 0xFF)
                {
                    Sum1 = 0;
                }
                if ((0xFF - Sum2) < Sum1)
                {
                    Sum2++;
                }
                Sum2 = (Sum2 + Sum1);
                if (Sum2 == 0xFF)
                {
                    Sum2 = 0;
                }
            }
            
            for (i = 0, TxFrameBufferIndex = 0; i < Length; i++, TxFrameBufferIndex++)
            {
                if (hexData[i] == 0x7E)
                {
                    TxFrameBuffer[TxFrameBufferIndex++] = 0x7D;
                    TxFrameBuffer[TxFrameBufferIndex] = 0x5E;
                }
                else if(hexData[i] == 0x7D)
                {
					TxFrameBuffer[TxFrameBufferIndex++] = 0x7D;
                    TxFrameBuffer[TxFrameBufferIndex] = 0x5D;
                }
				else
				{
					TxFrameBuffer[TxFrameBufferIndex] = hexData[i];
				}

            }

            if (Sum1 == 0x7E)
            {
                TxFrameBuffer[TxFrameBufferIndex++] = 0x7D;
               	TxFrameBuffer[TxFrameBufferIndex++] = 0x5E;
            }
            else if (Sum1 == 0x7D)
            {
                TxFrameBuffer[TxFrameBufferIndex++] = 0x7D;
               	TxFrameBuffer[TxFrameBufferIndex++] = 0x5D;
            }
            else
            {
                TxFrameBuffer[TxFrameBufferIndex++] = Sum1;
            }
            if (Sum2 == 0x7E)
            {
                TxFrameBuffer[TxFrameBufferIndex++] = 0x7D;
               	TxFrameBuffer[TxFrameBufferIndex++] = 0x5E;
            }
            else if (Sum2 == 0x7D)
            {
                TxFrameBuffer[TxFrameBufferIndex++] = 0x7D;
               	TxFrameBuffer[TxFrameBufferIndex++] = 0x5D;
            }
            else
            {
                TxFrameBuffer[TxFrameBufferIndex++] = Sum2;
            }
			TxFrameBuffer[TxFrameBufferIndex] = 0x7E;
			return TxFrameBufferIndex;
        }
void CalculateRxCRC(unsigned int length, unsigned char byte)
{	
			// Calculates CRC and decodes frame on the fly
			static int StaffedByteFlag = 0;
			static int DataEndFlag = 0;
            static unsigned char Sum1 = 0x00;
            static unsigned char Sum2 = 0x00;

		if(byte == 0x7D)							// enters here when staffed bytes discovered ( staffed byte sequence begins with 0x7D
		{
			StaffedByteFlag = 1;					// set flag to indicate that next byte belonges to staffed byte sequence
		}
		else if(DataEndFlag == 0)					// enters here when read bytes are data (not CRC) bytes
		{
			if(StaffedByteFlag == 1)				// enters here when previous byte was first from staffed byte sequence
			{
				byte += 0x20;						// in staffed byte sequence first byte (0x7D) is discarded and second byte is increased with 0x20
				StaffedByteFlag = 0;				// reset flag to indicate end of staffed byte sequence
			}
            if ((0xFF - Sum1) < byte)				// calculate CRC Sum1 and Sum2 over each frame byte (command byte to last data byte)
            {										// Sum1 and Sum2 equal to 0 at the begining of the frame and change with every byte
               Sum1++;
            }
            Sum1 = (Sum1 + byte);

           	if (Sum1 == 0xFF)
            {
               Sum1 = 0;
            }
            if ((0xFF - Sum2) < Sum1)
            {
               Sum2++;
            }
            Sum2 = (Sum2 + Sum1);
            if (Sum2 == 0xFF)
            {
                Sum2 = 0;
            }
			RxFrameBuffer[RxFrameBufferIndex++] = byte;	// coppy currently read byte to RxFrameBuffer and increment its index
			if(RxFrameBufferIndex > length)				// enters here when all data (not CRC) byte were read
			{
				DataEndFlag = 1;						// sets flag to indicate that all data bytes were read
			}
		}
		else											// enters here when CRC bytes are being read
        {
			if(StaffedByteFlag == 1)					// enters here when previous byte was firs from staffed byte sequence
			{
				byte += 0x20;							// in staffed byte sequence first byte (0x7D) is discarded and second byte is increased with 0x20
				StaffedByteFlag = 0;					// reset flag to indicate end of staffed byte sequence
			}
			RxFrameBuffer[RxFrameBufferIndex++] = byte;	// coppy currently read byte to RxFrameBuffer and increment its index
			if(RxFrameBufferIndex > length + 2)			// enters here when both CRC bytes were read
			{
				DataEndFlag = 0;						// resets flag to allow for reading data bytes in next frame
				if(Sum1 == RxFrameBuffer[RxFrameBufferIndex - 2] && Sum2 == RxFrameBuffer[RxFrameBufferIndex - 1])	// enters here when CRC Sums are consistent
				{
					AckFlag = 1;						// set flag to indicate that frame reception was correct
					RxFrameBufferIndex = 0;				// set RxFrameBuffer intex to prepare it for reading next frame
					Sum1 = 0x00;						// set both CRC sums to 0 to prepare it for reading next frame
            		Sum2 = 0x00;
					if(RxFrameBuffer[1] == 0x03)		// enters here if decoded frame is Ping frame
					{
						PingFlag = 1;					// set flag to indicate thad connection is alive
					}
					else								// enters here if decoded frame is other than Ping frame
					{
						SegwayUpdate(RxFrameBuffer);	// uses decoded frame to update data according to command passed in the frame
					}
				}
				else 									// enters here when CRS sums are inconsistent
				{
					NackFlag = 1;						// set flag to indicate that frame reception was incorrect
					RxFrameBufferIndex = 0;				// set RxFrameBuffer intex to prepare it for reading next frame
					Sum1 = 0x00;						// set both CRC sums to 0 to prepare it for reading next frame
            		Sum2 = 0x00;
				}
			}
		}

         
}
void Ping( unsigned int incrementValue)
{
	static unsigned long ConnectionTimeOut = 0;
	if(PingFlag == 1)
	{
		PingFlag = 0;
		ConnectedFlag = 1;
		ConnectionTimeOut = 0;
	}
	else
	{
		ConnectionTimeOut += incrementValue;
		if(ConnectionTimeOut > ST_TIMEOUT)
		{
			ConnectedFlag = 0;
			ConnectionTimeOut = 0;
		}
	}
}
