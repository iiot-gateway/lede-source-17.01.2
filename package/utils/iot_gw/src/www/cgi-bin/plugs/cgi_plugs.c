// ------------------------------------------------------------------
// CGI program - Plug Meters
// ------------------------------------------------------------------
// Program that generates the IoT Plug Meters webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \brief CGI Program for the Plug Meters webpage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "atoi.h"
#include "iotError.h"
#include "newDb.h"
#include "socket.h"
#include "jsonCreate.h"
#include "newLog.h"
#include "gateway.h"
#include "plugUsage.h"

#define INPUTBUFFERLEN  500

// #define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

#define COMMAND_NONE            0
#define COMMAND_TSCHECK         1
#define COMMAND_GET_ROOMS       2
#define COMMAND_GET_LIST        3
#define COMMAND_GET_PLUG        4
#define COMMAND_CTRL            5

#define CONTROL_PORT    "2001"

#define MAXBUF   2000

// -------------------------------------------------------------
// Globals
// -------------------------------------------------------------

static int command = COMMAND_NONE;
static char mac[40];
static char cmd[8];
static char buf[MAXBUF];

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
    } else if ( strcmp( name, "mac" ) == 0 ) {
        strcpy( mac, value );
    } else if ( strcmp( name, "cmd" ) == 0 ) {
        strcpy( cmd, value );
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
// XML response
// -------------------------------------------------------------

static void xmlOpen( void ) {
    printf( "Content-type: text/xml\r\n\r\n" );
    printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
    printf( "<plugs>\n" );
    printf( "    <cmd>%d</cmd>\n", command );
}

static void xmlError( int err ) {
    printf( "    <err>%d</err>\n", err );
}

static void xmlClose( void ) {
    printf( "</plugs>\n" );
}

// -------------------------------------------------------------
// Main
// -------------------------------------------------------------

/**
 * \brief CGI's main entry point: Generates the basic page layout and then waits for Ajax requests.
 */
int main( int argc, char * argv[] ) {
    int err = 0, lastupdate;
    char * list;

    int clk = (int)time( NULL );

    char * formdata = (char *)getenv( "QUERY_STRING" );
    char formdata_cpy[200];
    formdata_cpy[0] = '\0';
    if ( formdata != NULL && strlen( formdata ) > 0 ) {
        strcpy( formdata_cpy, formdata );
        parsePostdata( formdata );
    }

    char postdata[200];
    char postdata_cpy[200];
    postdata_cpy[0] = '\0';
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

        printf( "<link href='/css/c3.css'  rel='stylesheet' type='text/css'/>\n" );
        printf( "<link href='/css/iot.css' rel='stylesheet' type='text/css'/>\n" );

        printf( "<script src='/js/iot.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/jquery.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/d3.min.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/c3.min.js'></script>\n" );
        printf( "<script src='/js/iotDbSpec.js'></script>\n" );
        printf( "<script src='/js/plugs.js'></script>\n" );

        printf( "<meta http-equiv='X-UA-Compatible' content='chrome=1, IE=edge' />\n" );
        printf( "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n" );
        printf( "<title>IoT Plug Meters - %s</title>\n", myIP() );
        printf( "</head>\n" );
        printf( "<body>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "CGI Plug Meters started" );

        printf( "<h1>IoT Plug Meters - " );
        printf( "<div style='display: inline' id='headerDate'>%s</div>", timeString( clk ) );
        printf( "</h1>\n" );

        // printf( "Mode = %d\n", mode );
        // printf( "Formdata = %s\n", formdata_cpy );
        // printf( "Postdata = %s\n", postdata_cpy );

        printf( "<div id='javaerror' style='color: red; font-size: 1.4em'>Javascript error?</div>\n" );

        printf( "<fieldset class='cbi-section' background-color='#FFFFFF';>\n" );
        printf( "<legend id='fieldlegend'>Plug Meters</legend>\n" );

        // INFO
        printf( "<div id='info'>\n" );
        printf( "    <div id='hourchart'></div>\n" );
        printf( "    <div id='daychart'></div>\n" );
        printf( "    <div id='actsum'>Please wait...</div>\n" );
        printf( "    <div id='onoff' onclick='togglePlug()'></div>\n" );
        printf( "</div>\n" );

        // List of plugs
        printf( "<div id='pluglist'>Please wait ...</div>\n" );

        printf( "</fieldset>\n" );

        printf( "<img id='backarrow' src='/img/back.png' style='display:none' onclick='goBack(%d)' width='80' />\n", ( strlen( mac ) > 0 ) ? 1 : 0 );

        printf( "<div id='debug'></div>\n" );

        printf( "<script>\n" );
        // strcpy( mac, "00158D000036054E" );
        printf( "console.log( 'form = '+'%s');\n", formdata_cpy );
        printf( "console.log( 'post = '+'%s');\n", postdata_cpy );
        printf( "console.log( 'HAHAHA '+'%s');\n", mac );
        printf( "onLoad('%s');\n", mac );
        printf( "</script>\n" );

        printf( "</body>\n" );
        printf( "</html>\n" );
        break;

    case COMMAND_TSCHECK:
        xmlOpen();
        newDbOpen();
        lastupdate = newDbGetLastupdatePlug();
        newDbClose();
        xmlError( 0 );
        printf( "    <clk>%d</clk>\n", clk );
        printf( "    <pid>%d</pid>\n", getpid() );
        printf( "    <ts>%d</ts>\n", lastupdate );
        xmlClose();
        break;

    case COMMAND_GET_LIST:
        xmlOpen();
        newDbOpen();
        list = newDbSerializePlugs( MAXBUF, buf );
        newDbClose();
        xmlError( list == NULL );
        printf( "    <clk>%d</clk>\n", clk );
        printf( "    <list>%s</list>\n", ( list ) ? list : "-" );
        xmlClose();
        break;

    case COMMAND_GET_PLUG:
        xmlOpen();
        
        newDbOpen();
        
        newdb_dev_t device;
        if ( newDbGetDevice( mac, &device ) ) {

            int hourHistory[60];
            int dayHistory[24];

            plugGetHistory( mac, (int)clk, 1,  60, hourHistory );
            plugGetHistory( mac, (int)clk, 60, 24, dayHistory );

            printf( "<time>%s</time>",     timeString( clk ) );
            printf( "<mac>%s</mac>\n",     mac );
            printf( "<onoff>%d</onoff>\n", ( strcmp( device.cmd, "on" ) == 0 ) );
            printf( "<act>%d</act>\n",     device.act );
            printf( "<sum>%d</sum>\n",     device.sum );
            printf( "<ts>%s</ts>\n",       timeString( device.lastupdate ) );

            printf( "<hourhistory>\n" );
            int i, first=1;
            for ( i=0; i<60; i++ ) {
                if ( first ) {
                    first = 0;
                } else {
                    printf( "," );
                }
                printf( "%d", hourHistory[i] );
            }
            printf( "</hourhistory>\n" );

            printf( "<dayhistory>\n" );
            first=1;
            for ( i=0; i<24; i++ ) {
                if ( first ) {
                    first = 0;
                } else {
                    printf( "," );
                }
                printf( "%d", dayHistory[i] );
            }
            printf( "</dayhistory>\n" );
        } else {
            err = 1;
        }
        
        newDbClose();
        
        xmlError( err );
        xmlClose();
        break;

    case COMMAND_CTRL:
        xmlOpen();
        printf( "Content-type: text/xml\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<plug>\n" );
        if ( strlen( mac ) > 0 ) {
            int handle = socketOpen( "localhost", CONTROL_PORT, 0 );

            if ( handle >= 0 ) {
                socketWriteString( handle,
                       jsonControl( mac, cmd, -1, -1, -1, -1, -1, -1 ) );
                socketClose( handle );
            } else {
                err = 5;
            }
        } else {
            err = 1;
        }
        xmlError( err );
        xmlClose();
        break;

    default:
        xmlOpen();
        xmlError( 123 );
        xmlClose();
        break;
    }

    return( 0 );
}
