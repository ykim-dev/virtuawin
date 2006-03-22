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

#include <windows.h>
#include "ListStructures.h"

#ifndef _DISKROUTINES_H_
#define _DISKROUTINES_H_

typedef enum { vwLIST, vwHELP, vwTRICKY, vwMODULES, vwCONFIG, vwSTICKY, vwDISABLED, vwWINDOWS_STATE, vwFILE_COUNT } eFileNames;
extern char *VirtuaWinPath ;
extern char *UserAppPath ;

int  GetFilename(eFileNames filetype, char* outStr);
int  loadDisabledModules(disModules* theDisList);
void saveDisabledList(int theNOfModules, moduleType* theModList);
int  loadTrickyList(stickyType *theTrickyList);
int  loadStickyList(stickyType *theStickyList);
void saveStickyWindows(int theNOfWin, windowType* theWinList);
int  loadAssignedList(assignedType* theAssignList);
void saveAssignedList(int theNOfWin, windowType* theWinList);
int  loadUserList(userType *theUserList);
void writeConfig(void);
void readConfig(void);
BOOL tryToLock(void);
void clearLock(void);

#endif

/*
 * $Log$
 * Revision 1.11  2005/03/10 08:02:11  rexkerr
 * Added multi-user support
 *
 * Revision 1.10  2004/04/10 14:33:14  jopi
 * Updated for gcc/mingw
 *
 * Revision 1.9  2004/01/10 11:15:52  jopi
 * Updated copyright for 2004
 *
 * Revision 1.8  2003/01/27 20:23:53  jopi
 * Updated copyright header for 2003
 *
 * Revision 1.7  2002/12/29 15:20:36  jopi
 * Fixed copyright info.
 *
 * Revision 1.6  2002/02/14 21:23:39  jopi
 * Updated copyright header
 *
 * Revision 1.5  2001/12/01 00:05:53  jopi
 * Added alternative window hiding for troublesome windows like InternetExplorer
 *
 * Revision 1.4  2001/11/12 18:23:05  jopi
 * Added support for classnames that contains spaces which will fix some
 * problems with desktop state save and sticky save.
 *
 * Revision 1.3  2001/02/05 21:13:07  jopi
 * Updated copyright header
 *
 * Revision 1.2  2001/01/12 18:11:25  jopi
 * Moved some disk stuff from VirtuaWin to DiskRoutines
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  jopi
 * Added first time
 *
 */
