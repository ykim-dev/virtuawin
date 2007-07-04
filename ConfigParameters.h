//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  ConfigParameters.h - Declaration of configured variables
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

#ifndef _CONFIGPARAMETERS_H_
#define _CONFIGPARAMETERS_H_

extern HWND setupHWnd;   // handle to the setup dialog, NULL if not open
extern BOOL setupOpen;         

extern int saveInterval; // Number of time loops before saving desktop state
extern int nOfModules;   // Number of loaded modules
extern int nWin;         // Number of windows in the system
extern int currentDeskX; // Current desktop x wise
extern int currentDeskY; // Current desktop y wise
extern int currentDesk;  // Current desktop
extern int nDesks;              // indicates the total number of desks (nDesksX * nDesksY)
extern int nDesksX;             // indicates the number of desks wide the virtual area is
extern int nDesksY;             // indicates the number of desks tall the virtual area is
extern int warpLength;          // How far to jump into new desktop
extern int mouseDelay;          // Mouse change delay 50ms*mouseDelay 
extern int preserveZOrder;      // Should we preserve the window Z order
extern int hiddenWindowAct;     // Hidden window activation action
extern BOOL noMouseWrap;        // if we don't want to move the mouse pointer after switch
extern BOOL useMouseKey;        // if user must use a modify key to warp with mouse
extern BOOL keyEnable;		// key support
extern BOOL releaseFocus;	// release focus on switch
extern BOOL minSwitch;		// if we should switch minimized windows
extern UINT mouseEnable;        // mouse support
extern UINT modAlt;		// switch key
extern UINT modShift;		// switch key
extern UINT modCtrl;		// switch key
extern UINT modWin;		// switch key
extern UINT mouseModAlt;	// mouse warp key
extern UINT mouseModShift;	// mouse warp key
extern UINT mouseModCtrl;	// mouse warp key
extern BOOL taskBarWarp;        // if removing taskbar height when warping down
extern BOOL saveSticky;         // if we shall save sticky win. on exit
extern BOOL mouseWarpCalled;    // if MouseDll has posted a warp message
extern BOOL refreshOnWarp;      // if we should refresh desktop after switch
extern BOOL stickyKeyRegistered; // if the sticky hot key is registered
extern BOOL crashRecovery;      // Should we use recovery functionality
extern BOOL deskWrap;           // If we want to have desktop cycling
extern BOOL invertY;            // if up/down should be inverted
extern short assignMenu;        // if assign window list menu should be shown
extern short accessMenu;        // if access window list menu should be shown
extern short showMenu;          // if show window list menu should be shown
extern short stickyMenu;        // if sticky window list menu should be shown
extern short compactMenu;       // if window list menu should be compact
extern BOOL useDeskAssignment;  // if we have desktop assignment 
extern BOOL saveLayoutOnExit;   // save the desktop layout upon exit
extern BOOL assignOnlyFirst;    // only assign the first window to a saved desktop 
extern BOOL assignImmediately;  // move an assigned window immediately
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

extern TCHAR *desktopName[vwDESKTOP_SIZE];
extern UINT deskHotkey[vwDESKTOP_SIZE];
extern UINT deskHotkeyMod[vwDESKTOP_SIZE];
extern UINT deskHotkeyWin[vwDESKTOP_SIZE];

extern UINT hotkeyWListEn;
extern UINT hotkeyWList;
extern UINT hotkeyWListMod;
extern UINT hotkeyWListWin;
extern UINT hotkeyWMenuEn;
extern UINT hotkeyWMenu;
extern UINT hotkeyWMenuMod;
extern UINT hotkeyWMenuWin;
extern UINT hotkeyStickyEn;
extern UINT hotkeySticky;
extern UINT hotkeyStickyMod;
extern UINT hotkeyStickyWin;
extern UINT hotkeyDismissEn;
extern UINT hotkeyDismiss;
extern UINT hotkeyDismissMod;
extern UINT hotkeyDismissWin;

#endif
