// ------------------------------------------------------------------
// CGI program - System
// ------------------------------------------------------------------
// Javascript for the IoT System webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2015. All rights reserved
// ------------------------------------------------------------------

var  COMMAND_NONE                  = 0;
var  COMMAND_TSCHECK               = 1;
var  COMMAND_SYNC_TIME             = 2;
var  COMMAND_REBOOT                = 3;
var  COMMAND_SET_HOSTNAME          = 4;
var  COMMAND_SET_WIRELESS_SSID     = 5;
var  COMMAND_SET_WIRELESS_CHANNEL  = 6;
var  COMMAND_SET_TIMEZONE          = 7;
var  COMMAND_SET_ZONENAME          = 8;
var  COMMAND_NETWORK_RESTART       = 9;
var  COMMAND_SHUTDOWN              = 10;

var STATE_NORMAL           = 0;
var STATE_CONFIRM_SHUTDOWN = 1;
var STATE_SHUTTING_DOWN    = 2;
var STATE_SHUTDOWN_READY   = 3;
var STATE_CONFIRM_REBOOT   = 4;
var STATE_REBOOTING        = 5;
var STATE_REBOOT_READY     = 6;
var currentState = STATE_NORMAL;

var pollingSec = 5;
var timerVar   = 0;
var ts         = 0;

var first      = 1;
var hostname   = "hostname";
var tzIndex    = 0;
var ssid       = 0;
var channel    = 0;

// ------------------------------------------------------------------
// On page load
// ------------------------------------------------------------------

function iotOnLoad() {
    document.getElementById('javaerror').style.display = 'none';
    drawContent();
    timerRestart();
    SysUpdate();
};

// ------------------------------------------------------------------
// Timer
// ------------------------------------------------------------------

function timerRestart() {
    if ( timerVar ) clearInterval( timerVar );
    // Avoid strict x-sec pace
    var period = ( pollingSec * 1000 ) - 50 + Math.floor( Math.random() * 100 );
    timerVar = setInterval( SysUpdate, period );
}

// ------------------------------------------------------------------
// HTTP/XML
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
    xmlhttp.open( 'POST', 'iot_system.cgi', true );
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
// Print table
// ------------------------------------------------------------------

function htmlTable( table ) {
    var html = "<table border=1>";

    var rows = table.split( ";" );
    for ( var r=0; r<rows.length; r++ ) {
        html += "<tr>";
        var cols = rows[r].split( "|" );
        for ( var c=0; c<cols.length; c++ ) {
            var val = cols[c];
            if ( r==0 )            html += "<td class='head'>"+val+"</td>";
            else                   html += "<td>"+val+"</td>";
        }
        html += "</tr>";
    }
    html += "</table>";

    return( html );
}

// ------------------------------------------------------------------
// Parse system table
// ------------------------------------------------------------------

function parseSystemTable( table ) {
    var rows = table.split( ";" );
    for ( var r=0; r<rows.length; r++ ) {
        var cols = rows[r].split( "|" );
        if ( cols[0] == "Hostname" ) {
            hostname = cols[1];
        } else if ( cols[0] == "Timezone" ) {
            tzIndex = tzFindIndexTimezone( cols[1] );
        } else if ( cols[0] == "Zonename" ) {             // Zonename normally comes after Timezone
            tzIndex = tzFindIndexZonename( cols[1] );
        } else if ( cols[0] == "SSID" ) {
            ssid = cols[1];
        } else if ( cols[0] == "Channel" ) {
            channel = cols[1];
        }
    }
}

function updateEdits() {
    var html = "<table border=1>";
    html += "<tr><td>Hostname</td><td>";
    html += "<input type='text' id='i_hostname' value='" + hostname + "'/>";
    html += "<input type='submit' value='Set' onclick='setHostname();' />";
    html += "</td></tr>";
    html += "<tr><td>Timezone*</td><td>";
    html += tzSelector( tzIndex );
    html += "<input type='submit' value='Set' onclick='setTimezone();setZonename();' />";
    html += "</td></tr>";
    if ( ssid != 0 ) {
        html += "<tr><td>Wifi SSID**</td><td>";
        html += "<input type='text' id='i_ssid' value='" + ssid + "'/>";
        html += "<input type='submit' value='Set' onclick='setSSID();' />";
        html += "</td></tr>";
    }
    if ( channel != 0 ) {
        html += "<tr><td>Wifi Channel**</td><td>";
        html += "<input type='text' id='i_channel' value='" + channel + "'/>";
        html += "<input type='submit' value='Set' onclick='setChannel();' />";
        html += "</td></tr>";
    }
    html += "</table>";
    html += "<i>* Requires a reboot to become effective.</i><br />";
    if ( ssid != 0 || channel != 0 ) {
        html += "<i>** Requires a network restart to become effective. Your connection may get lost.</i><br />";
    }
    document.getElementById('div_edits').innerHTML = html;
}

// ------------------------------------------------------------------
// Error handling
// ------------------------------------------------------------------

function printError( err ) {
    console.log('err='+err );
    var errText = "Error " + err;
    document.getElementById('debug').innerHTML = errText;
}

function clearError() {
    document.getElementById('debug').innerHTML = "";
}

function hasIllegalChars( str ) {
    // Allowed are alphanumeric and underscore
    if( /[^a-zA-Z0-9_]/.test( str ) ) {
        return 1;
    }
    return 0;
}

function noNumber( str ) {
    // Allowed are numeric and +/- at beginning
    if ( str.substring(0,1) == "-" || str.substring(0,1) == "+" ) str = str.substring(1);
    if( /[^0-9]/.test( str ) ) {
        return 1;
    }
    return 0;
}

// ------------------------------------------------------------------
// Exec command
// ------------------------------------------------------------------

function SysExec( post ) {
    timerRestart();
    console.log('SysExec : '+post);
    doHttp( post,
        function( xmlhttp ) {
            var xmlDoc = xmlhttp.responseXML;
            var str = xml2Str (xmlDoc);
            console.log('xml='+str );
            var cmd = getXmlVal( xmlDoc, 'cmd' );
            var err = getXmlVal( xmlDoc, 'err' );

            for ( i=0; i<10; i++ ) {
                var dbg = getXmlVal( xmlDoc, 'dbg'+i );
                if ( dbg != -1 ) console.log('dbg='+dbg );
            }
            // if ( cmd == COMMAND_INSTALL_UPDATES ) timerRestart();
            if ( err == 0 ) {
                switch ( parseInt( cmd ) ) {
                case COMMAND_TSCHECK:
                    var clk = getXmlVal( xmlDoc, 'clk' );
                    var pid = getXmlVal( xmlDoc, 'pid' );
                    document.getElementById('headerDate').innerHTML =
                           timeConv( clk ) + " (" + pid + ")";
                    var sys = getXmlVal( xmlDoc, 'sys' );
                    console.log('sys='+sys );
                    
                    if ( first ) {
                        parseSystemTable( sys );
                        updateEdits();
                        first = 0;
                    }
                    
                    var html = htmlTable( sys );
                    document.getElementById('div_system').innerHTML = html;
                    break;

                case COMMAND_SET_HOSTNAME:
                case COMMAND_SET_WIRELESS_SSID:
                case COMMAND_SET_WIRELESS_CHANNEL:
                case COMMAND_SET_TIMEZONE:
                case COMMAND_SET_ZONENAME:
                    SysUpdate();
                    break;
                    
                case COMMAND_REBOOT:
                    console.log('Reboot procedure started');
                    var downtime;
                    downtime = parseInt( getXmlVal( xmlDoc, 'downtime' ) );  // in secs
                    setTimeout( gotoRebootReady, downtime * 1000 );
                    setTimeout( gotoNormal, ( downtime + 5 ) * 1000 );
                    break;
                    
                case COMMAND_SHUTDOWN:
                    console.log('Shutdown procedure started');
                    var downtime;
                    downtime = parseInt( getXmlVal( xmlDoc, 'downtime' ) );  // in secs
                    setTimeout( gotoShutdownReady, downtime * 1000 );
                    break;
                    
                default:
                    console.log('cmd='+cmd );
                    break;
                }
            } else {
                printError( err );
            }
        } );
}

// ------------------------------------------------------------------
// Update
// ------------------------------------------------------------------

function SysUpdate() {
    var d = new Date();
    var n = d.getTime() / 1000;
    document.getElementById('div_browsertime').innerHTML = timeConv( n );

    console.log('SysUpdate');
    var post = 'command='+COMMAND_TSCHECK;
    SysExec( post );
}

// ------------------------------------------------------------------
// Command
// ------------------------------------------------------------------

function SysCommand( cmd ) {
    console.log('SysCommand : '+cmd);
    clearError();
    var post = 0;
    switch ( parseInt( cmd ) ) {
        
    case COMMAND_SYNC_TIME:
        var d = new Date();
        var n = d.getTime() / 1000;
        post = 'command=' + cmd + "&browsertime=" + n;
        break;
        
    case COMMAND_SET_HOSTNAME:
        var hname = document.getElementById( 'i_hostname' ).value;
        if ( !hname || typeof hname == 'undefined' ) {
            printError( "No hostname" );
        } else if ( hname.length < 4 ) {
            printError( "Hostname too short" );
        } else if ( hname.length > 14 ) {
            printError( "Hostname too long" );
        } else if ( hasIllegalChars( hname ) ) {
            printError( "Hostname has illegal characters" );
        } else {
            hostname = hname;
            post = 'command=' + cmd + "&hostname=" + hostname;
        }
        break;
        
    case COMMAND_SET_WIRELESS_SSID:
        var ss = document.getElementById( 'i_ssid' ).value;
        if ( !ss || typeof ss == 'undefined' ) {
            printError( "No SSID" );
        } else if ( ss.length < 4 ) {
            printError( "SSID too short" );
        } else if ( ss.length > 14 ) {
            printError( "SSID too long" );
        } else if ( hasIllegalChars( ss ) ) {
            printError( "SSID has illegal characters" );
        } else {
            ssid = ss;
            post = 'command=' + cmd + "&ssid=" + ssid;
        }
        break;
        
    case COMMAND_SET_WIRELESS_CHANNEL:
        var ch = document.getElementById( 'i_channel' ).value;
        if ( !ch || typeof ch == 'undefined' ) {
            printError( "No channel" );
        } else if ( noNumber( ch ) ) {
            printError( "Channel is not a number" );
        } else if ( parseInt( ch ) < 1 || parseInt( ch ) > 14 ) {
            printError( "Channel number out of range [1..14]" );
        } else {
            channel = ch;
            post = 'command=' + cmd + "&channel=" + channel;
        }
        break;
        
    case COMMAND_SET_TIMEZONE:
        var tz = document.getElementById( 'tz_sel' ).value;
        post = 'command=' + cmd + "&timezone=" + tz;
        break;
        
    case COMMAND_SET_ZONENAME:
        var zn = tzGetZonename( document.getElementById( 'tz_sel' ).selectedIndex );
        post = 'command=' + cmd + "&zonename=" + zn;
        break;
                        
    case COMMAND_REBOOT:
        gotoRebootConfirm();
        break;

    case COMMAND_NETWORK_RESTART:
        post = 'command=' + cmd;
        break;
        
    case COMMAND_SHUTDOWN:
        gotoShutdownConfirm();
        break;
    }
    if ( post != 0 ) SysExec( post );
}

function setHostname() {
    SysCommand( COMMAND_SET_HOSTNAME );
}

function setSSID() {
    SysCommand( COMMAND_SET_WIRELESS_SSID );
}

function setChannel() {
    SysCommand( COMMAND_SET_WIRELESS_CHANNEL );
}

function setTimezone() {
    SysCommand( COMMAND_SET_TIMEZONE );
}

function setZonename() {
    SysCommand( COMMAND_SET_ZONENAME );
}

// ------------------------------------------------------------------
// Shutdown
// ------------------------------------------------------------------

function drawContent() {
    var html;

    switch (currentState) {
    case STATE_NORMAL:
        document.getElementById('div_normal').style.display = 'inline-block';
        document.getElementById('div_down').style.display = 'none';
        break;
    
    case STATE_CONFIRM_REBOOT:
        document.getElementById('div_normal').style.display = 'none';
        document.getElementById('div_down').style.display = 'inline-block';
        html = '<h1 align="center">Are you sure you want to reboot the system?</h1>';
        html += '<p align="center">';
        html += '<input type="submit" value="OK" onclick="gotoRebooting();" />';
        html += '<input type="submit" value="Cancel" onclick="gotoNormal();" />';
        html += '</p>';
        document.getElementById('div_down').innerHTML = html;
        break;
    
    case STATE_REBOOTING:
        html = '<h1 align="center">Rebooting</h1>';
        html += '<p align="center">';
        html += '<img src="/img/spinner.gif" width="100"/>';
        html += '</p>';
        document.getElementById('div_down').innerHTML = html;
        SysExec( "command="+COMMAND_REBOOT );
        break;
    
    case STATE_REBOOT_READY:
        html = '<h1 align="center">Device is ready to use again.</h1>';
        html += '<h2>Press F5 if this screen does not refresh itself within a few seconds</h2>';
        document.getElementById('div_down').innerHTML = html;
        SysExec( "command="+COMMAND_TSCHECK );
        break;

    case STATE_CONFIRM_SHUTDOWN:
        document.getElementById('div_normal').style.display = 'none';
        document.getElementById('div_down').style.display = 'inline-block';
        html = '<h1 align="center">Are you sure you want to shutdown the system?</h1>';
        html += '<p align="center">';
        html += '<input type="submit" value="OK" onclick="gotoShuttingDown();" />';
        html += '<input type="submit" value="Cancel" onclick="gotoNormal();" />';
        html += '</p>';
        document.getElementById('div_down').innerHTML = html;
        break;
    
    case STATE_SHUTTING_DOWN:
        html = '<h1 align="center">Shutting down</h1>';
        html += '<p align="center">';
        html += '<img src="/img/spinner.gif" width="100"/>';
        html += '</p>';
        document.getElementById('div_down').innerHTML = html;
        SysExec( "command="+COMMAND_SHUTDOWN );
        break;
    
    case STATE_SHUTDOWN_READY:
        html = '<h1 align="center">It is now safe to switch off the device</h1>';
        document.getElementById('div_down').innerHTML = html;
        break;
    }
}
        
function gotoNormal() {
    console.log('User selected cancel button');
    currentState = STATE_NORMAL;
    drawContent();
}

function gotoRebootConfirm() {
    console.log('User selected reboot button');
    currentState = STATE_CONFIRM_REBOOT;
    drawContent();
}

function gotoRebooting() {
    console.log('User selected yes button');
    currentState = STATE_REBOOTING;
    drawContent();
}

function gotoRebootReady() {
    currentState = STATE_REBOOT_READY;
    drawContent();
}

function gotoShutdownConfirm() {
    console.log('User selected shutdown button');
    currentState = STATE_CONFIRM_SHUTDOWN;
    drawContent();
}

function gotoShuttingDown() {
    console.log('User selected yes button');
    currentState = STATE_SHUTTING_DOWN;
    drawContent();
}

function gotoShutdownReady() {
    currentState = STATE_SHUTDOWN_READY;
    drawContent();
}
