# Wedit Makefile for project module
OSTYPE   = $(shell uname -msr)

ifeq ($(findstring CYGWIN,$(OSTYPE)),CYGWIN)
CC      = gcc
CFLAGS	= -mno-cygwin -Wall -O2 -DNDEBUG
CFLAGSD = -mno-cygwin -Wall -g
LDFLAGS	= -mwindows -mno-cygwin -O2
LDFLAGS	= -mwindows -mno-cygwin -g
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

.SUFFIXES: .rc .res .coff .c .o .od

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

PRG=Module.exe
SRC=Module.c
OBJ=$(SRC:.c=.o)

$(PRG):	$(OBJ) Makefile
	$(CC) $(LDFLAGS) -o $(PRG) $(OBJ) $(LIBS)
	$(STRIP) $(PRG)

clean: 
	rm -f $(OBJ)

all:    clean $(PRG)
