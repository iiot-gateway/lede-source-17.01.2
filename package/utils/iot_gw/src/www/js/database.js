// ------------------------------------------------------------------
// CGI program - Database
// ------------------------------------------------------------------
// Javascript for the IoT Database webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

var  COMMAND_NONE               = 0;
var  COMMAND_TSCHECK            = 1;
var  COMMAND_RESERVED_1         = 3;
var  COMMAND_CLEAR_LAMPS        = 4;
var  COMMAND_CLEAR_PLUGS        = 5;
var  COMMAND_CLEAR_DEVICES      = 6;
var  COMMAND_CLEAR_SYSTEM       = 7;
var  COMMAND_CLEAR_ZCB          = 8;
var  COMMAND_CLEAR_PLUGHISTORY  = 9;
var  COMMAND_GET_TABLE          = 10;

var  timerVar   = 0;
var  ts         = 0;

var  NUM_TABLES = 4;

var  DEVICETABLE       = 0;
var  SYSTEMTABLE       = 1;
var  ZCBTABLE          = 2;
var  PLUGHISTORYTABLE  = 3;

var  tables = ["iot_devs", "iot_system",
               "iot_zcb", "iot_plughistory" ];
var  lus;

// ------------------------------------------------------------------
// On page load
// ------------------------------------------------------------------

$(document).ready(function() {
    $('#javaerror').hide();
    timerRestart();
    lus = new Array();
    for ( var i=0; i<NUM_TABLES; i++ ) lus[i] = -1;
    DbUpdate();
});

// ------------------------------------------------------------------
// Timer
// ------------------------------------------------------------------

function timerRestart() {
    if ( timerVar ) clearInterval( timerVar );
    // Avoid strict 2-sec pace
    var period = 1950 + Math.floor( Math.random() * 100 );
    timerVar = setInterval( DbUpdate, period );
}

// ------------------------------------------------------------------
// HTTP / XML
// ------------------------------------------------------------------

function getXmlVal( xmlDoc, tag ) {
    var v = -1;
    if ( xmlDoc ) {
        var x = xmlDoc.getElementsByTagName( tag );
        if ( x.length > 0 ) {
            v = x[0].childNodes[0].nodeValue;
        }
    }
    return( v );
}

function doHttp( posts, onReady ) {
    var xmlhttp;
    if ( window.XMLHttpRequest ) {
        // IE7+, Firefox, Chrome, Opera, Safari
        xmlhttp = new XMLHttpRequest();
    } else {
        // IE6, IE5
        xmlhttp = new ActiveXObject('Microsoft.XMLHTTP');
    }
    xmlhttp.onreadystatechange=function() {
        if ( xmlhttp.readyState==4 && xmlhttp.status==200 ) {
            onReady( xmlhttp );
        }
    }
    xmlhttp.open( 'POST', 'iot_database.cgi', true );
    xmlhttp.setRequestHeader( 'Content-type',
                              'application/x-www-form-urlencoded' );
    xmlhttp.send( posts );
}

// ------------------------------------------------------------------
// Time helper
// ------------------------------------------------------------------

function timeConv( ts ) {
    var days   = [ 'Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat' ];
    var months = [ 'Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
                   'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec' ];   
    var a     = new Date( ts * 1000 );
    var day   = days[ a.getDay() ];
    var month = months[ a.getMonth() ];
    var date  = a.getDate();
    var hour  = a.getHours();
    var min   = a.getMinutes();
    var sec   = a.getSeconds();
    var year  = a.getFullYear();
    if ( min < 10 ) min = '0' + min;
    if ( sec < 10 ) sec = '0' + sec;
    var time  = day+' '+month+' '+date+' '+hour+':'+min+':'+sec+' '+year;

    return time;
}

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------

function rgbi2rgbs( rgb ) {
    var r = ( rgb >> 16 ) & 0xFF;
    var g = ( rgb >> 8  ) & 0xFF;
    var b = ( rgb       ) & 0xFF;
    var rstr = r.toString(16); if ( rstr.length < 2 ) rstr = "0"+rstr;
    var gstr = g.toString(16); if ( gstr.length < 2 ) gstr = "0"+gstr;
    var bstr = b.toString(16); if ( bstr.length < 2 ) bstr = "0"+bstr;
    var rgbs = rstr + gstr + bstr;
    return rgbs.toUpperCase();
}

// ------------------------------------------------------------------
// Table printing
// ------------------------------------------------------------------

function htmlTable( table, lastistimestamp, intsarehex ) {
    var html = "<table border=1>";
    var macCol = -1;

    var rows = table.split( ";" );
    for ( var r=0; r<rows.length; r++ ) {
        html += "<tr>";
        var cols = rows[r].split( "," );
        for ( var c=0; c<cols.length; c++ ) {
            var num = parseInt( cols[c] );
            // console.log( "================= " + cols[c] + " -> " + num );
            if ( r==0 ) {
                html += "<td class='head'>"+cols[c]+"</td>";
                if ( cols[c] == 'mac' ) macCol = c;
            } else if ( cols[c] == 'NULL' ) {
                html += "<td>-</td>";
            } else if ( lastistimestamp && c==(cols.length-1) ) {
                html += "<td>"+timeConv(cols[c])+"</td>";
            } else if ( intsarehex && (c!=macCol) ) {
                html += "<td>"+num.toString(16)+"</td>";
            } else {
                html += "<td>"+cols[c]+"</td>";
            }
        }
        html += "</tr>";
    }
    html += "</table>";

    return( html );
}

function htmlDevTable( table, lastistimestamp ) {
    var html = "<table border=1>";

    var rows = table.split( ";" );
    for ( var r=0; r<rows.length; r++ ) {
        html += "<tr>";
        var cols = rows[r].split( "," );
        for ( var c=0; c<cols.length; c++ ) {
            if ( r==0 ) {
                html += "<td class='head'>"+cols[c]+"</td>";
            } else if ( c == COLUMN_DEVS_DEV ) {
                var col = "red";
                var dev = cols[c];
                switch ( parseInt( cols[c] ) ) {
                case DEVICE_DEV_MANAGER:
                    col = "palevioletred";
                    dev = "man";
                    break;
                case DEVICE_DEV_UI:
                    col = "mediumseagreen";
                    dev = "ui";
                    break;
                case DEVICE_DEV_SENSOR:
                    col = "violet";
                    dev = "sen";
                    break;
                case DEVICE_DEV_UISENSOR:
                    col = "lightgreen";
                    dev = "uisen";
                    break;
                case DEVICE_DEV_PUMP:
                    col = "lightblue";
                    dev = "pump";
                    break;
                case DEVICE_DEV_LAMP:
                    col = "yellow";
                    dev = "lamp";
                    break;
                case DEVICE_DEV_PLUG:
                    col = "darksalmon";
                    dev = "plug";
                    break;
                case DEVICE_DEV_SWITCH:
                    col = "aqua";
                    dev = "switch";
                    break;
                }
                html += "<td style='background:"+col+"'>"+dev+"</td>";
            } else if ( c == COLUMN_DEVS_RGB ) {
                html += "<td>" + rgbi2rgbs( cols[c] ) + "</td>";
            } else if ( cols[c] == 'NULL' ) {
                html += "<td>-</td>";
            } else if ( lastistimestamp && c==(cols.length-1) ) {
                html += "<td>"+timeConv(cols[c])+"</td>";
            } else {
                html += "<td>"+cols[c]+"</td>";
            }
        }
        html += "</tr>";
    }
    html += "</table>";

    return( html );
}

// ------------------------------------------------------------------
// Error handling
// ------------------------------------------------------------------

function printError( err ) {
    console.log('err='+err );
    var errText = "Error ";
    switch ( parseInt( err ) ) {
    case 1:
        errText += "Problem opening database";
        break;
    case 2:
        errText += "Problem opening database 2";
        break;
    default:
        errText += err;
        break;
    }
    document.getElementById('debug').innerHTML = errText;
}

function clearError() {
    document.getElementById('debug').innerHTML = "";
}

// ------------------------------------------------------------------
// Execute
// ------------------------------------------------------------------

function DbExec( post ) {
    console.log('DbExec : '+post);
    doHttp( post,
        function( xmlhttp ) {
            var xmlDoc = xmlhttp.responseXML;
            var err = getXmlVal( xmlDoc, 'err' );
            if ( err == 0 ) {
                var cmd = getXmlVal( xmlDoc, 'cmd' );

                for ( i=0; i<10; i++ ) {
                    var dbg = getXmlVal( xmlDoc, 'dbg'+i );
                    if ( dbg != -1 ) console.log('dbg'+i+'='+dbg );
                }
                
                switch ( parseInt( cmd ) ) {
                case COMMAND_TSCHECK:
                    var clk = getXmlVal( xmlDoc, 'clk' );
                    var pid = getXmlVal( xmlDoc, 'pid' );
                    document.getElementById('headerDate').innerHTML =
                           timeConv( clk ) + " (" + pid + ")";

                    var lusStr = getXmlVal( xmlDoc, 'lus' );
                    console.log('lusStr='+lusStr );
                    var lus2 = lusStr.split( "," );

                    for ( var i=0; i<NUM_TABLES; i++ ) {
                        if ( lus[i] != lus2[i] ) {
                            lus[i] = lus2[i];
                            GetTable( i );
                        }
                    }

                    break;

                case COMMAND_GET_TABLE:
                    var tableno = getXmlVal( xmlDoc, 'tableno' );
                    var table = getXmlVal( xmlDoc, 'table' );
                    console.log('table='+table );
                    var html;
                    if ( table == "-" ) html = "Empty table";
                    else if ( table == "+" ) html = "Table too big to show";
                    else if ( tableno == ZCBTABLE ) html = htmlTable( table, 1, 1 );
                    else if ( tableno != DEVICETABLE ) html = htmlTable( table, 1, 0 );
                    else html = htmlDevTable( table, 1 );
                    document.getElementById('div_'+tables[tableno]).innerHTML =
                         html;
                    break;

                case COMMAND_CLEAR_LAMPS:
                case COMMAND_CLEAR_DEVICES:
                    GetTable( DEVICETABLE );
                    break;

                case COMMAND_CLEAR_SYSTEM:
                    GetTable( SYSTEMTABLE );
                    break;

                case COMMAND_CLEAR_PLUGS:
                    GetTable( DEVICETABLE );
                    break;

                case COMMAND_CLEAR_PLUGHISTORY:
                    GetTable( PLUGHISTORYTABLE );
                    break;

                default:
                    console.log('cmd='+cmd );
                    break;
                }
            } else {
                console.log('err='+err );
            }
        } );
}

// ------------------------------------------------------------------
// Commands
// ------------------------------------------------------------------

function DbUpdate() {
    // console.log('DbUpdate');
    var post = 'command='+COMMAND_TSCHECK;
    DbExec( post );
}

function DbCommand( c ) {
    console.log('DbCommand : '+c);
    var post = 'command='+c;
    DbExec( post );
}

function GetTable( i ) {
    console.log('GetTable : '+i);
    var post = 'command='+COMMAND_GET_TABLE+'&tableno='+i;
    DbExec( post );
}


