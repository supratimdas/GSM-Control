/* Name: sim300.h
 * Project: GSMControl home sms based home automation system
 * Author: Supratim Das
 * Creation Date: 2012-01-29
 * Tabsize: 4
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

/*Author: Supratim Das
*Institute: Calcutta Institute of Engineering and Management
*version: 20110826
*description: sim300 driver header
*/

/*This driver api is a part of Remote GSM based power management and intrusion detection system. Final year B.tech project by the
*author
*/

/*this header file contains definition of various constants, buffers and constants for the sim300 gsm module. This header can be considered a basic driver
*module for the sim300 hardware. USART communication and and other interrupt service routines for the message reception from the sim300 has been incorporated
*this file. the implementer must put the definition of the function processRequest() in the main application code, as per his/her needs. Whenever sim300
*sends a message, upon reception, the message is entirely stored in a buffer, and processRequest() function is invoked. Apart from this the implementer must
*keep in his/her mind that, he should call from the main application code the initialisation code for the sim300 initSim300(), as well as sim300Poll(). This
*driver is largely based on USART_RXC interrupt and TIMER0_OVF interrupt. Global interrupts are automatically enabled in the initialisation code, so one should
*keep in mind not to disable global interrupts from the application code after initSim300(), or else the driver will not work.*/

#ifndef _SIM_300_H_
#define _SIM_300_H_

#ifndef _AVR_IO_H_
#include<avr/io.h>
#endif

#ifndef F_CPU
#define F_CPU 12000000l
#endif

#ifndef _UTIL_DELAY_H_
#include<util/delay.h>
#endif

#ifndef _AVR_INTERRUPT_H
#include<avr/interrupt.h>
#endif

#ifndef _STDLIB_H_
#include<stdlib.h>
#endif

#ifndef _AVR_WDT_H
#include<avr/wdt.h>
#endif

#ifndef _STRING_H_
#include<string.h>
#endif

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#include "lcd.h"
#endif

#ifndef USE_WATCHDOG
#define USE_WATCHDOG 0	//define this to 1 if watchdog timer is used! set the timer to WDTO_2S in the program
#endif	//also in the program in the infinite loop, call wdt_reset() in short times ~ 250ms

#define SET_BIT(x)	(1<<(x))	//sets bit at position x
#define RESET_BIT(x)	(~SET_BIT(x))	//resets bit at position x

#define TX_ON	UCSRB |=SET_BIT(TXEN)
#define TX_OFF	UCSRB &=RESET_BIT(TXEN)
#define RX_ON	UCSRB |=SET_BIT(RXEN)
#define RX_OFF	UCSRB &=RESET_BIT(RXEN)
#define RX_INTR_DISABLE UCSRB &= RESET_BIT(RXCIE)
#define RX_INTR_ENABLE UCSRB |= SET_BIT(RXCIE)

#define STOP_TIMER0 TCCR0=0x00
#define START_TIMER0 TCCR0=(1<<CS02)|(1<<CS00)	//clock prescaler --> 1024
#define RESET_TIMER0 TCNT0=0x00
#define BUFF_MAX 80
#define TRUE 1
#define FALSE 0

#define STANDARD_MS_DELAY 250
#define STANDARD_US_DELAY 500

char buffer[BUFF_MAX];	//recieve buffer max 128 characters
unsigned char buffLength=0;	//length of message recieved
char incomingMsg=FALSE;	//status flag to show inbound message

#if DEBUG
lcdline buff[2];
lcdInit();
lcdClearScreen(buff);
#endif

//if you don't intend to send sms in any part of the program then assign FALSE value to it
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*implement this function in the main application as per need. when this function is called expect a string in the buffer[] array*/
extern void processRequest();	//one must reset buffLength back to 0, for proper reception of next messages
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void delayMs(int ms)	//watchdog safe delay routine
{
	ms/=2;
	while(ms--)
	{
		#if USE_WATCHDOG
			wdt_reset();
		#endif
		_delay_ms(2);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this is the timer initialisaton code*/
void initTimer0()	//ok
{
	STOP_TIMER0;
	RESET_TIMER0;
	TIFR|=1<<TOV0;	//clear interrupt
	TIMSK|=1<<TOIE0;	//enable overflow interrupt
	incomingMsg=FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void timer0Interrupt()	//actual implementaion of the TIMER)_OVF_INTERRUPT service routine
{
	initTimer0();
	buffer[buffLength++]='\0';
	delayMs(STANDARD_MS_DELAY);
	if(!strncmp(buffer,"\n+CMT",5))
		processRequest();	//call process request to take required actions
	delayMs(STANDARD_MS_DELAY);
	buffLength=0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*interrupt service routine for timer0 overflow*/
ISR(TIMER0_OVF_vect)	//ok
{
	timer0Interrupt();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*USART initialisation code. this function accepts baud rate as the only parameter. Recommended to set baud rate as 4800bps @12Mhz crystal. The baud
*is set using U2X bit*/
void usartInit(unsigned int baud_rate)	//ok
{
	baud_rate=baud_rate/2;
	int ubrr=(int)(F_CPU/(16*baud_rate))-1;
	UCSRA = 0x62;	//set RXC bit, UDRE bit,U2X bit
	UCSRB = 0x98;	//enable receiver,transmitter,interrupt on receive,
	UCSRC = 0x86;	//8 bit data width, no parity
	UCSRC &= 0x7f;
	UBRRH=ubrr>>8 ;
	UBRRL=ubrr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void rxcInterrupt()	//actual implementation of the USART_RXC_INTERRUPT service routine
{
	if(buffLength>=(BUFF_MAX-2))//boundary checking
		buffLength=0;
	buffer[buffLength++]=UDR;
	if(buffer[buffLength-1]=='\r')	//ignore carriage return responses
		buffLength--;
	RESET_TIMER0;	//reset timer to indicate message reception not complete
	if(incomingMsg==FALSE)	//indicates message transfer initiation
	{
		incomingMsg=TRUE;	//change status of flag variable to indicate incoming message
		START_TIMER0;	//start the timer, timer overflow condition indicates message reception complete
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*Interrupt service routine for USART_RXC interrupt.*/
ISR(USART_RXC_vect)	//ok
{
	rxcInterrupt();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this function writes a byte of data to the usart*/
void usartWrite(char ch)	//ok
{
	loop_until_bit_is_set(UCSRA,UDRE);	//wait for previous transmission request to complete
	UDR=ch;	//copy data to Usart Data Register to initiate transfer of data
	buffLength=0;
	return;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this function sends a string to the usart for sending*/
void sendResponse(char* msg)	//ok
{
	unsigned char i=0;
	while(msg[i])	//send data untill null character
	{
		usartWrite(msg[i++]);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this is the sim300 initialisation code. this must be called from the application program. this initialises the USART @ 4800bps baud and also performs
*a synchronisation with the sim300. finally it disables the echo mode*/
int sim300Init()
{
	unsigned char i;
	int sigStrength=0;
	char sig[3];

#if USE_WATCHDOG
	wdt_disable();
#endif

#if DEBUG
	lcdWriteString("\nsim300Init()");
#endif

	usartInit(4800);	//initialise usart @ 4800 baud
	sei();	//enable global interrupts
	for(i=0;i<64;i++)
	{
		usartWrite('U');	//synchronisation sequence
	}
	usartWrite('\r');
	_delay_ms(500);
	sendResponse("at\r");	//dummy at command
	_delay_ms(STANDARD_MS_DELAY);

#if DEBUG
	buffer[buffLength++]='\0';
	lcdClearScreen(buff);
	lcdWriteString(buff,buffer);
#endif

	sendResponse("ate0\r");	//disable echo from sim300
	_delay_ms(STANDARD_MS_DELAY);

#if DEBUG
	buffer[buffLength++]='\0';
	lcdClearScreen(buff);
	lcdWriteString(buff,buffer);
#endif

	sendResponse("at+cmgf=1\r");	//enable text mode
	_delay_ms(STANDARD_MS_DELAY);

#if DEBUG
	buffer[buffLength++]='\0';
	lcdClearScreen(buff);
	lcdWriteString(buff,buffer);
#endif

	sendResponse("at+cnmi=1,2,0,0,0\r");	//enable auto recieve sms mode
	_delay_ms(STANDARD_MS_DELAY);

#if DEBUG
	buffer[buffLength++]='\0';
	lcdClearScreen(buff);
	lcdWriteString(buff,buffer);
#endif

	buffLength=0;

#if USE_WATCHDOG
	wdt_enable(WDTO_2S);
#endif
	while(!sigStrength)	//wait for network access and retrieve signal strength
	{
		sendResponse("at+csq\r");
		_delay_ms(250);
		sig[0]=buffer[7];
		sig[1]=buffer[8];
		sig[2]='\0';
		sigStrength=atoi(sig);
		if(!(sigStrength>0 && sigStrength<32))
			sigStrength=0;
		else
			sigStrength+=10;
		sigStrength=sigStrength/10;
		buffLength=0;
	}
	delayMs(500);
	initTimer0();	//initialise timer0, basically enable timer0 overflow interrupt
	return sigStrength;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this function takes as input two string parameters, first is the mobile number and second is the message and sends the sms to the given number
*NOTE: mobile number must be of the format "+XXXXXXXXXXXX" not including the qoutes*/
void sendSms(char* mobNo,char* message)	//ok
{
	char end=26;
	char* command="at+cmgs=";
	sendResponse(command);
	sendResponse("\"");
	sendResponse(mobNo);
	sendResponse("\"");
	usartWrite('\r');
	_delay_us(100);
	sendResponse(message);
	usartWrite(end);
	usartWrite('\r');
	delayMs(2000);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*this function checks the signal strength*/
int checkSignalStrength()
{
	char sig[3];
	int sigStrength;
	sendResponse("at+csq\r");
	sig[0]=buffer[7];
	sig[1]=buffer[8];
	sig[2]='\0';
	sigStrength=atoi(sig);
	if(!(sigStrength>0 && sigStrength<32))
		sigStrength=0;
	else
		sigStrength+=10;
	sigStrength=sigStrength/10;
	return sigStrength;	
}
#endif
