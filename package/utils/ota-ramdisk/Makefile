#
# Copyright (C) 2015 NXP semiconductors
#
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

PKG_NAME:=ota-ramdisk
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/keinYang/ramdisk-lede.git
PKG_SOURCE_VERSION:=4c614facdd39ae6dd69b1c6931c2878bb1912502

include $(INCLUDE_DIR)/package.mk

define Package/ota-ramdisk
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=ota-ramdisk
endef

define Package/ota-ramdisk/description
	ota-ramdisk is a ramdisk to rollback rootfs for OTA.
endef

define Build/Compile
endef

define Package/ota-ramdisk/install
	$(CP) $(PKG_BUILD_DIR)/root.ext4.gz.uboot $(KDIR)/
endef

$(eval $(call BuildPackage,ota-ramdisk))
