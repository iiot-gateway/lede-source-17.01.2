#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-CherryPy
PKG_VERSION:=11.0.0
PKG_RELEASE:=1
PKG_LICENSE:=MIT

PKG_SOURCE:=CherryPy-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/CherryPy-$(PKG_VERSION)/
PKG_SOURCE_URL:=https://pypi.python.org/packages/bf/2b/febc9a1f09bf9249b0ce1723d06dbedd9ff34879b76d17180611c48a7f63/
PKG_MD5SUM:=3caf903447ed39057466256fa9c69554
PKG_BUILD_DEPENDS:=python python-setuptools

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-CherryPy
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  MAINTAINER:=xu chen <xu.chen_3@nxp.com>
  TITLE:=Object-Oriented HTTP framework
  URL:=https://github.com/cherrypy/cherrypy
  DEPENDS:=+python +python-cheroot +python-tempora +python-portend 
endef

define Package/python-CherryPy/description
	CherryPy is a pythonic, object-oriented HTTP framework.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-CherryPy/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
	    $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
	    $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-CherryPy))
