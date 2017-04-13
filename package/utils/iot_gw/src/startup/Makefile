# ------------------------------------------------------------------
# Startup script makefile
# ------------------------------------------------------------------
# Author:    nlv10677
# Copyright: NXP B.V. 2014. All rights reserved
# ------------------------------------------------------------------

# Diversity scripts
ifeq ($(TARGET_MACHINE),RASPBERRYPI)
ifeq ($(TARGET_OS),RASPBIAN)
INIT_SCRIPT         = iot_init.sh.rpi
UPDATE_SCRIPT       = iot_su.sh.rpi
UPDATE_INITD_SCRIPT = iot_su_initd.rpi
else
INIT_SCRIPT         = iot_init.sh.orpi
UPDATE_SCRIPT       = iot_su.sh.openwrt
UPDATE_INITD_SCRIPT = iot_su_initd.openwrt
endif
else
INIT_SCRIPT         = iot_init.sh.openwrt
UPDATE_SCRIPT       = iot_su.sh.openwrt
UPDATE_INITD_SCRIPT = iot_su_initd.openwrt
endif

# Other scripts
OTHER_SCRIPTS = iot_start.sh iot_restart.sh \
	iot_stop.sh iot_kill.sh killbyname iot_wget.sh

# init.d scripts
INITD_SCRIPTS  = \
	iot_sj_initd \
	iot_ci_initd iot_nd_initd \
	iot_pt_initd iot_su_initd \
	iot_gd_initd iot_zb_initd \
	iot_dbp_initd
	
all: clean build

build:
	@echo "TARGET_MACHINE = $(TARGET_MACHINE)"
	install -D $(INIT_SCRIPT)          ../swupdate/images/usr/bin/iot_init.sh
	install -D $(UPDATE_SCRIPT)        ../swupdate/images/usr/bin/iot_su.sh
	install -D $(OTHER_SCRIPTS)        ../swupdate/images/usr/bin/
	install -m644 -D ../swupdate/iot_version.txt        ../swupdate/images/usr/share/iot/iot_version.txt
	mkdir -p ../swupdate/images/etc/init.d/
	install -t ../swupdate/images/etc/init.d/ $(INITD_SCRIPTS)
	install -D $(UPDATE_INITD_SCRIPT)  ../swupdate/images/etc/init.d/iot_su_initd
ifeq ($(TARGET_MACHINE),LINUX_PC)
	install -D test_start.sh test_kill.sh ../swupdate/images/usr/bin/
endif

clean:

