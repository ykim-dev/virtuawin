# MINGW Makefile for project AutoSwitcher
OSTYPE   = $(shell uname -msr)

ifeq ($(findstring CYGWIN,$(OSTYPE)),CYGWIN)
CC      = gcc
CFLAGS	= -mno-cygwin -Wall -O2
LDFLAGS	= -mwindows -mno-cygwin -O2
RC      = windres 
STRIP	= strip
endif

ifeq ($(findstring MINGW32,$(OSTYPE)),MINGW32)
CC      = gcc
CFLAGS	= -Wall -O2
LDFLAGS	= -mwindows -O2
RC      = windres 
STRIP	= strip
endif

ifeq ($(findstring Linux,$(OSTYPE)),Linux)
CC      = i586-mingw32msvc-gcc
CFLAGS  = -Wall -O2
LDFLAGS = -mwindows -O2
RC	= i586-mingw32msvc-windres
STRIP	= strip
endif

TARGET	= AutoSwitcher.exe
OBJS	= autoswitcher.o autoswitcher.coff
LIBS	= -lcomctl32

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS) 
	$(STRIP) $(TARGET)

autoswitcher.coff: autoswitcher.rc
	$(RC) --input-format rc --output-format coff -o $@ -i $<

clean: 
	rm -f $(OBJS)

all:    clean $(TARGET)
