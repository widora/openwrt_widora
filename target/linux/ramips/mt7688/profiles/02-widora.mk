#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Widora
	NAME:=Widora
	PACKAGES:=\
		kmod-usb-core kmod-usb2 kmod-usb-ohci \
		uboot-envtools kmod-ledtrig-netdev \
  		mountd \
        	mjpg-streamer \
		uhttpd rpcd rpcd-mod-iwinfo \
		luci luci-app-mjpg-streamer luci-app-samba luci-lib-json \
		rpcd-mod-rpcsys cgi-io avrdude spi-tools \
		kmod-fs-vfat kmod-i2c-core kmod-i2c-ralink \
		kmod-nls-base kmod-nls-cp437 kmod-nls-iso8859-1 kmod-nls-utf8 \
		kmod-sdhci-mt7620 kmod-usb-storage \
		kmod-video-core kmod-video-uvc \
		kmod-sound-core kmod-sound-mtk madplay-alsa alsa-utils \
		mtk-sdk-wifi \
        	maccalc shairport_mmap reg
endef

define Profile/Widora/Description
	widora base packages.
endef
$(eval $(call Profile,Widora))
