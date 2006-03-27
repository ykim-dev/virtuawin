# MINGW Makefile for project VirtuaWin
OSTYPE   = $(shell uname -msr)

ifeq ($(findstring CYGWIN,$(OSTYPE)),CYGWIN)
CC      = gcc
CFLAGS	= -mno-cygwin -Wall -O2 -DNDEBUG
CFLAGSD = -mno-cygwin -Wall -g
LDFLAGS	= -mwindows -mno-cygwin -O2
LDFLAGSD= -mwindows -mno-cygwin -g
RC      = windres 
STRIP	= strip
endif

ifeq ($(findstring MINGW32,$(OSTYPE)),MINGW32)
CC      = gcc
CFLAGS	= -Wall -O2 -DNDEBUG
CFLAGSD = -Wall -g
LDFLAGS	= -mwindows -O2
LDFLAGSD= -mwindows -g
RC      = windres 
STRIP	= strip
endif

ifeq ($(findstring Linux,$(OSTYPE)),Linux)
CC      = i586-mingw32msvc-gcc
CFLAGS  = -Wall -O2 -DNDEBUG
CFLAGSD = -Wall -g
LDFLAGS = -mwindows -O2
LDFLAGSD= -mwindows -g
RC	= i586-mingw32msvc-windres
STRIP	= strip
endif

SRC	= VirtuaWin.c DiskRoutines.c SetupDialog.c ModuleRoutines.c
LIBS	= -lshell32 -luser32 -lgdi32 -lcomctl32
COFFS   = VirtuaWin.coff

TARGET	= VirtuaWin.exe
OBJS    = $(SRC:.c=.o)

TARGETD	= VirtuaWinD.exe
OBJSD   = $(SRC:.c=.od)

.SUFFIXES: .rc .res .coff .c .o .od
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<
.c.od:
	$(CC) $(CFLAGSD) -c -o $@ $<

$(TARGET): $(OBJS) $(COFFS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(COFFS) $(LIBS)
	$(STRIP) $(TARGET)

$(TARGETD): $(OBJSD) $(COFFS)
	$(CC) $(LDFLAGSD) -o $(TARGETD) $(OBJSD) $(COFFS) $(LIBS)

VirtuaWin.coff: VirtuaWin.rc
	$(RC) --input-format rc --output-format coff -o $@ -i $<

clean: 
	rm -f $(OBJS) $(OBJSD) $(COFFS)

all:    clean $(TARGET)

alld:   clean $(TARGETD)
