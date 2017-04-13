// ------------------------------------------------------------------
// CGI program - Control
// ------------------------------------------------------------------
// Program that generates the IoT Control webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \file
 * \brief CGI Program for the Control webpage
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
#include "newDb.h"
#include "systemtable.h"
#include "queue.h"
#include "socket.h"
#include "jsonCreate.h"
#include "newLog.h"
#include "gateway.h"
#include "nfcreader.h"

#define INPUTBUFFERLEN            500

#define COMMAND_NONE              0
#define COMMAND_TSCHECK           1
#define COMMAND_RESET             2
#define COMMAND_FACTORY_RESET     3
#define COMMAND_SET_CHANMASK      4
// #define COMMAND_NETWORK_START     5
#define COMMAND_VERSION_REFRESH   6
#define COMMAND_GENERATE_SECRET   7
#define COMMAND_TOPO_UPLOAD       8
//#define COMMAND_.........          9
#define COMMAND_RESTART_ALL       10
#define COMMAND_RESTART_ZB        11
// #define COMMAND_RESTART_DP        12
#define COMMAND_RESTART_SJ        13
// #define COMMAND_RESTART_CI        14
#define COMMAND_RESTART_ND        15
#define COMMAND_PLUG_START        16
#define COMMAND_PLUG_STOP         17
#define COMMAND_GET_SYSTEMTABLE   18
#define COMMAND_GET_QUEUES        19
#define COMMAND_CURRENT_VERSION   20
#define COMMAND_CHECK_UPDATES     21
#define COMMAND_INSTALL_UPDATES   22
#define COMMAND_PERMITJOIN_START  23
#define COMMAND_PERMITJOIN_STOP   24
#define COMMAND_SYNC_TIME         25
#define COMMAND_SET_NFCMODE       26

#define UPDATE_NO_CHANGES         51
#define UPDATE_CHANGES_NOINFO     52
#define UPDATE_CHANGES_WITHINFO   53

#define ERROR_EMPTY_SYSTEMTABLE   1
#define ERROR_NO_QUEUE_INFO       2
#define ERROR_UNABLE_TO_WRITE_Q   3
#define ERROR_NOVERSIONINFO       4
#define ERROR_SCRIPT_DOWNLOAD     5

#define PERMIT_DURATION       180  // 3 minutes = 180 secs

// #define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

#define CONTROL_PORT       "2001"

#define QUEUESDIR          "/tmp"

#define MAXBUFFER    1000

// -------------------------------------------------------------
// Globals
// -------------------------------------------------------------

static int command     = 0;
static int chanmask    = 0;
static int nfcmode     = 0;
static int browsertime = 0;

static int site       = 0;
static char * sites[] = { "EHV", "SGP", "UK" };

static int released   = 0;

// -------------------------------------------------------------
// XML response
// -------------------------------------------------------------

static void xmlOpen( void ) {
    printf( "Content-type: text/xml\r\n\r\n" );
    printf( "<?xml version='1.0' encoding='utf-8'?>\n" );
    printf( "<control>\n" );
    printf( "    <cmd>%d</cmd>\n", command );
}

static void xmlError( int err ) {
    printf( "    <err>%d</err>\n", err );
}

static void xmlDebug( int n, char * dbg ) {
    printf( "    <dbg%d>%s</dbg%d>\n", n, dbg, n );
}

static void xmlDebugInt( int n, int dbg ) {
    printf( "    <dbg%d>%d</dbg%d>\n", n, dbg, n );
}

static void xmlResponseWQ( int err ) {
    xmlError( ( err ) ? ERROR_UNABLE_TO_WRITE_Q : 0 );
}

static void xmlResponse2( int err, int status, char * extra ) {
    xmlError( err );
    printf( "    <status>%d</status>\n", status );
    if ( extra != NULL ) {
        printf( "    <extra>%s</extra>\n", extra );
    }
}

static void xmlClose( void ) {
    printf( "</control>\n" );
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
        return( 888 );
    }
    return( 123 );
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
    } else if ( strcmp( name, "site" ) == 0 ) {
        site = Atoi( value );
    } else if ( strcmp( name, "released" ) == 0 ) {
        released = Atoi( value );
    } else if ( strcmp( name, "chanmask" ) == 0 ) {
        chanmask = Atoi( value );
    } else if ( strcmp( name, "nfcmode" ) == 0 ) {
        nfcmode = Atoi( value );
    } else if ( strcmp( name, "browsertime" ) == 0 ) {
        browsertime = Atoi( value );
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
// Print queues and number of messages in them
// -------------------------------------------------------------

static char * serializeQueues( int len, char * buf ) {
    buf[0] = '\0';
    strcat( buf, "name,num" );

    char * queues[5] = { "none", "control interface", "secure joiner", "zcb-in", "dbp" };
        
    int i, num, queue;
    for ( i=1; i<5; i++ ) {
        if ( ( queue = queueOpen( i, 0 ) ) != -1 ) {
           num = queueGetNumMessages( queue );
           queueClose( queue );
        }
        char line[50];
        sprintf( line, ";%s,%d", queues[i], num );
        int l = strlen( line );
        if ( len > l ) {
            strcat( buf, line );
            len -= l;
        }
    }

    return( ( len > 0 ) ? buf : NULL );
}

// -------------------------------------------------------------
// Serialize version text
// -------------------------------------------------------------

static char * serializeVersionText( char * filename, int len, char * buf ) {
    buf[0] = '\0';

    FILE * file;
    if ( ( file = fopen( filename, "r" ) ) != NULL ) {
        char line[100];
        while ( fgets( line, 100, file ) != NULL ) {
            int l = strlen( line );
            if ( len > l ) {
                strcat( buf, line );
                len -= l;
            }
        }
        fclose( file );
        return( buf );
    }

    return( NULL );
}

// -------------------------------------------------------------
// Button helper
// -------------------------------------------------------------

static void printButton( char * value, int cmd ) {
    printf( "<input type='submit' value='%s' onclick='CtrlCommand(%d);' />\n", value, cmd );
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
    char   buf[MAXBUFFER];
    int    i, ret, err;
    char * txt;
    char * sys;
    char * qs;
    int    lastupdate;
    
    if ( argc > 1 ) {
        printf( "Help: %s <command> <site> <released> <chanmask> <nfcmode> <browsertime>\n", argv[0] );
        // Called from command line
        while ( i < argc ) {
            printf( "Argument %d = %s\n", i, argv[i] );
            if      ( i==1 ) command     = atoi( argv[i] );
            else if ( i==2 ) site        = atoi( argv[i] );
            else if ( i==3 ) released    = atoi( argv[i] );
            else if ( i==4 ) chanmask    = atoi( argv[i] );
            else if ( i==5 ) nfcmode     = atoi( argv[i] );
            else if ( i==6 ) browsertime = atoi( argv[i] );
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
        printf( "<script src='/js/jquery.js' charset='utf-8'></script>\n" );
        printf( "<script src='/js/control.js' charset='utf-8'></script>\n" );
        
        printf( "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />\n" );
        printf( "<title>IoT Control - %s</title>\n", myIP() );
        printf( "</head>\n" );
        printf( "<body onload='iotOnLoad();'>\n" );

        newLogAdd( NEWLOG_FROM_CGI, "CGI Control started" );

        printf( "<h1>IoT Control - " );
        printf( "<div style='display: inline' id='headerDate'>%s</div>", timeString( clk ) );
        printf( "</h1>\n" );

        // printf( "Command = %d\n",  command );
        // printf( "Formdata = %s\n", formdata );
        // printf( "Postdata = %s\n", postdata_cpy );

        printf( "<div id='javaerror' style='color: red; font-size: 1.4em'>Javascript error?</div>\n" );

        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>Zigbee Network</legend>\n" );
        printButton( "Normal Reset",  COMMAND_RESET );
        printButton( "Create Network", COMMAND_FACTORY_RESET );
        printf( "Permit join:" );
        printButton( "Start", COMMAND_PERMITJOIN_START );
        printf( "<div id='div_permitstop' style='display:inline'>" );
        printButton( "Stop", COMMAND_PERMITJOIN_STOP );
        printf( "</div>" );
        printf( "<div id='div_resetmsg' style='display:inline'></div>" );
        printf( "<div id='div_chanmask'>Please wait...</div>" );
        printf( "</fieldset>\n" );

        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>NFC Mode</legend>\n" );
        printf( "<div id='div_nfcmode'>Please wait...</div>" );
        printf( "</fieldset>\n" );

        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>Version</legend>\n" );
        printf( "<table border=0><tr><td class='grey'>\n" );
        printButton( "Refresh", COMMAND_VERSION_REFRESH );
        printf( "</td><td class='grey'><div id='div_version'></div></td></tr></table>" );
        printf( "</fieldset>\n" );

        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>Topology</legend>\n" );
        printButton( "Upload", COMMAND_TOPO_UPLOAD );
        printf( "<div id='div_upload' style='display:inline'></div>" );
        printf( "</fieldset>\n" );

        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>Time</legend>\n" );
        printButton( "Sync with browser", COMMAND_SYNC_TIME );
        printf( "<div id='div_browsertime' style='display:inline'></div>" );
        printf( "</fieldset>\n" );

        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>Restart</legend>\n" );
        printButton( "All",               COMMAND_RESTART_ALL );
        printButton( "Zcb",               COMMAND_RESTART_ZB  );
        printButton( "Secure Joiner",     COMMAND_RESTART_SJ  );
        // printButton( "Control Interface", COMMAND_RESTART_CI  );
        printButton( "NTAG Daemon",       COMMAND_RESTART_ND  );
        printf( "</fieldset>\n" );

        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>Plug Emulator</legend>\n" );
        printButton( "Start", COMMAND_PLUG_START );
        printButton( "Stop",  COMMAND_PLUG_STOP  );
        printf( "</fieldset>\n" );
/* Removed because it makes use of NXP remote server
        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>System update</legend>\n" );
        printButton( "Current Version", COMMAND_CURRENT_VERSION );
        printButton( "Check for updates", COMMAND_CHECK_UPDATES );
        printf( "<div id='div_sites' style='display:inline'></div>" );
        printf( "<div id='div_release' style='display:inline'></div>" );
        printf( "<div id='div_changes'>Please wait...</div>" );
        printf( "<div id='div_install'>" );
        printButton( "Install updates",  COMMAND_INSTALL_UPDATES  );
        printf( "Note: Database will be cleared by this operation" );
        printf( "</div>" );  // install
        printf( "<div id='div_reinstall'>" );
        printButton( "Re-install anyway",  COMMAND_INSTALL_UPDATES  );
        printf( "Note: Database will be cleared by this operation" );
        printf( "</div>" );  // reinstall
        printf( "</fieldset>\n" );
*/
        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>System table</legend>\n" );
        printf( "<div id='div_table'>Please wait...</div>" );
        printf( "</fieldset>\n" );

        printf( "<fieldset class='cbi-section'>\n" );
        printf( "<legend>Queue Overview</legend>\n" );
        printf( "<div id='div_queues'>Please wait...</div>" );
        printf( "</fieldset>\n" );

        printf( "<div id='debug'></div>" );

        printf( "</body>\n" );
        printf( "</html>\n" );

        break;

    case COMMAND_TSCHECK:
        xmlOpen();
        newDbOpen();
        lastupdate = newDbGetLastupdateSystem();
        newDbClose();
        xmlError( 0 );
        printf( "    <clk>%d</clk>\n", clk );
        printf( "    <pid>%d</pid>\n", getpid() );
        printf( "    <ts>%d</ts>\n", lastupdate );
        xmlClose();
        break;

    case COMMAND_RESET:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Normal reset" );
        xmlResponseWQ( writeControl( jsonCmdReset(1) ) );   // Normal
        xmlClose();
        break;

    case COMMAND_FACTORY_RESET:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Factory reset" );
        xmlResponseWQ( writeControl( jsonCmdReset(0) ) );   // Factory
        newLogAdd( NEWLOG_FROM_CGI, "Clear devices" );
        xmlError( writeControl( jsonDbEditClr( "devices" ) ) ); // Clear Devices database
        xmlClose();
        break;

//    case COMMAND_NETWORK_START:
//        xmlOpen();
//        newLogAdd( NEWLOG_FROM_CGI, "Network start" );
//        xmlResponseWQ( writeControl( jsonCmdStartNetwork() ) );
//        xmlClose();
//        break;

    case COMMAND_VERSION_REFRESH:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Get ZCB version" );
        xmlResponseWQ( writeControl( jsonCmdGetVersion() ) );
        xmlDebugInt( 6, argc );
        xmlClose();
        break;

    case COMMAND_TOPO_UPLOAD:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Topo upload" );
        xmlResponseWQ( writeControl( jsonTopoUpload() ) );
        xmlClose();
        break;

    case COMMAND_RESTART_ALL:
        xmlOpen();
        err  = writeControl( jsonProcStop( "zb" ) );
        err  = writeControl( jsonProcStop( "sj" ) );
        err  = writeControl( jsonProcStop( "nd" ) );
        err  = writeControl( jsonProcStop( "dbp" ) );
            
        err  = writeControl( jsonProcStart( "zb" ) );
        err  = writeControl( jsonProcStart( "sj" ) );
        err  = writeControl( jsonProcStart( "nd" ) );
        err  = writeControl( jsonProcStart( "dbp" ) );
        xmlError( err );
        xmlClose();
        break;

    case COMMAND_RESTART_ZB:
        xmlOpen();
        err  = writeControl( jsonProcRestart( "zb" ) );
        xmlError( err );
        xmlClose();
        break;

    case COMMAND_RESTART_SJ:
        xmlOpen();
        err  = writeControl( jsonProcRestart( "sj" ) );
        xmlError( err );
        xmlClose();
        break;

    case COMMAND_RESTART_ND:
        xmlOpen();
        err  = writeControl( jsonProcRestart( "nd" ) );
        xmlError( err );
        xmlClose();
        break;

    case COMMAND_PLUG_START:
        xmlOpen();
        err  = writeControl( jsonProcStart( "pt" ) );
        xmlError( err );
        xmlClose();
        break;

    case COMMAND_PLUG_STOP:
        xmlOpen();
        err  = writeControl( jsonProcStop( "pt" ) );
        xmlError( err );
        xmlClose();
        break;

    case COMMAND_GET_SYSTEMTABLE:
        xmlOpen();
        newDbOpen();
        sys = newDbSerializeSystem( MAXBUFFER, buf );
        newDbClose();
        xmlError( ( sys == NULL ) ? ERROR_EMPTY_SYSTEMTABLE : 0 );
        printf( "    <clk>%d</clk>\n", clk );
        printf( "    <sys>%s</sys>\n", ( sys ) ? sys : "-" );
        xmlClose();
        break;

    case COMMAND_GET_QUEUES:
        xmlOpen();
        qs = serializeQueues( MAXBUFFER, buf );
        xmlError( ( qs == NULL ) ? ERROR_NO_QUEUE_INFO : 0 );
        printf( "    <qs>%s</qs>\n", ( qs ) ? qs : "-" );
        xmlClose();
        break;

    case COMMAND_SET_CHANMASK:
        xmlOpen();
        sprintf( buf, "%08x", chanmask );
        sprintf( logbuffer, "Set channelmask %s", buf );
        newLogAdd( NEWLOG_FROM_CGI, logbuffer );
        xmlResponseWQ( writeControl( jsonCmdSetChannelMask( buf ) ) );
        xmlClose();
        break;

    case COMMAND_CURRENT_VERSION:
        xmlOpen();
        txt = serializeVersionText( "/usr/share/iot/iot_version.txt",
                                    MAXBUFFER, buf );
        if ( txt != NULL ) {
            xmlResponse2( 0, 0, txt );
        } else {
            xmlError( ERROR_NOVERSIONINFO );
        }
        xmlClose();
        break;

    case COMMAND_CHECK_UPDATES:
        xmlOpen();
#ifdef TARGET_RASPBIAN
        sprintf( buf, "sudo /usr/bin/iot_wget.sh %s%s/iot_version%s.txt -O /tmp/iot_version.txt",
                CLOUD_IMAGES_URL_BASE, sites[site],
                ( ( released ) ? "_rel" : "" ) );
#else
        sprintf( buf, "/usr/bin/iot_wget.sh %s%s/iot_version%s.txt -O /tmp/iot_version.txt",
                CLOUD_IMAGES_URL_BASE, sites[site],
                ( ( released ) ? "_rel" : "" ) );
#endif
        xmlDebug( 1, buf );
        ret = system( buf );
        xmlDebugInt( 2, ret );
        if ( ( ret & 0xFF ) == 0 ) {
            if ( ( ret = system( "diff /tmp/iot_version.txt /usr/share/iot/iot_version.txt > /dev/null" ) ) == 0 ) {
                xmlResponse2( 0, UPDATE_NO_CHANGES, NULL );
            } else {
                txt = serializeVersionText( "/tmp/iot_version.txt",
                                                MAXBUFFER, buf );
                if ( txt != NULL ) {
                    xmlResponse2( 0, UPDATE_CHANGES_WITHINFO, txt );
                } else {
                    xmlResponse2( 0, UPDATE_CHANGES_NOINFO, NULL );
                }
            }
        } else {
            xmlError( ERROR_NOVERSIONINFO );
        }
        xmlClose();
        break;

    case COMMAND_INSTALL_UPDATES:
        xmlOpen();
#ifdef TARGET_RASPBIAN
        sprintf( buf, "sudo /usr/bin/iot_wget.sh %s%s/iot_update%s.sh -O /tmp/iot_update.sh",
                CLOUD_IMAGES_URL_BASE, sites[site],
                ( ( released ) ? "_rel" : "" ) );
#else
        sprintf( buf, "/usr/bin/iot_wget.sh %s%s/iot_update%s.sh -O /tmp/iot_update.sh",
                CLOUD_IMAGES_URL_BASE, sites[site],
                ( ( released ) ? "_rel" : "" ) );
#endif
        xmlDebug( 1, buf );
        if ( ( ret = system( buf ) ) == 0 ) {
            xmlError( 0 );
        } else {
            xmlDebugInt( 2, ret );
            xmlError( ERROR_SCRIPT_DOWNLOAD );
        }
        xmlClose();
        break;

    case COMMAND_PERMITJOIN_START:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Start permit-join" );
        xmlResponseWQ( writeControl( jsonCmdSetPermitJoin( NULL,
                                        PERMIT_DURATION ) ) );
        xmlClose();
        break;

    case COMMAND_PERMITJOIN_STOP:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Stop permit-join" );
        xmlResponseWQ( writeControl( jsonCmdSetPermitJoin( NULL, 0 ) ) );
        xmlClose();
        break;

    case COMMAND_SYNC_TIME:
        xmlOpen();
        newLogAdd( NEWLOG_FROM_CGI, "Time synced with browser" );
        newtime.tv_sec = (time_t) browsertime;
        settimeofday( &newtime, NULL );
        xmlError( 0 );
        xmlClose();
        break;

    case COMMAND_SET_NFCMODE:
        xmlOpen();
        sprintf( logbuffer, "Set NFC mode %d", nfcmode );
        newLogAdd( NEWLOG_FROM_CGI, logbuffer );
        xmlResponseWQ( writeControl( jsonCmdNfcmode( nfcmode ) ) );
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
