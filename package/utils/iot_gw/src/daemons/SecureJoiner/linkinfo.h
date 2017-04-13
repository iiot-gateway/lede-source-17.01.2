// ------------------------------------------------------------------
// Link Info - include file
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

#define LEN_MAC_NIBBLE  16
#define LEN_LINKKEY     32

typedef struct linkinfo_s {
    int  version;
    int  type;
    int  profile;
    char mac[LEN_MAC_NIBBLE+2];
    char linkkey[LEN_LINKKEY+2];
} linkinfo_t;

void         linkinfoInit( void );
linkinfo_t * linkinfoHandle( void );

