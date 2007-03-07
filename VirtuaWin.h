//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  VirtuaWin.h - Main variable and function definitions.
// 
//  Copyright (c) 1999-2005 Johan Piculell
//  Copyright (c) 2006 VirtuaWin (VirtuaWin@home.se)
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
extern HWND hWnd;             // The handle to VirtuaWin 
extern int screenLeft;	      // the screen dimensions, from VirtuaWin.h
extern int screenRight;	  
extern int screenTop;	  
extern int screenBottom;

extern int curDisabledMod;    // how many disabled modules we have
extern int knockMode;         // mouse edge kncking mode

// logging defines & macros
extern int   vwLogFlag ;
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
void enableMouse(BOOL turnOn) ;
void setMouseKey(void);
void reLoadIcons(void);
void registerAllKeys(void);
void unRegisterAllKeys(void);
void getWorkArea(void);
int  assignWindow(HWND theWin, int theDesk, BOOL force);
int  gotoDesk(int theDesk, BOOL force);
void showHelp(HWND aHWnd, UINT context);

#endif
