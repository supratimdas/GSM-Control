/* Name: gsm_control_with_usb.c
 * Project: GSMControl home sms based home automation system
 * Author: Supratim Das
 * Creation Date: 2012-01-29
 * Tabsize: 4
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */

#include <avr/io.h>
#include <avr/interrupt.h>  /* for sei() */
#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/eeprom.h>
#define USE_WATCHDOG 1
#include "lcd.h"
#include "sim300.h"
#include <string.h>
#include "usb_command_codes.h"
#include "usbdrv/usbdrv.h"

#define MAX_DEVICE 4
#define EEPROM_DEVICE_START_ADDR	0x000010
#define EEPROM_DEVICE_OFFSET 0x000008
#define EEPROM_MOB_ADDR	0x000000
#define EEPROM_MOB_LEN 14
#define EEPROM_DEVICE_STATUS_ADDR (EEPROM_DEVICE_START_ADDR + (EEPROM_DEVICE_OFFSET*MAX_DEVICE))

lcdline lcdBuff[2];	//lcd output buffer

#define STOP_TIMER1 TCCR1B=0x00
#define START_TIMER1 TCCR1B=0x05
#define RESET_TIMER1 TCNT1=0x0000
#define OFF_TIMER 2
#define ON_TIMER 1
#define NO_TIMER 0
#define ON 1
#define OFF 0


////////////////////////////////////////////////////////////////////////////////////
////////////////////////CONTROLLER INTERFACE////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

char device[MAX_DEVICE][EEPROM_DEVICE_OFFSET];
char mobNo[14]="";
unsigned int timerStatus[MAX_DEVICE];
unsigned char timerMask[MAX_DEVICE];
unsigned char outputStatus;
long int pollTime=20000;
unsigned int securityTimer;


void switchPortPin(unsigned char pin,unsigned char action)
{
	if(action) //on
	{
		PORTC|=1<<pin;
	}
	else	//off
	{
		PORTC&=~(1<<pin);
	}
	outputStatus=PORTC;
	eeprom_write_byte((uint8_t*)EEPROM_DEVICE_STATUS_ADDR,0x0f & outputStatus);
}

void initTimer1()	//initialisation of 16bit timer/counter 
{
	TIMSK|=1<<OCIE1A;	//enable compare match A interrupt
	TIFR|=1<<OCF1A;	//clear interrupt flag
	OCR1A=0xe4e2;	//compare match every 5 secs --> experimentally determined for accuracy
	TCCR1A=0x00;
	STOP_TIMER1;
}

void timer1Interrupt()
{
	unsigned char i,required=0;
	RESET_TIMER1;
	//lcdWriteString(lcdBuff,"t1 isr-->");
	for(i=0;i<MAX_DEVICE;i++)	//scan through all the timer variables of devices attached
	{
		if(timerStatus[i])	//if variable is non zero
		{
			timerStatus[i]--;
			required++;	//this variable is used to decide whether to keep the timer running
		}
		else	//timer timeout
		{
			switch(timerMask[i])	//check the timer mask of the device in question
			{
				case ON_TIMER:
					switchPortPin(i,ON);
					break;
				case OFF_TIMER:
					switchPortPin(i,OFF);
					break;
				default:
					break;
			}
		}
	}
	if(securityTimer)
	{
		required++;
		securityTimer--;
	}
	else
	{
		GICR|=1<<INT1;	//re-enable int1 ext interrupt!
	}
	if(!required)	//if no device is having a pending timer
		STOP_TIMER1;	//shut down timer
}

ISR(TIMER1_COMPA_vect)	//called every 5 secs
{
	timer1Interrupt();
}

ISR(INT1_vect)
{
	GICR&=~(1<<INT1);	//disable int1 ext interrupt
	securityTimer=12*2;	//setting for re-enabling security timer after 2 mins
	lcdClearScreen(lcdBuff);
	lcdWriteString(lcdBuff,"ALERT! INTRUSION");
	sendSms(mobNo,"ALERT! INTRUSION DETECTED!");
	lcdWriteString(lcdBuff,"ALERT SMS SENT");
	START_TIMER1;
}

int controller()
{
	unsigned short int i,sigStrength;
	char sigVal[16];
	lcdWriteString(lcdBuff,"GSM Control 1.0\ninitialising...");
	_delay_ms(2000);
	initTimer1();
	sigStrength=sim300Init();
	/*read data from non volatile eeprom*/
	eeprom_read_block((void*)mobNo,(const void*)EEPROM_MOB_ADDR,EEPROM_MOB_LEN);
	mobNo[EEPROM_MOB_LEN-1]='\0';
	for(i=0;i<MAX_DEVICE;i++)
	{
		eeprom_read_block((void*)device[i],(const void*)(EEPROM_DEVICE_START_ADDR+(i*EEPROM_DEVICE_OFFSET)),EEPROM_DEVICE_OFFSET);
	}
	outputStatus=eeprom_read_byte((uint8_t*)EEPROM_DEVICE_STATUS_ADDR);
	
	DDRC=0xff;	//configure output ports
	PORTC=0x0f & outputStatus;	

	MCUCR&=~((1<<ISC11)|(1<<ISC10));
	GICR|=1<<INT1;	//enable int1 external interrupt; level trigerred

	delayMs(2000);
	//sendSms(mobNo,"GSM Control online!");
	for(;;)
	{
	CHECK_AGAIN:
		sigStrength=checkSignalStrength();
		lcdClearScreen(lcdBuff);
		lcdWriteString(lcdBuff," System  Online");
		lcdWriteString(lcdBuff,"\nsignal: ");
		switch(sigStrength)
		{
			case 1:
				lcdWriteString(lcdBuff,"WEAK");
				break;
			case 2:
				lcdWriteString(lcdBuff,"MEDIUM");
				break;
			case 3:
				lcdWriteString(lcdBuff,"STRONG");
				break;
			default:
				lcdWriteString(lcdBuff,"UNKNOWN");
				lcdWriteString(lcdBuff,buffer);
				delayMs(1000);
				goto CHECK_AGAIN;
		}
		delayMs(pollTime);
	}
}

int countTokens(char* str)	//this function counts the number of tokens in the command string
{
	unsigned short int i=0;
	unsigned int short count=0;
	unsigned char space=FALSE;
	while(str[i])
	{
		if(!space && str[i]==' ')
		{
			count++;
			space=TRUE;
		}
		else if(str[i]!=' ')
		{
			space=FALSE;
		}
		i++;
	}
	return count+1;
}

void processRequest()	//application specific logic is implemented here, rest is handled by the embedded OS
{
	char *tmp;
	char *token[4];
	unsigned short int count;
	unsigned short int i;
	pollTime+=10000;
	tmp=strtok(buffer,"\"");
	tmp=strtok(NULL,"\"");
	if(!strncmp(mobNo,tmp,13))	//valid number
	{
		tmp=strtok(NULL,"\n");
		tmp=strtok(NULL,"\n");	//now tmp contains the command
		tmp=strupr(tmp);
		count=countTokens(tmp);
		for(i=0;i<count;)	//extract tokens
		{
			if(i==0)
				token[i]=strtok(tmp," ");
			else
				token[i]=strtok(NULL," ");
			if(token[i][0]==' ')
				continue;
			i++;
		}
		buffer[0]='\0';
		switch(count)
		{
			case 1:
				if(!strcmp(token[0],"RESET") || !strcmp(tmp,"RESET"))
				{
					lcdWriteString(lcdBuff,"\nDEVICE RESET");
					_delay_ms(3000);	//reset using watchdog!
				}
				break;
			case 2:
				if(!(strcmp(token[0],"SHOW") || strcmp(token[1],"STATUS")))
				{
					lcdWriteString(lcdBuff,"\nshow status cmd");
					for(i=0;i<MAX_DEVICE;i++)
					{
						strcat(buffer,device[i]);
						if(outputStatus & (1<<i))
						{
							strcat(buffer,":ON\r");
						}
						else
						{
							strcat(buffer,":OFF\r");
						}
					}
					count=0;
				}
				break;
			case 3:
				if(!(strcmp(token[0],"SET") || strcmp(token[2],"ON")))
				{
					lcdWriteString(lcdBuff,"\ndevice on cmd");
					for(i=0;i<MAX_DEVICE;i++)
					{
						if(!(strcmp(token[1],device[i])))
						{
							switchPortPin(i,ON);
							strcat(buffer,device[i]);
							strcat(buffer," TURNED ON");
							count=0;
							break;
						}
					}
				}
				else if(!(strcmp(token[0],"SET") || strcmp(token[2],"OFF")))
				{
					lcdWriteString(lcdBuff,"\ndevice off cmd");
					for(i=0;i<MAX_DEVICE;i++)
					{
						if(!(strcmp(token[1],device[i])))
						{
							switchPortPin(i,OFF);
							strcat(buffer,device[i]);
							strcat(buffer," TURNED OFF");
							count=0;
							break;
						}
					}
				}
				break;
			case 4:
				if(!(strcmp(token[0],"SET") || strcmp(token[2],"ON")))
				{
					lcdWriteString(lcdBuff,"\ndevice on timer cmd");
					for(i=0;i<MAX_DEVICE;i++)
					{
						if(!(strcmp(token[1],device[i])))
						{
							switchPortPin(i,OFF);
							timerMask[i]=ON_TIMER;
							timerStatus[i]=atoi(token[3])*12;
							strcat(buffer,device[i]);
							strcat(buffer," ON TIMER SET FOR ");
							strcat(buffer,token[3]);
							strcat(buffer," mins");
							count=0;
							break;
						}
					}
				}
				else if(!(strcmp(token[0],"SET") || strcmp(token[2],"OFF")))
				{
					lcdWriteString(lcdBuff,"\ndevice off timer cmd");
					for(i=0;i<MAX_DEVICE;i++)
					{
						if(!(strcmp(token[1],device[i])))
						{
							switchPortPin(i,ON);
							timerMask[i]=OFF_TIMER;
							timerStatus[i]=atoi(token[3])*12;
							strcat(buffer,device[i]);
							strcat(buffer," OFF TIMER SET FOR ");
							strcat(buffer,token[3]);
							strcat(buffer," mins");
							count=0;
							break;
						}
					}
				}
				START_TIMER1;
				break;
			default:
				break;
		}
		if(!count)
		{
			delayMs(250);
			lcdClearScreen(lcdBuff);
			lcdWriteString(lcdBuff,buffer);
			sendSms(mobNo,buffer);
		}
		else
		{
			delayMs(250);
			lcdClearScreen(lcdBuff);
			lcdWriteString(lcdBuff,"error parsing \ncommand");
			sendSms(mobNo,"error parsing command");
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

unsigned char dataLength,requestType;
void *eepromAddress;
uchar usbFunctionWrite(uchar *data, uchar len)		//this function is called in chunks of 8 bytes to transmit data from host--->device	(device write function)
{
	lcdWriteString(lcdBuff,"\nusb write...");
	switch(requestType)
	{
		case SET_MOB_NO:
		case SET_DEV_ID:
		case SET_DEV_STATUS:
			eeprom_write_block(data,eepromAddress,len);
			break;
		default:
			return -1;
			break;
	}
	dataLength-=len;
	eepromAddress+=len;
	if(dataLength)
		return 0;
	else
		return 1;

}


uchar usbFunctionRead(uchar *data, uchar len)		//this function is called in chunks of 8 bytes to transmit data from device----->host (device read function)
{
	lcdWriteString(lcdBuff,"\nusb read...");
	switch(requestType)
	{
		case GET_MOB_NO:
		case GET_DEV_ID:
		case GET_DEV_STATUS:
			eeprom_read_block(data,eepromAddress,len);
			break;
		default:
			return -1;
			break;
	}
	dataLength-=len;
	eepromAddress+=len;
	return len;
}


USB_PUBLIC usbMsgLen_t  usbFunctionSetup(uchar data[8])	//this function is called at the start of each control transfer, appropriate flag variables are set in this function
{
	usbRequest_t    *rq = (void *)data;
	unsigned char valueType;
	dataLength=rq->wLength.bytes[0];
	requestType=rq->bRequest;
	valueType=rq->wValue.bytes[0]-1;
	switch(requestType)
	{
		case GET_MOB_NO:
		case SET_MOB_NO:
			eepromAddress=(void*)EEPROM_MOB_ADDR;
			break;
		case GET_DEV_ID:
		case SET_DEV_ID:
			eepromAddress=(void*)EEPROM_DEVICE_START_ADDR+(valueType*EEPROM_DEVICE_OFFSET);
			break;
		case GET_DEV_STATUS:
		case SET_DEV_STATUS:
			eepromAddress=(void*)EEPROM_DEVICE_STATUS_ADDR;
			break;
		default:
			break;
	}
	return 0xff;
}

/* ------------------------------------------------------------------------- */



int	main(void)		//the main function which does usb initialisation and polling
{
	uchar   i;
	controller();
	usbInit();
	usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
	i = 0;
	while(--i)
	{             /* fake USB disconnect for > 250 ms */
	    _delay_ms(1);
	}
	usbDeviceConnect();
	sei();
	for(;;)
	{                /* main event loop */
		usbPoll();
		if(!i)
		{
			lcdClearScreen(lcdBuff);
			lcdWriteString(lcdBuff,"USB ready...");
		}
		i--;
		_delay_ms(50);
	}
	return 0;
}

/* ------------------------------------------------------------------------- */


/*int main()
{
	DDRC=0xdf;
	PORTC=0x40;
	lcdInit();
	wdt_disable();
	_delay_ms(20);
	//if(PINC & 0x20)
		usb();
	//controller();
	return 0;
}*/
