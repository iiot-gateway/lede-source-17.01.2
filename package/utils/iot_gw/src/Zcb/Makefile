# ------------------------------------------------------------------
# Makefile for copying the Zigbee Coordinator binary
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

ifeq ($(strip $(ZCB_VERSION)),)
	ZCB_VERSION=_3v0
endif

ZCB = \
ZigbeeNodeControlBridge*_JN5168.bin \
ZigbeeNodeControlBridge*_JN5169.bin \
ZigbeeNodeControlBridge*_JN5179.bin \
ZigbeeProtocolVersion.txt

all: clean build

build:
	echo $(ZCB_VERSION) > ./ZigbeeProtocolVersion.txt
	mkdir -p ../swupdate/images/tmp
	cp -t ../swupdate/images/tmp $(ZCB)
	ln -sf ZigbeeNodeControlBridge_3v0_JN5179.bin \
		../swupdate/images/tmp/ZigbeeNodeControlBridge_2v0_JN5179.bin # for JN5179, ZB2.0 is not supported
	mkdir -p ../swupdate/images/usr/share/iot
	cp -t ../swupdate/images/usr/share/iot $(ZCB)
	ln -sf ZigbeeNodeControlBridge_3v0_JN5179.bin \
		../swupdate/images/usr/share/iot/ZigbeeNodeControlBridge_2v0_JN5179.bin # for JN5179, ZB2.0 is not supported

clean:
	-rm -rf ./ZigbeeProtocolVersion.txt
	-rm -rf ../swupdate/images/tmp
	-rm -rf ../swupdate/images/usr/share/iot

