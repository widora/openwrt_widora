-------  LIRC --  In test and construction    --------    

This is a driver program for infrared remote control receiver demodulation module.
An infrared receiver module has three pins, two for power supply and one for data output.
The data output pin shall be connected to Widora IO17 PIN.

Once an interrupter is triggered by falling-edge on IO17,the module driver will start decoding signals from infrared receiver.
The module driver will return the latest received codes when you read the device.

Midas Zhou
