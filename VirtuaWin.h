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
void assignWindow(HWND theWin, int theDesk, BOOL force);
int  gotoDesk(int theDesk, BOOL force);

#endif

/*
 * $Log$
 * Revision 1.30  2005/03/10 08:02:11  rexkerr
 * Added multi-user support
 *
 * Revision 1.29  2004/12/07 19:18:42  jopi
 * SF1053738, added application icons to the window list
 *
 * Revision 1.28  2004/04/10 10:20:01  jopi
 * Updated to compile with gcc/mingw
 *
 * Revision 1.27  2004/02/28 23:50:26  jopi
 * SF905625 Added module message for changing the sticky state of a window
 *
 * Revision 1.26  2004/02/28 18:54:01  jopi
 * SF904069 Added possibility to choose if sticky should be permanent for all instances of the same classname.
 *
 * Revision 1.25  2004/01/10 11:15:52  jopi
 * Updated copyright for 2004
 *
 * Revision 1.24  2003/09/24 19:26:28  jopi
 * SF770859 Window menu heading will not be displayed if only one meny is used
 *
 * Revision 1.23  2003/06/26 19:56:52  jopi
 * Added module support for assigning a window to specified desktop
 *
 * Revision 1.22  2003/06/24 19:52:05  jopi
 * SF693876 Fixed option to handle XP skinned style taskbars
 *
 * Revision 1.21  2003/04/09 16:47:59  jopi
 * SF710500, removed all the old menu handling code to make menus work the same independently of numner of menus used.
 *
 * Revision 1.20  2003/03/10 20:48:17  jopi
 * Changed so that doubleclick will bring up setup and added a disabled menu item instead.
 *
 * Revision 1.19  2003/01/27 20:23:54  jopi
 * Updated copyright header for 2003
 *
 * Revision 1.18  2002/12/23 15:42:25  jopi
 * Added config options to disable taskbar detection and the alternative hiding technique.
 *
 * Revision 1.17  2002/09/27 16:45:12  Johan Piculell
 * Added mutex protection for the window list
 *
 * Revision 1.16  2002/06/15 11:17:50  Johan Piculell
 * Fixed so that window coordinates are reloaded when resolution is changed, and also so that taskbar location is reloaded if moved.
 *
 * Revision 1.15  2002/06/01 21:15:22  Johan Piculell
 * Multiple fixes by Christian Storm.
 *
 * Revision 1.14  2002/06/01 19:33:33  Johan Piculell
 * *** empty log message ***
 *
 * Revision 1.13  2002/02/14 21:23:41  Johan Piculell
 * Updated copyright header
 *
 * Revision 1.12  2001/12/01 00:05:52  Johan Piculell
 * Added alternative window hiding for troublesome windows like InternetExplorer
 *
 * Revision 1.11  2001/11/12 21:39:41  Johan Piculell
 * Added functionality for disabling the systray icon
 *
 * Revision 1.10  2001/11/12 18:33:42  Johan Piculell
 * Fixed so that user windows are also checked if they are saved as sticky.
 *
 * Revision 1.9  2001/02/05 21:13:08  Administrator
 * Updated copyright header
 *
 * Revision 1.8  2001/01/28 16:26:56  Administrator
 * Configuration behaviour change. It is now possible to test all settings by using apply and all changes will be rollbacked if cancel is pressed
 *
 * Revision 1.7  2001/01/12 18:11:25  Administrator
 * Moved some disk stuff from VirtuaWin to DiskRoutines
 *
 * Revision 1.6  2000/12/11 21:29:47  Administrator
 * Changed version number
 *
 * Revision 1.5  2000/08/28 21:38:37  Administrator
 * Added new functions for menu hot key registration. Fixed bug with needing to have hot keys enabled for menu keys to work and also better error message
 *
 * Revision 1.4  2000/08/19 00:02:13  Administrator
 * Changed version number
 *
 * Revision 1.3  2000/08/18 23:43:08  Administrator
 *  Minor modifications by Matti Jagula <matti@proekspert.ee> List of modifications follows: Added window title sorting in popup menus (Assign, Direct, Sticky) Added some controls to Setup Misc tab and support for calling the popup menus from keyboard.
 *
 * Revision 1.2  2000/08/18 21:41:32  Administrator
 * Added the code again that removes closed windows, this will avoid having closed child windows reappearing again. Also updated the mail adress
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  Administrator
 * Added first time
 *
 */
