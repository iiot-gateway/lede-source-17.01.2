// ---------------------------------------------------------------------
// Audio Feedback Handler
// ---------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ------------------------------------------------------------------

/** \addtogroup nd
 * \file
* \brief Audio Feedback Handler
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "audiofb.h"

// #define AFB_DEBUG
#ifdef AFB_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* AFB_DEBUG */

#define GPIO_EXPORT        "/sys/class/gpio/export"
#define GPIO_BEEPER_PIN    "/sys/class/gpio/gpio22"
#define GPIO_BEEPER_VAL    "/sys/class/gpio/gpio22/value"
#define GPIO_BEEPER_DIR    "/sys/class/gpio/gpio22/direction"

// ------------------------------------------------------------------
// Globals
// ------------------------------------------------------------------

static pthread_t tid;
static int threadPattern = AFB_PATTERN_SILENT;
static int beeperPin = 0;

// ------------------------------------------------------------------
// Beeper HW pin
// ------------------------------------------------------------------

static void beeperOn( void ) {
    DEBUG_PRINTF( "Beeper pin HIGH\n" );
    if ( beeperPin ) write( beeperPin, "1", 1 );
}

static void beeperOff( void ) {
    DEBUG_PRINTF( "Beeper pin LOW\n" );
    if ( beeperPin ) write( beeperPin, "0", 1 );
}

// ------------------------------------------------------------------
// Beeper thread
// ------------------------------------------------------------------

#define BEEP_LONG    400   // msec
#define BEEP_MEDIUM  160   // msec
#define BEEP_SHORT    80   // msec
#define BEEP_TICK     20   // msec
#define BEEP_PAUSE    80   // msec

static void * doAudioFb( void * arg ) {
    int pattern = *((int *)arg);

    DEBUG_PRINTF("AudioFb, pattern = %d\n", pattern );

    switch ( pattern ) {

    case AFB_PATTERN_ONELONG:
        beeperOn();
        usleep( BEEP_LONG * 1000 );
        // beeperOff();
        break;

    case AFB_PATTERN_TWOMEDIUM:
        beeperOn();
        usleep( BEEP_MEDIUM * 1000 );
        beeperOff();
        usleep( BEEP_PAUSE * 1000 );
        beeperOn();
        usleep( BEEP_MEDIUM * 1000 );
        // beeperOff();
        break;

    case AFB_PATTERN_SHORTLONG:
        beeperOn();
        usleep( BEEP_SHORT * 1000 );
        beeperOff();
        usleep( BEEP_PAUSE * 1000 );
        beeperOn();
        usleep( BEEP_LONG * 1000 );
        // beeperOff();
        break;

    case AFB_PATTERN_THREESHORT:
        beeperOn();
        usleep( BEEP_SHORT * 1000 );
        beeperOff();
        usleep( BEEP_PAUSE * 1000 );
        beeperOn();
        usleep( BEEP_SHORT * 1000 );
        beeperOff();
        usleep( BEEP_PAUSE * 1000 );
        beeperOn();
        usleep( BEEP_SHORT * 1000 );
        // beeperOff();
        break;

    case AFB_PATTERN_FOURTICKS:
        beeperOn();
        usleep( BEEP_TICK * 1000 );
        beeperOff();
        usleep( BEEP_PAUSE * 1000 );
        beeperOn();
        usleep( BEEP_TICK * 1000 );
        beeperOff();
        usleep( BEEP_PAUSE * 1000 );
        beeperOn();
        usleep( BEEP_TICK * 1000 );
        beeperOff();
        usleep( BEEP_PAUSE * 1000 );
        beeperOn();
        usleep( BEEP_TICK * 1000 );
        // beeperOff();
        break;

    }

    // Always end with beeper off
    beeperOff();

    // printf( "Pthread ends\n" );
    threadPattern = AFB_PATTERN_SILENT;

    return NULL;
}

// ------------------------------------------------------------------
// Start / stop thread
// ------------------------------------------------------------------

static void startThread( int mode ) {
    DEBUG_PRINTF( "Start thread %d\n", mode );

    threadPattern = mode;
    int err = pthread_create( &tid, NULL, &doAudioFb, (void *)&threadPattern);
    if (err != 0) {
        printf("Can't create thread :[%s]", strerror(err));
    } else {
        DEBUG_PRINTF("Thread created successfully\n");
    }
}

static void killPrevious( void ) {
    int err;
    DEBUG_PRINTF( "Stop thread %d\n", threadPattern );
    if ( threadPattern != AFB_PATTERN_SILENT ) {
        // Kill previous thread
        threadPattern = AFB_PATTERN_SILENT;

        // Wait for thread to finish
        void * res;
        err = pthread_join(tid, &res);
        if (err != 0) {
            printf( "Error %d joining thread\n", err );
        }

        DEBUG_PRINTF( "Thread returned value %s\n", (char *) res );
        free( res );
    }
}

// ------------------------------------------------------------------
// Init
// ------------------------------------------------------------------

/**
 * \brief Init audio feedback handler by allocating GPIO pin and muting beeper sound
 */
void afbInit( void ) {
    // No sound for the moment
    threadPattern = AFB_PATTERN_SILENT;
    
    // Check if beeper gpio pin has been created
    int hasBeeperGpio = 0;
    int fd = open( GPIO_BEEPER_PIN, O_RDONLY );
    if ( fd <= 0 ) {
        // Pin not exported yet
        DEBUG_PRINTF( "Create beeper pin\n" );
        if ( ( fd = open( GPIO_EXPORT, O_WRONLY ) ) > 0 ) {
        if ( write( fd, "22", 2 ) == 2 ) {
            hasBeeperGpio = 1;
        }
        close( fd );
        sleep( 1 );
        }
    } else {
        DEBUG_PRINTF( "System already has %s\n", GPIO_BEEPER_PIN );
        hasBeeperGpio = 1;
        close( fd );
    }

    if ( hasBeeperGpio ) {
        // Make sure it is an output
        beeperPin = open( GPIO_BEEPER_DIR, O_WRONLY );
        if ( beeperPin <= 0 ) {
            printf( "Could not open beeper direction port '%s' (%s)",
                    GPIO_BEEPER_DIR, strerror(errno) );
            beeperPin = 0;
        } else {
            write(beeperPin,"out",3);
            close(beeperPin);
        }

        // Make sure it is off
        beeperPin = open( GPIO_BEEPER_VAL, O_RDWR );
        if ( beeperPin <= 0 ) {
            printf( "Could not open beeper value port '%s' (%s)",
                    GPIO_BEEPER_VAL, strerror(errno) );
            beeperPin = 0;
        } else {
            beeperOff();
        }
    }
}

// ------------------------------------------------------------------
// Controls
// ------------------------------------------------------------------

/**
 * \brief Start sound-pattern thread
 */
void afbPlayPattern( int pattern ) {
    DEBUG_PRINTF( ">>>>>>>>>> Pattern %d\n", pattern );
    // Start thread to make sound-pattern
    killPrevious();
    startThread( pattern );
}

/**
 * \brief Stop sound thread
 */
void afbOff( void ) {
    DEBUG_PRINTF( ">>>>>>>>>> Beeper OFF\n" );
    // Stop thread
    killPrevious();
}

