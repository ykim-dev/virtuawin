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

// Includes
#include "VirtuaWin.h"
#include "DiskRoutines.h"
#include "SetupDialog.h"
#include "ConfigParameters.h"
#include "Messages.h"
#include "Resource.h"
#include "ModuleRoutines.h"

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commctrl.h>
#include <math.h>
#include <time.h>
#include <Psapi.h>

#define USE_SHOWWIN 1

#define calculateDesk(x,y) (((y) * nDesksX) - (nDesksX - (x)))

// Variables
HWND hWnd;		  // handle to VirtuaWin
HANDLE hMutex;
BOOL taskbarFixRequired;  // TRUE if tricky window tasks need to be continually hidden
BOOL mouseEnabled=TRUE;   // Status of the mouse thread, always running at startup 
HANDLE mouseThread;       // Handle to the mouse thread
int curAssigned = 0;      // how many predefined desktop belongings we have (saved)
int curSticky = 0;        // how many stickywindows we have (saved)
int curTricky = 0;        // how many tricky windows we have (saved)
int curUser = 0;          // how many user applications we have

int screenLeft;
int screenRight;
int screenTop;
int screenBottom;

UINT RM_Shellhook;

BOOL keysRegistred = FALSE;	// if the switch keys are registrered
BOOL hotKeysRegistred = FALSE;	// if the switch hot keys are registrered
BOOL cyclingKeysRegistered = FALSE; // if the cycling keys are registrered
BOOL menuHotKeyRegistered = FALSE;  // if the menu hotkey is registered

ATOM vwLeft;
ATOM vwRight;
ATOM vwUp;
ATOM vwDown;
ATOM vwDesk[MAXDESK];
ATOM vwMenu;
ATOM cyclingKeyUp;
ATOM cyclingKeyDown;
ATOM stickyKey;
ATOM dismissKey;

BOOL enabled = TRUE;		// if VirtuaWin enabled or not
BOOL isDragging = FALSE;	// if we are currently dragging a window

int taskBarLeftWarp   = 0;      // the warp size for Left taskbar
int taskBarRightWarp  = 0;      // the warp size for Right taskbar
int taskBarTopWarp    = 0;      // the warp size for Top taskbar
int taskBarBottomWarp = 0;      // the warp size for Bottom taskbar
int taskbarOffset     = 3;      // Default 3, 0 if XP skinned taskbar is used.

HINSTANCE hInst;		// current instance
HWND taskHWnd;                  // handle to taskbar
HWND desktopHWnd;		// handle to the desktop window
HWND lastFGHWnd;		// handle to the last foreground window
int  lastFGStyle;               // style of the last foreground window

// vector holding icon handles for the systray
HICON icons[MAXDESK];           // 0=disabled, 1=9=nromal desks, 10=private desk
NOTIFYICONDATA nIconD;

HBITMAP iconReferenceVector[MAXWIN];
int vectorPosition = 0;

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
BOOL mouseEnable = FALSE; 
BOOL useMouseKey = FALSE;
BOOL keyEnable = TRUE;		
BOOL hotKeyEnable = FALSE;      
BOOL releaseFocus = FALSE;	
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
BOOL taskBarWarp = FALSE;       
BOOL saveSticky = FALSE;        
BOOL refreshOnWarp = FALSE;     
BOOL stickyKeyRegistered = FALSE;
BOOL dismissKeyRegistered = FALSE;
BOOL preserveZOrder = FALSE;      
BOOL deskWrap = FALSE;          
BOOL setupOpen = FALSE;         
BOOL invertY = FALSE;           
short stickyMenu = 1;         
short assignMenu = 1;
short directMenu = 1;
BOOL useDeskAssignment = FALSE;
BOOL saveLayoutOnExit = FALSE;
BOOL assignOnlyFirst = FALSE;
BOOL assignImmediately = TRUE;
BOOL cyclingKeysEnabled = FALSE;
BOOL displayTaskbarIcon = TRUE;
BOOL noTaskbarCheck = FALSE;
BOOL trickyWindows = TRUE;
BOOL permanentSticky = FALSE;
BOOL hiddenWindowRaise = TRUE;
BOOL hiddenWindowPopup = TRUE;

UINT MOUSEKEY = 0;           // Holds the modifier for enabling mouse warp
UINT deskHotkey[MAXDESK];
UINT deskHotkeyMod[MAXDESK];
UINT deskHotkeyWin[MAXDESK];
UINT hotCycleUp = 0;
UINT hotCycleUpMod = 0;
UINT hotCycleUpWin = 0;
UINT hotCycleDown = 0;
UINT hotCycleDownMod = 0;
UINT hotCycleDownWin = 0;
UINT hotkeyMenuEn = 0;
UINT hotkeyMenu = 0;
UINT hotkeyMenuMod = 0;
UINT hotkeyMenuWin = 0;
UINT hotkeyStickyEn = 0;
UINT hotkeySticky = 0;
UINT hotkeyStickyMod = 0;
UINT hotkeyStickyWin = 0;
UINT hotkeyDismissEn = 0;
UINT hotkeyDismiss = 0;
UINT hotkeyDismissMod = 0;
UINT hotkeyDismissWin = 0;

int curDisabledMod = 0; 
unsigned long vwZOrder=0 ;

#define windowIsNotHung(hWnd,waitTime) (SendMessageTimeout(hWnd,(int)NULL,0,0,SMTO_ABORTIFHUNG|SMTO_BLOCK,waitTime,NULL))

#define vwSHWIN_TRYHARD   0x01
BOOL showHideWindow(windowType *aWindow, int shwFlags, unsigned char show);

#ifdef NDEBUG
#define vwLogPrint(a)
#else
FILE *vwLog ;
#define vwLogPrint(a) (fprintf a , fflush(vwLog))
#endif

/************************************************
 * Locks the window list protection
 */
void lockMutex(void)
{
    if (hMutex != (HANDLE)0 && WaitForSingleObject(hMutex,0) == WAIT_TIMEOUT) {
        WaitForSingleObject(hMutex,INFINITE);
    }
}

/************************************************
 * Releases the window list protection
 */
void releaseMutex(void)
{
    if (hMutex != (HANDLE)0) ReleaseMutex(hMutex);
}

/*************************************************
 * Checks if mouse key modifier is pressed
 */
inline BOOL checkMouseState(void)
{
    if(!GetSystemMetrics(SM_SWAPBUTTON)) {  // Check the state of mouse button(s)
        if(HIWORD(GetAsyncKeyState(VK_LBUTTON)))
            return TRUE;
        else
            return FALSE;
    } else if(HIWORD(GetAsyncKeyState(VK_RBUTTON)))
        return TRUE;
    else
        return FALSE;
}

/*************************************************
 * The mouse thread. This function runs in a thread and checks the mouse
 * position every 50ms. If mouse support is disabled, the thread will be in 
 * suspended state.
 */
DWORD WINAPI MouseProc(LPVOID lpParameter)
{
   // in this function I'm using int for bool & 0,1 rather than
   // false,true since the file extension is .c and it's being compiled
   // as C rather than C++ ... this should change when we port to C++
  
   int mousekeyPressed = 0;
   POINT firstPoint;
   POINT pt;

   // infinite loop
   while(1) {
      // If the useMouseKey function is turned on (the function
      // that requires a modifier key to be pressed to change
      // desktops using the mouse) and that key is pressed, then
      // we need to get the position at which the modifier key was
      // pressed.  Later we'll use that first position to see if
      // there's a motion tendency towards the edge of the screen that
      // we're switching to.  If they're not moving the mouse in that
      // general direction then we don't want to make the switch
      // because that sometimes causes the screen to switch when the
      // user presses the modifier key for some other purpose if the
      // mouse is near the edge of the screen.  Checking for their
      // general motion tendency feels natural and prevents accidental
      // switching

      if(useMouseKey) { // Are we using a mouse key
        if(HIWORD(GetAsyncKeyState(MOUSEKEY))) {
          if(!mousekeyPressed) {
             mousekeyPressed = 1;
             GetCursorPos(&firstPoint);
          }
        }
        else {
          mousekeyPressed = 0;
        }
      }

      // sleep between iterations of this function.  If we're using
      // modifier keys it loops more often to be sure to watch the
      // tendency.
      if(useMouseKey) {
         Sleep(4);
      }
      else {
        Sleep(50); 
      }

      // Now get the second point
      GetCursorPos(&pt);

      // Now figure out the motion tendency
      int xDelta = pt.x - firstPoint.x;
      int yDelta = pt.y - firstPoint.y;

      // If they're not using the modifier keys we'll just set all of
      // these to true to simplify the logic later on
      int movingLeft  = !useMouseKey || (mousekeyPressed && (xDelta < -25)); // && (yDelta > -30 && yDelta < 30);
      int movingRight = !useMouseKey || (mousekeyPressed && (xDelta >  25)); // && (yDelta > -30 && yDelta < 30);
      int movingUp    = !useMouseKey || (mousekeyPressed && (yDelta < -25)); // && (xDelta > -30 && xDelta < 30);
      int movingDown  = !useMouseKey || (mousekeyPressed && (yDelta >  25)); // && (xDelta > -30 && xDelta < 30);

      // ...and if we're moving in the right direction and close
      // enough to the side of the screen, send the message to switch
      // desktops
      if( movingLeft  &&  pt.x < (screenLeft   + 3 + (taskBarWarp * taskBarLeftWarp   * checkMouseState()))) {
         // switch left
         SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, 
                           MAKELPARAM(checkMouseState(), VW_MOUSELEFT));
      }
      else if( movingRight && pt.x > (screenRight  - 3 - (taskBarWarp * taskBarRightWarp  * checkMouseState()))) { 
         // switch right
         SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, 
                           MAKELPARAM(checkMouseState(), VW_MOUSERIGHT));
      }
      else if( movingUp && pt.y < (screenTop    + 3 + (taskBarWarp * taskBarTopWarp    * checkMouseState()))) { 
         // switch up
         SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, 
                           MAKELPARAM(checkMouseState(), VW_MOUSEUP));
      }
      else if( movingDown && pt.y > (screenBottom - 3 - (taskBarWarp * taskBarBottomWarp * checkMouseState()))) {
         // switch down
         SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, 
                           MAKELPARAM(checkMouseState(), VW_MOUSEDOWN));
      }
      else {
         SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, MAKELPARAM(0, VW_MOUSERESET));
      }
   }

   return TRUE;
}

/************************ *************************
 * Turns on/off the mouse thread. Makes sure that the the thread functions
 * only is called if needed.
 */
void enableMouse(BOOL turnOn)
{
    // Try to turn on thread if not already running
    if( turnOn && !mouseEnabled )
    {
        ResumeThread(mouseThread);
        mouseEnabled = TRUE;
    }
    // Try to turn of thread if already not stopped
    else if( !turnOn && mouseEnabled )
    {
        SuspendThread(mouseThread);
        mouseEnabled = FALSE;
    }
}

/*************************************************
 * Sets the icon in the systray and updates the currentDesk variable
 */
static void setIcon(int theNumber)
{
    nIconD.hIcon = icons[theNumber];
    if(displayTaskbarIcon)
        Shell_NotifyIcon(NIM_MODIFY, &nIconD);
}

/************************ *************************
 * Loads the icons for the systray according to the current setup
 */
static void loadIcons(void)
{
    int xIcon = GetSystemMetrics(SM_CXSMICON);
    int yIcon = GetSystemMetrics(SM_CYSMICON);
    int ii, iconId ;
    char buff[16] ;
    
    if(nDesksY == 2 && nDesksX == 2) // if 2 by 2 mode
        iconId = IDI_SMALL_DIS ;
    else
        iconId = IDI_ICON0 ;
    
    strcpy(buff,"icons/X.ico") ;
    ii = nDesksY * nDesksX ;
    do {
        /* Try to load user defined icons */
        buff[6] = '0' + ii ;
        icons[ii] = (HICON) LoadImage(hInst, buff, IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE);
        if(icons[ii] == NULL)
            /* use the built in standard */
            icons[ii] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(iconId+ii), IMAGE_ICON, xIcon, yIcon, 0);
    } while(--ii >= 0) ;
    if(deskHotkey[vwDESK_PRIVATE1] != 0)
        /* create icon for the private desktop - use the disabled icon */
        icons[vwDESK_PRIVATE1] = icons[0] ;
}

/*************************************************
 * Resets all icons and reloads them
 */
void reLoadIcons(void)
{
    int ii=MAXDESK-1;
    do
        icons[ii] = NULL;
    while(--ii >= 0) ;
    loadIcons();
    setIcon(currentDesk);
}

/*************************************************
 * Sets the modifier key for switching desktop with number mouse
 */
void setMouseKey(void)
{
    if(mouseModAlt == TRUE)
        MOUSEKEY = VK_MENU;
    else if(mouseModShift == TRUE)
        MOUSEKEY = VK_SHIFT;
    else if(mouseModCtrl == TRUE)
        MOUSEKEY = VK_CONTROL;
}

/*************************************************
 * Register the keys to use for switching desktop with the arrow keys
 */
static BOOL registerKeys(void)
{
    UINT MODKEY;
    if(keyEnable && !keysRegistred) {
        keysRegistred = TRUE;
        MODKEY = modAlt | modShift | modCtrl | modWin;
        vwLeft = GlobalAddAtom("atomKeyLeft");
        if((RegisterHotKey(hWnd, vwLeft, MODKEY, VK_LEFT) == FALSE))
            return FALSE;
        vwRight = GlobalAddAtom("atomKeyRight");
        if((RegisterHotKey(hWnd, vwRight, MODKEY, VK_RIGHT) == FALSE))
            return FALSE;
        vwUp = GlobalAddAtom("atomKeyUp");
        if((RegisterHotKey(hWnd, vwUp, MODKEY, VK_UP) == FALSE))
            return FALSE;
        vwDown = GlobalAddAtom("atomKeyDown");
        if((RegisterHotKey(hWnd, vwDown, MODKEY, VK_DOWN) == FALSE))
            return FALSE;
        return TRUE;
    }
    return TRUE;
}

/*************************************************
 * Unegister the keys to use for switching desktop
 */
static void unRegisterKeys(void)
{
    if(keysRegistred) {
        keysRegistred = FALSE;
        UnregisterHotKey(hWnd, vwLeft);
        UnregisterHotKey(hWnd, vwRight);
        UnregisterHotKey(hWnd, vwUp);
        UnregisterHotKey(hWnd, vwDown);
    }
}

/*************************************************
 * Translates virtual key codes to "hotkey codes"
 */
static WORD hotKey2ModKey(BYTE vModifiers)
{
    WORD mod = 0;
    if (vModifiers & HOTKEYF_ALT)
        mod |= MOD_ALT;
    if (vModifiers & HOTKEYF_CONTROL)
        mod |= MOD_CONTROL;
    if (vModifiers & HOTKEYF_SHIFT)
        mod |= MOD_SHIFT;
    return mod;
}

/*************************************************
 * Register the hotkeys to use for switching desktop and the winlist hotkey
 */
static BOOL registerHotKeys(void)
{
    char buff[16];
    int ii ;
    if(!hotKeysRegistred)
    {
        hotKeysRegistred = TRUE;
        strcpy(buff,"atomKeyP") ;
        if(deskHotkey[vwDESK_PRIVATE1])
        {
            // private desktop
            vwDesk[vwDESK_PRIVATE1] = GlobalAddAtom(buff);
            if((RegisterHotKey(hWnd, vwDesk[vwDESK_PRIVATE1], hotKey2ModKey(deskHotkeyMod[vwDESK_PRIVATE1]) | deskHotkeyWin[vwDESK_PRIVATE1],
                               deskHotkey[vwDESK_PRIVATE1]) == FALSE))
                return FALSE;
        }
        if(hotKeyEnable)
        {
            ii = nDesksY * nDesksX ;
            do {
                if(deskHotkey[ii])
                {
                    buff[7] = '0'+ii ;
                    vwDesk[ii] = GlobalAddAtom(buff);
                    if((RegisterHotKey(hWnd, vwDesk[ii], hotKey2ModKey(deskHotkeyMod[ii]) | deskHotkeyWin[ii],
                                       deskHotkey[ii]) == FALSE))
                        return FALSE;
                }
            } while(--ii > 0) ;
        }
    }
    return TRUE;
}

/*************************************************
 * Unregister the hot keys
 */
static void unRegisterHotKeys(void)
{
    int ii ;
    if(hotKeysRegistred) {
        hotKeysRegistred = FALSE;
        ii = MAXDESK-1 ;
        do {
            if(vwDesk[ii] != 0)
            {
                UnregisterHotKey(hWnd,vwDesk[ii]);
                vwDesk[ii] = 0 ;
            }
        } while(--ii > 0) ;
    }
}

/*************************************************
 * Register the desktop cycling hot keys, if defined
 */
static BOOL registerCyclingKeys(void)
{
    if(!cyclingKeysRegistered && cyclingKeysEnabled) {
        cyclingKeysRegistered = TRUE;
        if(hotCycleUp)
        {
            cyclingKeyUp = GlobalAddAtom("VWCyclingKeyUp");
            if((RegisterHotKey(hWnd, cyclingKeyUp, hotKey2ModKey(hotCycleUpMod) | hotCycleUpWin, hotCycleUp) == FALSE))
                return FALSE;
        }
        if(hotCycleDown)
        {
            cyclingKeyDown = GlobalAddAtom("VWCyclingKeyDown");
            if((RegisterHotKey(hWnd, cyclingKeyDown, hotKey2ModKey(hotCycleDownMod) | hotCycleDownWin, hotCycleDown) == FALSE))
                return FALSE;
        }
    }
    return TRUE;
}

/*************************************************
 * Unregister the cycling  hot keys, if previosly registred
 */
static void unRegisterCyclingKeys(void)
{
    if(cyclingKeysRegistered) {
        cyclingKeysRegistered = FALSE;
        UnregisterHotKey(hWnd, cyclingKeyUp);
        UnregisterHotKey(hWnd, cyclingKeyDown);
    }
}

/*************************************************
 * Register the window menu hot key, if defined
 */
static BOOL registerMenuHotKey(void)
{
    if(!menuHotKeyRegistered && hotkeyMenuEn && hotkeyMenu) {
        menuHotKeyRegistered = TRUE;
        vwMenu = GlobalAddAtom("atomKeyMenu");
        if(RegisterHotKey(hWnd, vwMenu, hotKey2ModKey(hotkeyMenuMod) | hotkeyMenuWin, hotkeyMenu) == FALSE)
            return FALSE;
    }
    return TRUE;
}

/*************************************************
 * Unregister the window menu hot key, if previosly registred
 */
static void unRegisterMenuHotKey(void)
{
    if(menuHotKeyRegistered) {
        menuHotKeyRegistered = FALSE;
        UnregisterHotKey(hWnd, vwMenu);
    }
}

/*************************************************
 * Register the sticky hot key, if defined
 */
static BOOL registerStickyKey(void)
{
    if(!stickyKeyRegistered && hotkeyStickyEn && hotkeySticky) {
        stickyKeyRegistered = TRUE;
        stickyKey = GlobalAddAtom("VWStickyKey");
        if((RegisterHotKey(hWnd, stickyKey, hotKey2ModKey(hotkeyStickyMod) | hotkeyStickyWin, hotkeySticky) == FALSE))
            return FALSE;
        else
            return TRUE;
    }
    return TRUE;
}

/*************************************************
 * Unregister the sticky hot key, if previosly registred
 */
static void unRegisterStickyKey(void)
{
    if(stickyKeyRegistered) {
        stickyKeyRegistered = FALSE;
        UnregisterHotKey(hWnd, stickyKey);
    }
}

/*************************************************
 * Register the dismiss hot key, if defined
 */
static BOOL registerDismissKey(void)
{
    if(!dismissKeyRegistered && hotkeyDismissEn && hotkeyDismiss) {
        dismissKeyRegistered = TRUE;
        dismissKey = GlobalAddAtom("VWDismissKey");
        if((RegisterHotKey(hWnd, dismissKey, hotKey2ModKey(hotkeyDismissMod) | hotkeyDismissWin, hotkeyDismiss) == FALSE))
            return FALSE;
        else
            return TRUE;
    }
    return TRUE;
}

/*************************************************
 * Unregister the dismiss hot key, if previosly registred
 */
static void unRegisterDismissKey(void)
{
    if(dismissKeyRegistered)
    {
        dismissKeyRegistered = FALSE;
        UnregisterHotKey(hWnd, dismissKey);
    }
}

/************************************************
 * Convinient function for registering all hotkeys
 */
void registerAllKeys(void)
{
    if(!registerKeys())
        MessageBox(hWnd, "Invalid key modifier combination, check control keys!", 
                   NULL, MB_ICONWARNING);
    if(!registerCyclingKeys())
        MessageBox(hWnd, "Invalid key modifier combination, check cycling hot keys!", 
                   NULL, MB_ICONWARNING);
    if(!registerHotKeys())
        MessageBox(hWnd, "Invalid key modifier combination, check hot keys!", 
                   NULL, MB_ICONWARNING);
    if(!registerMenuHotKey())
        MessageBox(hWnd, "Invalid key modifier combination, check menu hot key!", 
                   NULL, MB_ICONWARNING);
    if(!registerStickyKey())
        MessageBox(hWnd, "Invalid key modifier combination, check sticky hot key!", 
                   NULL, MB_ICONWARNING);
    if(!registerDismissKey())
        MessageBox(hWnd, "Invalid key modifier combination, check dismiss window hot key!", 
                   NULL, MB_ICONWARNING);
}

/************************************************
 * Convinient function for unregistering all hotkeys
 */
void unRegisterAllKeys(void)
{
    unRegisterDismissKey();
    unRegisterStickyKey();
    unRegisterMenuHotKey();
    unRegisterHotKeys();
    unRegisterCyclingKeys();
    unRegisterKeys();
}

/************************************************
 * Get screen width and height and store values in
 * global variables
 */
static void getScreenSize(void)
{
#if 0
    // TODO:  make a user configured flag to specify whether to only act on the primary display or not
   if(0)
   {
      // TODO:  Figure out how to get the size of JUST the primary
      // monitor... I have the width, but dont' know what to do to get
      // the origin w/o using the GetDeviceCaps functions that don't
      // seem to work in MingW
      screenLeft   = GetSystemMetrics(SM_XVIRTUALSCREEN);
      screenRight  = GetSystemMetrics(SM_CXVIRTUALSCREEN) + screenLeft;
      screenTop    = GetSystemMetrics(SM_YVIRTUALSCREEN);
      screenBottom = GetSystemMetrics(SM_CYVIRTUALSCREEN) + screenTop;
   }
#endif
   {
      screenLeft   = GetSystemMetrics(SM_XVIRTUALSCREEN);
      screenRight  = GetSystemMetrics(SM_CXVIRTUALSCREEN) + screenLeft;
      screenTop    = GetSystemMetrics(SM_YVIRTUALSCREEN);
      screenBottom = GetSystemMetrics(SM_CYVIRTUALSCREEN) + screenTop;
   }
}

/************************************************
 * Tries to locate the handle to the taskbar
 */
static void goGetTheTaskbarHandle(void)
{
    if(!noTaskbarCheck)
    {
        HWND hwndTray = FindWindowEx(NULL, NULL, "Shell_TrayWnd", NULL);
        HWND hwndBar = FindWindowEx(hwndTray, NULL, "ReBarWindow32", NULL );
        
        // Maybe "RebarWindow32" is not a child to "Shell_TrayWnd", then try this
        if( hwndBar == NULL )
            hwndBar = hwndTray;
        
        taskHWnd = FindWindowEx(hwndBar, NULL, "MSTaskSwWClass", NULL);
        
        if( taskHWnd == NULL )
            MessageBox(hWnd, "Could not locate handle to the taskbar.\n This will disable the ability to hide troublesome windows correctly.",vwVIRTUAWIN_NAME " Error", 0); 
    }
}

/************************************************
 * Grabs and stores the taskbar coordinates
 */
void getTaskbarLocation(void)
{
    if(!noTaskbarCheck)
    {
        RECT r;
        taskBarLeftWarp   = 0;
        taskBarRightWarp  = 0;
        taskBarTopWarp    = 0;
        taskBarBottomWarp = 0;
        
        /* Get the height of the task bar */
        GetWindowRect(FindWindow("Shell_traywnd", ""), &r);
        /* Determine position of task bar */
        if ((r.bottom + r.top) == (screenBottom - screenTop)) // task bar is on side
        {
            if (r.left <= screenLeft)                          // task bar is on left
                taskBarLeftWarp   = r.right - r.left - taskbarOffset;
            else                                               // task bar is on right
                taskBarRightWarp  = r.right - r.left - taskbarOffset;
        }
        else                                                  // task bar is on top/bottom
        {
            if (r.top <= screenTop)                            // task bar is on top
                taskBarTopWarp    = r.bottom - r.top - taskbarOffset;
            else                                               // task bar is on bottom
                taskBarBottomWarp = r.bottom - r.top - taskbarOffset;
        }
    }
}

/************************************************
 * Show the setup dialog and perform some stuff before and after display
 */
static void showSetup(void)
{
    if(!setupOpen)
    {   // Stupid fix, can't get this modal
        setupOpen = TRUE;
        // reload load current config
        readConfig();
        createPropertySheet(hInst,hWnd);
        setupOpen = FALSE;
    }
    else
        // setup dialog has probably been lost under the windows raise it.
        SetForegroundWindow(hWnd);
}

/************************************************
 * Show the VirtuaWin help pages
 */
void showHelp(HWND aHWnd)
{
    char buff[MAX_PATH];
    GetFilename(vwHELP,0,buff);
    WinHelp(aHWnd, buff, HELP_CONTENTS, 0);
}

/*************************************************
 * Returns index if window is found, -1 otherwise
 */
static inline int winListFind(HWND hwnd)
{
    int index = nWin ;
    while(--index >= 0)
        if(winList[index].Handle == hwnd)
            break ;
    return index ;
}

/************************************************
 * Checks if this window is saved as a tricky window that needs the 
 * "move technique" to be hidden.
 */
static BOOL isTrickyWindow(char *className, RECT *pos)
{
#if USE_SHOWWIN
    if(trickyWindows)
    {
        int i=curTricky ;
        while(--i >= 0)
        {
            vwLogPrint((vwLog, "Tricky comparing [%s] with %d [%s]\n",className,trickyList[i].winClassLen,trickyList[i].winClassName)) ;
            if(!strncmp(trickyList[i].winClassName,className,trickyList[i].winClassLen)) 
                return vwTRICKY_WINDOW ;
        }
        if((pos->left == -32000) && (pos->top == -32000))
            // some apps hide the window by pushing it to -32000, the windows
            // cannot be moved from here VirtualWin handles these by making them
            // Tricky (not hiding them) so crash recovery will still find them.
            return vwTRICKY_POSITION ;
    }
    return 0 ;  
#else 
    return vwTRICKY_WINDOW ;
#endif 
}

/*************************************************
 * Checks if a window is a previous saved sticky window
 */
static BOOL checkIfSavedSticky(char *className)
{
    int i=curSticky ;
    while(--i >= 0)
    {
        vwLogPrint((vwLog, "Sticky comparing [%s] with %d [%s]\n",className,stickyList[i].winClassLen,stickyList[i].winClassName)) ;
        if(!strncmp(stickyList[i].winClassName, className, stickyList[i].winClassLen))
            // Typically user windows will loose their stickiness if
            // minimized, therefore we do not remove their name from 
            // the list as done above.
            return TRUE;
    }
    return FALSE;
}

/*************************************************
 * Checks if a window is an predifined desktop to go to
 */
static int checkIfAssignedDesktop(char *className)
{
    int i;
    for(i = 0; i < curAssigned; ++i) 
    {
        vwLogPrint((vwLog, "Assign comparing [%s] with %d [%s]\n",className,assignedList[i].winClassLen,assignedList[i].winClassName)) ;
        
        if((assignedList[i].winClassLen > 0) &&
           !strncmp(assignedList[i].winClassName, className, assignedList[i].winClassLen))
        {
            if(assignOnlyFirst)
                assignedList[i].winClassLen = 0 ;
            if((assignedList[i].desktop > (nDesksX * nDesksY)) && (assignedList[i].desktop != vwDESK_PRIVATE1))
                MessageBox(hWnd, "Tried to assign an application to an unavaliable desktop.\nIt will not be assigned.\nCheck desktop assignmet configuration.",vwVIRTUAWIN_NAME " Error", 0); 
            else
                return assignedList[i].desktop; // Yes, assign
        }
    }
    return currentDesk; // No assignment, return current
}

/*************************************************
 * Forces a window into the foreground. Must be done in this way to avoid
 * the flashing in the taskbar insted of actually changing active window.
 */
static void setForegroundWin(HWND theWin, int makeTop)
{
    DWORD ThreadID1;
    DWORD ThreadID2;
    HWND cwHwnd ;
    int index, err ;
    
    vwLogPrint((vwLog,"setForegroundWin: %x %d (%x)\n",(int) theWin,makeTop,(int) hWnd)) ;
    if(theWin == NULL)
    {
        /* releasing the focus, the current foreground window MUST be
         * released otherwise VW can think its a popup. Assigning the focus
         * to a window when no window has the focus is prone to failure so
         * give the focus to VW's hidden window rather than to the Desktop */
        theWin = hWnd ;
        makeTop = 0 ;
    }
    else if(((index = winListFind(theWin)) < 0) || !winList[index].Visible ||
            !windowIsNotHung(theWin,100))
        /* don't make the foreground a hidden or non-managed or hung window */
        return ;
    
    index = 2 ;
    for(;;)
    {
        /* do not bother making it the foreground if it already is */
        if((cwHwnd = GetForegroundWindow()) == theWin)
            break ;
        if(cwHwnd != hWnd)
        {
            /* To get the required permission to make theWin the active window, VW must become the active window.
             * To get the required permissions to do that VW must attatch to the thread of the current active window.
             * But don't do this if the current process hung */
            if((cwHwnd != NULL) && windowIsNotHung(cwHwnd,100))
            {
                /* Get the thread responsible for VirtuaWin,
                   and the thread for the foreground window */
                ThreadID1 = GetWindowThreadProcessId(cwHwnd, NULL);
                ThreadID2 = GetWindowThreadProcessId(hWnd, NULL);
                /* By sharing input state, threads share their concept of
                   the active window */
                if(ThreadID1 != ThreadID2)
                {
                    AttachThreadInput(ThreadID1, ThreadID2, TRUE);
                    err = SetForegroundWindow(hWnd) ; 
                    // Set VirtuaWin active. Don't no why, but it seems to work
                    AttachThreadInput(ThreadID1, ThreadID2, FALSE);
                    vwLogPrint((vwLog, "Attached to foreground Window: %x - %x\n",(int) cwHwnd,(int) GetForegroundWindow())) ;
                }
                else
                {
                    SetForegroundWindow(hWnd) ; 
                    vwLogPrint((vwLog, "VW owns foreground Window: %x - %x\n",(int) cwHwnd,(int) GetForegroundWindow())) ;
                }
            }
            else
            {
                SetForegroundWindow(hWnd) ; 
                vwLogPrint((vwLog, "No foreground Window or hung: %x - %x\n",(int) cwHwnd,(int) GetForegroundWindow())) ;
            }
        }
        SetForegroundWindow(theWin) ;
        /* SetForegroundWindow can return success (non-zero) but not succeed (GetForegroundWindow != theWin)
         * Getting the foreground window right is really important because if the existing foreground window
         * is left as the foreground window but hidden (common when moving the app or desk) VW will confuse
         * it with a popup */
        cwHwnd = GetForegroundWindow() ;
        vwLogPrint((vwLog,"Set foreground window: %d, %x -> %x\n",(theWin == cwHwnd),(int) theWin,(int) cwHwnd)) ;
        if((cwHwnd == theWin) || (--index < 0))
            break ;
        /* A short sleep allows the rest of the system to catch up */
        vwLogPrint((vwLog,"About to FG sleep\n")) ;
        Sleep(1) ;
    }
    /* bring to the front if requested as swapping desks can muddle the order */
    if(makeTop)
        BringWindowToTop(theWin);
}

/************************************************
 * Moves a window to a given desk
 */
static int windowSetDesk(HWND theWin, int theDesk, int move)
{
    HWND ownerWin, activeHWnd ;
    unsigned char show ;
    BOOL setActive ;
    int index, ret=0 ;
    
    activeHWnd = GetForegroundWindow() ;
    vwLogPrint((vwLog,"Set window desk: %x %d %d (%x)\n",(int) theWin,theDesk,move,(int) activeHWnd)) ;
    setActive = FALSE ;
    do {
        ownerWin = 0 ;
        index = nWin ;
        while(--index >= 0)
        {
            if((winList[index].Handle == theWin) || (winList[index].Owner == theWin))
            {
                // Even if the window does not have to be moved, it was found so return true
                ret = 1 ;
                if(winList[index].Desk != theDesk)
                    winList[index].ZOrder[theDesk] = winList[index].ZOrder[winList[index].Desk] ;
                else if(winList[index].Visible == vwVISIBLE_YESTEMP)
                    winList[index].ZOrder[theDesk] = winList[index].ZOrder[currentDesk] ;
                if(((winList[index].Desk != theDesk) || (winList[index].Visible == vwVISIBLE_YESTEMP)) &&
                   !winList[index].Sticky)
                {
                    // set the windows zorder on the new desk to be its zorder on the current desk
                    winList[index].ZOrder[theDesk] = winList[index].ZOrder[winList[index].Desk] ;
                    /* if temporarily copying the window to this desk */ 
                    if(move == 2)
                    {
                        if(winList[index].Desk == currentDesk)
                            show = vwVISIBLE_YES ;
                        else if(theDesk == currentDesk)
                            show = vwVISIBLE_YESTEMP ;
                        else
                            show = vwVISIBLE_NO ;
                    }
                    else
                    {
                        winList[index].Desk = theDesk;
                        if(currentDesk == theDesk)
                            show = vwVISIBLE_YES ;
                        else if(move)
                            show = vwVISIBLE_NO ;
                        else
                            show = vwVISIBLE_YESTEMP ;
                    }
                    if(!show && (activeHWnd == winList[index].Handle))
                        setActive = TRUE ;
                    showHideWindow(&winList[index],0,show) ;
                    if((winList[index].Owner != 0) && (winList[index].Owner != theWin))
                        ownerWin = winList[index].Owner ;
                }
            }
        }
    } while((theWin = ownerWin) != 0) ;
    if(setActive)
    {
        // we have just assigned the foreground window to a different
        // desktop, we must find a replacement as leaving this one as the
        // active foreground window can lead to problems 
        unsigned long activeZOrder=0 ;
        activeHWnd = NULL ;
        index = nWin ;
        while(--index >= 0)
        {
            if((winList[index].Desk == currentDesk) &&
               ((winList[index].Style & (WS_MINIMIZE|WS_VISIBLE)) == WS_VISIBLE) && 
               (winList[index].ZOrder[currentDesk] > activeZOrder))
            {
                activeHWnd = winList[index].Handle;
                activeZOrder = winList[index].ZOrder[currentDesk];
            }
        }
        vwLogPrint((vwLog,"Looking for replacement active: %x\n",(int) activeHWnd)) ;
        setForegroundWin(activeHWnd,0) ;
    }
    return ret ;
}

/************************************************
 * Sets a windows sticky setting
 */
static int windowSetSticky(HWND theWin, int state)
{
    unsigned long zorder ;
    HWND ownerWin ;
    int index, ii, ret=0 ;
    
    do {
        ownerWin = 0 ;
        index = nWin ;
        while(--index >= 0)
        {
            if((winList[index].Handle == theWin) || (winList[index].Owner == theWin))
            {
                if(state < 0) // toggle sticky state - set state so all owner windows are set correctly.
                    state = winList[index].Sticky ^ TRUE;
                vwLogPrint((vwLog,"Setting Sticky: %x %x - %d -> %d\n",(int) winList[index].Handle,
                            (int) theWin,(int) winList[index].Sticky,state)) ;
                winList[index].Sticky = state ;
                if(winList[index].Sticky)
                {
                    // set its zorder of all desks to its zorder on its current desk
                    zorder = winList[index].ZOrder[winList[index].Desk] ;
                    ii = MAXDESK - 1 ;
                    do
                        winList[index].ZOrder[ii] = zorder ;
                    while(--ii >= 0) ;
                    // if not visible (i.e. was on another desktop) then make it visible
                    if(!winList[index].Visible)
                        showHideWindow(&winList[index],0,vwVISIBLE_YES) ;
                }
                winList[index].Desk = currentDesk;
                if((winList[index].Owner != 0) && (winList[index].Owner != theWin))
                    ownerWin = winList[index].Owner ;
                ret = 1 ;
            }
        }
    } while((theWin = ownerWin) != 0) ;
    return ret ;
}

/*************************************************
 * Searches for windows in user list
 */
static void findUserWindows(void) 
{
    HWND tmpHnd;
    int i;
    
    for(i = 0; i < curUser; ++i)
    {
        if(userList[i].isClass)
            tmpHnd = FindWindow(userList[i].winNameClass, NULL);
        else
            tmpHnd = FindWindow(NULL, userList[i].winNameClass);
        
        if(tmpHnd != NULL)
        {
            lockMutex();
            if(winListFind(tmpHnd) < 0)
            {
                winList[nWin].Handle = tmpHnd;
                winList[nWin].Style = GetWindowLong(tmpHnd, GWL_STYLE);
                winList[nWin].ExStyle = GetWindowLong(tmpHnd, GWL_EXSTYLE);
                nWin++;
            }
            releaseMutex();
        }
        else
            userList[i].isClass ^= TRUE;
    }
}


/*************************************************
 * Callback function. Integrates all enumerated windows
 */
static inline BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam) 
{
    int idx, style, exstyle ;
    RECT pos ;
    
    if((style = GetWindowLong(hwnd, GWL_STYLE)) & WS_CHILD)                // No child windows
            return TRUE;
    if((idx=winListFind(hwnd)) < 0)
    {
        exstyle = GetWindowLong(hwnd, GWL_EXSTYLE) ;
        /* Criterias for a window to be handeled by VirtuaWin */
        if(!(style & WS_VISIBLE) ||                                       // Must be visible
           (exstyle & WS_EX_TOOLWINDOW) ||                                // No toolwindows
           (GetParent(hwnd) && (GetParent(hwnd) != desktopHWnd)))          // Only toplevel or owned by desktop
            // Ignore this window
            return TRUE;
            
        if(nWin >= MAXWIN)
        {
            enabled = FALSE;
            MessageBox(hWnd, "Oops! Maximum windows reached. \n" vwVIRTUAWIN_NAME " has been disabled. \nMail " vwVIRTUAWIN_EMAIL " and tell me this.",vwVIRTUAWIN_NAME " Error", MB_ICONERROR);
            return FALSE;
        }
        idx = nWin++;
        winList[idx].Handle = hwnd;
        winList[idx].Style = style;
        winList[idx].ExStyle = exstyle;
        return TRUE;
    }
    winList[idx].State = 1 ;
    if(winList[idx].Visible)
    {
        if(!(style & WS_VISIBLE))
        {
            /* this window has been hidden by some else - stop handling it
             * unless VirtualWin knows its not visible (which means it is
             * probably a hung process so keep it.) */
            winList[idx].State = 0 ;
            return TRUE ;
        }
        /* if visible store the latest style flags */
        winList[idx].Style = style;
        winList[idx].ExStyle = GetWindowLong(hwnd, GWL_EXSTYLE) ;
    }
    else if(winList[idx].Tricky)
    {
        GetWindowRect(hwnd,&pos) ;
        if((pos.left > -10000) || (pos.top > -10000))
        {
            /* Something has moved this window back into a visible area (or
             * at least outside VirtuaWins domain) so make it belong to this
             * desktop, update the list entry, also check the location as the
             * app may have only made the window visible */
            vwLogPrint((vwLog,"Got tricky window state change: %x %d (%d) %d -> %d %d\n",
                        (int) winList[idx].Handle,winList[idx].Desk,currentDesk,
                        winList[idx].Visible,(int)pos.left,(int)pos.top)) ;
            winList[idx].State = 2 ;
        }
    }
    else if(style & WS_VISIBLE)
    {
        /* Something has made this window visible so make it belong to this desktop, update the list entry,
         * also check the location as the app may have only made the window visible */
        vwLogPrint((vwLog,"Got window state change: %x %d (%d) %d -> %d\n",
                    (int) winList[idx].Handle,winList[idx].Desk,currentDesk,
                    winList[idx].Visible,IsWindowVisible(winList[idx].Handle))) ;
        winList[idx].State = 2 ;
    }
    return TRUE;
}


static int winListUpdate(void)
{
    HWND activeHWnd ;
    RECT pos ;
    char buf[vwCLASSNAME_MAX+1];
    int inWin, i, j, hungCount=0 ;
    
    vwLogPrint((vwLog,"Updating winList, nWin %d, fgw %x tpw %x\n",nWin,
                (int) GetForegroundWindow(),(int) GetTopWindow(NULL))) ;
    /* We now own the mutex. */
    i = inWin = nWin ;
    while(--i >= 0)
        winList[i].State = 0 ;

    // Get all windows
    EnumWindows(enumWindowsProc,0);
    if(curUser)
        findUserWindows();
    
    // finish the initialisation of new windows
    for(i=inWin ; i < nWin ; ++i)
    {
        // Store the owner if there is one, but only if the owner is visible
        GetClassName(winList[i].Handle,buf,vwCLASSNAME_MAX);
        buf[vwCLASSNAME_MAX] = '\0' ;
        GetWindowRect(winList[i].Handle,&pos) ;
        winList[i].Tricky = isTrickyWindow(buf,&pos) ;
        winList[i].Visible = TRUE;
        winList[i].Desk = currentDesk;
        winList[i].ZOrder[currentDesk] = 1;
        winList[i].menuId = 0 ;
        winList[i].State = 1 ;
        if(((winList[i].Owner = GetWindow(winList[i].Handle, GW_OWNER)) != NULL) && IsWindowVisible(winList[i].Owner))
        {
            j = inWin ;
            while(--j >= 0)
                if(winList[j].State && (winList[j].Handle == winList[i].Owner))
                {
                    // an existing app has either unhidden an old window or popped up a new one 
                    // use the existing window's settings for this one
                    winList[i].Sticky = winList[j].Sticky ;
                    if((winList[i].Desk=winList[j].Desk) != currentDesk)
                        winList[i].State = 2 ;
                    break ;
                }
        }
        else
        {
            winList[i].Owner = NULL ;
            j = -1 ;
        }
        if(j < 0)
        {
            // isn't part of an existing app thats opened a new window
            if((winList[i].Sticky=checkIfSavedSticky(buf)))
                windowSetSticky(winList[i].Handle,TRUE) ;
            else if(useDeskAssignment &&
                    ((j = checkIfAssignedDesktop(buf)) != currentDesk))
                windowSetDesk(winList[i].Handle,j,assignImmediately) ;
        }
        vwLogPrint((vwLog,"Got new window %8x %x %x Desk %d Stk %d Trk %d Vis %d Own %x Pos %d %d\n",(int) winList[i].Handle,
                    (int)winList[i].Style,(int)winList[i].ExStyle,winList[i].Desk,winList[i].Sticky,winList[i].Tricky,
                    winList[i].Visible,(int) winList[i].Owner,(int) pos.left,(int) pos.top)) ;
    }
          
    // remove windows that have gone
    for(i=0, j=0 ; i < nWin; ++i)
    {
        if(winList[i].State)
        {
            if(i != j)
                winList[j] = winList[i] ;
            j++ ;
        }
        else
        {
            vwLogPrint((vwLog,"Lost window %8x %d %d %d %d %x\n",(int) winList[i].Handle,
                        winList[i].Desk,winList[i].Sticky,winList[i].Tricky,
                        winList[i].Visible,(int) winList[i].Owner)) ;
        }
    }
    nWin = j ;
    
    // Handle the re-assignment of any popped up window, set the zorder and count hung windows
    activeHWnd = GetForegroundWindow() ;
    i = nWin ;
    while(--i >= 0)
    {
        if(winList[i].State == 2)
            windowSetDesk(winList[i].Handle,currentDesk,2-hiddenWindowPopup) ;
        if(winList[i].Handle == activeHWnd)
        {
            // one problem with handling tricky windows is if the user closes the last window on a desktop
            // windows will select a Tricky hidden window as the replacement - try to spot this
            if(activeHWnd != lastFGHWnd)
            {
                // one problem with handling tricky windows is if the user closes the last window on a desktop
                // windows will select a Tricky hidden window as the replacement - try to spot & handle this
                if(!winList[i].Visible && winList[i].Tricky &&
                   ((lastFGHWnd == NULL) || !IsWindow(lastFGHWnd) ||
                    (((lastFGStyle & WS_MINIMIZE) == 0) && (GetWindowLong(lastFGHWnd,GWL_STYLE) & WS_MINIMIZE))))
                    setForegroundWin(NULL,0) ;
                else
                {
                    if(!winList[i].Visible && hiddenWindowRaise)
                        windowSetDesk(winList[i].Handle,currentDesk,2-hiddenWindowPopup) ;
                    winList[i].ZOrder[currentDesk] = ++vwZOrder ;
                    // if this is only a temporary display increase its zorder in its main desk
                    if(winList[i].Desk != currentDesk)
                        winList[i].ZOrder[winList[i].Desk] = vwZOrder ;
                }
            }
        }
        if(!winList[i].Visible)
        {
            if(winList[i].Sticky || (winList[i].Desk == currentDesk))
                hungCount++ ;
        }
        else if(((winList[i].Visible & vwVISIBLE_TEMP) == 0) &&
                !winList[i].Sticky && (winList[i].Desk != currentDesk))
            hungCount++ ;
    }
    if((lastFGHWnd = activeHWnd) != NULL)
        lastFGStyle = GetWindowLong(activeHWnd,GWL_STYLE) ;
    
    vwLogPrint((vwLog,"Updated winList, %d windows - %d hung\n",nWin,hungCount)) ;
    return hungCount ;
}

/*************************************************
 * Makes all windows visible
 */
static void showAll(int shwFlags)
{
    int x ;
    lockMutex();
    winListUpdate() ;
    for (x = 0; x < nWin; ++x) 
    {
        // still ignore windows on a private desktop unless exiting (vwSHWIN_TRYHARD)
        if((winList[x].Desk < vwDESK_PRIVATE1) || (shwFlags & vwSHWIN_TRYHARD))
        {
            winList[x].Desk = currentDesk ;
            showHideWindow(&winList[x],shwFlags,vwVISIBLE_YES);
        }
    }
    releaseMutex();
}

/************************************************
 * Does necessary stuff before shutting down
 */
static void shutDown(void)
{
    KillTimer(hWnd, 0x29a);                // Remove the timer
    if(saveLayoutOnExit || saveSticky)
    {
        lockMutex();
        winListUpdate() ;
        if(saveLayoutOnExit)
            saveAssignedList(nWin,winList); 
        if(saveSticky)
            saveStickyWindows(nWin,winList);
        releaseMutex();
    }
    saveDisabledList(nOfModules,moduleList);
    unloadModules();
    unRegisterAllKeys();
    showAll(0);                            // gather all windows quickly
    Shell_NotifyIcon(NIM_DELETE, &nIconD); // This removes the icon
    showAll(vwSHWIN_TRYHARD);            // try harder to gather remaining ones before exiting
    PostQuitMessage(0);
}

/************************************************
 * Callback for new window assignment and taskbar fix (removes bogus reappearances of tasks on win9x).
 */
static VOID CALLBACK monitorTimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    static unsigned long mtCount=0 ;
    int index, hungCount ;
    lockMutex();
    if((hungCount=winListUpdate()) > 0)
    {
        /* there's a hung process, try to handle it - every second to start
         * with, after 32 seconds only try once every 8 seconds */
        mtCount++ ;
        index = (mtCount & ~0x7f) ? 31:3;
        if((mtCount & index) == 0)
        {
            if(displayTaskbarIcon)
            {
                /* flash the icon for half a second each time we try */
                sprintf(nIconD.szTip,"%d window%s not responding",hungCount,(hungCount==1) ? "":"s") ;
                nIconD.hIcon = NULL; // No icon
                Shell_NotifyIcon(NIM_MODIFY, &nIconD);
            }
            index = nWin ;
            while(--index >= 0)
            {
                if(!winList[index].Visible)
                {
                   if(winList[index].Sticky || (winList[index].Desk == currentDesk))
                       showHideWindow(&winList[index],0,vwVISIBLE_YES) ;
                }
                else if(!(winList[index].Visible & vwVISIBLE_TEMP))
                {
                   if(!winList[index].Sticky && (winList[index].Desk != currentDesk))
                       showHideWindow(&winList[index],0,vwVISIBLE_NO) ;
                }
            }
        }
        else if((((mtCount-1) & index) == 0) && displayTaskbarIcon)
        {
            /* restore the icon */
            setIcon(currentDesk);
        }
    }
    else if(mtCount)
    {
        /* hung process problem has been resolved. */
        mtCount = 0 ;
        strcpy(nIconD.szTip,vwVIRTUAWIN_NAME_VERSION);	// Restore Tooltip
        setIcon(currentDesk);
    }

    if(taskbarFixRequired)
    {
        index = nWin ;
        while(--index >= 0)
            if(winList[index].Tricky && !winList[index].Visible)
                PostMessage(taskHWnd, RM_Shellhook, HSHELL_WINDOWDESTROYED, (LPARAM) winList[index].Handle);
    }
    releaseMutex();
}

/*************************************************
 * Helper function for all the step* functions below
 * Does the actual switching work 
 */
static int changeDesk(int newDesk, WPARAM msgWParam)
{
    HWND activeHWnd, zoh ;
    unsigned long activeZOrder, zox, zoy, zob ;
    int x, y, notHung=0 ;
    
    if(newDesk == currentDesk)
        // Nothing to do
        return 0;
#ifndef NDEBUG
    {
        SYSTEMTIME stime;
    
        GetLocalTime (&stime);
        vwLogPrint((vwLog, "[%04d-%02d-%02d %02d:%02d:%02d] Step Desk Start: %d -> %d\n",
                    stime.wYear, stime.wMonth, stime.wDay, stime.wHour,
                    stime.wMinute, stime.wSecond, currentDesk, newDesk)) ;
    }
#endif
    lockMutex();
    winListUpdate() ;
    
    if(isDragging)
    {
        /* move the app we are dragging to the new desktop, must handle owner windows */
        HWND ownerWnd;
        
        activeHWnd = GetForegroundWindow() ;
        do {
            ownerWnd = 0 ;
            x = nWin ;
            while(--x >= 0)
            {
                if((winList[x].Handle == activeHWnd) || (winList[x].Owner == activeHWnd))
                {
                    winList[x].Desk = newDesk;
                    winList[x].ZOrder[newDesk] = winList[x].ZOrder[currentDesk] ;
                    if((winList[x].Owner != 0) && (winList[x].Owner != activeHWnd))
                        ownerWnd = winList[x].Owner ;
                }
            }
        } while((activeHWnd = ownerWnd) != 0) ;
    }
    
    currentDesk = newDesk;
    /* Calculations for getting the x and y positions */
    currentDeskY = ((currentDesk - 1)/nDesksX) + 1 ;
    currentDeskX = nDesksX + currentDesk - (currentDeskY * nDesksX);
    
    activeHWnd = NULL;
    activeZOrder = 0;
    for (x = 0; x < nWin ; ++x)
    {
        if(winList[x].Sticky || (!minSwitch && (winList[x].Style & WS_MINIMIZE)))
            winList[x].Desk = currentDesk ;
        if(winList[x].Desk == currentDesk)
        {
            // Show these windows
            if(!winList[x].Visible)
                notHung = showHideWindow(&winList[x],0,vwVISIBLE_YES) ;
            else if(preserveZOrder)
                notHung = windowIsNotHung(winList[x].Handle,100) ;
            if(((winList[x].Style & (WS_MINIMIZE|WS_VISIBLE)) == WS_VISIBLE) && 
               (winList[x].ZOrder[currentDesk] > activeZOrder))
            {
                activeHWnd = winList[x].Handle;
                activeZOrder = winList[x].ZOrder[currentDesk];
            }
            if(preserveZOrder && notHung && ((winList[x].ExStyle & WS_EX_TOPMOST) == 0))
            {
                zox = winList[x].ZOrder[currentDesk] ;
                zoh = HWND_NOTOPMOST ;
                zob = 0xffffffff ;
                y = x ;
                while(--y >= 0)
                {
                    if((winList[y].Desk == currentDesk) &&
                       ((zoy=winList[y].ZOrder[currentDesk]) > zox) && (zoy < zob))
                    {
                        zob = zoy ;
                        zoh = winList[y].Handle ;
                    }
                }
                SetWindowPos(winList[x].Handle, zoh, 0, 0, 0, 0,
                             SWP_DEFERERASE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_NOMOVE) ;
            }
        }
        else if(winList[x].Visible)
            // Hide these, not iconic or "Switch minimized" enabled
            showHideWindow(&winList[x],0,vwVISIBLE_NO) ;
    }
    if(releaseFocus)     // Release focus maybe?
        activeHWnd = NULL ;
    vwLogPrint((vwLog, "Active found: %x (%d,%d)\n",(int) activeHWnd,(int)activeZOrder,releaseFocus)) ;
    setForegroundWin(activeHWnd,TRUE) ;
    // reset the monitor timer to give the system a chance to catch up first
    SetTimer(hWnd, 0x29a, 250, monitorTimerProc);
    releaseMutex();
    
    setIcon(currentDesk) ;
    if(refreshOnWarp) // Refresh the desktop 
        RedrawWindow( NULL, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN );
    
    postModuleMessage(MOD_CHANGEDESK,msgWParam,currentDesk);
#ifndef NDEBUG
    {
        SYSTEMTIME stime;
    
        GetLocalTime (&stime);
        vwLogPrint((vwLog, "[%04d-%02d-%02d %02d:%02d:%02d] Step Desk End (%x)\n",
                    stime.wYear, stime.wMonth, stime.wDay, stime.wHour,
                    stime.wMinute, stime.wSecond,(int)GetForegroundWindow())) ;
    }
#endif
    return currentDesk ;
}

/*************************************************
 * Goto a specified desktop specifying desk number
 */
int gotoDesk(int theDesk, BOOL force)
{
    if((theDesk >= MAXDESK) || ((theDesk > (nDesksY * nDesksX)) && !force))
        return 0;
    
    return changeDesk(theDesk,MOD_CHANGEDESK);
}

/*************************************************
 * Goto a specified desktop specifying desk number
 */
static int stepDelta(int delta)
{
    int newDesk ;
    if(currentDesk >= vwDESK_PRIVATE1)
        /* on a private desktop - go to first if delta is +ve, last otherwise */
        newDesk = (delta < 0) ? (nDesksX * nDesksY):1 ;
    else if((newDesk=currentDesk+delta) < 1)
    {
        if(!deskWrap)
            return 0 ;
        newDesk = nDesksX * nDesksY ;
    }
    else if(newDesk > (nDesksX * nDesksY))
    {
        if(!deskWrap)
            return 0 ;
        newDesk = 1 ;
    }
    return changeDesk(newDesk,MOD_CHANGEDESK);
}


/*************************************************
 * Step on desk to the right
 */
static int stepRight(void)
{
    int deskX, deskY=currentDeskY ;
    
    if(currentDesk >= vwDESK_PRIVATE1)
    {   /* on a private desktop - go to first */
        deskX = 1;
        deskY = 1;
    }
    else if((deskX=currentDeskX + 1) > nDesksX) {
        if(!deskWrap)
            return 0;
        deskX = 1;
    }
    return changeDesk(calculateDesk(deskX,deskY),MOD_STEPRIGHT);
}

/*************************************************
 * Step one desk to the left
 */
static int stepLeft(void)
{
    int deskX, deskY=currentDeskY ;
    
    if(currentDesk >= vwDESK_PRIVATE1)
    {   /* on a private desktop - go to last */
        deskX = nDesksX;
        deskY = nDesksY;
    }
    else if((deskX=currentDeskX - 1) < 1) {
        if(!deskWrap)
            return 0;
        deskX = nDesksX;
    }
    return changeDesk(calculateDesk(deskX,deskY),MOD_STEPLEFT);
}

/*************************************************
 * Step one desk down
 */
static int stepDown(void)
{
    int deskX=currentDeskX, deskY ;
    
    if(currentDesk >= vwDESK_PRIVATE1)
    {   /* on a private desktop - go to first */
        deskX = 1;
        deskY = 1;
    }
    else if((deskY = currentDeskY + 1) > nDesksY) {
        if(!deskWrap)
            return 0;
        deskY = 1;
    }
    return changeDesk(calculateDesk(deskX,deskY),MOD_STEPDOWN);
}

/*************************************************
 * Step one desk up
 */
static int stepUp(void)
{
    int deskX=currentDeskX, deskY ;
    
    if(currentDesk >= vwDESK_PRIVATE1)
    {   /* on a private desktop - go to last */
        deskX = nDesksX;
        deskY = nDesksY;
    }
    else if((deskY = currentDeskY - 1) < 1) {
        if(!deskWrap)
            return 0;
        deskY = nDesksY;
    }
    return changeDesk(calculateDesk(deskX,deskY),MOD_STEPUP);
}

/*************************************************
 * Converts an icon to a bitmap representation
 */
static HBITMAP createBitmapIcon(HICON anIcon)
{
    HDC aHDC = GetDC(hWnd);
    HDC aCHDC = CreateCompatibleDC(aHDC);
    HBITMAP hBMP = CreateCompatibleBitmap(aHDC, 13, 13);
    RECT rect ;
    hBMP = SelectObject(aCHDC, hBMP);    
    rect.top = rect.left = 0 ;
    rect.bottom = rect.right = 13 ;
    FillRect(aCHDC,&rect,GetSysColorBrush(COLOR_WINDOW));
    if(DrawIconEx(aCHDC, -1, -1, anIcon, 15, 15, 0, NULL, DI_NORMAL) == 0)
    {
        DeleteObject(hBMP);
        hBMP = 0;
    }
    else
    {
        hBMP = SelectObject(aCHDC, hBMP);
        iconReferenceVector[vectorPosition] = hBMP;
        vectorPosition++;
    }
    DeleteDC(aCHDC);
    ReleaseDC(hWnd, aHDC);
    
    return hBMP;
}

/*************************************************
 * Deletes bitmaps allocated when displaying window menu
 */
static void clearBitmapVector(void)
{
    while(vectorPosition > 0)
    {
        vectorPosition--;
        DeleteObject(iconReferenceVector[vectorPosition]);
    }
}

/*************************************************
 * createSortedWinList_cos creates a popup menu for the window-hotkey
 * which displays all windows in one list vertically seperated by a line.
 * first column is sticky, second is direct access and third is assign.
 * so you don't have to step through submenus.
 * 
 * Author: Christian Storm aka cosmic (Christian.Storm@Informatik.Uni-Oldenburg.de)
 */
#define vwPMENU_STICKY 0x100
#define vwPMENU_ACCESS 0x200
#define vwPMENU_ASSIGN 0x400
#define vwPMENU_MASK   0xf00

static HMENU createSortedWinList_cos(void)
{
    HMENU hMenu;         // menu bar handle
    char title[48];
    MenuItem *items[MAXWIN], *item;
    char buff[MAX_PATH];
    int i,x,y,c,d,e;
    BOOL useTitle;    // Only use title if we have more than one menu
    BOOL menuBreak;   // indicates if vertical seperator is needed
    hMenu = NULL;
    
    hMenu = CreatePopupMenu();
    
    // Don't show titles if only one menu is enabled
    if((stickyMenu+directMenu+assignMenu) == 1)
        useTitle = FALSE;
    else
        useTitle = TRUE;
    
    // create the window list
    lockMutex();
    winListUpdate() ;
    i = 0 ;
    for(c = 0; c < nWin; ++c)
    {
        // ignore owned windows and ones on a private desktop
        if((winList[c].Owner == NULL) && (winList[c].Desk < vwDESK_PRIVATE1))
        {
            HICON hSmallIcon ;
            
            GetWindowText(winList[c].Handle, buff, 30);
#ifdef NDEBUG
            sprintf(title, "%d - %s", winList[c].Desk, buff);
#else
            sprintf(title, "%d - %s (%x)", winList[c].Desk, buff,(int) winList[c].Handle);
#endif
            item = malloc(sizeof(MenuItem));
            items[i++] = item;
            item->name = strdup(title) ;
            
#if 0
            // this only works on WinNT and requires psapi.lib
            if((hSmallIcon = (HICON)GetClassLong(winList[c].Handle, GCL_HICON)) == 0)
            {
                HINSTANCE instance;
                DWORD processId;
                HANDLE process;
                GetWindowThreadProcessId(winList[c].Handle, &processId);
                process = OpenProcess(PROCESS_ALL_ACCESS,FALSE,processId);
                if(process != NULL)
                {
                    if(((instance = (HINSTANCE) GetWindowLong(winList[c].Handle,GWL_HINSTANCE)) != 0) &&
                       (GetModuleFileNameEx(process,instance,buff,MAX_PATH) != 0))
                        hSmallIcon = ExtractIcon(hInst,buff,0);
                    CloseHandle(process);
                }
            }
            if(hSmallIcon != 0)
#else
            if((hSmallIcon = (HICON)GetClassLong(winList[c].Handle, GCL_HICON)) != 0)
#endif
                item->icon = createBitmapIcon(hSmallIcon);
            else
                item->icon = 0 ;
            item->desk = winList[c].Desk;
            item->sticky = winList[c].Sticky;
            item->id = i ;
            winList[c].menuId = i ;
        }
        else
            winList[c].menuId = 0 ;
    }
    releaseMutex();
    
    // sorting using bubble sort
    for (x = 0; x < i; x++ )
    {
        for (y = 0; y<i; y++)
        {
            if( strcmp(items[x]->name, items[y]->name) < 0 )
            {
                item = items [x];
                items[x] = items[y];
                items[y] = item;
            }
        }
    }
    
    c = 0; d=1; e=0; menuBreak = FALSE;
    if(stickyMenu) {
        for (x=0; x < i; x++ )
        {
            if ((!c || c != items[x]->desk) &&d )
            {
                if(c) AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                c = items[x]->desk; d=0;
            }
            if (!e && useTitle ) {
                AppendMenu(hMenu, MF_STRING, 0, "Sticky" );
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                e=1;
            }
            AppendMenu( hMenu, MF_STRING | (items[x]->sticky ? MF_CHECKED: 0),
                        vwPMENU_STICKY | (items[x]->id), items[x]->name );
            if(items[x]->icon != 0)
                SetMenuItemBitmaps(hMenu, vwPMENU_STICKY | (items[x]->id), MF_BYCOMMAND, items[x]->icon, 0);
            d=1;
        }
    }
    
    c=0; d=1; e=0;
    if(directMenu) {
        if (stickyMenu) menuBreak = TRUE;
        for (x=0; x < i; x++ )
        {
            // accessing current desk - direct assign makes no sense
            if (items[x]->desk != currentDesk) {
                if ((!c || c != items[x]->desk)&&d)
                {
                    if(c) AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                    c = items[x]->desk; d=0;
                }
                if (!e && useTitle) {
                    if (menuBreak) {
                        AppendMenu( hMenu,
                                    MF_STRING | MF_MENUBARBREAK, 0, "Access" );
                        menuBreak = FALSE;
                    }
                    else
                        AppendMenu(hMenu, MF_STRING, 0, "Access" );
                    
                    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                    e=1;
                }
                AppendMenu( hMenu, MF_STRING | (items[x]->sticky ? MF_CHECKED: 0),
                            vwPMENU_ACCESS | (items[x]->id), items[x]->name );
                if(items[x]->icon != 0)
                    SetMenuItemBitmaps(hMenu, vwPMENU_ACCESS | (items[x]->id), MF_BYCOMMAND, items[x]->icon, 0);
                d=1;
            }
        }
    }
    
    c=0; d=1; e=0;
    if(assignMenu) {
        if (stickyMenu || directMenu) menuBreak=TRUE;
        for (x=0; x < i; x++ )
        {
            //sticky windows can't be assigned cause they're sticky :-) so leave them.out..
            //cannot assign to current Desktop
            if ((!items[x]->sticky) && (items[x]->desk != currentDesk))
            {
                if ((!c || c != items[x]->desk)&&d)
                {
                    if(c) AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                    c = items [x]->desk; d=0;
                }
                if (!e && useTitle) {
                    if ( menuBreak ) {
                        AppendMenu( hMenu, MF_STRING | MF_MENUBARBREAK, 0, "Assign" );
                        menuBreak = FALSE; d=1;
                    }
                    else
                        AppendMenu(hMenu, MF_STRING, 0, "Assign" );
                    
                    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                    e=1;
                }
                AppendMenu( hMenu, MF_STRING, (vwPMENU_ASSIGN | (items[x]->id)), items[x]->name );
                if(items[x]->icon != 0)
                    SetMenuItemBitmaps(hMenu, (vwPMENU_ASSIGN | (items[x]->id)), MF_BYCOMMAND, items[x]->icon, 0);
                d=1;
            }
        }
    }
    
    // destroy the generated window-list
    for (x=0; x<i; x++){
        free ( items[x]->name );
        free ( items[x]);
        items [x] = NULL;
    }
    
    return hMenu;
}

/*************************************************
 * Tries to find windows saved before a crash by classname matching
 */
static inline BOOL CALLBACK recoverWindowsEnumProc(HWND hwnd, LPARAM lParam) 
{
    int style = GetWindowLong(hwnd, GWL_STYLE);
    int exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    char buf[vwCLASSNAME_MAX+1];
    unsigned char Tricky;
    RECT pos;
    int info;
    
    /* any window that VirtuaWin may have lost would be a managed window
     * positioned at least -1000,-1000. Note the only different in the
     * criteria here to the main winListUpdate is that window may be
     * invisible or a toolwindow */
    GetWindowRect(hwnd,&pos) ;
    if((style & WS_CHILD) ||                                                   // No child windows
       (((style & WS_VISIBLE) == 0) ^ ((exstyle & WS_EX_TOOLWINDOW) == 0)) ||  // Must be (not visible) xor (a toolwindow)
       (((pos.left > -10000) || (pos.top > -10000) ||                          // Not a hidden position
         (pos.left < -30000) || (pos.top < -30000)) &&
        (((style & WS_VISIBLE) == 0) || (pos.left != -32000) || (pos.top != -32000))) ||
       ((GetParent(hwnd) != NULL) && (GetParent(hwnd) != desktopHWnd)))         // Only toplevel or owned by desktop
    {
        // Ignore this window
        vwLogPrint((vwLog,"Ignore  %x %d %d %d %d %d %d %d\n",(int) hwnd,
                    (pos.left > -10000),(pos.top > -10000),(style & WS_CHILD),
                    ((style & WS_VISIBLE) == 0),((exstyle & WS_EX_TOOLWINDOW) != 0),
                    (((style & WS_VISIBLE) == 0) ^ ((exstyle & WS_EX_TOOLWINDOW) != 0)),
                    (GetParent(hwnd) != NULL))) ;
        return TRUE;
    }
    
    GetClassName(hwnd,buf,vwCLASSNAME_MAX);
    buf[vwCLASSNAME_MAX] = '\0' ;
    Tricky = isTrickyWindow(buf,&pos) ;
    
    // tricky windows are changed to toolwindows, non-tricky are hidden, ignore others
    if(((exstyle & WS_EX_TOOLWINDOW) == 0) ^ (Tricky == 0))
    {
        vwLogPrint((vwLog,"Ignore  %x %d %d\n",(int) hwnd,((exstyle & WS_EX_TOOLWINDOW) == 0),(Tricky == 0))) ;
        return TRUE;
    }
            
    if(windowIsNotHung(hwnd,2000))
    {
        info = *((int *) lParam) ;
        vwLogPrint((vwLog,"Recover %x %d - %d\n",(int) hwnd,Tricky,info)) ;
        if(info < 0)
        {
            if(!Tricky)
            {
                ShowWindow(hwnd,SW_SHOWNA);
                ShowOwnedPopups(hwnd,TRUE);
            }
            else
            {
                // Restore the window mode
                SetWindowLong(hwnd, GWL_EXSTYLE, (exstyle & (~WS_EX_TOOLWINDOW))) ;  
                // Notify taskbar of the change
                PostMessage(taskHWnd, RM_Shellhook, HSHELL_WINDOWCREATED, (LPARAM) hwnd );
            }
            // Bring back to visible area, SWP_FRAMECHANGED makes it repaint
            if(pos.left != -32000)
            {
                pos.left += 20000 ;
                pos.top += 20000 ;
                if(pos.left < screenLeft)
                    pos.left = screenLeft + 10 ;
                else if(pos.left > screenRight)
                    pos.left = screenLeft + 10 ;
                if(pos.top < screenTop)
                    pos.top = screenTop + 10 ;
                else if(pos.top > screenBottom)
                    pos.top = screenTop + 10 ;
                SetWindowPos(hwnd, 0, pos.left, pos.top, 0, 0,
                             SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE ); 
            }
        }
        else
            *((int *) lParam) = info + 1 ;
    }
    return TRUE;
}


static void recoverWindows(void)
{
    int info ;
    char buff[32];
    
    info = 0 ;
    EnumWindows(recoverWindowsEnumProc,(LPARAM) &info);
    if(info > 0)
    {
        sprintf(buff, "%d hidden window%s been found, recover them?",info,(info == 1) ? " has":"s have");
        if(MessageBox(hWnd, buff,vwVIRTUAWIN_NAME, MB_YESNO|MB_ICONQUESTION) == IDYES)
        {
            info = -1 ;
            EnumWindows(recoverWindowsEnumProc,(LPARAM) &info);
        }
    }
}

/************************************************
 * This function decides what switching technique that should be used
 * and calls the appropriate switching function
 */
BOOL showHideWindow(windowType* aWindow, int shwFlags, unsigned char show)
{
    // Do nothing if we are dragging a window or the show hide state is right
    if((show != 0) ^ (aWindow->Visible != 0))
    {
        unsigned char Tricky;
        RECT pos;
        UINT swpFlags ;
        if(!windowIsNotHung(aWindow->Handle,(shwFlags & vwSHWIN_TRYHARD) ? 10000:100))
        {
            vwLogPrint((vwLog,"showHideWindow %8x %d %d %x %x %d %d %d - HUNG\n",(int) aWindow->Handle,shwFlags,show,(int)aWindow->Style,(int)aWindow->ExStyle,aWindow->Visible,aWindow->Desk,currentDesk)) ;
            return FALSE ;
        }
        GetWindowRect( aWindow->Handle, &pos );
        vwLogPrint((vwLog,"showHideWindow %8x %d %d %x %x %d %d %d - %d %d\n",(int) aWindow->Handle,shwFlags,show,(int)aWindow->Style,(int)aWindow->ExStyle,aWindow->Visible,aWindow->Desk,currentDesk,(int) pos.left,(int) pos.top)) ;
        Tricky = aWindow->Tricky ;
        if(trickyWindows && (Tricky != vwTRICKY_WINDOW))
        {
            aWindow->Tricky = ((pos.left == -32000) && (pos.top == -32000)) ? vwTRICKY_POSITION:0 ;
            // must still make the window visible in the same way it was hidden
            if(!show)
                Tricky = aWindow->Tricky ;
        }
        swpFlags = SWP_DEFERERASE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING ;
        if(!Tricky)
        {
            ShowWindow(aWindow->Handle,(show) ? SW_SHOWNA:SW_HIDE) ;
            /* if minimized dont show popups */
            if(!(aWindow->Style & WS_MINIMIZE))
                ShowOwnedPopups(aWindow->Handle,(show) ? TRUE:FALSE) ;
        }
        else if(aWindow->Owner == NULL)
        {
            // show/hide the window in the toolbar
            if(show)
            {
                // Restore the window mode
                SetWindowLong( aWindow->Handle, GWL_EXSTYLE, aWindow->ExStyle );  
                // Notify taskbar of the change
                PostMessage(taskHWnd, RM_Shellhook, HSHELL_WINDOWCREATED, (LPARAM) aWindow->Handle );
            }
            else
            {
                // This removes window from taskbar and alt+tab list
                SetWindowLong( aWindow->Handle, GWL_EXSTYLE, 
                               (aWindow->ExStyle & (~WS_EX_APPWINDOW)) | WS_EX_TOOLWINDOW);
                // Notify taskbar of the change
                PostMessage(taskHWnd, RM_Shellhook, HSHELL_WINDOWDESTROYED, (LPARAM) aWindow->Handle);
            }
            swpFlags |= SWP_FRAMECHANGED ;
        }
        if(show)
        {
            if((pos.left <= -10000) && (pos.top <= -10000) &&
               (pos.left >= -30000) && (pos.top >= -30000))
            {   // Move the window back onto visible area
                pos.left += 20000 ;
                pos.top += 20000 ;
                swpFlags |= SWP_NOMOVE ;
            }
        }
        else if((pos.left > -10000) && (pos.top > -10000))
        {   // Move the window off visible area
            pos.left -= 20000 ;
            pos.top -= 20000 ;
            swpFlags |= SWP_NOMOVE ;
        }
        // SWP_NOMOVE is set when a move IS required
        if(swpFlags & (SWP_FRAMECHANGED | SWP_NOMOVE))
            SetWindowPos(aWindow->Handle, 0, pos.left, pos.top, 0, 0, (swpFlags ^ SWP_NOMOVE)) ; 
    }
    aWindow->Visible = show;
    return TRUE ;
}


/************************************************
 * Toggles the disabled state of VirtuaWin
 */
static void disableAll(HWND aHWnd)
{
    if(enabled)
    {   // disable VirtuaWin
        strcpy(nIconD.szTip,vwVIRTUAWIN_NAME " - Disabled"); //Tooltip
        setIcon(0);
        unRegisterAllKeys();
        KillTimer(hWnd, 0x29a);
        enabled = FALSE;
    }
    else
    {   // Enable VirtuaWin
        strcpy(nIconD.szTip,vwVIRTUAWIN_NAME_VERSION);	// Tooltip
        setIcon(currentDesk);
        registerAllKeys();
        SetTimer(hWnd, 0x29a, 250, monitorTimerProc);
        enabled = TRUE;
    }
}

/************************************************
 * Assigns a window to the specified desktop
 * Used by the module message VW_ASSIGNWIN
 */
int assignWindow(HWND theWin, int theDesk, BOOL force)
{
    int ret ;
    vwLogPrint((vwLog,"Assign window: %x %d %d\n",(int) theWin,theDesk,force)) ;
    if((theWin == NULL) || (theWin == hWnd) ||
       (((theDesk > (nDesksY * nDesksX)) || (theDesk < 1)) && !force))
        return 0 ; // Invalid window or desk
    
    lockMutex();
    winListUpdate() ;
    ret = windowSetDesk(theWin,theDesk,1) ;
    releaseMutex();
    return ret ;
}

/************************************************
 * Changes sticky state of a window
 * Used by the module message VW_SETSTICKY
 */
static int setSticky(HWND theWin, int state)
{
    int ret ;
    vwLogPrint((vwLog,"Set sticky window: %x %d\n",(int) theWin,state)) ;
    if(theWin == NULL)
        return 0 ;
    
    lockMutex();
    winListUpdate() ;
    ret = windowSetSticky(theWin,state) ;
    releaseMutex();
    return ret ;
}

/************************************************
 * Dismisses the current window by either moving it
 * back to its assigned desk or minimizing.
 */
static int windowDismiss(HWND theWin)
{
    int ret, idx, activeZOrder ;
    HWND hwnd ;
    
    vwLogPrint((vwLog,"Dismissing window: %x\n",(int) theWin)) ;
    if(theWin == NULL)
        return 0 ;
    
    while((GetWindowLong(theWin, GWL_STYLE) & WS_CHILD) && 
          ((hwnd = GetParent(theWin)) != NULL) && (hwnd != desktopHWnd))
        theWin = hwnd ;

    lockMutex();
    winListUpdate();
    if(((idx = winListFind(theWin)) >= 0) && (winList[idx].Visible == vwVISIBLE_YESTEMP))
        ret = windowSetDesk(theWin,winList[idx].Desk,1) ;
    else
    {
        ret = CloseWindow(theWin) ;
        /* need to change the focus to another window */
        hwnd = NULL;
        activeZOrder = 0;
        for (idx = 0; idx < nWin ; ++idx)
        {
            if((winList[idx].Desk == currentDesk) &&
               (winList[idx].Handle != theWin) && (winList[idx].Owner != theWin) && 
               ((winList[idx].Style & (WS_MINIMIZE|WS_VISIBLE)) == WS_VISIBLE) && 
               (winList[idx].ZOrder[currentDesk] > activeZOrder))
            {
                hwnd = winList[idx].Handle;
                activeZOrder = winList[idx].ZOrder[currentDesk];
            }
        }
        setForegroundWin(hwnd,TRUE) ;
    }
    releaseMutex();
    return ret ;
}

/*************************************************
 * Pops up and handles the window list menu
 */
static void winListPopupMenu(HWND aHWnd)
{
    HMENU hpopup;
    POINT pt;
    HWND hwnd ;
    int retItem, id, ii;
    
    hpopup = createSortedWinList_cos();
    GetCursorPos(&pt);
    SetForegroundWindow(aHWnd);
    
    retItem = TrackPopupMenu(hpopup, TPM_RETURNCMD |  // Return menu code
                             TPM_LEFTBUTTON, (pt.x-2), (pt.y-2), // screen coordinates
                             0, aHWnd, NULL);
    
    if(retItem)
    {
        id = retItem & ~vwPMENU_MASK ;
        ii = nWin ;
        while(--ii >= 0)
            if(winList[ii].menuId == id)
                break ;
        if(ii >= 0)
        {
            hwnd = winList[ii].Handle ;
            vwLogPrint((vwLog,"Menu select %x %d %x\n",retItem,ii,(int) hwnd)) ;
            if(retItem & vwPMENU_STICKY)
                // Sticky toggle
                setSticky(hwnd,-1) ;
            else if(retItem & vwPMENU_ACCESS)
            {   // window access
                gotoDesk(winList[ii].Desk,FALSE);
                setForegroundWin(hwnd,0);
            } 
            else
            {   // Assign to this desktop
                assignWindow(hwnd,currentDesk,TRUE) ;
                setForegroundWin(hwnd,0) ;
            }
        }
    }
    
    PostMessage(aHWnd, 0, 0, 0);  // see above
    DestroyMenu(hpopup);       // Delete loaded menu and reclaim its resources
    clearBitmapVector();
}

/*************************************************
 * Main window callback, this is where all main window messages are taken care of
 */
static LRESULT CALLBACK
wndProc(HWND aHWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static UINT taskbarRestart; 
    POINT pt;
    
    switch (message)
    {
    case VW_MOUSEWARP:
        // Try to avoid switching if we press on the taskbar
        if((LOWORD(lParam) && (GetForegroundWindow() == FindWindow("Shell_traywnd", ""))))
            goto skipMouseWarp; // if so, skip whole sequence
        // Is virtuawin enabled
        if(enabled)
        {
            // Are we using a mouse key
            if(useMouseKey && !HIWORD(GetAsyncKeyState(MOUSEKEY)))
                // If key not pressed skip whole sequence
                goto skipMouseWarp;
            
            // Suspend mouse thread during message processing, 
            // otherwise we might step over several desktops
            SuspendThread(mouseThread); 
            GetCursorPos(&pt);
            
            switch HIWORD(lParam)
            {
            case VW_MOUSELEFT:
                warpMultiplier++;
                if((warpMultiplier >= configMultiplier)) {
                    isDragging = LOWORD(lParam);
                    if(stepLeft() != 0) {
                        isDragging = FALSE;
                        if(noMouseWrap)
                            SetCursorPos(pt.x + warpLength, pt.y);
                        else
                            SetCursorPos(screenRight-warpLength, pt.y);
                    }
                    warpMultiplier = 0;
                }
                ResumeThread(mouseThread);
                break;
                
            case VW_MOUSERIGHT:
                warpMultiplier++;
                if((warpMultiplier >= configMultiplier)) {
                    isDragging = LOWORD(lParam);
                    if(stepRight() != 0) {
                        isDragging = FALSE;
                        if(noMouseWrap)
                            SetCursorPos(pt.x - warpLength, pt.y);
                        else
                            SetCursorPos(screenLeft+warpLength, pt.y);
                    }
                    warpMultiplier = 0;
                }
                ResumeThread(mouseThread);
                break;
                
            case VW_MOUSEUP:
                warpMultiplier++;
                if((warpMultiplier >= configMultiplier)) {
                    int switchVal;
                    isDragging = LOWORD(lParam);
                    if(invertY)
                        switchVal = stepDown();
                    else
                        switchVal = stepUp();
                    if(switchVal != 0) {
                        isDragging = FALSE;
                        if(noMouseWrap)
                            SetCursorPos(pt.x, pt.y + warpLength);
                        else
                            SetCursorPos(pt.x, screenBottom-warpLength);
                    }
                    warpMultiplier = 0;
                }
                ResumeThread(mouseThread);
                break;
                
            case VW_MOUSEDOWN:
                warpMultiplier++;
                if((warpMultiplier >= configMultiplier)) {
                    int switchVal;
                    isDragging = LOWORD(lParam);
                    if(invertY)
                        switchVal = stepUp();
                    else
                        switchVal = stepDown();
                    if(switchVal != 0) {
                        isDragging = FALSE;
                        if(noMouseWrap)
                            SetCursorPos(pt.x, pt.y - warpLength);
                        else
                            SetCursorPos(pt.x, screenTop+warpLength);
                    }
                    warpMultiplier = 0;
                }
                ResumeThread(mouseThread);
                break;
                
            case VW_MOUSERESET:
                warpMultiplier = 0;
            }
            ResumeThread(mouseThread);
        }
skipMouseWarp:  // goto label for skipping mouse stuff
        
        return TRUE;
        
    case WM_HOTKEY:				// A hot key was pressed
        // Cycling hot keys
        if(wParam == cyclingKeyUp)
            stepDelta(1) ;
        else if(wParam == cyclingKeyDown)
            stepDelta(-1) ;
        // Desk switch hot keys
        else if(wParam == vwLeft)
            stepLeft();
        else if(wParam == vwRight)
            stepRight();
        else if(wParam == vwUp)
        {
            if(invertY)
                stepDown();
            else
                stepUp();
        }
        else if(wParam == vwDown)
        {
            if(invertY)
                stepUp();
            else
                stepDown();
        }
        else if(wParam == stickyKey)
            setSticky(GetForegroundWindow(),-1);
        else if(wParam == dismissKey)
            windowDismiss(GetForegroundWindow());
        else if(wParam == vwMenu)
        {
            if(enabled)
                winListPopupMenu(aHWnd) ;
        }
        else
        {
            // Desktop Hot keys
            int ii=MAXDESK-1 ;
            do {
                if(wParam == vwDesk[ii])
                {
                    gotoDesk(ii,TRUE);
                    break ;
                }
            } while(--ii > 0) ;
        }
        return TRUE;
        
        // Plugin messages
    case VW_CHANGEDESK: 
        switch (wParam) {
        case VW_STEPLEFT:
            stepLeft();
            break;
        case VW_STEPRIGHT:
            stepRight();
            break;
        case VW_STEPUP:
            stepUp();
            break;
        case VW_STEPDOWN:
            stepDown();
            break;
        default:
            gotoDesk(wParam,FALSE);
            break;
        }
        return TRUE;
        
    case VW_CLOSE: 
        shutDown();
        return TRUE;
        
    case VW_SETUP:
        showSetup();
        return TRUE;
        
    case VW_DELICON:
        Shell_NotifyIcon(NIM_DELETE, &nIconD); // This removes the icon
        return TRUE;
        
    case VW_SHOWICON:
        Shell_NotifyIcon(NIM_ADD, &nIconD);    // This re-adds the icon
        return TRUE;
        
    case VW_HELP:
        showHelp(aHWnd);
        return TRUE;
        
    case VW_GATHER:
        showAll(0);
        return TRUE;
        
    case VW_DESKX:
        return nDesksX;
        
    case VW_DESKY:
        return nDesksY;
        
    case VW_CURDESK:
        return currentDesk;
        
    case VW_ASSIGNWIN:
        return assignWindow((HWND)wParam,(int)lParam,FALSE);
        
    case VW_SETSTICKY:
        return setSticky((HWND)wParam, (int)lParam);
        
    case VW_FOREGDWIN:
        {
            int index ;
            if((index = winListFind((HWND)wParam)) < 0)
                return 0;
            /* found the window, if visible bring to the front otherwise set the zorder */
            if(lParam == 0)
                lParam = currentDesk ;
            if((lParam == currentDesk) && winList[index].Visible)
                setForegroundWin((HWND)wParam,0);
            winList[index].ZOrder[lParam] = vwZOrder ;
            return 1;
        }
    
    case VW_WINLIST:
        {
            // Send over the window list with WM_COPYDATA
            COPYDATASTRUCT cds;         
            lockMutex();
            winListUpdate() ;
            cds.dwData = nWin;
            cds.cbData = sizeof(winList);
            cds.lpData = (void*)winList;
            releaseMutex();
            sendModuleMessage(WM_COPYDATA, (WPARAM) aHWnd, (LPARAM)&cds); 
            return TRUE;
        }
        
    case VW_INSTALLPATH:
    case VW_USERAPPPATH:
        {
            // Send over the VirtuaWin install path with WM_COPYDATA
            COPYDATASTRUCT cds;
            char *ss = (message == VW_INSTALLPATH) ? VirtuaWinPath:UserAppPath ;
            cds.dwData = 0 - message ;
            cds.cbData = strlen(ss) + 1;
            cds.lpData = (void*)ss;
            sendModuleMessage(WM_COPYDATA, (WPARAM) aHWnd, (LPARAM)&cds); 
            return TRUE;
        }

        // End plugin messages
        
    case WM_CREATE:		       // when main window is created
        // register message for explorer/systray crash restart
        // only works with >= IE4.0 
        taskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
        return TRUE;
        
    case WM_ENDSESSION:
        if(wParam)
            shutDown();
        return TRUE;
        
    case WM_DESTROY:	  // when application is closed
        shutDown();            
        return TRUE;
        
    case UWM_SYSTRAY:		   // We are being notified of mouse activity over the icon
        switch (lParam)
        {
        case WM_LBUTTONDOWN:               // Show the window list
            if(enabled)
                winListPopupMenu(aHWnd) ;
            break;
        
        case WM_LBUTTONDBLCLK:             // double click on icon
            showSetup();
            break;
            
        case WM_MBUTTONUP:		   // Move to the next desktop
            stepDelta(1) ;
            break;
            
        case WM_RBUTTONUP:		   // Let's track a popup menu
            if(setupOpen)
            {
                /* dont allow access to this menu while setup is open as its
                 * behaviour is less predictable (shutting down while setup
                 * is open will hang the virtuawin process) */
                SetForegroundWindow(hWnd);
            }
            else
            {
                HMENU hmenu, hpopup;
                GetCursorPos(&pt);
                hmenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1));
                
                hpopup = GetSubMenu(hmenu, 0);
                
                UINT nID = GetMenuItemID(hpopup, 3); // Get the Disable item
                if(enabled) // Change the text depending on state
                    ModifyMenu(hpopup, nID, MF_BYCOMMAND, nID, "Disable");
                else
                    ModifyMenu(hpopup, nID, MF_BYCOMMAND, nID, "Enable");
                SetForegroundWindow(aHWnd);
                
                switch (TrackPopupMenu(hpopup, TPM_RETURNCMD |    // Return menu code
                                       TPM_RIGHTBUTTON, (pt.x-2), (pt.y-2), // screen coordinates
                                       0, aHWnd, NULL))
                {
                case ID_SETUP:		// show setup box
                    showSetup();
                    break;
                case ID_EXIT:		// exit application
                    DestroyWindow(aHWnd);
                    break;
                case ID_FORWARD:	// move to the next desktop
                    stepDelta(1) ;
                    break;
                case ID_BACKWARD:	// move to the previous desktop
                    stepDelta(-1) ;
                    break;
                case ID_GATHER:		// gather all windows
                    showAll(0);
                    break;
                case ID_DISABLE:	// Disable VirtuaWin
                    disableAll(aHWnd);
                    break;
                case ID_HELP:		// show help
                    showHelp(aHWnd);
                    break;
                }
                PostMessage(aHWnd, 0, 0, 0);	
                DestroyMenu(hpopup);  // Delete loaded menu and reclaim its resources
                DestroyMenu(hmenu);		
            }
            break;
        }
        return TRUE;
        
    case WM_DISPLAYCHANGE:
        getScreenSize();
        return TRUE;
    case WM_SETTINGCHANGE:
        getTaskbarLocation();
        return TRUE;
        
    default:
        // If taskbar restarted
        if((message == taskbarRestart) && displayTaskbarIcon )
        {
            Shell_NotifyIcon(NIM_ADD, &nIconD);	// This re-adds the icon
            goGetTheTaskbarHandle();
        }
        break;
    }
    return DefWindowProc(aHWnd, message, wParam, lParam);
}

/*************************************************
 * VirtuaWin start point
 */
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    WNDCLASSEX wc;
    DWORD threadID;
    BOOL awStore;
    char *classname = vwVIRTUAWIN_NAME "MainClass";
    hInst = hInstance;
    
    /* Only one instance may be started */
    hMutex = CreateMutex(NULL, FALSE, vwVIRTUAWIN_NAME "PreventSecond");
    
    if(GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // Display configuration window...
        PostMessage( FindWindow(classname, NULL), VW_SETUP, 0, 0);
        return 0; // ...and quit 
    }
    
#ifndef NDEBUG
    vwLog = fopen("c:\\" vwVIRTUAWIN_NAME ".log","w+") ;
#endif
    
    /* Create a window class for the window that receives systray notifications.
       The window will never be displayed */
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = wndProc;
    wc.cbClsExtra = wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIRTWIN));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = classname;
    wc.hIconSm = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_VIRTWIN), IMAGE_ICON,
                                   GetSystemMetrics(SM_CXSMICON),
                                   GetSystemMetrics(SM_CYSMICON), 0);
    RegisterClassEx(&wc);
    
    getScreenSize();
    
    /* set the window to give focus to when releasing focus on switch also used to refresh */
    desktopHWnd = GetDesktopWindow();
    
    readConfig();	// Read the config file
    getTaskbarLocation(); // This is dependent on the config
    
    // Fix some things for the alternate hide method
    RM_Shellhook = RegisterWindowMessage("SHELLHOOK");
    goGetTheTaskbarHandle();
    
    loadIcons();
    
    // Load tricky windows, must be done before the crashRecovery checks
    if(trickyWindows)
        curTricky = loadTrickyList(trickyList) ;
    
    /* Now, check for any windows that may have been lost from last time */
    recoverWindows();
    
    /* Create window. Note that WS_VISIBLE is not used, and window is never shown. */
    hWnd = CreateWindowEx(0, classname, classname, WS_POPUP, CW_USEDEFAULT, 0,
                          CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
    
    nIconD.cbSize = sizeof(NOTIFYICONDATA); // size
    nIconD.hWnd = hWnd;		    // window to receive notifications
    nIconD.uID = 1;		    // application-defined ID for icon (can be any UINT value)
    nIconD.uFlags = NIF_MESSAGE |   // nIconD.uCallbackMessage is valid, use it
          NIF_ICON |		    // nIconD.hIcon is valid, use it
          NIF_TIP;		    // nIconD.szTip is valid, use it
    nIconD.uCallbackMessage = UWM_SYSTRAY;  // message sent to nIconD.hWnd
    nIconD.hIcon = icons[1];
    
    strcpy(nIconD.szTip,vwVIRTUAWIN_NAME_VERSION);		// Tooltip
    if( displayTaskbarIcon )
    {
        // This adds the icon
        if(Shell_NotifyIcon(NIM_ADD, &nIconD) == 0)
        {
            BOOL retry = 1;
            do
            {
                Sleep(2000);
                // Maybe Systray process hasn't started yet, try again
                retry = Shell_NotifyIcon(NIM_ADD, &nIconD);  // This adds the icon
            }
            while(retry != 0);
        }
    }
    /* Register the keys */
    registerAllKeys();
    setMouseKey();
    
    /* Load some stuff */
    curSticky   = loadStickyList(stickyList);
    curAssigned = loadAssignedList(assignedList);
    curUser     = loadUserList(userList);  // load any user defined windows
    
    /* always move windows immediately on startup */
    awStore = assignImmediately ;
    assignImmediately = TRUE ;
    lockMutex();
    winListUpdate() ;
    releaseMutex();
    assignImmediately = awStore ;
    
    /* Load user modules */
    curDisabledMod = loadDisabledModules(disabledModules);
    loadModules();
    
    /* Create the thread responsible for mouse monitoring */   
    mouseThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MouseProc, NULL, 0, &threadID); 	
    if(!mouseEnable) // Suspend the thread if no mouse support
        enableMouse(FALSE);
    //SuspendThread(mouseThread);
    
    // if on win9x the tricky windows need to be continually hidden
    taskbarFixRequired = (GetVersion() >= 0x80000000) ;
    SetTimer(hWnd, 0x29a, 250, monitorTimerProc); 

    /* Main message loop */
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    CloseHandle(hMutex);
    return msg.wParam;
}

/*
 * $Log$
 * Revision 1.50  2005/08/10 15:35:34  rexkerr	
 * Fixed a bug that was causing the right edge or bottom edge of the screen to be detected incorrectly when the secondary monitor was to the left of or above the primary monitor.	
 *
 * Revision 1.49  2005/08/02 22:05:34  rexkerr	
 * Fixed grammar error when recovering windows.  It now properly says "# windows were recovered" or "1 window was recovered"	
 *
 * Revision 1.48  2005/07/29 06:37:59  rexkerr	
 * Updated mousekeys functionality to require a motion tendency towards the edge of the screen to prevent the desktop from swapping unintentionally just because the mouse is near the end of the screen when the meta key is pressed.	
 *
 * Revision 1.47  2005/07/21 19:55:11  jopi
 * SF 1204278, Help path was not initialized correctly after the multi user changes.
 *
 * Revision 1.46  2005/06/11 20:59:48  jopi
 * SF1205908, periodic check that moved application doesn't reappear in the taskbar
 *
 * Revision 1.45  2005/03/10 08:02:11  rexkerr
 * Added multi-user support
 *
 * Revision 1.44  2005/02/16 07:44:20  jopi
 * Removed unused variable
 *
 * Revision 1.43  2005/02/04 11:04:41  jopi
 * SF936865, use virtual sceensize for mouse switching instead since multimonitor setups would switch desktop prematurely otherwise.
 *
 * Revision 1.42  2004/12/07 19:18:42  jopi
 * SF1053738, added application icons to the window list
 *
 * Revision 1.41  2004/04/10 10:20:01  jopi
 * Updated to compile with gcc/mingw
 *
 * Revision 1.40  2004/02/28 23:50:26  jopi
 * SF905625 Added module message for changing the sticky state of a window
 *
 * Revision 1.39  2004/02/28 18:54:01  jopi
 * SF904069 Added possibility to choose if sticky should be permanent for all instances of the same classname.
 *
 * Revision 1.38  2004/01/10 11:23:43  jopi
 * When assigning a visible window to the current desktop it would be lost
 *
 * Revision 1.37  2004/01/10 11:15:52  jopi
 * Updated copyright for 2004
 *
 * Revision 1.36  2004/01/10 11:04:12  jopi
 * Changed what windows that should be handled, was some problems with some windows
 *
 * Revision 1.35  2003/09/24 19:26:28  jopi
 * SF770859 Window menu heading will not be displayed if only one meny is used
 *
 * Revision 1.34  2003/07/08 21:10:28  jopi
 * SF745820, excluded some special types of windows from beeing handled by VirtuaWin
 *
 * Revision 1.33  2003/06/26 19:56:52  jopi
 * Added module support for assigning a window to specified desktop
 *
 * Revision 1.32  2003/06/24 19:52:04  jopi
 * SF693876 Fixed option to handle XP skinned style taskbars
 *
 * Revision 1.31  2003/04/09 16:47:58  jopi
 * SF710500, removed all the old menu handling code to make menus work the same independently of numner of menus used.
 *
 * Revision 1.30  2003/03/10 20:48:18  jopi
 * Changed so that doubleclick will bring up setup and added a disabled menu item instead.
 *
 * Revision 1.29  2003/03/10 19:23:43  jopi
 * Fixed so that we retry to add the systray icon incase of failure, we might be in a position where we try to add the icon before the systray process is started.
 *
 * Revision 1.28  2003/02/27 19:57:01  jopi
 * Old taskbar position was not deleted if taskbar position moved during operation. Also improved left/right/up/down taskbar position detection, seems like we can have negative coordinates on the position.
 *
 * Revision 1.27  2003/01/27 20:22:59  jopi
 * Updated copyright header for 2003
 *
 * Revision 1.26  2002/12/23 15:42:24  jopi
 * Added config options to disable taskbar detection and the alternative hiding technique.
 *
 * Revision 1.25  2002/12/21 08:44:20  jopi
 * The "tricky" windows was not moved away far enough from the screen so you could see a small grey bar at the screen bottom.
 *
 * Revision 1.24  2002/09/26 21:00:50  Johan Piculell
 * Added mutex protection for the window list
 *
 * Revision 1.23  2002/06/15 11:25:13  Johan Piculell
 * Now we try to locate the MSTaskSwWClass even if it is a direct child of Shell_TrayWnd, this will make it work on more windows version since this differs sometimes.
 *
 * Revision 1.22  2002/06/15 11:17:50  Johan Piculell
 * Fixed so that window coordinates are reloaded when resolution is changed, and also so that taskbar location is reloaded if moved.
 *
 * Revision 1.21  2002/06/11 20:10:27  Johan Piculell
 * Improved the window menus so that unnecessary menus and items won't show and they all have a lable. Fixes by Ulf Jaenicke-Roessler.
 *
 * Revision 1.20  2002/06/11 19:43:57  Johan Piculell
 * Removed the MF_POPUP flag from the window menus since they shouldn't be created like this. Fixed by Ulf Jaenicke-Roessler.
 *
 * Revision 1.19  2002/06/01 21:15:22  Johan Piculell
 * Multiple fixes by Christian Storm.
 *
 * Revision 1.18  2002/06/01 19:33:33  Johan Piculell
 * *** empty log message ***
 *
 * Revision 1.17  2002/02/14 21:23:41  Johan Piculell
 * Updated copyright header
 *
 * Revision 1.16  2001/12/19 17:34:34  Johan Piculell
 * Classname will now always be "VirtuaWinMainClass" and not version dependent.
 *
 * Revision 1.15  2001/12/01 00:05:52  Johan Piculell
 * Added alternative window hiding for troublesome windows like InternetExplorer
 *
 * Revision 1.14  2001/11/12 21:39:15  Johan Piculell
 * Added functionality for disabling the systray icon
 *
 * Revision 1.13  2001/11/12 20:11:47  Johan Piculell
 * Display setup dialog if started a second time instead of just quit
 *
 * Revision 1.12  2001/11/12 18:33:42  Johan Piculell
 * Fixed so that user windows are also checked if they are saved as sticky.
 *
 * Revision 1.11  2001/02/05 21:13:08  Administrator
 * Updated copyright header
 *
 * Revision 1.10  2001/01/28 16:26:56  Administrator
 * Configuration behaviour change. It is now possible to test all settings by using apply and all changes will be rollbacked if cancel is pressed
 *
 * Revision 1.9  2001/01/14 16:27:42  Administrator
 * Moved io.h include to DiskRoutines.c
 *
 * Revision 1.8  2001/01/12 18:11:25  Administrator
 * Moved some disk stuff from VirtuaWin to DiskRoutines
 *
 * Revision 1.7  2001/01/12 16:58:11  Administrator
 * Added module message for getting the current desktop number
 *
 * Revision 1.6  2000/12/11 21:28:41  Administrator
 * Fixed the sticky symbol in the winlist again, got lost during some changes
 *
 * Revision 1.5  2000/08/28 21:38:37  Administrator
 * Added new functions for menu hot key registration. Fixed bug with needing to have hot keys enabled for menu keys to work and also better error message
 *
 * Revision 1.4  2000/08/19 15:00:26  Administrator
 * Added multiple user setup support (Alasdair McCaig) and fixed creation of setup file if it don't exist
 *
 * Revision 1.3  2000/08/18 23:43:08  Administrator
 *  Minor modifications by Matti Jagula <matti@proekspert.ee> List of modifications follows: Added window title sorting in popup menus (Assign, Direct, Sticky) Added some controls to Setup Misc tab and support for calling the popup menus from keyboard.
 *
 * Revision 1.2  2000/08/18 21:41:31  Administrator
 * Added the code again that removes closed windows, this will avoid having closed child windows reappearing again. Also updated the mail adress
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  Administrator
 * Added first time
 *
 */
