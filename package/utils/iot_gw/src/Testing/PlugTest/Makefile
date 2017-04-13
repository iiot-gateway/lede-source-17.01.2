# ------------------------------------------------------------------
# Plug Test makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

TARGET = iot_pt

INCLUDES = -I../../IotCommon
OBJECTS = pt_main.o \
	../../IotCommon/atoi.o \
	../../IotCommon/colorConv.o \
	../../IotCommon/RgbSpaceMatrices.o \
	../../IotCommon/blackbody.o \
	../../IotCommon/iotError.o \
	../../IotCommon/jsonCreate.o \
	../../IotCommon/fileCreate.o \
	../../IotCommon/newDb.o \
	../../IotCommon/iotSemaphore.o \
	../../IotCommon/queue.o \
	../../IotCommon/plugUsage.o \
	../../IotCommon/dump.o \
	../../IotCommon/newLog.o


%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@

all: clean build

build: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET) -lc
	mkdir -p ../../swupdate/images/usr/bin/
	cp -f $(TARGET) ../../swupdate/images/usr/bin/

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)

