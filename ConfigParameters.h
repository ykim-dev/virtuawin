//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K
// 
//  Copyright (c) 1999, 2000 jopi
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

extern HWND hWnd; // The handle to VirtuaWin 

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
extern BOOL mouseModWin;	// mouse warp key
extern BOOL taskBarWarp;        // if removing taskbar height when warping down
extern BOOL saveSticky;         // if we shall save sticky win. on exit
extern BOOL mouseWarpCalled;    // if MouseDll has posted a warp message
extern BOOL refreshOnWarp;      // if we should refresh desktop after switch
extern BOOL stickyKeyRegistered; // if the sticky hot key is registered
extern BOOL crashRecovery;      // Should we use recovery functionality
extern BOOL deskWrap;           // If we want to have desktop cycling
extern BOOL setupOpen;          // if setup dialog is visible
extern BOOL invertY;            // if up/down should be inverted
extern BOOL stickyMenu;         // if sticky window menu should be shown
extern BOOL assignMenu;         // if assign window menu should be shown
extern BOOL directMenu;         // if direct access window menu should be shown
extern BOOL useDeskAssignment;  // if we have desktop assignment 
extern BOOL saveLayoutOnExit;   // save the desktop layout upon exit
extern BOOL assignOnlyFirst;    // only assign the first window to a saved desktop 
extern BOOL cyclingKeysEnabled; // If we are using the cycling hotkeys

extern UINT MOUSEKEY;           // Holds the modifier for enabling mouse warp
extern UINT VW_STICKY;
extern UINT VW_STICKYMOD;
extern UINT hotkey1;
extern UINT hotkey1Mod;
extern UINT hotkey1Win;
extern UINT hotkey2;
extern UINT hotkey2Mod;
extern UINT hotkey2Win;
extern UINT hotkey3;
extern UINT hotkey3Mod;
extern UINT hotkey3Win;
extern UINT hotkey4;
extern UINT hotkey4Mod;
extern UINT hotkey4Win;
extern UINT hotkey5;
extern UINT hotkey5Mod;
extern UINT hotkey5Win;
extern UINT hotkey6;
extern UINT hotkey6Mod;
extern UINT hotkey6Win;
extern UINT hotkey7;
extern UINT hotkey7Mod;
extern UINT hotkey7Win;
extern UINT hotkey8;
extern UINT hotkey8Mod;
extern UINT hotkey8Win;
extern UINT hotkey9;
extern UINT hotkey9Mod;
extern UINT hotkey9Win;
extern UINT hotCycleUp;
extern UINT hotCycleUpMod;
extern UINT hotCycleDown;
extern UINT hotCycleDownMod;

extern int screenWidth;	  // the width of screen
extern int screenHeight;  // the height of screen

extern int curDisabledMod; // how many disabled modules we have

extern LPSTR vwPath;          // holds path to VirtuaWin directory
extern LPSTR vwConfig;        // holds config filename + path
extern LPSTR vwList;          // holds userlist filename + path
extern LPSTR vwHelp;          // holds help filename + path
extern LPSTR vwSticky;        // holds sticky file + path
extern LPSTR vwState;         // holds sticky file + path
extern LPSTR vwLock;          // holds desktop state file + path
extern LPSTR vwModules;       // holds module search criteria
extern LPSTR vwDisabled;      // holds disabled module file + path
extern LPSTR vwWindowsState;  // holds windowsata file name + path

#endif

/*
 * $Log$
 */
