// ------------------------------------------------------------------
// CGI program - System
// ------------------------------------------------------------------
// Program that generates the IoT System webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \brief CGI Program for the System webpage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>

#include "atoi.h"
#include "socket.h"
#include "jsonCreate.h"
#include "newLog.h"
#include "gateway.h"

#define INPUTBUFFERLEN               500

#define COMMAND_NONE                 0
#define COMMAND_TSCHECK              1
#define COMMAND_SYNC_TIME            2
#define COMMAND_REBOOT               3
#define COMMAND_SET_HOSTNAME         4
#define COMMAND_SET_WIRELESS_SSID    5
#define COMMAND_SET_WIRELESS_CHANNEL 6
#define COMMAND_SET_TIMEZONE         7
#define COMMAND_SET_ZONENAME         8
#define COMMAND_NETWORK_RESTART      9
#define COMMAND_SHUTDOWN             10

// #define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

#define CONTROL_PORT       "2001"

// #define QUEUESDIR          "/tmp"

#define MAXBUFFER    1000

// -------------------------------------------------------------
// Globals
// -------------------------------------------------------------

static int command     = 0;
static char strval[40] = { 0 };

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
        return( 888 );
    }
    return( 123 );
}

// -------------------------------------------------------------
// XML response
// -------------------------------------------------------------

static void xmlOpen( void ) {
    printf( "Content-type: text/xml\r\n\r\n" );
    printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
    printf( "<system>\n" );
    printf( "    <cmd>%d</cmd>\n", command );
}

static void xmlError( int err ) {
    printf( "    <err>%d</err>\n", err );
}

#if 0
static void xmlDebug( int n, char * dbg ) {
    printf( "    <dbg%d>%s</dbg%d>\n", n, dbg, n );
}

static void xmlDebugInt( int n, int dbg ) {
    printf( "    <dbg%d>%d</dbg%d>\n", n, dbg, n );
}
#endif

static void xmlClose( void ) {
    printf( "</system>\n" );
}

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
        command = Atoi( value );
    } else if ( strcmp( name, "hostname" ) == 0 ) {
        strcpy( strval, value );
    } else if ( strcmp( name, "ssid" ) == 0 ) {
        strcpy( strval, value );
    } else if ( strcmp( name, "channel" ) == 0 ) {
        strcpy( strval, value );
    } else if ( strcmp( name, "timezone" ) == 0 ) {
        strcpy( strval, value );
    } else if ( strcmp( name, "zonename" ) == 0 ) {
        strcpy( strval, value );
    } else if ( strcmp( name, "browsertime" ) == 0 ) {
        strcpy( strval, value );
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
// Button helper
// -------------------------------------------------------------

static void printButton( char * value, int cmd ) {
    printf( "<input type='submit' value='%s' onclick='SysCommand(%d);' />\n", value, cmd );
}

// -------------------------------------------------------------
// Main
// -------------------------------------------------------------

int main( int argc, char * argv[] ) {
    int    i;
    char buf[MAXBUFFER];
    int downtime;
    
    if ( argc > 1 ) {
        printf( "Help: %s <command> [ <hostname> | <ssid> | <channel> | <timezone> | <browsertime> ]\n", argv[0] );
        // Called from command line
        while ( i < argc ) {
            printf( "Argument %d = %s\n", i, argv[i] );
            if      ( i==1 ) command = atoi( argv[i] );
            else if ( i==2 ) strcpy( strval, argv[i] );
            i++;
        }
        printf( "command = %d\n", command );
    }

    struct timeval newtime;

    int clk = (int)time( NULL );

    // char * formdata = (char *)getenv( "QUERY_STRING" );

    char postdata[200];
    char postdata_cpy[200];
    char * lenstr = (char *)getenv( "CONTENT_LENGTH" );
    if ( lenstr != NULL ) {
        int len = Atoi( lenstr );
        fgets( postdata, len+1, stdin );
        strcpy( postdata_cpy, postdata );
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
        // printf( "<script src='/js/jquery.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/tz.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/system.js' charset='utf-8'></script>\n" );
        
        printf( "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n" );
        printf( "<title>IoT System - %s</title>\n", myIP() );
        printf( "</head>\n" );
        printf( "<body onload='iotOnLoad();'>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "CGI System started" );

        printf( "<h1>IoT System - " );
        printf( "<div style='display: inline' id='headerDate'>...</div>" );
        printf( "</h1>\n" );

        printf( "<div id='javaerror' style='color: red; font-size: 1.4em'>Javascript error?</div>\n" );

        printf( "<div id='div_normal'>" );
        
        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>System Information</legend>\n" );
        printf( "<div id='div_system'>Please wait...</div>" );
        printf( "</fieldset>\n" );
        
        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>Edits</legend>\n" );
        printf( "<div id='div_edits'>Please wait...</div>" );
        printf( "</fieldset>\n" );

        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>Time</legend>\n" );
        printButton( "Sync with browser", COMMAND_SYNC_TIME );
        printf( "<div id='div_browsertime' style='display:inline'></div>" );
        printf( "</fieldset>\n" );

        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>Control</legend>\n" );
#ifdef TARGET_RASPBERRYPI
        printButton( "Network restart", COMMAND_NETWORK_RESTART );
#endif
        printButton( "Reboot", COMMAND_REBOOT );
        printButton( "Shutdown", COMMAND_SHUTDOWN );
        printf( "</fieldset>\n" );

        printf( "</div>" );
        
        printf( "<div id='div_down' style='display: inline'>" );
        printf( "</div>" );
        
        printf( "<div id='debug'></div>" );

        printf( "</body>\n" );
        printf( "</html>\n" );

        break;

    case COMMAND_TSCHECK:
        xmlOpen();
        char * sys = gwSerializeSystem( MAXBUFFER, buf );
        xmlError( ( sys == NULL ) );
        printf( "    <clk>%d</clk>\n", clk );
        printf( "    <pid>%d</pid>\n", getpid() );
        printf( "    <sys>%s</sys>\n", ( sys ) ? sys : "-" );        
        xmlClose();
        break;

    case COMMAND_SYNC_TIME:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Time synced with browser" );
        newtime.tv_sec = (time_t)Atoi0( strval );
        settimeofday( &newtime, NULL );
        xmlError( 0 );
        xmlClose();
        break;

    case COMMAND_SET_HOSTNAME:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Set new hostname" );
        xmlError( writeControl( jsonCmdSystem( CMD_SYSTEM_SET_HOSTNAME, strval ) ) );
        xmlClose();
        break;

    case COMMAND_SET_WIRELESS_SSID:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Set new wifi ssid" );
        xmlError( writeControl( jsonCmdSystem( CMD_SYSTEM_SET_SSID, strval ) ) );
        xmlClose();
        break;

    case COMMAND_SET_WIRELESS_CHANNEL:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Set new wifi channel" );
        xmlError( writeControl( jsonCmdSystem( CMD_SYSTEM_SET_CHANNEL, strval ) ) );
        xmlClose();
        break;

    case COMMAND_SET_TIMEZONE:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Set new timezone" );
        xmlError( writeControl( jsonCmdSystem( CMD_SYSTEM_SET_TIMEZONE, strval ) ) );
        xmlClose();
        break;

    case COMMAND_SET_ZONENAME:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Set new zonename" );
        xmlError( writeControl( jsonCmdSystem( CMD_SYSTEM_SET_ZONENAME, strval ) ) );
        xmlClose();
        break;

    case COMMAND_NETWORK_RESTART:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Network restart" );
        xmlError( writeControl( jsonCmdSystem( CMD_SYSTEM_NETWORK_RESTART, NULL ) ) );
        xmlClose();
        break;

    case COMMAND_REBOOT:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Reboot" );
        xmlError( writeControl( jsonCmdSystem( CMD_SYSTEM_REBOOT, NULL ) ) );
#ifdef TARGET_RASPBIAN
        downtime = 35;
#else
        downtime = 25;
#endif
        printf( "<downtime>%d</downtime>", downtime );
        xmlClose();
        break;

    case COMMAND_SHUTDOWN:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Shutting down..." );
        xmlError( writeControl( jsonCmdSystem( CMD_SYSTEM_SHUTDOWN, NULL ) ) );
#ifdef TARGET_RASPBIAN
        downtime = 20;
#else
        downtime = 10;
#endif
        printf( "<downtime>%d</downtime>", downtime );
        xmlClose();
        break;

    default:
        xmlOpen();
        xmlError( 100 );
        xmlClose();
        break;
    }

    return( 0 );
}
