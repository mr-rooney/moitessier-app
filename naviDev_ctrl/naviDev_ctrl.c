/*
    User space program to configure/control the nav.HAT.
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
    
    arm-linux-gnueabihf-gcc -Wall naviDev_ctrl.c -o naviDev_ctrl
    
    Usage
    =====
    
    Get help and supported commands: 
    
    ./naviDev_ctrl
    
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stropts.h>
#include <stdint.h>
#include <stdbool.h>


#define IOC_MAGIC 'N'
#define IOCTL_CMDs                  7
#define IOCTL_GET_STATISTICS        _IO(IOC_MAGIC,0)
#define IOCTL_GET_INFO              _IO(IOC_MAGIC,1)
#define IOCTL_RESET_HAT             _IO(IOC_MAGIC,2)
#define IOCTL_RESET_STATISTICS      _IO(IOC_MAGIC,3)
#define IOCTL_GNSS                  _IO(IOC_MAGIC,4)
#define IOCTL_CONFIG                _IO(IOC_MAGIC,5)
#define IOCTL_ID_EEPROM             _IO(IOC_MAGIC,6)

#define NUM_RCV_CHANNELS            2       /* number of receiver channels per receiver, should be 2 */
#define NUM_RCV                     2       /* number of receivers, should be 2 */

struct st_receiverConfig
{
    uint8_t         metaDataMask;                       
    uint32_t        afcRange;                           
    uint32_t        afcRangeDefault;                    
    uint32_t        tcxoFreq;                           
    uint32_t        channelFreq[NUM_RCV_CHANNELS];      
};

struct st_info_rcv{
    struct st_receiverConfig    config;
    uint8_t                     rng[NUM_RCV_CHANNELS];
};

struct st_info_serial{
    uint32_t    h;      /* bits 95...64 */
    uint32_t    m;      /* bits 63...32 */
    uint32_t    l;      /* bits 31...0 */
};

struct st_simulator
{
    uint32_t mmsi[NUM_RCV_CHANNELS];
    uint32_t enabled;
    uint32_t interval;
};

struct st_info{
    uint8_t                     mode;
    uint8_t                     hwId[16];
    uint8_t                     hwVer[8];
    uint8_t                     bootVer[22];
    uint8_t                     appVer[22];
    uint32_t                    functionality;
    uint32_t                    systemErrors;
    struct st_info_serial       serial;
    struct st_info_rcv          rcv[NUM_RCV];
    struct st_simulator         simulator;
    uint8_t                     wpEEPROM;
    bool        valid;
};

struct st_statistics{
    uint64_t                spiCycles;
    uint64_t                totalRxPayloadBytes;
    uint64_t                fifoOverflows;
    uint64_t                fifoBytesProcessed;
    uint64_t                payloadCrcErrors;
    uint64_t                headerCrcErrors;
    uint64_t                keepAliveErrors;
};

struct st_configHAT{
    struct st_receiverConfig    rcv[NUM_RCV];
    struct st_simulator         simulator;
    uint8_t                     wpEEPROM;
};


int main (int argc,char** argv)
{
    int fd_navidev;
    int cmd = 0;
    int ioctlCmd = 0;
    int size;
    unsigned char buf[1024];
    uint32_t params[255];
    uint32_t i;
    uint32_t k;
    FILE *fd_param;
    char* fileName;
    
    struct st_info *info;
    struct st_statistics *statistics;
    struct st_configHAT configHAT;
    
    
    if(argc < 3)
    {
        printf("ERROR: missing parameters\n");
        printf("Usage: ./naviDev_ctrl.o <DEVICE> <CMD_NR> <PARAM> <PARAM> ... <PARAM>\n");
        printf("\tRead HAT statistics:\t\t\t ./naviDev_ctrl.o /dev/naviDev.ctrl 0\n");
        printf("\tGet HAT info:\t\t\t\t ./naviDev_ctrl.o /dev/naviDev.ctrl 1\n");
        printf("\tReset HAT:\t\t\t\t ./naviDev_ctrl.o /dev/naviDev.ctrl 2\n");
        printf("\tReset HAT statistics:\t\t\t ./naviDev_ctrl.o /dev/naviDev.ctrl 3\n");
        printf("\tEnable GNSS:\t\t\t\t ./naviDev_ctrl.o /dev/naviDev.ctrl 4 1\n");
        printf("\tDisable GNSS:\t\t\t\t ./naviDev_ctrl.o /dev/naviDev.ctrl 4 0\n");
        printf("\tConfigure HAT:\t\t\t\t ./naviDev_ctrl.o /dev/naviDev.ctrl 5 config.txt\n");
        printf("\tEnable ID EEPROM write protection:\t ./naviDev_ctrl.o /dev/naviDev.ctrl 6 1\n");
        printf("\tDisable ID EEPROM write protection:\t ./naviDev_ctrl.o /dev/naviDev.ctrl 6 0\n");
        return -1;
    }
    
    fd_navidev = open(argv[1], O_RDONLY);
    cmd = atoi(argv[2]);
    for(i = 0; i < (argc - 3); i++)
        params[i] = atoi(argv[i + 3]);
    
    if(fd_navidev)
    {
        if(cmd < 0 || cmd >= IOCTL_CMDs)
        {
            printf("ERROR: command not supported\n");
            close(fd_navidev);
            return -1;            
        }
        
        printf("opening device %s\n", argv[1]);
        
        switch(cmd)
        {
            case 0:
                ioctlCmd = IOCTL_GET_STATISTICS;
                break;
            case 1:
                ioctlCmd = IOCTL_GET_INFO;
                break;
            case 2:
                ioctlCmd = IOCTL_RESET_HAT;
                break;
            case 3: 
                ioctlCmd = IOCTL_RESET_STATISTICS;
                break;
            case 4:
                ioctlCmd = IOCTL_GNSS;
                buf[0] = (char)params[0];
                break;
            case 5:
                ioctlCmd = IOCTL_CONFIG;
                fileName = argv[3];
                fd_param = fopen(fileName, "r");
                if(!fd_param)
                {
                    printf("ERROR: could not open file \"%s\"\n", fileName);
                    return -1;
                }   
                i = 0;

                /* the AFC default range is not used, so we set it to 0 */
                configHAT.rcv[0].afcRangeDefault = 0;
                configHAT.rcv[1].afcRangeDefault = 0;

                while(fgets((char*)buf, sizeof(buf), fd_param) != NULL && i <= 14)
                {
                    for(k = 0; k < strlen((char*)buf); k++)
                    {
                        if(buf[k] == '\r' || buf[k] == '\n')
                            buf[k] = '\0';
                    }
                    
                    switch(i)
                    {
                        case 0:
                            configHAT.rcv[0].channelFreq[0] = (uint32_t)atoi((char*)buf);
                            break;
                        case 1:
                            configHAT.rcv[0].channelFreq[1] = (uint32_t)atoi((char*)buf);
                            break;
                        case 2:
                            configHAT.rcv[0].metaDataMask = (uint8_t)atoi((char*)buf);
                            break;
                        case 3:
                            configHAT.rcv[0].afcRange = (uint32_t)atoi((char*)buf);
                            break;
                        case 4: 
                            configHAT.rcv[0].tcxoFreq = (uint32_t)atoi((char*)buf);
                            break;
                        case 5:
                            configHAT.rcv[1].channelFreq[0] = (uint32_t)atoi((char*)buf);
                            break;
                        case 6:
                            configHAT.rcv[1].channelFreq[1] = (uint32_t)atoi((char*)buf);
                            break;
                        case 7:
                            configHAT.rcv[1].metaDataMask = (uint8_t)atoi((char*)buf);
                            break;
                        case 8:
                            configHAT.rcv[1].afcRange = (uint32_t)atoi((char*)buf);
                            break;
                        case 9:
                            configHAT.rcv[1].tcxoFreq = (uint32_t)atoi((char*)buf);
                            break;
                        case 10:
                            configHAT.simulator.enabled = (uint8_t)atoi((char*)buf);
                            break;
                        case 11:
                            configHAT.simulator.interval = (uint32_t)atoi((char*)buf);
                            break;
                        case 12:
                            configHAT.simulator.mmsi[0] = (uint32_t)atoi((char*)buf);
                            break;
                        case 13:
                            configHAT.simulator.mmsi[1] = (uint32_t)atoi((char*)buf);
                            break;
                        case 14:
                            configHAT.wpEEPROM = (uint8_t)atoi((char*)buf);
                            break;
                        default:
                            break;
                    }
                    i++;
                } 
                fclose(fd_param);
                memset(buf, 0, sizeof(buf));
                memcpy(buf, &configHAT, sizeof(struct st_configHAT));
                break;
            case 6:
                ioctlCmd = IOCTL_ID_EEPROM;
                buf[0] = (char)params[0];
                break;
            default:
                break;
        }
        
        size = ioctl(fd_navidev, ioctlCmd, &buf);
        printf("size - %u\n", size);
               
        switch(cmd)
        {
            case 0:
                statistics = (struct st_statistics*)buf;
                printf("spiCycles - %lu\n", (long unsigned int)statistics->spiCycles);
                printf("totalRxPayloadBytes - %lu\n", (long unsigned int)statistics->totalRxPayloadBytes);
                printf("fifoOverflows - %lu\n", (long unsigned int)statistics->fifoOverflows);
                printf("fifoBytesProcessed - %lu\n", (long unsigned int)statistics->fifoBytesProcessed);
                printf("payloadCrcErrors - %lu\n", (long unsigned int)statistics->payloadCrcErrors);
                printf("headerCrcErrors - %lu\n", (long unsigned int)statistics->headerCrcErrors);
                printf("keepAliveErrors - %lu\n", (long unsigned int)statistics->keepAliveErrors);
                break;
            case 1:
                info = (struct st_info*)buf;
                if(info->valid)
                {
                    printf("mode - %c\n", (char)info->mode);
                    printf("hardware ID - %s\n", (char*)info->hwId);
                    printf("hardware version - %s\n", (char*)info->hwVer);
                    printf("boot version - %s\n", (char*)info->bootVer);
                    printf("app version - %s\n", (char*)info->appVer);
                    printf("functionality - 0x%08x\n", (unsigned int)info->functionality);
                    printf("system errors - 0x%08x\n", (unsigned int)info->systemErrors);
                    printf("serial - %08x%08x%08x\n", (unsigned int)info->serial.h, (unsigned int)info->serial.m, (unsigned int)info->serial.l);
                    printf("receiver 1\n");
                    printf("\tchannel frequency 1 [Hz]:\t %u\n", (unsigned int)info->rcv[0].config.channelFreq[0]);
                    printf("\tchannel frequency 2 [Hz]:\t %u\n", (unsigned int)info->rcv[0].config.channelFreq[1]);
                    printf("\ttcxo frequency [Hz]:\t\t %u\n", (unsigned int)info->rcv[0].config.tcxoFreq);
                    printf("\tmeta data mask:\t\t\t 0x%02x\n", (unsigned int)info->rcv[0].config.metaDataMask);
                    printf("\tafc range [Hz]:\t\t\t %u\n", (unsigned int)info->rcv[0].config.afcRange);
                    printf("\tdefault afc range [Hz]:\t\t %u\n", (unsigned int)info->rcv[0].config.afcRangeDefault);
                    printf("\trng:\t\t\t\t 0x%02x 0x%02x\n", (unsigned int)info->rcv[0].rng[0], (unsigned int)info->rcv[0].rng[1]);
                    printf("receiver 2\n");
                    printf("\tchannel frequency 1 [Hz]:\t %u\n", (unsigned int)info->rcv[1].config.channelFreq[0]);
                    printf("\tchannel frequency 2 [Hz]:\t %u\n", (unsigned int)info->rcv[1].config.channelFreq[1]);
                    printf("\ttcxo frequency [Hz]:\t\t %u\n", (unsigned int)info->rcv[1].config.tcxoFreq);
                    printf("\tmeta data mask:\t\t\t 0x%02x\n", (unsigned int)info->rcv[1].config.metaDataMask);
                    printf("\tafc range [Hz]:\t\t\t %u\n", (unsigned int)info->rcv[1].config.afcRange);
                    printf("\tdefault afc range [Hz]:\t\t %u\n", (unsigned int)info->rcv[1].config.afcRangeDefault);
                    printf("\trng:\t\t\t\t 0x%02x 0x%02x\n", (unsigned int)info->rcv[1].rng[0], (unsigned int)info->rcv[1].rng[1]);
                    printf("simulator\n");
                    printf("\tenabled:\t\t\t %u\n", (unsigned int)info->simulator.enabled);
                    printf("\tinterval:\t\t\t %u\n", (unsigned int)info->simulator.interval);
                    printf("\tmmsi:\t\t\t\t %09u %09u\n", (unsigned int)info->simulator.mmsi[0], (unsigned int)info->simulator.mmsi[1]);
                    printf("misc\n");
                    printf("\twrite protection ID EEPROM:\t %u\n", (unsigned int)info->wpEEPROM);
                    
                    if(info->systemErrors)
                        printf("\n\n***** SYSTEM ERRORS HAVE OCCURRED *****\n\n");
                }
                else
                {
                    printf("Data is not valid yet!!!\n");
                }
                break;
            case 2:
                break;
            case 3:
                printf("statistics reset\n");
                break;
            case 4:
                printf("GNSS enabled = %u\n", params[0]);
                break;
            case 5:
                printf("configuration set\n");
                printf("receiver 1\n");
                printf("\tchannel frequency 1 [Hz]:\t %u\n", (unsigned int)configHAT.rcv[0].channelFreq[0]);
                printf("\tchannel frequency 2 [Hz]:\t %u\n", (unsigned int)configHAT.rcv[0].channelFreq[1]);
                printf("\ttcxo frequency [Hz]:\t\t %u\n", (unsigned int)configHAT.rcv[0].tcxoFreq);
                printf("\tmeta data mask:\t\t\t 0x%02x\n", (unsigned int)configHAT.rcv[0].metaDataMask);
                printf("\tafc range [Hz]:\t\t\t %u\n", (unsigned int)configHAT.rcv[0].afcRange);
                printf("receiver 2\n");
                printf("\tchannel frequency 1 [Hz]:\t %u\n", (unsigned int)configHAT.rcv[1].channelFreq[0]);
                printf("\tchannel frequency 2 [Hz]:\t %u\n", (unsigned int)configHAT.rcv[1].channelFreq[1]);
                printf("\ttcxo frequency [Hz]:\t\t %u\n", (unsigned int)configHAT.rcv[1].tcxoFreq);
                printf("\tmeta data mask:\t\t\t 0x%02x\n", (unsigned int)configHAT.rcv[1].metaDataMask);
                printf("\tafc range [Hz]:\t\t\t %u\n", (unsigned int)configHAT.rcv[1].afcRange);
                printf("simulator\n");
                printf("\tenabled:\t\t\t %u\n", (unsigned int)configHAT.simulator.enabled);
                printf("\tinterval:\t\t\t %u\n", (unsigned int)configHAT.simulator.interval);
                printf("\tmmsi:\t\t\t\t %09u %09u\n", (unsigned int)configHAT.simulator.mmsi[0], (unsigned int)configHAT.simulator.mmsi[1]);
                printf("misc\n");
                printf("\twrite protection ID EEPROM:\t %u\n", (unsigned int)configHAT.wpEEPROM);
                
                break;
            default:
                break;
        }  
    }
    else
    {
        printf("error opening device %s\n", argv[1]);
        return -1;
    }
    close(fd_navidev);
    return 0;
}       
