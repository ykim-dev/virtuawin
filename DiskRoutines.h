//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K
// 
//  Copyright (c) 1999, 2000, 2001 jopi
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

void loadFilePaths();
void writeDisabledList(int* theNOfModules, moduleType* theModList);
int loadDisabledModules(disModules* theDisList);
int loadStickyList(stickyType* theStickyList);
void saveStickyWindows(int* theNOfWin, windowType* theWinList);
void saveDesktopState(int* theNOfWin, windowType* theWinList);
void saveDesktopConfiguration(int* theNOfWin, windowType* theWinList);
int loadAssignedList(assignedType* theAssignList);
int loadUserList(userType* theUserList);
void writeConfig();
void readConfig();
BOOL tryToLock();

#endif

/*
 * $Log$
 * Revision 1.2  2001/01/12 18:11:25  jopi
 * Moved some disk stuff from VirtuaWin to DiskRoutines
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  jopi
 * Added first time
 *
 */
