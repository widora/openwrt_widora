#if compile x86 demo on unbuntu 64.
CC:=//Users/mango/openwrt/openwrt-3.10.14/staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin/mipsel-openwrt-linux-gcc
export STAGING_DIR:=/Users/mango/openwrt/openwrt-3.10.14/staging_dir


all: app0
	@echo build complete


clean:
	-rm airkiss

app0:airkiss.c
	$(CC) airkiss.c -o airkiss -O0 -g3 -I"./include" -L"./lib" -lairkiss_log -lpthread -ldl -lstdc++ -lm -lrt
