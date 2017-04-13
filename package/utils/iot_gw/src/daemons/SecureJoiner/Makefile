# ------------------------------------------------------------------
# Secure Joiner makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

TARGET = iot_sj

INCLUDES = -I../../IotCommon
OBJECTS = sj_main.o \
	linkinfo.o \
	commission.o \
	../../IotCommon/strtoupper.o \
	../../IotCommon/atoi.o \
	../../IotCommon/colorConv.o \
	../../IotCommon/RgbSpaceMatrices.o \
	../../IotCommon/blackbody.o \
	../../IotCommon/iotError.o \
	../../IotCommon/gateway.o \
	../../IotCommon/newDb.o \
	../../IotCommon/systemtable.o \
	../../IotCommon/parsing.o \
	../../IotCommon/json.o \
	../../IotCommon/jsonCreate.o \
	../../IotCommon/fileCreate.o \
	../../IotCommon/iotSemaphore.o \
	../../IotCommon/queue.o \
	../../IotCommon/socket.o \
	../../IotCommon/dump.o \
	../../IotCommon/nibbles.o \
	../../IotCommon/newLog.o


%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@

all: clean build

build: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET) -lc
	cp $(TARGET) ../../swupdate/images/usr/bin/

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
	-rm -f ../../swupdate/images/usr/bin/$(TARGET)

