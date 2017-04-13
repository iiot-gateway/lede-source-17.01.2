#
# Copyright (C) 2015 Rafał Miłecki <zajec5@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnfc-nci
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

define Package/libnfc-nci
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Linux demo Application for NFC PN71xx module
  DEPENDS:=+libstdcpp
endef

define Package/libnfc-nci/description
 This package contains an application and some libraries for NFC module
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* -rf $(PKG_BUILD_DIR)/
endef

define Build/Configure
	(cd $(PKG_BUILD_DIR);\
	./bootstrap;\
	./configure --enable-pn7150 --host aarch64-openwrt-linux \
	);
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall"
endef

define Package/libnfc-nci/install
	mkdir -p $(1)/data
	mkdir -p $(1)/data/nfc
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/.libs/libnfc_nci_linux* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/data/nfc
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/conf/PN7150/libnfc-nxp.conf \
		$(1)/data/nfc/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/.libs/nfcDemoApp* $(1)/data/nfc/
endef

$(eval $(call BuildPackage,libnfc-nci))
