/****************************************************************************

 Copyright (C) 1997 Marius Greuel. All rights reserved.

 Title:		uart.c : Hardware support for the serial port

 Version:	1.00

 Date:		16-Aug-97

 Author:	MG

****************************************************************************/

#include <ntddk.h>
#include "uart.h"

// Get the serial port modem control register
UCHAR UART_GetModemCtrl(IN PUCHAR Port)
{
	return READ_PORT_UCHAR(Port+ACE_MCR);
}

// Set the serial port modem control register
UCHAR UART_SetModemCtrl(IN PUCHAR Port, UCHAR Value)
{
	UCHAR OldValue = UART_GetModemCtrl(Port);
	WRITE_PORT_UCHAR(Port+ACE_MCR, Value);
	return OldValue;
}

// Get the serial port modem status register
UCHAR UART_GetModemStatus(IN PUCHAR Port)
{
	return READ_PORT_UCHAR(Port+ACE_MSR);
}

// Get the complete state of the serial port. May be used for save/restore.
VOID UART_GetState(IN PUCHAR Port, IN PUART_STATE pState)
{
	pState->LineCtrl		= READ_PORT_UCHAR(Port+ACE_LCR);
	WRITE_PORT_UCHAR(Port+ACE_LCR, 0);
	pState->ModemCtrl	 = READ_PORT_UCHAR(Port+ACE_MCR);
	pState->InterruptCtrl = READ_PORT_UCHAR(Port+ACE_IER);
}

// Set the complete state of the serial port. May be used for save/restore.
VOID UART_SetState(IN PUCHAR Port, IN PUART_STATE pState)
{
	WRITE_PORT_UCHAR(Port+ACE_LCR, 0);
	WRITE_PORT_UCHAR(Port+ACE_MCR, pState->ModemCtrl);
	WRITE_PORT_UCHAR(Port+ACE_IER, pState->InterruptCtrl);
	WRITE_PORT_UCHAR(Port+ACE_LCR, pState->LineCtrl);
	// dummy read
	READ_PORT_UCHAR(Port+ACE_MSR);
	READ_PORT_UCHAR(Port+ACE_RBR);
}


