#
# Copyright (C) 2015 NXP semiconductors
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nxp_iot_gw
PKG_VERSION:=5.15
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
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

define Build/Prepare
	@echo Preparing $(PKG_NAME)
	$(CP) -R src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	@echo Compiling $(PKG_NAME)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(IOT_GW_CROSS_CONFIG) \
		clean build

endef

define Package/iot_gw/install
	$(CP) $(PKG_BUILD_DIR)/swupdate/images/* $(1)/
	rm $(1)/etc/init.d/iot_nd_initd
	chmod o+w $(1)/tmp
	chmod +x $(1)/etc/init.d/iot_*
	chmod +x $(1)/usr/bin/iot_*
	chmod +x $(1)/usr/bin/killbyname
	chmod +x $(1)/www/cgi-bin/iot_*
endef



$(eval $(call BuildPackage,iot_gw))
