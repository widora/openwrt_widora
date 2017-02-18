#################################################################
# Compile Options
# v - 1:Old Agent,2:Agent with new packet format
# P = 1 for PC, p = 2 for APSOC
# m = 1 for pthread, m = 2 for fork, m = 3 for IPC socket
# Note: 
# If you use v=1, then m=0
#################################################################
v ?= 2
m ?= 3#Multi-Processing
#################################################################
AGENT_VERSION = $(v)
MULTI_PROC = $(m)
ifeq ($(PLATFORM),2)
NVDIR = $(ROOTDIR)/lib/libnvram
CFLAGS += -I$(NVDIR) 
LDFLAGS += -lnvram
EXTRA_CLFAG += -DMT_ATED_APSOC
endif

EXEC = ated
SRCS = ate.c
ifeq ($(AGENT_VERSION),2)
SRCS = ate-v2.c
ETH = eth.c
DRI = ioctl2driver.c
endif
#################################################################
#PLATFORM = 5VT
#PLATFORM = STAR
#PLATFORM = INF_TWINPASS
#PLATFORM = BROADCOM
PLATFORM = APSOC
#PLATFORM = TC3162U
#PLATFORM = PC

ifeq ($(PLATFORM),5VT)
#Please fix the PATH to fit your environment...
CROSS_COMPILE = /opt/crosstool/uClibc/bin/arm-linux-
endif

ifeq ($(PLATFORM),STAR)
#Please fix the PATH to fit your environment...
CROSS_COMPILE = /opt/star/tools/arm-linux/bin/arm-linux-
endif

ifeq ($(PLATFORM),INF_TWINPASS)
#Please fix the PATH to fit your environment...
CROSS_COMPILE = /opt/uclibc-toolchain/ifx-lxdb-1-1/gcc-3.3.6/toolchain-mips/bin/mips-linux-uclibc-
EXTRA_CFLAGS += -Dbig_endian
endif

ifeq ($(PLATFORM),BROADCOM)
#Please fix the PATH to fit your environment...
CROSS_COMPILE = /opt/uclibc-toolchain/bin/mips-linux-uclibc-
EXTRA_CFLAGS += -Dbig_endian
endif

ifeq ($(PLATFORM), TC3162U)
STRIP = $(CROSS_COMPILE)strip
EXTRA_CFLAGS += -Dbig_endian
endif

ifeq ($(PLATFORM),PC)
CROSS_COMPILE =
endif
#################################################################

#EXTRA_CFLAGS += -DDBG
ifeq ($(MULTI_PROC),1)
$(info ************  MULTI-PROCESS WITH PTHREAD ************)
EXTRA_CFLAGS += -DMT_ATED_THREAD=1
LDFLAGS += -pthread
MULTI = multi_thread.c
endif
ifeq ($(MULTI_PROC),2)
$(info ************  MULTI-PROCESS WITH FORK  ************)
EXTRA_CFLAGS += -DMT_ATED_SHM=1
LDLIBS += -lrt
#LDFLAGS += -lrt
LDFLAGS += -pthread
MULTI = multi_fork.c
endif
ifeq ($(MULTI_PROC),3)
$(info ************  MULTI-PROCESS WITH IPC SOCKET  ************)
EXTRA_CFLAGS += -DMT_ATED_IPC_SOCK=1
MULTI = ipc_socket.c
endif

#CC := $(CROSS_COMPILE)gcc
#LD := $(CROSS_COMPILE)ld

all: $(EXEC)

ifeq ($(PLATFORM),PC)
ifeq ($(AGENT_VERSION),1)
$(EXEC): $(SRCS)
	$(CC)  -v -Wall $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $(SRCS) $(LDLIBS)
else
$(EXEC): $(SRCS) $(ETH) $(DRI) $(MULTI)
	$(CC) -v -Wall $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $(SRCS) $(ETH) $(DRI) $(MULTI) $(LDLIBS)
endif# Version
else
ifeq ($(PLATFORM),APSOC)
ifeq ($(AGENT_VERSION),1)
$(EXEC): $(SRCS) $(ROOTDIR)/$(LINUXDIR)/include/linux/autoconf.h
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $(SRCS) $(LDLIBS)
else
$(EXEC): $(SRCS) $(ETH) $(DRI) $(MULTI)
	$(CC) -v -Wall $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $(SRCS) $(ETH) $(DRI) $(MULTI) $(LDLIBS)
endif#Version
else
ifeq ($(AGENT_VERSION),1)
$(EXEC): $(SRCS)
	$(CC)  -v -Wall $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $(SRCS) $(LDLIBS)
	cp ated /tftpboot
else
$(EXEC): $(SRCS) $(ETH) $(DRI) $(MULTI)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $(SRCS) $(ETH) $(DRI) $(MULTI) $(LDLIBS)
ifeq ($(PLATFORM), TC3162U)
	$(STRIP) -s $(EXEC)
else
	cp ated /tftpboot
endif
endif#Version
endif
endif

clean:
	-rm -f $(EXEC) *.o

