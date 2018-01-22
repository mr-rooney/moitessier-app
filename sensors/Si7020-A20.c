/*
    User space program to read temperature and humidity from the Silicon Labs
    Si7020-A20 sensor.
    This source code is for demonstation purpose only.
    
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
    
    arm-linux-gnueabihf-gcc -Wall Si7020-A20.c -o Si7020-A20.o
    
    Usage
    =====
    
    ./Si7020-A20.o
    
*/

#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define I2C_ADDR                    0x40                /* slave address of the sensor */
#define I2C_BUS                     "/dev/i2c-1"        /* I2C bus where the sensor is connected to */

/* sensor commands */
/* YOU MUST NOT USE CLOCK STRETCHING COMMANDS ON THE RASPBERRY PI */
#define CMD_MEAS_RH                 0xF5
#define CMD_MEAS_TEMP               0xF3
#define CMD_READ_TEMP_FROM_RH_MEAS  0xE0
#define CMD_RESET                   0xFE
#define CMD_READ_FW_REV             0x84B8
#define CMD_READ_ID_1               0xFA0F
#define CMD_READ_ID_2               0xFCC9

/* measure temperature */
int measTemp(int fd, uint8_t *buf, uint32_t bufSize)
{
    uint8_t cmd = CMD_MEAS_TEMP;
    
    if(bufSize < 2)
        return -1;
        
    write(fd, &cmd, 1);
    do
    {
       //usleep(100); 
    }while(read(fd, buf, 2) != 2);

    return 0;
}

/* measure humidity */
int measHum(int fd, uint8_t *buf, uint32_t bufSize)
{
    uint8_t cmd;
    
    if(bufSize < 4)
        return -1;
    
    cmd = CMD_MEAS_RH;
    write(fd, &cmd, 1);
    do
    {
       //usleep(100); 
    }while(read(fd, buf, 2) != 2);

    cmd = CMD_READ_TEMP_FROM_RH_MEAS;
    write(fd, &cmd, 1);
    do
    {
       //usleep(100); 
    }while(read(fd, buf + 2, 2) != 2);

    return 0;
}
 
int main (void) {
	uint8_t buffer[4];
	int fd;
    double temp;
    double tempHum;
    double hum;
    uint32_t tempRead;
    uint32_t humRead;

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
	buffer[0]=(uint8_t)(CMD_READ_FW_REV >> 8);
    buffer[1]=(uint8_t)CMD_READ_FW_REV;
	write(fd, buffer, 2);
	read(fd, buffer, 1);
	printf("Firmware revision: 0x%02X\n", buffer[0]);

    while(1)
    {
        if(measTemp(fd, buffer, sizeof(buffer)) != 0)
        {
            printf("Temperature measurement failed.\n");
            return 1;
        }
        tempRead = buffer[0] << 8;
        tempRead |= buffer[1];
        temp = 175.72 * (double)tempRead / 65536 - 46.85;

        if(measHum(fd, buffer, sizeof(buffer)) != 0)
        {
            printf("Humidity measurement failed.\n");
            return 1;
        }
        humRead = buffer[0] << 8;
        humRead |= buffer[1];
        hum = 125 * (double)humRead / 65536 - 6;
        tempRead = buffer[2] << 8;
        tempRead |= buffer[3];
        tempHum = 175.72 * (double)tempRead / 65536 - 46.85;

        printf("%.2f °C, %.2f %%RH (%.2f °C)\n", temp, hum, tempHum);
        sleep(1);
    }
    
	return 0;
}
