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

#ifndef _VIRTUAWIN_H_
#define _VIRTUAWIN_H_

// Includes
#include "Resource.h"
#include "ListStructures.h"

// Standard includes
#include <windows.h>
#include <shellapi.h>

// Forward declarations of callbacks
LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);	// main window
BOOL CALLBACK enumWindowsProc(HWND, LPARAM);            // for collecting windows
VOID CALLBACK TimerProc(HWND, UINT, UINT, DWORD);
VOID CALLBACK FlashProc(HWND, UINT, UINT, DWORD);
DWORD WINAPI MouseProc(LPVOID lpParameter);
 
// Forward declarations of functions
void integrateWindow(HWND*);
void initData();
void showAll();
BOOL inWinList(HWND*);
void stepDesk();
int  stepLeft();
int  stepRight();
int  stepUp();
int  stepDown();
int  gotoDesk(int);
int  gotoDeskXY(int, int);
void setIcon(int);
int calculateDesk();
BOOL registerKeys();
BOOL registerHotKeys();
BOOL registerStickyKey();
void unRegisterStickyKey();
BOOL registerCyclingKeys();
void unRegisterCyclingKeys();
BOOL registerMenuHotKey();
void unRegisterMenuHotKey();
void setKeyMod();
void setHotKeyMod();
void packList();
void forceForeground(HWND*);
void findUserWindows();
void loadIcons();
void reLoadIcons();
HMENU createWinList();
void setMouseKey();
WORD hotKey2ModKey(BYTE);
void toggleActiveSticky();
void recoverWindows();
BOOL checkIfSavedSticky(HWND* hwnd);
void shutDown();
void showSetup();
int checkIfAssignedDesktop(HWND*);
BOOL safeShowWindow(HWND*, int);
void warningIcon();
BOOL checkMouseState();
HMENU createSortedWinList(int);
void enableMouse(BOOL);

// Variables
HWND topWindow;        // holds the top window on a desktop
HWND currentActive;    // holds the current active window
BOOL userDefinedWin = FALSE;  // if we have any user defined

HANDLE mouseThread;       // Handle to the mouse thread
BOOL mouseEnabled = TRUE; // Status of the mouse thread, always running at startup 

static int curAssigned = 0;   // how many predefined desktop belongings we have (saved)
static int curSticky = 0;     // how many stickywindows we have (saved)
static int curUser = 0;       // how many user applications we have

static UINT MODKEY;	      // Holds the switch key modifiers

static char appName[] = "VirtuaWin 2.5";   // application name

ATOM stickyKey;
ATOM vwLeft;
ATOM vwRight;
ATOM vwUp;
ATOM vwDown;
ATOM vw1;
ATOM vw2;
ATOM vw3;
ATOM vw4;
ATOM vw5;
ATOM vw6;
ATOM vw7;
ATOM vw8;
ATOM vw9;
ATOM cyclingKeyUp;
ATOM cyclingKeyDown;
ATOM vwMenu;

BOOL keysRegistred = FALSE;	// if the switch keys are registrered
BOOL hotKeysRegistred = FALSE;	// if the switch hot keys are registrered
BOOL enabled = TRUE;		// if VirtuaWin enabled or not
BOOL isDragging = FALSE;	// if we are currently dragging a window
BOOL cyclingKeysRegistered = FALSE; // if the cycling keys are registrered
BOOL menuHotKeyRegistered = FALSE;  // if the menu hotkey is registered

int taskBarHeight;		// the height of the taskbar

HINSTANCE hInst;		// current instance
HWND hWnd;			// handle to VirtuaWin
HWND releaseHnd;		// handle to the window to give focus on release focus

// vector holding icon handles for the systray
HICON icons[10]; // 9 + 1 icons
NOTIFYICONDATA nIconD;

// Config parameters, see ConfigParameters.h for descriptions
int saveInterval = 0;
int nOfModules = 0;  
int nWin = 0;        
int currentDeskX = 1;
int currentDeskY = 1;
int currentDesk = 1; 
int nDesksX = 2;     
int nDesksY = 2;     
int warpLength = 20; 
int warpMultiplier = 0;
int configMultiplier = 1;
BOOL noMouseWrap = FALSE;
BOOL mouseEnable = TRUE; 
BOOL useMouseKey = FALSE;
BOOL keyEnable = TRUE;		
BOOL hotKeyEnable = FALSE;      
BOOL releaseFocus = FALSE;	
BOOL keepActive = TRUE;	        
BOOL minSwitch = TRUE;		
BOOL modAlt = FALSE;		
BOOL modShift = FALSE;		
BOOL modCtrl = FALSE;		
BOOL modWin = TRUE;		
BOOL hotModAlt = FALSE;		
BOOL hotModShift = FALSE;	
BOOL hotModCtrl = FALSE;	
BOOL hotModWin = FALSE;		
BOOL mouseModAlt = FALSE;	
BOOL mouseModShift = FALSE;	
BOOL mouseModCtrl = FALSE;	
BOOL mouseModWin = FALSE;	
BOOL taskBarWarp = FALSE;       
BOOL saveSticky = FALSE;        
BOOL refreshOnWarp = FALSE;     
BOOL stickyKeyRegistered = FALSE;
BOOL crashRecovery = TRUE;      
BOOL deskWrap = FALSE;          
BOOL setupOpen = FALSE;         
BOOL invertY = FALSE;           
BOOL stickyMenu = TRUE;         
BOOL assignMenu = FALSE;
BOOL directMenu = FALSE;
BOOL useDeskAssignment = FALSE;
BOOL saveLayoutOnExit = FALSE;
BOOL assignOnlyFirst = FALSE;
BOOL cyclingKeysEnabled = FALSE;

UINT MOUSEKEY;                  
UINT VW_STICKY = 0;
UINT VW_STICKYMOD = 0;
UINT hotkey1 = 0;
UINT hotkey1Mod = 0;
UINT hotkey1Win = 0;
UINT hotkey2 = 0;
UINT hotkey2Mod = 0;
UINT hotkey2Win = 0;
UINT hotkey3 = 0;
UINT hotkey3Mod = 0;
UINT hotkey3Win = 0;
UINT hotkey4 = 0;
UINT hotkey4Mod = 0;
UINT hotkey4Win = 0;
UINT hotkey5 = 0;
UINT hotkey5Mod = 0;
UINT hotkey5Win = 0;
UINT hotkey6 = 0;
UINT hotkey6Mod = 0;
UINT hotkey6Win = 0;
UINT hotkey7 = 0;
UINT hotkey7Mod = 0;
UINT hotkey7Win = 0;
UINT hotkey8 = 0;
UINT hotkey8Mod = 0;
UINT hotkey8Win = 0;
UINT hotkey9 = 0;
UINT hotkey9Mod = 0;
UINT hotkey9Win = 0;
UINT hotCycleUp = 0;
UINT hotCycleUpMod = 0;
UINT hotCycleDown = 0;
UINT hotCycleDownMod = 0;
UINT hotkeyMenu = 0;
UINT hotkeyMenuMod = 0;
UINT hotkeyMenuWin = 0;
UINT hotkeyMenuEn = 0;

int screenWidth;
int screenHeight;
int curDisabledMod = 0; 

// Paths and filenames to various working/config files
// See ConfigParameters.h for descriptions
LPSTR vwPath;
LPSTR vwConfig;
LPSTR vwList;
LPSTR vwHelp;
LPSTR vwSticky;
LPSTR vwState; 
LPSTR vwLock; 
LPSTR vwModules;
LPSTR vwDisabled; 
LPSTR vwWindowsState;

#endif

/*
 * $Log$
 * Revision 1.7  2001/01/12 18:11:25  jopi
 * Moved some disk stuff from VirtuaWin to DiskRoutines
 *
 * Revision 1.6  2000/12/11 21:29:47  jopi
 * Changed version number
 *
 * Revision 1.5  2000/08/28 21:38:37  jopi
 * Added new functions for menu hot key registration. Fixed bug with needing to have hot keys enabled for menu keys to work and also better error message
 *
 * Revision 1.4  2000/08/19 00:02:13  jopi
 * Changed version number
 *
 * Revision 1.3  2000/08/18 23:43:08  jopi
 *  Minor modifications by Matti Jagula <matti@proekspert.ee> List of modifications follows: Added window title sorting in popup menus (Assign, Direct, Sticky) Added some controls to Setup Misc tab and support for calling the popup menus from keyboard.
 *
 * Revision 1.2  2000/08/18 21:41:32  jopi
 * Added the code again that removes closed windows, this will avoid having closed child windows reappearing again. Also updated the mail adress
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  jopi
 * Added first time
 *
 */
