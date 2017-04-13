// ---------------------------------------------------------------------
// HSV-RGB Color picker
// ---------------------------------------------------------------------
// http://github.com/rennat/jQuery-ColorPicker
// ---------------------------------------------------------------------

// ---------------------------------------------------------------------
// Conversions
// ---------------------------------------------------------------------

function rgbToHsv(r, g, b){
    r = r/255, g = g/255, b = b/255;
    var max = Math.max(r, g, b), min = Math.min(r, g, b);
    var h, s, v = max;

    var d = max - min;
    s = max == 0 ? 0 : d / max;

    if(max == min){
        h = 0; // achromatic
    }else{
        switch(max){
            case r: h = (g - b) / d + (g < b ? 6 : 0); break;
            case g: h = (b - r) / d + 2; break;
            case b: h = (r - g) / d + 4; break;
        }
        h /= 6;
    }

    return [Math.round(h * 360), s, v];
}

function hsvToRgb(h, s, v){
    var r, g, b;
    var i;
    var f, p, q, t;

    while (h < 0) { h += 360; };
    while (h > 360) { h -= 360; };
    s = Math.max(0, Math.min(1, s));
    v = Math.max(0, Math.min(1, v));

    if(s == 0) {
        r = g = b = v;
        return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
    }

    h /= 60;
    i = Math.floor(h);
    f = h - i;
    p = v * (1 - s);
    q = v * (1 - s * f);
    t = v * (1 - s * (1 - f));

    switch(i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q;
    }

    return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
}

function hsvToHex(h,s,v){
    var rgb = hsvToRgb(h,s,v);
    return rgbToHex(rgb[0], rgb[1], rgb[2]);
};

function hexToRgb(hex) {
    if (hex.substring(0,1) == '#') { hex = hex.substring(1,hex.length); };

    var strPattern = /^([0-9a-f]{3}|[0-9a-f]{6})$/i;
    if(strPattern.test(hex) == true) {
        if (hex.length == 3) {
            var r = hex.substring(0,0);
            var g = hex.substring(1,1);
            var b = hex.substring(2,2);
            hex = r + r + g + g + b + b;
        };
        var r = parseInt(hex.substring(0,2), 16);
        var g = parseInt(hex.substring(2,4), 16);
        var b = parseInt(hex.substring(4,6), 16);
        return [r, g, b];
    } else {
        return false;
    };
};

function rgbToHex(r,g,b) {
    r = parseInt(r).toString(16);
    g = parseInt(g).toString(16);
    b = parseInt(b).toString(16);
    if (r.length < 2) { r = '0' + r; };
    if (g.length < 2) { g = '0' + g; };
    if (b.length < 2) { b = '0' + b; };
    return r + g + b;
};

// ---------------------------------------------------------------------
// JQUERY PICKER
// ---------------------------------------------------------------------

jQuery.ColorPicker = function(container, options) {
    var picker = this;

    picker.build = function(){
        // Container setup
        var container = picker.el.container
            .empty()
            .css({
                'display': 'block',
                'position': 'relative',
                'width': '312px',
                'height': '296px' // 272 + 24
            });

        // Wheel setup
        var wheel_container = picker.el.wheel_container = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '1',
                'top': '8px',
                'left': '8px',
                'width': '272px',
                'height': '272px;'
            }).appendTo(container);
        var wheel = picker.el.wheel = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '2',
                'top': '0',
                'left': '0',
                'width': '256px',
                'height': '256px',
                'background': 'url(/img/jquery.colorpicker.wheel.png)'
            }).appendTo(wheel_container);
        var wheel_mask = picker.el.wheel_mask = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '3',
                'top': '0',
                'left': '0',
                'width': '256px',
                'height': '256px',
                'background': 'url(/img/jquery.colorpicker.wheel_mask.png)',
                'opacity': 0
            }).appendTo(wheel_container);
        var wheel_cursor = picker.el.wheel_cursor = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '4',
                'top': '120px',
                'left': '120px',
                'width': '16px',
                'height': '16px',
                'background': 'url(/img/jquery.colorpicker.wheel_cursor.png)'
            }).appendTo(wheel_container);
        var wheel_hit = picker.el.wheel_hit = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '5',
                'top': '-8px',
                'left': '-8px',
                'width': '272px',
                'height': '272px'
            }).appendTo(wheel_container);

        // Slider setup
        var slider_container = picker.el.slider_container = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '1',
                'top': '8px',
                'left': '280px',
                'width': '24px',
                'height': '288px;' // +32
            }).appendTo(container);
        var slider_bg = picker.el.slider_bg = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '2',
                'top': '0',
                'left': '0',
                'width': '16px',
                'height': '288px', // +32
                'background': picker.color.hex
            }).appendTo(slider_container);
        var slider = picker.el.slider = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '3',
                'top': '0',
                'left': '0',
                'width': '16px',
                'height': '288px', // 256 + 32
                'background': 'url(/img/jquery.colorpicker.longer.slider.png)'
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
                'height': '304px' // 272 + 32
            }).appendTo(slider_container);

        // Horizontal Slider setup
        var horizontal_slider_container = picker.el.horizontal_slider_container = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '1',
                'top': '280px', // 8 + 272
                'left': '8px',
                'width': '256px',
                'height': '24px;'
            }).appendTo(container);
        var horizontal_slider_bg = picker.el.horizontal_slider_bg = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '2',
                'top': '0',
                'left': '0',
                'width': '256px',
                'height': '16px',
                'background': picker.color.hex
            }).appendTo(horizontal_slider_container);
        var horizontal_slider = picker.el.horizontal_slider = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '3',
                'top': '0',
                'left': '0',
                'width': '256px',
                'height': '16px',
                'background': 'url(/img/jquery.colorpicker.slider_horizontal.png)'
            }).appendTo(horizontal_slider_container);
        var horizontal_slider_cursor = picker.el.horizontal_slider_cursor = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '4',
                'top': '0px',
                'left': '-8px',
                'width': '16px',
                'height': '16px',
                'background': 'url(/img/jquery.colorpicker.slider_horizontal_cursor.png)'
            }).appendTo(horizontal_slider_container);
        var horizontal_slider_hit = picker.el.horizontal_slider_hit = jQuery('<div/>')
            .css({
                'position': 'absolute',
                'z-index': '5',
                'top': '-8px',
                'left': '-8px',
                'width': '272px',
                'height': '40px'
            }).appendTo(horizontal_slider_container);

        // events
        wheel_cursor.click(function(){return false;});
        wheel_cursor.mousedown(function(){return false;});
        wheel_hit.mousedown(function(){ picker.mouse.wheel = true; });
        wheel_hit.mouseup(function(){ picker.mouse.wheel = false; });
        wheel_hit.mouseout(function(){ picker.mouse.wheel = false; });
        wheel_hit.click(function(e){ picker.wheel_update(e); });

        slider_cursor.click(function(){return false;});
        slider_cursor.mousedown(function(){return false;});
        slider_hit.mousedown(function(){ picker.mouse.slider = true; });
        slider_hit.mouseup(function(){ picker.mouse.slider = false; });
        slider_hit.mouseout(function(){ picker.mouse.slider = false; });
        slider_hit.click(function(e){ picker.slider_update(e); });

        horizontal_slider_cursor.click(function(){return false;});
        horizontal_slider_cursor.mousedown(function(){return false;});
        horizontal_slider_hit.mousedown(function(){ picker.mouse.horslider = true; });
        horizontal_slider_hit.mouseup(function(){ picker.mouse.horslider = false; });
        horizontal_slider_hit.mouseout(function(){ picker.mouse.horslider = false; });
        horizontal_slider_hit.click(function(e){ picker.horizontal_slider_update(e); });

        jQuery(window).mousemove(function(e){
            if (picker.mouse.wheel) { picker.wheel_update(e); } else
            if (picker.mouse.slider) { picker.slider_update(e); } else
            if (picker.mouse.horslider) { picker.horizontal_slider_update(e); };
        });

        return picker;
    };

    // --------------------------------------------------------
    // Update
    // --------------------------------------------------------

    picker.wheel_update = function(e) {
        console.log("picker.wheel_update");
        var x = Math.max(0,
            Math.min(255, e.pageX - Math.round(picker.el.wheel.offset().left)));
        var y = Math.max(0,
            Math.min(255, e.pageY - Math.round(picker.el.wheel.offset().top)));
        var d = Math.sqrt(Math.pow(x-picker.math.center.x,2) +
                          Math.pow(y-picker.math.center.y,2));
        if (d <= picker.math.radius) {
            picker.el.wheel_cursor.css({'top': y-8+'px', 'left': x-8+'px'});
            var angle = picker.math.angle =
                      Math.atan2((picker.math.center.y - y),
                                 (picker.math.center.x - x));
            var hue = picker.color.hue = Math.round(angle * 180/Math.PI) + 90;
            if (hue < 0) { hue = picker.color.hue += 360; };
            picker.color.sat = d/128;

            picker.color.hex = hsvToHex(picker.color.hue, picker.color.sat, 1);
            picker.el.slider_bg.css('background', '#' + picker.color.hex);
            picker.lastkelvin = 0;

            picker.update();
        };
        return picker;
    };

    picker.slider_update = function(e) {
        console.log( "start val = " + picker.color.val );
        var pxOffset = Math.max(0,
            Math.min(304, e.pageY - Math.round(picker.el.slider.offset().top)));
        var val = Math.round(100 - (pxOffset * 0.32895)) * 0.01; // change from  0.390625 == 1/2.56 to 1/3.04
        picker.el.slider_cursor.css('top', pxOffset - 8 + 'px');
        picker.color.val = val;

        var fullVal = hsvToHex(picker.color.hue, picker.color.sat, 1);
        picker.el.slider_bg.css('background', '#' + fullVal);
        // Do no modify picker.el.wheel_mask.css('opacity', 0);

        picker.update();

        return picker;
    };

    picker.horizontal_slider_update = function(e) {
        console.log( "HOR start val = " + picker.color.val );
        var pxOffset = Math.max(0, Math.min(255, e.pageX - Math.round(picker.el.horizontal_slider.offset().left)));

        var delta;
        delta = ( picker.settings.kelvinmax - picker.settings.kelvinmin ) / 255;
        picker.color.kelvin = Math.round( picker.settings.kelvinmin + delta * pxOffset );
        picker.color.hex = picker.kelvins_rgbs[pxOffset];

        picker.el.horizontal_slider_cursor.css('left', pxOffset - 8 + 'px');
        picker.lastkelvin = 1;

        picker.update();

        return picker;
    };

    picker.update = function() {
        console.log( "update val = " + picker.color.val );
        picker.fn.change(picker.color.kelvin, '#' + picker.color.hex,
            Math.round( picker.color.val*100 ), picker.lastkelvin );
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

    // --------------------------------------------------------
    // Public functions
    // --------------------------------------------------------
    picker.kelvins = function(kmin,kmax) {
        picker.settings.kelvinmin = kmin;
        picker.settings.kelvinmax = kmax;
        calcKelvins();
    }

    picker.set = function(kelvin,hex,lvl) {
        console.log( "COL set kelvin="+kelvin+", hex="+hex+", lvl="+lvl );

        // KELVIN
        picker.color.kelvin = kelvin;

        var delta, index;
        delta = ( picker.settings.kelvinmax - picker.settings.kelvinmin ) / 255;
        kelvin -= picker.settings.kelvinmin;
        index = Math.round( kelvin / delta );
        console.log( "- index="+index );

        var x = index;
        var y = picker.math.center.y;
        picker.el.horizontal_slider_cursor.css({
            'top': '0px',
            'left': x - 4 + 'px'
        });
        
        // HEX
        if (hex.substring(0,1) == '#') { hex = hex.substring(1,hex.length); };

        var rgb = hexToRgb(hex);
        var hsv = rgbToHsv(rgb[0], rgb[1], rgb[2]);

        picker.color.hex = hex;
        picker.color.hue = hsv[0];
        picker.color.sat = hsv[1];
        picker.color.val = hsv[2];

        var adeg = picker.color.hue + 90;
        if (adeg < 0) { adeg += 360; };
        var arad = adeg * Math.PI/180;
        var x2 = picker.math.center.x +
            Math.round(picker.color.sat * picker.math.radius * Math.cos(arad));
        var y2 = picker.math.center.y +
            Math.round(picker.color.sat * picker.math.radius * Math.sin(arad));
        picker.el.wheel_cursor.css({
            'top': y2 - 8 + 'px',
            'left': x2 - 8 + 'px'
        });
        picker.el.slider_bg.css('background', '#' + picker.color.hex);

        // VAL
		// var opacity = (100 - lvl) / 400 // Reduce opacity range, wheel is not visible anymore at low levels
        picker.color.val = lvl/100;

        var top =  304 * (1 - picker.color.val) - 8;
        picker.el.slider_cursor.css('top', top + 'px');
        // picker.el.wheel_mask.css('opacity', opacity); // picker.el.wheel_mask.css('opacity', 1 - picker.color.val);

        picker.update();

        return picker;
    };

    picker.init = function(){
        var container = arguments[0];
        var options = (arguments[1]) ? arguments[1] : {};

        picker.settings = jQuery.extend({
            color: '#ffffff',
            kelvinmin: 1000,
            kelvinmax: 10000,
            change: function(kelvin,hex,level){}
        }, options);

        picker.kelvins_r = [];
        picker.kelvins_g = [];
        picker.kelvins_b = [];
        picker.kelvins_rgbs = [];
        
        picker.lastkelvin = 0;

        picker.fn = {};
        picker.fn.change = picker.settings.change;
        picker.el = {};
        picker.el.container = jQuery(container);
        picker.io = {shift: false};
        picker.mouse = {wheel: false, slider: false};
        picker.math = {};
        picker.math.angle = 0;
        picker.math.center = {x: 128, y: 128};
        picker.math.radius = 128;
        picker.color = {};
		picker.color.kelvin = 2700;
        picker.color.hue = 0;
        picker.color.sat = 0;
        picker.color.val = 1;
        picker.color.hex = "ffffff";
        picker.build().set(picker.color.kelvin, picker.color.hex, picker.color.val);

        calcKelvins();

        return picker;
    };
    return picker.init(container, options);
};
