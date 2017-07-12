PROJROOT=$(shell pwd)
HSDK_ROOT=$(PROJROOT)/..
BUILDDIR=$(PROJROOT)/build
BINDIR=$(PROJROOT)/bin
BUILDLIB=$(HSDK_ROOT)/build

BT_INC=-I$(HSDK_ROOT)/include/physical/BT
HCI_INC=-I$(HSDK_ROOT)/include/protocol/HCI
PROTO_INC=-I$(HSDK_ROOT)/include/protocol
SER_INC=-I$(HSDK_ROOT)/include/physical
SYS_INC=-I$(HSDK_ROOT)/include/sys
UART_INC=-I$(HSDK_ROOT)/include/physical/UART
PCAP_INC=-I$(HSDK_ROOT)/include/physical/PCAP
SPI_INC=-I$(HSDK_ROOT)/include/physical/SPI
FSCI_INC=-I$(HSDK_ROOT)/include/protocol/FSCI

CFLAGS=-O3 -Wall -Wno-unused-function
BUILDFLAGS=-c $(BT_INC) $(HCI_INC) $(PROTO_INC) $(SER_INC) $(SYS_INC) $(UART_INC) $(PCAP_INC) $(SPI_INC) $(FSCI_INC)
HSDK_LIBS=-L$(BUILDLIB) -lframer -lfsci -lphysical -luart -lspi $(LRNDIS) -lsys

UNAME := Linux

ifeq ($(UNAME), Linux)
	LDFLAGS=-lpthread -lrt
	LRNDIS=-lrndis -lpcap
endif
ifeq ($(UNAME), Darwin)
	HSDK_LIBS=-lframer -lfsci -lphysical -luart -lsys
	LDFLAGS=-lpthread
endif

ifeq ($(OPENWRT), yes)
	CC=mips-openwrt-linux-uclibc-gcc
	CFLAGS+=-DOPENWRT -Os -s
endif
ifeq ($(ARMHF), yes)
	CC=arm-linux-gnueabihf-gcc
	LRNDIS=
	HSDK_LIBS+=-L../res/ -ludev-armhf
endif


build: clean pre-build FsciBootloader GetKinetisDevices Thread_KW_Tun PCAPTest

spi: SPITest

pre-build:
	mkdir -p $(BUILDDIR)
	mkdir -p $(BINDIR)

FsciBootloader: FsciBootloader.o
	$(CC) $(BUILDDIR)/$^ -o $(BINDIR)/$@ $(HSDK_LIBS) $(LDFLAGS)
FsciBootloader.o: FsciBootloader.c
	$(CC) $(CFLAGS) $(BUILDFLAGS) $^ -o $(BUILDDIR)/$@

GetKinetisDevices: GetKinetisDevices.o
	$(CC) $(BUILDDIR)/$^ -o $(BINDIR)/$@ $(HSDK_LIBS) $(LDFLAGS) -ludev
GetKinetisDevices.o: GetKinetisDevices.c
	$(CC) $(CFLAGS) $(BUILDFLAGS) $^ -o $(BUILDDIR)/$@

Thread_KW_Tun: Thread_KW_Tun.o
	$(CC) $(BUILDDIR)/$^ -o $(BINDIR)/$@ $(HSDK_LIBS) $(LDFLAGS)
Thread_KW_Tun.o: Thread_KW_Tun.c
	$(CC) $(CFLAGS) $(BUILDFLAGS) $^ -o $(BUILDDIR)/$@

PCAPTest: PCAPTest.o
	$(CC) $(BUILDDIR)/$^ -o $(BINDIR)/$@ $(HSDK_LIBS) $(LDFLAGS)
PCAPTest.o: PCAPTest.c
	$(CC) $(CFLAGS) $(BUILDFLAGS) $^ -o $(BUILDDIR)/$@

SPITest: SPITest.o
	$(CC) $(BUILDDIR)/$^ -o $(BINDIR)/$@ -lphysical -lframer -lfsci -lsys -lspi
SPITest.o: SPITest.c
	$(CC) $(CFLAGS) $(BUILDFLAGS) $^ -o $(BUILDDIR)/$@

clean:
	rm -f $(BUILDDIR)/*
	rm -f $(BINDIR)/*
