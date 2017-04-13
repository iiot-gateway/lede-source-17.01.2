// ------------------------------------------------------------------
// Out Tester
// ------------------------------------------------------------------
// Tests the ZCB by putting messages in its input queue
// This can be free text or (JSON) files as parameter
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \section zcbtest ZCB Testing
 * \brief Tests the ZCB by putting messages in its input queue. 
 * This can be free text or (JSON) files as parameter
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <mqueue.h>

#include "newLog.h"
#include "queue.h"

#define MAXFILENAME      50
#define INPUTBUFFERLEN   200

// -------------------------------------------------------------
// Globals
// -------------------------------------------------------------

char jsonFileName[MAXFILENAME + 2];
static char inputBuffer[INPUTBUFFERLEN];

// -------------------------------------------------------------
// Main
// -------------------------------------------------------------

/**
 * \brief Control Interface Test's main entry point: opens the socket to the
 * Control Interface and either sends a JSON file (parameter) or a string
 * message that is typed in. "q" stops the reading in interactive mode.
 * \param argc Number of command-line parameters
 * \param argv Parameter list (-h = help, -j <JSON file> = file to send)
 */
int main( int argc, char * argv[] ) {
    signed char opt;

    printf( "\n\nOut testing\n");
    jsonFileName[0] = '\0';

    while ( ( opt = getopt( argc, argv, "hj:" ) ) != -1 ) {
        switch ( opt ) {
        case 'h':
            printf( "Usage: ot [-j testfile.json]\n" );
            exit( 0 );
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

    newLogAdd( NEWLOG_FROM_TEST, "Out testing started" );

    int zcbQueue = queueOpen( QUEUE_KEY_ZCB_IN, 1 );
    if ( zcbQueue != -1 ) {

        if ( strlen( jsonFileName ) > 0 ) {

            printf( "UART writing input file %s\n", jsonFileName );

            FILE * fp;
            if ( ( fp = fopen( jsonFileName, "r" ) ) == NULL ) {
                printf( "Error opening file %s (%d - %s)\n",
                     jsonFileName, errno, strerror( errno ) );
            } else {
                // Read characters and send them as blocks to the ZCB queue
                // A block is terminated with a newline character

                while ( fgets( inputBuffer, INPUTBUFFERLEN, fp ) != NULL ) {
                    printf( "Line: %s", inputBuffer );

                    queueWrite( zcbQueue, inputBuffer );
                }
                fclose( fp );
            }

        } else {
            printf( "Read from STDIN and write to queue endlessly...\n" );
    
            while ( gets( inputBuffer ) != NULL ) {
                if ( strcmp( inputBuffer, "q" ) == 0  ) {
                    break;
                } else {
                    strcat( inputBuffer, "\n" );
                    printf( "Line: %s", inputBuffer );

                    queueWrite( zcbQueue, inputBuffer );
                }
            }
        }

        printf( "Ready, close down ZCB queue\n" );
        queueClose( zcbQueue );
    }

    return( 0 );
}
