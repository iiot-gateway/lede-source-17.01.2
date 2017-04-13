# ------------------------------------------------------------------
# UART out Test makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

TARGET = iot_ot
OTHERS = sensor.json ui.json topo.json lamp.json \
	topo_clear.json topo_addman.json topo_addui.json topo_addsensor.json topo_end.json

INCLUDES = -I../../IotCommon
OBJECTS = ot_main.o \
	../../IotCommon/iotError.o \
	../../IotCommon/fileCreate.o \
	../../IotCommon/queue.o \
	../../IotCommon/dump.o \
	../../IotCommon/iotSemaphore.o \
	../../IotCommon/newLog.o


%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@

all: clean build

build: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) -lc
	mkdir -p ../../swupdate/images/usr/bin/
	mkdir -p ../../swupdate/images/usr/share/iot/
	cp -f $(TARGET) ../../swupdate/images/usr/bin/
	cp -f $(OTHERS) ../../swupdate/images/usr/share/iot/

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)

