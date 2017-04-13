// ------------------------------------------------------------------
// Secure Joiner Server
// ------------------------------------------------------------------
// Program that offers a socket interface over which clients can get
// their linkinfo struct encoded into a securejoin struct
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \addtogroup sj
 * \file
 * \section securejoiner Secure Joiner
 * \brief Secure Joiner socket program that handles commissioning requests from the LAN.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <mqueue.h>

#include "iotError.h"
#include "parsing.h"
#include "json.h"
#include "jsonCreate.h"
#include "queue.h"
#include "socket.h"
#include "newDb.h"
#include "newLog.h"
#include "gateway.h"

#include "linkinfo.h"
#include "commission.h"

#define SOCKET_HOST           "0.0.0.0"
#define SOCKET_PORT           "2000"

#define INPUTBUFFERLEN        200

// #define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

// -------------------------------------------------------------
// Globals
// -------------------------------------------------------------

static int  running = 1;

static linkinfo_t * linkinfo_read;

static char socketHost[80];
static char socketPort[80];

static int  globalClientSocketHandle;

// -------------------------------------------------------------
// Parsing
// -------------------------------------------------------------

static void sj_onError(int error, char * errtext, char * lastchars) {
    printf("onError( %d, %s ) @ %s\n", error, errtext, lastchars);
}

static void sj_onObjectStart(char * name) {
    // printf("onObjectStart( %s )\n", name);
    parsingReset();
}

/**
 * \brief Object parser: Calls LinkInfo parser in case of a linkinfo object
 * and then starts a commissioning sequence with the ZCB-JenOS.
 */
static void sj_onObjectComplete(char * name) {
    // printf("onObjectComplete( %s )\n", name);
    if ( strcmp( name, "linkinfo" ) == 0 ) {
        linkinfo_read = linkinfoHandle();

        if ( linkinfo_read != NULL ) {
            commission( globalClientSocketHandle,
                        linkinfo_read->mac,
                        linkinfo_read->linkkey );
        }
    }
}

static void sj_onArrayStart(char * name) {
    // printf("onArrayStart( %s )\n", name);
}

static void sj_onArrayComplete(char * name) {
    // printf("onArrayComplete( %s )\n", name);
}

static void sj_onString(char * name, char * value) {
    // printf("onString( %s, %s )\n", name, value);
    parsingStringAttr( name, value );
}

static void sj_onInteger(char * name, int value) {
    // printf("onInteger( %s, %d )\n", name, value);
    parsingIntAttr( name, value );
}

// ------------------------------------------------------------------------
// Handle client
// ------------------------------------------------------------------------

/**
 * \brief Client child process to parse and handle the JSON commands.
 * \param socketHandle Handle to the client's TCP socket
 */

void handleClient( int socketHandle ) {
    // printf( "Handling client %d\n", socketHandle );
    globalClientSocketHandle = socketHandle;

    // Start with clean sheet
    iotError = IOT_ERROR_NONE;
    int ok = 1;

    char socketInputBuffer[INPUTBUFFERLEN + 2];

    jsonReset();

    linkinfo_read  = NULL;

    while( ok ) {
        int len;
        len = socketRead( socketHandle, socketInputBuffer, INPUTBUFFERLEN ); 
        if ( len <= 0 ) {
            ok = 0;
        } else {
            // printf( "Incoming packet, length %d\n", len );

            int i;
            for ( i=0; i<len; i++ ) {
                // printf( "%c", socketInputBuffer[i] );
                jsonEat( socketInputBuffer[i] );
            }
        }
    }
}

// ------------------------------------------------------------------------
// QUIT handler
// ------------------------------------------------------------------------

static void vQuitSignalHandler( int sig ) {
    DEBUG_PRINTF("Got signal %d\n", sig );

    switch ( sig ) {
    case SIGTERM:
    case SIGINT:
    case SIGKILL:
        running = 0;
        exit(0);
        break;
    }

    signal (sig, vQuitSignalHandler);
}

// ------------------------------------------------------------------------
// Main
// ------------------------------------------------------------------------

/**
 * \brief Secure Joiner's main entry point: initializes the JSON parsers and opens the
 * server socket to wait for clients. The server can handle one client at a time.
 * \param argc Number of command-line parameters
 * \param argv Parameter list (-h = help, -H <ip> is IP address, -P <port> = TCP port)
 */
int main( int argc, char * argv[] ) {
    signed char opt;

    signal( SIGTERM, vQuitSignalHandler );
    signal( SIGINT,  vQuitSignalHandler );

    printf( "\n\nSecure Joiner Started\n" );

    strcpy( socketHost, SOCKET_HOST );
    strcpy( socketPort, SOCKET_PORT );

    while ( ( opt = getopt( argc, argv, "hH:P:" ) ) != -1 ) {
        switch ( opt ) {
        case 'h':
            printf( "Usage: ci [-H host] [-P port]\n\n");
            exit(0);
        case 'H':
            strcpy( socketHost, optarg );
            break;
        case 'P':
            strcpy( socketPort, optarg );
            break;
        }
    }

    newLogAdd( NEWLOG_FROM_SECURE_JOINER, "Secure Joiner Started" );

    int serverSocketHandle = socketOpen( socketHost, socketPort, 1 );

    if ( serverSocketHandle >= 0 ) {
        newLogAdd( NEWLOG_FROM_SECURE_JOINER, "Secure Joiner socket opened" );

        jsonSetOnError(sj_onError);
        jsonSetOnObjectStart(sj_onObjectStart);
        jsonSetOnObjectComplete(sj_onObjectComplete);
        jsonSetOnArrayStart(sj_onArrayStart);
        jsonSetOnArrayComplete(sj_onArrayComplete);
        jsonSetOnString(sj_onString);
        jsonSetOnInteger(sj_onInteger);
        jsonReset();

        printf( "Init parsers ...\n" );

        linkinfoInit();

        printf( "Waiting for clients to serve from socket %s/%s ...\n",
            socketHost, socketPort );

        while ( running ) {

            int clientSocketHandle = socketAccept( serverSocketHandle );
            if ( clientSocketHandle >= 0 ) {
                handleClient( clientSocketHandle );
                socketClose( clientSocketHandle );
            }

            checkOpenFiles( 6 );   // STDIN, STDOUT, STDERR, ServerSocket, DB
        }

        // printf( "Secure Joiner exit\n");
        socketClose( serverSocketHandle );

    } else {
        sprintf( logbuffer, "Error opening Secure Joiner socket %s/%s\n",
                 socketHost, socketPort );
        newLogAdd( NEWLOG_FROM_SECURE_JOINER, logbuffer );
    }

    newLogAdd( NEWLOG_FROM_SECURE_JOINER, "Exit" );
    return( 0 );
}

