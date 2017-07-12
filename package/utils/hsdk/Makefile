#
# Copyright (C) 2015 NXP semiconductors
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=nxp_hsdk
PKG_VERSION:=5.15
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

define Package/hsdk
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=hsdk
  DEPENDS:=+libudev +libpcap
endef

define Package/hsdk/description
	HSDK is a thread software for NXP's KW41Z.
endef

define Build/Prepare
	@echo Preparing $(PKG_NAME)
	$(CP) -R src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	@echo Compiling $(PKG_NAME) $(TARGET_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) \
	CC="$(TARGET_CC) --sysroot=$(STAGING_DIR)"
endef

define Package/hsdk/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/build/* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/demo/bin/* $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/demo/make_tun.sh $(1)/usr/bin/
	chmod +x $(1)/usr/bin/make_tun.sh
	$(INSTALL_DIR) $(1)/usr/share/hsdk
	cp $(PKG_BUILD_DIR)/res/hsdk.conf $(1)/usr/share/hsdk/
endef

$(eval $(call BuildPackage,hsdk))
