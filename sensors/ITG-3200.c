/*
    User space program to check communication with the ITG-3200 sensor connected to
    the IO expansion header of the Raspberry Pi 3 Model B. You need to use i2c-gpio-param 
    (GPIO bitbanged I2C) to communicate with the sensor.
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
    
    arm-linux-gnueabihf-gcc -Wall ITG-3200.c -o ITG-3200
    
    Usage
    =====
    
    sudo insmod i2c-gpio-param.ko
    echo 5 26 20 > /sys/class/i2c-gpio/add_bus
    ./ITG-3200
    
*/

#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define I2C_ADDR                    0x68                /* slave address of the sensor */
#define I2C_BUS                     "/dev/i2c-5"        /* I2C bus where the sensor is connected to */
#define REG_WHO_AM_I                0
#define REG_TEMPERATURE             0x1B
 
int main (void) {
	uint8_t buffer[4];
	int fd;
    double temp;
    int16_t raw;

	fd = open(I2C_BUS, O_RDWR);

	if(fd < 0)
	{
		printf("opening file failed: %s\n", strerror(errno));
		return 1;
	}

	if(ioctl(fd, I2C_SLAVE, I2C_ADDR) < 0)
	{
		printf("ioctl error: %s\n", strerror(errno));
		return 1;
	}

    /* read firmware revision */
	buffer[0]=REG_WHO_AM_I;
    if(write(fd, buffer, 1) < 0)
    {
        printf("Communication with sensor failed.\n");
        return 1;
    }
	if(read(fd, buffer, 1) < 0)
	{
        printf("Communication with sensor failed.\n");
        return 1;
    }
    
	printf("Device ID: 0x%02X - %s\n", buffer[0], (buffer[0] == (I2C_ADDR + 1)) ? "ITG-3200 found" : "ITG-3200 not found");
    
    while(1)
    {
        buffer[0]=REG_TEMPERATURE;
        if(write(fd, buffer, 1) < 0)
        {
            printf("Communication with sensor failed.\n");
            return 1;
        }
	    if(read(fd, buffer, 2) < 0)
	    {
            printf("Communication with sensor failed.\n");
            return 1;
        }
        raw = (buffer[0] << 8) | (buffer[1]);
        temp = (double)raw / 280 + 82.142857;
        printf("%.2f °C\n", temp);
        sleep(1);
    }
	return 0;
}
