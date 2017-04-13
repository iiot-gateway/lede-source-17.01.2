# ------------------------------------------------------------------
# Test makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2015. All rights reserved
# ------------------------------------------------------------------

AUTOPROGSCRIPT = JennicModuleAutoProgram.sh
PROGSCRIPT = JennicModuleProgram.sh
DETECTSCRIPT = JennicModuleDetect.sh

ifeq ($(TARGET_OS),RASPBIAN)
AUTOPROGSCRIPT_SRC = JennicModuleAutoProgram.sh.rpi
PROGSCRIPT_SRC = JennicModuleProgram.sh.rpi
DETECTSCRIPT_SRC = JennicModuleDetect.sh.rpi
else
AUTOPROGSCRIPT_SRC = JennicModuleAutoProgram.sh.orpi
PROGSCRIPT_SRC = JennicModuleProgram.sh.orpi
DETECTSCRIPT_SRC = JennicModuleDetect.sh.orpi
endif

all: clean build

build:
	install -D $(AUTOPROGSCRIPT_SRC) ../../swupdate/images/usr/bin/$(AUTOPROGSCRIPT)
	install -D $(PROGSCRIPT_SRC) ../../swupdate/images/usr/bin/$(PROGSCRIPT)
	install -D $(DETECTSCRIPT_SRC) ../../swupdate/images/usr/bin/$(DETECTSCRIPT)

clean:
