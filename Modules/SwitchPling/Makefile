# Wedit Makefile for project SwitchPling
CFLAGS=-Ic:\lcc\include 
CC=lcc.exe
OBJS=\
	SwitchPling.obj 

LIBS=winmm.lib

SwitchPling.exe:	$(OBJS) Makefile
	c:\lcc\bin\lcclnk  -subsystem windows -o SwitchPling.exe $(OBJS) $(LIBS)

# Build SwitchPling.c

SwitchPling.obj: $(SWITCHPLING_C) SwitchPling.c
	$(CC) -c $(CFLAGS) SwitchPling.c

