#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=pwm_test
PKG_VERSION:=0.0.1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/pwm_test
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+kmod-widora_pwm
  TITLE:=PWM testing utility
endef

define Package/pwm_test/description
  MT7688 PWM testing utility.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) CFLAGS="$(TARGET_CFLAGS) -I$(LINUX_DIR)/include"
endef

define Package/pwm_test/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pwm_test $(1)/sbin/
endef

$(eval $(call BuildPackage,pwm_test))
