// ------------------------------------------------------------------------
// Control Interface Test
// ------------------------------------------------------------------------
// Tests the Control Interface-socket-handler via JSON files as a parameter
// ------------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------------

/** \file
 * \section controltest Control Interface Testing
 * \brief Control Interface Test
 * Tests the Control Interface-socket-handler via JSON files as a parameter
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "newLog.h"
#include "socket.h"

#define SOCKET_PORT      "2001"

#define MAXFILENAME      40
#define INPUTBUFFERLEN   200

// ------------------------------------------------------------------------
// Globals
// ------------------------------------------------------------------------

static char jsonFileName[MAXFILENAME];
static char inputBuffer[INPUTBUFFERLEN];
static int  wait = 1;

static char socketPort[80];

// ------------------------------------------------------------------------
// Perform the test
// ------------------------------------------------------------------------

/**
 * \brief Send a JSON file or a 'hello'-string
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


    if ( wait ) {
        char buf[200];
        while ( ( len = socketReadWithTimeout( handle, buf, 200, 5 ) ) > 0 ) {
            printf( "Received from socket %d bytes\n", len );
            // dump( buf, len );
        }
    }

    return( 0 );
}

// ------------------------------------------------------------------------
// Main
// ------------------------------------------------------------------------

/**
 * \brief Control Interface Test's main entry point: opens the socket to the
 * Control Interface and either sends a JSON file (parameter) or a 'hello'-string
 * message.
 * \param argc Number of command-line parameters
 * \param argv Parameter list (-h = help, -P <port> = TCP port -j <JSON file> = file to send, -n = do not wait for a response)
 */
int main( int argc, char * argv[] ) {
    signed char opt;

    printf( "Control Interface Tester Started\n" );

    strcpy( socketPort, SOCKET_PORT );
    jsonFileName[0] = '\0';

    while ( ( opt = getopt( argc, argv, "hP:j:n" ) ) != -1 ) {
        switch ( opt ) {
        case 'h':
            printf( "Usage: ct [-P port] [-n] [-j testfile.json]\n" );
            exit( 0 );
        case 'P':
            strcpy( socketPort, optarg );
            break;
        case 'n':
            // No wait
            printf( "No wait\n" );
            wait = 0;
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

    newLogAdd( NEWLOG_FROM_TEST, "Control Interface Tester Started" );

    int handle = socketOpen( "localhost", socketPort, 0 );

    if ( handle >= 0 ) {
        sprintf( logbuffer, "Socket %s opened\n", socketPort );
        newLogAdd( NEWLOG_FROM_TEST, logbuffer );

        performAction( handle, jsonFileName );

        printf( "Control Interface Tester exit\n");
        socketClose( handle );
    } else {
        sprintf( logbuffer, "Error opening port to %s\n", socketPort );
        newLogAdd( NEWLOG_FROM_TEST, logbuffer );
    }

    return( 0 );
}

