// ---------------------------------------------------------------------
// Tunable White color picker
// ---------------------------------------------------------------------
// http://www.tannerhelland.com/4435/convert-temperature-rgb-algorithm-code/
// http://github.com/rennat/jQuery-ColorPicker
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
// Conversions
// ---------------------------------------------------------------------

function kelvin2rgbi(colorTemp) {
    var red = 0, green = 0, blue = 0;
 
    // Handle min and max values of algorithm
    if (colorTemp < 1000) {
        colorTemp = 1000;
    } else if (colorTemp > 40000) {
        colorTemp = 40000;
    }
 
    // Divide by 100 here instead of everywhere
    colorTemp /= 100;
 
    // Calculate Red
    if (colorTemp <= 66) {
        red = 255;
    } else {
        red = colorTemp - 60;
        red = Math.round(329.698727446 * Math.pow(red, -0.1332047592));
    }
 
    // Calculate Green
    if (colorTemp <= 66) {
        green = colorTemp;
        green = Math.round(99.4708025861 * Math.log(green) - 161.1195681661);
    } else {
        green = colorTemp - 60;
        green = Math.round(288.1221695283 * Math.pow(green, -0.0755148492));
    }
 
    // Calculate Blue
    if (colorTemp >= 66) {
        blue = 255;
    } else {
        if (colorTemp <= 19) {
            blue = 0;
        } else {
            blue = colorTemp - 10;
            blue = Math.round(138.5177312231 * Math.log(blue) - 305.0447927307);
        }
    }
 
    // Check RGB boundaries
    if (red   < 0) red   = 0; else if (red   > 255) red   = 255;
    if (green < 0) green = 0; else if (green > 255) green = 255;
    if (blue  < 0) blue  = 0; else if (blue  > 255) blue  = 255;

    // console.log( "Kelvin: " + colorTemp + " ->  ", red, green, blue );

    return( ( red << 16 ) + ( green << 8 ) + blue );
}

function rgbi2rgbs(rgbi) {
    var red   = ( rgbi >> 16 ) & 0xFF;
    var green = ( rgbi >> 8  ) & 0xFF;
    var blue  = ( rgbi       ) & 0xFF;

    // Pack
    var r = red.toString(16);   if (r.length < 2) { r = '0' + r; };
    var g = green.toString(16); if (g.length < 2) { g = '0' + g; };
    var b = blue.toString(16);  if (b.length < 2) { b = '0' + b; };

    return r+g+b;
}

function rgbs2rgbi(rgbs) {
    var r = parseInt( rgbs.substr(0,2), 16 );
    var g = parseInt( rgbs.substr(2,2), 16 );
    var b = parseInt( rgbs.substr(4,2), 16 );
    return( ( r << 16 ) + ( g << 8 ) + b );
}

// ---------------------------------------------------------------------
// JQUERY PICKER
// ---------------------------------------------------------------------

jQuery.TwPicker = function(container, options) {
    var picker = this;
    
    picker.build = function(){
        // Container setup
        var container = picker.el.container
            .empty()
            .css({
                'display': 'block',
                'position': 'relative',
                'width': '312px',
                'height': '272px'
            });
        
        // Wheel setup
        var twcurve_container = picker.el.twcurve_container = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '1',
                'top': '8px',
                'left': '8px',
                'width': '272px',
                'height': '272px;'
            }).appendTo(container);
        var twcurve = picker.el.twcurve = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '2',
                'top': '0',
                'left': '0',
                'width': '256px',
                'height': '256px',
                'background': '#444444'
            }).appendTo(twcurve_container);

        var twcanvas = jQuery( '<canvas/>' )
            .css({
                'position': 'absolute',
                'z-index': '3',
                'top': '0px',
                'left': '0px',
                'width': '256px',
                'height': '256px',
                'background': '#222244'
            }).appendTo(twcurve);
        picker.el.twcurve_canvas = twcanvas.get(0);  // twcanvas[0];
        
        
        console.log( "Type of "+ typeof( twcanvas ) );
        for (var key in twcanvas[0] ) {
            if (typeof twcanvas[0][key] == "function") {
                console.log( "Method "+key );
            } else {
                console.log( "Property "+key );
            }
        }

        var twcurve_mask = picker.el.twcurve_mask = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '4',
                'top': '0',
                'left': '0',
                'width': '256px',
                'height': '256px',
                'background': 'url(/img/tw_mask.png)',
                'opacity': 0
            }).appendTo(twcurve_container);
        var twcurve_cursor = picker.el.twcurve_cursor = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '5',
                'top': '0px',
                'left': '124px',
                'width': '8px',
                'height': '256px',
                'background': 'url(/img/tw_cursor.png)'
            }).appendTo(twcurve_container);
        var twcurve_hit = picker.el.twcurve_hit = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '6',
                'top': '-8px',
                'left': '-8px',
                'width': '272px',
                'height': '272px'
            }).appendTo(twcurve_container);
        
        // Slider setup
        var slider_container = picker.el.slider_container = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '1',
                'top': '8px',
                'left': '280px',
                'width': '24px',
                'height': '256px;'
            }).appendTo(container);
        var slider_bg = picker.el.slider_bg = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '2',
                'top': '0',
                'left': '0',
                'width': '16px',
                'height': '256px',
                'background': picker.color.hex
            }).appendTo(slider_container);
        var slider = picker.el.slider = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '3',
                'top': '0',
                'left': '0',
                'width': '16px',
                'height': '256px',
                'background': 'url(/img/jquery.colorpicker.slider.png)'
            }).appendTo(slider_container);
        var slider_cursor = picker.el.slider_cursor = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '4',
                'top': '-8px',
                'left': '8px',
                'width': '16px',
                'height': '16px',
                'background': 'url(/img/jquery.colorpicker.slider_cursor.png)'
            }).appendTo(slider_container);
        var slider_hit = picker.el.slider_hit = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '5',
                'top': '-8px',
                'left': '-8px',
                'width': '40px',
                'height': '272px'
            }).appendTo(slider_container);

        // events
        twcurve_cursor.click(function(){return false;});
        twcurve_cursor.mousedown(function(){return false;});
        twcurve_hit.mousedown(function(){ picker.mouse.twcurve = true; });
        twcurve_hit.mouseup(function(){ picker.mouse.twcurve = false; });
        twcurve_hit.mouseout(function(){ picker.mouse.twcurve = false; });
        twcurve_hit.click(function(e){ picker.twcurve_update(e); });
        
        slider_cursor.click(function(){return false;});
        slider_cursor.mousedown(function(){return false;});
        slider_hit.mousedown(function(){ picker.mouse.slider = true; });
        slider_hit.mouseup(function(){ picker.mouse.slider = false; });
        slider_hit.mouseout(function(){ picker.mouse.slider = false; });
        slider_hit.click(function(e){ picker.slider_update(e); });
        
        jQuery(window).mousemove(function(e){
            if (picker.mouse.twcurve) { picker.twcurve_update(e); } else
            if (picker.mouse.slider) { picker.slider_update(e); };
        });
        
        return picker;
    };
    
    // --------------------------------------------------------
    // UI
    // --------------------------------------------------------

    picker.twcurve_update = function(e) {
        var x = Math.max(0, Math.min(255, e.pageX - Math.round(picker.el.twcurve.offset().left)));

        var delta;
        delta = ( picker.settings.kelvinmax - picker.settings.kelvinmin ) / 255;
        picker.color.kelvin = Math.round( picker.settings.kelvinmin + delta*x );

        picker.color.hex = picker.kelvins_rgbs[x];
        console.log("twcurve_update, X="+x+", col="+picker.color.hex );
        picker.el.twcurve_cursor.css({'top': '0px', 'left': x-4+'px'});
        picker.el.slider_bg.css('background', '#' + picker.color.hex);
            
        picker.update();
        return picker;
    };
    
    picker.slider_update = function(e) {
console.log("picker.slider_update");
        var pxOffset = Math.max(0, Math.min(255, e.pageY - Math.round(picker.el.slider.offset().top)));
        var val = Math.round(100 - (pxOffset * 0.390625)) * 0.01;
        picker.el.slider_cursor.css('top', pxOffset - 8 + 'px');
        picker.color.val = val;
        
        picker.el.slider_bg.css('background', '#' + picker.color.hex);
        // picker.el.twcurve_mask.css('opacity', 1 - picker.color.val);
        
        picker.update();

        return picker;
    };
    
    // --------------------------------------------------------
    // Update
    // --------------------------------------------------------
    
    picker.update = function() {
        picker.fn.change( picker.color.kelvin, '#' + picker.color.hex, 
            Math.round( picker.color.val*100 ) );
    };
    picker.change = picker.update;
    
    // --------------------------------------------------------
    // Kelvins
    // --------------------------------------------------------

    function calcKelvins() {
        var i, kelvin, delta;
        delta = ( picker.settings.kelvinmax - picker.settings.kelvinmin ) / 255;
        kelvin = picker.settings.kelvinmin;
        for ( i=0; i<256; i++ ) {
            var rgbi = kelvin2rgbi( kelvin );
            picker.kelvins_r[i] = ( rgbi >> 16 ) & 0xFF;
            picker.kelvins_g[i] = ( rgbi >> 8  ) & 0xFF;
            picker.kelvins_b[i] = ( rgbi       ) & 0xFF;
            picker.kelvins_rgbs[i] = rgbi2rgbs( rgbi );
            kelvin += delta;
        }
    }
    
    function calcCanvas() {
        console.log( "Type of "+ typeof( picker.el.twcurve_canvas ) );
        for (var key in picker.el.twcurve_canvas) {
            if (typeof picker.el.twcurve_canvas[key] == "function") {
                console.log( "Method "+key );
            } else {
                console.log( "Property "+key );
            }
        }
        var cvs = picker.el.twcurve_canvas;
        var ctx = cvs.getContext("2d");
        cvs.width = cvs.height = 256;
        // ctx.fillStyle = "#FF0000";
        // ctx.fillRect( 40, 40, 100, 100 );
        // ctx.globalAlpha = 1.0;
        var i;
        for ( i=0; i<256; i++ ) {
            var r = picker.kelvins_r[i].toString(16);
            if (r.length < 2) { r = '0' + r; };
            var g = picker.kelvins_g[i].toString(16);
            if (g.length < 2) { g = '0' + g; };
            var b = picker.kelvins_b[i].toString(16);
            if (b.length < 2) { b = '0' + b; };
            ctx.strokeStyle = '#' + r + g + b;
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.moveTo( i, 0 );
            ctx.lineTo( i, 256 );
            ctx.stroke();
        }
        // ctx.fillStyle = "#00FF00";
        // ctx.fillRect( 40, 40, 40, 40 );
    }
        
    // --------------------------------------------------------
    // Public functions
    // --------------------------------------------------------

    picker.set = function(kelvin,lvl) {
        console.log( "TW set kelvin="+kelvin+", lvl="+lvl );

        picker.color.kelvin = kelvin;

        var delta, index;
        delta = ( picker.settings.kelvinmax - picker.settings.kelvinmin ) / 255;
        kelvin -= picker.settings.kelvinmin;
        index = Math.round( kelvin / delta );
        console.log( "- index="+index );

        var x = index;
        var y = picker.math.center.y;
        picker.el.twcurve_cursor.css({
            'top': '0px',
            'left': x - 4 + 'px'
        });
        picker.color.hex = picker.kelvins_rgbs[index];
        console.log( "- rgb="+picker.color.hex );
        
        // VAL
        picker.color.val = lvl/100;
        
        var top =  256 * (1 - picker.color.val) - 8;
        picker.el.slider_cursor.css('top', top + 'px');
        // picker.el.twcurve_mask.css('opacity', 1 - picker.color.val);
    
        picker.update();

        return picker;
    };
    
    picker.kelvins = function(kmin,kmax) {
        picker.settings.kelvinmin = kmin;
        picker.settings.kelvinmax = kmax;
        calcKelvins();
        calcCanvas();
    }

    picker.init = function(){
        var container = arguments[0];
        var options = (arguments[1]) ? arguments[1] : {};
        
        picker.settings = jQuery.extend({
            color: '#ffffff',
            kelvinmin: 1000,
            kelvinmax: 10000,
            change: function(hex,level){}
        }, options);
        
        picker.kelvins_r = [];
        picker.kelvins_g = [];
        picker.kelvins_b = [];
        picker.kelvins_rgbs = [];

        picker.fn = {};
        picker.fn.change = picker.settings.change;
        picker.el = {};
        picker.el.container = jQuery(container);
        picker.io = {shift: false};
        picker.mouse = {twcurve: false, slider: false};
        picker.math = {};
        picker.math.center = {x: 128, y: 128};
        picker.math.radius = 128;
        picker.color = {};
        picker.color.kelvin = 2700;
        picker.color.hue = 0;
        picker.color.sat = 0;
        picker.color.val = 1;
        picker.color.hex = "ffffff";
        picker.build().set(picker.color.kelvin, picker.color.val);

        calcKelvins();
        calcCanvas();

        return picker;
    };
    return picker.init(container, options);
};
