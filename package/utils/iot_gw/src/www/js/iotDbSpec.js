// ---------------------------------------------------------------
// IOT DATABASE SPEC
// ---------------------------------------------------------------


var COLUMN_DEVS_ID           = 0;
var COLUMN_DEVS_MAC          = COLUMN_DEVS_ID + 1;
var COLUMN_DEVS_DEV          = COLUMN_DEVS_MAC + 1;
var COLUMN_DEVS_TY           = COLUMN_DEVS_DEV + 1;
var COLUMN_DEVS_PARENT       = COLUMN_DEVS_TY + 1;
var COLUMN_DEVS_NM           = COLUMN_DEVS_PARENT + 1;
var COLUMN_DEVS_HEAT         = COLUMN_DEVS_NM + 1;
var COLUMN_DEVS_COOL         = COLUMN_DEVS_HEAT + 1;
var COLUMN_DEVS_TMP          = COLUMN_DEVS_COOL + 1;
var COLUMN_DEVS_HUM          = COLUMN_DEVS_TMP + 1;
var COLUMN_DEVS_PRS          = COLUMN_DEVS_HUM + 1;
var COLUMN_DEVS_CO2          = COLUMN_DEVS_PRS + 1;
var COLUMN_DEVS_BAT          = COLUMN_DEVS_CO2 + 1;
var COLUMN_DEVS_BATL         = COLUMN_DEVS_BAT + 1;
var COLUMN_DEVS_ALS          = COLUMN_DEVS_BATL + 1;
var COLUMN_DEVS_XLOC         = COLUMN_DEVS_ALS + 1;
var COLUMN_DEVS_YLOC         = COLUMN_DEVS_XLOC + 1;
var COLUMN_DEVS_ZLOC         = COLUMN_DEVS_YLOC + 1;
var COLUMN_DEVS_SID          = COLUMN_DEVS_ZLOC + 1;
var COLUMN_DEVS_CMD          = COLUMN_DEVS_SID + 1;
var COLUMN_DEVS_LVL          = COLUMN_DEVS_CMD + 1;
var COLUMN_DEVS_RGB          = COLUMN_DEVS_LVL + 1;
var COLUMN_DEVS_KELVIN       = COLUMN_DEVS_RGB + 1;
var COLUMN_DEVS_ACT          = COLUMN_DEVS_KELVIN + 1;
var COLUMN_DEVS_SUM          = COLUMN_DEVS_ACT + 1;
var COLUMN_DEVS_FLAGS        = COLUMN_DEVS_SUM + 1;
var COLUMN_DEVS_LASTUPDATE   = COLUMN_DEVS_FLAGS + 1;

var DEVICE_DEV_UNKNOWN       = 0;
var DEVICE_DEV_MANAGER       = 1;
var DEVICE_DEV_UI            = 2;
var DEVICE_DEV_SENSOR        = 3;
var DEVICE_DEV_UISENSOR      = 4;
var DEVICE_DEV_PUMP          = 5;
var DEVICE_DEV_LAMP          = 6;
var DEVICE_DEV_PLUG          = 7;
var DEVICE_DEV_SWITCH        = 8;

var TYPE_ONOFF     = "onoff";
var TYPE_DIM       = "dim";
var TYPE_COL       = "col";
var TYPE_TW        = "tw";

var TYPE_HEATING   = "h";
var TYPE_COOLING   = "c";

var FLAG_NONE                = 0x00;
var FLAG_DEV_JOINED          = 0x01;
var FLAG_UI_IGNORENEXT_HEAT  = 0x02;
var FLAG_LASTKELVIN          = 0x04;
var FLAG_UI_IGNORENEXT_COOL  = 0x20;

