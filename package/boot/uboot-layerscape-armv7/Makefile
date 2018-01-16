#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

PKG_NAME:=uboot-layerscape-armv7
PKG_VERSION:=v1.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_MIRROR_HASH:=6aba7cb32d655dacb49bd188706bf1284159ae9b1fa69056df634b90070c3768
PKG_SOURCE_URL:=https://github.com/openil/u-boot.git
PKG_SOURCE_VERSION:=4ebc5c0fea263da867e41abb3f410b7cc4392def

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0 GPL-2.0+
PKG_LICENSE_FILES:=Licenses/README

include $(INCLUDE_DIR)/package.mk

define uboot/Default
	BUILD_TARGET:=layerscape
	BUILD_SUBTARGET:=armv7
	BUILD_DEVICES:=$(1)
    	UBOOT_IMAGE:=u-boot-dtb.bin
	HIDDEN:=y
endef


define uboot/ls1021aiot
	TITLE:=U-Boot $(PKG_NAME)-$(PKG_VERSION) for NXP ls1021aiot Dev Board
	CONFIG=ls1021aiot_sdcard
	IMAGE=u-boot-with-spl-pbl.bin
endef

UBOOTS := \
	ls1021aiot

define Package/uboot/template
define Package/uboot-layerscape-armv7-$(1)
	SECTION:=boot
	CATEGORY:=Boot Loaders
	DEPENDS:= @TARGET_layerscape_armv7
	TITLE:=$(2)
	URL:=http://git.freescale.com/git/cgit.cgi/ppc/sdk/u-boot.git/snapshot
	VARIANT:=$(1)
	MAINTAINER:=Jiang Yutang <jiangyutang1978@gmail.com>
endef
endef

define BuildUBootPackage
	$(eval $(uboot/Default))
	$(eval $(uboot/$(1)))
	$(call Package/uboot/template,$(1),$(TITLE))
endef

ifdef BUILD_VARIANT
$(eval $(call uboot/$(BUILD_VARIANT)))
UBOOT_CONFIG:=$(if $(CONFIG),$(CONFIG),$(BUILD_VARIANT))
UBOOT_IMAGE:=$(if $(IMAGE),$(IMAGE),u-boot-dtb.bin)
endif

define Build/Configure
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(UBOOT_CONFIG)_config
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CROSS_COMPILE=$(TARGET_CROSS) DTC=$(LINUX_DIR)/scripts/dtc/dtc
endef

define Package/uboot/install/default
	$(CP) $(PKG_BUILD_DIR)/$(UBOOT_IMAGE) $(KDIR)/$(1)-armv7-uboot.bin
	$(PKG_BUILD_DIR)/tools/mkenvimage -s 131072 -o $(KDIR)/$(1)-uboot-env.bin uboot-env/$(BUILD_VARIANT)-uboot-env.txt
endef

define Package/uboot/install/template
define Package/uboot-layerscape-armv7-$(1)/install
	$(call Package/uboot/install/default,$(2))
endef
endef

$(foreach u,$(UBOOTS), \
	$(eval $(call Package/uboot/install/template,$(u),$(u))) \
)

$(foreach u,$(UBOOTS), \
	$(eval $(call BuildUBootPackage,$(u))) \
	$(eval $(call BuildPackage,uboot-layerscape-armv7-$(u))) \
)
