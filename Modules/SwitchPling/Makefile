# MINGW Makefile for project SwitchPling
OSTYPE   = $(shell uname -msr)

ifeq ($(findstring CYGWIN,$(OSTYPE)),CYGWIN)
CC      = i686-pc-mingw32-gcc
CFLAGS	= -Wall -O2
LDFLAGS	= -mwindows -O2
RC      = i686-pc-mingw32-windres
endif

ifeq ($(findstring MINGW32,$(OSTYPE)),MINGW32)
CC      = gcc
CFLAGS	= -Wall -O2
LDFLAGS	= -mwindows -O2
RC      = windres 
endif

ifeq ($(findstring Linux,$(OSTYPE)),Linux)
CC      = i586-mingw32msvc-gcc
CFLAGS  = -Wall -O2
LDFLAGS = -mwindows -O2
RC	= i586-mingw32msvc-windres
endif

TARGET	= SwitchPling.exe
OBJS	= SwitchPling.o
LIBS	= -lwinmm

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

clean: 
	rm -f $(OBJS)

all:    clean $(TARGET)
