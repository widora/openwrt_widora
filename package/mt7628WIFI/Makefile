

include $(TOPDIR)/rules.mk

PKG_NAME:=mt7628WIFI
PKG_RELEASE:=4.0.1.3.M



include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/mt7628WIFI
  SECTION:=Ralink SDK Mango
  CATEGORY:=Ralink SDK Mango
  TITLE:=mt7628 WIFI driver by mango
  DEPENDS:=@TARGET_ramips_mt7688 +iwinfo +libuci +wireless-tools
endef

define Package/mt7628WIFI/description
  mt7628 WIFI driver by mango
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef


TARGET_CFLAGS += -luci -Wall
TARGET_CFLAGS += -Wno-error=format-security



define Build/Configure
endef

define Package/mt7628WIFI/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uci2dat $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/connect2ap $(1)/usr/bin
	$(INSTALL_DIR) $(1)/lib/wifi
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mt7628.sh $(1)/lib/wifi
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ralink_common.sh $(1)/lib/wifi
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mt7628.ko $(1)/lib/modules/$(LINUX_VERSION)
	$(INSTALL_DIR) $(1)/etc/wireless/mt7628
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mt7628.dat $(1)/etc/wireless/mt7628
	$(INSTALL_DIR) $(1)/etc/modules.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/52-mt7628 $(1)/etc/modules.d
	

	
	

endef


$(eval $(call BuildPackage,mt7628WIFI))
