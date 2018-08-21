EXE:=switch

ifeq ($(CONFIG_RALINK_MT7620),y)
SRC=switch_gsw.c
else
ifeq ($(CONFIG_RALINK_MT7621),y)
SRC=switch_gsw.c
else
SRC=switch.c
endif
endif

ifeq ($(CONFIG_P5_RGMII_TO_MT7530_MODE),y)
EXE+=switch_7530
endif

all: $(EXE) 

switch_7530:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ switch_7530.c $(LDLIBS)

switch:
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC) $(LDLIBS)

clean:
	-rm -f switch *.elf *.gdb *.o

