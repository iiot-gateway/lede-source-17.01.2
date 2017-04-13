# ------------------------------------------------------------------
# Top-level IoT Makefile
# Build (or clean) all IoT software in one go
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

all:
	cd ZCB;                     make
	cd SecureJoiner;            make
	cd ControlInterface;        make
	cd nfcRPiAlt;               make
	cd GwDiscovery;             make
	cd dbp;                     make

build:
	cd ZCB;                     make build
	cd SecureJoiner;            make build
	cd ControlInterface;        make build
	cd nfcRPiAlt;               make build
	cd GwDiscovery;             make build
	cd dbp;                     make build

clean:
	cd ZCB;                     make clean
	cd SecureJoiner;            make clean
	cd ControlInterface;        make clean
	cd nfcRPiAlt;               make clean
	cd GwDiscovery;             make clean
	cd dbp;                     make clean


