#
# Copyright (C) 2015 Rafał Miłecki <zajec5@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libnfc-nci
PKG_RELEASE:=1
PKG_VERSION:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/NXPNFCLinux/linux_libnfc-nci.git
PKG_SOURCE_VERSION:=44873c5c75150afecc7122f60ffebebbcb59a745

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
include $(INCLUDE_DIR)/package.mk

define Package/libnfc-nci
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Linux demo Application for NFC PN71xx module
  URL:=https://github.com/NXPNFCLinux/linux_libnfc-nci.git
  DEPENDS:=+libstdcpp
endef

define Package/libnfc-nci/description
 This package contains an application and some libraries for NFC module
endef

define Build/Configure
	(cd $(PKG_BUILD_DIR);\
	./bootstrap;\
	./configure --enable-pn7150 --host $(CONFIG_ARCH)-openwrt-linux \
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
