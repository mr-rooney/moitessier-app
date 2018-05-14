/*
    User space program to check communication with the MPU-9250 sensor
    available on the Moitessier HAT.
    This source code is for demonstation purpose only and was tested
    on a Raspberry Pi 3 Model B.
    
    Copyright (C) 2018  Thomas POMS <hwsw.development@gmail.com>
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*
    Compiling
    =========
    
    arm-linux-gnueabihf-gcc -Wall MPU-9250.c -o MPU-9250
    
    Usage
    =====
    
    Running in endless loop:
    ./MPU-9250
    
    Running with specified iterations:
    ./MPU-9250 <ITERATIONS> <HUMAN_READABLE>
    
    Read device ID:
    ./MPU-9250 0 <HUMAN_READABLE> 
*/

#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>

#define I2C_ADDR                    0x68                /* slave address of the sensor */
#define I2C_BUS                     "/dev/i2c-1"        /* I2C bus where the sensor is connected to */
#define REG_WHO_AM_I                117
#define REG_TEMPERATURE             65
 
int main (int argc,char** argv)
{
	uint8_t buffer[4];
	int fd;
    double temp;
    int iterations = 0;
    int cycles = 0;
    int humanReadable = 1;
    
	if(argc >= 2)
    {
        iterations = atoi(argv[1]);    
    }
                
    if(argc == 3)
    {
        humanReadable = atoi(argv[2]);
    }

    fd = open(I2C_BUS, O_RDWR);

	if(fd < 0)
	{
	    if(humanReadable)
		    printf("opening file failed: %s\n", strerror(errno));
		return 1;
	}

	if(ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0)
	{
	    if(humanReadable)
		    printf("ioctl error: %s\n", strerror(errno));
		return 1;
	}
	
	/* read firmware revision */
	buffer[0]=REG_WHO_AM_I;
    write(fd, buffer, 1);
	read(fd, buffer, 1);
	
	if(argc < 2 || iterations == 0)
    {
        if(humanReadable)
            printf("Device ID: 0x%02X - %s\n", buffer[0], (buffer[0] == 0x71) ? "MPU-9250" : (buffer[0] == 0x73) ? "MPU-9255" : "failure");
        else
            printf("%02X\n", buffer[0]);
    }
    
    while(argc < 2 || (argc >= 2 && cycles < iterations))
    {
        cycles++;
        buffer[0]=REG_TEMPERATURE;
        write(fd, buffer, 1);
	    read(fd, buffer, 2);
        temp = (double)(((uint16_t)buffer[0] << 8) | buffer[1]) / 333.87 + 21;
        if(humanReadable)
            printf("%.2f °C\n", temp);
        else
            printf("%.2f\n", temp);
        sleep(1);
    }
	return 0;
}
