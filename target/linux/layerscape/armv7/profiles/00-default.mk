define Profile/Default
	NAME:=Default Profile
	PRIORITY:=1
endef

define Profile/Default/Description
	Default package set compatible with most boards.
endef

DEFAULT_PACKAGES+= \
	uboot-layerscape-$(SUBTARGET)-ls1021aiot

$(eval $(call Profile,Default))
