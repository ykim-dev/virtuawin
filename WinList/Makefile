# Wedit Makefile for project WinList
CFLAGS=-Ic:\lcc\include 
CC=lcc.exe
OBJS= WinList.obj\
	WinList.res

LIBS=

WinList.exe:	$(OBJS) Makefile
	lcclnk  -subsystem windows -o WinList.exe $(OBJS) $(LIBS)

# Build WinList.c

WinList.obj: $(WINLIST_C) WinList.c
	$(CC) -c $(CFLAGS) WinList.c

# Build WinList.res
WINLIST_RC=\
	WinListres.h

Winlist.res:	$(WINLIST_RC) WinList.rc
	lrc -Ic:\WinList -IC:\lcc\include  WinList.rc
