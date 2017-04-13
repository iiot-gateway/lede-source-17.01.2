// ---------------------------------------------------------------------
// LED Feedback Handler
// ---------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ------------------------------------------------------------------

/** \addtogroup nd
 * \file
* \brief LED Feedback Handler
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "ledfb.h"

// #define LED_DEBUG
#ifdef LED_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* LED_DEBUG */

#define GPIO_LED_GREEN     7
#define GPIO_LED_RED       8

// ------------------------------------------------------------------
// Globals
// ------------------------------------------------------------------

static int greenPin = 0;
static int redPin   = 0;

// ------------------------------------------------------------------
// LED control
// ------------------------------------------------------------------

/**
 * \brief LED green on
 */
void ledGreenOn( void ) {
    if ( greenPin ) write( greenPin, "1", 1 );
}

/**
 * \brief LED green off
 */
void ledGreenOff( void ) {
    if ( greenPin ) write( greenPin, "0", 1 );
}

/**
 * \brief LED red on
 */
void ledRedOn( void ) {
    if ( redPin ) write( redPin, "1", 1 );
}

/**
 * \brief LED red off
 */
void ledRedOff( void ) {
    if ( redPin ) write( redPin, "0", 1 );
}

// ------------------------------------------------------------------
// Verify pin
// ------------------------------------------------------------------

static int verifyPin( int pin ) {
    char buf[40];
    // Check if green-led gpio pin has been created
    int hasGpio = 0;
    sprintf( buf, "/sys/class/gpio/gpio%d", pin );
    int fd = open( buf, O_RDONLY );
    if ( fd <= 0 ) {
        // Pin not exported yet
        printf( "Create led pin %s\n", buf );
        if ( ( fd = open( "/sys/class/gpio/export", O_WRONLY ) ) > 0 ) {
        sprintf( buf, "%d", pin );
        if ( write( fd, buf, 2 ) == 2 ) {
            hasGpio = 1;
        }
        close( fd );
        }
    } else {
        DEBUG_PRINTF( "System already has pin %s\n", buf );
        hasGpio = 1;
        close( fd );
    }

    if ( hasGpio ) {
        // Make sure it is an output
        sprintf( buf, "/sys/class/gpio/gpio%d/direction", pin );
        fd = open( buf, O_WRONLY );
        if ( fd <= 0 ) {
            printf( "Could not open direction port '%s' (%s)",
                    buf, strerror(errno) );
        } else {
            if ( write(fd,"out",3) == 3 ) {
                DEBUG_PRINTF( "Pin %d now an output\n", pin );
            }
            close(fd);

            // Make sure it is off
            sprintf( buf, "/sys/class/gpio/gpio%d/value", pin );
            fd = open( buf, O_RDWR );
            if ( fd <= 0 ) {
                printf( "Could not open value port '%s' (%s)",
                        buf, strerror(errno) );
            } else {
                if ( write( fd, "0", 1 ) == 1 ) {
                    DEBUG_PRINTF( "Pin %d now off\n", pin );
                }
                return( fd );
            }
        }
    }

    return( 0 );
}

// ------------------------------------------------------------------
// Init
// ------------------------------------------------------------------

/**
 * \brief Init led feedback handler by allocating GPIO pins
 */
void ledInit( void ) {
    greenPin = verifyPin( GPIO_LED_GREEN );
    redPin   = verifyPin( GPIO_LED_RED   );
}

