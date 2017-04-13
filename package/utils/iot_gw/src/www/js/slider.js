/**
 * Slider
 */

jQuery.fn.slider = function (callback) {
  $.slider(this, callback);
  return this;
};

jQuery.slider = function (container, callback) {
  var container = $(container).get(0);
  return container.slider || (container.slider = new jQuery._slider(container, callback));
}

jQuery._slider = function (container, callback) {
  // Store slider object
  var fb = this;

  // Insert markup
  $(container).html('<div class="slider"><div class="level"></div><div class="wheel"><div class="overlay"></div><div class="sl-marker marker"></div></div>');
  var e = $('.slider', container);
  fb.wheel = $('.wheel', container).get(0);
  // Dimensions
  fb.radius = 84;
  fb.square = 100;
  fb.width = 100;

  // Fix background PNGs in IE6
  if (navigator.appVersion.match(/MSIE [0-6]\./)) {
    $('*', e).each(function () {
      if (this.currentStyle.backgroundImage != 'none') {
        var image = this.currentStyle.backgroundImage;
        image = this.currentStyle.backgroundImage.substring(5, image.length - 2);
        $(this).css({
          'backgroundImage': 'none',
          'filter': "progid:DXImageTransform.Microsoft.AlphaImageLoader(enabled=true, sizingMethod=crop, src='" + image + "')"
        });
      }
    });
  }

  /**
   * Link to the given element(s) or callback.
   */
  fb.linkTo = function (callback) {
    // Unbind previous nodes
    if (typeof fb.callback == 'object') {
      $(fb.callback).unbind('keyup', fb.updateValue);
    }

    // Reset level
    fb.level = 100;

    // Bind callback or elements
    if (typeof callback == 'function') {
      fb.callback = callback;
    }
    else if (typeof callback == 'object' || typeof callback == 'string') {
      fb.callback = $(callback);
      fb.callback.bind('keyup', fb.updateValue);
      if (fb.callback.get(0).value) {
        fb.setLevel(fb.callback.get(0).value);
      }
    }
    return this;
  }
  fb.updateValue = function (event) {
    if (this.value && this.value != fb.level) {
      fb.setLevel(this.value);
    }
  }

  /**
   * Change level
   */
  fb.setLevel = function (lvl) {
    if (fb.level != lvl ) {
      fb.level = lvl;
      fb.updateDisplay();
    }
    return this;
  }

  /////////////////////////////////////////////////////

  /**
   * Retrieve the coordinates of the given event relative to the center
   * of the widget.
   */
  fb.widgetCoords = function (event) {
    var x, y;
    var el = event.target || event.srcElement;
    var reference = fb.wheel;

    // console.log( 'X,Y = ' + el.mouseX + ', ' + el.mouseY );
    // console.log( 'offsetX,Y = ' + event.offsetX + ', ' + event.offsetY );

    if (typeof event.offsetX != 'undefined') {
      // Use offset coordinates and find common offsetParent
      var pos = { x: event.offsetX, y: event.offsetY };
      // console.log( '1 posX,Y = ' + pos.x + ', ' + pos.y );

      // Send the coordinates upwards through the offsetParent chain.
      var e = el;
      while (e) {
        e.mouseX = pos.x;
        e.mouseY = pos.y;
        pos.x += e.offsetLeft;
        pos.y += e.offsetTop;
        e = e.offsetParent;
      }

      // Look for the coordinates starting from the wheel widget.
      var e = reference;
      var offset = { x: 0, y: 0 }
      while (e) {
        if (typeof e.mouseX != 'undefined') {
          x = e.mouseX - offset.x;
          y = e.mouseY - offset.y;
          break;
        }
        offset.x += e.offsetLeft;
        offset.y += e.offsetTop;
        e = e.offsetParent;
      }

      // Reset stored coordinates
      e = el;
      while (e) {
        e.mouseX = undefined;
        e.mouseY = undefined;
        e = e.offsetParent;
      }
      // console.log( '1 posX,Y = ' + pos.x + ', ' + pos.y );
    }
    else {
      // Use absolute coordinates
      var pos = fb.absolutePosition(reference);
      // console.log( '2 posX,Y = ' + pos.x + ', ' + pos.y );
      x = (event.pageX || 0*(event.clientX + $('html').get(0).scrollLeft)) - pos.x;
      y = (event.pageY || 0*(event.clientY + $('html').get(0).scrollTop)) - pos.y;
    }
    // console.log( 'X,Y = ' + x + ', ' + y );
    // Subtract distance to middle
    return { x: x - fb.width / 2, y: y - fb.width / 2 };
  }

  /**
   * Mousedown handler
   */
  fb.mousedown = function (event) {
    // Capture mouse
    if (!document.dragging) {
      $(document).bind('mousemove', fb.mousemove).bind('mouseup', fb.mouseup);
      document.dragging = true;
    }

    // Check which area is being dragged
    var pos = fb.widgetCoords(event);
    // console.log('Mouse-down ' + pos.x );

    // Process
    fb.mousemove(event);
    return false;
  }

  /**
   * Mousemove handler
   */
  fb.mousemove = function (event) {
    // Get coordinates relative to level picker center
    var pos = fb.widgetCoords(event);
    var lvl = Math.max(0, Math.min(1, (pos.x / fb.square) + .5));
    // console.log('Mouse-move ' + pos.x + ', lvl = ' + lvl );
    fb.setLevel(lvl);
    return false;
  }

  /**
   * Mouseup handler
   */
  fb.mouseup = function () {
    // Uncapture mouse
    $(document).unbind('mousemove', fb.mousemove);
    $(document).unbind('mouseup', fb.mouseup);
    document.dragging = false;
  }

  /**
   * Update the markers and styles
   */
  fb.updateDisplay = function () {
    // Markers
    $('.sl-marker', e).css({
      left: Math.round(fb.square * (fb.level - 0.5) + fb.width / 2) + 'px',
      top: Math.round(fb.width / 2) + 'px'
    });

    // Linked elements or callback
    if (typeof fb.callback == 'object') {
      // Change linked value
      $(fb.callback).each(function() {
        if (this.value && this.value != fb.level) {
          this.value = fb.level;
        }
      });
    }
    else if (typeof fb.callback == 'function') {
      fb.callback.call(fb, fb.level);
    }
  }

  /**
   * Get absolute position of element
   */
  fb.absolutePosition = function (el) {
    var r = { x: el.offsetLeft, y: el.offsetTop };
    // Resolve relative to offsetParent
    if (el.offsetParent) {
      var tmp = fb.absolutePosition(el.offsetParent);
      r.x += tmp.x;
      r.y += tmp.y;
    }
    return r;
  };

  console.log('SLIDER - JAVASCRIPT');

  // Install mousedown handler (the others are set on the document on-demand)
  $('*', e).mousedown(fb.mousedown);

    // Init Level
  fb.setLevel(99);

  // Set linked elements/callback
  if (callback) {
    fb.linkTo(callback);
  }
}
