# ------------------------------------------------------------------
# FTDI CBUS Control makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2015. All rights reserved
# ------------------------------------------------------------------

ifeq ($(TARGET_OS),RASPBIAN)
LDFLAGS += -lusb-1.0
LDLIBS = ../libftdi1-1.2/build/src/libftdi1.a
INCLUDES = -I../libftdi1-1.2/src
else
# LDFLAGS += -lusb-1.0 -lftdi
LDLIBS = -lftdi -lusb -lusb-1.0 -lc
INCLUDES = -I$(STAGING_DIR)/usr/include
endif

TARGET = iot_jf

OBJECTS = jf_main.o


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

