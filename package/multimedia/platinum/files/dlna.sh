#!/bin/sh

NAME="$(uci get wireless.ap.ssid)"

FileMediaServerTest -f "$NAME-DMS" /tmp &
MediaRendererTest -f "$NAME-L" >/dev/console 2>&1 &
