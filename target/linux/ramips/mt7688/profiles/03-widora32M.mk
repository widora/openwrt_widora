#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/Widora32M
	NAME:=Widora32M
	PACKAGES:=\
		kmod-usb-core kmod-usb2 kmod-usb-ohci \
		uboot-envtools kmod-ledtrig-netdev \
  		mountd \
        	mjpg-streamer \
		uhttpd rpcd rpcd-mod-iwinfo \
		rpcd-mod-rpcsys cgi-io spi-tools \
		kmod-fs-vfat kmod-fs-exfat kmod-fs-ext4 block-mount e2fsprogs \
		kmod-i2c-core kmod-i2c-ralink \
		kmod-nls-base kmod-nls-cp437 kmod-nls-iso8859-1 kmod-nls-utf8 \
		kmod-sdhci-mt7620 kmod-usb-storage \
		kmod-video-core kmod-video-uvc \
		kmod-sound-core kmod-sound-mtk madplay-alsa alsa-utils \
		mtk-wifi airkiss webui ated \
        	maccalc shairport_mmap reg ser2net
endef

define Profile/Widora32M/Description
	widora32M base packages.
endef
$(eval $(call Profile,Widora32M))
