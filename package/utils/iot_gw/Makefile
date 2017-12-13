#
# Copyright (C) 2015 NXP semiconductors
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nxp-ZigbeeDemo
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/iiot-gateway/nxp-ZigbeeDemo.git
PKG_SOURCE_VERSION:=71d5f4ff73f38180b7aecd6726c26129d27b79d7

include $(INCLUDE_DIR)/package.mk

define Package/iot_gw
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Internet of Thing - Gateway software
  DEPENDS:=+libftdi +libpthread +librt +libstdcpp +libusb-1.0 +libusb-compat
endef

define Package/iot_gw/description
	IoT_GW is a wrapper around the Host software for NXP's Internet Of Things Gateway.
endef

define Package/iot_gw/config
  choice
    depends on PACKAGE_iot_gw
    prompt "Select Zigbee protocol version for Control Bridge"
    default IOT_GW_ZB_3_0
    config IOT_GW_ZB_3_0
      bool "Zigbee 3.0"
    config IOT_GW_ZB_2_0
      bool "Zigbee 2.0"
  endchoice
endef


ifeq ("$(CONFIG_IOT_GW_ZB_3_0)","y")
	ZCB_VERSION=3v0
endif

ifeq ("$(CONFIG_IOT_GW_ZB_2_0)","y")
	ZCB_VERSION=2v0
endif


IOT_GW_CROSS_CONFIG= \
	TARGET_MACHINE=RASPBERRYPI \
	TARGET_OS=OPENWRT \
	CFLAGS="$(TARGET_CFLAGS) -DTARGET_RASPBERRYPI -DTARGET_OPENWRT" \
	CC="$(TARGET_CC_NOCACHE)" \
	CXX="$(TARGET_CXX_NOCACHE)" \
	AR="$(TARGET_AR)" \
	LD="$(TARGET_CROSS)ld" \
	ZCB_VERSION="_$(ZCB_VERSION)"

define Build/Configure
	(cd $(PKG_BUILD_DIR)/daemons/nfcRPiAlt/libnci/linux_libnfc-nci/;\
	chmod +x bootstrap;\
	./bootstrap;\
	./configure --enable-pn7150 --host $(CONFIG_ARCH)-openwrt-linux \
	);
endef

define Build/Compile
	@echo Compiling $(PKG_NAME)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(IOT_GW_CROSS_CONFIG) \
		clean build

	$(MAKE) -C $(PKG_BUILD_DIR)/daemons/nfcRPiAlt/libnci/linux_libnfc-nci \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall"
endef

define Package/iot_gw/install
	$(CP) $(PKG_BUILD_DIR)/swupdate/images/* $(1)/
	chmod o+w $(1)/tmp
	chmod +x $(1)/etc/init.d/iot_*
	chmod +x $(1)/usr/bin/iot_*
	chmod +x $(1)/usr/bin/killbyname
	chmod +x $(1)/www/cgi-bin/iot_*

	mkdir -p $(1)/data
	mkdir -p $(1)/data/nfc
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/daemons/nfcRPiAlt/libnci/linux_libnfc-nci/.libs/libnfc_nci_linux* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/data/nfc
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/daemons/nfcRPiAlt/libnci/linux_libnfc-nci/conf/PN7150/libnfc-nxp.conf \
		$(1)/data/nfc/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/daemons/nfcRPiAlt/libnci/linux_libnfc-nci/.libs/nfcDemoApp* $(1)/data/nfc/
endef



$(eval $(call BuildPackage,iot_gw))
