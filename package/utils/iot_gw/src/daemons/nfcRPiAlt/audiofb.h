// ---------------------------------------------------------------------
// Audio Feedback Handler - include file
// ---------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

#define AFB_PATTERN_SILENT       0
#define AFB_PATTERN_ONELONG      1
#define AFB_PATTERN_TWOMEDIUM    2
#define AFB_PATTERN_SHORTLONG    3
#define AFB_PATTERN_THREESHORT   4
#define AFB_PATTERN_FOURTICKS    5

void afbInit( void );
void afbPlayPattern( int pattern );

