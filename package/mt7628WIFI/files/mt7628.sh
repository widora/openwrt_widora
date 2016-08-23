#!/bin/sh
append DRIVERS "mt7628"

. /lib/wifi/ralink_common.sh

prepare_mt7628() {
	prepare_ralink_wifi mt7628
}

scan_mt7628() {
	scan_ralink_wifi mt7628 mt7628
}


disable_mt7628() {
	disable_ralink_wifi mt7628
}

enable_mt7628() {
	enable_ralink_wifi mt7628 mt7628
}

detect_mt7628() {
#	detect_ralink_wifi mt7628 mt7628
#	ssid=mt7628-`ifconfig eth0 | grep HWaddr | cut -c 51- | sed 's/://g'`
	cd /sys/module/
	[ -d $module ] || return
	[ -e /etc/config/wireless ] && return

	password=
	encryption=0
	wpa_cipher=3
	
	case $encryption in
		0)encryption="none";;
		1)encryption="wep-auto";;
		2)encryption="psk";; 
		4)encryption="psk2";;
		6)encryption="psk-mixed";;
		*)encryption="none";;
	esac
	
	case $wpa_cipher in
		1)wpa_cipher="TKIP";;
		2)wpa_cipher="AES";;
		3)wpa_cipher="TKIP+AES";; 
		*)wpa_cipher="AES";; 
	esac
	
         cat <<EOF
config wifi-device         mt7628
        option type        mt7628
        option vendor      ralink
        option band        2.4G
        option channel 	   'auto'
        option autoch_skip '12;13;'
        option wifimode    '9'
        option bw          '1'
        option country     'CN'
        option region      '1'

config wifi-iface
        option device      mt7628
        option ifname      ra0
        option network     lan
        option mode        ap
        option disabled    '0'
        option ssid        Widora-$(cat /sys/class/net/eth0/address|awk -F ":" '{print $5""$6}'| tr a-z A-Z)
        option encryption  $encryption
        option wpa_crypto  $wpa_cipher
        option key         $password



EOF


}


