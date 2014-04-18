/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package gsmcontrolusb;

import ch.ntb.usb.USBException;
import java.awt.Color;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import usbio.USBIO;

/**
 *
 * @author supratim This is the main GUI implementation and uses the USBIO class
 * for USB communication
 */
public class Main {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // TODO code application logic here

        final JFrame frame = new JFrame("GSM Control Configuration Tool");//main gui window frame
        frame.setLayout(new GridLayout(8, 1));
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setLocation(200, 200);

        final JPanel deviceInfo = new JPanel();

        JPanel header1 = new JPanel();
        JLabel headerInfo1 = new JLabel("Change the mobile number to which the device responds");
        header1.add(headerInfo1);
        header1.setBackground(Color.MAGENTA);

        JPanel header2 = new JPanel();
        JLabel headerInfo2 = new JLabel("Change the device names and current status(7 character limited names)");
        header2.add(headerInfo2);
        header2.setBackground(Color.ORANGE);

        final JPanel panel1 = new JPanel();
        panel1.setBackground(Color.pink);
        final JPanel panel2 = new JPanel();
        panel2.setBackground(Color.cyan);
        final JPanel panel3 = new JPanel();
        panel3.setBackground(Color.pink);
        final JPanel panel4 = new JPanel();
        panel4.setBackground(Color.cyan);
        final JPanel panel5 = new JPanel();
        panel5.setBackground(Color.pink);

        JLabel label1 = new JLabel("    +91");
        JLabel label2 = new JLabel("Device 1");
        JLabel label3 = new JLabel("Device 2");
        JLabel label4 = new JLabel("Device 3");
        JLabel label5 = new JLabel("Device 4");

        final JTextField mobileNumber = new JTextField();
        mobileNumber.setColumns(10);
        mobileNumber.setEditable(false);
        final JTextField device1 = new JTextField();
        device1.setColumns(7);
        final JTextField device2 = new JTextField();
        device2.setColumns(7);
        final JTextField device3 = new JTextField();
        device3.setColumns(7);
        final JTextField device4 = new JTextField();
        device4.setColumns(7);

        JButton mobileRead = new JButton("read");
        JButton dev1Read = new JButton("read");
        JButton dev2Read = new JButton("read");
        JButton dev3Read = new JButton("read");
        JButton dev4Read = new JButton("read");

        mobileRead.setActionCommand("mobileRead");
        dev1Read.setActionCommand("dev1Read");
        dev2Read.setActionCommand("dev2Read");
        dev3Read.setActionCommand("dev3Read");
        dev4Read.setActionCommand("dev4Read");

        final JButton mobileChange = new JButton("change");
        final JButton dev1Change = new JButton("change");
        final JButton dev2Change = new JButton("change");
        final JButton dev3Change = new JButton("change");
        final JButton dev4Change = new JButton("change");

        mobileChange.setActionCommand("mobileChange");
        dev1Change.setActionCommand("dev1Change");
        dev2Change.setActionCommand("dev2Change");
        dev3Change.setActionCommand("dev3Change");
        dev4Change.setActionCommand("dev4Change");

        final USBIO usb = new USBIO();    //create a new USBIO object
        try {
            usb.open();
            JLabel info = new JLabel("Device: " + usb.getProductName() + "          Developer: " + usb.getVendorName());
            deviceInfo.setBackground(Color.lightGray);
            deviceInfo.add(info);
            frame.add(deviceInfo);
        } catch (USBException e) {
            JFrame error = new JFrame("error!");
            JLabel msg = new JLabel(e.toString());
            error.add(msg);
            error.pack();
            error.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            error.setVisible(true);
            frame.dispose();
        }
        final JButton disableField = new JButton("enable");
        disableField.setActionCommand("disableField");
        String status;

        final JButton dev1Switch = new JButton();
        final JButton dev2Switch = new JButton();
        final JButton dev3Switch = new JButton();
        final JButton dev4Switch = new JButton();

        dev1Switch.setActionCommand("dev1Switch");
        dev2Switch.setActionCommand("dev2Switch");
        dev3Switch.setActionCommand("dev3Switch");
        dev4Switch.setActionCommand("dev4Switch");

        try {
            if ((status = usb.usbRead(USBIO.GET_DEV_STATUS, 1)).equals("ON")) {
                dev1Switch.setBackground(Color.GREEN);
            } else {
                dev1Switch.setBackground(Color.RED);
            }
            dev1Switch.setText(status);

            if ((status = usb.usbRead(USBIO.GET_DEV_STATUS, 2)).equals("ON")) {
                dev2Switch.setBackground(Color.GREEN);
            } else {
                dev2Switch.setBackground(Color.RED);
            }
            dev2Switch.setText(status);

            if ((status = usb.usbRead(USBIO.GET_DEV_STATUS, 3)).equals("ON")) {
                dev3Switch.setBackground(Color.GREEN);
            } else {
                dev3Switch.setBackground(Color.RED);
            }
            dev3Switch.setText(status);

            if ((status = usb.usbRead(USBIO.GET_DEV_STATUS, 4)).equals("ON")) {
                dev4Switch.setBackground(Color.GREEN);
            } else {
                dev4Switch.setBackground(Color.RED);
            }
            dev4Switch.setText(status);
        } catch (Exception e) {
            JFrame error = new JFrame("error!");
            JLabel msg = new JLabel(e.toString());
            error.add(msg);
            error.pack();
            error.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            error.setVisible(true);
            frame.dispose();
        }


        mobileChange.setEnabled(false);
        dev1Change.setEnabled(false);
        dev2Change.setEnabled(false);
        dev3Change.setEnabled(false);
        dev4Change.setEnabled(false);

        panel1.add(label1);
        panel1.add(mobileNumber);
        panel1.add(mobileRead);
        panel1.add(mobileChange);
        panel1.add(disableField);

        panel2.add(label2);
        panel2.add(device1);
        panel2.add(dev1Read);
        panel2.add(dev1Change);
        panel2.add(dev1Switch);

        panel3.add(label3);
        panel3.add(device2);
        panel3.add(dev2Read);
        panel3.add(dev2Change);
        panel3.add(dev2Switch);

        panel4.add(label4);
        panel4.add(device3);
        panel4.add(dev3Read);
        panel4.add(dev3Change);
        panel4.add(dev3Switch);

        panel5.add(label5);
        panel5.add(device4);
        panel5.add(dev4Read);
        panel5.add(dev4Change);
        panel5.add(dev4Switch);

        frame.add(header1);
        frame.add(panel1);
        frame.add(header2);
        frame.add(panel2);
        frame.add(panel3);
        frame.add(panel4);
        frame.add(panel5);
        try {
            mobileNumber.setText(usb.usbRead(USBIO.GET_MOB_NO, 0).substring(3));
            device1.setText(usb.usbRead(USBIO.GET_DEV_ID, 1));
            device2.setText(usb.usbRead(USBIO.GET_DEV_ID, 2));
            device3.setText(usb.usbRead(USBIO.GET_DEV_ID, 3));
            device4.setText(usb.usbRead(USBIO.GET_DEV_ID, 4));
        } catch (Exception e) {
            JFrame error = new JFrame("error!");
            JLabel msg = new JLabel(e.toString());
            error.add(msg);
            error.pack();
            error.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
            error.setVisible(true);
            frame.dispose();
        }
        frame.pack();
        frame.setVisible(true);

        // GUI code is completed here
        //Event handling coed is what follows

        ActionListener eventHandler = new ActionListener() {   //implented as an innerclass
            public void actionPerformed(ActionEvent e) {
                String actionCommand = e.getActionCommand();
                try {
                    if (actionCommand.equals("mobileRead")) //mobile read button is pressed
                    {
                        mobileNumber.setText(usb.usbRead(USBIO.GET_MOB_NO, 0).substring(3));
                    } else if (actionCommand.equals("mobileChange")) //mobile change button is pressed
                    {
                        String number = mobileNumber.getText().substring(0, 10);
                        if (Long.parseLong(number) <= 0 || number.length() != 10) {
                            mobileNumber.setText("10 digit mob no");
                        } else {
                            number = "+91" + number;
                            usb.usbWrite(USBIO.SET_MOB_NO, number, 0);
                        }
                    } else if (actionCommand.equals("dev1Read")) //device 1 read button is pressed
                    {
                        device1.setText(usb.usbRead(USBIO.GET_DEV_ID, 1).trim());
                    } else if (actionCommand.equals("dev2Read")) //device 2 read button is pressed
                    {
                        device2.setText(usb.usbRead(USBIO.GET_DEV_ID, 2).trim());
                    } else if (actionCommand.equals("dev3Read")) //device 3 read button is pressed
                    {
                        device3.setText(usb.usbRead(USBIO.GET_DEV_ID, 3).trim());
                    } else if (actionCommand.equals("dev4Read")) //device 4 read button is pressed
                    {
                        device4.setText(usb.usbRead(USBIO.GET_DEV_ID, 4).trim());
                    } else if (actionCommand.equals("dev1Change")) //device 1 change button is pressed
                    {
                        String name = device1.getText();
                        if (name.length() > 7) {
                            name = name.substring(0, 7);
                            name = name.trim();
                        }
                        usb.usbWrite(USBIO.SET_DEV_ID, name.toUpperCase(), 1);
                    } else if (actionCommand.equals("dev2Change")) //device 2 change button is pressed
                    {
                        String name = device2.getText();
                        if (name.length() > 7) {
                            name = name.substring(0, 7);
                            name = name.trim();
                        }
                        usb.usbWrite(USBIO.SET_DEV_ID, name.toUpperCase(), 2);
                    } else if (actionCommand.equals("dev3Change")) //device 3 change button is pressed
                    {
                        String name = device3.getText();
                        if (name.length() > 7) {
                            name = name.substring(0, 7);
                            name = name.trim();
                        }
                        usb.usbWrite(USBIO.SET_DEV_ID, name.toUpperCase(), 3);
                    } else if (actionCommand.equals("dev4Change")) //device 4 change button is pressed
                    {
                        String name = device4.getText();
                        if (name.length() > 7) {
                            name = name.substring(0, 7);
                            name = name.trim();
                        }
                        usb.usbWrite(USBIO.SET_DEV_ID, name.toUpperCase(), 4);
                    } else if (actionCommand.equals("dev1Switch")) //device 1 switch button is pressed
                    {
                        if (dev1Switch.getText().equals("ON")) {
                            usb.usbWrite(USBIO.SET_DEV_STATUS, "OFF", 1);
                            dev1Switch.setText("OFF");
                            dev1Switch.setBackground(Color.red);
                        } else {
                            usb.usbWrite(USBIO.SET_DEV_STATUS, "ON", 1);
                            dev1Switch.setText("ON");
                            dev1Switch.setBackground(Color.green);
                        }
                    } else if (actionCommand.equals("dev2Switch")) //device 2 switch button is pressed
                    {
                        if (dev2Switch.getText().equals("ON")) {
                            usb.usbWrite(USBIO.SET_DEV_STATUS, "OFF", 2);
                            dev2Switch.setText("OFF");
                            dev2Switch.setBackground(Color.red);
                        } else {
                            usb.usbWrite(USBIO.SET_DEV_STATUS, "ON", 2);
                            dev2Switch.setText("ON");
                            dev2Switch.setBackground(Color.green);
                        }
                    } else if (actionCommand.equals("dev3Switch")) //device 3 switch button is pressed
                    {
                        if (dev3Switch.getText().equals("ON")) {
                            usb.usbWrite(USBIO.SET_DEV_STATUS, "OFF", 3);
                            dev3Switch.setText("OFF");
                            dev3Switch.setBackground(Color.red);
                        } else {
                            usb.usbWrite(USBIO.SET_DEV_STATUS, "ON", 3);
                            dev3Switch.setText("ON");
                            dev3Switch.setBackground(Color.green);
                        }
                    } else if (actionCommand.equals("dev4Switch")) //device 4 switch button is pressed
                    {
                        if (dev4Switch.getText().equals("ON")) {
                            usb.usbWrite(USBIO.SET_DEV_STATUS, "OFF", 4);
                            dev4Switch.setText("OFF");
                            dev4Switch.setBackground(Color.red);
                        } else {
                            usb.usbWrite(USBIO.SET_DEV_STATUS, "ON", 4);
                            dev4Switch.setText("ON");
                            dev4Switch.setBackground(Color.green);
                        }
                    } else if (actionCommand.equals("disableField")) //the enable/disable button is pressed
                    {
                        if (disableField.getText().equals("enable")) {
                            disableField.setText("disable");
                            mobileNumber.setEditable(true);
                            mobileChange.setEnabled(true);
                            dev1Change.setEnabled(true);
                            dev2Change.setEnabled(true);
                            dev3Change.setEnabled(true);
                            dev4Change.setEnabled(true);
                        } else {
                            disableField.setText("enable");
                            mobileNumber.setEditable(false);
                            mobileChange.setEnabled(false);
                            dev1Change.setEnabled(false);
                            dev2Change.setEnabled(false);
                            dev3Change.setEnabled(false);
                            dev4Change.setEnabled(false);
                        }
                    }
                } catch (Exception ex) {
                    JFrame error = new JFrame("error!");
                    JLabel msg = new JLabel(ex.toString());
                    error.add(msg);
                    error.pack();
                    error.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
                    error.setVisible(true);
                    frame.dispose();
                }
            }
        };

        //register the components agains the event handler for responding to the events
        disableField.addActionListener(eventHandler);
        mobileChange.addActionListener(eventHandler);
        dev1Change.addActionListener(eventHandler);
        dev2Change.addActionListener(eventHandler);
        dev3Change.addActionListener(eventHandler);
        dev4Change.addActionListener(eventHandler);
        dev1Switch.addActionListener(eventHandler);
        dev2Switch.addActionListener(eventHandler);
        dev3Switch.addActionListener(eventHandler);
        dev4Switch.addActionListener(eventHandler);
        mobileRead.addActionListener(eventHandler);
        dev1Read.addActionListener(eventHandler);
        dev2Read.addActionListener(eventHandler);
        dev3Read.addActionListener(eventHandler);
        dev4Read.addActionListener(eventHandler);
    }
}