//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  DiskRoutines.h - Disk routine function definitions.
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

#include <windows.h>
#include "ListStructures.h"

#ifndef _DISKROUTINES_H_
#define _DISKROUTINES_H_

typedef enum { vwMODULES, vwHELP, vwCONFIG, vwLIST, vwTRICKY, vwSTICKY, vwDISABLED, vwWINDOWS_STATE, vwFILE_COUNT } eFileNames;

extern TCHAR *VirtuaWinPath ;
extern TCHAR *UserAppPath ;
#ifdef _UNICODE
extern char *VirtuaWinPathStr ;
extern char *UserAppPathStr ;
#else
#define VirtuaWinPathStr VirtuaWinPath
#define UserAppPathStr   UserAppPath
#endif

void GetFilename(eFileNames filetype, int location, TCHAR *outStr);
int  loadDisabledModules(disModules *theDisList);
void saveDisabledList(int theNOfModules, moduleType* theModList);
int  loadTrickyList(vwWindowMatch *theTrickyList);
int  loadStickyList(vwWindowMatch *theStickyList);
void saveStickyWindows(int theNOfWin, windowType *theWinList);
int  loadAssignedList(vwWindowMatch *theAssignList);
void saveAssignedList(int theNOfWin, windowType *theWinList);
int  loadUserList(vwWindowMatch *theUserList);
void writeConfig(void);
void readConfig(void);

#endif
