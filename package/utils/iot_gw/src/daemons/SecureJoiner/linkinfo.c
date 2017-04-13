// ------------------------------------------------------------------
// Link Info
// ------------------------------------------------------------------
// Linkinfo parser receives linkinfo struct
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \addtogroup sj
 * \file
 * \brief Linkinfo JSON parser
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "iotError.h"
#include "newLog.h"
#include "parsing.h"
#include "queue.h"
#include "socket.h"
#include "json.h"
#include "jsonCreate.h"
#include "nibbles.h"
#include "dump.h"
#include "linkinfo.h"

// #define LI_DEBUG

#ifdef LI_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* LI_DEBUG */

// ------------------------------------------------------------------
// Attributes
// ------------------------------------------------------------------
    
#define NUMINTATTRS  3
static char * intAttrs[NUMINTATTRS] = { "version", "type", "profile" };

#define NUMSTRINGATTRS  2
static char * stringAttrs[NUMSTRINGATTRS] = { "mac", "linkkey" };
static int    stringMaxlens[NUMSTRINGATTRS] = { 16,    32 };

// ------------------------------------------------------------------
// Globals
// ------------------------------------------------------------------

static linkinfo_t linkinfo;

// ------------------------------------------------------------------
// LinkInfo
// ------------------------------------------------------------------

/**
 * \brief Init the JSON Parser for this type of commands
 */
void linkinfoInit( void ) {
    int i;
    for ( i=0; i<NUMINTATTRS; i++ ) {
        parsingAddIntAttr( intAttrs[i] );
    }
    for ( i=0; i<NUMSTRINGATTRS; i++ ) {
        parsingAddStringAttr( stringAttrs[i], stringMaxlens[i] );
    }
}

#ifdef LI_DEBUG
static void linkinfoDump ( void ) {
    printf( "Linkinfo dump:\n" );
    int i;
    for ( i=0; i<NUMINTATTRS; i++ ) {
        printf( "- %-12s = %d\n", intAttrs[i], parsingGetIntAttr( intAttrs[i] ) );
    }
    for ( i=0; i<NUMSTRINGATTRS; i++ ) {
        printf( "- %-12s = %s\n", stringAttrs[i], parsingGetStringAttr( stringAttrs[i] ) );
    }
}
#endif

// ------------------------------------------------------------------
// Handle
// ------------------------------------------------------------------

/**
 * \brief JSON Parser for linkinfo data
 * \retval 0 When OK
 * \returns A pointer to a global linkinfo structure, or NULL on case of an error
 */
linkinfo_t * linkinfoHandle( void ) {
    // linkinfoDump();

    newLogAdd( NEWLOG_FROM_SECURE_JOINER, "Handle link info" );

    char * mac     = parsingGetStringAttr( "mac" );
    char * linkkey = parsingGetStringAttr( "linkkey" );

    if ( mac != NULL && linkkey != NULL ) {
        linkinfo.version = parsingGetIntAttr( "version" );
        linkinfo.type    = parsingGetIntAttr( "type" );
        linkinfo.profile = parsingGetIntAttr( "profile" );
        strcpy( linkinfo.mac, mac );
        strcpy( linkinfo.linkkey, linkkey );
        return( &linkinfo );
    }

    return( NULL );
}

