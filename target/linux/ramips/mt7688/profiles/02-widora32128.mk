#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/WIDORA32128
	NAME:=WIDORA32128
	PACKAGES:=\
		kmod-usb-core kmod-usb2 kmod-usb-ohci \
		kmod-ledtrig-netdev \
  		mountd \
		uhttpd rpcd rpcd-mod-iwinfo \
		rpcd-mod-rpcsys cgi-io spi-tools \
		kmod-sdhci-mt7620 kmod-fs-vfat kmod-fs-exfat kmod-fs-ext4 block-mount e2fsprogs \
		kmod-i2c-core kmod-i2c-mt7621 \
		kmod-nls-base kmod-nls-cp437 kmod-nls-iso8859-1 kmod-nls-utf8 \
		kmod-video-core kmod-video-uvc mjpg-streamer \
		kmod-usb-storage kmod-usb-net kmod-usb-net-rndis kmod-usb-serial kmod-usb-serial-wwan \
		mtk-wifi airkiss webui ated luci\
        maccalc reg ser2net
endef

define Profile/WIDORA32128/Description
	widora 32MB flash/128MB ram base packages.
endef
$(eval $(call Profile,WIDORA32128))
