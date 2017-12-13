#
# Copyright (C) 2015 NXP semiconductors
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=docker
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/iiot-gateway/docker-glibc.git
PKG_SOURCE_VERSION:=589d424a2b0dc679b3171d30f25381ec8f2ea3ba

include $(INCLUDE_DIR)/package.mk

define Package/docker
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=docker
endef

define Package/docker/description
	docker is a glibc application to run docker.
endef

define Build/Compile
endef

define Package/docker/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/bin/* $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/lib/* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,docker))
