/*
    User space program to read temperature and humidity from the Silicon Labs
    Si7020-A20 sensor available on the Moitessier HAT.
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
    
    arm-linux-gnueabihf-gcc -Wall Si7020-A20.c -o Si7020-A20
    
    Usage
    =====
    
    Running in endless loop:
    ./Si7020-A20 /dev/i2c-1
    
    Running with specified iterations:
    ./Si7020-A20 /dev/i2c-1 <ITERATIONS> <HUMAN_READABLE>
    
    Read firmware revision:
    ./Si7020-A20 /dev/i2c-1 0 <HUMAN_READABLE>
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

#define I2C_ADDR                    0x40                /* slave address of the sensor */
//#define I2C_BUS                     "/dev/i2c-1"        /* I2C bus where the sensor is connected to */
char* I2C_BUS;

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
    int32_t rc;
    
    if(bufSize < 2)
        return -1;
        
    if(write(fd, &cmd, 1) < 0)
        return -1;
    
    do
    {
       //usleep(100); 
        rc = read(fd, buf, 2);
        if(rc < 0)
            return -1;
    }while(rc != 2);

    return 0;
}

/* measure humidity */
int measHum(int fd, uint8_t *buf, uint32_t bufSize)
{
    uint8_t cmd;
    int32_t rc;
    
    if(bufSize < 4)
        return -1;
    
    cmd = CMD_MEAS_RH;
    if(write(fd, &cmd, 1) < 0)
        return -1;
    
    do
    {
       //usleep(100); rc = (read(fd, buf, 2);
        rc = read(fd, buf, 2);
        if(rc < 0)
            return -1;
    }while(rc != 2);

    cmd = CMD_READ_TEMP_FROM_RH_MEAS;
    if(write(fd, &cmd, 1) < 0)
        return 1;
    
    do
    {
       //usleep(100); 
        rc = read(fd, buf + 2, 2);
        if(rc < 0)
            return -1;
    }while(rc != 2);

    return 0;
}
 
int main (int argc,char** argv)
{
	uint8_t buffer[4];
	int fd;
    double temp;
    double tempHum;
    double hum;
    uint32_t tempRead;
    uint32_t humRead;
    int iterations = 0;
    int cycles = 0;
    int humanReadable = 1;
    
    if(argc < 2)
    {
        printf("Missing parameter.\n");
        printf("Usage: %s <I2C_BUS> <ITERATIONS> <HUMAN_READABLE>\n", argv[0]);
        printf("       <ITERATIONS> is optional, 0...read firmware, > 1 iterations. Default = 1\n");
        printf("       <HUMAN_READABLE> is optional, 1...human readable output, else 0. Default = 1\n");
        return 1;
    }
    
    I2C_BUS = argv[1];
    
    if(argc >= 3)
    {
        iterations = atoi(argv[2]);    
    }
                
    if(argc == 4)
    {
        humanReadable = atoi(argv[3]);
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
	buffer[0]=(uint8_t)(CMD_READ_FW_REV >> 8);
    buffer[1]=(uint8_t)CMD_READ_FW_REV;
    if(write(fd, buffer, 2) < 0)
    {
        printf("Communication with sensor failed.\n");
        return 1;
    }
	if(read(fd, buffer, 1) < 0)
	{
        printf("Communication with sensor failed.\n");
        return 1;
    }
    
	if(argc < 2 || iterations == 0)
    {
        if(humanReadable)
            printf("Firmware revision: 0x%02X\n", buffer[0]);
        else
            printf("%02X\n", buffer[0]);
    }
    
    while(argc < 2 || (argc >= 2 && cycles < iterations))
    {
        cycles++;
        printf("4\n");
        if(measTemp(fd, buffer, sizeof(buffer)) != 0)
        {
            if(humanReadable)
                printf("Temperature measurement failed.\n");
            return 1;
        }
        printf("5\n");
	
        tempRead = buffer[0] << 8;
        tempRead |= buffer[1];
        temp = 175.72 * (double)tempRead / 65536 - 46.85;

        if(measHum(fd, buffer, sizeof(buffer)) != 0)
        {
            if(humanReadable)
                printf("Humidity measurement failed.\n");
            return 1;
        }
        humRead = buffer[0] << 8;
        humRead |= buffer[1];
        hum = 125 * (double)humRead / 65536 - 6;
        tempRead = buffer[2] << 8;
        tempRead |= buffer[3];
        tempHum = 175.72 * (double)tempRead / 65536 - 46.85;

        if(humanReadable)
            printf("%.2f °C, %.2f %%RH (%.2f °C)\n", temp, hum, tempHum);
        else
            printf("%.2f,%.2f,%.2f\n", temp, hum, tempHum);
        sleep(1);
    }
    
	return 0;
}
