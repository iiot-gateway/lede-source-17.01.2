// ------------------------------------------------------------------
// Commission
// ------------------------------------------------------------------
// Sends authorize CMD to Zigbee Coordinator.
// The response ZCB is expected from the joiner queue
// and translated into a secure join message towards the called.
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \addtogroup sj
 * \file
 * \brief Sends authorize CMD to Zigbee Coordinator
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "strtoupper.h"
#include "iotError.h"
#include "newLog.h"
#include "parsing.h"
#include "queue.h"
#include "socket.h"
#include "json.h"
#include "jsonCreate.h"
#include "nibbles.h"
#include "dump.h"
#include "commission.h"

// #define COMM_DEBUG

#ifdef COMM_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* COMM_DEBUG */

// ------------------------------------------------------------------
// ZCB parsing
// ------------------------------------------------------------------

#define LEN_AUTH     16
#define LEN_NWKEY    32
#define LEN_MIC      8
#define LEN_TCADDR   16
#define LEN_PAN      4
#define LEN_EXTPAN   16

static int  zcbOk;
static int  keyseq;
static int  channel;
static char authorize[LEN_AUTH+2];
static char nwkey[LEN_NWKEY+2];
static char mic[LEN_MIC+2];
static char tcaddress[LEN_TCADDR+2];
static char pan[LEN_PAN+2];
static char extpan[LEN_EXTPAN+2];

void zcb_onError(int error, char * errtext, char * lastchars) {
    printf("onError( %d, %s ) @ %s\n", error, errtext, lastchars);
}

void zcb_onObjectStart(char * name) {
    DEBUG_PRINTF("onObjectStart( %s )\n", name);
    keyseq       = -1;
    channel      = -1;
    authorize[0] = '\0';
    nwkey[0]     = '\0';
    mic[0]       = '\0';
    tcaddress[0] = '\0';
    pan[0]       = '\0';
    extpan[0]    = '\0';
}

void zcb_onObjectComplete(char * name) {
    DEBUG_PRINTF("onObjectComplete( %s )\n", name);
    DEBUG_PRINTF(" - keyseq    = %d\n", keyseq );
    DEBUG_PRINTF(" - channel   = %d\n", channel );
    DEBUG_PRINTF(" - authorize = %s\n", authorize );
    DEBUG_PRINTF(" - nwkey     = %s\n", nwkey );
    DEBUG_PRINTF(" - mic       = %s\n", mic );
    DEBUG_PRINTF(" - tcaddress = %s\n", tcaddress );
    DEBUG_PRINTF(" - pan       = %s\n", pan );
    DEBUG_PRINTF(" - extpan    = %s\n", extpan );
    if ( ( keyseq  != -1 ) &&
         ( channel != -1 ) &&
         ( strlen( authorize ) != 0 ) &&
         ( strlen( nwkey     ) != 0 ) &&
         ( strlen( mic       ) != 0 ) &&
         ( strlen( tcaddress ) != 0 ) &&
         ( strlen( pan       ) != 0 ) &&
         ( strlen( extpan    ) != 0 ) ) {
        zcbOk = 1;
    }
}

void zcb_onArrayStart(char * name) {
    DEBUG_PRINTF("onArrayStart( %s )\n", name);
}

void zcb_onArrayComplete(char * name) {
    DEBUG_PRINTF("onArrayComplete( %s )\n", name);
}

void zcb_onString(char * name, char * value) {
    DEBUG_PRINTF("onString( %s, %s )\n", name, value);
    int error = 0;
    if ( strcmp( name, "authorize" ) == 0 ) {
        if ( strlen( value ) <= LEN_AUTH ) strcpy( authorize, value );
        else error = 1;
    } else if ( strcmp( name, "key" ) == 0 ) {
        if ( strlen( value ) <= LEN_NWKEY ) strcpy( nwkey, value );
        else error = 2;
    } else if ( strcmp( name, "mic" ) == 0 ) {
        if ( strlen( value ) <= LEN_MIC ) strcpy( mic, value );
        else error = 3;
    } else if ( strcmp( name, "tcaddr" ) == 0 ) {
        if ( strlen( value ) <= LEN_TCADDR ) strcpy( tcaddress, value );
        else error = 4;
    } else if ( strcmp( name, "pan" ) == 0 ) {
        if ( strlen( value ) <= LEN_PAN ) strcpy( pan, value );
        else error = 5;
    } else if ( strcmp( name, "extpan" ) == 0 ) {
        if ( strlen( value ) <= LEN_EXTPAN ) strcpy( extpan, value );
        else error = 6;
    }
    if ( error ) {
        sprintf( logbuffer, "String error %d", error );
        newLogAdd( NEWLOG_FROM_SECURE_JOINER, logbuffer );
    }
}

void zcb_onInteger(char * name, int value) {
    DEBUG_PRINTF("onInteger( %s, %d )\n", name, value);
    if ( strcmp( name, "keyseq" ) == 0 ) {
        keyseq = value;
    } else if ( strcmp( name, "chan" ) == 0 ) {
        channel = value;
    }
}

// ------------------------------------------------------------------
// Handle
// ------------------------------------------------------------------

/**
 * \brief Receives credentials (mac, linkkey) of the device that wants to join, packs that into
 * a JSON request towards the ZCB-JenOS, reads from the joiner queue for the commissioning response,
 * parses that, packs the results in a JSON message, and sends that back to the caller client
 * \param socketHandle Handle to the client socket to which the commissioning result must be sent
 * \param mac Mac of the device that wants to join
 * \param linkkey Link-key of the device that wants to join
 */
void commission( int socketHandle,
                 char * mac,
                 char * linkkey ) {
    // linkinfoDump();

    newLogAdd( NEWLOG_FROM_SECURE_JOINER, "Handle authorization request" );

    int joinerQueue = -1;
    
    strtoupper( mac );

    // Assemble JSON authorize-message towards coordinator
    char * message = jsonCmdAuthorizeRequest( mac, linkkey );
    char * response;

    // Send message to the ZCB and Wait for response on joiner queue
    // - First open my own joiner queue (for the answer) and flush it
    if ( ( joinerQueue = queueOpen( QUEUE_KEY_SECURE_JOINER, 0 ) ) != -1 ) {
        // printf( "Joiner queue opened\n" );

        int numBytes = 0;
        char queueInputBuffer[MAXMESSAGESIZE+2];

        // Flush the joiner queue
        while ( queueReadWithMsecTimeout( joinerQueue, queueInputBuffer,
                                          MAXMESSAGESIZE, 100 ) > 0 ) ;

        // Send to the ZCB message queue
        if ( queueWriteOneMessage( QUEUE_KEY_ZCB_IN, message ) ) {

            printf( "Wait for a response on the joiner queue ...\n" );

            numBytes = queueReadWithMsecTimeout( joinerQueue,
                      queueInputBuffer, MAXMESSAGESIZE, 2000 );

            if ( numBytes > 0 ) {

                // Parse (ZCB authorize) message
                jsonSelectNext();

                jsonSetOnError(zcb_onError);
                jsonSetOnObjectStart(zcb_onObjectStart);
                jsonSetOnObjectComplete(zcb_onObjectComplete);
                jsonSetOnArrayStart(zcb_onArrayStart);
                jsonSetOnArrayComplete(zcb_onArrayComplete);
                jsonSetOnString(zcb_onString);
                jsonSetOnInteger(zcb_onInteger);
                jsonReset();

                zcbOk = 0;

                int i;
                for ( i=0; i<numBytes; i++ ) {
                    DEBUG_PRINTF( "%c", queueInputBuffer[i] );
                    jsonEat( queueInputBuffer[i] );
                }

                jsonSelectPrev();

                // Parsing went OK and the response matches the request
                if ( zcbOk && ( strcmp( mac, authorize ) == 0 ) ) {

                    // Assemble JSON join-secure-message towards client
                    response = jsonJoinSecure( channel, keyseq,
                           pan, extpan, nwkey, mic, tcaddress );

                } else {
                    response = jsonError( IOT_ERROR_AUTHORIZE_RESPONSE );
                }

            } else {
                // Create timeout error message
                response = jsonError( IOT_ERROR_AUTHORIZE_TIMEOUT );
            }

            // Output the response over the socket to the client
            socketWrite( socketHandle, response, strlen( response ) );

        } else {
            printf( "Error sending authorize to the ZCB queue\n" );
        }

        queueClose( joinerQueue );

    } else {
        printf( "Error opening joiner queue\n" );
    }
}

