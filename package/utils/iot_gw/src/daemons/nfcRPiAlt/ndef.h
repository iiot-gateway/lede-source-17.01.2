// ---------------------------------------------------------------------
// NDEF layer - include file
// ---------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ------------------------------------------------------------------

#define NDEF_MAX_SIZE     2048
#define NDEF_MAX_PAYLOAD  2048
#define NDEF_MAX_URI      40

int ndef_read( int handle, char * payload, int maxpayload, char * uri, int maxuri );
int ndef_write( int handle, char * payload, int payload_len, char * uri, int uri_len );


