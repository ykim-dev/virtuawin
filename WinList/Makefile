# Wedit Makefile for project WinList
CFLAGS=-Ic:\lcc\include 
CC=lcc.exe
OBJS= WinList.obj WinList.res

LIBS=

WinList.exe:	$(OBJS) Makefile
	lcclnk  -s -subsystem windows -o WinList.exe $(OBJS) $(LIBS)

# Build WinList.res
WINLIST_RC=  WinListres.h

WinList.res:	$(WINLIST_RC) WinList.rc
	lrc -Ic:\WinList -IC:\lcc\include  WinList.rc

# Build WinList.c

WinList.obj: $(WINLIST_C) WinList.c
	$(CC) -c $(CFLAGS) WinList.c


