//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K/XP
// 
//  Copyright (c) 1999-2003, 2004 Johan Piculell
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

// externally accessible variables
extern HWND hWnd; // The handle to VirtuaWin 
extern int screenRight;	  // the width of screen, from VirtuaWin.h
extern int screenBottom;  // the height of screen, from VirtuaWin.h

extern int curDisabledMod; // how many disabled modules we have
extern int taskbarOffset;  // Default 3, 0 if XP skinned taskbar is used.

// Forward declarations of functions
void setMouseKey(void);
void reLoadIcons(void);
void registerAllKeys(void);
void unRegisterAllKeys(void);
void getTaskbarLocation(void);
int  assignWindow(HWND theWin, int theDesk, BOOL force);
int  gotoDesk(int theDesk, BOOL force);
void showHelp(HWND aHWnd);

#endif
