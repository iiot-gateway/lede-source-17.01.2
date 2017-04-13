// ------------------------------------------------------------------
// CGI program - Plug Meters
// ------------------------------------------------------------------
// Javascript for the IoT Plug Meters webpage
// Uses C3.js and D3.js for the charts
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

var COMMAND_NORMAL     = 0;
var COMMAND_TSCHECK    = 1;
var COMMAND_GET_LIST   = 3;
var COMMAND_GET_PLUG   = 4;
var COMMAND_CTRL       = 5;

var UI_LIST         = 0;
var UI_INFO         = 1;

var NULL            = 0;

// ------------------------------------------------------------------
// Variables
// ------------------------------------------------------------------

var uimode     = UI_LIST;
var plugmac    = 0;
var plugonoff  = 0;

var timerVar   = 0;
var ts         = 0;
var regular    = 0;
var interval   = 2;

var now        = 0;
var ageTimer   = 0;

var plugs;

// ------------------------------------------------------------------
// On page load
// ------------------------------------------------------------------

function onLoad( mac ) {
    console.log( "================ onload ", mac );
    $('#javaerror').hide();

    timerRestart();

    if ( mac == 0 ) getPlugList();
    else getPlugInfo( mac );
}

// ------------------------------------------------------------------
// Timer
// ------------------------------------------------------------------

function timerRestart() {
    if ( timerVar ) clearInterval( timerVar );
    // Avoid strict x-sec pace
    var period = ( interval * 1000 ) - 50 + Math.floor( Math.random() * 100 );
    timerVar = setInterval( PlugUpdate, period );
}

// ------------------------------------------------------------------
// UI
// ------------------------------------------------------------------

function showUi() {
    switch ( uimode ) {
    case UI_LIST:
        $('#info').hide();
        $('#onoff').hide();
        $('#backarrow').hide();
        $('#pluglist').show();
        break;
    case UI_INFO:
        $('#info').show();
        $('#onoff').show();
        $('#backarrow').show();
        $('#pluglist').hide();
        break;
    }
};

// ------------------------------------------------------------------
// Goto
// ------------------------------------------------------------------

function gotoPlug( mac, onoff, act, sum ) {
    plugmac   = mac;
    plugonoff = onoff;
    console.log( 'gotoPlug: mac = '+mac );
    uimode = UI_INFO;
    showUi();
    regular = 0;
    PlugUpdate();
    document.getElementById('fieldlegend').innerHTML = 'Plug - '+ mac;
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
    xmlhttp.open( 'POST', 'iot_plugs.cgi', true );
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
// Table
// ------------------------------------------------------------------

function num1( num ) {
    return( isNaN(num) ? "-" : num );
}

function htmlPlugsList() {
    var html = "";

    for ( var r=1; r<plugs.length; r++ ) {   // Skip header row
        var cols = plugs[r].split( "," );

        var mac  = cols[COLUMN_DEVS_MAC];
        var name = cols[COLUMN_DEVS_NM];
        var cmd  = cols[COLUMN_DEVS_CMD];
        var act  = cols[COLUMN_DEVS_ACT];
        var sum  = cols[COLUMN_DEVS_SUM];
        var upd  = cols[COLUMN_DEVS_LASTUPDATE];

        html += "<div class='plug'>\n"; 
        html += "<div class='plug_col1'>\n";
        html += mac+"</br />\n";
        html += (( name ) ? name : "No name" ) +"</br />\n";
        html += timeConv( upd )+"</br />\n";
        html += "</div>\n";

        html += "<div class='plug_col2'>\n";
        console.log( "now=" + now + ", upd=" + upd );        
        if ( ( now - upd ) < 10 ) {
            html += num1( act )+" W\n";
        } else {
            html += "-- W\n";
        }
        html += "</div>\n";

        html += "<div class='plug_col3'>\n";
        html += num1( sum )+" Wh\n";
        html += "</div>\n";

        var onoff = ( cmd == "on" );

        html += "<div class='plug_col4'>\n";
        html += "<img src='/img/info.png' height='36'";
        // html += " onclick='gotoPlug(\""+mac+"\","+onoff+","+act+","+sum+")' />";
        html += " onclick='getPlugInfo(\""+mac+"\")' />";
        html += "</div>\n";

        html += "<div class='plug_col5'>\n";
        if ( onoff ) {
            html += "<img src='/img/plug.png' height='36'";
            html += " onclick='ctrlPlug(\""+mac+"\",0)' />";
        } else {
            html += "<img src='/img/noplug.png' height='36'";
            html += " onclick='ctrlPlug(\""+mac+"\",1)' />";
        }
        html += "</div>\n";

        html += "</div>\n";
        html += "<br />\n";
    }

    return( html );
}

function redrawList() {
    console.log( "Calling ageTimer" );
    var html = htmlPlugsList();
    document.getElementById('pluglist').innerHTML = html;
    ageTimer = 0;
}

// ------------------------------------------------------------------
// Chart
// ------------------------------------------------------------------

function setChartData( hourdata, daydata ) {
    console.log('hourdata = ' + hourdata);
    var nums  = hourdata.split(',');
    var line  = new Array();
    var delta = new Array();
    var x     = new Array();
    line[0]   = 'Hour data';
    delta[0]  = 'Delta';
    x[0]      = 'x';
    var offset = -1;
    var num, cnt = 0;
    for ( i=nums.length-1; i>=0; i-- ) {
        num = nums[i];
        if ( cnt > 0 ) {
            x[cnt]     = 1 - cnt;
            line[i+1]  = num;
            delta[i+1] = num - offset;
        }
        offset = num;
        cnt++;
    }
    var chart = c3.generate({
        bindto: '#hourchart',
        data: {
            x: 'x',
            columns: [
                x, line, delta
            ],
            axes: {
                Delta: 'y2'
            },
            types: {
                Delta: 'bar'
            }
        },
        axis: {
            y: {
                label: {
                    text: 'Cumulative hour data [Wh]',
                    position: 'outer-middle'
                }
            },
            y2: {
                show: true,
                label: {
                    text: 'Delta [Wh]',
                    position: 'outer-middle'
                }
            }
        }
    });

    console.log('daydata = ' + daydata);
    var nums  = daydata.split(',');
    var line  = new Array();
    var delta = new Array();
    var x     = new Array();
    line[0]   = 'Day data';
    delta[0]  = 'Delta';
    x[0]      = 'x';
    var offset = -1;
    var num, cnt = 0;
    for ( i=nums.length-1; i>=0; i-- ) {
        num = nums[i];
        if ( cnt > 0 ) {
            x[cnt]     = 1 - cnt;
            line[i+1]  = num;
            delta[i+1] = num - offset;
        }
        offset = num;
        cnt++;
    }
    var chart = c3.generate({
        bindto: '#daychart',
        data: {
            x: 'x',
            columns: [
                x, line, delta
            ],
            axes: {
                Delta: 'y2'
            },
            types: {
                Delta: 'bar'
            }
        },
        axis: {
            y: {
                label: {
                    text: 'Cumulative day data [Wh]',
                    position: 'outer-middle'
                }
            },
            y2: {
                show: true,
                label: {
                    text: 'Delta [Wh]',
                    position: 'outer-middle'
                }
            }
        }
    });
}

// ------------------------------------------------------------------
// Error handling
// ------------------------------------------------------------------

function printError( command, err ) {
    var errText = 'Command '+command+', error ';
    switch ( parseInt( err ) ) {
    case 1:
        errText += "No MAC specified";
        break;
    case 2:
        errText += "Unable to open database";
        break;
    case 3:
        errText += "Unable to open database 2";
        break;
    case 4:
        errText += "Device not found";
        break;
    case 5:
        errText += "Unable to communicate with Control Interface";
        break;
    default:
        errText += err;
        break;
    }
    console.log(errText);
    document.getElementById('debug').innerHTML = errText;
}

function clearError() {
    document.getElementById('debug').innerHTML = "";
}

// ------------------------------------------------------------------
// Execute
// ------------------------------------------------------------------

function PlugExec( post ) {
    console.log('PlugExec : '+post);
    console.log( 'uimode='+uimode );
    doHttp( post,
        function( xmlhttp ) {
            var xmlDoc = xmlhttp.responseXML;
            var err = getXmlVal( xmlDoc, 'err' );
            if ( err == 0 ) {
                var cmd = getXmlVal( xmlDoc, 'cmd' );
                console.log('cmd='+cmd );

                switch ( parseInt( cmd ) ) {
                case COMMAND_TSCHECK:
                    now = getXmlVal( xmlDoc, 'clk' );
                    var pid = getXmlVal( xmlDoc, 'pid' );
                    document.getElementById('headerDate').innerHTML =
                           timeConv( now ) + " (" + pid + ")";

                    var ts2 = getXmlVal( xmlDoc, 'ts' );
                    console.log('ts2='+ts2, 'uimode='+uimode );
                    if ( ( ts2 != ts ) || ( regular <= 0 ) ) {
                        if ( regular <= 0 ) {
                            regular = 60;
                        } else {
                            ts = ts2;
                            if ( uimode == UI_LIST ) {
                                getPlugList();
                            } else {
                                getPlugInfo( plugmac );
                            }
                        }
                    }
                    break;

                case COMMAND_GET_LIST:
                    now = getXmlVal( xmlDoc, 'clk' );
                    var list = getXmlVal( xmlDoc, 'list' );
                    console.log('list='+list );
                    plugs = list.split( ";" );
                    var html = htmlPlugsList();
                    document.getElementById('pluglist').innerHTML = html;
                    uimode = UI_LIST;
                    showUi();
                    document.getElementById('fieldlegend').innerHTML = 'Plugs';
                    
                    console.log( "Installing ageTimer" );
                    if ( ageTimer ) clearTimeout( ageTimer );
                    // Note: Assumes a plug update every 60 secs
                    ageTimer = setTimeout( redrawList, 65000 );
                    break;

                case COMMAND_GET_PLUG:
                    var time, onoff, act, sum, dayhistory;
                    mac         = getXmlVal( xmlDoc, 'mac' );
                    onoff       = parseInt( getXmlVal( xmlDoc, 'onoff' ) );
                    act         = getXmlVal( xmlDoc, 'act' );
                    sum         = getXmlVal( xmlDoc, 'sum' );
                    hourhistory = getXmlVal( xmlDoc, 'hourhistory' );
                    dayhistory  = getXmlVal( xmlDoc, 'dayhistory' );

                    plugonoff = onoff;
                    if ( onoff == 1 ) {
                        document.getElementById('onoff').innerHTML =
                               "<img src='/img/plug.png' width='36'/>";
                    } else {
                        document.getElementById('onoff').innerHTML =
                               "<img src='/img/noplug.png' width='36'/>";
                    }
                    document.getElementById('actsum').innerHTML =
                         'Actual: '+num1(act)+', Sum: '+num1(sum)+' ('+ts+')';
                    // console.log( 'dayhistory=' + dayhistory );
                    setChartData( hourhistory, dayhistory );
                    gotoPlug( mac, onoff, act, sum );
                    break;
                }
            };
        } );
}

// ------------------------------------------------------------------
// Commands
// ------------------------------------------------------------------

function PlugUpdate() {
    // console.log('PlugUpdate '+regular);
    if ( uimode != UI_LIST ) {
        regular -= interval;
    }
    var post = 'command='+COMMAND_TSCHECK;
    PlugExec( post );
}

function getPlugList() {
    PlugExec( 'command='+COMMAND_GET_LIST );
}

function getPlugInfo( mac ) {
    PlugExec( 'command='+COMMAND_GET_PLUG+'&mac='+mac );
}

function ctrlPlug( mac, onoff ) {
    console.log('ctrlPlug, mac='+mac+', onoff='+onoff);
    var post = 'command='+COMMAND_CTRL+'&mac='+mac+'&cmd=';
    post += ( onoff ) ? 'on' : 'off';
    doHttp( post,
        function( xmlhttp ) {
            var xmlDoc = xmlhttp.responseXML;
            var err = getXmlVal( xmlDoc, 'err' );
            console.log('ctrlPlug err=' + err);
            switch ( uimode ) {
            case UI_LIST:
                getPlugList();
                break;
            default:
                getPlugInfo( mac );
                break;
            }
        } );
}

function togglePlug() {
    var newonoff = ( plugonoff == 1 ) ? 0 : 1;
    ctrlPlug( plugmac, newonoff );
}

// ------------------------------------------------------------------
// Misc
// ------------------------------------------------------------------

function goBack( really ) {
    if ( really ) window.history.back();
    else getPlugList();
}

