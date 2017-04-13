// ------------------------------------------------------------------
// CGI program - Logging
// ------------------------------------------------------------------
// Program that generates the IoT Logging webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \brief CGI Program for the Logging webpage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// #include <dirent.h>
// #include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include "atoi.h"
#include "newLog.h"
#include "gateway.h"

// #define BUFLEN          200

#define MAXLOGROWS            40

#define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

#define COMMAND_NONE        0
#define COMMAND_GET_LOGS    1

// -------------------------------------------------------------
// Globals
// -------------------------------------------------------------

static int  startIndex = -1;

static int  postcommand = COMMAND_NONE;
static char postfilename[40];

static char logRows[MAXLOGROWS][NEWLOG_MAX_TEXT+20];   // from + Text + ts
static int  numLogs = 0;

// -------------------------------------------------------------
// Variables
// -------------------------------------------------------------

static int strpos( char * haystack, char * needle ) {
    char * p = strstr( haystack, needle );
    if ( p ) return( (int)( p-haystack ) );
    return( -1 );
}

static void parseNameValue( char * name, char * value ) {
    DEBUG_PRINTF( "Name/Value: %s/%s\n", name, value );
    if ( strcmp( name, "command" ) == 0 ) {
        postcommand = Atoi( value );
    } else if ( strcmp( name, "index" ) == 0 ) {
        startIndex = AtoiMI( value );
        if ( startIndex < 0 ) startIndex = -1;
    }
}

static void parsePostdata( char * formdata ) {
    DEBUG_PRINTF( "Parse: %s\n", formdata );
    int pos;
    if ( ( pos = strpos( formdata, "&" ) ) != -1 ) {
        formdata[pos] = '\0';
        parsePostdata( formdata );
        parsePostdata( &formdata[pos+1] );
    } else {
        // One name/value pair
        if ( ( pos = strpos( formdata, "=" ) ) != -1 ) {
            formdata[pos] = '\0';
            parseNameValue( formdata, &formdata[pos+1] );
        }
    }
}

// -------------------------------------------------------------
// Time string helper
// -------------------------------------------------------------

static char timstr[40];

static char * timeString( int ts ) {
    time_t now = (time_t)ts;
    strcpy( timstr, ctime( &now ) );
    int l = strlen(timstr);
    if ( l > 1 ) timstr[ l-1 ] = '\0';
    return( timstr );
}

// -------------------------------------------------------------
// Looper call-back
// -------------------------------------------------------------

/**
 * \brief Looper callback that prints a log line
 * \returns 1 to continue looping
 */
static int logCb( int i, onelog_t * l ) {
    if ( numLogs < MAXLOGROWS ) {
        printf( "%d|%d|%d|%s", i, l->from, l->ts, l->text );
        sprintf( logRows[numLogs++], "%d|%d|%d|%s", i, l->from, l->ts, l->text );
    }
    
    return( numLogs < MAXLOGROWS );
}   

// -------------------------------------------------------------
// Main
// -------------------------------------------------------------

/**
 * \brief CGI's main entry point: Generates the basic page layout and then waits for Ajax requests.
 * \param argc Number of command-line parameters
 * \param argv Syntax: <progname> [ <command> <startindex> ]
 */
int main( int argc, char * argv[] ) {
    int i, err = 0;
    
    if ( argc > 1 ) {
        // Called from command line
        while ( i < argc ) {
            printf( "Argument %d = %s\n", i, argv[i] );
            if ( i==1 ) postcommand = atoi( argv[i] );
            else if ( i==2 ) startIndex = atoi( argv[i] );
            i++;
        }
        printf( "command = %d, startIndex = %d\n", postcommand, startIndex );
    }

    int clk = (int)time( NULL );

    // char * formdata = (char *)getenv( "QUERY_STRING" );

    char postdata[200];
    char postdata_cpy[200];
    char * lenstr = (char *)getenv( "CONTENT_LENGTH" );
    if ( lenstr != NULL ) {
        int len = Atoi( lenstr );
        fgets( postdata, len+1, stdin );
        strcpy( postdata_cpy, postdata );
        postfilename[0] = '\0';
        parsePostdata( postdata );
    }

    switch ( postcommand ) {

    case COMMAND_NONE:
        printf( "Content-type: text/html\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<html xmlns='http://www.w3.org/1999/xhtml'>\n" );
        printf( "<head>\n" );
        printf( "<link href='/css/iot.css' rel='stylesheet' type='text/css'/>\n" );
        printf( "<script src='/js/iot.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/logs.js'></script>\n" );
        printf( "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n" );
        printf( "<title>IoT Logs - %s</title>\n", myIP() );
        printf( "</head>\n" );
        printf( "<body onload='iotOnLoad();'>\n" );

        // Mode switch
        printf( "<div id='modeswitch' style='display:none' ></div>\n" );

        printf( "<h1 style='display:inline'>IoT Logs - " );
        printf( "<div style='display: inline' id='headerDate'>%s</div>", timeString( clk ) );
        printf( "</h1>\n" );

        printf( "<div id='javaerror' style='color: red; font-size: 1.4em'>Javascript error?</div>\n" );

        // Graph
        printf( "<div id='graph' style='display:none'>\n" );
        printf( "<fieldset class='cbi-section' background-color='#FFFFFF';>\n" );
        printf( "<legend id='fieldlegend'>System</legend>\n" );
        printf( "<canvas id='systemCanvas' width='620' height='560' style='border:1px solid #000000;'>\n" );
        printf( "Your browser does not support HTML5 canvas\n" );
        printf( "</canvas>\n" );
        printf( "</div>\n" );    // graph

        // Logs lists
        printf( "<div id='logs'>Please wait ...</div>\n" );

        printf( "<div id='debug'></div>\n" );

        printf( "</body>\n" );
        printf( "</html>\n" );
        break;

    case COMMAND_GET_LOGS:

        printf( "Content-type: text/xml\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<logs>\n" );
        
        numLogs = 0;
        startIndex = newLogLoop( NEWLOG_FROM_NONE, startIndex, logCb );
        
        err = ( startIndex < 0 ) ? 1 : 0;
        
        printf( "    <err>%d</err>\n", err );
        printf( "    <cmd>%d</cmd>\n", postcommand );
        printf( "    <clk>%d</clk>\n", clk );
        printf( "    <pid>%d</pid>\n", getpid() );
        
        if ( !err ) {
            printf( "    <index>%d</index>\n", startIndex );
            if ( numLogs > 0 ) {
                printf( "    <rows>" );
                int first = 1;
                for ( i=0; i<numLogs; i++ ) {
                    if ( first ) first = 0;
                    else printf( ";" );
                    printf( "%s", logRows[i] );
                }
                printf( "</rows>\n" );
            }
        }
        printf( "</logs>\n" );
        break;

    default:

        // Send the log data
        printf( "Content-type: text/xml\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<logs>\n" );
        printf( "    <err>123</err>\n" );
        printf( "</logs>\n" );
        break;
    }

    return( 0 );
}
