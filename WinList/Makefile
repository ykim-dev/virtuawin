# Wedit Makefile for project WinList
CFLAGS=-IC:\lcc\include -O
LINKFLAGS=-s -subsystem windows
CC=lcc.exe
TARGET=WinList.exe
OBJS= WinList.obj WinList.res

LIBS=

WinList.exe:	$(OBJS) Makefile
	lcclnk	$(LINKFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Build WinList.res
WINLIST_RC=  WinListres.h

WinList.res:	$(WINLIST_RC) WinList.rc
	lrc -Ic:\WinList -IC:\lcc\include  WinList.rc

# Build WinList.c

WinList.obj: $(WINLIST_C) WinList.c
	$(CC) -c $(CFLAGS) WinList.c

clean: 
	@rm $(OBJS)

all: clean $(TARGET)
