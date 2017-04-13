# ------------------------------------------------------------------
# Nfc program makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2015. All rights reserved
# ------------------------------------------------------------------

LDFLAGS += -Llibnci/lib -Wl,--no-as-needed
LDLIBS += -lpthread -ldl -lrt -lc -lnci

TARGET = iot_nd

INCLUDES = -I. -Ilibnci/inc -I../../IotCommon

OBJECTS = nd_main.o \
	mful.o \
	ndef.o \
	commission.o \
	audiofb.o \
	ledfb.o \
	tools.o \
	nciConfig.o \
	../../IotCommon/atoi.o \
	../../IotCommon/nibbles.o \
	../../IotCommon/jsonCreate.o \
	../../IotCommon/colorConv.o \
	../../IotCommon/RgbSpaceMatrices.o \
	../../IotCommon/blackbody.o \
	../../IotCommon/gateway.o \
	../../IotCommon/fileCreate.o \
	../../IotCommon/iotSemaphore.o \
	../../IotCommon/newDb.o \
	../../IotCommon/systemtable.o \
	../../IotCommon/iotError.o \
	../../IotCommon/parsingBig.o \
	../../IotCommon/json.o \
	../../IotCommon/socket.o \
	../../IotCommon/dump.o \
	../../IotCommon/newLog.o

LIBNCI = libnci/lib/libnci.a

# ------------------------------------------------------------------
	
	
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@

%.o: %.cpp $(HEADERS)
	$(CXX) $(CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@

all: clean build

build: $(LIBNCI) $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(TARGET) $(LDLIBS)
	cp $(TARGET) ../../swupdate/images/usr/bin/

$(LIBNCI):
	cd libnci; make

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
	-rm -f ../../swupdate/images/usr/bin/$(TARGET)
	cd libnci; make clean

