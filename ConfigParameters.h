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

#ifndef _CONFIGPARAMETERS_H_
#define _CONFIGPARAMETERS_H_

extern int saveInterval; // Number of time loops before saving desktop state
extern int nOfModules;   // Number of loaded modules
extern int nWin;         // Number of windows in the system
extern int currentDeskX; // Current desktop x wise
extern int currentDeskY; // Current desktop y wise
extern int currentDesk;  // Current desktop
extern int nDesksX;      // indicates the number of desks wide the virtual area is
extern int nDesksY;      // indicates the number of desks tall the virtual area is
extern int warpLength;    // How far to jump into new desktop
extern int warpMultiplier; // Counts the emouse delay
extern int configMultiplier;   // Mouse warp delay 250ms*warpMultiplier 
extern BOOL noMouseWrap;   // if we don't want to move the mouse pointer after switch
extern BOOL mouseEnable;    // mouse support
extern BOOL useMouseKey;   // if user must use a modify key to warp with mouse
extern BOOL keyEnable;		// key support
extern BOOL hotKeyEnable;      // hot key support
extern BOOL releaseFocus;	// release focus on switch
extern BOOL keepActive;	        // if active windows will be remembered
extern BOOL minSwitch;		// if we should switch minimized windows
extern BOOL modAlt;		// switch key
extern BOOL modShift;		// switch key
extern BOOL modCtrl;		// switch key
extern BOOL modWin;		// switch key
extern BOOL hotModAlt;		// hot switch key
extern BOOL hotModShift;	// hot switch key
extern BOOL hotModCtrl;	        // hot switch key
extern BOOL hotModWin;		// hot switch key
extern BOOL mouseModAlt;	// mouse warp key
extern BOOL mouseModShift;	// mouse warp key
extern BOOL mouseModCtrl;	// mouse warp key
extern BOOL taskBarWarp;        // if removing taskbar height when warping down
extern BOOL saveSticky;         // if we shall save sticky win. on exit
extern BOOL mouseWarpCalled;    // if MouseDll has posted a warp message
extern BOOL refreshOnWarp;      // if we should refresh desktop after switch
extern BOOL stickyKeyRegistered; // if the sticky hot key is registered
extern BOOL crashRecovery;      // Should we use recovery functionality
extern BOOL deskWrap;           // If we want to have desktop cycling
extern BOOL setupOpen;          // if setup dialog is visible
extern BOOL invertY;            // if up/down should be inverted
extern short stickyMenu;        // if sticky window menu should be shown
extern short assignMenu;        // if assign window menu should be shown
extern short directMenu;        // if direct access window menu should be shown
extern BOOL useDeskAssignment;  // if we have desktop assignment 
extern BOOL saveLayoutOnExit;   // save the desktop layout upon exit
extern BOOL assignOnlyFirst;    // only assign the first window to a saved desktop 
extern BOOL displayTaskbarIcon; // Should we display the systray icon
extern BOOL noTaskbarCheck;     // Should we skip the taskbar search
extern BOOL trickyWindows;      // Use the alternate hiding technique
extern BOOL permanentSticky;    // If a sticky classname should be permanent

extern BOOL cyclingKeysEnabled; // If we are using the cycling hotkeys
extern UINT hotCycleUp;
extern UINT hotCycleUpMod;
extern UINT hotCycleUpWin;
extern UINT hotCycleDown;
extern UINT hotCycleDownMod;
extern UINT hotCycleDownWin;

extern UINT deskHotkey[MAXDESK];
extern UINT deskHotkeyMod[MAXDESK];
extern UINT deskHotkeyWin[MAXDESK];

extern UINT hotkeyMenuEn;
extern UINT hotkeyMenu;
extern UINT hotkeyMenuMod;
extern UINT hotkeyMenuWin;
extern UINT hotkeyStickyEn;
extern UINT hotkeySticky;
extern UINT hotkeyStickyMod;
extern UINT hotkeyStickyWin;

#endif

/*
 * $Log$
 * Revision 1.15  2005/03/10 08:02:10  rexkerr
 * Added multi-user support
 *
 * Revision 1.14  2004/02/28 18:54:01  jopi
 * SF904069 Added possibility to choose if sticky should be permanent for all instances of the same classname.
 *
 * Revision 1.13  2004/01/10 11:15:52  jopi
 * Updated copyright for 2004
 *
 * Revision 1.12  2003/09/24 19:26:28  jopi
 * SF770859 Window menu heading will not be displayed if only one meny is used
 *
 * Revision 1.11  2003/06/24 19:49:08  jopi
 * SF693876 Fixed option to handle XP skinned style taskbars
 *
 * Revision 1.10  2003/01/27 20:23:52  jopi
 * Updated copyright header for 2003
 *
 * Revision 1.9  2002/12/23 15:42:30  jopi
 * Added config options to disable taskbar detection and the alternative hiding technique.
 *
 * Revision 1.8  2002/06/01 21:15:23  Johan Piculell
 * Multiple fixes by Christian Storm.
 *
 * Revision 1.7  2002/06/01 19:33:33  Johan Piculell
 * *** empty log message ***
 *
 * Revision 1.6  2002/02/14 21:23:38  Johan Piculell
 * Updated copyright header
 *
 * Revision 1.5  2001/12/01 00:05:52  Johan Piculell
 * Added alternative window hiding for troublesome windows like InternetExplorer
 *
 * Revision 1.4  2001/11/12 21:39:15  Johan Piculell
 * Added functionality for disabling the systray icon
 *
 * Revision 1.3  2001/02/05 21:13:07  Administrator
 * Updated copyright header
 *
 * Revision 1.2  2000/08/18 23:43:07  Administrator
 *  Minor modifications by Matti Jagula <matti@proekspert.ee> List of modifications follows: Added window title sorting in popup menus (Assign, Direct, Sticky) Added some controls to Setup Misc tab and support for calling the popup menus from keyboard.
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  Administrator
 * Added first time
 *
 */
