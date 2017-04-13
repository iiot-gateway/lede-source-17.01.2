# ------------------------------------------------------------------
# Upload makefile for IoT website stuff
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

INDEX = index.html

ifeq ($(TARGET_MACHINE),RASPBERRYPI)
ifeq ($(TARGET_OS),RASPBIAN)
HEADER = header_rpi.html
else
HEADER = header_orpi.html
endif
else
HEADER = header_openwrt.html
endif

all: clean build

build:
	cd img; make build
	cd js;  make build
	cd css; make build
	cd cgi-bin; make build
	install -m 644 -D $(INDEX)  ../swupdate/images/www/$(INDEX)
	install -m 644 -D $(HEADER) ../swupdate/images/www/header.html

clean:
	cd img; make clean
	cd js;  make clean
	cd css; make clean
	cd cgi-bin; make clean

