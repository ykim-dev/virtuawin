# Wedit Makefile for project VirtuaWin
CFLAGS=-IC:\lcc\include -O
LINKFLAGS=-s -subsystem windows
CC=lcc.exe
TARGET=VirtuaWin.exe
OBJS = VirtuaWin.res VirtuaWin.obj DiskRoutines.obj SetupDialog.obj ModuleRoutines.obj

LIBS = shell32.lib

$(TARGET):	$(OBJS) Makefile
	lcclnk $(LINKFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# Build VirtuaWin.res
VIRTUAWIN_RC=\
	C:\lcc\include\windows.h\
	C:\lcc\include\win.h\
	C:\lcc\include\limits.h\
	C:\lcc\include\stdarg.h\
	resource.h\

VirtuaWin.res:	$(VIRTUAWIN_RC) VirtuaWin.rc
	lrc -Ic:\VirtuaWin -IC:\lcc\include  VirtuaWin.rc

# Build VIRTUAWIN.C
VIRTUAWIN_C=\
	VirtuaWin.h\
	Resource.h\
	C:\lcc\include\windows.h\
	C:\lcc\include\win.h\
	C:\lcc\include\limits.h\
	C:\lcc\include\stdarg.h\
	C:\lcc\include\shellapi.h\
	C:\lcc\include\stdio.h\
	C:\lcc\include\_syslist.h\
	C:\lcc\include\stdlib.h\
	C:\lcc\include\stddef.h\
	C:\lcc\include\string.h\
	C:\lcc\include\commctrl.h\
	C:\lcc\include\math.h\
	C:\lcc\include\io.h\
	C:\lcc\include\sys\stat.h\

VirtuaWin.obj: $(VIRTUAWIN_C) VirtuaWin.c 
	$(CC) -c $(CFLAGS) VirtuaWin.c 

DiskRoutines.obj: $(DISKROUTINES_C) DiskRoutines.c
	$(CC) -c $(CFLAGS) DiskRoutines.c 

SetupDialog.obj: $(SETUPDIALOG_C) Setupdialog.c
	$(CC) -c $(CFLAGS) Setupdialog.c 

ModuleRoutines.obj: $(MODULEROUTINES_C) ModuleRoutines.c
	$(CC) -c $(CFLAGS) ModuleRoutines.c 

clean: 
	@rm $(OBJS)

all: clean $(TARGET)
