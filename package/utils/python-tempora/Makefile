#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-tempora
PKG_VERSION:=1.8
PKG_RELEASE:=1
PKG_LICENSE:=MIT

PKG_SOURCE:=tempora-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/tempora-$(PKG_VERSION)/
PKG_SOURCE_URL:=https://pypi.python.org/packages/b4/61/9a4b4e07b8a2a1051ff15fc36083fb9174f8474047fb2822d32b473aedb9/
PKG_MD5SUM:=dd5e0248713e5d61009eabc55b032850
PKG_BUILD_DEPENDS:=python python-setuptools

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-tempora
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  MAINTAINER:=xu chen <xu.chen_3@nxp.com>
  TITLE:=Objects and routines pertaining to date and time
  URL:=https://github.com/jaraco/tempora
  DEPENDS:=+python
endef

define Package/python-tempora/description
  Objects and routines pertaining to date and time (tempora)
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-tempora/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
	    $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
	    $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-tempora))
