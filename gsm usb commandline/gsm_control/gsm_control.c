/* Name: gsm_control.c
 * Project: GSMControl home sms based home automation system
 * Author: Supratim Das
 * Creation Date: 2012-01-29
 * Tabsize: 4
 * License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <usb_command_codes.h>
#include <lusb0_usb.h>    /* this is libusb, see http://libusb.sourceforge.net/ */

#define USBDEV_SHARED_VENDOR    0x16C0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   0x05DC  /* Obdev's free shared PID */
/* Use obdev's generic shared VID/PID pair and follow the rules outlined
 * in firmware/usbdrv/USBID-License.txt.
 */

/* These are the vendor specific SETUP commands implemented by our USB device */

/* PowerSwitch uses the free shared default VID/PID. If you want to see an
 * example device lookup where an individually reserved PID is used, see our
 * RemoteSensor reference implementation.
 */

#define USB_ERROR_NOTFOUND  1
#define USB_ERROR_ACCESS    2
#define USB_ERROR_IO        3

char errorDescription[256];

static int usbOpenDevice(usb_dev_handle **device, int vendor, char *vendorName, int product, char *productName)
{
struct usb_bus      *bus;
struct usb_device   *dev;
usb_dev_handle      *handle = NULL;
int                 errorCode = USB_ERROR_NOTFOUND;
static int          didUsbInit = 0;
static char    string[256];

    if(!didUsbInit){
        didUsbInit = 1;
        usb_init();
    }
    usb_find_busses();
    usb_find_devices();
    for(bus=usb_get_busses(); bus; bus=bus->next){
        for(dev=bus->devices; dev; dev=dev->next){
            if(dev->descriptor.idVendor == vendor && dev->descriptor.idProduct == product){
                int     len;
                handle = usb_open(dev); /* we need to open the device in order to query strings */
                if(!handle){
                    errorCode = USB_ERROR_ACCESS;
					strcpy(errorDescription,"Warning: cannot open USB device:");
                    strcat(errorDescription, usb_strerror());
                    continue;
                }
                if(vendorName == NULL && productName == NULL){  /* name does not matter */
                    break;
                }
                /* now check whether the names match: */
                len = usb_get_string_simple(handle, dev->descriptor.iManufacturer, string, sizeof(string));
                if(len < 0){
                    errorCode = USB_ERROR_IO;
					 strcpy(errorDescription,"Warning: cannot query manufacturer for device:");
                     strcat(errorDescription, usb_strerror());
                }else{
					strcat(errorDescription,"found vendor : ");
					strcat(errorDescription,string);
                    errorCode = USB_ERROR_NOTFOUND;
                    if(strcmp(string, vendorName) == 0){
                        len = usb_get_string_simple(handle, dev->descriptor.iProduct, string, sizeof(string));
                        if(len < 0){
                            errorCode = USB_ERROR_IO;
							strcpy(errorDescription,"Warning: cannot query product for device : ");
                            strcat(errorDescription, usb_strerror());
                        }else{
                            errorCode = USB_ERROR_NOTFOUND;
                            if(strcmp(string, productName) == 0)
							{
								strcat(errorDescription,"\nfound product : ");
								strcat(errorDescription,string);
                                break;
							}
                        }
                    }
                }
                usb_close(handle);
                handle = NULL;
            }
        }
        if(handle)
            break;
    }
    if(handle != NULL){
        errorCode = 0;
        *device = handle;
    }
    return errorCode;
}


int main(int argc, char **argv)
{
	usb_dev_handle      *handle = NULL;
	static unsigned char       buffer[255];
	char choice;
	int                 nBytes,i;

    if(usbOpenDevice(&handle, USBDEV_SHARED_VENDOR, "supratimofficio@gmail.com", USBDEV_SHARED_PRODUCT, "GSM Control") != 0){
        fprintf(stderr, "Could not find USB device \"GSM Control\" with vid=0x%x pid=0x%x\n", USBDEV_SHARED_VENDOR, USBDEV_SHARED_PRODUCT);
		getch();
        exit(1);
    }
	printf("\n%s\n",errorDescription);
/* We have searched all devices on all busses for our USB device above. Now
 * try to open it and perform the vendor specific control operations for the
 * function requested by the user.
 */
	while(1)
	{
		printf("\n\nGSM control main menu :");
		printf("\n1. Get Mobile number");
		printf("\n2. Set Mobile number");
		printf("\n3. Get Device names");
		printf("\n4. Set Device names");
		printf("\n5. Get Device status");
		printf("\n6. Set Device status");
		printf("\n7. Exit\n");
		choice=getch();
		switch(choice)
		{
			case '1':
				nBytes=16;
				nBytes=usb_control_msg(handle,USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,GET_MOB_NO,0,1,buffer,nBytes,5000);
				if(nBytes>=16)
				{
					printf("\nmobile no: \"%s\"",buffer);
				}
				else
				{
					printf("\nusb read error: %s", usb_strerror());
					getch();
				}
				break;
			case '2':
				printf("\nenter mobile number in format \"+xxxxxxxxxxxx\" : ");
				gets(buffer);
				nBytes=strlen(buffer)+1;
				if(nBytes==usb_control_msg(handle,USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,SET_MOB_NO,0,1,buffer,nBytes,5000))
				{
					printf("\nMobile number set successfully");
				}
				else
				{
					printf("\nUsb write error: %s",usb_strerror());
				}
				break;
			case '3':
				for(i=1;i<=4;i++)
				{
					nBytes=8;
					nBytes=usb_control_msg(handle,USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,GET_DEV_ID,i,1,buffer,nBytes,5000);
					if(nBytes<8)
					{
						printf("\nusb read error: %s", usb_strerror());
					}
					else
					{
						printf("\nDEVICE %d :: %s",i,buffer);
					}
				}
				break;
			case '4':
				printf("\nplease identify device by number : ");
				scanf("%d",&i);
				printf("\nplease give a name for the device (within 7 characters): ");
				fflush(stdin);
				gets(buffer);
				nBytes=strlen(buffer);
				nBytes++;
				if(nBytes>8)
				{
					printf("\nname is too big");
					break;
				}
				printf("\n%s %d\n",buffer,nBytes);
				switch(i)
				{
					case 1:
						nBytes=usb_control_msg(handle,USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,SET_DEV_ID,i,1,buffer,nBytes,5000);
						break;
					case 2:
						nBytes=usb_control_msg(handle,USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,SET_DEV_ID,i,1,buffer,nBytes,5000);
						break;
					case 3:
						nBytes=usb_control_msg(handle,USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,SET_DEV_ID,i,1,buffer,nBytes,5000);
						break;
					case 4:
						nBytes=usb_control_msg(handle,USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,SET_DEV_ID,i,1,buffer,nBytes,5000);
						break;
					default:
						printf("\ndevice %d is not in system",i);
						break;
				}
				if(nBytes<0)
				{
					printf("\nUsb write error: %s",usb_strerror());
				}
				break;
			case '5':
				nBytes=usb_control_msg(handle,USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN,GET_DEV_STATUS,0,1,buffer,1,5000);
				if(nBytes)
				{
					for(i=0;i<4;i++)
					{
						if((buffer[0]>>i)&0x01)
						{
							printf("\ndevice %d ON",i+1);
						}
						else
						{
							printf("\ndevice %d OFF",i+1);
						}
					}
				}
				else
				{
					printf("\nusb read error: %s", usb_strerror());
				}
				break;
			case '6':
				printf("\nplease enter the status byte value: ");
				scanf("%d",&nBytes);
				buffer[0]=nBytes;
				nBytes=usb_control_msg(handle,USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,SET_DEV_STATUS,0,1,buffer,1,5000);
				break;
			case '7':
				exit(0);
			default:
				break;
		}
	}
    usb_close(handle);
    return 0;
}