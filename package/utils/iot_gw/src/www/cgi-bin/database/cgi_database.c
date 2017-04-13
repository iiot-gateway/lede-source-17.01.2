// ------------------------------------------------------------------
// CGI program - Database
// ------------------------------------------------------------------
// Program that generates the IoT Database webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \brief CGI Program for the Database webpage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "socket.h"
#include "jsonCreate.h"
#include "atoi.h"
#include "newDb.h"
#include "newLog.h"
#include "gateway.h"

#define INPUTBUFFERLEN  500

#define COMMAND_NONE                0
#define COMMAND_TSCHECK             1
#define COMMAND_RESERVED            2
#define COMMAND_RESERVED_2          3
#define COMMAND_CLEAR_LAMPS         4
#define COMMAND_CLEAR_PLUGS         5
#define COMMAND_CLEAR_DEVICES       6
#define COMMAND_CLEAR_SYSTEM        7
#define COMMAND_CLEAR_ZCB           8
#define COMMAND_CLEAR_PLUGHISTORY   9
#define COMMAND_GET_TABLE           10

// #define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

#define CONTROL_PORT       "2001"

#define MAXBUF    10000

// -------------------------------------------------------------
// Globals
// -------------------------------------------------------------

#define NUM_DBTABLES   4


static char * dbTables[NUM_DBTABLES] = {
    "iot_devs",              // 0
    "iot_system",            // 1
    "iot_zcb",               // 2
    "iot_plughistory" };     // 3 = special ! MUST BE THE LAST ONE

#define DEVICESTABLE         0
#define SYSTEMTABLE          1
#define ZCBTABLE             2
#define PLUGHISTORYTABLE     3

static char buf[MAXBUF];

// -------------------------------------------------------------
// Variables
// -------------------------------------------------------------

static int command  = 0;
static int tableno  = 0;

// -------------------------------------------------------------
// Post Variables
// -------------------------------------------------------------

static int strpos( char * haystack, char * needle ) {
    char * p = strstr( haystack, needle );
    if ( p ) return( (int)( p-haystack ) );
    return( -1 );
}

static void parseNameValue( char * name, char * value ) {
    DEBUG_PRINTF( "Name/Value: %s/%s\n", name, value );
    if ( strcmp( name, "command" ) == 0 ) {
        command = Atoi( value );
    } else if ( strcmp( name, "tableno" ) == 0 ) {
        tableno = Atoi( value );
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
// Print Table (reservation)
// -------------------------------------------------------------

static void printTable( int num ) {
    printf( "<h2>%s</h2>\n", dbTables[num] );
    printf( "<div id='div_%s'>Please wait ...</div>\n", dbTables[num] );
}

// -------------------------------------------------------------
// XML response
// -------------------------------------------------------------

static void xmlOpen( void ) {
    printf( "Content-type: text/xml\r\n\r\n" );
    printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
    printf( "<database>\n" );
    printf( "    <cmd>%d</cmd>\n", command );
}

static void xmlError( int err ) {
    printf( "    <err>%d</err>\n", err );
}

#if 0
static void xmlDebug( int n, char * dbg ) {
    printf( "    <dbg%d>%s</dbg%d>\n", n, dbg, n );
}
#endif

static void xmlDebugInt( int n, int dbg ) {
    printf( "    <dbg%d>%d</dbg%d>\n", n, dbg, n );
}

static void xmlClose( void ) {
    printf( "</database>\n" );
}

// -------------------------------------------------------------
// Button helper
// -------------------------------------------------------------

static void printButton( char * value, int cmd ) {
    printf( "<input type='submit' value='%s' onclick='DbCommand(%d);' />\n",
            value, cmd );
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
// Write control
// -------------------------------------------------------------

static int writeControl( char * cmd ) {

    int handle = socketOpen( "localhost", CONTROL_PORT, 0 );

    if ( handle >= 0 ) {

        socketWriteString( handle, cmd );

        socketClose( handle );

        return( 0 );
    } else {
        newLogAdd( NEWLOG_FROM_CGI, "Control socket problem" );
        return( 999 );
    }
    return( 456 );
}

// -------------------------------------------------------------
// Main
// -------------------------------------------------------------

/**
 * \brief CGI's main entry point: Generates the basic page layout and then waits for Ajax requests.
 * \param argc Number of command-line parameters
 * \param argv Syntax: <progname> [ <command> <tableno> ]
 */
int main( int argc, char * argv[] ) {
    int i = 0, err = 0;
    char * table = NULL; 
    
    if ( argc > 1 ) {
        // Called from command line
        while ( i < argc ) {
            printf( "Argument %d = %s\n", i, argv[i] );
            if ( i==1 ) command = atoi( argv[i] );
            else if ( i==2 ) tableno = atoi( argv[i] );
            i++;
        }
        printf( "command = %d, tableno = %d\n", command, tableno );
    }

    int clk = time( NULL );

    // char * formdata = (char *)getenv( "QUERY_STRING" );

    char postdata[200];
    // char postdata_cpy[200];
    char * lenstr = (char *)getenv( "CONTENT_LENGTH" );
    if ( lenstr != NULL ) {
        int len = Atoi( lenstr );
        fgets( postdata, len+1, stdin );
        // strcpy( postdata_cpy, postdata );
        parsePostdata( postdata );
    }

    switch ( command ) {

    case COMMAND_NONE:
        printf( "Content-type: text/html\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<html xmlns='http://www.w3.org/1999/xhtml'>\n" );
        printf( "<head>\n" );

        printf( "<link href='/css/iot.css' rel='stylesheet' type='text/css'/>\n" );
        
        printf( "<script src='/js/iot.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/jquery.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/iotDbSpec.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/database.js' charset='utf-8'></script>\n" );

        printf( "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n" );
        printf( "<title>IoT Database - %s</title>\n", myIP() );
        printf( "</head>\n" );
        printf( "<body>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "CGI Database started" );

        printf( "<h1>IoT Database - " );
        printf( "<div style='display: inline' id='headerDate'>%s</div>", timeString( clk ) );
        printf( "</h1>\n" );

        // printf( "Command = %d\n",  command );
        // printf( "Formdata = %s\n", formdata );
        // printf( "Postdata = %s\n", postdata_cpy );

        printf( "<fieldset class='cbi-section'><legend>Device Tables overview</legend>\n" );
            printButton( "Clear Lamps",       COMMAND_CLEAR_LAMPS );
            printButton( "Clear Plugs",       COMMAND_CLEAR_PLUGS );
            printButton( "Clear Plughistory", COMMAND_CLEAR_PLUGHISTORY );
            printButton( "Clear",             COMMAND_CLEAR_DEVICES );

            // All dbTables except system and zcb
            for ( i=0; i<NUM_DBTABLES; i++ ) {
               if ( ( i != SYSTEMTABLE ) && ( i != ZCBTABLE ) )
                   printTable( i );
            }

        printf( "</fieldset>\n" );
        printf( "</br>\n" );

        printf( "<fieldset class='cbi-section'><legend>System Table</legend>\n" );
            printButton( "Clear", COMMAND_CLEAR_SYSTEM );

            // System table
            printTable( SYSTEMTABLE );

        printf( "</fieldset>\n" );
        printf( "</br>\n" );

        printf( "<fieldset class='cbi-section'><legend>ZCB</legend>\n" );
            printButton( "Clear ZCB",  COMMAND_CLEAR_ZCB );

            // ZCB table
            printTable( ZCBTABLE );

        printf( "</fieldset>\n" );

        printf( "<div id='debug'></div>" );

        printf( "</body>\n" );
        printf( "</html>\n" );
        break;

    case COMMAND_CLEAR_LAMPS:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Clear lamps" );
        xmlError( writeControl( jsonDbEditClr( "lamps" ) ) );
        xmlClose();
        break;

    case COMMAND_CLEAR_PLUGS:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Clear plugs" );
        xmlError( writeControl( jsonDbEditClr( "plugs" ) ) );
        xmlClose();
        break;

    case COMMAND_CLEAR_DEVICES:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Clear devices" );
        xmlError( writeControl( jsonDbEditClr( "devices" ) ) );
        xmlClose();
        break;

    case COMMAND_CLEAR_SYSTEM:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Clear system" );
        xmlError( writeControl( jsonDbEditClr( "system" ) ) );
        xmlClose();
        break;

    case COMMAND_CLEAR_ZCB:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Clear zcb" );
        xmlError( writeControl( jsonDbEditClr( "zcb" ) ) );
        xmlClose();
        break;

    case COMMAND_CLEAR_PLUGHISTORY:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Clear plughistory" );
        xmlError( writeControl( jsonDbEditClr( "plughistory" ) ) );
        xmlClose();
        break;

    case COMMAND_TSCHECK:
        xmlOpen();
        buf[0] = '\0';
        newDbOpen();
        int lupDevs, lupSys, lupZcb, lupPlug;
        lupDevs  = newDbGetLastupdateDevices();
        lupSys   = newDbGetLastupdateSystem();
        lupZcb   = newDbGetLastupdateZcb();
        lupPlug  = newDbGetLastupdatePlugHist();
        newDbClose();
        sprintf( buf, "%d,%d,%d,%d",
                lupDevs, lupSys, lupZcb, lupPlug );

        xmlError( err );
        printf( "    <clk>%d</clk>\n", clk );
        printf( "    <pid>%d</pid>\n", getpid() );
        printf( "    <lus>%s</lus>\n", buf );
        xmlClose();
        break;

    case COMMAND_GET_TABLE:
        xmlOpen();
        buf[0] = '\0';
        newDbOpen();
        switch ( tableno ) {   
            case DEVICESTABLE:
                table = newDbSerializeDevs( MAXBUF, buf );
                break;
            case SYSTEMTABLE:
                table = newDbSerializeSystem( MAXBUF, buf );
                break;
            case ZCBTABLE:
                table = newDbSerializeZcb( MAXBUF, buf );
                break;
            case PLUGHISTORYTABLE:
                table = newDbSerializePlugHist( MAXBUF, buf );
                if ( !table ) {
                    xmlDebugInt( 1, newDbGetNumOfPlughist() );
                    table = "+";
                }
                break;
            default:
                err = 1;
                break;
        }
        newDbClose();

        xmlError( err );
        printf( "    <tableno>%d</tableno>\n", tableno );
        printf( "    <table>%s</table>\n", ( table ) ? table : "-" );
        xmlClose();
        break;

    default:
        xmlOpen();
        xmlError( -13 );
        xmlClose();
        break;

    }

    return( 0 );
}
