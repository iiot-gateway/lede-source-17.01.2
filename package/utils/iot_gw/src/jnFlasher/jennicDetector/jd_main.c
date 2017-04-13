// ------------------------------------------------------------------
// Jennic Module Detector
// ------------------------------------------------------------------
// Detects what type of Jennic device is connected and returns that
// to the caller-script
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \brief Program that detects what type of Jennic device is connected
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include "JN51xx_BootLoader.h"
#include "Firmware.h"
#include "uart.h"
#include "ChipID.h"
#include "dbg.h"

#define vDelay(a) usleep(a * 1000)

#define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

/** Import binary data from FlashProgrammerExtension_JN5168.bin */
int _binary_FlashProgrammerExtension_JN5168_bin_start;
int _binary_FlashProgrammerExtension_JN5168_bin_size;
int _binary_FlashProgrammerExtension_JN5169_bin_start;
int _binary_FlashProgrammerExtension_JN5169_bin_size;
int _binary_FlashProgrammerExtension_JN5179_bin_start;
int _binary_FlashProgrammerExtension_JN5179_bin_size;


const char *cpSerialDevice = "/dev/ttyUSB0";

int iInitialSpeed=38400;
int iProgramSpeed=1000000;

int main(int argc, char *argv[])
{
    tsPRG_Context   sPRG_Context;
    int ret = 0;
    
    printf("JennicModuleDetector Version: %s\n", pcPRG_Version);
    
    memset(&sPRG_Context, 0, sizeof(tsPRG_Context));

    
    if (eUART_Initialise((char *)cpSerialDevice, iInitialSpeed, &sPRG_Context.iUartFD, &sPRG_Context.sUartOptions) != E_PRG_OK)
    {
        fprintf(stderr, "Error opening serial port\n");
        return 1;
    }

    if (iInitialSpeed != iProgramSpeed)
    {
        DEBUG_PRINTF("Setting baudrate for port %d to %d\n", sPRG_Context.iUartFD, iProgramSpeed);

        /* Talking at initial speed - change bootloader to programming speed */
        if (eBL_SetBaudrate(&sPRG_Context, iProgramSpeed) != E_PRG_OK)
        {
            fprintf(stderr, "Error setting (bootloader) baudrate to iProgramSpeed (%d)\n", iProgramSpeed);
            ret = 2;
        }
        /* change local port to programming speed */
        if (eUART_SetBaudRate(sPRG_Context.iUartFD, &sPRG_Context.sUartOptions, iProgramSpeed) != E_PRG_OK)
        {
            fprintf(stderr, "Error setting (local port) baudrate to iProgramSpeed (%d)\n", iProgramSpeed);
            // eBL_SetBaudrate(&sPRG_Context, iInitialSpeed);
            ret = 3;
        }
    }

    if ( ret != 0 ) return ret;

    /* Read module details at initial baud rate */
    if (ePRG_ChipGetDetails(&sPRG_Context) != E_PRG_OK)
    {
        fprintf(stderr, "Error reading module information - check cabling and power\n");
        ret = 4;
    }
    
    // Set BL and local port back to initial speed (in reverse order)
    eBL_SetBaudrate(&sPRG_Context, iInitialSpeed);
    eUART_SetBaudRate(sPRG_Context.iUartFD, &sPRG_Context.sUartOptions, iInitialSpeed);

    if ( ret != 0 ) return ret;

    const char *pcPartName = "Unknown";
    int dev = 0;

    switch (sPRG_Context.sChipDetails.u32ChipId)
    {
        case (CHIP_ID_JN5148_REV2A):    pcPartName = "JN5148";    dev = 48;   break;
        case (CHIP_ID_JN5148_REV2B):    pcPartName = "JN5148";    dev = 48;   break;
        case (CHIP_ID_JN5148_REV2C):    pcPartName = "JN5148";    dev = 48;   break;
        case (CHIP_ID_JN5148_REV2D):    pcPartName = "JN5148J01"; dev = 48;   break;
        case (CHIP_ID_JN5148_REV2E):    pcPartName = "JN5148Z01"; dev = 48;   break;

        case (CHIP_ID_JN5142_REV1A):    pcPartName = "JN5142";    dev = 42;   break;
        case (CHIP_ID_JN5142_REV1B):    pcPartName = "JN5142";    dev = 42;   break;
        case (CHIP_ID_JN5142_REV1C):    pcPartName = "JN5142J01"; dev = 42;   break;

        case (CHIP_ID_JN5168):          pcPartName = "JN5168";    dev = 68;   break;
        case (CHIP_ID_JN5168_COG07A):   pcPartName = "JN5168";    dev = 68;   break;
        case (CHIP_ID_JN5168_COG07B):   pcPartName = "JN5168";    dev = 68;   break;

        case (CHIP_ID_JN5169):          pcPartName = "JN5169";    dev = 69;   break;
        case (CHIP_ID_JN5169_DONGLE):   pcPartName = "JN5169";    dev = 69;   break;

        case (CHIP_ID_JN5172):          pcPartName = "JN5172";    dev = 72;   break;

        case (CHIP_ID_JN5179):          pcPartName = "JN5179";    dev = 79;   break;

        default:
            fprintf(stderr, "Module type unknown\n");
            dev = 5;
            break;
    }
    
    printf("Device = %s (%d)\n", pcPartName, dev);
        
    return dev;
}

#if 1
void dbg_vPrintfImpl(const char *pcFile, uint32_t u32Line, const char *pcFormat, ...)
{
    va_list argp;
    va_start(argp, pcFormat);
#ifdef DBG_VERBOSE
    printf("%s:%d ", pcFile, u32Line);
#endif /* DBG_VERBOSE */
    vprintf(pcFormat, argp);
    return;
}
#endif
