#
# Project manager generated MAKEFILE
#
TITLE = tst850
DEVFILE = C:\Program Files\NEC Electronics Tools\DEV\DF3716.800
PROJDIR = C:\user\Prj_V850\tst850
TOOLDIR = C:\Program Files\NEC Electronics Tools\PM+\V6.11\BIN
WORKDIR = C:\user\Prj_V850\tst850
DEBUG = 

CC	= "C:\Program Files\NEC Electronics Tools\CA850\E3.11a\bin\ca850.exe"
CFLAGS	= -cpu F3716 -ansi -w2 -Xsconst=512 -Xcxxcom -Xkt=s -Xr -Wp,-S
AS	= "C:\Program Files\NEC Electronics Tools\CA850\E3.11a\bin\as850.exe"
ASFLAGS	= -cpu F3716
LD	= "C:\Program Files\NEC Electronics Tools\CA850\E3.11a\bin\ld850.exe"
LIBDIR	= C:\Program Files\NEC Electronics Tools\CA850\E3.11a\lib850\r32
STARTUP	= startup.o
DEP_STARTUP = 
LINKDIR	= tst850.dir
LDFLAGS	= -cpu F3716 -o tst850.out -D $(LINKDIR) -m
LIBRARY	= "$(LIBDIR)\libc.a" \
	  "$(LIBDIR)\libr.a"
ROMPCRT	= "$(LIBDIR)\rompcrt.o"
LDLIBS	= -lc -lr $(ROMPCRT)
ROMP	= "C:\Program Files\NEC Electronics Tools\CA850\E3.11a\bin\romp850.exe"
RPFLAGS	= 
HX	= "C:\Program Files\NEC Electronics Tools\CA850\E3.11a\bin\hx850.exe"
HXFLAGS	= -o tst850.hex -fS

OBJS = main.o  \
	mmc.o  \
	ff.o  \
	cc932.o  \
	uart.o  \
	xprintf.o 

DEP_main_c = "C:\Program Files\NEC Electronics Tools\CA850\E3.11a\inc850\string.h" \
	"C:\Program Files\NEC Electronics Tools\CA850\E3.11a\inc850\stddef.h" \
	C:\user\Prj_V850\tst850\uart.h \
	C:\user\Prj_V850\tst850\v850es.h \
	C:\user\Prj_V850\tst850\integer.h \
	C:\user\Prj_V850\tst850\xprintf.h \
	C:\user\Prj_V850\tst850\diskio.h \
	C:\user\Prj_V850\tst850\ff.h \
	C:\user\Prj_V850\tst850\ffconf.h

DEP_mmc_c = C:\user\Prj_V850\tst850\v850es.h \
	C:\user\Prj_V850\tst850\diskio.h \
	C:\user\Prj_V850\tst850\integer.h

DEP_ff_c = C:\user\Prj_V850\tst850\ff.h \
	C:\user\Prj_V850\tst850\integer.h \
	C:\user\Prj_V850\tst850\ffconf.h \
	C:\user\Prj_V850\tst850\diskio.h \
	"C:\Program Files\NEC Electronics Tools\CA850\E3.11a\inc850\stdarg.h"

DEP_unicode_cc932_c = C:\user\Prj_V850\tst850\ff.h \
	C:\user\Prj_V850\tst850\integer.h \
	C:\user\Prj_V850\tst850\ffconf.h

DEP_uart_c = C:\user\Prj_V850\tst850\uart.h \
	C:\user\Prj_V850\tst850\v850es.h \
	C:\user\Prj_V850\tst850\integer.h

DEP_xprintf_c = C:\user\Prj_V850\tst850\xprintf.h \
	"C:\Program Files\NEC Electronics Tools\CA850\E3.11a\inc850\stdarg.h"

GOAL : C:\user\Prj_V850\tst850\romp.out















main.o : main.c $(DEP_main_c)
	$(CC) $(CFLAGS) -Os -c main.c

mmc.o : mmc.c $(DEP_mmc_c)
	$(CC) $(CFLAGS) -Os -c mmc.c

ff.o : ff.c $(DEP_ff_c)
	$(CC) $(CFLAGS) -Os -c ff.c

cc932.o : unicode\cc932.c $(DEP_unicode_cc932_c)
	$(CC) $(CFLAGS) -Os -c unicode\cc932.c

uart.o : uart.c $(DEP_uart_c)
	$(CC) $(CFLAGS) -Os -c uart.c

xprintf.o : xprintf.c $(DEP_xprintf_c)
	$(CC) $(CFLAGS) -Os -c xprintf.c

startup.o : startup.s $(DEP_STARTUP)
	$(AS) $(ASFLAGS) startup.s

romp.out : $(STARTUP) $(LINKDIR) $(OBJS) $(LIBRARY) $(ROMPCRT)
	$(LD) @tst850.cld
	$(ROMP) $(RPFLAGS) tst850.out
	$(HX) $(HXFLAGS) romp.out

