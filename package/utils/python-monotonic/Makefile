#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-monotonic
PKG_VERSION:=1.2
PKG_RELEASE:=1
PKG_LICENSE:=MIT

PKG_SOURCE:=monotonic-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/monotonic-$(PKG_VERSION)/
PKG_SOURCE_URL:=https://pypi.python.org/packages/08/35/9e06c881c41962d7367e9466724beda2b1101439b149b7ff708d708890de/
PKG_MD5SUM:=d14c93aabc3d6af25ef086b032b123cf
PKG_BUILD_DEPENDS:=python python-setuptools

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-monotonic
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  MAINTAINER:=Feng Guo <feng.guo@nxp.com>
  TITLE:=An implementation of time.monotonic()
  URL:=https://github.com/atdt/monotonic
  DEPENDS:=+python
endef

define Package/python-monotonic/description
  This module provides a monotonic() function which returns the value (in fractional seconds) of a clock which never goes backwards.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-monotonic/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
	    $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
	    $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-monotonic))
