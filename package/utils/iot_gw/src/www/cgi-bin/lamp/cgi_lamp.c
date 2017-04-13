// ------------------------------------------------------------------
// CGI program - Lamp
// ------------------------------------------------------------------
// Program that generates the IoT Lamp webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \brief CGI Program for the Lamps webpage
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

#define COMMAND_NONE            0
#define COMMAND_TSCHECK         1
#define COMMAND_GET_ROOMS       2
#define COMMAND_GET_LIST        3
#define COMMAND_GET_LAMP        4
#define COMMAND_CTRL            5
#define COMMAND_SETLEVEL        6
#define COMMAND_SETCOLOR        7
#define COMMAND_SETKELVIN       8

#define CONTROL_PORT    "2001"

#define WHO "lamp.cgi"

#define MAXBUF  2000

// -------------------------------------------------------------
// Globals
// -------------------------------------------------------------

static int command = COMMAND_NONE;
static char mac[40];
static char cmd[8];
static int postlvl    = -1;
static int postrgb    = -1;
static int postkelvin = -1;
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
    printf( "<lamps>\n" );
    printf( "    <cmd>%d</cmd>\n", command );
}

#if 0
static void xmlDebug( int n, char * dbg ) {
    printf( "    <dbg%d>%s</dbg%d>\n", n, dbg, n );
}
#endif

static void xmlError( int err ) {
    printf( "    <err>%d</err>\n", err );
}

static void xmlClose( void ) {
    printf( "</lamps>\n" );
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

    mac[0] = '\0';

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

        printf( "<link href='/css/slider.css'  rel='stylesheet' type='text/css'/>\n" );
        printf( "<link href='/css/iot.css' rel='stylesheet' type='text/css'/>\n" );
        printf( "<script src='/js/iot.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/jquery.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/mousewheel.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/colorpicker.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/twpicker.js' charset='utf-8'></script>\n" );

        printf( "<script src='/js/slider.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/iotDbSpec.js'></script>\n" );
        printf( "<script src='/js/lamps.js'></script>\n" );

        printf( "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n" );
        printf( "<title>IoT Lamp - %s</title>\n", myIP() );
        printf( "</head>\n" );
        printf( "<body>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "CGI Lamp started" );

        printf( "<h1>IoT Lamp - " );
        printf( "<div style='display: inline' id='headerDate'>%s</div>", timeString( clk ) );
        printf( "</h1>\n" );

        // printf( "Mode = %d\n", mode );
        // printf( "Formdata = %s\n", formdata_cpy );
        // printf( "Postdata = %s\n", postdata_cpy );

        printf( "<div id='javaerror' style='color: red; font-size: 1.4em'>Javascript error?</div>\n" );

        printf( "<fieldset class='cbi-section' background-color='#FFFFFF';>\n" );
        printf( "<legend id='fieldlegend'>Lamps</legend>\n" );

        // For RGB lamps
        printf( "<div id='colorpick' style='display:none'>\n" );
        printf( "  <style type='text/css' media='screen'>\n" );
        // printf( "    #colorpicker_wrapper { float: left; margin: 1em; padding: 1ex; background: #808080;}\n" );
        printf( "    #colorpicker_wrapper { float: left; margin: 1em; padding: 1ex; background: #ffffff;}\n" );
        printf( "    #preview { height: 16px; margin: 1ex; text-align: center }\n" );
        printf( "    #form { margin: 1ex; color: #666; }\n" );
        printf( "  </style>\n" );
        printf( "  <div id='colorpicker_wrapper'>\n" );
        printf( "    <div id='preview'>RGB</div>\n" );
        printf( "    <div id='colorpicker'>\n" );
        printf( "        replaced content\n" );
        printf( "    </div>\n" );
        printf( "    <ul style='padding-left: 1ex;'>\n" );
        printf( "        <li>H - Use the scroll wheel to adjust the Hue.</li>\n" );
        printf( "        <li>S - Use the scroll wheel while holding shift to adjust the Saturation.</li>\n" );
        printf( "        <li>V - Use the scroll wheel over the slider to adjust the Value.</li>\n" );
        printf( "    </ul>\n" );
        printf( "    <div id='form'>\n" );
        printf( "        <label>Hexadecimal</label> <input type=='text' id='hex_input' />\n" );
        printf( "    </div>\n" );
        printf( "  </div>\n" );
        printf( "</div>\n" );

        // For TW lamps
        printf( "<div id='twpick' style='display:none'>\n" );
        printf( "  <style type='text/css' media='screen'>\n" );
        //printf( "    #twpicker_wrapper { float: left; margin: 1em; padding: 1ex; background: #808080;}\n" );
        printf( "    #twpicker_wrapper { float: left; margin: 1em; padding: 1ex; background: #ffffff;}\n" );
        printf( "    #twpreview { height: 16px; margin: 1ex; text-align: center }\n" );
//        printf( "    #form { margin: 1ex; color: #fff; }\n" );
        printf( "  </style>\n" );
        printf( "  <div id='twpicker_wrapper'>\n" );
        printf( "    <div id='twpreview'>RGB</div>\n" );
        printf( "    <div id='twpicker'>\n" );
        printf( "        replaced content\n" );
        printf( "    </div>\n" );
        printf( "  </div>\n" );
        printf( "</div>\n" );

        // For dimmable lamps
        // printf( "<div id='dimmable' style='display:none'>\n" );
        printf( "<div id='dimmable'>\n" );
        printf( "  <input type='text' id='level' name='level' value='100' />\n" );
        printf( "  <div id='dimming'></div>\n" );
        printf( "</div>\n" );

        // For all lamps
        printf( "<div id='onoff' onclick='toggleLamp()'></div>\n" );

        // List of lamps
        printf( "<div id='lamplist'>Please wait ...</div>\n" );

        // Groups
        printf( "<div id='groups' style='display:none'>\n" );
        printf( "<a href='iot_groups.cgi'>Groups and scenes</a>\n" );
        printf( "</div>\n" );

        // Extras
        printf( "<div id='extras' style='display:none'></div>\n" );

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
        lastupdate = newDbGetLastupdateLamp();
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
        list = newDbSerializeLamps( MAXBUF, buf );
        newDbClose();
        xmlError( list == NULL );
        printf( "    <list>%s</list>\n", ( list ) ? list : "-" );
        xmlClose();
        break;

    case COMMAND_GET_LAMP:
        xmlOpen();
        
        newDbOpen();
        
        newdb_dev_t device;
        if ( newDbGetDevice( mac, &device ) ) {

            int type = 1;   // on/off
            if      ( strcmp( device.ty, "dim" ) == 0 ) type = 2;
            else if ( strcmp( device.ty, "col" ) == 0 ) type = 3;
            else if ( strcmp( device.ty, "tw"  ) == 0 ) type = 4;

            printf( "<time>%s</time>",       timeString( clk ) );
            printf( "<mac>%s</mac>\n",       mac );
            printf( "<type>%d</type>\n",     type );
            printf( "<onoff>%d</onoff>\n",   ( strcmp( device.cmd, "on" ) == 0 ) );
            printf( "<lvl>%d</lvl>\n",       device.lvl );
            printf( "<rgb>%d</rgb>\n",       device.rgb );
            printf( "<kelvin>%d</kelvin>\n", device.kelvin );
            printf( "<lastkelvin>%d</lastkelvin>\n",
                    ( device.flags & FLAG_LASTKELVIN ) ? 1 : 0 );
            printf( "<ts>%s</ts>\n",         timeString( device.lastupdate ) );
        } else {
            err = 1;
        }
        
        newDbClose();

        xmlError( err );
        xmlClose();
        break;

    case COMMAND_CTRL:
        xmlOpen();
        if ( strlen( mac ) > 0 ) {
            // Let the ControlInterface do the real changes
            int handle = socketOpen( "localhost", CONTROL_PORT, 0 );
            if ( handle >= 0 ) {
                socketWriteString( handle,
                    jsonControl( mac, cmd, -1, -1, -1, -1, -1, -1 ) );
                socketClose( handle );

                sprintf( logbuffer, "Lamp control %s", cmd );
                newLogAdd( NEWLOG_FROM_CGI, logbuffer );
            } else {
                err = 4;
            }
        } else {
            err = 1;
        }
        xmlError( err );
        xmlClose();
        break;

    case COMMAND_SETLEVEL:
    case COMMAND_SETCOLOR:
    case COMMAND_SETKELVIN:
        xmlOpen();
        printf( "<postlvl>%d</postlvl>\n", postlvl );
        printf( "<postrgb>%d</postrgb>\n", postrgb );
        printf( "<postkelvin>%d</postkelvin>\n", postkelvin );
        if ( strlen( mac ) > 0 ) {
            // Let the ControlInterface do the real changes
            int handle = socketOpen( "localhost", CONTROL_PORT, 0 );
            if ( handle >= 0 ) {
                socketWriteString( handle,
                    jsonControl( mac, NULL, -1,
                                 postlvl, postrgb, postkelvin, -1, -1 ) );
                socketClose( handle );

                if ( postrgb < 0 )
                     sprintf( logbuffer, "Lamp lvl/rgb/kelvin %d/-1/%d",
                         postlvl, postkelvin );
                else sprintf( logbuffer, "Lamp lvl/rgb/kelvin %d/%06x/%d",
                         postlvl, postrgb, postkelvin );
                newLogAdd( NEWLOG_FROM_CGI, logbuffer );
            } else {
                err = 4;
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
