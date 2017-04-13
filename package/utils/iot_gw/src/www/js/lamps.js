// ------------------------------------------------------------------
// Lamps 
// ------------------------------------------------------------------
// Javascript for the IoT Lamp webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

var COMMAND_NORMAL     = 0;
var COMMAND_TSCHECK    = 1;
var COMMAND_GET_LIST   = 3;
var COMMAND_GET_LAMP   = 4;
var COMMAND_CTRL       = 5;
var COMMAND_SETLEVEL   = 6;
var COMMAND_SETCOLOR   = 7;
var COMMAND_SETKELVIN  = 8;

var UI_LIST         = 0;
var UI_LAMP_ONOFF   = 1;
var UI_LAMP_DIM     = 2;
var UI_LAMP_RGB     = 3;
var UI_LAMP_TW      = 4;

var NULL            = 0;

// ------------------------------------------------------------------
// Variables
// ------------------------------------------------------------------

var uimode      = UI_LIST;
var new_lvl     = 0;
var new_rgb     = 0;
var new_kelvin  = 0;
var prev_lvl    = 0;
var prev_rgb    = 0;
var prev_kelvin = 0;
var lamptype    = 0;
var lampmac     = 0;
var lamponoff   = 0;
var lampalt     = 1;   // RGB

var timerVar    = 0;
var delayedVar  = 0;
var ts          = 0;
var regular     = 0;
var interval    = 2;

var picker      = 0;
var twpicker    = 0;


var kelvintable;

var twCycle = 0;
var twTimerVar = 0;

// ------------------------------------------------------------------
// At startup
// ------------------------------------------------------------------

function onLoad( mac ) {
    console.log( "================ onload" );

    $('#javaerror').hide();

    kelvintable = new Array();
    var i = 0;
    for ( var k=1000; k<=10000; k+=100 ) {
        kelvintable[i++] = kelvin2rgbi( k );
    }

    // Instantiate color picker
    picker = new jQuery.ColorPicker('#colorpicker', {
        color: '#3d668a',
        change: colorCallBack
    });

    // Instantiate tunablewhite picker
    twpicker = new jQuery.TwPicker('#twpicker', {
        color: '#ffffff',
        change: twCallBack
    });

    $('#dimming').slider(dimmableCallBack);
    timerRestart();

    uimode = UI_LIST;
    showUi();
    showExtras();

    if ( mac == 0 ) getLampList();
    else getLampInfo( mac );
}

// ------------------------------------------------------------------
// Timer
// ------------------------------------------------------------------

function timerRestart() {
    if ( timerVar ) clearInterval( timerVar );
    // Avoid strict 2-sec pace
    var period = 1950 + Math.floor( Math.random() * 100 );
    timerVar = setInterval( LampUpdate, period );
}

// ------------------------------------------------------------------
// Extras
// ------------------------------------------------------------------

function showExtras() {
    var extra = "<br />";
    extra += "<input type='submit' value='Bbc-Cycle' style='color:";
    extra += ( twCycle == 1 ) ? "red" : "darkgrey";
    extra += "'; onclick='toggleCycle();'/>";
    document.getElementById('extras').innerHTML = extra;
}

function startCycle() {
    if ( twTimerVar ) clearInterval( twTimerVar );
    twTimerVar = setInterval( twStep, 1000 );
    twCycle = 1;
    showExtras();
}

function stopCycle() {
    if ( twTimerVar ) {
        clearInterval( twTimerVar );
        twTimerVar = 0;
    }
    twCycle = 0;
    showExtras();
}

function toggleCycle() {
    if ( twCycle == 0 ) startCycle(); else stopCycle();
    // console.log( twCycle );
}

function twStep() {
    var minKelvin = 1000;
    var maxKelvin = 10000;
    if ( uimode == UI_LAMP_TW ) {
        // TW
        minKelvin = 2700;
        maxKelvin = 6500;
    }
    new_kelvin = prev_kelvin;
    if ( new_kelvin < minKelvin ) new_kelvin = minKelvin;
    else {
        new_kelvin += ( new_kelvin < 3500 ) ? 100 : 200;
        if ( new_kelvin > maxKelvin ) new_kelvin = minKelvin;
    }

    // LampSetKelvin();
    gotoLamp( lamptype, lampmac, lamponoff, new_lvl, 0, new_kelvin, lampalt );
}

// ------------------------------------------------------------------
// UI
// ------------------------------------------------------------------

function showUi() {
    console.log( "showUi " + uimode );

    // Everything default off
    $('#twpick').hide();
    $('#colorpick').hide();
    $('#dimmable').hide();
    $('#tunablewhite').hide();
    $('#backarrow').hide();
    $('#onoff').hide();
    $('#lamplist').hide();
    $('#extras').hide();

    switch ( uimode ) {
    case UI_LIST:
        $('#lamplist').show();
        stopCycle();
        break;
    case UI_LAMP_ONOFF:
        $('#backarrow').show();
        $('#onoff').show();
        stopCycle();
        break;
    case UI_LAMP_DIM:
        $('#dimmable').show();
        $('#backarrow').show();
        $('#onoff').show();
        stopCycle();
        break;
    case UI_LAMP_RGB:
        if ( lampalt == 1 ) $('#colorpick').show();
        else $('#twpick').show();
        $('#backarrow').show();
        $('#onoff').show();
        $('#extras').show();
        break;
    case UI_LAMP_TW:
        $('#twpick').show();
        $('#backarrow').show();
        $('#onoff').show();
        $('#extras').show();
        break;
    }
};

// ---------------------------------------------------------------------
// Conversions
// ---------------------------------------------------------------------

function rgbi2rgbs( rgb ) {
    var r = ( rgb >> 16 ) & 0xFF;
    var g = ( rgb >> 8  ) & 0xFF;
    var b = ( rgb       ) & 0xFF;
    var rstr = r.toString(16); if ( rstr.length < 2 ) rstr = "0"+rstr;
    var gstr = g.toString(16); if ( gstr.length < 2 ) gstr = "0"+gstr;
    var bstr = b.toString(16); if ( bstr.length < 2 ) bstr = "0"+bstr;
    return rstr + gstr + bstr;
}

function rgbi2kelvin( rgbi ) {
    var red   = ( rgbi >> 16 ) & 0xFF;
    var green = ( rgbi >> 8  ) & 0xFF;
    var blue  = ( rgbi       ) & 0xFF;

    // console.log( "Find kelvin", rgbi, typeof rgbi, red, green, blue );

    var i, dr, dg, db, dsum, dbest = 3*0xFF, index = 0;
    for ( i=0; i<kelvintable.length; i++ ) {
        var rgbi = kelvintable[i];
        var r = ( rgbi >> 16 ) & 0xFF;
        var g = ( rgbi >> 8  ) & 0xFF;
        var b = ( rgbi       ) & 0xFF;
        dr = red   - r; if ( dr < 0 ) dr = -dr;
        dg = green - g; if ( dg < 0 ) dg = -dg;
        db = blue  - b; if ( db < 0 ) db = -db;
        dsum = dr + dg + db;
        // console.log( "K[" + i + "] = " + rgbi, dsum, dbest, index );
        if ( dsum < dbest ) {
           dbest = dsum;
           index = i;
        }
    }

    return( 1000 + index*100 );
}

// ---------------------------------------------------------------------
// ---------------------------------------------------------------------

function gotoLamp( type, mac, onoff, lvl, rgb, kelvin, alt ) {
    console.log('gotoLamp ' + mac + ', type=' + type + ', rgb='+rgb +
                ', kelvin='+kelvin + ', alt='+alt);

    lamptype  = type;
    lampmac   = mac;
    lamponoff = onoff;
    lampalt   = alt;

    switch ( type ) {
    case 1: uimode = UI_LAMP_ONOFF;
        break;
    case 2: uimode = UI_LAMP_DIM;
        // console.log('Set level to ' + lvl);
        var dimming = $.slider('#dimming');
        // console.log( 'lvl = '+lvl );
        dimming.setLevel(lvl / 100);
        $('#level').val(lvl / 100);
        break;
    case 3: uimode = UI_LAMP_RGB;
        if ( rgb == 0 && kelvin > 0 ) {
            rgb = kelvin2rgbi( kelvin );
        }
        var rgbstr = rgbi2rgbs( rgb );
        // console.log('Set RGB to ' + rgb + ', ' + rgbstr );
        if ( lampalt == 1 ) {
            picker.set(rgbstr,lvl);
        } else {
            if ( kelvin == 0 ) {
                kelvin = rgbi2kelvin( rgb );
            }
            twpicker.kelvins(1000,10000);
            twpicker.set(kelvin,lvl);
        }
        break;
    case 4: uimode = UI_LAMP_TW;
        // console.log('Set TW to ' + kelvin );
        twpicker.kelvins(2700,6500);
        twpicker.set(kelvin,lvl);
        break;
    }
    console.log( "gl- showUi " + uimode );
    showUi();
    if ( lamponoff == 1 ) {
        document.getElementById('onoff').innerHTML =
             "<img src='/img/on.png' width='36'/>";
    } else {
        document.getElementById('onoff').innerHTML =
             "<img src='/img/off.png' width='36'/>";
    }

    document.getElementById('fieldlegend').innerHTML = 'Lamp - '+ mac; 
}

// ------------------------------------------------------------------
// Callbacks
// ------------------------------------------------------------------

function dimmableCallBack( lvl ) {
    // Stop delayed timer
    timerRestart();
    if ( delayedVar ) clearTimeout( delayedVar );
    // Set the level input field
    $('#level').val(lvl);
    // Prepare new color for delayed timer
    new_lvl = Math.round( lvl * 100 );
    // console.log('new_lvl = ' + new_lvl );
    // Restart delayed timer
    delayedVar = setTimeout( LampSetLevel, 66 );
};

function colorCallBack( color, lvl ) {
    console.log( "colorCallBack( " + color + ", " + lvl + " )" );

    // Stop delayed timer
    timerRestart();
    if ( delayedVar ) clearTimeout( delayedVar );

    // Give a color preview
    $('#preview').css('background', color);
    $('#preview').html("RGB "+color+", level "+lvl);
    $('#hex_input').val(color);

    // Prepare new color for delayed timer
    newcolor = color.substr(1);
    // console.log('newcolor = ' + newcolor );
    var r = parseInt( newcolor.substr(0,2), 16 );
    var g = parseInt( newcolor.substr(2,2), 16 );
    var b = parseInt( newcolor.substr(4,2), 16 );
    // console.log('r=' + r + ', g=' + g + ', b=' + b );
    new_rgb = ( r << 16 ) + ( g << 8 ) + b;
    new_lvl = lvl;

    // Restart delayed timer
    delayedVar = setTimeout( LampSetColor, 66 );
};

function twCallBack( kelvin, color, lvl ) {
    console.log( "twCallBack( " + kelvin + ", " + color + ", " + lvl + " )" );

    // Stop delayed timer
    timerRestart();
    if ( delayedVar ) clearTimeout( delayedVar );

    // Give a color preview
    $('#twpreview').css('background', color);
    $('#twpreview').html("Kelvin "+ Math.round(kelvin) +", level "+lvl);

    // Prepare new kelvin for delayed timer
    new_kelvin = kelvin;
    newcolor = color.substr(1);
    // console.log('newcolor = ' + newcolor );
    var r = parseInt( newcolor.substr(0,2), 16 );
    var g = parseInt( newcolor.substr(2,2), 16 );
    var b = parseInt( newcolor.substr(4,2), 16 );
    // console.log('r=' + r + ', g=' + g + ', b=' + b );
    new_rgb = ( r << 16 ) + ( g << 8 ) + b;
    new_lvl    = lvl;

    // Restart delayed timer
    delayedVar = setTimeout( LampSetKelvin, 66 );
};

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
    xmlhttp.open( 'POST', 'iot_lamp.cgi', true );
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
// Error handling
// ------------------------------------------------------------------

function printError( command, err ) {
    var errText = 'Command '+command+', error ';
    switch ( parseInt( err ) ) {
    case 1:
        errText += "1: No MAC specified";
        break;
    case 2:
        errText += "2: Unable to open database";
        break;
    case 3:
        errText += "3: Device not found";
        break;
    case 4:
        errText += "4: Unable to communicate with Control Interface";
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
// Table view
// ------------------------------------------------------------------

function htmlTable( table ) {
    var html = "";

    var rows = table.split( ";" );
    for ( var r=1; r<rows.length; r++ ) {   // Skip header row
        var cols = rows[r].split( "," );

        var mac    = cols[COLUMN_DEVS_MAC];
        var name   = cols[COLUMN_DEVS_NM];
        var ty     = cols[COLUMN_DEVS_TY];
        var cmd    = cols[COLUMN_DEVS_CMD];
        var lvl    = cols[COLUMN_DEVS_LVL];
        var rgb    = parseInt( cols[COLUMN_DEVS_RGB] );
        var kelvin = parseInt( cols[COLUMN_DEVS_KELVIN] );
        var upd    = cols[COLUMN_DEVS_LASTUPDATE];

        // console.log( r, rgb, kelvin );

        html += "<div class='lamp'>\n";
        html += "<div class='lamp_col1'>\n";
        html += mac+"</br />\n";
        html += ty + " - " + (( name ) ? name : "No name" )+"</br />\n";
        html += timeConv( upd )+"</br />\n";
        html += "</div>\n";

        var type = 1;   // on/off
        if ( ty == "dim" ) type = 2;
        if ( ty == "col" ) type = 3;
        if ( ty == "tw"  ) type = 4;

        var img1 = 0, img2 = 0;
        if ( type > 1 ) {
            html += "<div class='lamp_col2'>\n";
            html += "L=" + ( ( lvl ) ? lvl : "0" )+"\n";
            html += "</div>\n";
            img2 = "/img/greybar.png";
        }

        if ( type == 3 ) {
            if ( rgb == 0 && kelvin > 0 ) {
                rgb = kelvin2rgbi( kelvin );
            }
            html += "<div class='lamp_col3'>\n";
            html += "C=" + ( ( rgb ) ? rgbi2rgbs(rgb) : "0" ) + "\n";
            html += "</div>\n";
            img1 = "/img/jquery.colorpicker.wheel.png";
            img2 = "/img/tw.png";
        }

        if ( type == 4 ) {
            html += "<div class='lamp_col3'>\n";
            html += "K=" + kelvin + "\n";
            html += "</div>\n";
            img2 = "/img/tw.png";
        }

        var onoff = ( cmd == "on" );

        if ( img1 ) {
            html += "<div class='lamp_col4'>\n";
            html += "<img src='" + img1 + "' height='36'";
            html += " onclick='gotoLamp(" + type + ",\""+mac+"\","+onoff+","+lvl+","+rgb+","+kelvin+",1)' />";
            html += "</div>\n";
        }

        if ( img2 ) {
            html += "<div class='lamp_col5'>\n";
            html += "<img src='" + img2 + "' height='36'";
            html += " onclick='gotoLamp(" + type + ",\""+mac+"\","+onoff+","+lvl+","+rgb+","+kelvin+",2)' />";
            html += "</div>\n";
        }

        html += "<div class='lamp_col6'>\n";
        if ( onoff ) {
            html += "<img src='/img/on.png' height='36'";
            html += " onclick='ctrlLamp(\""+mac+"\",0)' />";
        } else {
            html += "<img src='/img/off.png' height='36'";
            html += " onclick='ctrlLamp(\""+mac+"\",1)' />";
        }
        html += "</div>\n";

        html += "</div>\n";
        html += "<br />\n";
    }

    return( html );
}

// ------------------------------------------------------------------
// Set
// ------------------------------------------------------------------

function LampSetLevel() {
    if ( lampmac != 0 ) {
        console.log('SETLEVEL: ' + new_lvl);
        var cmd = 'command='+COMMAND_SETLEVEL+'&mac='+lampmac+ '&lvl='+new_lvl;
        // console.log( 'cmd = ' + cmd );
        doHttp( cmd,
            function( xmlhttp ) {
                var xmlDoc = xmlhttp.responseXML;
                var err = getXmlVal( xmlDoc, 'err' );
                if ( err == 0 ) console.log('new level set');
                else printError( COMMAND_SETLEVEL, err );
            } );
    }
}

function LampSetColor() {
    if ( ( lampmac != 0 ) &&
        ( ( new_rgb != prev_rgb ) || ( new_lvl != prev_lvl ) ) ) {
        console.log( 'SETCOLOR: mac='+lampmac+
                     ', rgb='+new_rgb+', lvl='+new_lvl );
        prev_rgb    = new_rgb;
        prev_lvl    = new_lvl;
        prev_kelvin = rgbi2kelvin( new_rgb );
        var cmd = 'command='+COMMAND_SETCOLOR+'&mac='+lampmac+
                    '&kelvin=-1&rgb='+new_rgb+'&lvl='+new_lvl;
        // console.log( 'cmd = ' + cmd );
        doHttp( cmd,
            function( xmlhttp ) {
                var xmlDoc = xmlhttp.responseXML;
                // var str = xml2Str (xmlDoc);
                // console.log('xml='+str );
                var err = getXmlVal( xmlDoc, 'err' );
                if ( err == 0 ) console.log('new color set');
                else printError( COMMAND_SETCOLOR, err );
            } );
    }
}

function LampSetKelvin() {
    if ( ( lampmac != 0 ) &&
        ( ( new_kelvin != prev_kelvin ) || ( new_lvl != prev_lvl ) ) ) {
        console.log( 'SETKELVIN: mac='+lampmac+
                     ', kelvin='+new_kelvin+', lvl='+new_lvl );
        // prev_rgb    = kelvin2rgbi( new_kelvin );
        prev_kelvin = new_kelvin;
        prev_lvl    = new_lvl;
        var cmd = 'command='+COMMAND_SETKELVIN+'&mac='+lampmac+
//                    '&kelvin='+new_kelvin+'&rgb='+new_rgb+'&lvl='+new_lvl;
                    '&kelvin='+new_kelvin+'&rgb=-1&lvl='+new_lvl;
        // console.log( 'cmd = ' + cmd );
        doHttp( cmd,
            function( xmlhttp ) {
                var xmlDoc = xmlhttp.responseXML;
                // var str = xml2Str (xmlDoc);
                // console.log('xml='+str );
                var err = getXmlVal( xmlDoc, 'err' );
                if ( err == 0 ) console.log('new kelvin set');
                else printError( COMMAND_SETKELVIN, err );
            } );
    }
}

// ------------------------------------------------------------------
// Execute command
// ------------------------------------------------------------------

function LampExec( post ) {
    console.log('LampExec : '+post);
    doHttp( post,
        function( xmlhttp ) {
            var xmlDoc = xmlhttp.responseXML;
            // var str = xml2Str (xmlDoc);
            // console.log('xml='+str );

            var err = getXmlVal( xmlDoc, 'err' );
            var cmd = getXmlVal( xmlDoc, 'cmd' );
	    for ( i=0; i<10; i++ ) {
                var dbg = getXmlVal( xmlDoc, 'dbg'+i );
                if ( dbg != -1 ) console.log('dbg='+dbg );
                console.log('dbg='+dbg );
	    }
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
                    if ( ( ts2 != ts ) || ( regular <= 0 ) ) {
                        if ( regular <= 0 ) {
                            console.log('============= GET REGULAR ======' );
                            regular = 60;
                        } else {
                            console.log('============= TABLE CHANGED ======' );
                            ts = ts2;
                            if ( uimode == UI_LIST ) {
                                getLampList();
                            } else {
                                getLampInfo( lampmac );
                            }
                        }
                    }
                    break;

                case COMMAND_GET_LIST:
                    var list = getXmlVal( xmlDoc, 'list' );
                    console.log('list='+list );
                    var html = htmlTable( list );
                    document.getElementById('lamplist').innerHTML = html;
                    uimode = UI_LIST;
                    showUi();
                    document.getElementById('fieldlegend').innerHTML = 'Lamps';
                    var numrows = list.split( ";" ).length;
                    if ( numrows > 1 ) $('#groups').show();
                    else $('#groups').hide();
                    break;

                case COMMAND_GET_LAMP:
                    var onoff, lvl, rgb, kelvin, lastkelvin;
                    mac        = getXmlVal( xmlDoc, 'mac' );
                    type       = parseInt( getXmlVal( xmlDoc, 'type' ) );
                    onoff      = parseInt( getXmlVal( xmlDoc, 'onoff' ) );
                    lvl        = parseInt( getXmlVal( xmlDoc, 'lvl' ) );
                    rgb        = parseInt( getXmlVal( xmlDoc, 'rgb' ) );
                    kelvin     = parseInt( getXmlVal( xmlDoc, 'kelvin' ) );
                    lastkelvin = parseInt( getXmlVal( xmlDoc, 'lastkelvin' ) );
                    // console.log( 'GET_LAMP: onoff=' + onoff +
                    //    ', lvl='+lvl + ', rgb='+rgb + ', kelvin='+kelvin );
                    gotoLamp( type, mac, onoff,
                              lvl, rgb, kelvin, lampalt );
                    break;
                }
            } else printError( cmd, err );
        } );
}

// ------------------------------------------------------------------
// Commands
// ------------------------------------------------------------------

function LampUpdate() {
    // console.log('LampUpdate '+regular);
    regular -= interval;
    var post = 'command='+COMMAND_TSCHECK;
    LampExec( post );
}

function getLampList() {
    LampExec( 'command='+COMMAND_GET_LIST );
}

function getLampInfo( mac ) {
    LampExec( 'command='+COMMAND_GET_LAMP+'&mac='+mac );
}

function ctrlLamp( mac, onoff ) {
    console.log('ctrlLamp, mac='+mac+', onoff='+onoff);
    var post = 'command='+COMMAND_CTRL+'&mac='+mac+'&cmd=';
    post += ( onoff ) ? 'on' : 'off';
    if ( onoff == 0 ) stopCycle();
    doHttp( post,
        function( xmlhttp ) {
            var xmlDoc = xmlhttp.responseXML;
            var err = getXmlVal( xmlDoc, 'err' );
            if ( err == 0 ) {
                switch ( uimode ) {
                case UI_LIST:
                    getLampList();
                    break;
                default:
                    getLampInfo( mac );
                    break;
                }
            } else printError( COMMAND_CTRL, err );
        } );
}

function toggleLamp() {
    var newonoff = ( lamponoff == 1 ) ? 0 : 1;
    ctrlLamp( lampmac, newonoff );
}

// ------------------------------------------------------------------
// Misc
// ------------------------------------------------------------------

function goBack( really ) {
    if ( really ) window.history.back();
    else getLampList();
}

