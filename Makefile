# VirtuaWin (virtuawin.sourceforge.net)
# Makefile - VirtuaWin make file for MinGW & Cygwin GNU GCC.
#
# Copyright (c) 2006-2012 VirtuaWin (VirtuaWin@home.se)
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 675 Mass Ave, Cambridge, MA 02139, USA.
#
##############################################################################

OSTYPE   = $(shell uname -msr)

ifeq ($(findstring CYGWIN,$(OSTYPE)),CYGWIN)
CC      = i686-pc-mingw32-gcc
CFLAGS	= -Wall -O2 -DNDEBUG
CFLAGSD = -Wall -g
LDFLAGS	= -mwindows -O2
LDFLAGSD= -mwindows -g
DLLFLAGS= -mwindows -O2 -shared
RC      = i686-pc-mingw32-windres 
STRIP	= strip
endif

ifeq ($(findstring MINGW32,$(OSTYPE)),MINGW32)
CC      = gcc
CFLAGS	= -Wall -O2 -DNDEBUG
CFLAGSD = -Wall -g
LDFLAGS	= -mwindows -O2
LDFLAGSD= -mwindows -g
DLLFLAGS= -mwindows -O2 -shared
RC      = windres 
STRIP	= strip
endif

ifeq ($(findstring Linux,$(OSTYPE)),Linux)
CC      = i586-mingw32msvc-gcc
CFLAGS  = -Wall -O2 -DNDEBUG
CFLAGSD = -Wall -g
LDFLAGS = -mwindows -O2
LDFLAGSD= -mwindows -g
DLLFLAGS= -mwindows -O2 -shared
RC	= i586-mingw32msvc-windres
STRIP	= strip
endif

ifeq ($(vwUNICODE),1)
CUCDEFS = -DUNICODE -D_UNICODE
endif
ifeq ($(vwVERBOSED),1)
CVDDEFS = -DvwLOG_VERBOSE
endif

SRC	= VirtuaWin.c DiskRoutines.c SetupDialog.c WinRuleDialog.c ModuleRoutines.c
HEADERS = VirtuaWin.h Resource.h Messages.h \
	  DiskRoutines.h Defines.h ConfigParameters.h vwCommands.def
COFFS   = VirtuaWin.coff
OBJRES  = VirtuaWin.res
LIBS	= -lshell32 -luser32 -lgdi32 -ladvapi32 -lcomctl32

TARGET	= VirtuaWin.exe
OBJS    = $(SRC:.c=.o)

TARGETD	= VirtuaWinD.exe
OBJSD   = $(SRC:.c=.od)

HOOKTGT = vwHook.dll
HOOKSRC = vwHook.c
HOOKOBJ = $(HOOKSRC:.c=.o)
HOOKCFF = vwHook.coff
HOOKRES = vwHook.res

.SUFFIXES: .rc .res .coff .c .o .od
.c.o:
	$(CC) $(CFLAGS) $(CVDDEFS) $(CUCDEFS) -c -o $@ $<
.c.od:
	$(CC) $(CFLAGSD) $(CVDDEFS) $(CUCDEFS) -c -o $@ $<

all:    $(TARGET) $(HOOKTGT)

alld:   $(TARGETD) $(HOOKTGT)

$(TARGET): $(OBJS) $(COFFS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS) $(COFFS) $(LIBS)
	$(STRIP) $(TARGET)

$(TARGETD): $(OBJSD) $(COFFS)
	$(CC) $(LDFLAGSD) -o $(TARGETD) $(OBJSD) $(COFFS) $(LIBS)

VirtuaWin.coff: VirtuaWin.rc
	$(RC) $(CUCDEFS) --input-format rc --output-format coff -o $@ -i $<

$(HOOKCFF): vwHook.rc
	$(RC) $(CUCDEFS) --input-format rc --output-format coff -o $@ -i $<

$(HOOKTGT): $(HOOKOBJ) $(HOOKCFF)
	$(CC) $(DLLFLAGS) -o $(HOOKTGT) $(HOOKOBJ) $(HOOKCFF) $(LIBS)
	$(STRIP) $(HOOKTGT)

clean: 
	rm -f $(OBJS) $(OBJSD) $(COFFS) $(HOOKOBJ) $(HOOKCFF)

all_clean:   clean all

alld_clean:  clean alld


spotless: clean 
	rm -f $(TARGET) $(TARGETD) $(OBJRES) $(HOOKRES) $(HOOKTGT) vc60.pch

# Dependancies
$(OBJS):  $(HEADERS)
$(OBJSD): $(HEADERS)
$(COFFS): $(HEADERS) VirtuaWin.exe.manifest
$(HOOKOBJ):$(HEADERS)
