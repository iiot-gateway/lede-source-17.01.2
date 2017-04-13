// ---------------------------------------------------------------------
// Mifare Ultralight Handler
// ---------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ------------------------------------------------------------------

/** \addtogroup nd
 * \file
* \brief Mifare Ultralight Handler
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>

#include "linux_nfc_api.h"
#include "dump.h"
#include "nfcreader.h"
#include "ndef.h"
#include "audiofb.h"
#include "ledfb.h"
#include "commission.h"
#include "mful.h"

#define MFUL_DEBUG
#ifdef MFUL_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MFUL_DEBUG */

// ------------------------------------------------------------------
// Global data
// ------------------------------------------------------------------

extern int hasTag;

// ------------------------------------------------------------------
// Handler
// ------------------------------------------------------------------

/**
 * \brief Handle Mifare Ultralight tag by reading the NDEF uri and payload and calling the
 * appropriate uri handler (app). When necessary, the NDEF result is written back to the tag.
 * Audible feedback informs the user about the progress.
 * \param handle to tag
 */
void mfulHandle( int handle ) {
  
    DEBUG_PRINTF("\n\n******* MIFARE Ultralight handler ******\n\n");

    int plen;
    struct timeval start;
    struct timeval end;
    double elapsed;
    int first = 1;
    char payload[NDEF_MAX_PAYLOAD+2];
    char uri[NDEF_MAX_URI+2];

    gettimeofday( &start, NULL );

    while ( hasTag && 
            ( ( plen = ndef_read( handle, payload, NDEF_MAX_PAYLOAD, uri, NDEF_MAX_URI ) ) >= 0 ) ) {
      
        if ( plen > 0 ) {
            int ret = 0;
            printf( "MFUL URI: %s\n", uri );
            if ( strcmp( uri, COMMISSION_URI ) == 0 ) {
                ret = commissionHandleNfc( handle, payload, plen );
            } else {
              printf( "MFUL error: Unsupported URI\n" );
            }
            if ( ret <= 0 ) {
                afbPlayPattern( AFB_PATTERN_FOURTICKS );
                ledRedOn();
            } else {
                ledGreenOn();
            }
        } else if ( first ) {
            printf( "NDEF corrupt\n" );
            afbPlayPattern( AFB_PATTERN_FOURTICKS );
            ledRedOn();
        }
        
        first = 0;
                
        gettimeofday( &end, NULL );

        elapsed = (double)(end.tv_sec - start.tv_sec) + 
           ((double)(end.tv_usec - start.tv_usec) / 1000000.0F);

        printf( "NDEF loop: Elapsed REAL time: %.2f seconds\n\n",
                 elapsed );

        if ( hasTag ) usleep( 1000 * 1000 );   // war 250
        gettimeofday( &start, NULL );
    }
}

// ------------------------------------------------------------------
// Init / Reset
// ------------------------------------------------------------------

/**
 * \brief Calls the uri handler's reset function(s)
 */
void mfulReset( void ) {
    commissionReset();
}

/**
 * \brief Calls the uri handler's init function(s), initializes the Leds and Audio feedback
 * handles and gives the initial beeper sound.
 */
void mfulInit( void ) {
    commissionInit();
    
    ledInit();
    afbInit();
    afbPlayPattern( AFB_PATTERN_ONELONG );  // Beep once when ready
}