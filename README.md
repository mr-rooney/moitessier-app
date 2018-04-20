User Space Applications
=======================

These user space examples are part of the Moitessier HAT project (https://github.com/mr-rooney/moitessier/wiki) 
and should illustrate how to communicate with the HAT using a Raspberry Pi. 

Compiling the user space examples: host> make

Edit the top level makefile, if a different compiler should be used or call make CC=<SPECIFIC_COMPILER>.


moitessier_ctrl
---------------
Features:
* reseting the HAT
* reading/reseting statistics
* enabling/disabling GNSS
* configuring the HAT (receiver frequency, simulator mode etc.)
* enable/disable write protection of ID EEPROM


Si7020-A20
----------
Reading humidity and temperature.


MS5607-02BA03
-------------
Reading pressure and temperature.


MPU-9250
--------
Reading device ID and temperature.


check_functionality
-------------------
Script to test the sensors by initiating a measurement.

