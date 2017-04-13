# ------------------------------------------------------------------
# GW discovery makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

TARGET = iot_gd

LDLIBS = -lc
INCLUDES = -I../../IotCommon
OBJECTS = gd_main.o \
	../../IotCommon/atoi.o \
	../../IotCommon/blackbody.o \
	../../IotCommon/colorConv.o \
	../../IotCommon/RgbSpaceMatrices.o \
	../../IotCommon/jsonCreate.o \
	../../IotCommon/gateway.o \
	../../IotCommon/dump.o \
	../../IotCommon/fileCreate.o \
	../../IotCommon/iotSemaphore.o \
	../../IotCommon/newLog.o

	
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@

all: clean build

build: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET) $(LDLIBS) 
	cp $(TARGET) ../../swupdate/images/usr/bin/

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
	-rm -f ../../swupdate/images/usr/bin/$(TARGET)

