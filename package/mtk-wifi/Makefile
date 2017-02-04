# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mtk-wifi
PKG_VERSION:=4013P1

include $(INCLUDE_DIR)/package.mk


define Package/mtk-wifi
  CATEGORY:=Ralink SDK Mango
  TITLE:=Ralink mt7628 wifi AP and Apcli driver
  DEPENDS:=@TARGET_ramips_mt7688 +wireless-tools +maccalc
endef

define Build/Configure
endef

define Package/mtk-wifi/install
	$(CP) ./files/* $(1)/
	$(CP) ./wifi_ko/mt_wifi_$(LINUX_VERSION)_mango.ko $(1)/lib/modules/$(LINUX_VERSION)/mt_wifi.ko
endef

$(eval $(call BuildPackage,mtk-wifi))
