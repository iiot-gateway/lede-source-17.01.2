#
# Copyright (C) 2015 NXP semiconductors
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hsdk
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/iiot-gateway/thread-kw41z.git
PKG_SOURCE_VERSION:=d6c9842fbc5d587fa4aec2f5b5edc6e14f8b7aff

include $(INCLUDE_DIR)/package.mk

define Package/hsdk
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=hsdk(thread-kw41z)
  DEPENDS:=+libudev +libpcap
endef

define Package/hsdk/description
	hsdk(thread-kw41z) is a thread software for NXP's KW41Z.
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
