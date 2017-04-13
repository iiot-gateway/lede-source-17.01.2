// ------------------------------------------------------------------
// Groups 
// ------------------------------------------------------------------
// Javascript for the IoT Groups webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

var COMMAND_NORMAL           = 0;
var COMMAND_TSCHECK          = 1;
var COMMAND_GET_LAMPS        = 2;
var COMMAND_GROUP_MANIPULATE = 3;
var COMMAND_SCENE_MANIPULATE = 4;
var COMMAND_SCENE_EXECUTE    = 5;

var COMMAND_GROUP_ON         = 11;
var COMMAND_GROUP_OFF        = 12;
var COMMAND_GROUP_0          = 13;
var COMMAND_GROUP_25         = 14;
var COMMAND_GROUP_50         = 15;
var COMMAND_GROUP_75         = 16;
var COMMAND_GROUP_100        = 17;
var COMMAND_GROUP_RED        = 18;
var COMMAND_GROUP_GREEN      = 19;
var COMMAND_GROUP_BLUE       = 20;
var COMMAND_GROUP_WHITE      = 21;

var ID_GM_MAC   = 1;
var ID_GM_CMD   = 2;
var ID_GM_GID   = 3;

var ID_GC_GID   = 4;

var ID_SM_MAC   = 5;
var ID_SM_GID   = 6;
var ID_SM_CMD   = 7;
var ID_SM_SID   = 8;

var ID_SC_GID   = 9;
var ID_SC_CMD   = 10;
var ID_SC_SID   = 11;

var NULL        = 0;

// ------------------------------------------------------------------
// Variables
// ------------------------------------------------------------------

var timerVar   = 0;
var delayedVar = 0;
var ts         = 0;

// Group manipulate
var gm_mac     = 0;
var gm_cmd     = 0;
var gm_cmds    = [ 'command', 'add', 'rem', 'remall' ];
var gm_gid     = 0;

// Group commands
var gc_gid     = 0;

// Scene manipulate
var sm_mac     = 0;
var sm_gid     = 0;
var sm_cmd     = 0;
var sm_cmds    = [ 'command', 'add', 'rem', 'remall', 'store', 'recall' ];
var sm_sid     = 0;

// Scene commands
var sc_gid     = 0;
var sc_cmd     = 0;
var sc_cmds    = [ 'command', 'recall', 'remall' ];
var sc_sid     = 0;

// MAC array
var lampMACs = new Array();

// ------------------------------------------------------------------
// At startup
// ------------------------------------------------------------------

$(document).ready(function() {
    document.getElementById('javaerror').style.display = 'none';
    getLampList();

    // Group manipulate
    var html = strSelector( ID_GM_CMD, gm_cmd, gm_cmds );
    document.getElementById('gm_cmd').innerHTML = html;
    var html = numSelector( ID_GM_GID, gm_gid, 5, 'group-id' );
    document.getElementById('gm_gid').innerHTML = html;

    // Group commands
    var html = numSelector( ID_GC_GID, gc_gid, 5, 'group-id' );
    document.getElementById('gc_gid').innerHTML = html;

    // Scene manipulate
    var html = numSelector( ID_SM_GID, sm_gid, 5, 'group-id' );
    document.getElementById('sm_gid').innerHTML = html;
    var html = strSelector( ID_SM_CMD, sm_cmd, sm_cmds );
    document.getElementById('sm_cmd').innerHTML = html;
    var html = numSelector( ID_SM_SID, sm_sid, 5, 'scene-id' );
    document.getElementById('sm_sid').innerHTML = html;

    // Scene commands
    var html = numSelector( ID_SC_GID, sc_gid, 5, 'group-id' );
    document.getElementById('sc_gid').innerHTML = html;
    var html = strSelector( ID_SC_CMD, sc_cmd, sc_cmds );
    document.getElementById('sc_cmd').innerHTML = html;
    var html = numSelector( ID_SC_SID, sc_sid, 5, 'scene-id' );
    document.getElementById('sc_sid').innerHTML = html;

    timerRestart();
});

// ------------------------------------------------------------------
// Timer
// ------------------------------------------------------------------

function timerRestart() {
    if ( timerVar ) clearInterval( timerVar );
    // Avoid strict 2-sec pace
    var period = 1950 + Math.floor( Math.random() * 100 );
    timerVar = setInterval( GroupsUpdate, period );
}

// ------------------------------------------------------------------
// Do selector
// ------------------------------------------------------------------

function doSelector( id, val ) {
    hideError();
    switch ( id ) {
        // Group manipulate
        case ID_GM_MAC: gm_mac = val; break;
        case ID_GM_CMD:
            gm_cmd = val;
            document.getElementById('gm_gid').style.display =
               ( val==1 || val==2 ) ? 'inline' : 'none';
            break;
        case ID_GM_GID: gm_gid = val; break;

        // Group commands
        case ID_GC_GID: gc_gid = val; break;

        // Scene manipulate
        case ID_SM_MAC: sm_mac = val; break;
        case ID_SM_GID: sm_gid = val; break;
        case ID_SM_CMD:
            sm_cmd = val;
            document.getElementById('sm_sid').style.display =
               ( val==1 || val==2 || val==4 || val==5 ) ? 'inline' : 'none';
            break;
        case ID_SM_SID: sm_sid = val; break;

        // Scene commands
        case ID_SC_GID: sc_gid = val; break;
        case ID_SC_CMD:
            sc_cmd = val;
            document.getElementById('sc_sid').style.display =
               ( val==1 ) ? 'inline' : 'none';
            break;
        case ID_SC_SID: sc_sid = val; break;
    }
}

// ------------------------------------------------------------------
// Number selector
// ------------------------------------------------------------------

function numSelector( id, curr, max, header ) {
    var html = "<select onchange='doSelector("+id+",this.selectedIndex);'>\n";
    html += "<option value=0";
    if ( curr == 0 ) html += " selected";
    html += ">" + header + "</option>\n";
    for ( var i=1; i <= max; i++ ) {
        html += "<option value=" + i;
        if ( i == curr ) html += " selected";
        html += ">" + i + "</option>\n";
    }
    html += "</select>\n";
    return html;
}

// ------------------------------------------------------------------
// String selector
// ------------------------------------------------------------------

function strSelector( id, curr, strings ) {
    var html = "<select onchange='doSelector("+id+",this.selectedIndex);'>\n";
    for ( var i=0; i < strings.length; i++ ) {
        html += "<option value=" + strings[i];
        if ( strings[i] == curr ) html += " selected";
        html += ">" + strings[i] + "</option>\n";
    }
    html += "</select>\n";
    return html;
}

// ------------------------------------------------------------------
// Lamp MACs
// ------------------------------------------------------------------

function getLampMACs( table ) {
    var macs = new Array();
    var rows = table.split( ";" );
    for ( var i=0; i < rows.length; i++ ) {
        var cols = rows[i].split( "," );
        macs[i] = cols[1];
    }
    return macs;
}

// ------------------------------------------------------------------
// Lamp Selector
// ------------------------------------------------------------------

function lampSelector( id, curr, table ) {
    var rows = table.split( ";" );

    var html = "<select onchange='doSelector("+id+",this.selectedIndex);'>\n";
    for ( var i=0; i < rows.length; i++ ) {
        var cols = rows[i].split( "," );
        html += "<option value=" + cols[1];
        if ( i == curr ) html += " selected";
        html += ">" + cols[1] + "</option>\n";
    }
    html += "</select>\n";
    return html;
}

// ------------------------------------------------------------------
// Button handler
// ------------------------------------------------------------------

function ButtonCmd( but ) {
    hideError();
    switch ( but ) {
    case COMMAND_GROUP_MANIPULATE:
        console.log( "mac="+gm_mac+",cmd="+gm_cmd+",gid="+gm_gid );
        if ( gm_mac > 0 && ( ( gm_cmd == 3 ) || ( gm_gid > 0 ) ) ) {
            var post = 'command='+but;
            post += '&mac='+lampMACs[gm_mac];
            post += '&cmd='+gm_cmds[gm_cmd];
            if ( gm_cmd != 3 ) post += '&gid='+gm_gid;
            GroupsExec( post );
        } else {
            showError( "First select lamp, command and group-id" );
        }
        break;

    case COMMAND_GROUP_ON:
    case COMMAND_GROUP_OFF:
    case COMMAND_GROUP_0:
    case COMMAND_GROUP_25:
    case COMMAND_GROUP_50:
    case COMMAND_GROUP_75:
    case COMMAND_GROUP_100:
    case COMMAND_GROUP_RED:
    case COMMAND_GROUP_GREEN:
    case COMMAND_GROUP_BLUE:
    case COMMAND_GROUP_WHITE:
        console.log( "gid="+gc_gid );
        if ( gc_gid > 0 ) {
            switch ( but ) {
            case COMMAND_GROUP_ON:
            case COMMAND_GROUP_OFF:
                var post = 'command='+but;
                post += '&gid='+gc_gid;
                GroupsExec( post );
                break;
            case COMMAND_GROUP_0:
            case COMMAND_GROUP_25:
            case COMMAND_GROUP_50:
            case COMMAND_GROUP_75:
            case COMMAND_GROUP_100:
                var post = 'command='+but;
                post += '&gid='+gc_gid;
                var lvl = 0;
                switch ( but ) {
                case COMMAND_GROUP_25: lvl=25; break;
                case COMMAND_GROUP_50: lvl=50; break;
                case COMMAND_GROUP_75: lvl=75; break;
                case COMMAND_GROUP_100: lvl=100; break;
                }
                post += '&lvl='+lvl;
                GroupsExec( post );
                break;
            case COMMAND_GROUP_RED:
            case COMMAND_GROUP_GREEN:
            case COMMAND_GROUP_BLUE:
            case COMMAND_GROUP_WHITE:
                var post = 'command='+but;
                post += '&gid='+gc_gid;
                var rgb = 0;
                switch ( but ) {
                case COMMAND_GROUP_RED:   rgb=0xff0000; break;
                case COMMAND_GROUP_GREEN: rgb=0x00ff00; break;
                case COMMAND_GROUP_BLUE:  rgb=0x0000ff; break;
                case COMMAND_GROUP_WHITE: rgb=0xffffff; break;
                }
                post += '&rgb='+rgb;
                GroupsExec( post );
                break;
            }
        } else {
            showError( "First select group-id" );
        }
        break;

    case COMMAND_SCENE_MANIPULATE:
        console.log( "mac="+sm_mac+",gid="+sm_gid+",cmd="+sm_cmd+",sid="+sm_sid );
        if ( sm_mac > 0 && sm_gid > 0 && ( ( sm_cmd == 3 ) || ( sm_sid > 0 ) ) ) {
            var post = 'command='+but;
            post += '&mac='+lampMACs[sm_mac];
            post += '&gid='+sm_gid;
            post += '&cmd='+sm_cmds[sm_cmd];
            if ( sm_cmd != 3 ) post += '&sid='+sm_sid;
            GroupsExec( post );
        } else {
            showError( "First select lamp, group-id, scene-command and scene-id" );
        }
        break;

    case COMMAND_SCENE_EXECUTE:
        console.log( "gid="+sc_gid+",cmd="+sc_cmd+",sid="+sc_sid );
        if ( sc_gid > 0 && ( ( sc_cmd == 2 ) || ( sc_sid > 0 ) ) ) {
            var post = 'command='+but;
            post += '&gid='+sc_gid;
            post += '&cmd='+sc_cmds[sc_cmd];
            if ( sc_cmd != 2 ) post += '&sid='+sc_sid;
            GroupsExec( post );
        } else {
            showError( "First select group-id, scene-command and scene-id" );
        }
        break;
    }
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
    xmlhttp.open( 'POST', 'iot_groups.cgi', true );
    xmlhttp.setRequestHeader( 'Content-type',
                              'application/x-www-form-urlencoded' );
    xmlhttp.send( posts );
}

// ------------------------------------------------------------------
// Helpers
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
// Error
// ------------------------------------------------------------------

function hideError() {
    document.getElementById('debug').innerHTML = "";
}

function showError( err ) {
    console.log( err );
    document.getElementById('debug').innerHTML = err;
    document.getElementById('debug').style.color = "#ff0000";
}

function commandError( command, err ) {
    var html = 'Command '+command+', error '+err;
    console.log( html );
    document.getElementById('debug').innerHTML = html;
    document.getElementById('debug').style.color = "#ff00ff";
}

// ------------------------------------------------------------------
// Execute command
// ------------------------------------------------------------------

function GroupsExec( post ) {
    console.log('GroupsExec : '+post);
    doHttp( post,
        function( xmlhttp ) {
            var xmlDoc = xmlhttp.responseXML;
            var str = xml2Str (xmlDoc);
            console.log('xml='+str );
            var err = getXmlVal( xmlDoc, 'err' );
            var cmd = getXmlVal( xmlDoc, 'cmd' );
            // console.log('cmd='+cmd );
            if ( err == 0 ) {

                switch ( parseInt( cmd ) ) {
                case COMMAND_TSCHECK:
                    var clk = getXmlVal( xmlDoc, 'clk' );
                    var pid = getXmlVal( xmlDoc, 'pid' );
                    document.getElementById('headerDate').innerHTML =
                           timeConv( clk ) + " (" + pid + ")";
                    var ts2 = getXmlVal( xmlDoc, 'ts' );
                    console.log('ts2='+ts2 );
                    if ( ts2 != ts ) {
                        ts = ts2;
                        regular = 60;
                        getLampList();
                    }
                    break;

                case COMMAND_GET_LAMPS:
                    var list = getXmlVal( xmlDoc, 'list' );
                    // console.log('list='+list );
                    lampMACs = getLampMACs( list );
                    var html = lampSelector( ID_GM_MAC, gm_mac, list );
                    document.getElementById('gm_lamps').innerHTML = html;
                    var html = lampSelector( ID_SM_MAC, sm_mac, list );
                    document.getElementById('sm_lamps').innerHTML = html;
                    break;

                }
            } else commandError( cmd, err );
        } );
}

// ------------------------------------------------------------------
// Commands
// ------------------------------------------------------------------

function GroupsUpdate() {
    // console.log('GroupsUpdate');
    var post = 'command='+COMMAND_TSCHECK;
    GroupsExec( post );
}

function getLampList() {
    GroupsExec( 'command='+COMMAND_GET_LAMPS );
}

