#!bin/sh
restoreEsw()
{
	switch reg w 14 5555
	switch reg w 40 1001
	switch reg w 44 1001
	switch reg w 48 1001
	switch reg w 4c 1
	switch reg w 50 2001
	switch reg w 70 ffffffff
	switch reg w 98 7f7f
	switch reg w e4 7f
	
	#clear mac table if vlan configuration changed
	switch clear
}
configEsw()
{
	switch reg w 14 405555
	switch reg w 50 2001
	switch reg w 98 7f3f
if [ "$CONFIG_RAETH_SPECIAL_TAG" == "y" ]; then
	switch reg w e4 40043f  
else
	switch reg w e4 3f
fi

	if [ "$1" = "LLLLW" ]; then
if [ "$CONFIG_RAETH_SPECIAL_TAG" == "y" ]; then
		switch reg w 40 7007
		switch reg w 44 7007
		switch reg w 48 7008
		switch reg w 70 48444241
		switch reg w 74 50ef6050
else
		switch reg w 40 1001
		switch reg w 44 1001
		switch reg w 48 1002
		switch reg w 70 ffff506f
fi
	elif [ "$1" = "WLLLL" ]; then
if [ "$CONFIG_RAETH_SPECIAL_TAG" == "y" ]; then
		switch reg w 40 7008
		switch reg w 44 7007
		switch reg w 48 7007
		switch reg w 70 48444241
		switch reg w 74 41fe6050
else
		switch reg w 40 1002
		switch reg w 44 1001
		switch reg w 48 1001
		switch reg w 70 ffff417e
fi
	elif [ "$1" = "W1234" ]; then
		switch reg w 40 1005
		switch reg w 44 3002
		switch reg w 48 1004
		switch reg w 70 50484442
		switch reg w 74 ffffff41
	elif [ "$1" = "12345" ]; then
		switch reg w 40 2001
		switch reg w 44 4003
		switch reg w 48 1005
		switch reg w 70 48444241
		switch reg w 74 ffffff50
	elif [ "$1" = "GW" ]; then
		switch reg w 40 1001
		switch reg w 44 1001
		switch reg w 48 2001
		switch reg w 70 ffff605f
	elif [ "$1" = "G01234" ]; then
		switch reg w 40 2001
		switch reg w 44 4003
		switch reg w 48 6005
		switch reg w 70 48444241
		switch reg w 74 ffff6050
	fi

	#clear mac table if vlan configuration changed
	switch clear
}

setup_switch()
{
    has_vlan=`uci -q get network.wan.ifname`
    
    restoreEsw

    if [ ! -z $has_vlan ]; then
        configEsw WLLLL
    fi
}

reset_switch()
{
    restoreEsw
}
