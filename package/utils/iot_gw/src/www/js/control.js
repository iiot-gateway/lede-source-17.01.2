// ------------------------------------------------------------------
// CGI program - Control
// ------------------------------------------------------------------
// Javascript for the IoT Control webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

var  COMMAND_NONE              = 0;
var  COMMAND_TSCHECK           = 1;
var  COMMAND_RESET             = 2;
var  COMMAND_FACTORY_RESET     = 3;
var  COMMAND_SET_CHANMASK      = 4;
// var  COMMAND_NETWORK_START     = 5;
var  COMMAND_VERSION_REFRESH   = 6;
var  COMMAND_TOPO_UPLOAD       = 8;
// var  COMMAND_....              = 9;
var  COMMAND_RESTART_ALL       = 10;
var  COMMAND_RESTART_ZB        = 11;
// var  COMMAND_RESTART_DP        = 12;
var  COMMAND_RESTART_SJ        = 13;
// var  COMMAND_RESTART_CI        = 14;
var  COMMAND_RESTART_ND        = 15;
var  COMMAND_PLUG_START        = 16;
var  COMMAND_PLUG_STOP         = 17;
var  COMMAND_GET_SYSTEMTABLE   = 18;
var  COMMAND_GET_QUEUES        = 19;
var  COMMAND_CURRENT_VERSION   = 20;
var  COMMAND_CHECK_UPDATES     = 21;
var  COMMAND_INSTALL_UPDATES   = 22;
var  COMMAND_PERMITJOIN_START  = 23;
var  COMMAND_PERMITJOIN_STOP   = 24;
var  COMMAND_SYNC_TIME         = 25;
var  COMMAND_SET_NFCMODE       = 26;

var  UPDATE_NO_CHANGES         = 51;
var  UPDATE_CHANGES_NOINFO     = 52;
var  UPDATE_CHANGES_WITHINFO   = 53;

var  ERROR_EMPTY_SYSTEMTABLE   = 1;
var  ERROR_NO_QUEUE_INFO       = 2;
var  ERROR_UNABLE_TO_WRITE_Q   = 3;
var  ERROR_NOVERSIONINFO       = 4;
var  ERROR_SCRIPT_DOWNLOAD     = 5;

var  SYS_COLUMN_ID          = 0;
var  SYS_COLUMN_NAME        = 1;
var  SYS_COLUMN_INTVAL      = 2;
var  SYS_COLUMN_STRVAL      = 3;
var  SYS_COLUMN_LASTUPDATE  = 4;

var pollingSec = 2;
var timerVar   = 0;
var ts         = 0;
var lastsum    = 0;
var queueSkip  = 0;
var chanmask   = 0;
var nfcmode    = 4;
var extpanid   = -1;
var installing = 0;
var progress   = 0;
var permitend  = 0;
var blocktopo  = -1;
var topotimer  = 0;

// ------------------------------------------------------------------
// On page load
// ------------------------------------------------------------------

function iotOnLoad() {
   document.getElementById('javaerror').style.display = 'none';
   //document.getElementById('div_changes').style.display = 'none';
   //document.getElementById('div_install').style.display = 'none';
   //document.getElementById('div_reinstall').style.display = 'none';
   document.getElementById('div_permitstop').style.display = 'none';
   //siteSelector( 0 );
   //relSelector( 0 );
   timerRestart();
   CtrlUpdate();
   document.getElementById('div_chanmask').innerHTML = htmlChanmask( 0 );
   document.getElementById('div_nfcmode').innerHTML = htmlNfcmode( nfcmode );
};

// ------------------------------------------------------------------
// Timer
// ------------------------------------------------------------------

function timerRestart() {
    if ( timerVar ) clearInterval( timerVar );
    // Avoid strict x-sec pace
    var period = ( pollingSec * 1000 ) - 50 + Math.floor( Math.random() * 100 );
    timerVar = setInterval( CtrlUpdate, period );
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
    xmlhttp.open( 'POST', 'iot_control.cgi', true );
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

function htmlTable( table, timecol, greater1 ) {
    var html = "<table border=1>";

    var rows = table.split( ";" );
    for ( var r=0; r<rows.length; r++ ) {
        html += "<tr>";
        var cols = rows[r].split( "," );
        for ( var c=0; c<cols.length; c++ ) {
            var val = cols[c];
            if ( r==0 )            html += "<td class='head'>"+val+"</td>";
            else if ( c==timecol ) html += "<td>"+timeConv(val)+"</td>";
            else if ( c==greater1 && val > 1 )
                html += "<td class='bigred'>"+val+"</td>";
            else                   html += "<td>"+val+"</td>";
        }
        html += "</tr>";
    }
    html += "</table>";

    return( html );
}

// ------------------------------------------------------------------
// Print channel selector
// ------------------------------------------------------------------

function htmlChanmask( mask ) {
    var html = "Channel mask: 0x"+mask.toString(16)+"<br/>";
    html += "<table border=1>";
    html += "<tr class='head'>";
    for ( var i=11; i<=26; i++ ) {
        if ( i == 11 || i == 14 || i == 15 )
             html += "<td>"+i+"</td>";
        else html += "<td style='background-color:AAAAAA'>"+i+"</td>";
    }
    html += "</tr>";
    html += "<tr>";
    for ( var i=11; i<=26; i++ ) {
        html += "<td><input type='checkbox' name='cb'"+i;
        if ( mask & ( 1 << i ) )
             html += " onclick='CtrlChannel("+i+",0)' checked='yes'></td>";
        else html += " onclick='CtrlChannel("+i+",1)'></td>";
        html += "</td>";
    }
    html += "</tr>";
    html += "</table>";
    html += "(Green = preferrable HA channels for co-existance with WiFi. Changes only take effect after clicking Create Network)";

    return( html );
}

// ------------------------------------------------------------------
// Print nfcmode selector
// ------------------------------------------------------------------

function htmlNfcmode( mode ) {
    var modes = [ "Commission Device", "Decommission Device", "Factory Reset Device" ];
    var html = "<table border=1>";
    html += "<tr class='head'>";
    for ( var i=0; i<2; i++ ) {
        html += "<td ";
        if ( i != mode ) {
             html += " style='background-color:E6E6E6' "; // VeryLightGrey (90%)
        }
        html += " onclick='CtrlNfcmode("+i+")' >"+modes[i]+"</td>";
    }
    html += "</tr>";
    html += "</table>";

    return( html );
}

// ------------------------------------------------------------------
// SW upgrade
// ------------------------------------------------------------------

function progressTxt() {
    // progress bar made via "preloaders.net/en/horizontal"
    return "Installation in progress, please wait ... " + (progress++)  +
            "<br /><img src='/img/progress.gif' style='border:0;' />";
}

var sites = [ "EHV", "SGP", "UK" ];
var selectedSite = 0;

function siteSelector( site ) {
    var html = "<select onchange='siteSelector(this.selectedIndex);'>\n";
    for ( var i=0; i< sites.length; i++ ) {
        html += "<option value=" + i;
        if ( i == site ) html += " selected";
        html += ">" + sites[i] + "</option>\n";
    }
    html += "</select>\n";
    document.getElementById('div_sites').innerHTML = html;
    selectedSite = site;
    console.log('Site : '+site);
}

var released = 0;

function relSelector( release ) {
    released = ( release ) ? 1 : 0;
    var html = "<div style='display:inline' onclick='toggleReleased();'>\n";
    html += "<input type='checkbox'";
    if ( released ) html += " checked";
    html += ">Released only\n";
    html += "</div>\n";
    document.getElementById('div_release').innerHTML = html;
    console.log('Released : '+released);
}

function toggleReleased() {
    relSelector( 1 - released );
}

// ------------------------------------------------------------------
// Error handling
// ------------------------------------------------------------------

function printError( err ) {
    console.log('err='+err );
    var errText = "Error ";
    switch ( parseInt( err ) ) {
    case 1:
        errText += "Empty system table";
        break;
    case 2:
        errText += "No queue information";
        break;
    case 3:
        errText += "Unable to write to ZCB";
        break;
    case 4:
        errText += "No version info";
        break;
    case 5:
        errText += "Problems getting update script from cloud server";
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
// Exec command
// ------------------------------------------------------------------

function CtrlExec( post ) {
    timerRestart();
    console.log('CtrlExec : '+post);
    document.getElementById('div_resetmsg').innerHTML = "";
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
                    var ts2 = getXmlVal( xmlDoc, 'ts' );
                    var sum = getXmlVal( xmlDoc, 'sum' );
                    document.getElementById('headerDate').innerHTML =
                           timeConv( clk ) + " (" + pid + ")";
                    console.log('sum='+sum+', ts=' + ts + ', ts2=' + ts2);
                    var changed = ( sum != lastsum ) || ( ts2 != ts );
                    if ( changed ) {
                        lastsum = sum;
                        ts      = ts2;
                        // console.log('changed=' + ts );
                        CtrlCommand( COMMAND_GET_SYSTEMTABLE );
                    }

                    if ( clk < permitend ) document.getElementById('div_permitstop').style.display = 'inline';
                    else document.getElementById('div_permitstop').style.display = 'none';

                    break;

                case COMMAND_GET_SYSTEMTABLE:
                    var clk = getXmlVal( xmlDoc, 'clk' );
                    var sys = getXmlVal( xmlDoc, 'sys' );
                    console.log('sys='+sys );
                    var html = htmlTable( sys, 4, -1 );
                    document.getElementById('div_table').innerHTML = html;

                    // Handle some specials
                    var rows = sys.split( ";" );
                    chanmask = 0;
                    var version = "";
                    permitend = 0;
                    for ( var r=1; r<rows.length; r++ ) {
                        var cols = rows[r].split( "," );
                        if ( cols[SYS_COLUMN_NAME] == "version" ) {
                            version = cols[SYS_COLUMN_STRVAL];  // String
                        } else if ( cols[SYS_COLUMN_NAME] == "chanmask" ) {
                            chanmask = cols[SYS_COLUMN_INTVAL];  // Integer
                        } else if ( cols[SYS_COLUMN_NAME] == "permit" ) {
                            permitend = cols[SYS_COLUMN_INTVAL];  // Integer
                        } else if ( cols[SYS_COLUMN_NAME] == "nfcmode" ) {
                            nfcmode = cols[SYS_COLUMN_INTVAL];  // Integer
                        } else if ( cols[SYS_COLUMN_NAME] == "zcb_extpanid" ) {
                            var new_extpanid = cols[SYS_COLUMN_LASTUPDATE];  // Integer
                            if ( extpanid >= 0 && ( new_extpanid != extpanid ) ) {
                                document.getElementById('div_resetmsg').innerHTML = "System has been reset";
                                // alert("System has been reset");
                            }
                            extpanid = new_extpanid;
                        }
                    }
                    document.getElementById('div_version').innerHTML = version;
                    html = htmlChanmask( parseInt(chanmask) );
                    document.getElementById('div_chanmask').innerHTML = html;
                    html = htmlNfcmode( parseInt(nfcmode) );
                    document.getElementById('div_nfcmode').innerHTML = html;

                    if ( installing && ( rows.length > 1 ) ) {
                        installing = 0;
                        document.getElementById('div_changes').innerHTML = 
                            "Installation ready, system running again";
                        document.getElementById('div_install').style.display = 'none';
                        document.getElementById('div_reinstall').style.display = 'none';
                    }

                    if ( clk < permitend ) document.getElementById('div_permitstop').style.display = 'inline';
                    else document.getElementById('div_permitstop').style.display = 'none';

                    break;

                case COMMAND_GET_QUEUES:
                    var qs = getXmlVal( xmlDoc, 'qs' );
                    console.log('qs='+qs );
                    var html = htmlTable( qs, -1, 1 );
                    document.getElementById('div_queues').innerHTML = html;
                    break;

                case COMMAND_CURRENT_VERSION:
                    var extra = getXmlVal( xmlDoc, 'extra' );
                    document.getElementById('div_changes').innerHTML = 
                        "<pre>" + extra + "</pre>";
                    document.getElementById('div_changes').style.display = 'block';
                    document.getElementById('div_install').style.display = 'none';
                    document.getElementById('div_reinstall').style.display = 'none';
                    break;

                case COMMAND_CHECK_UPDATES:
                    var status = getXmlVal( xmlDoc, 'status' );
                    console.log('status='+status );
                    switch ( parseInt( status ) ) {
                        case UPDATE_NO_CHANGES:
                            document.getElementById('div_changes').innerHTML = 
                                "You already have the latest SW version";
                            document.getElementById('div_changes').style.display = 'block';
                            document.getElementById('div_install').style.display = 'none';
                            document.getElementById('div_reinstall').style.display = 'block';
                            break;
                        case UPDATE_CHANGES_WITHINFO:
                            var extra = getXmlVal( xmlDoc, 'extra' );
                            document.getElementById('div_changes').innerHTML = 
                                "<pre>" + extra + "</pre>";
                            document.getElementById('div_changes').style.display = 'block';
                            document.getElementById('div_install').style.display = 'block';
                            document.getElementById('div_reinstall').style.display = 'none';
                            break;
                        case UPDATE_CHANGES_NOINFO:
                            document.getElementById('div_install').style.display = 'block';
                            document.getElementById('div_reinstall').style.display = 'none';
                            break;
                    }
                    break;

                case COMMAND_INSTALL_UPDATES:
                    document.getElementById('div_changes').innerHTML = progressTxt();
                    document.getElementById('div_changes').style.display = 'block';
                    document.getElementById('div_install').style.display = 'none';
                    document.getElementById('div_reinstall').style.display = 'none';
                    break;

                default:
                    console.log('cmd='+cmd );
                    break;
                }
            } else {
                printError( err );

                switch ( parseInt( cmd ) ) {
                case COMMAND_CURRENT_VERSION:
                case COMMAND_CHECK_UPDATES:
                    if ( err == ERROR_NOVERSIONINFO ) {
                        document.getElementById('div_changes').innerHTML = 
                           "No version info available";
                        document.getElementById('div_changes').style.display = 'block';
                    }
                    document.getElementById('div_install').style.display = 'none';
                    document.getElementById('div_reinstall').style.display = 'none';
                    break;
                }
            }
        } );
}

// ------------------------------------------------------------------
// Update
// ------------------------------------------------------------------

function CtrlUpdate() {
    var d = new Date();
    var n = d.getTime() / 1000;
    document.getElementById('div_browsertime').innerHTML = timeConv( n );

    // console.log('CtrlUpdate');
    var post = 'command='+COMMAND_TSCHECK;
    CtrlExec( post );

    if ( --queueSkip < 0 ) {
        queueSkip = 10 / pollingSec;      // 10 sec
        CtrlCommand( COMMAND_GET_QUEUES );
    }

    if ( installing ) {
        document.getElementById('div_changes').innerHTML = progressTxt();
    }
}

// ------------------------------------------------------------------
// Topo blocker
// ------------------------------------------------------------------

function unblocktopo() {
    if ( blocktopo > 0 ) {
        document.getElementById('div_upload').innerHTML = blocktopo;
        document.getElementById('div_upload').style.display = 'inline';
    } else {
        document.getElementById('div_upload').style.display = 'none';
        if ( topotimer ) clearInterval( topotimer );
        topotimer = 0;
    }
    blocktopo--;
}

// ------------------------------------------------------------------
// Command
// ------------------------------------------------------------------

function CtrlCommand( cmd ) {
    console.log('CtrlCommand : '+cmd);
    var post = 'command='+cmd;
    switch ( parseInt( cmd ) ) {
    case COMMAND_CHECK_UPDATES:
        post += "&site=" + selectedSite;
        post += "&released=" + released;
        document.getElementById('div_install').style.display = 'none';
        document.getElementById('div_reinstall').style.display = 'none';
        break;
    case COMMAND_INSTALL_UPDATES:
        post += "&site=" + selectedSite;
        post += "&released=" + released;
        installing = 1;
        progress = 0;
        document.getElementById('div_install').style.display = 'none';
        document.getElementById('div_reinstall').style.display = 'none';
        break;
    case COMMAND_TOPO_UPLOAD:
        console.log( "blocktopo = " + blocktopo );
        if ( blocktopo < 0 ) {
            blocktopo = 5;
            unblocktopo();
            if ( topotimer ) clearInterval( topotimer );
            topotimer = setInterval( unblocktopo, 1000 );
        } else {
            post = 0;
        }
        break;
    case COMMAND_SYNC_TIME:
        var d = new Date();
        var n = d.getTime() / 1000;
        post += "&browsertime=" + n;
        break;
    }
    clearError();
    if ( post != 0 ) CtrlExec( post );
}

// ------------------------------------------------------------------
// Control channel
// ------------------------------------------------------------------

function CtrlChannel( ch, onoff ) {
    console.log('CtrlChannel : '+ch+', '+onoff);
    timerRestart();

    if ( onoff ) chanmask |= ( 1 << ch );
    else chanmask &= ~( 1 << ch );

    var html = htmlChanmask( chanmask );
    document.getElementById('div_chanmask').innerHTML = html;

    var post = 'command='+COMMAND_SET_CHANMASK+'&chanmask='+chanmask;
    clearError();
    CtrlExec( post );

    CtrlUpdate();
}

// ------------------------------------------------------------------
// Control NFC mode
// ------------------------------------------------------------------

function CtrlNfcmode( mode ) {
    console.log('CtrlNfcmode : '+mode);
    timerRestart();

    var html = htmlNfcmode( mode );
    document.getElementById('div_nfcmode').innerHTML = html;

    var post = 'command='+COMMAND_SET_NFCMODE+'&nfcmode='+mode;
    clearError();
    CtrlExec( post );

    CtrlUpdate();
}

