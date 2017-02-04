# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mtk-wifi
PKG_VERSION:=V1.1
PKG_RELEASE=$(PKG_SOURCE_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/mtk-wifi
  CATEGORY:=Ralink SDK Mango
  TITLE:=Ralink mt7628 wifi AP and Apcli driver
  DEPENDS:=@TARGET_ramips_mt7688 +wireless-tools +maccalc
endef

define Build/Configure
	true
endef

define Build/Compile
	true
endef

define Package/mtk-wifi/install
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION)/
	$(CP) ./wifi_ko/mt_wifi_$(LINUX_VERSION)_mango.ko $(1)/lib/modules/$(LINUX_VERSION)/mt_wifi.ko
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,mtk-wifi))
