SHELL=/bin/bash

BIN=/home/linuxbin/emacsimdir/bin

GCC=$(BIN)/m68k-elf-gcc
AR=$(BIN)/m68k-elf-ar

CFILES=verhogen.c passeren.c

SFILES=HALT.s STLDADDRESS.s STLDILLEGAL.s STLDSYS10.s STLDSYS14.s STLDSYS9.s STLDTERM2.s STST.s STLDCLOCK.s STLDMM.s STLDSYS11.s STLDSYS15.s STLDSYS.s STLDTERM3.s SYS.s LDIT.s STLDDISK0.s STLDPRINT0.s STLDSYS12.s STLDSYS16.s STLDTERM0.s STLDTERM4.s LDST.s STCK.s STLDFLOPPY0.s STLDPRIVILEG.s STLDSYS13.s STLDSYS17.s STLDTERM1.s STLDZERO.s


all: libutil.a


clean:
	rm -f lib/libutil.a


libutil.a: $(CFILES) $(SFILES)
	for FILE in $(CFILES) $(SFILES); do \
		BASE=$$(cut -d "." -f1 <<<$$FILE); \
		$(GCC) -m68000 -c -o $$BASE.o $$FILE -g -gstabs; \
		$(AR) r lib/libutil.a $$BASE.o; \
		rm $$BASE.o; \
	done
