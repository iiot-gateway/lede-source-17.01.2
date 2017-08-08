#
# Copyright (C) 2015 NXP semiconductors
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nxp_avs_alexa
PKG_VERSION:=master
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

define Package/avs_alexa
  SECTION:=utils
  CATEGORY:=Utilities
  MAINTAINER:=xu chen <xu.chen_3@nxp.com>
  TITLE:=client for Amazon's Alexa Voice Service 
  DEPENDS:=+mosquitto-nossl +python +python-webrtcvad +python-requests +python-pyaudio \
	   +python-pyalsaaudio +python-pocketsphinx  +python-monotonic +python-CherryPy \
	   +python-paho-mqtt +python-awsiot +python-six +python-setuptools 
endef

define Package/avs_alexa/description
	This is a client for Amazon's Alexa service. 
endef

avs_alexa_CROSS_CONFIG= \
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
	$(CP) -R src/Alexa  $(PKG_BUILD_DIR)
endef

define Build/Configure

endef

define Build/Compile
	@echo Compiling $(PKG_NAME)

endef

define Package/avs_alexa/install

	$(INSTALL_DIR) $(1)/root/
	$(CP) $(PKG_BUILD_DIR)/Alexa $(1)/root/

endef

$(eval $(call BuildPackage,avs_alexa))
