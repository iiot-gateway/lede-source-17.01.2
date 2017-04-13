# ------------------------------------------------------------------
# JennicModuleProgrammer program makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2015. All rights reserved
# ------------------------------------------------------------------

TARGET = iot_jd

EXTRA_CFLAGS = -DDBG_ENABLE -DVERSION="RHM1"

# Define to trace bootloader comms
# CFLAGS += -DDEBUG_BOOTLOADER 

# Define to trace firmware image info
# CFLAGS += -DDEBUG_FIRMWARE

# Define to trace serial comms
# CFLAGS += -DDEBUG_UART
 
# Define to trace programmer
# CFLAGS += -DDEBUG_PROGRAMMER

OBJECTS = ../jennicCommon/uart.o \
	../jennicCommon/programmer.o \
	../jennicCommon/JN51xx_BootLoader.o \
	../jennicCommon/Firmware.o \
	jd_main.o

INCLUDES = -I../jennicCommon
EXTRA_CFLAGS += -DPOSIX -DVERSION_MAJOR="\"0\"" -DVERSION_MINOR="\"1\"" -DVERSION_SVN="\"9961\""

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(INCLUDES) -Wall -g -c $< -o $@

all: clean build

build: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET) -lc
	cp $(TARGET) ../../swupdate/images/usr/bin/

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
	-rm -f ../../swupdate/images/usr/bin/$(TARGET)

