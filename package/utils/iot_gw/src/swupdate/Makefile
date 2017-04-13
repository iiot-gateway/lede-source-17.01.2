# ------------------------------------------------------------------
# IoT Update Makefile
# - Creates targz image. Version info has to be done by hand
# - Puts a copy of the image into the archive
# - Copies new SW image to the cloud server
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

SOURCES= iot_update.sh iot_image.tgz iot_version.txt iot_virgin.sh
RELEASE= iot_update_rel.sh iot_image_rel.tgz iot_version_rel.txt

ifeq ($(TARGET_MACHINE),RASPBERRYPI)
ifeq ($(TARGET_OS),RASPBIAN)
UPDATE_TEMPLATE=iot_update.sh.template.rpi
VIRGIN=iot_virgin.sh.rpi
else
UPDATE_TEMPLATE=iot_update.sh.template.orpi
VIRGIN=iot_virgin.sh.orpi
endif

else
UPDATE_TEMPLATE=iot_update.sh.template.openwrt
VIRGIN=iot_virgin.sh.openwrt
OPENWRT= openwrt-lpc32xx-JNRD6040-with-SPI.bin
endif

DESTINATION=$(CLOUD_USER)@$(CLOUD_SERVER):$(CLOUD_DIRECTORY)
ARCHIVE=`date +"iot_image_%Y%m%d%H%M%S"`

all: clean build

update:
	sed 's?CLOUD_IMAGES_URL?$(CLOUD_IMAGES_URL)?' < $(UPDATE_TEMPLATE) > iot_update.sh
	rm iot_version.txt
	ln -s images/usr/share/iot/iot_version.txt .
	date >> iot_version.txt
	vi iot_version.txt
	tar -czf iot_image.tgz images/*
	cp iot_image.tgz archive/$(ARCHIVE).tgz
	cp $(VIRGIN) iot_virgin.sh
	pscp -p -scp -pw $(CLOUD_PASSWORD) $(SOURCES) $(DESTINATION)

release: update
	sed 's?iot_image?iot_image_rel?' < iot_update.sh > iot_update_rel.sh
	cp iot_version.txt iot_version_rel.txt
	cp iot_image.tgz iot_image_rel.tgz
	pscp -p -scp -pw $(CLOUD_PASSWORD) $(RELEASE) $(DESTINATION)

build:

upload:

clean:

virgin:
	cp $(VIRGIN) iot_virgin.sh
	pscp -p -scp -pw $(TARGET_PASSWORD) iot_virgin.sh iot_image.tgz $(TARGET_USER)@$(TARGET_IP):/tmp

ifeq ($(TARGET_MACHINE),JNRD6040)
openwrt:
	pscp -p -scp -pw $(CLOUD_PASSWORD) $(OPENWRT) $(DESTINATION)
endif
