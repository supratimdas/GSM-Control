/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package usbio;

import ch.ntb.usb.LibusbJava;
import ch.ntb.usb.USB;
import ch.ntb.usb.USBException;
import ch.ntb.usb.Usb_Bus;
import ch.ntb.usb.Usb_Device;

/**
 * @author supratim
 * This Class is contains initialization code and I/O procedures for the GSM control USB device
 * The class also contains the predefined command set for communicating with the device. In the
 * Main program an object of this class needs to be instantiated and corresponding I/O procedures
 * or informative procedures must be invoked to get the job done!
 */
public class USBIO {
    private static short idVendor= 0x16C0;
    private static short idProduct= 0x05DC;
    private static boolean deviceFound=false;
    private static long handle=0;
//The predefined command set that the GSM control usb interface understands
    public static final int GET_MOB_NO=0;
    public static final int SET_MOB_NO=1;
    public static final int GET_DEV_ID=2;
    public static final int SET_DEV_ID=3;
    public static final int SYSTEM_CHECK=4;
    public static final int GET_LOG=5;
    public static final int CLEAR_LOG=6;
    public static final int GET_DEV_STATUS=7;
    public static final int SET_DEV_STATUS=8;

    private Usb_Bus bus;    //Usb_Bus is a linked list of USB busses
    private Usb_Device dev; //Usb_Device is a linked list of USB devices
    private String vendorName;
    private String productName;


    public void open() throws USBException
    {
        LibusbJava.usb_init();  //Libusb init() method initializes various internal data structures and must be called before any other code
        LibusbJava.usb_find_busses();   //finds all USB busses
        LibusbJava.usb_find_devices();  //finds all USB devices
        bus=LibusbJava.usb_get_busses();    //Gets the USB bus previously found
        dev=bus.getDevices();   //retrieve the first libusb device on the usb bus
        while(dev!=null)    //search the entire bus for the correct device
        {
            handle=LibusbJava.usb_open(dev);
            if(LibusbJava.usb_get_string_simple(handle, 2).equals("GSM Control"))   //match device name from device descriptor
            {
                deviceFound=true;
                break;
            }
            dev=dev.getNext();
        }
        if(!deviceFound)
        {
            throw new USBException("Device not found");
        }
 else
        {
            vendorName=LibusbJava.usb_get_string_simple(handle, 1); //get device vendor name from device descriptor
            productName=LibusbJava.usb_get_string_simple(handle, 2);    //get device product name from device descriptor
 }
    }

    public Usb_Bus getBus() {
        return bus;
    }

    public Usb_Device getDev() {
        return dev;
    }

    public static boolean isDeviceFound() {
        return deviceFound;
    }

    public static long getHandle() {
        return handle;
    }

    public static short getIdProduct() {
        return idProduct;
    }

    public static short getIdVendor() {
        return idVendor;
    }

    public String getProductName() {
        return productName;
    }

    public String getVendorName() {
        return vendorName;
    }
/**
 * The usbRead() method is used for a USB read operation on the GSM Control USB interface. USB control
 * message is used for communication. The command is one of the predefined command set previously
 * identified by this class. index is a number between 1-4 identifying the four devices that GSM Control
 * is attached to. Refer the firmware implementation and USB specification for better understanding.
 */
    public String usbRead(int command,int index) throws Exception
    {
        String readStr=null;
        byte[] data=new byte[16];
        switch(command)
        {
            case GET_MOB_NO:
                LibusbJava.usb_control_msg(handle, USB.REQ_TYPE_TYPE_VENDOR | USB.REQ_TYPE_DIR_DEVICE_TO_HOST, GET_MOB_NO, 0, 1, data, 13, 5000);
                readStr=new String(data);
                break;
            case GET_DEV_ID:
                LibusbJava.usb_control_msg(handle, USB.REQ_TYPE_TYPE_VENDOR | USB.REQ_TYPE_DIR_DEVICE_TO_HOST, GET_DEV_ID, index, 1, data, 8, 5000);
                readStr=new String(data);
                break;
            case GET_DEV_STATUS:
                index--;
                LibusbJava.usb_control_msg(handle, USB.REQ_TYPE_TYPE_VENDOR | USB.REQ_TYPE_DIR_DEVICE_TO_HOST, GET_DEV_STATUS, index, 1, data, 8, 5000);
                if(((data[0]>>index)&0x01)==0x01)
                {
                    readStr="ON";
                }
 else
                {
                    readStr="OFF";
 }
                break;
            default:
                throw new Exception("unknown read command");
        }
        return readStr;
    }

    /**
 * The usbWrite() method is used for a USB write operation on the GSM Control USB interface. USB control
 * message is used for communication. The command is one of the predefined command set previously
 * identified by this class. index is a number between 1-4 identifying the four devices that GSM Control
 * is attached to. Refer the firmware implementation and USB specification for better understanding.
 */
    public int usbWrite(int command,String writeStr,int index) throws USBException
    {
        byte[] data=new byte[16];
        byte[] str=writeStr.getBytes();
        for(int i=0;i<16;i++)
        {
            if(i<writeStr.length())
                data[i]=str[i];
            else
                data[i]=0;
        }
        switch(command)
        {
            case SET_MOB_NO:
                LibusbJava.usb_control_msg(handle, USB.REQ_TYPE_TYPE_VENDOR | USB.REQ_TYPE_DIR_HOST_TO_DEVICE, SET_MOB_NO, 0, 1, data, 13, 5000);
                break;
            case SET_DEV_ID:
                LibusbJava.usb_control_msg(handle, USB.REQ_TYPE_TYPE_VENDOR | USB.REQ_TYPE_DIR_HOST_TO_DEVICE, SET_DEV_ID, index, 1, data, 8, 5000);
                break;
            case SET_DEV_STATUS:
                LibusbJava.usb_control_msg(handle, USB.REQ_TYPE_TYPE_VENDOR | USB.REQ_TYPE_DIR_DEVICE_TO_HOST, GET_DEV_STATUS, 0, 1, data, 8, 5000);
                index--;
                if(writeStr.equals("ON"))
                {
                    data[0]|=(byte) (1 << index);
                }
                else
                {
                    data[0]&=(byte) ~(1 << index);
                }
                LibusbJava.usb_control_msg(handle, USB.REQ_TYPE_TYPE_VENDOR | USB.REQ_TYPE_DIR_HOST_TO_DEVICE, SET_DEV_STATUS, 0, 1, data, 1, 5000);
                break;
            default:
                throw new USBException("unknown write command");
        }
        return 0;
    }
}