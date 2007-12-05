//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  VirtuaWin.h - Main variable and function definitions.
// 
//  Copyright (c) 1999-2005 Johan Piculell
//  Copyright (c) 2006-2007 VirtuaWin (VirtuaWin@home.se)
// 
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
//  USA.
//

#ifndef _VIRTUAWIN_H_
#define _VIRTUAWIN_H_

// Standard includes
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "Defines.h"

// externally accessible variables
extern HWND hWnd;                                 // The handle to VirtuaWin 
extern int screenLeft;	                          // the screen dimensions, from VirtuaWin.h
extern int screenRight;	  
extern int screenTop;	  
extern int screenBottom;

extern int curDisabledMod;                        // how many disabled modules we have

extern windowType winList[vwWINDOW_MAX];          // list for holding windows
extern vwHotkey   hotkeyList[vwHOTKEY_MAX];       // list for holding hotkeys
extern moduleType moduleList[MAXMODULES];         // list that holds modules
extern disModules disabledModules[MAXMODULES*2];  // list with disabled modules

extern vwWindowMatch *userList;                   // list for holding user added applications
extern vwWindowMatch *stickyList;                 // list with saved sticky windows
extern vwWindowMatch *trickyList;                 // list with saved tricky windows
extern vwWindowMatch *assignedList;               // list with all windows that have a predefined desktop

// logging defines & macros
extern vwUByte vwLogFlag ;
extern FILE *vwLogFile ;

#define vwLogEnabled()        (vwLogFile != NULL)
#define vwLogBasic(a)         (vwLogEnabled() ? vwLogPrint a :0)

#ifdef vwLOG_VERBOSE
#define vwLogVerbose(a)       (vwLogEnabled() ? vwLogPrint a :0)
#else
#define vwLogVerbose(a)
#endif

// Forward declarations of functions
void vwLogPrint(const TCHAR *format, ...) ;
void enableMouse(int turnOn) ;
void setMouseKey(void);
void vwIconReload(void);
void vwHotkeyRegister(void);
void vwHotkeyUnregister(void);
void getWorkArea(void);
int  assignWindow(HWND theWin, int theDesk, vwUByte follow, vwUByte force, vwUByte setActive);
int  gotoDesk(int theDesk, vwUByte force);
void showHelp(HWND aHWnd, UINT context);

#endif
