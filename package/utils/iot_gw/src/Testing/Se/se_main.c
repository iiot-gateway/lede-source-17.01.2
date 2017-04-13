// ------------------------------------------------------------------
// Secure Element test program
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>

#include "dump.h"

#include "newLog.h"

// ------------------------------------------------------------------------
// GENERAL DEFINITIONS
// ------------------------------------------------------------------------

#define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

#define I2C_BUS          "/dev/i2c-1"

#define I2C_SEADDRESS    0x48
// #define I2C_SEADDRESS    0x50
// #define I2C_SEADDRESS    0x90

#define SE_I2C_MAX_RETRIES      4

// ----------------------------------------------------------------------------
// Global variables
// ----------------------------------------------------------------------------

static int iI2CFd = 0;

// ----------------------------------------------------------------------------
// Register access
// ----------------------------------------------------------------------------

static int se_write_byte( char byte ) {
    int len = write(iI2CFd, &byte, 1);
    if (len != 1) {
        printf( "Error (%s) writing byte 0x%02x to SE\n",
                strerror(errno), byte );
        return( 0 );
    }
    return( 1 );
}

// http://bunniestudios.com/blog/images/infocast_i2c.c

static int se_write_read_bytes( char * out, int outlen,
                                char * in,  int inlen ) {
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    // Create messages
    messages[0].addr  = I2C_SEADDRESS;
    messages[0].flags = 0;
    messages[0].len   = outlen;
    messages[0].buf   = (unsigned char *)out;

    messages[1].addr  = I2C_SEADDRESS;
    messages[1].flags = I2C_M_RD | I2C_M_NOSTART;
    messages[1].len   = inlen;
    messages[1].buf   = (unsigned char *)in;

    // Send the request to the kernel and get the result back
    packets.msgs      = messages;
    packets.nmsgs     = 2;
    if(ioctl(iI2CFd, I2C_RDWR, &packets) < 0) {
        perror("Unable to send data");
        return 0;
    }

    printf( "SE write: " ); dump( out, outlen );
    printf( "SE -read: " ); dump( in, inlen );

    return inlen;
}

static int se_read_bytes( void ) {
    char in[40];
    int len = read(iI2CFd, in, 16);
    if (len == 16) {
//        len = in[0];
//        if ( len <= 16 ) {
            printf( "SE read: " );
            dump( in, len );
//        }
        return( len );
    } else {
        printf( "Error (%s) reading bytes from SE (%d)\n",
                strerror(errno), len );
    }
    return( 0 );
}

#define SE_STATUS       0x07
#define SE_WAKEUP       0x0F
#define SE_SOFTRESET    0x1F
#define SE_RESETANSWER  0x2F

static void se_wakeup( void ) {
    printf( "Wakeup\n" );
    se_write_byte( SE_WAKEUP );
}

static void se_softreset( void ) {
    printf( "Soft reset\n" );
    char cmd = SE_SOFTRESET;
    char in[16];
    se_write_read_bytes( &cmd, 1, in, 16 );
}

static void se_resetanswer( void ) {
    printf( "Reset answer\n" );
    char cmd = SE_RESETANSWER;
    char in[16];
    se_write_read_bytes( &cmd, 1, in, 16 );
}

static void se_status( void ) {
    printf( "Status\n" );
    char cmd = SE_STATUS;
    char in[16];
    se_write_read_bytes( &cmd, 1, in, 16 );
}


static int se_read_register( unsigned int reg, char * data ) {
    int retries = SE_I2C_MAX_RETRIES;
    char out[4], in[4];
    out[0] = ( reg >> 8 ) & 0xFF;
    out[1] = reg & 0xFF;
    out[2] = 0;
    out[3] = 0;
    while ( retries >= 0 ) {

        int len = write(iI2CFd, out, 2);
        if (len == 2) {
            len = read(iI2CFd, in, 4);
            if (len == 4) {
                if ( in[1] != 0xFF ) {
                    printf( "SE read %d: ", reg );
                    dump( in, 4 );
                }
                return( 1 );
            } else {
                printf( "Error (%s) reading 4 bytes from SE 0x%04x (%d)\n",
                        strerror(errno), reg, len );
            }
        } else {
            printf( "Error (%s) writing 2 bytes to SE (%d)\n",
                    strerror(errno), len );
        }

        retries--;
        usleep( 1 * 100 );    
    }

    *data = 0;
    return( 0 );
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#define SFR_I2C_STAL   0x1B4

int se_read_status( void ) {
    char kok[4];
    se_read_register( SFR_I2C_STAL, kok );
    return( 0x1234 );
}

// ----------------------------------------------------------------------------
// Init
// ----------------------------------------------------------------------------

int se_init( char * i2cBus, int i2cAddress ) {

    // I2C bus
    iI2CFd = open( i2cBus, O_RDWR | O_NOCTTY);
    if (iI2CFd < 0) {
        DEBUG_PRINTF( "Could not open I2C bus '%s' (%s)", 
           i2cBus, strerror(errno));
        return( 0 );
    }

    // I2C bus
    if (ioctl(iI2CFd, I2C_SLAVE, i2cAddress) < 0) {
        DEBUG_PRINTF( "Cannot select SE I2C address (%s)", strerror(errno));
        close(iI2CFd);
        return( 0 );
    }

    DEBUG_PRINTF( "I2C bus opened to 0x%02x\n", i2cAddress );
    return( 1 );
}

// -------------------------------------------------------------
// Main
// -------------------------------------------------------------

int main( int argc, char * argv[] ) {

  newLogAdd( NEWLOG_FROM_TEST, "Secure Element test started" );

  if ( se_init(I2C_BUS, I2C_SEADDRESS) ) {

    se_wakeup();
    usleep( 400 * 1000 );
//    se_softreset();
//    usleep( 200 * 1000 );
    se_resetanswer();
    usleep( 200 * 1000 );

    while ( 1 ) {
        se_status();
        usleep( 1000 * 1000 );
    }

    while ( 1 ) {
        se_softreset();
        usleep( 1000 * 1000 );
        se_resetanswer();
        usleep( 1000 * 1000 );
    }

    int cnt = 0;
    char data;
    while ( 1 ) {
        se_read_register( cnt, &data );
        if ( ++cnt > 0xFFFF ) cnt = 0;
    }

    int status = se_read_status();
    printf( "SE status = 0x%02x\n", status );

  } else {
    DEBUG_PRINTF("Failed to init SE\n");
  }

  return( 0 );
}

