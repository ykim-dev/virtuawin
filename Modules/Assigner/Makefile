# Wedit Makefile for project Assigner
CFLAGS=-IC:\lcc\include -O
LINKFLAGS=-s -subsystem windows
CC=lcc.exe
TARGET=VWAssigner.exe
OBJS= assigner.obj assigner.res

LIBS=

VWAssigner.exe:	$(OBJS) Makefile
	lcclnk	$(LINKFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Build assigner.res
ASSIGNER_RC=  assignerres.h

assigner.res:	$(ASSIGNER_RC) assigner.rc
	lrc -Ic:\Assigner -IC:\lcc\include  assigner.rc

# Build Assigner.c

Assigner.obj: $(ASSIGNER_C) assigner.c
	$(CC) -c $(CFLAGS) assigner.c

clean: 
	@rm $(OBJS)

all: clean $(TARGET)
