#
# Copyright (C) 2008-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=drv_regopt
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/drv_regopt
  SUBMENU:=Other modules
  DEPENDS:=@!LINUX_3_3
  TITLE:=option the register
  FILES:=$(PKG_BUILD_DIR)/drv_regopt.ko
  AUTOLOAD:=$(call AutoLoad,30,drv_regopt,1)
  KCONFIG:=
endef

define KernelPackage/drv_regopt/description
 This is a replacement for the following in-kernel drivers:
 1) 
 2) 

 Instead of generating input events (like in-kernel drivers do) it generates
 uevent-s and broadcasts them. This allows disabling input subsystem which is
 an overkill for OpenWrt simple needs.
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

$(eval $(call KernelPackage,drv_regopt))
