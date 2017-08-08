#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pocketsphinx
PKG_VERSION:=0.1.3
PKG_RELEASE:=1
PKG_LICENSE:=MIT

PKG_SOURCE:=pocketsphinx-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/pocketsphinx-$(PKG_VERSION)/
PKG_SOURCE_URL:=https://pypi.python.org/packages/a0/4e/70c500da7558d2a8e67fd8cfd33f5a4b90faf7163d7e4023fa989a89902a/
PKG_MD5SUM:=e8401e131f7a193ea49ad77aa6c846e3
PKG_BUILD_DEPENDS:=python python-setuptools pulseaudio-daemon

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-pocketsphinx
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  MAINTAINER:=Feng Guo <feng.guo@nxp.com>
  TITLE:=Python interface to CMU SphinxBase and PocketSphinx libraries
  URL:=https://github.com/bambocher/pocketsphinx-python
  DEPENDS:=+python +pulseaudio-daemon
endef

define Package/python-pocketsphinx/description
  Python interface to CMU SphinxBase and PocketSphinx libraries
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-pocketsphinx/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
	    $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
	    $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-pocketsphinx))
