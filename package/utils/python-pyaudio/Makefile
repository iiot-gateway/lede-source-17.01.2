#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pyaudio
PKG_VERSION:=0.2.10
PKG_RELEASE:=1
PKG_LICENSE:=MIT

PKG_SOURCE:=PyAudio-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/PyAudio-$(PKG_VERSION)/
PKG_SOURCE_URL:=https://pypi.python.org/packages/44/39/f3b9b16d32cc8588fa06bb2e813cea35363f5502117cd6cc7f619b556d9f/
PKG_MD5SUM:=b65354d997fa00a9166f8dbcb83075e6
PKG_BUILD_DEPENDS:=python python-setuptools portaudio

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-pyaudio
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  MAINTAINER:=Feng Guo <feng.guo@nxp.com>
  TITLE:=Python Bindings for PortAudio
  URL:=http://people.csail.mit.edu/hubert/pyaudio/
  DEPENDS:=+python +portaudio
endef

define Package/python-pyaudio/description
  Bindings for PortAudio v19, the cross-platform audio input/output stream library.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-pyaudio/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
	    $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
	    $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-pyaudio))
