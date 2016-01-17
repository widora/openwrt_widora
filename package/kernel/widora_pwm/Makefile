#
# Copyright (C) 2013-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk


PKG_NAME:=widora_pwm
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/widora_pwm
  SUBMENU:=Other modules
  TITLE:=widora pwm driver
  FILES:=$(PKG_BUILD_DIR)/widora_pwm.ko
  AUTOLOAD:=$(call AutoLoad,90,widora_pwm,1)
  KCONFIG:=
endef

define KernelPackage/widora_pwm/description
	this is a pwm driver base on mtk pwm driver
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
	modules
endef

$(eval $(call KernelPackage,widora_pwm))
