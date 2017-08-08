#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-portend
PKG_VERSION:=2.1.2
PKG_RELEASE:=1
PKG_LICENSE:=MIT

PKG_SOURCE:=portend-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/portend-$(PKG_VERSION)/
PKG_SOURCE_URL:=https://pypi.python.org/packages/0a/be/db3acda03e226ae9d53a7221ffa110eac1e5ca8024dda3a2683960d9aefd/
PKG_MD5SUM:=e0d1469994a1b12fdb4104e14996ccd6
PKG_BUILD_DEPENDS:=python python-setuptools

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-portend
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  MAINTAINER:=xu chen <xu.chen_3@nxp.com>
  TITLE:= TCP port monitoring utilities
  URL:=https://github.com/jaraco/portend
  DEPENDS:=+python
endef

define Package/python-portend/description
  TCP port monitoring utilities
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-portend/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
	    $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
	    $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-portend))
