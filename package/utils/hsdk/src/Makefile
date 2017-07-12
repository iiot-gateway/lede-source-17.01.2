PROJROOT=$(shell pwd)
BUILDDIR=$(PROJROOT)/build/

SYS_INC=-Iinclude/sys
SER_INC=-Iinclude/physical
PROTO_INC=-Iinclude/protocol

UART_INC=-Iinclude/physical/UART
SPI_INC=-Iinclude/physical/SPI
PCAP_INC=-Iinclude/physical/PCAP
BT_INC=-Iinclude/physical/BT

FSCI_INC=-Iinclude/protocol/FSCI
HCI_INC=-Iinclude/protocol/HCI
ASCII_INC=-Iinclude/protocol/ASCII

CFLAGS=-O3 -Wall -Wno-unused-function -D$(USE_UDEV) -D$(USE_PCAP) -D$(USE_SPI)

UNAME := Linux

ifeq ($(UNAME), Linux)
	LUDEV=-ludev
	LDFLAGS=-lpthread -lrt $(LUDEV)
	USE_UDEV=__linux__udev__
	USE_PCAP=__linux__pcap__
	USE_SPI=__linux__spi__
	LPCAP=-lpcap
	LIBRNDIS=$(addsuffix $(EXTENSION), librndis)
	LRNDIS=-lrndis
	LSPI=-lspi
	FRAMEWORKS=
endif
ifeq ($(UNAME), Darwin)
	LDFLAGS=-lpthread
	LUDEV=
	FRAMEWORKS=-framework IOKit -framework CoreFoundation
endif

ifeq ($(OPENWRT), yes)
	CC=mips-openwrt-linux-uclibc-gcc
	CFLAGS+=-Os -s
endif

ifeq ($(ARMHF), yes)
	CC=arm-linux-gnueabihf-gcc
	LUDEV=-Lres/ -ludev-armhf
	USE_PCAP=PHONY
	LIBRNDIS=
	LRNDIS=
endif

BUILDFLAGS=-c $(SYS_INC) $(SER_INC) $(PROTO_INC) $(UART_INC) $(SPI_INC) $(PCAP_INC) $(FSCI_INC) $(HCI_INC) $(BT_INC) $(ASCII_INC)
LIB_OPTION=dynamic

ifeq ($(LIB_OPTION), static)
	LIB_INCLUDE=
	LIBCFLAGS=
	LIBLFLAGS=-rcs
	EXTENSION=.a
	LL=ar
else
	LIB_INCLUDE=-L$(BUILDDIR)
	LIBCFLAGS=-fPIC
	ifeq ($(UNAME), Darwin)
		LIBLFLAGS=-dynamiclib -o
		EXTENSION=.dylib
	else
		LIBLFLAGS=-shared -Wl,-soname,
		EXTENSION=.so
		VERSION=.1
		FULL_VERSION=.1.3.1
	endif
	LL=$(CC)
endif

all: clean pre-build build build-demo cleanObj install

documentation:
	rm -rf Documentation/html Documentation/latex
	doxygen Documentation/Doxyfile

build-demo:
	@$(MAKE) -C $(PROJROOT)/demo

build: pre-build $(addsuffix $(EXTENSION), libsys) $(addsuffix $(EXTENSION), libuart) $(addsuffix $(EXTENSION), libspi) $(LIBRNDIS) $(addsuffix $(EXTENSION), libfsci) $(addsuffix $(EXTENSION), libphysical) $(addsuffix $(EXTENSION), libframer)

build-extra: $(addsuffix $(EXTENSION), libascii) $(addsuffix $(EXTENSION), libhci)

pre-build:
	mkdir -p $(BUILDDIR)


$(addsuffix $(EXTENSION), libsys): utils.o RawFrame.o MessageQueue.o hsdkThread.o hsdkEvent.o hsdkFile.o hsdkLock.o hsdkSemaphore.o EventManager.o hsdkLogger.o
ifeq ($(LIB_OPTION), dynamic)
	$(LL) $(LIBLFLAGS)$@$(VERSION) -o $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^) -lpthread
else
	$(LL) $(LIBLFLAGS) $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^)
endif

utils.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) sys/utils.c -o $(BUILDDIR)$@

hsdkLogger.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) sys/hsdkLogger.c -o $(BUILDDIR)$@

RawFrame.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) sys/RawFrame.c -o $(BUILDDIR)$@

MessageQueue.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) sys/MessageQueue.c -o $(BUILDDIR)$@

hsdkThread.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) sys/hsdkThread.c -o $(BUILDDIR)$@

hsdkEvent.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) sys/hsdkEvent.c -o $(BUILDDIR)$@

hsdkFile.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) sys/hsdkFile.c -o $(BUILDDIR)$@

hsdkLock.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) sys/hsdkLock.c -o $(BUILDDIR)$@

hsdkSemaphore.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) sys/hsdkSemaphore.c -o $(BUILDDIR)$@

EventManager.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) sys/EventManager.c -o $(BUILDDIR)$@



$(addsuffix $(EXTENSION), libframer): Framer.o FSCIFramer.o
ifeq ($(LIB_OPTION), dynamic)
	$(LL) $(LIB_INCLUDE) $(LIBLFLAGS)$@$(VERSION) -o $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $<) -lsys -lfsci -lphysical
else
	$(LL) $(LIBLFLAGS) $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^)
endif

Framer.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) protocol/Framer.c -o $(BUILDDIR)$@



$(addsuffix $(EXTENSION), libphysical): PhysicalDevice.o
ifeq ($(LIB_OPTION), dynamic)
	$(LL) $(LIB_INCLUDE) $(LIBLFLAGS)$@$(VERSION) -o $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^) -lsys -luart $(LRNDIS) $(LUDEV) $(LSPI)
else
	$(LL) $(LIBLFLAGS) $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^)
endif

PhysicalDevice.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) physical/PhysicalDevice.c -o $(BUILDDIR)$@



$(addsuffix $(EXTENSION), libfsci): FSCIFrame.o FSCIFramer.o
ifeq ($(LIB_OPTION), dynamic)
	$(LL) $(LIB_INCLUDE) $(LIBLFLAGS)$@$(VERSION) -o $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^) -lsys
else
	$(LL) $(LIBLFLAGS) $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^)
endif

FSCIFrame.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) protocol/FSCI/FSCIFrame.c -o $(BUILDDIR)$@

FSCIFramer.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) protocol/FSCI/FSCIFramer.c -o $(BUILDDIR)$@

Commands.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) protocol/FSCI/Commands.c -o $(BUILDDIR)$@



$(addsuffix $(EXTENSION), libhci): HCIFrame.o HCIFramer.o
ifeq ($(LIB_OPTION), dynamic)
	$(LL) $(LIB_INCLUDE) $(LIBLFLAGS)$@$(VERSION) -o $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^) -lsys
else
	$(LL) $(LIBLFLAGS) $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^)
endif

HCIFrame.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) protocol/HCI/HCIFrame.c -o $(BUILDDIR)$@

HCIFramer.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) protocol/HCI/HCIFramer.c -o $(BUILDDIR)$@



$(addsuffix $(EXTENSION), libascii): ASCIIFrame.o ASCIIFramer.o
ifeq ($(LIB_OPTION), dynamic)
	$(LL) $(LIB_INCLUDE) $(LIBLFLAGS)$@$(VERSION) -o $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^) -lsys
else
	$(LL) $(LIBLFLAGS) $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^)
endif

ASCIIFrame.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) protocol/ASCII/ASCIIFrame.c -o $(BUILDDIR)$@

ASCIIFramer.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) protocol/ASCII/ASCIIFramer.c -o $(BUILDDIR)$@



$(addsuffix $(EXTENSION), libuart): UARTDiscovery.o UARTDevice.o UARTConfiguration.o
ifeq ($(LIB_OPTION), dynamic)
	$(LL) $(FRAMEWORKS) $(LIB_INCLUDE) $(LIBLFLAGS)$@$(VERSION) -o $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^) -lsys $(LUDEV)
else
	$(LL) $(LIBLFLAGS) $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^)
endif

UARTDiscovery.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) physical/UART/UARTDiscovery.c -o $(BUILDDIR)$@

UARTDevice.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) physical/UART/UARTDevice.c -o $(BUILDDIR)$@

UARTConfiguration.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) physical/UART/UARTConfiguration.c -o $(BUILDDIR)$@


$(addsuffix $(EXTENSION), libspi): SPIDevice.o SPIConfiguration.o
ifeq ($(LIB_OPTION), dynamic)
	$(LL) $(LIB_INCLUDE) $(LIBLFLAGS)$@$(VERSION) -o $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^) -lsys
else
	$(LL) $(LIBLFLAGS) $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^)
endif

SPIDevice.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) physical/SPI/SPIDevice.c -o $(BUILDDIR)$@

SPIConfiguration.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) physical/SPI/SPIConfiguration.c -o $(BUILDDIR)$@


$(addsuffix $(EXTENSION), librndis): PCAPDevice.o
ifeq ($(LIB_OPTION), dynamic)
	$(LL) $(LIB_INCLUDE) $(LIBLFLAGS)$@$(VERSION) -o $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^) -lsys -lpthread $(LPCAP)
else
	$(LL) $(LIBLFLAGS) $(BUILDDIR)$@ $(addprefix $(BUILDDIR), $^)
endif

PCAPDevice.o:
	$(CC) $(LIBCFLAGS) $(CFLAGS) $(BUILDFLAGS) physical/PCAP/PCAPDevice.c -o $(BUILDDIR)$@

clean:
	$(MAKE) -C $(PROJROOT)/demo clean
	rm -f $(BUILDDIR)*
	rm -rf $(BUILDDIR)

cleanObj:
	rm -f $(BUILDDIR)*.o

install:
	# Copy shared libraries to /usr/lib with fully-qualified major.minor names. Run ldconfig to generate symlinks to *.so.x.
	# Manually set symlinks .so -> .so.x
	for f in $(shell find $(BUILDDIR) -name '*$(EXTENSION)' -exec basename {} \;); do \
	    mv $(BUILDDIR)$$f $(BUILDDIR)$$f$(VERSION); \
	    ln -s $(BUILDDIR)$$f$(VERSION) $(BUILDDIR)$$f; \
	done
