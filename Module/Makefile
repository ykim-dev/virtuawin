# Wedit Makefile for project module
CFLAGS=-Ic:\lcc\include 
CC=lcc.exe
OBJS=\
	module.obj

LIBS=

module.exe:	$(OBJS) Makefile
	c:\lcc\bin\lcclnk  -subsystem windows -o module.exe $(OBJS) $(LIBS)

# Build module.c

module.obj: $(MODULE_C) module.c
	$(CC) -c $(CFLAGS) module.c

