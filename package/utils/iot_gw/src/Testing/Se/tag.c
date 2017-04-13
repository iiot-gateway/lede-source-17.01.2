// ------------------------------------------------------------------
// TAG layer driver
// ------------------------------------------------------------------
// Low level NFC tag handling layer
// Uses GPIOs to detect field and to power device
// Uses I2C to communicate to NFC memory
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>

#include "dump.h"
#include "tag.h"

// #define NTAG_DEBUG
#ifdef NTAG_DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...)
#endif /* NTAG_DEBUG */


#define TAG_I2C_MAX_RETRIES      4
#define TAG_ALWAYS_ON            1

#define USE_SRAM                 0
#define SRAM_FIXED_START         0xF8
#define SRAM_START               3
#define SRAM_END                 ( SRAM_START + 3 )

// Prototypes
static void print_session_registers( void );
static int tag_readblock( unsigned int block, char * pbuf );
static int tag_read_sessionreg( unsigned int reg, char * data );
static int tag_write_sessionreg( unsigned int reg, char mask, char data );

// ----------------------------------------------------------------------------
// Global variables
// ----------------------------------------------------------------------------

static int tagOn = 0;

static int iPwrFd = 0;
static int iInterruptFd = 0;
static int iI2CFd = 0;

// ----------------------------------------------------------------------------
// Tag (field) presence
// ----------------------------------------------------------------------------

int tag_present(void) {
    static int iTagPresent = 0;

    int len;
    char buf[2];

    if (iInterruptFd <= 0) {
        DEBUG_PRINTF( "Error with field-detect pin (%d)", iInterruptFd );
        return( 0 );
    }

    // Seek to the start of the file
    lseek(iInterruptFd, SEEK_SET, 0);

    // Read the field_detect line
    len = read(iInterruptFd, buf, 2);

    if (len != 2) {
        DEBUG_PRINTF( "Error with field-detect pin (%s)", strerror(errno));
        return( 0 );
    }

    if (buf[0] == '0') {
        if (!iTagPresent) {
            DEBUG_PRINTF( "Reader present\n" );
            iTagPresent = 1;
        }
    } else {
        if (iTagPresent) {
            DEBUG_PRINTF( "Reader removed\n" );
            iTagPresent = 0;
        }
    }
    return( iTagPresent );
}

// ----------------------------------------------------------------------------
// Power
// ----------------------------------------------------------------------------

void tag_power( int on ) {
  if ( on ) {
    
    if ( !tagOn ) {
      DEBUG_PRINTF( "*** TAG ON\r\n" );

      lseek(iPwrFd, SEEK_SET, 0);

      if (write(iPwrFd, "1", 1) != 1) {
          DEBUG_PRINTF( "Error powering up tag (%s)", strerror(errno));
      }

      // Give HW some time to settle
      usleep( 20 * 100 );    

      tagOn = 1;
  
#if USE_SRAM
      // Print session registers
      print_session_registers();

      // Enable the SRAM for communication
      tag_enable_sram();

      // Print session registers
      print_session_registers();
#endif
    }
    
  } else {
    DEBUG_PRINTF( "*** TAG OFF\r\n" );
    
      lseek(iPwrFd, SEEK_SET, 0);

      if (write(iPwrFd, "0", 1) != 1) {
          DEBUG_PRINTF( "Error powering down tag (%s)", strerror(errno));
      }

    // Give HW some time to settle
    usleep( 20 * 100 );    
    
    tagOn = 0;
  } 
}

// ----------------------------------------------------------------------------
// Power tag and lock further access, free access when disabled
// ----------------------------------------------------------------------------

static void tag_enable( int on ) {
  if ( on ) {
   tag_power( 1 );
  } else {
#ifndef TAG_ALWAYS_ON
    tag_power( 0 );
#endif
  }
}

static void tag_assert( void ) {
  DEBUG_PRINTF( "** TAG ASSERT **\r\n" );
  tag_enable( 0 );
  usleep( 10 * 1000 );
  tag_enable( 1 );
}

static void print_header( void ) {
  char buf[TAG_BLOCK_SIZE];

  if ( tag_readblock( 0, buf ) ) {
#ifdef NTAG_DEBUG
    DEBUG_PRINTF( "Tag header:\r\n" );
    DEBUG_PRINTF( "\t- I2C addr = 0x%x\r\n", buf[0] );
    DEBUG_PRINTF( "\t- Serial#  = " ); dump( buf+1, 6 );
    DEBUG_PRINTF( "\t- Internal = " ); dump( buf+7, 3 );
    DEBUG_PRINTF( "\t- Lock     = " ); dump( buf+10, 2 );
    DEBUG_PRINTF( "\t- Capabil. = " ); dump( buf+12, 4 );
#endif
  }
}

const char * config_reg_names[] = {
  "NC reg", "Last NDEF page", "SM reg", "WDT LS", "WDT MS", "I2C Clk stretch", "Reg lock" };
const char * session_reg_names[] = {
  "NC reg", "Last NDEF page", "SM reg", "WDT LS", "WDT MS", "I2C Clk stretch", "NS reg" };

static void print_config_registers( void ) {
  char buf[TAG_BLOCK_SIZE];
  int i;
  if ( tag_readblock( 0x7A, buf ) ) {
    DEBUG_PRINTF( "Config:\r\n" );
    for ( i=0; i<7; i++ ) {
      DEBUG_PRINTF( "\t- %16s = 0x%x\r\n", config_reg_names[i], buf[i] & 0xFF );
    }
  }
}

static void print_session_registers( void ) {
  // Print session registers
  char reg;
  int i;
  DEBUG_PRINTF( "Session:\r\n" ); 
  for ( i=0; i<7; i++ ) {
    tag_read_sessionreg( i, &reg );
    DEBUG_PRINTF( "\t- %16s = 0x%x\r\n", session_reg_names[i], reg & 0xFF );
  }
}

// ----------------------------------------------------------------------------
// Session register access
// ----------------------------------------------------------------------------

static int tag_read_sessionreg( unsigned int reg, char * data ) {
  if ( reg >= TAG_BLOCK_SIZE ) return( 0 );
  int retries = TAG_I2C_MAX_RETRIES;
  char buf[2];
  buf[0] = NTAG_SESSION_REGISTERS;
  buf[1] = reg;
  while ( retries >= 0 ) {

    int len = write(iI2CFd, buf, 2);
    if (len == 2) {
      len = read(iI2CFd, data, 1);
      if (len == 1) {
        // DEBUG_PRINTF("NTAG Session register %02d: 0x%02X\n", reg, *data);
        return( 1 );
      }
    }

    retries--;
    usleep( 1 * 100 );    
  }

  tag_assert();
  *data = 0;
  return( 0 );
}

static int tag_write_sessionreg( unsigned int reg, char mask, char data ) {
  if ( reg >= TAG_BLOCK_SIZE ) return( 0 );
  int retries = TAG_I2C_MAX_RETRIES;
  char buf[4];
  buf[0] = NTAG_SESSION_REGISTERS;
  buf[1] = reg;
  buf[2] = mask;
  buf[3] = data;

  while ( retries >= 0 ) {

    int len = write(iI2CFd, buf, 4);
    if ( len == 4) {
      DEBUG_PRINTF("Wrote NTAG Session register %02d : %02x %02x\n",
                    reg, mask, data);
      return( 1 );
    }
 
    retries--;
    usleep( 2 * 100 );    
  }

  tag_assert();
  return( 0 );
}

// ----------------------------------------------------------------------------
// Block level access
// ----------------------------------------------------------------------------

static int tag_readblock( unsigned int block, char * pbuf ) {
  int retries = TAG_I2C_MAX_RETRIES;
#if USE_SRAM
  if ( block >= SRAM_START && block <= SRAM_END ) {
    // I2C must always access the fixed SRAM address
    block = block - SRAM_START + SRAM_FIXED_START;
    // DEBUG_PRINTF( "==> Read SRAM block %d\r\n", block );
  }
#endif
  char buf[2];
  buf[0] = block;

  while ( retries >= 0 ) {

    int len = write(iI2CFd, buf, 1);
    if (len == 1) {
      len = read(iI2CFd, pbuf, TAG_BLOCK_SIZE);
      if (len == TAG_BLOCK_SIZE) {
#ifdef NTAG_DEBUG
        DEBUG_PRINTF("Block read: "); dump( pbuf, TAG_BLOCK_SIZE );
#endif
        return( 1 );
      }
    }

    retries--;
    usleep( 2 * 100 );    
  }

  tag_assert();
  DEBUG_PRINTF( ">>> Tag read block %d error\r\n", block );
  return( 0 );
}

static int tag_writeblock( unsigned int block, char * pbuf ) {
  int retries = TAG_I2C_MAX_RETRIES;
  int ret = 0;

  // DEBUG_PRINTF( ">>> Tag write block %d: ", block );
  // dump( pbuf, TAG_BLOCK_SIZE );

#if USE_SRAM
  int writing_sram = 0;
  if ( block >= SRAM_START && block <= SRAM_END ) {
    // I2C must always access the fixed SRAM address
    block = block - SRAM_START + SRAM_FIXED_START;
    // DEBUG_PRINTF( "==> Write SRAM block %d\r\n", block );
    writing_sram = 1;
    tag_write_sessionreg( NC_REG, NC_REG_PHTRU_DIR, ~NC_REG_PHTRU_DIR );
  }
#endif

  DEBUG_PRINTF( "==> Write block %d\r\n", block );

//#if USE_SRAM
//  if ( block >= SRAM_START && block <= SRAM_END ) {
//    DEBUG_PRINTF( "==> Ram block %d\r\n", block );
////    tag_write_sessionreg( NC_REG, NC_REG_PHTRU_DIR, ~NC_REG_PHTRU_DIR );
//  }
//#endif
  
  char buf[18];
  buf[0] = block;
  memcpy( &buf[1], pbuf, TAG_BLOCK_SIZE );

  while ( retries >= 0 && ret == 0 ) {

    int len = write(iI2CFd, buf, 17);
    if ( len == 17) {
#ifdef NTAG_DEBUG
      DEBUG_PRINTF("Wrote block %d: ", block ); dump( pbuf, TAG_BLOCK_SIZE );
#endif
      ret = 1;
    }
 
    retries--;
    usleep( 2 * 100 );    
  }

  if ( retries < 0 ) {
    tag_assert();
    DEBUG_PRINTF( ">>> Tag write block %d error\r\n", block );
    ret = 0;
  }

#if USE_SRAM
  if ( writing_sram ) {
    tag_write_sessionreg( NC_REG, NC_REG_PHTRU_DIR, NC_REG_PHTRU_DIR );
  } else {
    usleep( 5 * 100 );    
  }
#else
  usleep( 5 * 100 );    
#endif
  
  return( ret );
}

// ============================================================================
// GLOBAL functions
// ============================================================================

// ----------------------------------------------------------------------------
// User memory
// ----------------------------------------------------------------------------

int tag_get_userdata( unsigned int start, unsigned int len, char * pbuf ) {
  // DBG_vPrintf(TRUE, "Get userdata (%d)\r\n", len );
  tag_enable( 1 );
  int end = start + len;
  int ok = 1;
  char buf[TAG_BLOCK_SIZE];
  while ( ok && ( start < end ) ) {
    int block = ( start / TAG_BLOCK_SIZE ) + 1;   // User data starts at block 1
    int index = start % TAG_BLOCK_SIZE;
    if ( ( ok = tag_readblock( block, buf ) ) ) {
      while ( ( index < TAG_BLOCK_SIZE ) && ( start < end ) ) {
          *pbuf++ = buf[index++];
          start++;
      }
    }
  }
  tag_enable( 0 );
  return( ok );
}

int tag_set_userdata( unsigned int start, unsigned int len, char * pbuf ) {
  // DEBUG_PRINTF( "Set userdata:\r\n" );
  // dump( pbuf, len );
  tag_enable( 1 );
  int end = start + len;
  int ok = 1;
  int writing_done = 0;

//#if USE_SRAM
//  int sram_block_written = 0;
//  int sram_terminator_block_written = 0;
//#endif

  char buf[TAG_BLOCK_SIZE];
  while ( ok && ( start < end ) ) {
    int block = ( start / TAG_BLOCK_SIZE ) + 1;   // User data starts at block 1
    int index = start % TAG_BLOCK_SIZE;

    // DEBUG_PRINTF( ">>> start=%d, end=%d, block=%d, index=%d\r\n", start, end, block, index );
      
    // Read block first
    if ( ( ok = tag_readblock( block, buf ) ) ) {

      // printf( "Check                  block %d :", block );
      // dump( buf, TAG_BLOCK_SIZE );

      // Modify
      int changed = 0;
      while ( index < TAG_BLOCK_SIZE && ( start < end ) ) {
        if ( !changed && ( buf[index] != *(pbuf) ) ) {
          changed = 1;
          // printf( "Byte %d differs %x != %x\n", index, buf[index], *(pbuf) );
        }
        buf[index++] = *(pbuf++);
        start++;
      }

#if USE_SRAM
      // See section 10.3.3: When updating SRAM, we must also make sure to write to the
      // SRAM terminator page to trigger the RF
      if ( block >= SRAM_START && block <= SRAM_END ) changed = 1;
//      if ( block >= SRAM_START && block <= SRAM_END ) sram_block_written = 1;
//      if ( block == SRAM_END ) {
//        changed = 1;
//        sram_terminator_block_written = 1;
//      }
#endif

      // When real changes are done then write (prevents unnecessary writes)
      // printf( "Changed = %d\n", changed );
      if ( changed ) {
        if ( ( ok = tag_writeblock( block, buf ) ) ) {
            writing_done = 1;
            printf( "Real change written to block %d :", block );
            dump( buf, TAG_BLOCK_SIZE );
        }
      }
    }
  }

//#if USE_SRAM
//  // Write the terminator block if not yet done
//  if ( sram_block_written && !sram_terminator_block_written ) {
//    if ( ok = tag_readblock( SRAM_END, buf ) ) {
//      ok = tag_writeblock( SRAM_END, buf );
//    }
//  }
//#endif
  
  tag_enable( 0 );

  if ( !ok ) return( -1 );
  return( writing_done );
}

int tag_get_ns_reg( void ) {
  char regdata = 0;
  tag_enable( 1 );
  if ( !tag_read_sessionreg( NS_REG, &regdata ) ) {
    regdata = 0xFF;
  }
  tag_enable( 0 );
  return regdata;
}

void tag_check_link( void ) {
  char regdata = 0;
  tag_enable( 1 );
#if USE_SRAM
  if ( tag_read_sessionreg( SM_REG, &regdata ) ) {
    if ( regdata != SRAM_START ) {
#else
  if ( tag_read_sessionreg( WDT_LS, &regdata ) ) {
    if ( !regdata ) {
#endif
      DEBUG_PRINTF( "\n\n\n>>> SESSION REGISTERS CORRUPT <<<<<<<<<<<<<<<<<<<<<<<<<\r\n" );
      
      print_config_registers();
      print_session_registers();

#if 1
      tag_power( 0 );
#else
      char buf[TAG_BLOCK_SIZE];
      if ( tag_readblock( 0x7A, buf ) ) {
        // Restore the (volatile) session registers
        for ( int i=0; i<5; i++ ) {
          tag_write_sessionreg( i, 0xFF, buf[i] );
        }
        tag_write_sessionreg( I2C_CLOCK_STR, 0x01, buf[I2C_CLOCK_STR] );
#if USE_SRAM
        tag_enable_sram();
#endif
      }
      print_session_registers();
#endif
    }
  }
  tag_enable( 0 );
}

// ----------------------------------------------------------------------------
// Dump tag contents
// ----------------------------------------------------------------------------

void tag_dump_blocks( unsigned int block, unsigned int num ) {
  tag_enable( 1 );

  char buf[TAG_BLOCK_SIZE];
  int i;
  for ( i=block; i<block+num; i++ ) {
    if ( tag_readblock( i, buf ) ) {
// #ifdef NTAG_DEBUG
      DEBUG_PRINTF( "%2x: ", i );
      dump( buf, TAG_BLOCK_SIZE );
// #endif
    }
  }

  tag_enable( 0 );
}

void tag_dump_header( void ) {
  tag_enable( 1 );
  print_header();
  tag_enable( 0 );
}

void tag_dump_config( void ) {
  tag_enable( 1 );
  print_config_registers();
  tag_enable( 0 );
}

void tag_dump_session( void ) {
  tag_enable( 1 );
  print_session_registers();
  tag_enable( 0 );
}

char sram[64];

void tag_dump_sram( void ) {
  tag_enable( 1 );

  char buf[TAG_BLOCK_SIZE];

  int i, j, block, changed;

  for ( i=0; i<4; i++ ) {
    changed = 0;
    block = SRAM_FIXED_START + i;
    if ( tag_readblock( block, buf ) ) {
      for ( j=0; j<TAG_BLOCK_SIZE; j++ ) {
        if ( sram[(TAG_BLOCK_SIZE * i) + j] != buf[j] ) {
          sram[(TAG_BLOCK_SIZE * i) + j] = buf[j];
          changed = 1;
        }
      }
      if ( changed ) {
#ifdef NTAG_DEBUG
        DEBUG_PRINTF( "%2x: ", block );
        dump( buf, TAG_BLOCK_SIZE );
#endif
      }
    }
  }

  tag_enable( 0 );
}

// ----------------------------------------------------------------------------
// Config
// ----------------------------------------------------------------------------

void tag_set_config( int fd_off, int fd_on, int lastndef, int watchdog ) {
    char buf[TAG_BLOCK_SIZE];
    char buf2[TAG_BLOCK_SIZE];

    memcpy( buf2, buf, TAG_BLOCK_SIZE );

    int nc_reg = ( fd_off << 4 ) + ( fd_on << 2 );   // SRAM off

    // Write config registers
    if ( tag_readblock( NTAG_CONFIG_REGISTERS, buf ) ) {
        buf[NC_REG]         = nc_reg;
        buf[LAST_NDEF_PAGE] = lastndef;
        buf[SM_REG]         = SRAM_FIXED_START;
        buf[WDT_LS]         = watchdog & 0xFF;
        buf[WDT_MS]         = ( watchdog >> 8 ) & 0xFF;
        buf[I2C_CLOCK_STR]  = 1;  // Enabled
        if ( !memcmp( buf2, buf, TAG_BLOCK_SIZE ) ) {
            tag_writeblock( NTAG_CONFIG_REGISTERS, buf );
        }
    }

    // Also set this session right
    tag_write_sessionreg( NC_REG,         0xFF, nc_reg);
    tag_write_sessionreg( LAST_NDEF_PAGE, 0xFF, lastndef );
    tag_write_sessionreg( SM_REG,         0xFF, SRAM_FIXED_START );
    tag_write_sessionreg( WDT_LS,         0xFF, watchdog & 0xFF );
    tag_write_sessionreg( WDT_MS,         0xFF, ( watchdog >> 8 ) & 0xFF );
    tag_write_sessionreg( I2C_CLOCK_STR,  0xFF, 1 );
}

// ----------------------------------------------------------------------------
// SRAM Enable
// ----------------------------------------------------------------------------

void tag_enable_sram( void ) {
  // Set the (volatile) session registers
  tag_write_sessionreg( SM_REG, 0xFF, SRAM_START );
  int mask = NC_REG_SRAM_MIRROR_ON_OFF;
  tag_write_sessionreg( NC_REG, mask, NC_REG_SRAM_MIRROR_ON_OFF );
}

// ----------------------------------------------------------------------------
// END BLOCK
// ----------------------------------------------------------------------------

void tag_set_lastndef( int lastndef ) {
    char buf[TAG_BLOCK_SIZE];

    if ( tag_write_sessionreg( LAST_NDEF_PAGE, 0xFF, lastndef ) ) {
        // Write config registers
        if ( tag_readblock( NTAG_CONFIG_REGISTERS, buf ) ) {
            buf[LAST_NDEF_PAGE] = lastndef;
            tag_writeblock( NTAG_CONFIG_REGISTERS, buf );
        }
    }
}

// ----------------------------------------------------------------------------
// Init
// ----------------------------------------------------------------------------

int tag_init( char * powerPort, char * fielddetectPort,
              char * i2cBus, int i2cAddress ) {

    // Power pin
    if ( powerPort ) {
        iPwrFd = open( powerPort , O_RDWR);
        if (iPwrFd < 0) {
            DEBUG_PRINTF( "Could not open power control signal '%s' (%s)", 
                 powerPort, strerror(errno));
            iPwrFd = 0;
            return( 0 );
        }
    }

    // Field detect pin
    iInterruptFd = open( fielddetectPort, O_RDONLY);
    if (iInterruptFd < 0) {
        DEBUG_PRINTF( "Could not open field detect interrupt signal '%s' (%s)",
             fielddetectPort, strerror(errno));
        close(iPwrFd);
        return( 0 );
    }

    // I2C bus
    iI2CFd = open( i2cBus, O_RDWR | O_NOCTTY);
    if (iI2CFd < 0) {
        DEBUG_PRINTF( "Could not open I2C bus '%s' (%s)", 
           i2cBus, strerror(errno));
        close(iPwrFd);
        close(iInterruptFd);
        return( 0 );
    }

    // I2C bus
    if (ioctl(iI2CFd, I2C_SLAVE, i2cAddress) < 0) {
        DEBUG_PRINTF( "Cannot select NTAG I2C address (%s)", strerror(errno));
        close(iPwrFd);
        close(iInterruptFd);
        close(iI2CFd);
        return( 0 );
    }

    tag_power( 0 );  

    return( 1 );
}
