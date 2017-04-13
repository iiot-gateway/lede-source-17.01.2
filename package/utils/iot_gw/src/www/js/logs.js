// ------------------------------------------------------------------
// Logs 
// ------------------------------------------------------------------
// Javascript for the IoT Logs webpage
// ------------------------------------------------------------------
// Author:    nlv10677
// Copyright: NXP B.V. 2014. All rights reserved
// ------------------------------------------------------------------

var CANVAS_WIDTH  = 620;
var CANVAS_HEIGHT = 540;
var SMALLFONT     = 8;

var COMMAND_NORMAL   = 0;
var COMMAND_GET_LOGS = 1;

var MODE_LIST   = 0;
var MODE_GRAPH  = 1;

var modeSwitch = 0;
var mode = MODE_LIST;

// ------------------------------------------------------------------
// Variables
// ------------------------------------------------------------------

var timerVar   = 0;
var ts         = 0;
var cnt        = 0;

var startIndex = -1;

var logdata = new Array();
var loglups = new Array();

var isHtml5Compatible = 0;
var canvas      = 0;
var ctx         = 0;
var scalefactor = 1;
var transX      = 0;
var transY      = 0;
var strokeStyle = "#336633";
var lineWidth   = 1;

var objects     = [];

// ------------------------------------------------------------------
// On page load
// ------------------------------------------------------------------

function iotOnLoad() {
    document.getElementById('javaerror').style.display = 'none';
    isHtml5Compatible = document.createElement('canvas').getContext != undefined;
    if ( isHtml5Compatible ) {

        if ( typeof document.getElementById('systemCanvas') != 'undefined' ) {
        
            document.getElementById('modeswitch').style.display = 'inline';
            modeToggle();
        
            canvas = document.getElementById('systemCanvas');
            ctx = canvas.getContext("2d");

            window.addEventListener('resize', resizeCanvas, false);

            canvas.addEventListener("mousedown", mouseDown);
            canvas.addEventListener("mousemove", mouseDrag);
            canvas.addEventListener("mouseup",   mouseUp  );

            buildGraph();
        }
    }
    
    startIndex = -1;
    getLogs();
    timerRestart();
}

// ------------------------------------------------------------------
// Build graph
// ------------------------------------------------------------------

function buildGraph() {

    zb = addObject( new process( new pos(  290, 120 ),
                                 "ZCB" ) );
    ci = addObject( new process( new pos(  80, 140, objects[zb].pos ),
                                 "Control\nInterface" ) );
    sj = addObject( new process( new pos( -100, 140, objects[zb].pos ),
                                 "Secure\nJoiner" ) );
    xx = addObject( new process( new pos(  150,  35, objects[zb].pos ),
                                 "DBP" ) );
    ph = addObject( new process( new pos( -185, 140, objects[ci].pos ),
                                 "Phone", 0x336666 ) );
    nd = addObject( new process( new pos(  -80, 140, objects[ci].pos ),
                                 "NFC\daemon" ) );
    cg = addObject( new process( new pos(   25, 140, objects[ci].pos ),
                                 "CGI", 0x336666 ) );

    db = addObject( new database( new pos( 120, 50, objects[ci].pos ) ) );

    qz = addObject( new queue( new pos( -52,   0, objects[zb].pos ), 0 ) );
    qc = addObject( new queue( new pos(   0, -32, objects[ci].pos ), 1 ) );
    qs = addObject( new queue( new pos(  52,   0, objects[sj].pos ), 0 ) );
    qx = addObject( new queue( new pos( -52,   0, objects[xx].pos ), 0 ) );
    
    ss = addObject( new socket( new pos( 0, 32, objects[sj].pos ) ) );
    sc = addObject( new socket( new pos( 0, 32, objects[ci].pos ) ) );

    addObject( new connector( objects[zb].bottom, objects[qx].left, "angle", 0 ) );
    
    addObject( new connector( objects[zb].bottom, objects[qc].top, "flat", 0 ) );
    addObject( new connector( objects[zb].bottom, objects[qs].right ) );
    
    addObject( new connector( objects[ci].left, objects[qz].left ) );
    addObject( new connector( objects[sj].top, objects[qz].left ) );
    
    addObject( new connector( objects[ph].top, objects[ss].bottom, "flat", 0 ) );
    addObject( new connector( objects[ph].top, objects[sc].bottom, "flat", 0 ) );

    addObject( new connector( objects[nd].top, objects[ss].bottom, "flat", 0 ) );
    addObject( new connector( objects[nd].top, objects[sc].bottom, "flat", 0 ) );

    addObject( new connector( objects[cg].top, objects[sc].bottom, "flat", 0 ) );
    
    // Testing only
    tickDragging = addObject( new tickBox( new pos( 550, 510 ), "Dragging" ) );
    
    t_zbin = addObject( new callout( new pos( -110, -60, objects[zb].pos ),
                                 objects[zb].pos ) );
    t_zbout = addObject( new callout( new pos( 120, -60, objects[zb].pos ),
                                 objects[zb].pos ) );
    t_xx = addObject( new callout( new pos( 55, -40, objects[xx].pos ),
                                 objects[xx].pos ) );
    t_db = addObject( new callout( new pos( 55, 80, objects[db].pos ),
                                 objects[db].pos ) );
    t_cg = addObject( new callout( new pos( 80, 65, objects[cg].pos ),
                                 objects[cg].pos ) );
    t_nd = addObject( new callout( new pos( -80, 65, objects[nd].pos ),
                                 objects[nd].pos ) );
    t_sj = addObject( new callout( new pos( -100, 70, objects[sj].pos ),
                                 objects[sj].pos ) );
    t_ci = addObject( new callout( new pos( 150, -30, objects[ci].pos ),
                                 objects[ci].pos ) );
    t_yy = addObject( new callout( new pos( -20, 110, objects[db].pos ) ) );
    
    objects[t_zbin].setText( "zb-in" );
    objects[t_zbout].setText( "zb-out" );
    objects[t_xx].setText( "xx" );
    objects[t_db].setText( "db" );
    objects[t_cg].setText( "cg" );
    objects[t_nd].setText( "nd" );
    objects[t_sj].setText( "sj" );
    objects[t_ci].setText( "ci" );
        
    resizeCanvas();
}

// ------------------------------------------------------------------
// Scale to fit
// ------------------------------------------------------------------

function resizeCanvas() {
    // console.log( canvas.parentNode.width );
    // console.log( canvas.parentNode.style.width );

    canvas.width = window.innerWidth - 45;
    canvas.height = window.innerHeight - 80;
    ctx.setTransform(1, 0, 0, 1, 0, 0);
    console.log( canvas.width / CANVAS_WIDTH, canvas.height / CANVAS_HEIGHT );
    scalefactor = Math.min( canvas.width / CANVAS_WIDTH, canvas.height / CANVAS_HEIGHT );
    scalefactor = Math.max( 1, scalefactor );
    transX = ( canvas.width - scalefactor * CANVAS_WIDTH ) / 2;
    transY = ( canvas.height - scalefactor * CANVAS_HEIGHT ) / 2;
    ctx.translate( transX, transY );
    if ( scalefactor > 1 ) {
        ctx.scale( scalefactor, scalefactor );
    }
    drawAll(); 
}
       
// ------------------------------------------------------------------
// Helper class
// ------------------------------------------------------------------

function pos( x, y, relto ) {
    this.x = x;
    this.y = y;
    this.relto = relto;
}

function addObject( obj ) {
    return( objects.push( obj ) - 1 );
}

// ------------------------------------------------------------------
// Main objects
// ------------------------------------------------------------------

function process( center, name, color ) {
    var self    = this;
    this.object = "process";
    this.center = center;
    this.name   = name;
    var radius  = 40;
    this.color = 0x0066cc;
    if ( typeof color != 'undefined' ) this.color = color;
    
    // Positioning
    this.pos = function() {
        // console.log( self.center );
        if ( typeof self.center.relto != 'undefined' ) {
            var offset = self.center.relto();
            return( { x: self.center.x + offset.x, 
                      y: self.center.y + offset.y } ); 
        }
        return( { x: self.center.x, y: self.center.y } ); 
    }
    this.shift = function( dx, dy ) {
        self.center.x += dx;
        self.center.y += dy;
        console.log( self.center.x, self.center.y );
    }
    
    // Anchors
    this.top = function() {
        var p = self.pos();
        return( { x: p.x, y: ( p.y - radius/2 ) } );
    }
    this.right = function() {
        var p = self.pos();
        return( { x: ( p.x + radius ), y: p.y } );
    }
    this.bottom = function() {
        var p = self.pos();
        return( { x: p.x, y: ( p.y + radius/2 ) } );
    }
    this.left = function() {
        var p = self.pos();
        return( { x: ( p.x - radius ), y: p.y } );
    }
    
    // Inquiry
    this.isin = function( x, y ) {
        var p = self.pos();
        // console.log( x + ',' + y + ' isin ' + p.x + ',' + p.y );
        return( Math.sqrt( Math.pow( (p.x - x), 2 ) + 
                           Math.pow( 2*(p.y - y), 2 ) ) < radius );
    }

    // Draw
    this.draw = function() {
        var p = self.pos();
        drawOval3D( p.x, p.y, radius, 0.5, self.color );
        drawTextCentered( name, p.x, p.y, "white" );
    }
}

// ------------------------------------------------------------------
// Misc
// ------------------------------------------------------------------

function database( center, color ) {
    var self    = this;
    this.object = "database";
    this.center = center;
    this.width  = 70;
    this.height = 40;
    this.color = "#cc3366";
    if ( typeof color != 'undefined' ) this.color = color;
    
    // Positioning
    this.pos = function() {
        // console.log( self.center );
        if ( typeof self.center.relto != 'undefined' ) {
            var offset = self.center.relto();
            return( { x: self.center.x + offset.x, 
                      y: self.center.y + offset.y } ); 
        }
        return( { x: self.center.x, y: self.center.y } ); 
    }
    this.shift = function( dx, dy ) {
        self.center.x += dx;
        self.center.y += dy;
        console.log( self.center.x, self.center.y );
    }
    
    // Inquiry
    this.isin = function( x, y ) {
        var p = self.pos();
        // console.log( x + ',' + y + ' isin ' + p.x + ',' + p.y );
        return( Math.abs( p.x - x ) < self.width/2 &&
                Math.abs( p.y - y ) < self.height/2 );
    }
    
    // Draw
    this.draw = function() {
        var p = self.pos();
        drawCilinder( p.x, p.y, self.width, self.height, self.color );
        drawTextCentered( "dB", p.x, p.y, "white" );
    }
}

function queue( center, vertical, color ) {
    var self      = this;
    this.object   = "queue";
    this.center   = center;
    this.vertical = vertical;
    this.color    = "#44ff44";
    if ( typeof color != 'undefined' ) this.color = color;
    
    if ( vertical ) {
        this.width = 12;
        this.height = 24;
    } else {
        this.width = 24;
        this.height = 12;
    }
    
    // Positioning
    this.pos = function() {
        // console.log( self.center );
        if ( typeof self.center.relto != 'undefined' ) {
            var offset = self.center.relto();
            return( { x: self.center.x + offset.x, 
                      y: self.center.y + offset.y } ); 
        }
        return( { x: self.center.x, y: self.center.y } ); 
    }
    this.shift = function( dx, dy ) {
        self.center.x += dx;
        self.center.y += dy;
        console.log( self.center.x, self.center.y );
    }
    
    // Anchors
    this.top = function() {
        var p = self.pos();
        return( { x: p.x, y: ( p.y - self.height/2 ) } );
    }
    this.right = function() {
        var p = self.pos();
        return( { x: ( p.x + self.width/2 ), y: p.y } );
    }
    this.bottom = function() {
        var p = self.pos();
        return( { x: p.x, y: ( p.y + self.height/2 ) } );
    }
    this.left = function() {
        var p = self.pos();
        return( { x: ( p.x - self.width/2 ), y: p.y } );
    }
    
    // Inquiry
    this.isin = function( x, y ) {
        var p = self.pos();
        // console.log( x + ',' + y + ' isin ' + p.x + ',' + p.y );
        return( Math.abs( p.x - x ) < self.width/2 &&
                Math.abs( p.y - y ) < self.height/2 );
    }
    
    // Draw
    this.draw = function() {
        var p = self.pos();
        if ( vertical ) {
            p.y -= 9;
            for ( var i=0; i<4; i++ ) {
                drawRect( p.x, p.y, 12, 6, self.color );
                p.y += 6;
            }
        } else {
            p.x -= 9;
            for ( var i=0; i<4; i++ ) {
                drawRect( p.x, p.y, 6, 12, self.color );
                p.x += 6;
            }
        }
    }
}

function socket( center, color ) {
    var self    = this;
    this.object = "socket";
    this.center = center;
    this.color  = "#ffff00";
    if ( typeof color != 'undefined' ) this.color = color;
    
    this.width  = 10;
    this.height = 20;
    
    // Positioning
    this.pos = function() {
        // console.log( self.center );
        if ( typeof self.center.relto != 'undefined' ) {
            var offset = self.center.relto();
            return( { x: self.center.x + offset.x, 
                      y: self.center.y + offset.y } ); 
        }
        return( { x: self.center.x, y: self.center.y } ); 
    }
    this.shift = function( dx, dy ) {
        self.center.x += dx;
        self.center.y += dy;
        console.log( self.center.x, self.center.y );
    }
    
    // Anchors
    this.top = function() {
        var p = self.pos();
        return( { x: p.x, y: ( p.y - self.height/2 - 2 ) } );
    }
    this.bottom = function() {
        var p = self.pos();
        return( { x: p.x, y: ( p.y + self.height/2 + 2 ) } );
    }
    
    // Inquiry
    this.isin = function( x, y ) {
        var p = self.pos();
        // console.log( x + ',' + y + ' isin ' + p.x + ',' + p.y );
        return( Math.abs( p.x - x ) < self.width/2 &&
                Math.abs( p.y - y ) < self.height/2 );
    }
    
    // Draw
    this.draw = function() {
        var p = self.pos();
        drawCilinder( p.x, p.y, self.width, self.height, self.color );
    }
}

function connector( a, b, type, orientation ) {
    var self    = this;
    this.object = "connector";
    this.a      = a;
    this.b      = b;
    
    this.type = "angle";
    this.orientation = 1;
    
    if ( typeof type        != 'undefined' ) this.type = type;
    if ( typeof orientation != 'undefined' ) this.orientation = orientation;
    
    // Draw
    this.draw = function() {
        var p1 = self.a();
        var p2 = self.b();
        if ( self.type == "angle" ) {
            drawArrow90( p1.x, p1.y, p2.x, p2.y, self.orientation );
        } else if ( self.type == "flat" ) {
            drawArrow0( p1.x, p1.y, p2.x, p2.y, self.orientation );
        }
    }
}

// ------------------------------------------------------------------
// Callout
// ------------------------------------------------------------------

function callout( center, anchor, color, text ) {
    var self    = this;
    this.object = "callout";
    this.center = center;
    this.anchor = anchor;
    this.width  = 190;
    this.height = 0;
    this.color  = "#ffffcc";
    if ( typeof color != 'undefined' ) this.color = color;
    if ( typeof text  != 'undefined' ) this.setText( text );
    
    this.clrText = function() {
        this.lines = [];
        this.cnt   = 0;
    }

    this.clrText();
    
    this.setText = function( text ) {
        
        ctx.font = SMALLFONT + "pt Calibri";
        self.lines = [];
        self.cnt = 0;

        var tlines = text.split( ";" );
        // tlines.reverse();

        for ( var j=0; j < tlines.length; j++ ) {

            var txt = tlines[j];
            var line = "";
            var tw = 0;
        
            for ( var i=0; i < txt.length; i++ ) {
                var ch = txt.charAt(i);
                if ( tw < (self.width-10) ) {
                    line += ch;
                    var metrics = ctx.measureText( line );
                    tw = metrics.width;
                } else {
                    self.lines[self.cnt++] = line;
                    line = "    " + ch;
                    tw = 0;
                }
            }

            if ( tw > 0 ) {
                self.lines[self.cnt++] = line;
            }
            
            self.height = ( self.cnt * SMALLFONT ) + 6;
        }
    }
    
    // Positioning
    this.pos = function() {
        // console.log( self.center );
        if ( typeof self.center.relto != 'undefined' ) {
            var offset = self.center.relto();
            return( { x: self.center.x + offset.x, 
                      y: self.center.y + offset.y } ); 
        }
        return( { x: self.center.x, y: self.center.y } ); 
    }
    this.shift = function( dx, dy ) {
        self.center.x += dx;
        self.center.y += dy;
        console.log( self.center.x, self.center.y );
    }
    
    // Inquiry
    this.isin = function( x, y ) {
        var p = self.pos();
        // console.log( x + ',' + y + ' isin ' + p.x + ',' + p.y );
        return( Math.abs( p.x - x ) < self.width/2 &&
                Math.abs( p.y - y ) < self.height/2 );
    }
    
    // Draw
    this.draw = function() {
        var p = self.pos();
        if ( self.cnt > 0 ) {
            var x = p.x - self.width/2 - 4;
            var y = p.y - self.height/2 - 4;
            if ( typeof self.anchor != 'undefined' ) {
                var p2 = self.anchor();
                drawDottedLine( p.x, y, p2.x, p2.y );
            }
            drawRoundedRect( x, y, self.width, self.height, 6, self.color );
            for ( var i=0; i < self.lines.length; i++ ) {
                drawSmallText( self.lines[i], x+4, y+7+(i*SMALLFONT), "#444444" );
            }
        }
    }
}

// ------------------------------------------------------------------
// Tick box
// ------------------------------------------------------------------

function tickBox( center, text, color ) {
    var self    = this;
    this.object = "tickBox";
    this.center = center;
    this.value  = 0;
    this.width  = 10;
    this.height = 10;
    this.text   = 0;
    this.color  = "#ffffaa";
    if ( typeof text  != 'undefined' ) this.text = text;
    if ( typeof color != 'undefined' ) this.color = color;
    
    // Positioning
    this.pos = function() {
        // console.log( self.center );
        if ( typeof self.center.relto != 'undefined' ) {
            var offset = self.center.relto();
            return( { x: self.center.x + offset.x, 
                      y: self.center.y + offset.y } ); 
        }
        return( { x: self.center.x, y: self.center.y } ); 
    }
    this.shift = function( dx, dy ) {
        self.center.x += dx;
        self.center.y += dy;
        console.log( self.center.x, self.center.y );
    }
    
    // Inquiry
    this.isin = function( x, y ) {
        var p = self.pos();
        // console.log( 'tick: ' + x + ',' + y + ' isin ' + p.x + ',' + p.y );
        return( Math.abs( p.x - x ) < self.width/2 &&
                Math.abs( p.y - y ) < self.height/2 );
    }
    
    // Buttons
    this.onDown = function() {
        console.log( "OnDown" );
        self.value = 1 - self.value;
    }
    
    // Draw
    this.draw = function() {
        var p = self.pos();
        var x = p.x - self.width/2 - 4;
        var y = p.y - self.height/2 - 4;
        drawRect( p.x, p.y, self.width, self.height, self.color );
        if ( self.value ) {
            var x1 = p.x - self.width/2;
            var y1 = p.y - self.height/2;
            var x2 = p.x + self.width/2;
            var y2 = p.y + self.height/2;
            drawLine( x1, y1, x2, y2 );
            drawLine( x1, y2, x2, y1 );
        }
        drawTextLeft( self.text, x + self.width + 6, y + 8, "#444444" );
    }
}

// ------------------------------------------------------------------
// Callout helpers
// ------------------------------------------------------------------

var logNames = new Array( "none", 
    "Control Interface",  // 1
    "Database",           // 2
    "NFC Daemon",         // 3
    "Secure Joiner",      // 4
    "ZCB-in",             // 5
    "ZCB-out",            // 6
    "CGI-script",         // 7
    "DBP",                // 8
    "Gw Discovery",       // 9
    "Test" );             // 10

function setCallout( from, text ) {
    switch ( from ) {
    case 1: objects[t_ci].setText( text ); break;
    case 2: objects[t_db].setText( text ); break;
    case 3: objects[t_nd].setText( text ); break;
    case 4: objects[t_sj].setText( text ); break;
    case 5: objects[t_zbin].setText( text ); break;
    case 6: objects[t_zbout].setText( text ); break;
    case 7: objects[t_cg].setText( text ); break;
    case 8: objects[t_xx].setText( text ); break;
    case 9:
    case 10: objects[t_yy].setText( text ); break;
    }
}

function clearCallouts() {
    objects[t_zbin].clrText();
    objects[t_zbout].clrText();
    objects[t_xx].clrText();
    objects[t_db].clrText();
    objects[t_cg].clrText();
    objects[t_nd].clrText();
    objects[t_sj].clrText();
    objects[t_ci].clrText();
    objects[t_yy].clrText();
}

// ------------------------------------------------------------------
// Timer
// ------------------------------------------------------------------

function timerRestart() {
    if ( timerVar ) clearInterval( timerVar );
    // Avoid strict 2-sec pace
    var period = 1950 + Math.floor( Math.random() * 100 );
    timerVar = setInterval( LogsUpdate, period );
}

// ------------------------------------------------------------------
// Mode switch
// ------------------------------------------------------------------

function showModeSwitch() {
    console.log( "showModeSwitch " + mode );
    if ( mode == MODE_LIST ) {
        document.getElementById('modeswitch').innerHTML =
             "<img src='/img/switch_off.png' onclick='modeToggle();' />";
    } else {
        document.getElementById('modeswitch').innerHTML =
             "<img src='/img/switch_on.png' onclick='modeToggle();' />";
    }
}

function modeToggle() {
    switch ( mode ) {
    case MODE_LIST:
        mode = MODE_GRAPH;
        document.getElementById('graph').style.display = 'block';
        document.getElementById('logs').style.display = 'none';
        break;
    case MODE_GRAPH:
        mode = MODE_LIST;
        document.getElementById('graph').style.display = 'none';
        document.getElementById('logs').style.display = 'block';
        break;
    }
    showModeSwitch();
}

// ------------------------------------------------------------------
// Draw system
// ------------------------------------------------------------------

function drawAll() {
    ctx.clearRect(-transX,-transY,canvas.width,canvas.height);
    for ( var i=0; i<objects.length; i++ ) {
        // console.log( "Draw " + i );
        if ( objects[i].object == "callout" ) {
            objects[i].draw();
        }
    }
    for ( var i=0; i<objects.length; i++ ) {
        // console.log( "Draw " + i );
        if ( objects[i].object != "callout" ) {
            objects[i].draw();
        }
    }
}

   
// ------------------------------------------------------------------
// Mouse events
// ------------------------------------------------------------------

var draggingOn = 0;
var dragObject = -1;
var dragTimer = 0;

function mouseDown(e){
    var e = getMousePositionInElement( e, canvas );
    console.log( e );
    
    if ( typeof objects[tickDragging] != 'undefined' ) {
        draggingOn = objects[tickDragging].value;
    }
        
    for ( var i=0; i<objects.length; i++ ) {
        if ( typeof objects[i].isin != 'undefined' ) {
            if ( objects[i].isin( e.x, e.y ) ) {
                if ( typeof objects[i].onDown != 'undefined' ) {
                    objects[i].onDown();
                }
                if ( draggingOn ) {
                    dragObject = i;
                    console.log( "dragObject = " + dragObject );
                    prevx = e.x;
                    prevy = e.y;
                }
                break;
            }
        }
    }
}

function mouseUp(e){
    if ( dragObject >= 0 ) {
        if ( typeof objects[dragObject].onUp != 'undefined' ) {
            objects[i].onUp();
        }
        dragObject = -1;
    }
    drawAll();
}

function timedDrawAll() {
    dragTimer = 0;
}

function mouseDrag(e){
    if ( dragObject >= 0 ) {
        var e = getMousePositionInElement( e, canvas );
        // console.log( e );
        objects[dragObject].shift( e.x - prevx, e.y - prevy );
        prevx = e.x;
        prevy = e.y;
        if ( !dragTimer ) {
            drawAll();
            dragTimer = setTimeout(timedDrawAll, 16);
        }
    }
}

function getMousePositionInElement(event, element){
    var offsetX = 0;
    var offsetY = 0;
    var e = element;
    
    return {x: ( event.clientX - element.offsetLeft - transX ) / scalefactor, 
            y: ( event.clientY - element.offsetTop  - transY ) / scalefactor };
}

// ------------------------------------------------------------------
// Graphical primitives
// ------------------------------------------------------------------

function drawCilinder( x, y, w, h, color ) {
    drawOval( x, y+(h/2), w/2, 0.3, color );
    ctx.beginPath();
    ctx.fillStyle = color;
    ctx.fillRect( x - w/2, y - h/2, w, h );
    ctx.moveTo( x - w/2, y - h/2 );
    ctx.lineTo( x - w/2, y + h/2 );
    ctx.moveTo( x + w/2, y - h/2 );
    ctx.lineTo( x + w/2, y + h/2 );
    ctx.lineWidth = lineWidth;
    ctx.strokeStyle = strokeStyle;
    ctx.stroke();
    ctx.beginPath();
    drawOval( x, y-(h/2), w/2, 0.3, color );
}

function drawText( txt, x, y, color ) {
    ctx.font = "10pt Calibri";
    ctx.textBaseline = "middle";
    ctx.fillStyle = color;
    var lines = txt.split( "\n" );
    if ( lines.length > 1 ) {
        y -= ( lines.length - 1 ) * 5;
    }
    for ( var i=0; i<lines.length; i++ ) {
       ctx.fillText( lines[i], x, y );
       y += 10;
    }
}

function drawTextLeft( txt, x, y, color ) {
    ctx.textAlign = "left";
    drawText( txt, x, y, color );
}

function drawTextCentered( txt, x, y, color ) {
    ctx.textAlign = "center";
    drawText( txt, x, y, color );
}

function drawSmallText( txt, x, y, color ) {
    ctx.font = SMALLFONT + "pt Calibri";
    ctx.textAlign = "left";
    ctx.textBaseline = "middle";
    ctx.fillStyle = color;
    ctx.fillText( txt, x, y );
}

function drawRoundedRect( x, y, w, h, radius, color ) {
    ctx.fillStyle = color;
    ctx.beginPath();
    ctx.moveTo( x + radius, y );
    ctx.lineTo( x + w - radius, y );
    ctx.quadraticCurveTo( x+w, y, x+w, y + radius );
    ctx.lineTo( x+w, y + h - radius );
    ctx.quadraticCurveTo( x+w, y+h, x + w - radius, y+h );
    ctx.lineTo( x+radius, y+h );
    ctx.quadraticCurveTo( x, y+h, x, y+h-radius );
    ctx.lineTo( x, y+radius );
    ctx.quadraticCurveTo( x, y, x+radius, y );
    ctx.closePath();
    ctx.fill();
    ctx.lineWidth = lineWidth;
    ctx.strokeStyle = strokeStyle;
    ctx.stroke();
}

function drawRect( x, y, w, h, color ) {
    ctx.beginPath();
    ctx.lineWidth = lineWidth;
    ctx.strokeStyle = strokeStyle;
    ctx.fillStyle = color;
    ctx.fillRect( x - w/2, y - h/2, w, h );
    ctx.strokeRect( x - w/2, y - h/2, w, h );
}

function drawDottedLine( x1, y1, x2, y2 ) {
    ctx.beginPath();
    var w = x2-x1;
    var h = y2-y1;
    var l = Math.sqrt( Math.pow( w, 2 ) + Math.pow( h, 2 ) );
    while ( l > 1 ) {
        ctx.moveTo( x1, y1 );
        if ( l > 10 ) {
            var x = x1 + (w*5)/l;
            var y = y1 + (h*5)/l;
            ctx.lineTo( x, y );
            x1 = x1 + (w*10)/l;
            y1 = y1 + (h*10)/l;
        } else {
            ctx.lineTo( x2, y2 );
            x1 = x2; y1 = y2;
        }
        w = x2-x1;
        h = y2-y1;
        l = Math.sqrt( Math.pow( w, 2 ) + Math.pow( h, 2 ) );
    }
    ctx.closePath();
    ctx.stroke();
}

function drawPolygon( Xcenter, Ycenter, numOfSides, size, color ) {
    ctx.save();
    ctx.beginPath();
    ctx.translate( Xcenter, Ycenter );
    var r = (color>>16)&0xFF;
    var g = (color>>8)&0xFF;
    var b = color&0xFF;
    var color1 = "rgb("+r+","+g+","+b+")";
    var color2 = "rgb("+Math.round(r*0.7)+","+Math.round(g*0.7)+","+Math.round(b*0.7)+")";
    var gradient = ctx.createRadialGradient(0, 0, size/1.5, 0, 0, size);
    gradient.addColorStop(0, color1);
    gradient.addColorStop(1, color2);
    ctx.beginPath();
    ctx.moveTo(size * Math.cos(0), size * Math.sin(0) );
    for ( var i=1; i<= numOfSides; i++ ) {
        var angle = i * 2 * Math.PI / numOfSides;
        ctx.lineTo( size * Math.cos( angle ),
                    size * Math.sin( angle ) );
    }
    ctx.closePath();
    ctx.fillStyle = gradient;
    ctx.fill();
    ctx.lineWidth = lineWidth;
    ctx.strokeStyle = strokeStyle;
    ctx.stroke();
    ctx.restore();
}

function drawOval( Xcenter, Ycenter, radius, scale, color ) {
    ctx.save();
    ctx.beginPath();
    ctx.translate( Xcenter, Ycenter );
    if ( scale < 1 ) ctx.scale( radius, scale*radius );
    else ctx.scale( radius/scale, radius );
    ctx.arc( 0, 0, 1, 0, Math.PI * 2, false );
    ctx.closePath();
    ctx.fillStyle = color;
    ctx.fill();
    ctx.restore();
    ctx.lineWidth = lineWidth;
    ctx.strokeStyle = strokeStyle;
    ctx.stroke();
}

function drawOval3D( Xcenter, Ycenter, radius, scale, color ) {
    ctx.save();
    ctx.beginPath();
    ctx.translate( Xcenter, Ycenter );
    if ( scale < 1 ) ctx.scale( 1, scale );
    else ctx.scale( scale, 1 );
    var r = (color>>16)&0xFF;
    var g = (color>>8)&0xFF;
    var b = color&0xFF;
    var color1 = "rgb("+r+","+g+","+b+")";
    var color2 = "rgb("+Math.round(r*0.7)+","+Math.round(g*0.7)+","+Math.round(b*0.7)+")";
    var gradient = ctx.createRadialGradient(0, 0, radius/1.5, 0, 0, radius);
    gradient.addColorStop(0, color1);
    gradient.addColorStop(1, color2);
    ctx.arc( 0, 0, radius, 0, Math.PI * 2, false );
    // ctx.closePath();
    ctx.fillStyle = gradient;
    ctx.fill();
    ctx.restore();
    ctx.lineWidth = lineWidth;
    ctx.strokeStyle = strokeStyle;
    ctx.stroke();

    ctx.save();
    ctx.beginPath();
    ctx.translate( Xcenter, Ycenter );
    if ( scale < 1 ) ctx.scale( radius, scale*radius );
    else ctx.scale( radius/scale, radius );
    ctx.arc( 0, 0, 1, 0, Math.PI, false );
    ctx.restore();
    ctx.strokeStyle = 'black';
    ctx.stroke();
    ctx.strokeStyle = strokeStyle;
}

// ------------------------------------------------------------------
// Arrows
// ------------------------------------------------------------------

function drawArrow90( x1, y1, x2, y2, p ) {
    var midX;
    var midY;
    
    var pstyle = 1;
    if ( x1 < x2 ) {
        if ( y1 < y2 ) {
            if ( p > 0 ) {
                midX = x2;
                midY = y1;
                pstyle = 1;
            } else {
                midX = x1;
                midY = y2;
                pstyle = 2;
            }
        } else {
            if ( p > 0 ) {
                midX = x1;
                midY = y2;
                pstyle = 2;
            } else {
                midX = x2;
                midY = y1;
                pstyle = 3;
            }
        }
    } else {
        if ( y1 < y2 ) {
            if ( p > 0 ) {
                midX = x1;
                midY = y2;
                pstyle = 4;
            } else {
                midX = x2;
                midY = y1;
                pstyle = 1;
            }
        } else {
            if ( p > 0 ) {
                midX = x2;
                midY = y1;
                pstyle = 3;
            } else {
                midX = x1;
                midY = y2;
                pstyle = 4;
            }
        }
    }
    var a = 4;
    ctx.beginPath();
    ctx.moveTo( x1, y1 );
    ctx.quadraticCurveTo( midX, midY, x2, y2 );
    switch ( pstyle ) {
    case 1:
        ctx.lineTo( x2-a, y2-a );
        ctx.lineTo( x2, y2 );
        ctx.lineTo( x2+a, y2-a );
        break;
    case 2:
        ctx.lineTo( x2-a, y2-a );
        ctx.lineTo( x2, y2 );
        ctx.lineTo( x2-a, y2+a );
        break;
    case 3:
        ctx.lineTo( x2-a, y2+a );
        ctx.lineTo( x2, y2 );
        ctx.lineTo( x2+a, y2+a );
        break;
    case 4:
        ctx.lineTo( x2+a, y2-a );
        ctx.lineTo( x2, y2 );
        ctx.lineTo( x2+a, y2+a );
        break;
    }
    ctx.lineWidth = lineWidth;
    ctx.strokeStyle = strokeStyle;
    ctx.stroke();
}

function drawArrow0( x1, y1, x2, y2, p ) {
    var midX1, midX2;
    var midY1, midY2;
    var w = x2 - x1;
    var h = y2 - y1;
    
    var pstyle = 1;
    if ( x1 < x2 ) {
        if ( y1 < y2 ) {
            if ( p > 0 ) {
                // console.log( "Q1: w=" + w + ", h=" + h );
                midX1 = x1 + w/3;
                midY1 = y1;
                midX2 = x2 - w/3;
                midY2 = y2;
                pstyle = 2;
            } else {
                // console.log( "Q2: w=" + w + ", h=" + h );
                midX1 = x1;
                midY1 = y1 + h/3;
                midX2 = x2;
                midY2 = y2 - h/3;
                pstyle = 1;
            }
        } else {
            if ( p > 0 ) {
                // console.log( "Q3: w=" + w + ", h=" + h );
                midX1 = x1 + w/3;
                midY1 = y1;
                midX2 = x2 - w/3;
                midY2 = y2;
                pstyle = 2;
            } else {
                // console.log( "Q4: w=" + w + ", h=" + h );
                midX1 = x1;
                midY1 = y1 + h/3;
                midX2 = x2;
                midY2 = y2 - h/3;
                pstyle = 3;
            }
        }
    } else {
        if ( y1 < y2 ) {
            if ( p > 0 ) {
                // console.log( "Q5: w=" + w + ", h=" + h );
                midX1 = x1 + w/3;
                midY1 = y1;
                midX2 = x2 - w/3;
                midY2 = y2;
                pstyle = 4;
            } else {
                // console.log( "Q6: w=" + w + ", h=" + h );
                midX1 = x1;
                midY1 = y1 + h/3;
                midX2 = x2;
                midY2 = y2 - h/3;
                pstyle = 1;
            }
        } else {
            if ( p > 0 ) {
                // console.log( "Q7: w=" + w + ", h=" + h );
                midX1 = x1 + w/3;
                midY1 = y1;
                midX2 = x2 - w/3;
                midY2 = y2;
                pstyle = 4;
            } else {
                // console.log( "Q8: w=" + w + ", h=" + h );
                midX1 = x1;
                midY1 = y1 + h/3;
                midX2 = x2;
                midY2 = y2 - h/3;
                pstyle = 3;
            }
        }
    }
    var a = 4;
    ctx.beginPath();
    ctx.moveTo( x1, y1 );
    ctx.bezierCurveTo( midX1, midY1, midX2, midY2, x2, y2 );
    switch ( pstyle ) {
    case 1:
        ctx.lineTo( x2-a, y2-a );
        ctx.lineTo( x2, y2 );
        ctx.lineTo( x2+a, y2-a );
        break;
    case 2:
        ctx.lineTo( x2-a, y2-a );
        ctx.lineTo( x2, y2 );
        ctx.lineTo( x2-a, y2+a );
        break;
    case 3:
        ctx.lineTo( x2-a, y2+a );
        ctx.lineTo( x2, y2 );
        ctx.lineTo( x2+a, y2+a );
        break;
    case 4:
        ctx.lineTo( x2+a, y2-a );
        ctx.lineTo( x2, y2 );
        ctx.lineTo( x2+a, y2+a );
        break;
    }
    ctx.lineWidth = lineWidth;
    ctx.strokeStyle = strokeStyle;
    ctx.stroke();
}

function drawLine( x1, y1, x2, y2 ) {
    ctx.beginPath();
    ctx.moveTo( x1, y1 );
    ctx.lineTo( x2, y2 );
    ctx.lineWidth = lineWidth;
    ctx.strokeStyle = strokeStyle;
    ctx.stroke();
}

// ------------------------------------------------------------------
// HTTP/XML
// ------------------------------------------------------------------

function getXmlVal( xmlDoc, tag ) {
    console.log( "getXmlVal " + tag );
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
    xmlhttp.open( 'POST', 'iot_logs.cgi', true );
    xmlhttp.setRequestHeader( 'Content-type',
                              'application/x-www-form-urlencoded' );
    xmlhttp.send( posts );
}

// ------------------------------------------------------------------
// Helpers
// ------------------------------------------------------------------

function timeConv( ts, withdate ) {
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
    var time;
    if ( withdate ) time = day+' '+month+' '+date+' '+hour+':'+min+':'+sec+' '+year;
    else            time =                            hour+':'+min+':'+sec;

    return time;
}

// ------------------------------------------------------------------
// Process file names and last updates
// ------------------------------------------------------------------

function processLogfiles( logfiles ) {
    // callouts = new Array();
    clearCallouts();
    var html = "";
    var rows = logfiles.split( ";" );
    var changes = 0;
    for ( var r=0; r<rows.length; r++ ) {
        var cols = rows[r].split( "," );
        var filename = cols[0];
        var lup = cols[1];
        // html += filename + " - " + lup + "<br />";
        if ( typeof logdata[filename] == 'undefined' ) {
            logdata[filename] = filename;
            loglups[filename] = lup;
            getFileLog( filename );
            changes = 1;
        } else if ( loglups[filename] != lup ) {
            loglups[filename] = lup;
            getFileLog( filename );
            changes = 1;
        }
        html += "<fieldset class='cbi-section' background-color='#FFFFFF';>\n";
        html += "<legend>"+ filename + "</legend>\n";
        html += "<div id="+filename+">"+
                  logdata[filename].replace( /;/g, "<br />" ); + "</div>\n";
        html += "</fieldset>\n";

    }
    document.getElementById('logs').innerHTML = html;
}

// ------------------------------------------------------------------
// Process log data
// ------------------------------------------------------------------

function processLogRows( logrows ) {
    console.log( "Got logdata : "  + logrows );
    
    for ( i=1; i<=11; i++ ) {
        logdata[i] = "";
    }
    
    var rows = logrows.split( ";" );
    var rlen = rows.length;
    for ( i=rlen-1; i>=0; i-- ) {    // Reverse
        var cols = rows[i].split( "|" );
        var from = parseInt( cols[1] );
        var len = logdata[from].length;
        if ( len == 0 ) {
            logdata[from] = timeConv( cols[2], 0 ) + ":" + cols[3];
        } else if ( len < 200 ) {
            logdata[from] = timeConv( cols[2], 0 ) + ":" + cols[3] + ";" + logdata[from];
        }
    }
    
    var html = "";
    for ( i=1; i<=11; i++ ) {
        if ( isHtml5Compatible ) {
            setCallout( i, logdata[i] );
        }
    
        html += "<fieldset class='cbi-section' background-color='#FFFFFF';>\n";
        html += "<legend>"+ logNames[i] + "</legend>\n";
        html += logdata[i].replace( /;/g, "<br />" );
        html += "</fieldset>\n";
            
    }
    
    if ( isHtml5Compatible ) {
        drawAll();
    }
    
    document.getElementById('logs').innerHTML = html;

    /*
        var html = "";
        var text = "";
        var pdata = logdata[filename].split( ";" );  // Previous data
        var ndata = data.split( ";" );               // New data
        var plen = pdata.length - 1;
        var nlen = ndata.length - 1;

        while ( plen >= 0 && nlen >= 0 ) {
            if ( ndata[nlen].length == 0 ) {
                nlen--;    // Skip empty lines
            } else if ( pdata[plen] != ndata[nlen] ) {
                html = "<span style='color:red'>" + ndata[nlen] +
                       "</span><br />" + html;

                text += tsStripped( ndata[nlen] ) + ";";
                nlen--;
            } else if ( pdata[plen] == ndata[nlen] ) {
                html = ndata[nlen] + "<br />" + html;
                nlen--;
                plen--;
            }
        }

        while ( nlen >= 0 ) {
            html = "<span style='color:red'>" + ndata[nlen] +
                   "</span><br />" + html;
            text += ndata[nlen] + ";";
            nlen--;
        }

        logdata[filename] = data;
        document.getElementById(filename).innerHTML = html;

        if ( isHtml5Compatible ) {
            setCallout( filename, text );
            // callouts[filename] = text;
            drawAll();
        }
        */
}

// ------------------------------------------------------------------
// Execute command
// ------------------------------------------------------------------

function LogsExec( post ) {
    console.log('LogsExec : '+post);
    doHttp( post,
        function( xmlhttp ) {
            var xmlDoc = xmlhttp.responseXML;
            var err = getXmlVal( xmlDoc, 'err' );
            var cmd = getXmlVal( xmlDoc, 'cmd' );
            // console.log('cmd='+cmd );
            if ( err == 0 ) {

                var clk = getXmlVal( xmlDoc, 'clk' );
                var pid = getXmlVal( xmlDoc, 'pid' );
                document.getElementById('headerDate').innerHTML =
                           timeConv( clk, 1 ) + " (" + pid + ")";

                switch ( parseInt( cmd ) ) {
                case COMMAND_GET_LOGS:
                    startIndex = parseInt( getXmlVal( xmlDoc, 'index' ) );
                    var logrows = getXmlVal( xmlDoc, 'rows' );
                    if ( logrows != -1 ) processLogRows( logrows );
                    break;
                }
            }
        } );
}

// ------------------------------------------------------------------
// Commands
// ------------------------------------------------------------------

function LogsUpdate() {
    getLogs();
}

function getLogs() {
    LogsExec( 'command='+COMMAND_GET_LOGS+'&index='+startIndex );
}

