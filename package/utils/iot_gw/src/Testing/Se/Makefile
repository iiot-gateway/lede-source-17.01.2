# ------------------------------------------------------------------
# Secure Element makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

TARGET = iot_se

INCLUDES = -I../../IotCommon
OBJECTS = se_main.o \
	../../IotCommon/atoi.o \
	../../IotCommon/gateway.o \
	../../IotCommon/iotError.o \
	../../IotCommon/fileCreate.o \
	../../IotCommon/dump.o \
	../../IotCommon/iotSemaphore.o \
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

