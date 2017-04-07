#
# Copyright (C) Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/ppfe
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Freescale PPFE Driver support
  KCONFIG:=CONFIG_FSL_PPFE
  FILES:=$(LINUX_DIR)/drivers/staging/fsl_ppfe/pfe.ko
  AUTOLOAD:=$(call AutoLoad,35,ppfe)
endef

define KernelPackage/ppfe/description
 Kernel modules for Freescale PPFE Driver support.
endef

$(eval $(call KernelPackage,ppfe))

define KernelPackage/nxp_pn5xx
  SUBMENU:= $(OTHER_MENU)
  TITLE:= NXP NFC PN5XX driver support
  KCONFIG:= CONFIG_NFC_NXP_PN5XX
  FILES:= $(LINUX_DIR)/drivers/misc/nxp-pn5xx/pn5xx_i2c.ko
  AUTOLOAD:=$(call AutoLoad,35,nxp_pn5xx)
endef

define KernelPackage/nxp_pn5xx/description
        Kernel modules for NXP PN5XX NFC driver support
endef

$(eval $(call KernelPackage,nxp_pn5xx))
