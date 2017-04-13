// ------------------------------------------------------------------
// CGI program - Groups
// ------------------------------------------------------------------
// Program that generates the IoT Groups webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \brief CGI Program for the Groups webpage
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

#define INPUTBUFFERLEN  500

#define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

#define COMMAND_NONE              0
#define COMMAND_TSCHECK           1
#define COMMAND_GET_LAMPS         2
#define COMMAND_GROUP_MANIPULATE  3
#define COMMAND_SCENE_MANIPULATE  4
#define COMMAND_SCENE_EXECUTE     5
#define COMMAND_GROUP_ON          11
#define COMMAND_GROUP_OFF         12
#define COMMAND_GROUP_0           13
#define COMMAND_GROUP_25          14
#define COMMAND_GROUP_50          15
#define COMMAND_GROUP_75          16
#define COMMAND_GROUP_100         17
#define COMMAND_GROUP_RED         18
#define COMMAND_GROUP_GREEN       19
#define COMMAND_GROUP_BLUE        20
#define COMMAND_GROUP_WHITE       21

#define CONTROL_PORT    "2001"

#define WHO "lamp.cgi"

#define MAXBUF  2000

// -------------------------------------------------------------
// Globals
// -------------------------------------------------------------

static int command = COMMAND_NONE;
static char postmac[40];
static char postcmd[8];
static int  postgid    = -1;
static int  postsid    = -1;
static int  postlvl    = -1;
static int  postrgb    = -1;
static int  postkelvin = -1;
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
        strcpy( postmac, value );
    } else if ( strcmp( name, "cmd" ) == 0 ) {
        strcpy( postcmd, value );
    } else if ( strcmp( name, "gid" ) == 0 ) {
        postgid = Atoi( value );
    } else if ( strcmp( name, "sid" ) == 0 ) {
        postsid = Atoi( value );
    } else if ( strcmp( name, "lvl" ) == 0 ) {
        postlvl = Atoi( value );
    } else if ( strcmp( name, "rgb" ) == 0 ) {
        postrgb = Atoi( value );
    } else if ( strcmp( name, "kelvin" ) == 0 ) {
        postkelvin = Atoi( value );
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
    printf( "<input type='submit' value='%s' onclick='ButtonCmd(%d);' />\n", value, cmd );
}

// -------------------------------------------------------------
// XML response
// -------------------------------------------------------------

static void xmlResponse( int err ) {
    printf( "Content-type: text/xml\r\n\r\n" );
    printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
    printf( "<groups>\n" );
    printf( "    <cmd>%d</cmd>\n", command );
    printf( "    <err>%d</err>\n", err );
    printf( "</groups>\n" );
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
// Main
// -------------------------------------------------------------

int main( int argc, char * argv[] ) {
    int err = 0, handle, lastupdate;
    char * list;

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

        // printf( "<link href='/css/slider.css'  rel='stylesheet' type='text/css'/>\n" );
        printf( "<link href='/css/iot.css' rel='stylesheet' type='text/css'/>\n" );
        printf( "<script src='/js/iot.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/jquery.js'></script>\n" );
        printf( "<script src='/js/groups.js'></script>\n" );

        printf( "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n" );
        printf( "<title>IoT Groups - %s</title>\n", myIP() );
        printf( "</head>\n" );
        printf( "<body>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "CGI Groups started" );

        printf( "<h1>IoT Groups - " );
        printf( "<div style='display: inline' id='headerDate'>%s</div>", timeString( clk ) );
        printf( "</h1>\n" );

        // printf( "Mode = %d\n", mode );
        // printf( "Formdata = %s\n", formdata_cpy );
        // printf( "Postdata = %s\n", postdata_cpy );

        printf( "<div id='javaerror' style='color: red; font-size: 1.4em'>Javascript error?</div>\n" );

        printf( "<fieldset class='cbi-section' background-color='#FFFFFF';>\n" );
        printf( "<legend id='fieldlegend'>Device group manipulate</legend>\n" );
        printf( "<div id='gm_lamps' style='display:inline'>Please wait...</div>\n" );
        printf( "<div id='gm_cmd' style='display:inline'>Please wait...</div>\n" );
        printf( "<div id='gm_gid' style='display:none'>Please wait...</div>\n" );
        printButton( "Send", COMMAND_GROUP_MANIPULATE );
        printf( "</fieldset>\n" );

        printf( "<fieldset class='cbi-section' background-color='#FFFFFF';>\n" );
        printf( "<legend id='fieldlegend'>Device scene manipulate</legend>\n" );
        printf( "<div id='sm_lamps' style='display:inline'>Please wait...</div>\n" );
        printf( "<div id='sm_gid' style='display:inline'>Please wait...</div>\n" );
        printf( "<div id='sm_cmd' style='display:inline'>Please wait...</div>\n" );
        printf( "<div id='sm_sid' style='display:inline; display:none'>Please wait...</div>\n" );
        printButton( "Send", COMMAND_SCENE_MANIPULATE );
        printf( "</fieldset>\n" );

        printf( "<fieldset class='cbi-section' background-color='#FFFFFF';>\n" );
        printf( "<legend id='fieldlegend'>Group commands</legend>\n" );
        printf( "<div id='gc_gid' style='display:inline'>Please wait...</div>\n" );
        printf( "<div>\n" );
        printf( "On/Off: " );
        printButton( "On", COMMAND_GROUP_ON );
        printButton( "Off", COMMAND_GROUP_OFF );
        printf( "<br />Level: " );
        printButton( "0%", COMMAND_GROUP_0 );
        printButton( "25%", COMMAND_GROUP_25 );
        printButton( "50%", COMMAND_GROUP_50 );
        printButton( "75%", COMMAND_GROUP_75 );
        printButton( "100%", COMMAND_GROUP_100 );
        printf( "<br />Color: " );
        printButton( "Red", COMMAND_GROUP_RED );
        printButton( "Green", COMMAND_GROUP_GREEN );
        printButton( "Blue", COMMAND_GROUP_BLUE );
        printButton( "White", COMMAND_GROUP_WHITE );
        printf( "</div>\n" );
        printf( "</fieldset>\n" );

        printf( "<fieldset class='cbi-section' background-color='#FFFFFF';>\n" );
        printf( "<legend id='fieldlegend'>Group scene commands</legend>\n" );
        printf( "<div id='sc_gid' style='display:inline'>Please wait...</div>\n" );
        printf( "<div id='sc_cmd' style='display:inline'>Please wait...</div>\n" );
        printf( "<div id='sc_sid' style='display:inline; display:none'>Please wait...</div>\n" );
        printButton( "Send", COMMAND_SCENE_EXECUTE );
        printf( "</fieldset>\n" );

        printf( "<img src='/img/back.png' onclick='window.history.back()' width='80' />\n" );

        printf( "<div id='debug'></div>\n" );

        printf( "</body>\n" );
        printf( "</html>\n" );
        break;

    case COMMAND_TSCHECK:
        printf( "Content-type: text/xml\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<groups>\n" );
        newDbOpen();
        lastupdate = newDbGetLastupdateLamp();
        newDbClose();
        printf( "    <err>0</err>\n" );
        printf( "    <cmd>%d</cmd>\n", command );
        printf( "    <clk>%d</clk>\n", clk );
        printf( "    <pid>%d</pid>\n", getpid() );
        printf( "    <ts>%d</ts>\n", lastupdate );
        printf( "</groups>\n" );
        break;

    case COMMAND_GET_LAMPS:
        printf( "Content-type: text/xml\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<groups>\n" );
        newDbOpen();
        list = newDbSerializeLamps( MAXBUF, buf );
        newDbClose();
        printf( "    <err>%d</err>\n", list == NULL );
        printf( "    <cmd>%d</cmd>\n", command );
        printf( "    <list>%s</list>\n", ( list ) ? list : "-" );
        printf( "</groups>\n" );
        break;

    case COMMAND_GROUP_MANIPULATE:
        printf( "Content-type: text/xml\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<groups>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "Group manipulate" );

        // Let the ControlInterface handle it
        handle = socketOpen( "localhost", CONTROL_PORT, 0 );
        if ( handle >= 0 ) {
            socketWriteString( handle,
                jsonLampGroup( postmac, postcmd, postgid, NULL, -1 ) );
            socketClose( handle );
        } else {
            err = 1;
        }

        printf( "<err>%d</err>\n", err );
        printf( "</groups>\n" );
        break;

    case COMMAND_GROUP_ON:
    case COMMAND_GROUP_OFF:
        printf( "Content-type: text/xml\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<groups>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "Group on/off" );

        // Let the ControlInterface handle it
        handle = socketOpen( "localhost", CONTROL_PORT, 0 );
        if ( handle >= 0 ) {
            socketWriteString( handle,
                jsonGroup( postgid, NULL, -1,
                           ( command == COMMAND_GROUP_ON ) ? "on" : "off",
                           -1, -1, -1, -1, -1 ) );
            socketClose( handle );
        } else {
            err = 1;
        }

        printf( "<err>%d</err>\n", err );
        printf( "</groups>\n" );
        break;

    case COMMAND_GROUP_0:
    case COMMAND_GROUP_25:
    case COMMAND_GROUP_50:
    case COMMAND_GROUP_75:
    case COMMAND_GROUP_100:
        printf( "Content-type: text/xml\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<groups>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "Group level" );

        // Let the ControlInterface handle it
        handle = socketOpen( "localhost", CONTROL_PORT, 0 );
        if ( handle >= 0 ) {
            socketWriteString( handle,
                jsonGroup( postgid, NULL, -1,
                           NULL, postlvl, -1, -1, -1, -1 ) );
            socketClose( handle );
        } else {
            err = 1;
        }

        printf( "<err>%d</err>\n", err );
        printf( "</groups>\n" );
        break;

    case COMMAND_GROUP_RED:
    case COMMAND_GROUP_GREEN:
    case COMMAND_GROUP_BLUE:
    case COMMAND_GROUP_WHITE:
        printf( "Content-type: text/xml\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<groups>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "Group color" );

        // Let the ControlInterface handle it
        handle = socketOpen( "localhost", CONTROL_PORT, 0 );
        if ( handle >= 0 ) {
            socketWriteString( handle,
                jsonGroup( postgid, NULL, -1,
                           NULL, -1, postrgb, postkelvin, -1, -1 ) );
            socketClose( handle );
        } else {
            err = 1;
        }

        printf( "<err>%d</err>\n", err );
        printf( "</groups>\n" );
        break;

    case COMMAND_SCENE_MANIPULATE:
        printf( "Content-type: text/xml\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<groups>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "Scene manipulate" );

        // Let the ControlInterface handle it
        handle = socketOpen( "localhost", CONTROL_PORT, 0 );
        if ( handle >= 0 ) {
            socketWriteString( handle,
                jsonLampGroup( postmac, NULL, postgid, postcmd, postsid ) );
            socketClose( handle );
        } else {
            err = 1;
        }

        printf( "<err>%d</err>\n", err );
        printf( "</groups>\n" );
        break;

    case COMMAND_SCENE_EXECUTE:
        printf( "Content-type: text/xml\r\n\r\n" );
        printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
        printf( "<groups>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "Scene control" );

        // Let the ControlInterface handle it
        handle = socketOpen( "localhost", CONTROL_PORT, 0 );
        if ( handle >= 0 ) {
            socketWriteString( handle,
                jsonGroup( postgid, postcmd, postsid,
                           NULL, -1, -1, -1, -1, -1 ) );
            socketClose( handle );
        } else {
            err = 1;
        }

        printf( "<err>%d</err>\n", err );
        printf( "</groups>\n" );
        break;

    default:
        xmlResponse( 888 );
        break;
    }

    return( 0 );
}
