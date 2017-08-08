#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pyalsaaudio
PKG_VERSION:=0.8.4
PKG_RELEASE:=1
PKG_LICENSE:=MIT

PKG_SOURCE:=pyalsaaudio-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/pyalsaaudio-$(PKG_VERSION)/
PKG_SOURCE_URL:=https://pypi.python.org/packages/52/b6/44871791929d9d7e11325af0b7be711388dfeeab17147988f044a41a6d83/
PKG_MD5SUM:=b46f69561bc85fc52e698b2440ca251e
PKG_BUILD_DEPENDS:=python python-setuptools alsa-lib

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-pyalsaaudio
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  MAINTAINER:=Feng Guo <feng.guo@nxp.com>
  TITLE:=ALSA bindings
  URL:=http://larsimmisch.github.io/pyalsaaudio/
  DEPENDS:=+python +alsa-lib
endef

define Package/python-pyalsaaudio/description
  This package contains wrappers for accessing the ALSA API from Python. It is fairly complete for PCM devices and Mixer access.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-pyalsaaudio/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
	    $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
	    $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-pyalsaaudio))
