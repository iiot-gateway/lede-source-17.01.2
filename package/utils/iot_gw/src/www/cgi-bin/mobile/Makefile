# ------------------------------------------------------------------
# Mobile CGI script makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

TARGET = iot_mobile.cgi

INCLUDES = -I../../../IotCommon
OBJECTS = cgi_mobile.o \
	../../../IotCommon/atoi.o \
	../../../IotCommon/colorConv.o \
	../../../IotCommon/RgbSpaceMatrices.o \
	../../../IotCommon/blackbody.o \
	../../../IotCommon/iotError.o \
	../../../IotCommon/plugUsage.o \
        ../../../IotCommon/gateway.o \
	../../../IotCommon/fileCreate.o \
	../../../IotCommon/iotSemaphore.o \
	../../../IotCommon/newDb.o \
	../../../IotCommon/socket.o \
	../../../IotCommon/dump.o \
	../../../IotCommon/jsonCreate.o \
	../../../IotCommon/newLog.o


%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@

all: clean build

build: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET) -lc
	cp $(TARGET) ../../../swupdate/images/www/cgi-bin

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
	-rm -f ../../../swupdate/images/www/cgi-bin/$(TARGET)

