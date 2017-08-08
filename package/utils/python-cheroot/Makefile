#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-cheroot
PKG_VERSION:=5.7.0
PKG_RELEASE:=1
PKG_LICENSE:=MIT

PKG_SOURCE:=cheroot-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/cheroot-$(PKG_VERSION)/
PKG_SOURCE_URL:=https://pypi.python.org/packages/ba/27/7031f11bfa2650db4e7c0367e82688bbba6b5759d3abebb568aea402402d/
PKG_MD5SUM:=9d7fd13b673cde93c26a04b4aa5fa9a1
PKG_BUILD_DEPENDS:=python python-setuptools

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-cheroot
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  MAINTAINER:=
  TITLE:=An implementation of time.cheroot()
  URL:=https://github.com/atdt/cheroot
  DEPENDS:=+python 
endef

define Package/python-cheroot/description
  This module provides a cheroot() function which returns the value (in fractional seconds) of a clock which never goes backwards.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-cheroot/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
	    $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
	    $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-cheroot))
