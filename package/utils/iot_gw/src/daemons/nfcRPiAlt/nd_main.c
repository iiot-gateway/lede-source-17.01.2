// ---------------------------------------------------------------------
// NFC reader
// ---------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ---------------------------------------------------------------------

/** \addtogroup nd
 * \file
 * \section nfcdaemon NFC daemon
 * \brief NFC reader based on PN7120 HW
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include "iotSemaphore.h"
#include "newDb.h"
#include "linux_nfc_api.h"
#include "tools.h"
#include "mful.h"
#include "ledfb.h"
#include "newLog.h"

#define SEM_KEY     76578  // Unique random number

// ---------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------

int hasTag = 0;
int dbOpened = 0;

// ---------------------------------------------------------------------
// Globals, local
// ---------------------------------------------------------------------

static nfcTagCallback_t g_TagCB;

static int tagHandle     = 0;
static int tagNewCnt     = 0;
static int tagHandledCnt = 0;
 
// ---------------------------------------------------------------------
// Tag arrive / leave
// ---------------------------------------------------------------------

/**
 * \brief Called on tag arrival. When tag is Mifare, the SEM_KEY semaphore of
 * the main loop is released for further handling. Sets global 'hasTag' flag.
 * \param pTagInfo Info about the tag from Nfc library
 */
static void onTagArrival(nfc_tag_info_t *pTagInfo)
{
    printf( "onTagArrival\n" );
  
    if ( pTagInfo->technology == TARGET_TYPE_MIFARE_UL ) {
        printf("Found 'Type A - Mifare Ul'\n");
        tagHandle = pTagInfo->handle;
        hasTag = 1;
	tagNewCnt++;
	semV( SEM_KEY );
    }
}

/**
 * \brief Called on tag departure. Resets global 'hasTag' flag
 */
static void onTagDepature(void) {    
    printf( "onTagDepature\n" );
    hasTag = 0;
    mfulReset();
}

// ---------------------------------------------------------------------
// Polling
// ---------------------------------------------------------------------

static int InitPollMode()
{
    int res = 0x00;
    
    printf( "InitPollMode\n" );
    
    g_TagCB.onTagArrival = onTagArrival;
    g_TagCB.onTagDepature = onTagDepature;
    
    if(0x00 == res)
    {
        res = nfcManager_doInitialize();
        if(0x00 != res)
        {
            printf("NfcService Init Failed\n");
        }
    }
    if(0x00 == res)
    {
        nfcManager_registerTagCallback(&g_TagCB);
    }

    if(0x00 == res)
    {
        nfcManager_enableDiscovery(DEFAULT_NFA_TECH_MASK, 0, 0, 0);
    }
    return res;
}

static int DeinitPollMode()
{
    int res = 0x00;
    
    nfcManager_disableDiscovery();
    
    nfcManager_deregisterTagCallback(&g_TagCB);

    res = nfcManager_doDeinitialize();
    
    if(0x00 != res)
    {
        printf("NFC Service Deinit Failed\n");
    }
    return res;
}

// -------------------------------------------------------------
// Main
// -------------------------------------------------------------

/**
 * \brief NFC reader's main entry point: opens the IoT database (for the NfcMode flag),
 * starts polling for tags and waits on the SEM_KEY semaphore to be woken up for 
 * handling a Mifare card.
 * Note: P-operation has timeout so that the leds on the reader PCB can be flashed.
 */

int main(int argc, char ** argv) {

    newLogAdd( NEWLOG_FROM_NFC_DAEMON, "(ALT) NFC reader daemon started" );
    
    dbOpened = newDbOpen();
    
    mfulInit();
    
    int res = InitPollMode();
     
    printf( "Waiting for semaphore release\n" );
    while ( res == 0 ) {
        if ( ( tagNewCnt > tagHandledCnt ) || semPtimeout( SEM_KEY, 2 ) ) {
            printf( "Woken up...\n" );
            tagHandledCnt++;
            mfulHandle( tagHandle );
            printf( "Waiting for semaphore release %d %d\n", tagNewCnt, tagHandledCnt );
        } else {
            ledGreenOn();
            usleep( 50 * 1000 );
            ledGreenOff();
            ledRedOff();
        }
    }
    
    res = DeinitPollMode();
    
    newDbClose();

    return 0;
}
 
