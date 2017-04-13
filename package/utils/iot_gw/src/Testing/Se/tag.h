// ------------------------------------------------------------------
// TAG layer driver - include file
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

#define TAG_SRAM_SIZE             64
#define TAG_BLOCK_SIZE            16

#define NTAG_CONFIG_REGISTERS    0x7A
#define NTAG_SESSION_REGISTERS   0xFE

#define NC_REG                    0 
#define LAST_NDEF_PAGE            1 
#define SM_REG                    2 
#define WDT_LS                    3 
#define WDT_MS                    4 
#define I2C_CLOCK_STR             5 
#define NS_REG                    6 
#define ZEROES                    7 

#define NC_REG_I2C_RST_ON_OFF     ( 1 << 7 )
#define NC_REG_PHTRU_ON_OFF       ( 1 << 6 )
#define NC_REG_FD_OFF             ( 1 << 5 )
#define NC_REG_FD_ON              ( 1 << 3 )
#define NC_REG_SRAM_MIRROR_ON_OFF ( 1 << 1 )
#define NC_REG_PHTRU_DIR          ( 1 << 0 )

#define NS_REG_NDEF_DATA_READ     ( 1 << 7 )
#define NS_REG_I2C_LOCKED         ( 1 << 6 )
#define NS_REG_RF_LOCKED          ( 1 << 5 )
#define NS_REG_SRAM_I2C_READY     ( 1 << 4 )
#define NS_REG_SRAM_RF_READY      ( 1 << 3 )
#define NS_REG_EEPROM_WR_ERR      ( 1 << 2 )
#define NS_REG_EEPROM_WR_BUSY     ( 1 << 1 )
#define NS_REG_RF_FIELD_PRESENT   ( 1 << 0 )


void tag_power( int on );
int  tag_present( void );
int  tag_init( char * p, char * f, char * i, int a );

char tag_get_userbyte( unsigned int bytenumber );
int  tag_set_userbyte( unsigned int bytenumber, char data );

int tag_get_userdata( unsigned int start, unsigned int len, char * pbuf );
int tag_set_userdata( unsigned int start, unsigned int len, char * pbuf );

int tag_get_ns_reg( void );
void tag_check_link( void );

void tag_dump_blocks( unsigned int block, unsigned int num );
void tag_dump_header( void );
void tag_dump_config( void );
void tag_dump_session( void );
void tag_dump_sram( void );

void tag_enable_sram( void );
void tag_set_config( int fd_off, int fd_on, int lastndef, int watchdog );

