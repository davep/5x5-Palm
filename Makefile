SHELL = bash
OBJS = 5x5.o

CC = m68k-palmos-coff-gcc

CSFLAGS = -O2 -S
CFLAGS = -O2 -g

PILRC = pilrc
TXT2BITM = txt2bitm
OBJRES = m68k-palmos-coff-obj-res
BUILDPRC = build-prc

ICONTEXT = '5x5'
APPID = DAP0

all: 5x5.prc

.S.o:
	$(CC) $(TARGETFLAGS) -c $<

.c.s:
	$(CC) $(CSFLAGS) $<

5x5.prc: code0000.5x5.bin code0001.5x5.bin data0000.5x5.bin bin.res
	$(BUILDPRC) 5x5.prc $(ICONTEXT) $(APPID) code0001.5x5.grc code0000.5x5.grc data0000.5x5.grc *.bin pref0000.5x5.grc

code0000.5x5.bin: 5x5
	$(OBJRES) 5x5

code0001.5x5.bin: code0000.5x5.bin

data0000.5x5.bin: code0000.5x5.bin

bin.res: 5x5.rcp
	$(PILRC) 5x5.rcp .
	$(TXT2BITM) 5x5.pbitm
	touch bin.res

5x5: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o 5x5

cleanish:
	rm -rf *.[oa] 5x5 *.bin bin.res *~ *.grc *.BAK *.bak

clean: cleanish
	rm -rf *.prc
