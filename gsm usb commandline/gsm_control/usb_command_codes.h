#ifndef _USB_INTERFACE_COMMAND_H_
#define _USB_INTERFACE_COMMAND_H_
#endif
/*this is the command code the device is supposed to respond over the USB interface
*NOTE: THIS FILE IS TO BE INCLUDED IN BOTH DEVICE AND HOST SIDE
*C CODE*/

#define GET_MOB_NO	0
#define SET_MOB_NO	1
#define GET_DEV_ID		2
#define SET_DEV_ID		3
#define SYSTEM_CHECK	4
#define GET_LOG		5
#define CLEAR_LOG		6
#define GET_DEV_STATUS 7
#define SET_DEV_STATUS 8