

include $(TOPDIR)/rules.mk

PKG_NAME:=airkiss
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_KCONFIG:=RALINK_MT7620 RALINK_MT7628
#PKG_CONFIG_DEPENDS:=$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)))


include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/airkiss
  SECTION:=Ralink SDK Mango
  CATEGORY:=Ralink SDK Mango
  TITLE:=airkiss
  DEPENDS:= +libpthread +librt +libstdcpp +mt7628
endef

define Package/airkiss/description
  airkiss for wechat
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

#TARGET_CFLAGS += \
	$(foreach c, $(PKG_KCONFIG),$(if $(CONFIG_$c),-DCONFIG_$(c)=$(CONFIG_$c)))

LIBS:=-ltxdevicesdk -lpthread -ldl -lstdc++ -ltxdevicesdk

#MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFAGS="$(LIBS)"
#define Build/Compile
#		$(call Build/Compile/Default,)
#		$(TARGET_CC) smartlink.c -o smartlink -O0 -g3 -I"./include" -L"./lib" -ltxdevicesdk -lpthread -ldl -lstdc++
#		$(TARGET_CC) $(TARGET_CFLAGS) -Wall -Werror -o $(PKG_BUILD_DIR)/ap_client src/ap_client.c
#endef


define Package/airkiss/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/airkiss $(1)/bin
endef


$(eval $(call BuildPackage,airkiss))
