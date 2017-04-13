// ------------------------------------------------------------------
// CGI program - Mobile
// ------------------------------------------------------------------
// Javascript for the IoT Mobile webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

// ------------------------------------------------------------------
// Variables
// ------------------------------------------------------------------

// State machine related
var STATE_DEVICE_OVERVIEW  = 1;
var STATE_SINGLE_DEVICE    = 2;
var STATE_CONFIRM_SHUTDOWN = 3;
var STATE_SHUTTING_DOWN    = 4;
var STATE_SHUTDOWN_READY   = 5;
var currentState = STATE_DEVICE_OVERVIEW;
var currentDeviceIdentifier;
var justIntoDeviceUI = false;
var devs= "";


var timerVar   = 0;
var ts         = 0;
var lastlus    = 0;
var lastlus2   = 0;
var lastlus3   = 0;

var data       = new Array();

// Systme table related!
var  SYS_COLUMN_ID          = 0;
var  SYS_COLUMN_NAME        = 1;
var  SYS_COLUMN_INTVAL      = 2;
var  SYS_COLUMN_STRVAL      = 3;
var  SYS_COLUMN_LASTUPDATE  = 4;

var selectedNFCMode = 2; // Init value which is not updated by user yet!

var pollingSec = 2;
var timerVar   = 0;
var ts         = 0;
var queueSkip  = 0;
var installing = 0;
var progress   = 0;
var blocktopo  = -1;
var topotimer  = 0;

// Lamps related!
var UI_LIST         = 0;
var UI_LAMP_ONOFF   = 1;
var UI_LAMP_DIM     = 2;
var UI_LAMP_RGB     = 3;
var UI_LAMP_TW      = 4;

var uimode      = UI_LIST;
var new_lvl     = 0;
var new_rgb     = 0;
var new_kelvin  = 0;
var lastkelvin  = 0;
var prev_lvl    = 0;
var prev_rgb    = 0;
var prev_kelvin = 0;
var lamptype    = 0;
var lampmac     = 0;
var lamponoff   = 0;
var lampalt     = 1;   // RGB

var delayedVar  = 0;

var picker      = 0;
var twpicker    = 0;

// End of Lamps related


var imageLoaded     = 0;
var prevImageLoaded = 0;
var imageLoader     = 0;
var imageLoaderCnt  = 0;

var COMMAND_NORMAL          = 0;
var COMMAND_TSCHECK         = 1;
//var COMMAND_GET_TABLE       = 2;
var COMMAND_GET_DEVS        = 3;
var COMMAND_GET_PLUG        = 4
var COMMAND_CTRL            = 5;
var COMMAND_SETLEVEL        = 6;
var COMMAND_SETCOLOR        = 7;
var COMMAND_SETKELVIN       = 8;
var COMMAND_SET_NFCMODE     = 9;
var COMMAND_GET_SYSTEMTABLE = 10;
var COMMAND_SHUTDOWN        = 11;

var NULL               = 0;

// Plug related
var yesterdaydata = 0;
var plugonoff = 0;

// ------------------------------------------------------------------
// On page load
// ------------------------------------------------------------------

function onLoad() {

    var html;

    console.log( "onLoad()" );

    document.getElementById('javaerror').style.display = 'none';

    document.getElementById('mobile_header').innerHTML = addTopRowButtons();

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


    for ( var i=0; i<2; i++ ) {
//        lus[i] = -1; //Disable this due to error in JS console
        data[i] = new Array();
    }

    currentState = STATE_DEVICE_OVERVIEW;
    drawContent();
    updateContent();
    getSystemTable();
    timerRestart();
}

// ------------------------------------------------------------------
// Timer
// ------------------------------------------------------------------

function timerRestart() {
    if ( timerVar ) clearInterval( timerVar );
    // Avoid strict 2-sec pace
    var period = 1950 + Math.floor( Math.random() * 100 );
    timerVar = setInterval( mobileUpdate, period );
}

function timerStop() {
    if ( timerVar ) {
        clearInterval( timerVar );
        timerVar = 0;
    }
}

function mobileUpdate() {
    var post = 'command='+COMMAND_TSCHECK;
    MobileExec( post );
}

function updateContent() {
    var post;
    post = 'command=' + COMMAND_GET_DEVS;
    MobileExec(post);
}

function addTopRowButtons() {
    var html;

    html = '<p>';
    html += '<label id="but_shutdown"     class="navbutton" onclick="gotoShutdownConfirm()";>' + 'SHUTDOWN' + '</label>';
    html += '<label id="but_commission"   class="navbutton" onclick="gotoCommission()";>' + 'Commission' + '</label>';
    html += '<label id="but_decommission" class="navbutton" onclick="gotoDecommission()";>' + 'Decommission' + '</label>';
    html += '<label id="but_nextdev"      class="navbutton" onclick="gotoNextDevice()";>Next device</label>';
    html += '<label id="but_overview"     class="navbutton" onclick="gotoDeviceOverview()";>Overview</label>';
    html += '</p>';

    return html;
}


function drawNode(Dev_ID, Node_Image, Node_Name, Node_MAC_Address, Node_Disabled) {
    var html;

    console.log(Dev_ID);

    if (Node_Disabled == true) {
        html = '<table class="table_disabled">';
    }
    else {
        html = '<table>';
    }

    html += '<tr>';
    html += '<td width="96"> <img id="' + Dev_ID + '" ' + 'src=' + '"/img/' + Node_Image + '" ' + 'style="width:96px;height:96px"';
    html += ' onclick="gotoDevice(' + Dev_ID + ');" ';
    html += '/> </td>';
    html += '<td>' + '<p>' + Node_Name + '</p>';
    html += '<p>' + Node_MAC_Address + '</p>';
    html += '</td>';
    html += '</tr>';
    html += '</table>';

    html += '<p> </p>';

    return html;
}

// ------------------------------------------------------------------
// Callbacks
// ------------------------------------------------------------------

function dimmableCallBack( lvl ) {
    console.log( "dimmable" );
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

function colorCallBack( kelvin, color, lvl, lastk ) {
    console.log( "colorCallBack( " + kelvin + ", " + color + ", " + lvl + ", " + lastk + " )" );

    if (justIntoDeviceUI == true) {
        justIntoDeviceUI = false;
    } else {
        // Stop delayed timer
        timerRestart();
        if ( delayedVar ) clearTimeout( delayedVar );

        // Give a color preview
        //$('#preview').css('background', color);
        //$('#preview').html("RGB "+color+", level "+lvl);

        // Prepare new color for delayed timer
        new_kelvin = kelvin;
        lastkelvin = lastk;
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
    }
};

function twCallBack( kelvin, color, lvl ) {
    console.log( "twCallBack( " + kelvin + ", " + color + ", " + lvl + " )" );

    if (justIntoDeviceUI == true) {
        justIntoDeviceUI = false;
    } else {
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
    }
};

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

//var plugAct, plugSum; // Forcefully set these two as global so that it will be updated by GetPlug

function drawContent() {
    var currentDeviceRow;
    var html;

    console.log('Draw UI Now');

    $('#twpick').hide();
    $('#colorpick').hide();
    $('#onoff').hide();
    $('#hourchart').hide();
    $('#plugonoff').hide();
    $('#dimmable').hide();

    switch (currentState) {
    case STATE_DEVICE_OVERVIEW:
        document.getElementById('but_shutdown').style.display = 'inline-block';
        document.getElementById('but_commission').style.display = 'inline-block';
        document.getElementById('but_decommission').style.display = 'inline-block';
        document.getElementById('but_nextdev').style.display = 'none';
        document.getElementById('but_overview').style.display = 'none';
        // Use global devs to draw overview
        showCommissionSwitch();
        html = htmlDevTable(devs);
        document.getElementById('mobile_content').innerHTML = html;
    break;
    
    case STATE_SINGLE_DEVICE:
        document.getElementById('but_commission').style.display = 'none';
        document.getElementById('but_decommission').style.display = 'none';
        document.getElementById('but_nextdev').style.display = 'inline-block';
        document.getElementById('but_overview').style.display = 'inline-block';
        // Use global devs and dev_id to draw device specifics
        // e.g if device == LAMP, then draw ERGB and/or Kelvin
        // if == plug, then draw act/sum
        currentDeviceRow = getCurrentDeviceByIdentifer(currentDeviceIdentifier, devs);
        var devName, devType, lampKelvinValue, lampRGBValue;
        var plugAct = 0, plugSum = 0;
        var cols = currentDeviceRow.split(',');

        var mac    = cols[COLUMN_DEVS_MAC];
        var name   = cols[COLUMN_DEVS_NM];
        var ty     = cols[COLUMN_DEVS_TY];
        var cmd    = cols[COLUMN_DEVS_CMD];
        var lvl    = cols[COLUMN_DEVS_LVL];
        var rgb    = parseInt( cols[COLUMN_DEVS_RGB] );
        var kelvin = parseInt( cols[COLUMN_DEVS_KELVIN] );
        var upd    = cols[COLUMN_DEVS_LASTUPDATE];

        var type = 1;   // on/off
        if ( ty == "dim" ) type = 2;
        if ( ty == "col" ) type = 3;
        if ( ty == "tw"  ) type = 4;

        var onoff = (cmd == "on");

        switch (parseInt(cols[COLUMN_DEVS_DEV])) {
        case DEVICE_DEV_LAMP:
            devName = 'Lamp';
            lampmac = mac;
            lamponoff = onoff;

            if (ty == TYPE_DIM) {
                devType = 'Dimmable Lamp';
            } else if (ty == TYPE_TW) {
                devType = 'Kelvin Lamp';
                lampKelvinValue = cols[COLUMN_DEVS_KELVIN];
            } else if (ty == TYPE_COL) {
                devType = 'RGB Lamp';
                lampKelvinValue = cols[COLUMN_DEVS_KELVIN];
                lampRGBValue = cols[COLUMN_DEVS_RGB];
            }

            break;

        case DEVICE_DEV_PLUG:
            devName = 'Plug';
            plugmac = mac;

            plugAct = cols[COLUMN_DEVS_ACT];
            plugSum = cols[COLUMN_DEVS_SUM];
            break;
        }

        if (devName == 'Lamp') {
            if (devType == 'RGB Lamp') {
                document.getElementById('fieldlegend').innerHTML = 'RGB Lamp';
                document.getElementById('mobile_content').innerHTML = '';

                if ( rgb == 0 && kelvin > 0 ) {
                    rgb = kelvin2rgbi( kelvin );
                }
                var rgbstr = rgbi2rgbs( rgb );

                picker.kelvins(1000,10000);
                picker.set(kelvin,rgbstr,lvl);

                $('#colorpick').show();

            } else if (devType == 'Kelvin Lamp') { // TW lamp
                document.getElementById('fieldlegend').innerHTML = 'Color Tunable Lamp';
                document.getElementById('mobile_content').innerHTML = '';

                twpicker.kelvins(2700,6500);
                twpicker.set(kelvin,lvl);

                $('#twpick').show();
//                $('#extras').show();
                
            } else if (devType == 'Dimmable Lamp') {

                document.getElementById('fieldlegend').innerHTML = 'Dimmable Lamp';
                document.getElementById('mobile_content').innerHTML = '';

                var dimming = $.slider('#dimming');
                // console.log( 'lvl = '+lvl );
                dimming.setLevel(lvl / 100);
                $('#dimmable').show();
                
            } else {
                document.getElementById('fieldlegend').innerHTML = 'It is a ' + devName + ' of type ' + ty;
                document.getElementById('mobile_content').innerHTML = '';
            }

            // All lamps have an on/off
            if (onoff == true) {
                document.getElementById('onoff').innerHTML = "<img src='/img/on.png' width='36'/>";
            } else {
                document.getElementById('onoff').innerHTML = "<img src='/img/off.png' width='36'/>";
            }
            $('#onoff').show();
            
        } else { // Smart plug
            getPlugInfo(mac);

            document.getElementById('fieldlegend').innerHTML = 'Smart Plug';
            html = '<fieldset>';
            html += '<legend> Current energy consumpiton </legend>';
            html += '<h2 align="center">' + plugAct + ' watt' + '</h2>';
            html += '</fieldset>';

            html += '<p> </p>';

            html += '<fieldset>';
            html += '<legend> Cumulative energy consumption </legend>';
            html += '<h2 align="left">' + 'Last day: ' + yesterdaydata + ' Wh' + '</h2>';
            html += '<h2 align="left">' + 'Total: ' + plugSum + ' Wh' + '</h2>';
            html += '</fieldset>';

            html += '<p> </p>';

            $('#hourchart').show();
            $('#plugonoff').show();
            document.getElementById('mobile_content').innerHTML = html;

        }
        break;
    
    case STATE_CONFIRM_SHUTDOWN:
        document.getElementById('but_shutdown').style.display = 'none';
        document.getElementById('but_commission').style.display = 'none';
        document.getElementById('but_decommission').style.display = 'none';
        document.getElementById('but_nextdev').style.display = 'none';
        document.getElementById('but_overview').style.display = 'inline-block';
        html = '<h1 align="center">Are you sure you want to shutdown the system?</h1>';
        html += '<p align="center">';
        html += '<label id="sd_ok" class="navbutton" onclick="gotoShuttingDown()";>' + 'OK' + '</label>';
        html += '<label id="sd_cancel" class="navbutton" onclick="gotoDeviceOverview()";>' + 'Cancel' + '</label>';
        html += '</p>';
        document.getElementById('mobile_content').innerHTML = html;
    break;
    
    case STATE_SHUTTING_DOWN:
        document.getElementById('but_shutdown').style.display = 'none';
        document.getElementById('but_commission').style.display = 'none';
        document.getElementById('but_decommission').style.display = 'none';
        document.getElementById('but_nextdev').style.display = 'none';
        document.getElementById('but_overview').style.display = 'none';
        html = '<h1 align="center">Shutting down</h1>';
        html += '<p align="center">';
        html += '<img src="/img/spinner.gif" width="100"/>';
        html += '</p>';
        document.getElementById('mobile_content').innerHTML = html;
        MobileExec( "command="+COMMAND_SHUTDOWN );
    break;
    
    case STATE_SHUTDOWN_READY:
    html = '<h1 align="center">It is now safe to switch off the device</h1>';
        document.getElementById('mobile_content').innerHTML = html;
    break;
    }
}

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------

function checkImageLoading() {
    // console.log( "checkImageLoading " + imageLoaderCnt + ", " + imageLoaded );
    if ( imageLoaded != prevImageLoaded ) {
        prevImageLoaded = imageLoaded;
        drawAll();
    } else if ( imageLoaderCnt++ > 10 ) {
        clearInterval( imageLoader );
    }
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
    xmlhttp.open( 'POST', 'iot_mobile.cgi', true );
    xmlhttp.setRequestHeader( 'Content-type',
                            'application/x-www-form-urlencoded' );
    console.log( posts );
    xmlhttp.send( posts );
}

// ------------------------------------------------------------------
// Error handling
// ------------------------------------------------------------------

function printError( err ) {
    var errText = 'Error ';
    console.log(err);
    switch ( parseInt( err ) ) {
    case 1:
        errText += "Unable to open database";
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

function htmlDevTable( table ) {
    var html = "<p> </p>";
    var Dev_ID, Node_Image, Node_Name, Node_MAC_Address, Node_Disabled;

    console.log(table);
    var rows = table.split( ";" );

    for ( var r = 1; r < rows.length; r++ ) {
        var cols = rows[r].split( "," );

        for ( var c = 0; c < cols.length; c++ ) {
            if ( c == COLUMN_DEVS_DEV ) {
                var col = "red";
                var dev = cols[c];
                switch ( parseInt( cols[c] ) ) {
                case DEVICE_DEV_LAMP:
                    col = "yellow";
                    dev = "lamp";
                    Node_Image = "device_lamp.png";
                    Node_Name = "Lamp";
                    break;
                case DEVICE_DEV_PLUG:
                    col = "darksalmon";
                    dev = "plug";
                    Node_Image = "device_plugmeter.png";
                    Node_Name = "Smart Plug";
                    break;
                }

            } else if (c == COLUMN_DEVS_ID) {
                Dev_ID = cols[c];
            } else if ( c == COLUMN_DEVS_TY ) {
                //html += "<td>" + rgbi2rgbs( cols[c] ) + "</td>";
                if (Node_Image == "device_lamp.png") {
                    if (cols[c] == TYPE_TW) {
                        Node_Image = "device_lamp_tw.png";
                        Node_Name = "Color Tunable Lamp";
                    } else if (cols[c] == TYPE_COL) {
                        Node_Image = "device_lamp_rgb.png";
                        Node_Name = "RGB Lamp";
                    } else if (cols[c] == TYPE_ONOFF) {
                        Node_Name = "On/Off Lamp";
                    } else if (cols[c] == TYPE_DIM) {
                        Node_Name = "Dim Lamp";
                    }
                }
            } else if (c == COLUMN_DEVS_FLAGS) {
                if (cols[c] & FLAG_DEV_JOINED)
                    Node_Disabled = false;
                else
                    Node_Disabled = true;
            } else if (c == COLUMN_DEVS_MAC) {
                Node_MAC_Address = cols[c];
            } else if ( cols[c] == 'NULL' ) {
                //html += "<td>-</td>";
            } else if ( c==(cols.length-1) ) {
                //html += "<td>"+timeConv(cols[c])+"</td>";
            } else {
                //html += "<td>"+cols[c]+"</td>";
            }
        }

        console.log('DevID = ' + Dev_ID);

        html += drawNode(Dev_ID, Node_Image, Node_Name, Node_MAC_Address, Node_Disabled);
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
        MobileExec( cmd );
    }
}

function LampSetColor() {
    console.log('Last Kelvin = '+lastkelvin);
    if ( lastkelvin ) {
        console.log('Set Kelvin in setColor');
        LampSetKelvin();
    } else if ( ( lampmac != 0 ) && ( ( new_rgb != prev_rgb ) || ( new_lvl != prev_lvl ) ) ) {
        console.log( 'SETCOLOR: mac='+lampmac+
                    ', rgb='+new_rgb+', lvl='+new_lvl );
        prev_rgb    = new_rgb;
        prev_lvl    = new_lvl;
        //prev_kelvin = rgbi2kelvin( new_rgb );
        var cmd = 'command='+COMMAND_SETCOLOR+'&mac='+lampmac+
                    '&kelvin=-1&rgb='+new_rgb+'&lvl='+new_lvl;
        MobileExec( cmd );
    }
}

function LampSetKelvin() {
    console.log("lampmac="+lampmac);
    console.log("new_kelvin="+new_kelvin);
    console.log("prev_kelvin="+prev_kelvin);
    console.log("new_lvl="+new_lvl);
    console.log("prev_lvl="+prev_lvl);

    if ( ( lampmac != 0 ) && ( ( new_kelvin != prev_kelvin ) || ( new_lvl != prev_lvl ) ) ) {
        console.log( 'SETKELVIN: mac='+lampmac+
                    ', kelvin='+new_kelvin+', lvl='+new_lvl );
        // prev_rgb    = kelvin2rgbi( new_kelvin );
        prev_kelvin = new_kelvin;
        prev_lvl    = new_lvl;
        var cmd = 'command='+COMMAND_SETKELVIN+'&mac='+lampmac+
//                    '&kelvin='+new_kelvin+'&rgb='+new_rgb+'&lvl='+new_lvl;
                    '&kelvin='+new_kelvin+'&rgb=-1&lvl='+new_lvl;
        // console.log( 'cmd = ' + cmd );
        MobileExec( cmd );
    }
}

function getLampInfo( mac ) {
    LampExec( 'command='+COMMAND_GET_LAMP+'&mac='+mac );
}

function ctrlLamp( mac, onoff ) {
    console.log('ctrlLamp, mac='+mac+', onoff='+onoff);
    var cmd = 'command='+COMMAND_CTRL+'&mac='+mac+'&cmd=';
    cmd += ( onoff ) ? 'on' : 'off';
    MobileExec( cmd );
}

function toggleLamp() {
    var newonoff = ( lamponoff == true ) ? 0 : 1;
    ctrlLamp( lampmac, newonoff );
}

function ctrlPlug( mac, onoff ) {
    console.log('ctrlPlug, mac='+mac+', onoff='+onoff);
    var cmd = 'command='+COMMAND_CTRL+'&mac='+mac+'&cmd=';
    cmd += ( onoff ) ? 'on' : 'off';
    MobileExec( cmd );
}

function togglePlug() {
    var newonoff = ( plugonoff == 1 ) ? 0 : 1;
    ctrlPlug( plugmac, newonoff );
}

// ------------------------------------------------------------------
// Chart
// ------------------------------------------------------------------

function drawPlugChart( hourdata ) {
    console.log('hourdata = ' + hourdata);
    var nums  = hourdata.split(',');
    var line  = new Array();
    var delta = new Array();
    var x     = new Array();
    line[0]   = 'Min Data';
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
        size: {
            height: 200
        },
        data: {
            x: 'x',
            columns: [
                x, line
            ],
            axes: {
            //    Delta: 'y2'
                'Min Data': 'y2'
            },
            types:     {
                // Delta: 'bar'
                'Min Data': 'bar'
            }
        },
        axis: {
            y: {
                show: false,
                label: {
                    text: 'Cumulative hour data [Wh]',
                    position: 'outer-middle'
                }
            },
            y2: {
                show: true,
                label: {
                    //text: 'Delta [Wh]',
                    position: 'outer-middle'
                }
            }
        }
    });
}



function getPlugInfo( mac ) {
    MobileExec( 'command='+COMMAND_GET_PLUG+'&mac='+mac );
}

function getSystemTable() {
    MobileExec( 'command='+COMMAND_GET_SYSTEMTABLE );
}

// ------------------------------------------------------------------
// Mobile command execution
// ------------------------------------------------------------------

function MobileExec( post ) {
    console.log('MobileExec : '+post);
    document.getElementById('debug').innerHTML = "";
    doHttp( post,
        function( xmlhttp ) {
            var xmlDoc = xmlhttp.responseXML;
            var str = xml2Str (xmlDoc);
            console.log('xml='+str );
            var err = getXmlVal( xmlDoc, 'err' );
            var cmd = getXmlVal( xmlDoc, 'cmd' );
            console.log('cmd='+cmd );

            if ( err == 0 ) {

                switch ( parseInt( cmd ) ) {
                case COMMAND_TSCHECK:
                    var lus  = getXmlVal( xmlDoc, 'lus' );
                    var lus2 = getXmlVal( xmlDoc, 'lus2');
                    var lus3 = getXmlVal( xmlDoc, 'lus3');

                    console.log('lus='+lus+', lus2='+lus2+', lus3='+lus3);

                    if ( ( lus != lastlus ) || (lus2 != lastlus2) ) {
                        lastlus = lus;
                        lastlus2 = lus2;
                        updateContent();
                    }
                    console.log( lus3, lastlus3 );
                    if ( (lus3 != lastlus3) ) {
                        lastlus3 = lus3;
                        getSystemTable();
                    }
                    break;

                case COMMAND_GET_SYSTEMTABLE:
                    var clk = getXmlVal( xmlDoc, 'clk' );
                    var sys = getXmlVal( xmlDoc, 'sys' );
                    console.log('sys='+sys );

                    // Handle some specials
                    var nfcmode = selectedNFCMode;
                    var rows = sys.split( ";" );
                    for ( var r=1; r<rows.length; r++ ) {
                        var cols = rows[r].split( "," );
                        if ( cols[SYS_COLUMN_NAME] == "nfcmode" ) {
                            nfcmode = parseInt( cols[SYS_COLUMN_INTVAL] );  // Integer
                            console.log( "Found nfc mode in sys" );
                        }
                    }
                    
                    console.log( "nfcmode", nfcmode, selectedNFCMode );
                    
                    if (nfcmode != selectedNFCMode) {
                    selectedNFCMode = nfcmode;
                                showCommissionSwitch();
                    }

                    break;

                case COMMAND_GET_DEVS:
                    /** \todo Mobile JS: separate lamps from plugs */
                    devs = getXmlVal( xmlDoc, 'devs' );
                    console.log('devs='+devs );
                    drawContent();

                    break;

                case COMMAND_GET_PLUG:
                    var time, onoff, act, sum, dayhistory, hourhistory;
                    mac         = getXmlVal( xmlDoc, 'mac' );
                    onoff       = parseInt( getXmlVal( xmlDoc, 'onoff' ) );
                    act         = getXmlVal( xmlDoc, 'act' );
                    sum         = getXmlVal( xmlDoc, 'sum' );
                    hourhistory = getXmlVal( xmlDoc, 'hourhistory' );
                    dayhistory  = getXmlVal( xmlDoc, 'dayhistory' );
                    yesterdaydata  = getXmlVal( xmlDoc, 'yesterdaydata' );
                    plugonoff = onoff;

                    console.log('plugonoff = '+plugonoff);
                    if ( onoff == 1 ) {
                        document.getElementById('plugonoff').innerHTML =
                            "<img src='/img/on.png' width='36'/>";
                    } else {
                        document.getElementById('plugonoff').innerHTML =
                            "<img src='/img/off.png' width='36'/>";
                    }
                    //document.getElementById('actsum').innerHTML =
                    //     'Actual: '+num1(act)+', Sum: '+num1(sum)+' ('+ts+')';
                    console.log('act = '+act+', '+'sum = ' + sum);
//                    plugAct = act; // Workaround as devs not updated!
//                    plugSum = sum;
                    console.log( 'hourhistory=' + hourhistory );
                    drawPlugChart( hourhistory );
//                    gotoPlug( mac, onoff, act, sum );
                    break;
                    
                case COMMAND_SETLEVEL:
                    console.log('new level set');
                    break;
                    
                case COMMAND_SETCOLOR:
                    console.log('new color set');
                    // Turn lamp on when color being adjusted
                    // if (justIntoDeviceUI == true) {
                    //     justIntoDeviceUI = false;
                    // } else {
                    //     ctrlLamp( lampmac, 1 ); // Turn on lamp
                    // }
                    break;
                    
                case COMMAND_SETKELVIN:
                    console.log('new kelvin set');
                    // Turn the lamp on if it being adjusted
                    // if (justIntoDeviceUI == true) {
                    //     justIntoDeviceUI = false;
                    // } else {
                    //     ctrlLamp( lampmac, 1 ); // Turn on lamp
                    // }
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

function getCurrentDeviceByIdentifer(baseID, table) {
    var rows = table.split( ";" );

    for ( var r = 1; r < rows.length; r++ ) {
        var cols = rows[r].split( "," );

        if (cols[COLUMN_DEVS_ID] == baseID) { // Found
                return rows[r];
        }
    }

    alert('Non-existing currentDeviceIdentifier!');
}

function getNextDeviceByIdentifer(baseID, table) {
    var rows = table.split( ";" );

    for ( var r = 1; r < rows.length; r++ ) {
        var cols = rows[r].split( "," );

        if (cols[COLUMN_DEVS_ID] == baseID) { // Found
            if (r != (rows.length - 1)) { // Not the last row
                return rows[r + 1]; // return the next row
            } else {
                return rows[1]; // return the first row
            }
        }
    }

    alert('Non-existing currentDeviceIdentifier!');
}

function gotoNextDevice() {
    var nextDeviceRow;
    var cols;

    console.log("gotoNextDevice called");
    console.log(devs);

    nextDeviceRow = getNextDeviceByIdentifer(currentDeviceIdentifier, devs);

    cols = nextDeviceRow.split(',');
    currentDeviceIdentifier = cols[COLUMN_DEVS_ID];
    prev_lvl    = cols[COLUMN_DEVS_LVL];
    prev_kelvin = cols[COLUMN_DEVS_KELVIN];
    prev_rgb    = cols[COLUMN_DEVS_RGB];
    justIntoDeviceUI = true;
    drawContent();
}

function gotoDevice( dev_id ) {
    console.log( "User pressed device " + dev_id );
    currentState = STATE_SINGLE_DEVICE;

    currentDeviceIdentifier = dev_id;

    currentDeviceRow = getCurrentDeviceByIdentifer(currentDeviceIdentifier, devs);
    cols = currentDeviceRow.split(',');
    prev_lvl    = cols[COLUMN_DEVS_LVL];
    prev_kelvin = cols[COLUMN_DEVS_KELVIN];
    prev_rgb    = cols[COLUMN_DEVS_RGB];
    justIntoDeviceUI = true;

    drawContent();
}

function gotoDeviceOverview()
{
    console.log('Now go back to device overview');
    currentState = STATE_DEVICE_OVERVIEW;
    // updateContent();
    drawContent();
}

function showCommissionSwitch() {
    switch ( selectedNFCMode ) {
    case 1:
        document.getElementById('but_decommission').style.color = 'red';
        document.getElementById('but_commission').style.color = 'white';
        break;
    case 0:
        document.getElementById('but_commission').style.color = 'red';
        document.getElementById('but_decommission').style.color = 'white';
        break;
    default:
        document.getElementById('but_commission').style.color = 'white';
        document.getElementById('but_decommission').style.color = 'white';
        break;
    }
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

function gotoCommission() {
    console.log('User selected commission mode for NFC');
    selectedNFCMode = 0;
    var post = 'command='+COMMAND_SET_NFCMODE+'&nfcmode='+selectedNFCMode;
    MobileExec(post);
    showCommissionSwitch();
}

function gotoDecommission() {
    console.log('User selected decommission mode for NFC');
    selectedNFCMode = 1;
    var post = 'command='+COMMAND_SET_NFCMODE+'&nfcmode='+selectedNFCMode;
    MobileExec(post);
    showCommissionSwitch();
}

