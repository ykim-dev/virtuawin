# Wedit Makefile for project Assigner
CFLAGS=-IC:\lcc\include -O
LINKFLAGS=-s -subsystem windows
CC=lcc.exe
TARGET=AutoSwitcher.exe
OBJS= autoswitcher.obj autoswitcher.res

LIBS=

AutoSwitcher.exe:	$(OBJS) Makefile
	lcclnk	$(LINKFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Build autoswitcher.res
AUTOSWITCHER_RC=  autoswitcherres.h

autoswitcher.res:	$(AUTOSWITCHER_RC) autoswitcher.rc
	lrc -Ic:\Autoswitcher -IC:\lcc\include  autoswitcher.rc

# Build Autoswitcher.c

Autoswitcher.obj: $(AUTOSWITCHER_C) autoswitcher.c
	$(CC) -c $(CFLAGS) autoswitcher.c

clean: 
	@rm $(OBJS)

all: clean $(TARGET)
