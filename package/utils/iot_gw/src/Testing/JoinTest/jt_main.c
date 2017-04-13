// ------------------------------------------------------------------
// Join Test
// ------------------------------------------------------------------
// Tester for the secure joiner program, using JSON files as parameter
// Connects to the secure joiner via its socket (on localhost)
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \section joinertest Securer Joiner Testing
 * \brief Secure Joiner Test
 * Tester for the secure joiner program, using JSON files as parameter
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "socket.h"
#include "newLog.h"

#define SOCKET_PORT      "2000"

#define MAXFILENAME      40
#define INPUTBUFFERLEN   200

// ------------------------------------------------------------------------
// Globals
// ------------------------------------------------------------------------

static char jsonFileName[MAXFILENAME];
static char inputBuffer[INPUTBUFFERLEN + 2];

static char socketPort[80];

// ------------------------------------------------------------------------
// Perform the test
// ------------------------------------------------------------------------

/**
 * \brief Send a JSON file or a 'hello'-string, waits for a response and prints that
 * \param handle Handle to the Control Interface socket
 * \param filename Name of the JSON file to send. If empty, then send 'hello'
 * \returns 0 when OK, or -1 on error
 */
static int performAction( int handle, char * filename ) {
    char * hallo = "\"hallo\" : 10\n";
    int len = strlen( hallo );

    if ( strlen( filename ) > 0 ) {
        int fd;
        fd = open( filename, O_RDONLY );
        if ( fd == -1 ) {
            printf( "Error opening file %s (%d - %s)\n",
                 filename, errno, strerror( errno ) );
        } else {
            int numBytes;
            while ( ( numBytes = read( fd, inputBuffer, INPUTBUFFERLEN ) ) > 0 ) {
                inputBuffer[numBytes] = '\0';
                if ( socketWrite( handle, inputBuffer, numBytes ) != numBytes ) {
                    return( -1 );
                }
            }
        }
    } else {
        if ( socketWrite( handle, hallo, len ) != len ) {
            return( -1 );
        }
    }


    char buf[200];
    while ( ( len = socketReadWithTimeout( handle, buf, 200, 1 ) ) > 0 ) {
        printf( "Received from socket %d bytes\n", len );
        // dump( buf, len );
    }

    return( 0 );
}

// ------------------------------------------------------------------------
// Main
// ------------------------------------------------------------------------

/**
 * \brief Secure Joiner Test's main entry point: opens the socket to the
 * Secure Joiner and either sends a JSON file (parameter) or a 'hello'-string
 * message.
 * \param argc Number of command-line parameters
 * \param argv Parameter list (-h = help, -P <port> = TCP port -j <JSON file> = file to send)
 */
int main( int argc, char * argv[] ) {
    signed char opt;

    printf( "Join Tester Started\n" );

    strcpy( socketPort, SOCKET_PORT );
    jsonFileName[0] = '\0';

    while ( ( opt = getopt( argc, argv, "hP:j:" ) ) != -1 ) {
        switch ( opt ) {
        case 'h':
            printf( "Usage: jt [-P port] [-j testfile.json]\n" );
            exit( 0 );
        case 'P':
            strcpy( socketPort, optarg );
            break;
        case 'j':
            printf( "JSON file = %s\n", optarg );
            if ( strlen( optarg ) < MAXFILENAME ) {
                strcpy( jsonFileName, optarg );
            } else {
                printf( "JSON file name too long\n" );
            }
            break;
        }
    }

    newLogAdd( NEWLOG_FROM_TEST, "Join Tester Started" );

    int handle = socketOpen( "localhost", socketPort, 0 );

    if ( handle >= 0 ) {
        newLogAdd( NEWLOG_FROM_TEST, "Join Test socket opened" );

        performAction( handle, jsonFileName );

        socketClose( handle );
    } else { 
        sprintf( logbuffer, "Error opening port to %s\n", socketPort );
        newLogAdd( NEWLOG_FROM_TEST, logbuffer );
    }

    printf( "Join Tester exit\n");
    return( 0 );
}

