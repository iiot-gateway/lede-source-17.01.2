// ------------------------------------------------------------------
// GW discovery socket program
// ------------------------------------------------------------------
// IoT program that handles the GW discovery from the LAN
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

/** \addtogroup gd
 * \file
 * \section gwdiscovery GW discovery
 * \brief Program that handles the GW discovery from the LAN
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "atoi.h"
#include "jsonCreate.h"
#include "dump.h"
#include "gateway.h"
#include "newLog.h"

#define DISCOVERY_PORT      1999

#define INPUTBUFFERLEN      64

// #define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* MAIN_DEBUG */

// -------------------------------------------------------------
// Globals
// -------------------------------------------------------------

static int port = DISCOVERY_PORT;
static int running = 1;

// ------------------------------------------------------------------------
// Main
// ------------------------------------------------------------------------

/**
 * \brief GwDiscovery's main entry point: opens a UDP socket on port <port> and listens
 * for incoming broadcast messages. Reports hostname and API version to discovery clients.
 * \param argc Number of command-line parameters
 * \param argv Parameter list (-h = help, -p <port> = listen to UDP port <port>)
 */

int main( int argc, char * argv[] ) {
    signed char opt;

    printf( "\n\nGW Discovery Started\n" );

    while ( ( opt = getopt( argc, argv, "hP:" ) ) != -1 ) {
        switch ( opt ) {
        case 'h':
            printf( "Usage: gd [-H host] [-P port]\n\n");
            return(0);
        case 'P':
            port = Atoi0( optarg );
            break;
        }
    }

    newLogAdd( NEWLOG_FROM_GW_DISCOVERY, "GW Discovery Started" );

    int                sock;
    struct sockaddr_in LAddr; // connector's address information
    struct sockaddr_in TargetAddr; // connector's address information
    int                NumBytes;
    unsigned char      buf[INPUTBUFFERLEN];
    int                Len;
    int                requestCount = 0;
    int                Opt;

    if ( ( sock = socket( AF_INET, SOCK_DGRAM, 0 ) ) > 0 ) {
        setsockopt( sock, SOL_SOCKET, SO_BROADCAST, &Opt, sizeof(Opt) );
        ioctl(sock, FIONBIO, 1);
    
        LAddr.sin_family      = AF_INET;
        LAddr.sin_port        = htons( port );
        LAddr.sin_addr.s_addr = INADDR_ANY;
        bind( sock, (struct sockaddr *)&LAddr, sizeof(LAddr) );

        while( running ) {

            char * message = jsonGwProperties( myHostname(), JSON_API_VERSION );
                
            // Wait for incoming UDP discover packets
            memset( &TargetAddr, 0, sizeof( TargetAddr ) );
            TargetAddr.sin_family      = AF_INET;
            TargetAddr.sin_port        = htons( DISCOVERY_PORT );
            TargetAddr.sin_addr.s_addr = htonl( INADDR_BROADCAST );
    
            memset( buf, 0, sizeof( buf ) );
              
            Len = sizeof( TargetAddr );
            NumBytes = recvfrom( sock, buf, sizeof(buf),
                                 0, (struct sockaddr *)&TargetAddr,
                                 (socklen_t *)&Len );
              
            // buf contains identification data
            if ( NumBytes > 0 ) {
                // Get IP address out of data packet
                dump( (char *)buf, sizeof(buf) );

                // Send response packet
                NumBytes = sendto( sock, message, strlen(message),
                               0, (struct sockaddr*)&TargetAddr,
                               sizeof(TargetAddr));
            }

            requestCount++;
            char * ip = (char *)&(TargetAddr.sin_addr);

            sprintf( logbuffer, "GW discovery client %d.%d.%d.%d (%d)",
                     ip[0], ip[1], ip[2], ip[3], requestCount );
            newLogAdd( NEWLOG_FROM_GW_DISCOVERY, logbuffer );
        }
        close(sock);
    } else {
        sprintf( logbuffer, "Error opening GW discovery socket %d", port );
        newLogAdd( NEWLOG_FROM_GW_DISCOVERY, logbuffer );
    }

    return( 0 );
}
