// ------------------------------------------------------------------
// FTDI CBUS control program
// ------------------------------------------------------------------
// Program that drives CB[2-3] pins on FTDI for JN516x programming
// https://github.com/legege/libftdi/blob/master/examples/bitbang_cbus.c
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftdi.h>
#include <unistd.h>

// #define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

struct ftdi_context *ftdi;

// CB[2-3] = output, high 1100 1100
char bitmask = 0xCC;

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------

void ftdiCbus( void ) {
    unsigned char buf[1];
    int ret = ftdi_set_bitmode(ftdi, bitmask, BITMODE_CBUS);
    if (ret < 0) {
        fprintf(stderr, "set_bitmode failed for 0x%x, error %d (%s)\n", bitmask, ret, ftdi_get_error_string(ftdi));
        ftdi_usb_close(ftdi);
        ftdi_deinit(ftdi);
        exit(-1);
    }

    // read CBUS
    ret = ftdi_read_pins(ftdi, &buf[0]);
    if (ret < 0) {
        fprintf(stderr, "read_pins failed, error %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        ftdi_usb_close(ftdi);
        ftdi_deinit(ftdi);
        exit(-1);
    }
    DEBUG_PRINTF("Read returned 0x%01x\n", buf[0] & 0x0f);
}

void ftdiReset( void ) {
    DEBUG_PRINTF( "Reset\n" );
    bitmask &= ~0x04;
    ftdiCbus();
}

void ftdiNoReset( void ) {
    DEBUG_PRINTF( "No Reset\n" );
    bitmask |= 0x04;
    ftdiCbus();
}

void ftdiProg( void ) {
    DEBUG_PRINTF( "Prog\n" );
    bitmask &= ~0x08;
    ftdiCbus();
}

void ftdiNoProg( void ) {
    DEBUG_PRINTF( "No Prog\n" );
    bitmask |= 0x08;
    ftdiCbus();
}

// ------------------------------------------------------------------
// MAIN
// ------------------------------------------------------------------

int main( int argc, char * argv[] )
{
    int ret;
    if ((ftdi = ftdi_new()) == 0) {
        fprintf(stderr, "ftdi_new failed\n");
        return EXIT_FAILURE;
    }
    
    
#ifdef TARGET_RASPBIAN
#ifdef MAIN_DEBUG
    struct ftdi_version_info version;
    version = ftdi_get_library_version();
    printf("Initialized libftdi %s (major: %d, minor: %d, micro: %d, snapshot ver: %s)\n",
        version.version_str, version.major, version.minor, version.micro,
        version.snapshot_str);
#endif
#endif
    
    if ((ret = ftdi_usb_open(ftdi, 0x0403, 0x6001)) < 0) { // check idVendor/idProduct for JN5168 or JN5169 USB Dongle
        if ((ret = ftdi_usb_open(ftdi, 0x0403, 0x6015)) < 0) { // check idVendor/idProduct for JN5179 USB Dongle
            fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
            ftdi_free(ftdi);
            return EXIT_FAILURE;
        }
    }
    
#ifdef MAIN_DEBUG
    // Read out FTDIChip-ID of R type chips
    if (ftdi->type == TYPE_R) {
        unsigned int chipid;
        printf("ftdi_read_chipid: %d\n", ftdi_read_chipid(ftdi, &chipid));
        printf("FTDI chipid: %X\n", chipid);
    }
#endif

    if ( argc <= 1 ) {
        printf( "Usage: %s [prog|normal]\n", argv[0] );
    } else if ( strcmp( argv[1], "prog" ) == 0 ) {
        ftdiProg();
        usleep( 100 * 1000 );
        ftdiReset();
        usleep( 100 * 1000 );
        ftdiNoReset();
        usleep( 200 * 1000 );
    } else {
        ftdiNoProg();
        usleep( 100 * 1000 );
        ftdiReset();
        usleep( 100 * 1000 );
        ftdiNoReset();
        usleep( 200 * 1000 );
    }

    if ((ret = ftdi_usb_close(ftdi)) < 0)
    {
        fprintf(stderr, "unable to close ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        return EXIT_FAILURE;
    }
    ftdi_free(ftdi);
    return EXIT_SUCCESS;
}
