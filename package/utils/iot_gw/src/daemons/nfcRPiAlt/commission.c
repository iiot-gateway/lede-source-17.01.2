// ------------------------------------------------------------------
// Commissioning
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ------------------------------------------------------------------

/** \addtogroup nd
* \file
* \brief NDEF Commissioning 'app'
* Based on the incoming NDEF record, and the selected NfcMode, this 'app'
* completes the outgoing NDEF record to be written back to the device.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "atoi.h"
#include "nfcreader.h"
#include "newDb.h"
#include "systemtable.h"
#include "parsing.h"
#include "json.h"
#include "socket.h"
#include "nibbles.h"
#include "jsonCreate.h"
#include "dump.h"
#include "ndef.h"
#include "newLog.h"
#include "audiofb.h"
#include "commission.h"

#define SJ_SOCKET_PORT      "2000"
#define CI_SOCKET_PORT      "2001"

#define INPUTBUFFERLEN   200

// #define COMM_DEBUG
#ifdef COMM_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* COMM_DEBUG */

// ------------------------------------------------------------------------
// Globals
// ------------------------------------------------------------------------

static char inputBuffer[INPUTBUFFERLEN + 2];

static int success = 0;
static int ready   = 0;

static void * glob_join     = NULL;

static int nfcMode = NFC_MODE_COMMISSION;

static uint8_t handled_seqnr = -1;
static uint8_t out_seqnr = 0;

extern int dbOpened;

// ------------------------------------------------------------------
// Attributes
// ------------------------------------------------------------------
    
#define NUMINTATTRS  2
static char * intAttrs[NUMINTATTRS] = { "keyseq", "chan" };

#define NUMSTRINGATTRS  5
static char * stringAttrs[NUMSTRINGATTRS] = {
    "nwkey", "mic", "tcaddr", "pan", "extpan" };
static int    stringMaxlens[NUMSTRINGATTRS] = {
    32,      8,      16,       4,     16 };

// ------------------------------------------------------------------------
// Dump
// ------------------------------------------------------------------------

#ifdef COMM_DEBUG
static void commissionDump ( void ) {
    printf( "\nCommission dump:\n" );
    int i;
    for ( i=0; i<NUMINTATTRS; i++ ) {
        printf( "- %-12s = %d\n",
                intAttrs[i], parsingGetIntAttr( intAttrs[i] ) );
    }
    for ( i=0; i<NUMSTRINGATTRS; i++ ) {
        printf( "- %-12s = %s\n",
                stringAttrs[i], parsingGetStringAttr0( stringAttrs[i] ) );
    }
}
#endif

// -------------------------------------------------------------
// Parsing
// -------------------------------------------------------------

static void com_onError(int error, char * errtext, char * lastchars) {
    printf("onError( %d, %s ) @ %s\n", error, errtext, lastchars);
}

static void com_onObjectStart(char * name) {
    // printf("onObjectStart( %s )\n", name);
    parsingReset();
}

static void com_onObjectComplete(char * name) {
    // printf("onObjectComplete( %s )\n", name);

    if ( strcmp( name, "joinsecure" ) == 0 ) {
#ifdef COMM_DEBUG
        commissionDump();
#endif
        
    secjoin_t * secjoin = (secjoin_t *)glob_join;

    memset( (char *)secjoin, 0, sizeof( secjoin_t ) );

    secjoin->channel = parsingGetIntAttr( "chan" );
    secjoin->keyseq  = parsingGetIntAttr( "keyseq" );
    secjoin->pan     = AtoiHex( parsingGetStringAttr( "pan" ) );
    nibblestr2hex( secjoin->extpan, LEN_EXTPAN,
        parsingGetStringAttr0( "extpan" ), LEN_EXTPAN * 2 );
    nibblestr2hex( secjoin->nwkey, LEN_NWKEY,
        parsingGetStringAttr0( "nwkey" ), LEN_NWKEY * 2 );
    nibblestr2hex( secjoin->mic, LEN_MIC,
        parsingGetStringAttr0( "mic" ), LEN_MIC * 2 );
    nibblestr2hex( secjoin->tcaddr, LEN_TCADDR,
        parsingGetStringAttr0( "tcaddr" ), LEN_TCADDR * 2 );

        ready = 1;
    }
}

static void com_onArrayStart(char * name) {
    // printf("onArrayStart( %s )\n", name);
}

static void com_onArrayComplete(char * name) {
    // printf("onArrayComplete( %s )\n", name);
}

static void com_onString(char * name, char * value) {
    // printf("onString( %s, %s )\n", name, value);
    parsingStringAttr( name, value );
}

static void com_onInteger(char * name, int value) {
    // printf("onInteger( %s, %d )\n", name, value);
    parsingIntAttr( name, value );
}

// ------------------------------------------------------------------
// Functions
// ------------------------------------------------------------------

/**
* \brief Contacts SecureJoiner to get the SecJoin struct.
* \param linkinfo struct from the device
* \param join struct with (encrypted) network data for device to join
* \retval 1 when OK
* \retval 0 on error
*/

static int commission( linkinfo_t * linkinfo, secjoin_t * join ) {

    DEBUG_PRINTF( "Commissioning Started\n" );

    success = 1;

    // Check input parameters
    if ( linkinfo == NULL || join == NULL ) {
        return( 0 );
    }

    // Make globals of the two output parameters
    glob_join     = join;

    // Prepare creating of JSON linkinfo message
    char mac_str[LEN_MAC_NIBBLE + 2];
    char linkkey_str[(LEN_LINKKEY * 2 ) + 2];
    hex2nibblestr( mac_str,     linkinfo->mac,     LEN_MAC );
    hex2nibblestr( linkkey_str, linkinfo->linkkey, LEN_LINKKEY );

#ifdef COMM_DEBUG
    printf( "Linkinfo:\n" );
    dump( (char *)linkinfo, sizeof( linkinfo_t ) );
    // printf( "MAC:\n" ); dump( linkinfo->mac, LEN_MAC_NIBBLE );
    printf( "MAC = %s\n", mac_str );
    printf( "LinkkeyStr = %s\n", linkkey_str );
#endif

    int len;
    int handle = socketOpen( "localhost", SJ_SOCKET_PORT, 0 );

    if ( handle >= 0 ) {
        DEBUG_PRINTF( "SecJoin socket opened\n" );

        if ( success ) {
            if ( socketWriteString( handle,
                                    jsonLinkInfo( linkinfo->version,
                                                linkinfo->type,
                                                linkinfo->profile,
                                                mac_str,
                                                linkkey_str ) ) > 0 ) {
                DEBUG_PRINTF( "Sent linkinfo\n" );

                jsonReset();

                success     = 1;    // So far so good
                ready       = 0;

                while ( !ready &&
                        ( len = socketReadWithTimeout( handle,
                                                    inputBuffer,
                                                    200, 2 ) ) > 0 ) {
//#ifdef COMM_DEBUG
//                    printf( "Received from socket %d bytes\n", len );
//                    dump( inputBuffer, len );
//#endif
    
                    int i;
                    for ( i=0; i<len; i++ ) {
                        // printf( "%c", inputBuffer[i] );
                        jsonEat( inputBuffer[i] );
                    }
                }

                if ( success ) {
                    DEBUG_PRINTF( "Commissioning ready\n" );
                }
            } else {
                newLogAdd( NEWLOG_FROM_NFC_DAEMON, "Error writing linkinfo to socket" );
                success = 0;
            }
        }

        socketClose( handle );

    } else { 
        sprintf( logbuffer, "Error opening port to %s", SJ_SOCKET_PORT );
        newLogAdd( NEWLOG_FROM_NFC_DAEMON, logbuffer );
    }

    DEBUG_PRINTF( "Commission exit\n");
    return( success );
}

// ------------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------------

static int getDeviceDev( int from ) {
    switch ( from ) {
        default:
        case FROM_SETTINGSAPP:
        case FROM_GATEWAY:
            return( DEVICE_DEV_UNKNOWN );
        case FROM_ROUTER:
            return( DEVICE_DEV_SWITCH ); /** \todo Temporary use from=router for switches */
        case FROM_MANAGER:
            return( DEVICE_DEV_MANAGER );
        case FROM_UISENSOR:
            return( DEVICE_DEV_UISENSOR );
        case FROM_SENSOR:
            return( DEVICE_DEV_SENSOR );
        case FROM_ACTUATOR:
            return( DEVICE_DEV_PUMP );
        case FROM_UI_ONLY:
            return( DEVICE_DEV_UI );
        case FROM_PLUGMETER:
            return( DEVICE_DEV_PLUG );
        case FROM_LAMP_ONOFF:
        case FROM_LAMP_DIMM:
        case FROM_LAMP_TW:
        case FROM_LAMP_RGB:
            return( DEVICE_DEV_LAMP );
    }
}

static char * getDeviceType( int from ) {
    switch ( from ) {
        default:
            return( DEVICE_TYPE_UNKNOWN );
        case FROM_MANAGER:
            return( DEVICE_TYPE_MAN_HEATING );
        case FROM_LAMP_ONOFF:
            return( DEVICE_TYPE_LAMP_ONOFF );
        case FROM_LAMP_DIMM:
            return( DEVICE_TYPE_LAMP_DIMM  );
        case FROM_LAMP_TW:
            return( DEVICE_TYPE_LAMP_TW    );
        case FROM_LAMP_RGB:
            return( DEVICE_TYPE_LAMP_COLOR );
        case FROM_ROUTER:
            return( DEVICE_TYPE_SWITCH_ONOFF ); /** \todo Temporary use from=router for switches */
    }
}

// ------------------------------------------------------------------------
// DB edit via Control Interface
// ------------------------------------------------------------------------

/**
* \brief Sends dbEdit command to Control Interface
* \param mode 1=add, 0=remove
*/

static void dbEdit( int mode, char * mac, int dev, char * ty ) {

    char mac_str[LEN_MAC_NIBBLE + 2];
    hex2nibblestr( mac_str, mac, LEN_MAC );

    DEBUG_PRINTF( "Mode = %d, mac = %s, dev = %d, ty = %s\n",
            mode, mac_str, dev, ty );

    char * message;
    if ( mode ) {
        message = jsonDbEditAdd( mac_str, dev, ty );
    } else {
        message = jsonDbEditRem( mac_str );
    }
    DEBUG_PRINTF( "Message = %s\n", message );

    int handle = socketOpen( "localhost", CI_SOCKET_PORT, 0 );

    if ( handle >= 0 ) {
        DEBUG_PRINTF( "Control socket opened\n" );

        if ( socketWriteString( handle, message ) > 0 ) {
            DEBUG_PRINTF( "DB-edit sent\n" );
        }

        socketClose( handle );
    } else { 
        sprintf( logbuffer, "Error opening port to %s", CI_SOCKET_PORT );
        newLogAdd( NEWLOG_FROM_NFC_DAEMON, logbuffer );
    }
}

// ------------------------------------------------------------------
// Handle NFC commissioning
// ------------------------------------------------------------------

/**
* \brief Based on the incoming NDEF record, and the selected NfcMode, 
* completes the outgoing NDEF record to be written back to the device.
* \param handle  Handle to NDEF tag
* \param payload Pointer to NDEF payload array of bytes
* \param plen    Length of the payload array
* \retval 1 when OK
* \retval 0 on error
*/

int commissionHandleNfc( int handle, char * payload, int plen ) {
    static int prevNfcMode = -1;
    int ret = 0;
#ifdef COMM_DEBUG
    printf( "Commission handle NFC:\n" );
    // dump( payload, plen );
#endif
    commission_t * c = (commission_t *)payload;
    if ( c->version == COMMISSION_SUPPORTED_VERSION ) {
        
        nfcMode = newDbSystemGetInt( "nfcmode" );
        DEBUG_PRINTF( "nfcMode = %d\n", nfcMode );
            
        if ( ( c->out.SeqNr == handled_seqnr ) && ( prevNfcMode == nfcMode ) ) {
            printf( "Message already handled\n" );
            ret = 1;
        } else {
            
            prevNfcMode = nfcMode;
        
            if ( nfcMode == NFC_MODE_DECOMMISSION ) {
                newLogAdd( NEWLOG_FROM_NFC_DAEMON, "Decommission mode" );

                // use COMMAND_FACTORYNEW instead COMMAND_LEAVE. The device should handle a Leave
                if ( c->in.Command != COMMAND_FACTORYNEW ) {
                    memset( (char *)&c->in, 0, sizeof( commission_in_t ) );
                    c->in.From    = FROM_GATEWAY;
                    c->in.SeqNr   = out_seqnr++;
                    c->in.Command = COMMAND_FACTORYNEW;

                    if ( ndef_write( handle, payload, plen, COMMISSION_URI, strlen( COMMISSION_URI ) ) ) {
                        // Remove device from GW dB (via Control Interface)
                        dbEdit( 0, (char *)c->out.NodeMacAddress, 0, NULL );
                        ret = 1;
                    }
                } else {
                    printf( "Tag already contains FACTORYNEW command\n" );
                    ret = 1;
                }
                
                if ( ret ) afbPlayPattern( AFB_PATTERN_TWOMEDIUM );

            } else if ( nfcMode == NFC_MODE_COMMISSION ) {
                newLogAdd( NEWLOG_FROM_NFC_DAEMON, "Commission mode" );

                linkinfo_t linkinfo;
                secjoin_t  secjoin;

                linkinfo.version = 0;
                linkinfo.type    = 0;
                linkinfo.profile = 0;
                memcpy( linkinfo.mac,     c->out.NodeMacAddress, LEN_MAC );
                memcpy( linkinfo.linkkey, c->out.NodeLinkKey,    LEN_LINKKEY );
                
                ret = commission( &linkinfo, &secjoin );
                if ( ret ) {
#ifdef NDEF_DEBUG
                    printf( "secjoin :\n" );
                    dump( (char *)&secjoin, sizeof( secjoin_t ) );
                    printf(" - channel = 0x%02x\n", secjoin.channel );
                    printf(" - keyseq  = 0x%02x\n", secjoin.keyseq );
                    printf(" - pan     = 0x%04x\n", secjoin.pan );
                    printf(" - extpan  ");
                    dump( (char*)&secjoin.extpan, LEN_EXTPAN );
                    printf(" - nwkey ");
                    dump( (char*)&secjoin.nwkey, LEN_NWKEY );
                    printf(" - mic ");
                    dump( (char*)&secjoin.mic, LEN_MIC );
                    printf(" - tcaddr ");
                    dump( (char*)&secjoin.tcaddr, LEN_TCADDR );
#endif
    
                    memset( (char *)&c->in, 0, sizeof( commission_in_t ) );
                    c->in.From    = FROM_GATEWAY;
                    c->in.SeqNr   = out_seqnr++;
                    c->in.Command = COMMAND_JOIN;
                    
                    c->in.Channel = secjoin.channel;
                    c->in.KeySeq = secjoin.keyseq;
                    c->in.PanIdMSB = ( secjoin.pan >> 8 ) & 0xFF;
                    c->in.PanIdLSB = secjoin.pan & 0xFF;
                    memcpy( (char *)c->in.Mic,      secjoin.mic,    LEN_MIC );  // bytes[12..15] TODO
                    memcpy( (char *)c->in.EncKey,   secjoin.nwkey,  LEN_NWKEY );
                    memcpy( (char *)c->in.ExtPanId, secjoin.extpan, LEN_EXTPAN );
                    memcpy( (char *)c->in.TrustCenterAddress, secjoin.tcaddr, LEN_TCADDR );                

                    if ( ndef_write( handle, payload, plen, COMMISSION_URI, strlen( COMMISSION_URI ) ) ) {
                        // Add device to GW dB (via Control Interface)
                        int    dev = getDeviceDev( c->out.From );
                        char * ty  = getDeviceType( c->out.From );

                        DEBUG_PRINTF( "From = %d, dev = %d, ty = %s\n", c->out.From, dev, ty );
                        dbEdit( 1, linkinfo.mac, dev, ty );
                        ret = 1;
                    } else {
                        ret = 0;
                    }
                }
                
                if ( ret ) afbPlayPattern( AFB_PATTERN_ONELONG );
            }

            handled_seqnr = c->out.SeqNr;
        }
    } else {
        printf( "Commission version not supported (%d != %d)\n", 
                c->version, COMMISSION_SUPPORTED_VERSION );
    }
    printf( "Commission handle returns with %d\n", ret );
    return( ret );
}

// ------------------------------------------------------------------
// Init / Reset
// ------------------------------------------------------------------

/**
* \brief Needs to be called on tag-departure to avoid unnecessary tag writes
*/
void commissionReset( void ) {
    handled_seqnr = -1;
}

/**
* \brief Initialize parser for Secure-Joiner response
*/
void commissionInit( void ) {
    int i;

    jsonSetOnError(com_onError);
    jsonSetOnObjectStart(com_onObjectStart);
    jsonSetOnObjectComplete(com_onObjectComplete);
    jsonSetOnArrayStart(com_onArrayStart);
    jsonSetOnArrayComplete(com_onArrayComplete);
    jsonSetOnString(com_onString);
    jsonSetOnInteger(com_onInteger);
    jsonReset();

    for ( i=0; i<NUMINTATTRS; i++ ) {
        parsingAddIntAttr( intAttrs[i] );
    }
    for ( i=0; i<NUMSTRINGATTRS; i++ ) {
        parsingAddStringAttr( stringAttrs[i], stringMaxlens[i] );
    }
}

