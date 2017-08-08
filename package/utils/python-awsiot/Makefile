# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-awsiot
PKG_VERSION:=1.1.1
PKG_RELEASE:=1
PKG_LICENSE:=MIT

PKG_SOURCE:=AWSIoTPythonSDK-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/AWSIoTPythonSDK-$(PKG_VERSION)/
PKG_SOURCE_URL:=https://pypi.python.org/packages/eb/45/6cfe2580fa0e64ce17065cc3ba1c9965c20358f6028c485da3165ae79b9a/
PKG_MD5SUM:=470a37dfe48fb9e448a1dce41b9abc97
PKG_BUILD_DEPENDS:=python python-setuptools

include $(INCLUDE_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-awsiot
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  MAINTAINER:=Feng Guo <feng.guo@nxp.com>
  TITLE:=SDK for connecting to AWS IoT using Python.
  URL:=https://github.com/aws/aws-iot-device-sdk-python.git
  DEPENDS:=+python
endef

define Package/python-awsiot/description
  The AWS IoT Device SDK for Python allows developers to write Python script to use their devices to access the AWS IoT platform through MQTT or MQTT over the WebSocket protocol.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,install --prefix=/usr --root=$(PKG_INSTALL_DIR))
endef

define Package/python-awsiot/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)
	$(CP) \
	    $(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
	    $(1)$(PYTHON_PKG_DIR)
endef

$(eval $(call BuildPackage,python-awsiot))
