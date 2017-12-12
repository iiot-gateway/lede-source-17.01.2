#
# Copyright (C) 2016 Jiang Yutang <jiangyutang1978@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

FAT32_BLOCK_SIZE=1024
FAT32_BLOCKS=$(shell echo $$(($(CONFIG_BOOT_FATFS_PARTSIZE)*1024*1024/$(FAT32_BLOCK_SIZE))))
ROOTFS_EXT4_SIZE=$(shell echo $$((($(CONFIG_TARGET_ROOTFS_PARTSIZE)+$(CONFIG_BOOT_FATFS_PARTSIZE)+5)*1024*1024)))
ROOTFS_PARTOFFSET=$(shell echo $$(($(CONFIG_BOOT_FATFS_PARTSIZE)+4)))

define Build/append-ls-rcw
	rm -f $@
	dd if=$(KDIR)/$(1)-rcw.bin >> $@
endef

define Build/append-ls-uboot
	dd if=$(KDIR)/$(1)-$(SUBTARGET)-uboot.bin >> $@
endef

define Build/append-ls1012a-uboot
	dd if=$(KDIR)/$(1)-rcw.bin of=$(BIN_DIR)/$(1)-$(SUBTARGET)-rcw-uboot.bin bs=1M count=1 conv=sync
	dd if=$(KDIR)/$(1)-$(SUBTARGET)-uboot.bin of=$(BIN_DIR)/$(1)-$(SUBTARGET)-rcw-uboot.bin bs=1M seek=1 count=3 conv=sync
endef

define Build/append-ls-fman
	dd if=$(KDIR)/$(1)-fman.bin >> $@
endef

define Build/append-ls-dtb
	$(call Image/BuildDTB,$(DTS_DIR)/$(1).dts,$(DTS_DIR)/$(1).dtb)
	dd if=$(DTS_DIR)/$(1).dtb >> $@
endef

define Build/append-ls-rootfs-ext4
	$(STAGING_DIR_HOST)/bin/make_ext4fs -l $(word 2,$(1)) -b 4096 -i 6000 -m 0 -J $(KDIR)/$(word 1,$(1))-$(word 2,$(1)).root.ext4 $(TARGET_DIR)
	dd if=$(KDIR)/$(word 1,$(1))-$(word 2,$(1)).root.ext4 >> $@
endef

define Build/append-ls-head
	./gen_layerscape_sdcard_img_head.sh $(KDIR_TMP)/$(IMG_PREFIX)-$(PROFILE)-sdcard-head.bin $(CONFIG_BOOT_FATFS_PARTSIZE) $(CONFIG_TARGET_ROOTFS_PARTSIZE)
	dd if=$(KDIR_TMP)/$(IMG_PREFIX)-$(PROFILE)-sdcard-head.bin  >> $@
endef

define Build/append-ls-ubootEnv
	dd if=uboot-env/$(1)-uboot-env.bin >> $@
endef

define Build/append-ls-bootImage
	$(call Image/BuildDTB,$(DTS_DIR)/$(word 1,$(1)).dts,$(DTS_DIR)/$(word 1,$(1)).dtb)
	rm -f $(KDIR_TMP)/$(word 2,$(1))-$(SUBTARGET)-boot.img
	mkfs.fat $(KDIR_TMP)/$(word 2,$(1))-$(SUBTARGET)-boot.img -C $(FAT32_BLOCKS)
	mcopy -i $(KDIR_TMP)/$(word 2,$(1))-$(SUBTARGET)-boot.img $(DTS_DIR)/$(word 1,$(1)).dtb ::
	mcopy -i $(KDIR_TMP)/$(word 2,$(1))-$(SUBTARGET)-boot.img $(IMAGE_KERNEL) ::uImage
	cp -r backup $(KDIR_TMP)/$(word 2,$(1))-backup
	./gen_layerscape_version_json.sh $(KDIR_TMP)/$(word 2,$(1))-backup/version.json  $(word 2,$(1))-$(SUBTARGET)
	mcopy -i $(KDIR_TMP)/$(word 2,$(1))-$(SUBTARGET)-boot.img $(KDIR_TMP)/$(word 2,$(1))-backup ::backup
	dd if=$(KDIR_TMP)/$(word 2,$(1))-$(SUBTARGET)-boot.img >> $@
endef

define Device/Default
  PROFILES = Default
  FILESYSTEMS := ext4
  KERNEL := kernel-bin | gzip | uImage gzip
  DEVICE_DTS :=
  IMAGES = firmware.bin

ifeq ($(SUBTARGET),64b)
  KERNEL_LOADADDR = 0x80080000
  KERNEL_ENTRY_POINT = 0x80080000
endif
ifeq ($(SUBTARGET),32b)
  KERNEL_LOADADDR = 0x80008000
  KERNEL_ENTRY_POINT = 0x80008000
endif
ifeq ($(SUBTARGET),armv7)
  KERNEL_LOADADDR = 0x80008000
  KERNEL_ENTRY_POINT = 0x80008000
endif
endef

ifeq ($(SUBTARGET),armv7)

define Device/ls1021aiot
	DEVICE_TITLE := ls1021aiot
	DEVICE_PACKAGES += uboot-layerscape-$(SUBTARGET)-ls1021aiot
	DEVICE_DTS = ls1021a-iot
  IMAGE/firmware.bin = append-ls-head | pad-to 4K | \
		append-ls-uboot $(1) | pad-to 1M | \
		append-ls-ubootEnv $(1) | pad-to 2M | \
		append-ls-bootImage $$(DEVICE_DTS) $(1) | pad-to $(ROOTFS_PARTOFFSET)M | \
		append-rootfs | pad-rootfs | check-size $(ROOTFS_EXT4_SIZE)
endef
TARGET_DEVICES += ls1021aiot

else

define Device/ls1043ardb
  DEVICE_TITLE := ls1043ardb-$(SUBTARGET)
  DEVICE_PACKAGES += rcw-layerscape-ls1043ardb uboot-layerscape-$(SUBTARGET)-ls1043ardb fman-layerscape-ls1043ardb
ifeq ($(SUBTARGET),64b)
  DEVICE_DTS = freescale/fsl-ls1043a-rdb
endif
ifeq ($(SUBTARGET),32b)
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls1043a-rdb
endif
  IMAGE/firmware.bin = append-ls-head | pad-to 4K | \
		append-ls-uboot $(1) | pad-to 1M | \
		append-ls-ubootEnv $(1) | pad-to 1040K | \
		append-ls-fman $(1) | pad-to 2M | \
		append-ls-bootImage $$(DEVICE_DTS) $(1) | pad-to $(ROOTFS_PARTOFFSET)M | \
		append-rootfs | pad-rootfs | check-size $(ROOTFS_EXT4_SIZE)
endef
TARGET_DEVICES += ls1043ardb

define Device/ls1046ardb
  DEVICE_TITLE := ls1046ardb-$(SUBTARGET)
  DEVICE_PACKAGES += rcw-layerscape-ls1046ardb uboot-layerscape-$(SUBTARGET)-ls1046ardb fman-layerscape-ls1046ardb
ifeq ($(SUBTARGET),64b)
  DEVICE_DTS = freescale/fsl-ls1046a-rdb
endif
ifeq ($(SUBTARGET),32b)
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls1046a-rdb
endif
  IMAGE/firmware.bin = append-ls-head | pad-to 4K | \
		append-ls-uboot $(1) | pad-to 1M | \
		append-ls-ubootEnv $(1) | pad-to 1040K | \
		append-ls-fman $(1) | pad-to 2M | \
		append-ls-bootImage $$(DEVICE_DTS) $(1) | pad-to $(ROOTFS_PARTOFFSET)M | \
		append-rootfs | pad-rootfs | check-size $(ROOTFS_EXT4_SIZE)
endef
TARGET_DEVICES += ls1046ardb

define Device/ls1012ardb
  DEVICE_TITLE := ls1012ardb-$(SUBTARGET)
  DEVICE_PACKAGES += rcw-layerscape-ls1012ardb uboot-layerscape-$(SUBTARGET)-ls1012ardb kmod-ppfe ppfe-ls1012ardb
ifeq ($(SUBTARGET),64b)
  DEVICE_DTS = freescale/fsl-ls1012a-rdb
endif
ifeq ($(SUBTARGET),32b)
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls1012a-rdb
endif
  IMAGE/firmware.bin = append-ls-head | pad-to 2M | \
		append-ls1012a-uboot $(1) | \
		append-ls-bootImage $$(DEVICE_DTS) $(1) | pad-to $(ROOTFS_PARTOFFSET)M | \
		append-rootfs | pad-rootfs | check-size $(ROOTFS_EXT4_SIZE)
endef
TARGET_DEVICES += ls1012ardb

define Device/ls1088ardb
  DEVICE_TITLE := ls1088ardb-$(SUBTARGET)
  FILESYSTEMS := squashfs
  DEVICE_PACKAGES += rcw-layerscape-ls1088ardb uboot-layerscape-$(SUBTARGET)-ls1088ardb mc-binary-ls1088ardb
ifeq ($(SUBTARGET),64b)
  DEVICE_DTS = freescale/fsl-ls1088a-rdb
endif
ifeq ($(SUBTARGET),32b)
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls1088a-rdb
endif
  IMAGE/firmware.bin = append-ls-dtb $$(DEVICE_DTS) | pad-to 1M | append-kernel | pad-to 6M | \
  					append-ls-rootfs-ext4 $(1) 17M | check-size 24117249
endef
TARGET_DEVICES += ls1088ardb

define Device/ls2088ardb
  DEVICE_TITLE := ls2088ardb-$(SUBTARGET)
  FILESYSTEMS := squashfs
  DEVICE_PACKAGES += rcw-layerscape-ls2088ardb uboot-layerscape-$(SUBTARGET)-ls2088ardb mc-binary-ls2088ardb
ifeq ($(SUBTARGET),64b)
  DEVICE_DTS = freescale/fsl-ls2088a-rdb
endif
ifeq ($(SUBTARGET),32b)
  DEVICE_DTS = ../../../arm64/boot/dts/freescale/fsl-ls2088a-rdb
endif
  IMAGE/firmware.bin = append-ls-dtb $$(DEVICE_DTS) | pad-to 1M | append-kernel | pad-to 6M | \
					append-rootfs | pad-rootfs | check-size 24117249
endef
TARGET_DEVICES += ls2088ardb

endif

$(eval $(call BuildImage))
