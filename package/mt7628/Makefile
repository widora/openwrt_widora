# All rights reserved.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=mt7628
PKG_VERSION:=4.0.1.3.M
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)


include $(INCLUDE_DIR)/package.mk

define KernelPackage/mt7628
  CATEGORY:=Ralink SDK Mango
  TITLE:=Ralink MT7628 wifi AP driver by Mango
  AUTOLOAD:=$(call AutoLoad,90,mt7628)
  DEPENDS:=@TARGET_ramips_mt7688_Widora +uci2dat +iwinfo
endef



define KernelPackage/mt7628/install
	$(INSTALL_DIR) $(1)/lib/wifi/
	$(INSTALL_BIN) ./files/mt7628.sh $(1)/lib/wifi/
	$(INSTALL_BIN) ./files/ralink_common.sh $(1)/lib/wifi/
	$(INSTALL_DIR) $(1)/etc/wireless/mt7628/
	$(INSTALL_BIN) ./files/mt7628.dat $(1)/etc/wireless/mt7628/
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) ./files/connect2ap $(1)/bin/
	$(INSTALL_DIR) $(1)/lib/modules/3.18.29/
	$(INSTALL_BIN) ./files/mt7628.ko $(1)/lib/modules/3.18.29/
endef

$(eval $(call KernelPackage,mt7628))
