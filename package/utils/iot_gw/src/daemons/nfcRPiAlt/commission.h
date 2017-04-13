// ------------------------------------------------------------------
// Commissioning app - include file
// ------------------------------------------------------------------
// Contacts SecureJoiner to get SecJoin struct
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

#define COMMISSION_URI                "com.nxp:wise01"
#define COMMISSION_SUPPORTED_VERSION  11

// ------------------------------------------------------------------
// Device type
// ------------------------------------------------------------------

#define FROM_SETTINGSAPP           0x01
#define FROM_GATEWAY               0x02
#define FROM_MANAGER               0x03
#define FROM_ROUTER                0x04
#define FROM_UISENSOR              0x20
#define FROM_SENSOR                0x22
#define FROM_ACTUATOR              0x23
#define FROM_UI_ONLY               0x24
#define FROM_PLUGMETER             0x30
#define FROM_LAMP_ONOFF            0x40
#define FROM_LAMP_DIMM             0x41
#define FROM_LAMP_TW               0x42
#define FROM_LAMP_RGB              0x43

#define COMMAND_NONE               0
#define COMMAND_FACTORYNEW         1
#define COMMAND_JOIN               2
#define COMMAND_SECJOIN            3
#define COMMAND_LEAVE              4

// ------------------------------------------------------------------
// Payload
// ------------------------------------------------------------------

typedef struct commission_out_struct {
   /* From this device to NFC reader device */
   uint8_t   From;
   uint8_t   SeqNr;
   uint8_t   NodeExtra[69];
   uint8_t   NodeMacAddress[8];
   uint8_t   NodeLinkKey[16];
} __attribute__((__packed__)) commission_out_t;

   
typedef struct commission_in_struct {
   /* From NFC reader device to this device */
   uint8_t   From;
   uint8_t   SeqNr;
   uint8_t   Command;               // None=0, FactoryNew=1, Join=2, SecureJoin=3, Leave=4
   uint8_t   GatewayExtra[66];
   uint8_t   Channel;
   uint8_t   KeySeq;
   uint8_t   PanIdMSB;
   uint8_t   PanIdLSB;
   uint8_t   EncMic[12];            // bytes[12..15] also used for non ecrypted Mic
   uint8_t   Mic[4];                // bytes[12..15] also used for non ecrypted Mic
   uint8_t   EncKey[16];
   uint8_t   ExtPanId[8];
   uint8_t   TrustCenterAddress[8];
} __attribute__((__packed__)) commission_in_t;
   
typedef struct commission_struct {
   uint8_t          version;
   commission_out_t out;
   commission_in_t  in;
} __attribute__((__packed__)) commission_t;



#define LEN_MAC      8
#define LEN_LINKKEY  16

typedef struct linkinfo_s {
    uint8_t  version;
    uint8_t  type;
    uint16_t profile;
    char     mac[LEN_MAC];
    char     linkkey[LEN_LINKKEY];
} linkinfo_t;

#define LEN_EXTPAN   8
#define LEN_TCADDR   8
#define LEN_MIC      4
#define LEN_NWKEY    16

typedef struct secjoin_s {
    uint8_t  channel;
    uint8_t  keyseq;
    uint16_t pan;
    char     extpan[LEN_EXTPAN];
    char     nwkey[LEN_NWKEY];
    char     mic[LEN_MIC];
    char     tcaddr[LEN_TCADDR];
} secjoin_t;

int commissionHandleNfc( int handle, char * payload, int plen );
void commissionReset( void );
void commissionInit( void );

