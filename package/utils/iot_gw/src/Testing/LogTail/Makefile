# ------------------------------------------------------------------
# UART out Test makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

TARGET = iot_lt

INCLUDES = -I../../IotCommon
OBJECTS = lt_main.o \
	../../IotCommon/atoi.o \
	../../IotCommon/iotSemaphore.o \
	../../IotCommon/newLog.o


%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@

all: clean build

build: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) -lc
	mkdir -p ../../swupdate/images/usr/bin/
	cp -f $(TARGET) ../../swupdate/images/usr/bin/

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)

