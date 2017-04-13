# ------------------------------------------------------------------
# Database CGI script makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

TARGET = iot_database.cgi

INCLUDES = -I../../../IotCommon
OBJECTS = cgi_database.o \
	../../../IotCommon/atoi.o \
        ../../../IotCommon/gateway.o \
	../../../IotCommon/iotError.o \
	../../../IotCommon/fileCreate.o \
	../../../IotCommon/socket.o \
	../../../IotCommon/colorConv.o \
	../../../IotCommon/RgbSpaceMatrices.o \
	../../../IotCommon/blackbody.o \
	../../../IotCommon/jsonCreate.o \
	../../../IotCommon/iotSemaphore.o \
	../../../IotCommon/newDb.o \
	../../../IotCommon/dump.o \
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

