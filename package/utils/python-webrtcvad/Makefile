#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-webrtcvad
PKG_VERSION:=2.0.10
PKG_RELEASE:=1
PKG_LICENSE:=MIT

PKG_SOURCE:=webrtcvad-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/webrtcvad-$(PKG_VERSION)/
PKG_SOURCE_URL:=https://pypi.python.org/packages/89/34/e2de2d97f3288512b9ea56f92e7452f8207eb5a0096500badf9dfd48f5e6/
PKG_MD5SUM:=213d2848aeebbbd22485d4ad630b5fdb
PKG_BUILD_DEPENDS:=python python-setuptools

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-webrtcvad
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  MAINTAINER:=Feng Guo <feng.guo@nxp.com>
  TITLE:=Python interface to the Google WebRTC Voice Activity Detector (VAD)
  URL:=https://github.com/wiseman/py-webrtcvad
  DEPENDS:=+python
endef

define Package/python-webrtcvad/description
  A VAD classifies a piece of audio data as being voiced or unvoiced. It can be useful for telephony and speech recognition.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-webrtcvad/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
	    $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
	    $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-webrtcvad))

