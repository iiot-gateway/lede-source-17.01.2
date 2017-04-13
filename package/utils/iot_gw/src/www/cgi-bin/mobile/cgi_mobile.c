// ------------------------------------------------------------------
// CGI program - Mobile System
// ------------------------------------------------------------------
// Program that generates the IoT Mobile System webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \brief CGI Program for the Mobile-Site webpage
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

#define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

#define WHO "mobile.cgi"

#define COMMAND_NONE            0
#define COMMAND_TSCHECK         1
#define COMMAND_GET_TABLE       2
#define COMMAND_GET_DEVS        3
#define COMMAND_GET_PLUG        4
#define COMMAND_CTRL            5
#define COMMAND_SETLEVEL        6
#define COMMAND_SETCOLOR        7
#define COMMAND_SETKELVIN       8
#define COMMAND_SET_NFCMODE     9
#define COMMAND_GET_SYSTEMTABLE 10
#define COMMAND_SHUTDOWN        11

#define ERROR_EMPTY_SYSTEMTABLE   1
#define ERROR_NO_QUEUE_INFO       2
#define ERROR_UNABLE_TO_WRITE_Q   3
#define ERROR_NOVERSIONINFO       4
#define ERROR_SCRIPT_DOWNLOAD     5

#define CONTROL_PORT    "2001"

#define MAXBUF    5000

// -------------------------------------------------------------
// Globals
// -------------------------------------------------------------

static int command    = 0;
static int tableno    = 0;

static int nfcmode    = 0;

static char mac[40];
static char cmd[8];
static int postlvl    = -1;
static int postrgb    = -1;
static int postkelvin = -1;

static char buf[MAXBUF];

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
    } else if ( strcmp( name, "nfcmode" ) == 0 ) {
        nfcmode = Atoi( value );
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
    printf( "<mobile>\n" );
    printf( "    <cmd>%d</cmd>\n", command );
}

static void xmlError( int err ) {
    printf( "    <err>%d</err>\n", err );
}

static void xmlResponseWQ( int err ) {
    xmlError( ( err ) ? ERROR_UNABLE_TO_WRITE_Q : 0 );
}

static void xmlClose( void ) {
    printf( "</mobile>\n" );
}

// -------------------------------------------------------------
// Main
// -------------------------------------------------------------

/**
 * \brief CGI's main entry point: Generates the basic page layout and then waits for Ajax requests.
 */
int main( int argc, char * argv[] ) {
    int err = 0;
    char * list;
    int lus, lus2, lus3;
    int downtime;
    
    int clk = time( NULL );

    mac[0] = '\0';

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

        printf( "<link href='/css/slider.css'  rel='stylesheet' type='text/css'/>\n" );
        printf( "<link href='/css/iot.css' rel='stylesheet' type='text/css'/>\n" );
        printf( "<link href='/css/mobile.css' rel='stylesheet' type='text/css'/>\n" );

        printf( "<script src='/js/iot.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/jquery.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/mousewheel.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/combined_colorpicker.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/twpicker.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/slider.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/d3.min.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/c3.min.js' charset='utf-8'> </script>\n" );
        printf( "<script src='/js/iotDbSpec.js' charset='utf-8'></script>\n" );

        printf( "<script src='/js/mobile.js' charset='utf-8'></script>\n" );

        printf( "<meta http-equiv='X-UA-Compatible' content='chrome=1, IE=edge' />\n" );
        printf( "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n" );
        printf( "<meta name='viewport' content='width=device-width, initial-scale=1.0, user-scalable=no'> \n" );

        printf( "<title>IoT Mobile %s</title>\n", myIP() );
        printf( "</head>\n" );
        printf( "<body onload='onLoad()'>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "CGI Mobile started" );

        printf( "<div id='javaerror' style='color: red; font-size: 1.4em'>Javascript error?</div>\n" );

        printf( "<div id='mobile_header'>  </div>\n" );

        // printf( "Command = %d\n",  command );
        // printf( "Formdata = %s\n", formdata );
        // printf( "Postdata = %s\n", postdata_cpy );


        printf( "<fieldset class='cbi-section' background-color='#FFFFFF';>\n" );
        printf( "<legend id='fieldlegend'>NXP</legend>\n" );

        printf( "<div id='mobile_content'> </div>\n" );

        printf( "<center> \n" );
        
        // For RGB lamps
        printf( "<div id='colorpick' style='display:none'>\n" );
        //printf( "<div id='colorpick' class='be_centered'>\n" );
        printf( "  <style type='text/css' media='screen'>\n" );
        //printf( "    #colorpicker_wrapper { float: left; margin: 1em; padding: 1ex; background: #808080;}\n" );
        // printf( "    #colorpicker_wrapper { float: none; margin: 0em; padding: 0ex; background: #dddddd;}\n" );
        printf( "    #colorpicker_wrapper { float: none; margin: 0em; padding: 0ex; background: #ffffff;}\n" );
        //printf( "    #preview { height: 16px; margin: 1ex; text-align: center }\n" );
        // printf( "    #form { margin: 1ex; color: #fff; }\n" );
        printf( "  </style>\n" );
        printf( "  <div id='colorpicker_wrapper'>\n" );
        //printf( "    <div id='preview'>RGB</div>\n" );
        printf( "    <div id='colorpicker'>\n" );
        printf( "        replaced content\n" );
        printf( "    </div>\n" );
        printf( "  </div>\n" );
        printf( "</div>\n" );

        // For TW lamps
        printf( "<div id='twpick' style='display:none'>\n" );
        printf( "  <style type='text/css' media='screen'>\n" );
        // printf( "    #twpicker_wrapper { float: none; margin: 0em; padding: 0ex; background: #dddddd;}\n" );
        printf( "    #twpicker_wrapper { float: none; margin: 0em; padding: 0ex; background: #ffffff;}\n" );
        printf( "    #twpreview { height: 16px; margin: 1ex; text-align: center }\n" );
        // printf( "    #form { margin: 1ex; color: #fff; }\n" );
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
        printf( "  <div id='dimming'>slider</div>\n" );
        printf( "</div>\n" );

        printf( "<p> </p>\n" );

        // For all lamps
        printf( "<div id='onoff' onclick='toggleLamp()' style='display:none' </div>\n" );

        // Extras
        printf( "<div id='extras' style='display:none'></div>\n" );
        printf( "</center>\n" );

        // For plugs
        printf( "<center>\n" );
        printf( "<div id='hourchart' style='display:none'> </div>\n" );
        printf( "<div id='plugonoff' onclick='togglePlug()' style='display:none'> </div>\n" );
        printf( "</center>\n" );

        printf( "</fieldset>\n" );

        // NXP Logo as footer
        printf( "<p>\n" );
        printf( "<div id='nxp_logo'> <img src='/img/NXP_logo.gif' style='float:right'> </div>\n");
        printf( "</p> \n" );
        printf( "<div id='debug'></div>\n" );

        printf( "</body>\n" );
        printf( "</html>\n" );
        break;

    case COMMAND_TSCHECK:
        xmlOpen();
        buf[0] = '\0';
        newDbOpen();
        lus  = newDbGetLastupdatePlug();
        lus2 = newDbGetLastupdateLamp();
        lus3 = newDbGetLastupdateSystem();
        newDbClose();
        
        xmlError( 0 );
        printf( "    <clk>%d</clk>\n", clk );
        printf( "    <pid>%d</pid>\n", getpid() );
        printf( "    <lus>%d</lus>\n", lus );
        printf( "    <lus2>%d</lus2>\n", lus2);
        printf( "    <lus3>%d</lus3>\n", lus3);
        xmlClose();
        break;

    case COMMAND_GET_SYSTEMTABLE:
        xmlOpen();
        newDbOpen();
        list = newDbSerializeSystem( MAXBUF, buf );
        newDbClose();
        xmlError( list == NULL );
        printf( "    <clk>%d</clk>\n", clk );
        printf( "    <sys>%s</sys>\n", ( list ) ? list : "-" );
        xmlClose();
        break;

    case COMMAND_GET_DEVS:
        xmlOpen();
        newDbOpen();
        list = newDbSerializeLampsAndPlugs( MAXBUF, buf );
        newDbClose();
        xmlError( list == NULL );
        printf( "    <devs>%s</devs>\n", ( list ) ? list : "-" );
        xmlClose();

        break;

    case COMMAND_GET_PLUG:
        xmlOpen();
        
        newDbOpen();
        
        newdb_dev_t device;
        if ( newDbGetDevice( mac, &device ) ) {

            int hourHistory[60];
            int dayHistory[24];
            int yesterdayData;

            plugGetHistory( mac, (int)clk, 1,  9, hourHistory );
            plugGetHistory( mac, (int)clk, 60, 24, dayHistory );
            plugGetHistory( mac, (int)clk, 60 * 24, 1, &yesterdayData );

            printf( "<time>%s</time>",     timeString( clk ) );
            printf( "<mac>%s</mac>\n",     mac );
            printf( "<onoff>%d</onoff>\n", ( strcmp( device.cmd, "on" ) == 0 ) );
            printf( "<act>%d</act>\n",     device.act );
            printf( "<sum>%d</sum>\n",     device.sum );
            printf( "<ts>%s</ts>\n",       timeString( device.lastupdate ) );

            printf( "<hourhistory>\n" );
            int i, first=1;
            for ( i=0; i<9; i++ ) {
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
            
            printf( "<yesterdaydata>\n" );
            printf( "%d", yesterdayData );
            printf( "</yesterdaydata>\n" );

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

    case COMMAND_SET_NFCMODE:
        xmlOpen();
        sprintf( logbuffer, "Set NFC mode %d", nfcmode );
        newLogAdd( NEWLOG_FROM_CGI, logbuffer );
        xmlResponseWQ( writeControl( jsonCmdNfcmode( nfcmode ) ) );
        xmlClose();
        break;

    case COMMAND_SHUTDOWN:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Shutting down..." );
        xmlResponseWQ( writeControl( jsonCmdSystem( CMD_SYSTEM_SHUTDOWN, NULL ) ) );
#ifdef TARGET_RASPBIAN
        downtime = 20;
#else
        downtime = 3;
#endif
        printf( "<downtime>%d</downtime>", downtime );
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
