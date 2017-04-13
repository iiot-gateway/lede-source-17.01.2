// ------------------------------------------------------------------
// Log Tail
// ------------------------------------------------------------------
// Tails the log module
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \section logtest Log Tailing
 * \brief Tails the log module
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <time.h>

#include "atoi.h"
#include "newLog.h"

/**
 * \brief Looper callback that prints a log line
 * \returns 1 to continue looping
 */
static int printCb( int i, onelog_t * l ) {
    time_t ts = l->ts;
    char timestr[40];
    sprintf( timestr, "%s", ctime( &ts ) );
    timestr[ strlen( timestr ) - 1 ] = '\0';
    printf( "Log %d: %d, %s : %s\n", i, l->from, timestr, l->text );
    return 1;
}   
    
// -------------------------------------------------------------
// Main
// -------------------------------------------------------------

/**
 * \brief Log Tail's main entry point: Prints new logs and updates
 * every second. Can show all logs or only for a certain (from) program
 * \param argc Number of command-line parameters
 * \param argv Parameter list (-h = help, -f <from> = specific program to follow (all when omitted), -z = startIndex is zero)
 */
int main( int argc, char * argv[] ) {
    signed char opt;
    int i, from = NEWLOG_FROM_NONE, zero = 0;

    printf( "\n\nLog tail\n");

    while ( ( opt = getopt( argc, argv, "hf:z" ) ) != -1 ) {
        switch ( opt ) {
        case 'h':
            printf( "Usage: lt [-f <from>]\n\tWhere <from> is one of:\n" );
            for ( i=1; i<11; i++ ) {
                printf( "\t\t%2d %s\n", i, logNames[i] );
            }
            exit( 0 );
        case 'f':
            printf( "From = %s\n", optarg );
            from = Atoi0( optarg );
            break;
        case 'z':
            zero = 1;
            break;
        }
    }
     
    // Where to start
    int startIndex = ( zero ) ? 0 : newLogGetIndex();

    while ( 1 ) {
        
        startIndex = newLogLoop( from, startIndex, printCb );
        sleep( 1 );
    }
    
    return( 0 );
}
