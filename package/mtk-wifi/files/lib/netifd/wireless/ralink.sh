#!/bin/sh
. /lib/netifd/netifd-wireless.sh

init_wireless_driver "$@"
maclist=""
force_channel=""

drv_ralink_init_device_config() {
	config_add_string country region variant log_level short_preamble
	config_add_boolean noscan
	config_add_int channel
}

drv_ralink_init_iface_config() {
	config_add_string 'auth_server:host'
	config_add_string auth_secret
	config_add_int 'auth_port:port'

	config_add_string ifname apname mode bssid ssid encryption key key1 key2 key3 key4 wps_pushbutton macfilter led
        config_add_boolean hidden sta_isolate
        config_add_array 'maclist:list(macaddr)'
}

drv_ralink_cleanup() {
	logger drv_ralink_cleanup
}

ralink_setup_ap(){
	local name="$1"
	local eap=0

	json_select config
	json_get_vars mode ifname ssid encryption key key1 key2 key3 key4 wps_pushbutton server port hidden sta_isolate macfilter
	json_get_values maclist maclist

	ifconfig $ifname up

	[ -z "$force_channel" ] || iwpriv $ifname set Channel=$force_channel
	[ "$sta_isolate" = 1 ] || sta_isolate=0
	iwpriv $ifname set NoForwarding=$sta_isolate
	iwpriv $ifname set NoForwardingBTNBSSID=$sta_isolate
	iwpriv $ifname set NoForwardingMBCast=$sta_isolate
	
	[ "$hidden" = 1 ] || hidden=0
	iwpriv $ifname set HideSSID=$hidden

	wsc_methods=0
	wsc_mode=0
	case "$encryption" in
	psk*|mixed*)
		local enc="WPA2PSK"
		case "$encryption" in
			psk | psk+*) enc=WPAPSK;;
			psk2 | psk2*) enc=WPA2PSK;;
			mixed*) enc=WPAPSKWPA2PSK;;
			wpa | wpa+*) eap=1; enc=WPA;;
			wpa2*) eap=1; enc=WPA2;;
		esac
		local crypto="AES"
		case "$encryption" in
			*tkip+aes*) crypto=TKIPAES;;
			*tkip*) crypto=TKIP;;
		esac
		[ "$eap" = "1" ] && {
			iwpriv $ifname set RADIUS_Server=$server
			iwpriv $ifname set RADIUS_Port=$port
			iwpriv $ifname set RADIUS_Key=$key
			iwpriv $ifname set own_ip_addr=192.168.1.1
			iwpriv $ifname set IEEE8021X=0
			iwpriv $ifname set EAPifname=eth0.2
			iwpriv $ifname set PreAuthIfname=br-lan
		}
		iwpriv $ifname set AuthMode=$enc
		iwpriv $ifname set EncrypType=$crypto
		iwpriv $ifname set IEEE8021X=0
		iwpriv $ifname set "SSID=${ssid}"
		[ "$eap" = "1" ] || iwpriv $ifname set "WPAPSK=${key}"
		iwpriv $ifname set DefaultKeyID=2
		iwpriv $ifname set "SSID=${ssid}"
		if [ "$wps_pushbutton" == "1" ]; then
			wsc_methods=128
			wsc_mode=7
		fi
		;;
	wep)
		iwpriv $ifname set AuthMode=WEPAUTO
		iwpriv $ifname set EncrypType=WEP
		iwpriv $ifname set IEEE8021X=0
		for idx in 1 2 3 4; do
			json_get_var keyn key${idx}
			[ -n "$keyn" ] && iwpriv $ifname set "Key${idx}=${keyn}"
		done
		iwpriv $ifname set DefaultKeyID=${key}
		iwpriv $ifname set "SSID=${ssid}"
		;;
	none)
		iwpriv $ifname set "SSID=${ssid}"
		;;
	esac

	iwpriv $ifname set WscConfMode=$wsc_mode
	iwpriv $ifname set WscConfStatus=2
	iwpriv $ifname set WscMode=2
	iwpriv $ifname set ACLClearAll=1
	[ -n "$maclist" ] && {
		for m in ${maclist}; do
			logger iwpriv $ifname set ACLAddEntry="$m"
			iwpriv $ifname set ACLAddEntry="$m"
		done
	}
	case "$macfilter" in
	allow)
		iwpriv $ifname set AccessPolicy=1
		;;
	deny)
		iwpriv $ifname set AccessPolicy=2
		;;
	*)
		iwpriv $ifname set AccessPolicy=0
		;;
	esac
	json_select ..


	wireless_add_vif "$name" "$ifname"
}

ralink_setup_sta(){
	local name="$1"
	local hide=1
	json_select config
	json_get_vars mode apname ifname ssid bssid encryption key key1 key2 key3 key4 wps_pushbutton led

	linkit_mode="$(uci get wireless.radio0.linkit_mode)"
	[ "${linkit_mode}" == "ap" ] && return
	[ "${linkit_mode}" == "apsta" ] && hide=0

	key=
	case "$encryption" in
		psk*|mixed*) json_get_vars key;;
		wep) json_get_var key key1;;
	esac
	json_select ..
	killall ap_client
	/sbin/ap_client "ra0" "$ifname" "${ssid}" "${key}" "${bssid}" "${hide}" "${led}"
	sleep 1
	wireless_add_process "$(cat /tmp/apcli-${ifname}.pid)" /sbin/ap_client ra0 "$ifname" "${ssid}" "${key}" "${bssid}" "${hide}" "${led}"

	wireless_add_vif "$name" "$ifname"
}

drv_ralink_setup() {
	local ifname="$1"
	local bcn_active=0
	wmode=9
	VHT=0
	VHT_SGI=0
	HT=0
	EXTCHA=0

	linkit_mode="$(uci get wireless.radio0.linkit_mode)"
	[ "${linkit_mode}" != "sta" ] && bcn_active=1

	json_select config
	json_get_vars variant region country channel htmode log_level short_preamble noscan:0
	json_select ..

	[ -z "$region" ] && region=0

	[ "$short_preamble" = 1 ] || short_preamble=0 

	case ${htmode:-none} in
	HT20)
		wmode=9
		HT=0
		;;
	HT40)
		wmode=9
		HT=1
		EXTCHA=1
		;;
	VHT20)
		wmode=15
		HT=0
		VHT=0
		VHT_SGI=1
		;;
	VHT40)
		wmode=15
		HT=1
		VHT=0
		VHT_SGI=1
		EXTCHA=1
		;;
	VHT80)
		wmode=15
		HT=1
		VHT=1
		VHT_SGI=1
		EXTCHA=1
		;;
	*)
		case $hwmode in
		a) wmode=2;;
		g) wmode=3;;
		esac
		;;
	esac

	if [ "$auto_channel" -gt 0 ]; then
		force_channel=""
		channel=0
		auto_channel=3
	else
		force_channel=$channel
		auto_channel=0
	fi

	coex=1
	[ "$noscan" -gt 0 ] && coex=0

	cat /etc/Wireless/${variant}_tpl.dat > /tmp/${variant}.dat
	cat >> /tmp/${variant}.dat<<EOF
Beacon=${bcn_active}
BssidNum=4
HT_BW=${HT:-0}
HT_EXTCHA=${EXTCHA:-0}
CountryRegion=${region}
CountryCode=${country}
WirelessMode=${wmode:-9}
Channel=${channel:-8}
AutoChannelSelect=$auto_channel
Debug=${log_level:-3}
SSID=${ssid:-OpenWrt}
HT_BSSCoexistence=$coex
WscConfMethods=680
EOF
	for_each_interface "ap" ralink_setup_ap
	for_each_interface "sta" ralink_setup_sta
	wireless_set_up
	LED="$(uci get wireless.sta.led)"
	linkit_mode="$(uci get wireless.radio0.linkit_mode)"
	[ "${linkit_mode}" = "ap" -a -n "${LED}" -a -f /sys/class/leds/${LED}/trigger ] && ap_client ${LED} set
}

ralink_teardown() {
	json_select config
	json_get_vars ifname
	json_select ..

	ifconfig $ifname down

	LED="$(uci get wireless.sta.led)"
	[ -n "${LED}" -a -f /sys/class/leds/${LED}/trigger ] && ap_client ${LED} clear
}

drv_ralink_teardown() {
	wireless_process_kill_all
	for_each_interface "ap sta" ralink_teardown
}

add_driver ralink
