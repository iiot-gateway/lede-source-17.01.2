// ------------------------------------------------------------------
// Plug Tester
// ------------------------------------------------------------------
// Tests the Plugmeter system by generating plug meter updates in 
// the IoT database
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \section plugtest Plug Meter Testing
 * \brief Tests the Plugmeter system by generating plug meter updates in the IoT database
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <mqueue.h>
#include <time.h>

#include "newLog.h"
#include "queue.h"
#include "newDb.h"
#include "jsonCreate.h"
#include "plugUsage.h"

// -------------------------------------------------------------
// Main
// -------------------------------------------------------------

/**
 * \brief Plugmeter Test's main entry point: opens the database 
 * and inserts a plug updates with increasing "sum" value.
 * An update is sent every 10 seconds.
 */
int main( int argc, char * argv[] ) {

    printf( "\n\nPlug meter testing\n");

    newLogAdd( NEWLOG_FROM_TEST, "Plug meter testing started" );

    char * mac = "ABC0000001";
    int cleanupCnt = 0;

    srand( (unsigned)time(NULL) );
    
    newDbOpen();

    while ( 1 ) {
        
        int act = rand() % 200;
    
        newdb_dev_t device;
        device.flags = FLAG_NONE;
        if ( newDbGetDevice( mac, &device ) ) {
            device.act = act;
            device.sum += act / 6;
            device.flags |= FLAG_DEV_JOINED;
            newDbSetDevice( &device );
        } else if ( newDbGetNewDevice( mac, &device ) ) {
            device.dev = DEVICE_DEV_PLUG;
            device.act = 0;
            device.sum = 0;
            device.flags = FLAG_DEV_JOINED | FLAG_ADDED_BY_GW;
            newDbSetDevice( &device );
        }

        if ( device.flags != FLAG_NONE ) {

            int now = (int)time( NULL );
        
            newdb_plughist_t plughist;
            if ( newDbGetMatchingOrNewPlugHist( mac, now/60, &plughist ) ) {
                plughist.sum = device.sum;
                newDbSetPlugHist( &plughist );
            
                // Clean history entries older than one day
                if ( cleanupCnt-- < 0 ) {
                    if ( plugHistoryCleanup( mac, now ) ) {
                        cleanupCnt = 25;
                    }
                }
            }
        }
        
        sprintf( logbuffer, "MAC %s: Act = %d", mac, act );
        newLogAdd( NEWLOG_FROM_TEST, logbuffer );

        sleep( 10 );
    }
    
    newDbClose();

    return( 0 );
}
