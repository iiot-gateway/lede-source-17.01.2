# ------------------------------------------------------------------
# Upload makefile for IoT JS stuff
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

JS   = iot.js \
	control.js database.js logs.js tz.js system.js \
	lamps.js mousewheel.js colorpicker.js twpicker.js slider.js jquery.js \
	combined_colorpicker.js \
	plugs.js c3.min.js d3.min.js \
	groups.js mobile.js iotDbSpec.js

all: build upload

build:
	mkdir -p ../../swupdate/images/www/js
	cp -t ../../swupdate/images/www/js $(JS)

upload:
	pscp -p -scp -pw $(TARGET_PASSWORD) $(JS)  $(TARGET_USER)@$(TARGET_IP):$(TARGET_WWW)/js

clean:

