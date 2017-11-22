#
# Copyright (C) 2015 NXP semiconductors
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nxp-sigfox
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

define Package/nxp-sigfox
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=sigfox
endef

define Package/nxp-sigfox/description
	nxp-sigfox is a sigfox demo software for NXP's OL2385 sigfox device.
endef

define Build/Prepare
	@echo Preparing $(PKG_NAME)
	$(CP) -R src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	@echo Compiling $(PKG_NAME) $(TARGET_DIR)
	$(TARGET_CC) -o $(PKG_BUILD_DIR)/sigfox-demo $(PKG_BUILD_DIR)/sigfox-demo.c
endef

define Package/nxp-sigfox/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc
	$(CP) $(PKG_BUILD_DIR)/sigfox-demo $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/sigfox.conf $(1)/etc/
endef

$(eval $(call BuildPackage,nxp-sigfox))
