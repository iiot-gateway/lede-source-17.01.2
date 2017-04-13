// ---------------------------------------------------------------------
// NDEF layer
// ---------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ------------------------------------------------------------------

/** \addtogroup nd
 * \file
* \brief NDEF layer
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#include "linux_nfc_api.h"

#include "newLog.h"
#include "dump.h"
#include "jsonCreate.h"
#include "socket.h"
#include "nibbles.h"
#include "nfcreader.h"
#include "commission.h"
#include "ndef.h"

// #define NDEF_DEBUG
#define NDEF_DEBUG_DUMP

#ifdef NDEF_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* NDEF_DEBUG */

#define NDEF_OFFSET 4

// ------------------------------------------------------------------------
// TAG Header
// ------------------------------------------------------------------------

typedef struct tag_header_small_struct {
  uint8_t  isndef;          // 0x03
  uint8_t  len;             // 0x0e in case of "com.nxp:wise01"
} __attribute__((__packed__)) tag_header_small_t;

typedef struct tag_header_large_struct {
  uint8_t  isndef;          // 0x03
  uint8_t  isff;            // 0xff
  uint8_t  msblen;          // msbyte len
  uint8_t  lsblen;          // lsbyte len
} __attribute__((__packed__)) tag_header_large_t;

// ------------------------------------------------------------------------
// NDEF Header
// ------------------------------------------------------------------------

typedef struct ndef_header_small_struct {
  uint8_t  flags;           // 0xd4 (sr == 1)
  uint8_t  uri_len;         // 0x0e in case of "com.nxp:wise01"
  uint8_t  payload_len;
} __attribute__((__packed__)) ndef_header_small_t;

typedef struct ndef_header_large_struct {
  uint8_t  flags;           // 0xc4 (sr == 0)
  uint8_t  uri_len;         // 0x0e in case of "com.nxp:wise01"
  uint32_t payload_len;
} __attribute__((__packed__)) ndef_header_large_t;

// ------------------------------------------------------------------------
// Endianess
// ------------------------------------------------------------------------

#if 0

// Enable for Jennic
static uint16_t tag2local_uint16( uint16_t tag ) {
  return( tag );
}

static uint16_t local2tag_uint16( uint16_t local ) {
  return tag2local_uint16( local );
}

static uint32_t tag2local_uint32( uint32_t tag ) {
  return( tag );
}

static uint32_t local2tag_uint32( uint32_t local ) {
  return tag2local_uint32( local );
}

#else

#if 0
// BIG ENDIAN, enable for LPC's
static uint16_t tag2local_uint16( uint16_t tag ) {
  uint16_t local = ( ( tag & 0xFF ) << 8 ) + ( ( tag >> 8 ) & 0xFF );
  return( local );
}

static uint16_t local2tag_uint16( uint16_t local ) {
  return tag2local_uint16( local );
}
#endif

static uint32_t tag2local_uint32( uint32_t tag ) {
  uint32_t local = ( ( tag & 0xFF     ) << 24 ) +
		   ( ( tag & 0xFF00   ) << 8  ) +
	           ( ( tag & 0xFF0000 ) >> 8  ) +
	           ( ( tag >> 24 ) & 0xFF );
  return( local );
}

static uint32_t local2tag_uint32( uint32_t local ) {
  return tag2local_uint32( local );
}

#endif

// --------------------------------------------------------------------------
// NDEF cache (prevents unnecessary tag writes)
// --------------------------------------------------------------------------

static unsigned char ndefCache[NDEF_MAX_SIZE];

// --------------------------------------------------------------------------
// READ
// --------------------------------------------------------------------------

/**
 * \brief Reads the NDEF uri and payload from a tag
 * \param handle Handle to the tag
 * \param payload Pointer to a buffer to receive the payload data
 * \param maxpayload Size of the buffer to receive the payload data
 * \param uri Pointer to a buffer to receive the uri
 * \param maxuri Size of the buffer to receive the uri
 * \return payloadsize when NDEF is valid
 * \return 0 when NDEF is invalid
 * \return -1 when tag could not be read
 */
int ndef_read( int handle, char * payload, int maxpayload, char * uri, int maxuri ) {

    // Defined output
    uri[0] = '\0';
    payload[0] = '\0';

    // Read first 4 data blocks 2
    unsigned char MifareReadCmd[2] = {0x30U, 0};  // Address has 4 bytes resolution
    MifareReadCmd[1] = NDEF_OFFSET;
    if ( nfcTag_transceive(handle, MifareReadCmd, 2, ndefCache, 16, 200) ) {
        
        tag_header_small_t * t = (tag_header_small_t *)ndefCache;
        if ( t->isndef == 0x03 ) {
            // Is NDEF
            int currLen = t->len;
            int bytesRead = 16 - sizeof( tag_header_small_t );
            unsigned char * ndefStart = &ndefCache[ sizeof( tag_header_small_t ) ];
            if ( currLen == 0xFF ) {
                tag_header_large_t * tl = (tag_header_large_t *)ndefCache;
                currLen = ( tl->msblen * 256 ) + tl->lsblen;
                bytesRead = 16 - sizeof ( tag_header_large_t );
                ndefStart = &ndefCache[ sizeof( tag_header_large_t ) ];
            }
            
            if ( currLen < ( NDEF_MAX_SIZE - 2 ) ) {
   
                int bytesToRead = currLen - bytesRead;
                int num16 = ( bytesToRead + 15 ) / 16;
                num16++;
                int i, ok = 1;
                for ( i=1; i<num16 && ok; i++ ) {   // Start with 1 (second 16 bytes)
                    MifareReadCmd[1] = i*4 + NDEF_OFFSET;
                    ok = nfcTag_transceive(handle, MifareReadCmd, 2, &ndefCache[i*16], 16, 200);
                }
                
                if ( ok ) {
#ifdef NDEF_DEBUG_DUMP
                    dump( (char *)ndefCache, num16*16 );
#endif                    
                    int uri_len, plen;
                    ndef_header_small_t * h = (ndef_header_small_t *)ndefStart;
                    unsigned char * uriStart = ndefStart + sizeof(ndef_header_small_t);
                    if ( h->flags & 0x10 ) { // sr bit set?
                        uri_len = h->uri_len;
                        plen = h->payload_len;
                    } else {
                        ndef_header_large_t * hl = (ndef_header_large_t *)ndefStart;
                        uriStart = ndefStart + sizeof(ndef_header_large_t);
                        uri_len = hl->uri_len;
                        plen = tag2local_uint32( hl->payload_len );
                    }
                    
                    unsigned char * payloadStart = uriStart + uri_len;
                    
                    if ( uri_len < maxuri ) {
                        // Cut out the URI          
                        memcpy( uri, uriStart, uri_len );
                        uri[uri_len] = '\0';
                        DEBUG_PRINTF( "URI = %s\n", uri );
                    }
            
                    if ( plen < maxpayload ) {
                        // Cut out the payload
                        memcpy( (char *)payload, payloadStart, plen );
                    }
                    return( plen );
                
                } else {
                    printf("\t\t\t\tRead NDEF Content Failed\n");
                }
            } else {
                printf("\t\t\t\tNDEF contents too long\n");
            }
        } else {
            printf("\t\t\t\tTag not NDEF\n");
        }
        return( 0 );
        
    } else {
        printf("\t\t\t\tRead first block failed\n");
    }

    return( -1 );
}

// --------------------------------------------------------------------------
// WRITE
// --------------------------------------------------------------------------

/**
 * \brief Writes the NDEF uri and payload to a tag
 * \param handle Handle to the tag
 * \param payload Pointer to a buffer with the payload data
 * \param payload_len Length of the payload data
 * \param uri Pointer to the uri
 * \param uri_len Length of the uri
 * \return 1 on success, 0 on error
 */
int ndef_write( int handle, char * payload, int payload_len, char * uri, int uri_len ) {
  
    int res = 0;
    unsigned char ndefContent[NDEF_MAX_SIZE];
    
    // Determine NDEF header
    int ndef_hdr_size = sizeof(ndef_header_small_t);
    if ( payload_len >= 255 ) {
        ndef_hdr_size = sizeof(ndef_header_large_t);
    }

    // Determine TAG header
    int tag_hdr_size = sizeof( tag_header_small_t );
    int ndef_len = ndef_hdr_size + uri_len + payload_len;
    if ( ndef_len < 255 ) {
        tag_header_small_t * t = (tag_header_small_t *)ndefContent;
        t->isndef = 0x03;
        t->len = ndef_len;
    } else {
        tag_header_large_t * tl = (tag_header_large_t *)ndefContent;
        tl->isndef = 0x03;
        tl->isff = 0xFF;
        tl->msblen = ndef_len / 256;
        tl->lsblen = ndef_len % 256;
        tag_hdr_size = sizeof( tag_header_large_t );
    }

    unsigned char * ndefStart = ndefContent + tag_hdr_size;    
    if ( payload_len < 255 ) {
        ndef_header_small_t * h = (ndef_header_small_t *)ndefStart;
        h->flags = 0xd4;          // flags: mb=1, me=1, cf=0, sr=1, il=0, tnf=100
        h->uri_len = uri_len;
        h->payload_len = payload_len;
    } else {
        ndef_header_large_t * hl = (ndef_header_large_t *)ndefStart;
        hl->flags = 0xc4;          // flags: mb=1, me=1, cf=0, sr=0, il=0, tnf=100
        hl->uri_len = uri_len;
        hl->payload_len = local2tag_uint32( payload_len );
    }
    
    unsigned char * uriStart = ndefStart + ndef_hdr_size;
    memcpy( uriStart, uri, uri_len );

    unsigned char * payloadStart = uriStart + uri_len;
    memcpy( payloadStart, payload, payload_len );
    
    // Put an NDEF end-marker after our NDEF
    unsigned char * endmarkerStart = payloadStart + payload_len;
    *endmarkerStart = 0xFE;
    
    int len = tag_hdr_size + ndef_len + 1;

#ifdef NDEF_DEBUG
    printf( "NDEF - write:\n" );
#endif
    
#ifdef NDEF_DEBUG_DUMP
    dump( (char *)ndefContent, len );
#endif
    
    // Write the tag changes
    unsigned char MifareResp[255];
    unsigned char MifareReadCmd[2] = {0x30U, 220};  // Address has 4 bytes resolution
    unsigned char MifareWriteCmd[6] = {0xA2U,  /*block*/ 0x00, /*data*/ 0x11, 0x22, 0x33, 0x44};
    int i, num = 4;
    for ( i=0; i<((len+3)/4) && (num>0); i++ ) {
        if ( memcmp( &ndefCache[i*4], &ndefContent[i*4], 4 ) != 0 ) {
            printf( "Block %d differs\n", i );
            MifareWriteCmd[1] = i + NDEF_OFFSET;
            memcpy( &MifareWriteCmd[2], &ndefContent[i*4], 4 );
            num = nfcTag_transceive(handle, MifareWriteCmd, sizeof(MifareWriteCmd), 
                                            MifareResp, sizeof(MifareResp), 500);
        }
    }
    
    if ( num > 0 ) {
        printf("Write Tag OK\n");

        // In order to trigger the node, we have to read from the last block 55 (16 bytes) in 
        // the (1kByte) NTAG-I2C. 55*16 = byte 880. With 4 bytes resolution we read at addr 220
        MifareReadCmd[1] = 220;  // Address has 4 bytes resolution

        num = nfcTag_transceive(handle, MifareReadCmd, 2, MifareResp, 16, 200);
        if ( num > 0 ) {
#ifdef NDEF_DEBUG
            printf("\n\t\tMifare Read command sent %d bytes Response : \n", num);
            dump( (char *)MifareResp, num );
#endif
            res = 1;
        }
    } else {
        printf( "Num = %d\n", num );
    }
    
    if ( num == 0 ) {
        printf("Ndef: Write Tag Failed\n");
    }
    return res;
}
