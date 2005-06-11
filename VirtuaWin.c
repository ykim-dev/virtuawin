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
#include "ModuleRoutines.h"

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commctrl.h>
#include <math.h>

HANDLE hMutex;

/*************************************************
 * VirtuaWin start point
 */
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   MSG msg;
   WNDCLASSEX wc;
   DWORD threadID;
   char *classname = "VirtuaWinMainClass";
   hInst = hInstance;

   /* Only one instance may be started */
   hMutex = CreateMutex(NULL, FALSE, "PreventSecondVirtuaWin");

   if(GetLastError() == ERROR_ALREADY_EXISTS)
   {
      // Display configuration window...
      PostMessage( FindWindow(classname, NULL), VW_SETUP, 0, 0);
      return 0; // ...and quit 
   }

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
   releaseHnd = GetDesktopWindow();
  
   readConfig();	// Read the config file
   getTaskbarLocation(); // This is dependent on the config
   
   // Fix some things for the alternate hide method
   RM_Shellhook = RegisterWindowMessage("SHELLHOOK");
   goGetTheTaskbarHandle();

   loadIcons();
   
   // Load tricky windows, must be done before the crashRecovery
   if(trickyWindows)
      curTricky   = loadTrickyList( trickyList );
   
   /* Now, set the lock file */
   if(crashRecovery) {
      if(!tryToLock())
         if(MessageBox(hWnd, "VirtuaWin was not properly shut down.\n Would you try to recover your windows?", "VirtuaWin", MB_YESNO | MB_SYSTEMMODAL) == IDYES) {
            recoverWindows();
         }
   }
  
   /* Create window. Note that WS_VISIBLE is not used, and window is never shown */
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

   strcpy(nIconD.szTip, appName);		// Tooltip
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
   curSticky   = loadStickyList( stickyList );
   curAssigned = loadAssignedList( assignedList );

   initData();			      // init window list
   EnumWindows(enumWindowsProc, 0);   // get all windows
   lockMutex();
   saveDesktopState(&nWin, winList);  // Let's save them now
   releaseMutex();
   
   curUser = loadUserList(userList);  // load any user defined windows
   if(curUser != 0)
      userDefinedWin = TRUE;
   if(userDefinedWin)                 // locate windows, if any
      findUserWindows();

   /* Load user modules */
   curDisabledMod = loadDisabledModules(disabledModules);
   loadModules();

   /* Create the thread responsible for mouse monitoring */   
   mouseThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MouseProc, NULL, 0, &threadID); 	
   if(!mouseEnable) // Suspend the thread if no mouse support
      enableMouse(FALSE);
   //SuspendThread(mouseThread);
   
   /* Main message loop */
   while (GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
   
   CloseHandle(hMutex);
   return msg.wParam;
}

/*************************************************
 * The mouse thread. This function runs in a thread and checks the mouse
 * position every 50ms. If mouse support is disabled, the thread will be in 
 * suspended state.
 */
DWORD WINAPI MouseProc(LPVOID lpParameter)
{
   POINT pt;
   
   while(1) {
      Sleep(50);
      GetCursorPos(&pt);
      if(     pt.x < (screenLeft   + 3 + (taskBarWarp * taskBarLeftWarp   * checkMouseState()))) {
         // switch left
         SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, 
                           MAKELPARAM(checkMouseState(), VW_MOUSELEFT));
      }
      else if(pt.x > (screenRight  - 3 - (taskBarWarp * taskBarRightWarp  * checkMouseState()))) { 
         // switch right
         SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, 
                           MAKELPARAM(checkMouseState(), VW_MOUSERIGHT));
      }
      else if(pt.y < (screenTop    + 3 + (taskBarWarp * taskBarTopWarp    * checkMouseState()))) { 
         // switch up
         SendNotifyMessage(hWnd, VW_MOUSEWARP, 0, 
                           MAKELPARAM(checkMouseState(), VW_MOUSEUP));
      }
      else if(pt.y > (screenBottom - 3 - (taskBarWarp * taskBarBottomWarp * checkMouseState()))) {
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

/*************************************************
 * Checks if mouse key modifier is pressed
 */
inline BOOL checkMouseState()
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

/************************ *************************
 * Turns on/off the mouse thread. Makes sure that the the thread functions
 * only is called if needed.
 */
void enableMouse( BOOL turnOn )
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

/************************ *************************
 * Loads the icons for the systray according to the current setup
 */
void loadIcons() {
  int xIcon = GetSystemMetrics(SM_CXSMICON);
  int yIcon = GetSystemMetrics(SM_CYSMICON);

  /* Try to load user defined icons */
  icons[0] = (HICON) LoadImage(hInst, "icons/0.ico", IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE);
  icons[1] = (HICON) LoadImage(hInst, "icons/1.ico", IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE);
  icons[2] = (HICON) LoadImage(hInst, "icons/2.ico", IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE);
  icons[3] = (HICON) LoadImage(hInst, "icons/3.ico", IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE);
  icons[4] = (HICON) LoadImage(hInst, "icons/4.ico", IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE);
  icons[5] = (HICON) LoadImage(hInst, "icons/5.ico", IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE);
  icons[6] = (HICON) LoadImage(hInst, "icons/6.ico", IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE);
  icons[7] = (HICON) LoadImage(hInst, "icons/7.ico", IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE);
  icons[8] = (HICON) LoadImage(hInst, "icons/8.ico", IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE);
  icons[9] = (HICON) LoadImage(hInst, "icons/9.ico", IMAGE_ICON, xIcon, yIcon, LR_LOADFROMFILE);

  /* Create all icons for the system tray */
  if(nDesksY == 2 && nDesksX == 2) { // if 2 by 2 mode
    if(!icons[0])
      icons[0] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_SMALL_DIS), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[1])
      icons[1] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_SMALL_NW), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[2])
      icons[2] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_SMALL_NE), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[3])
      icons[3] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_SMALL_SW), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[4])
      icons[4] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_SMALL_SE), IMAGE_ICON, xIcon, yIcon, 0);
  } else { // otherwise load 1-9 icons
    if(!icons[0])
      icons[0] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON0), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[1])
      icons[1] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[2])
      icons[2] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON2), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[3])
      icons[3] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON3), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[4])
      icons[4] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON4), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[5])
      icons[5] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON5), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[6])
      icons[6] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON6), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[7])
      icons[7] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON7), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[8])
      icons[8] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON8), IMAGE_ICON, xIcon, yIcon, 0);
    if(!icons[9])
      icons[9] = (HICON) LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON9), IMAGE_ICON, xIcon, yIcon, 0);
  }
}

/*************************************************
 * Resets all icons and reloads them
 */
void reLoadIcons()
{
  int i;
  for(i = 0; i < 9; ++i)
    icons[i] = NULL;
  loadIcons();
  setIcon(calculateDesk());
}

/*************************************************
 * Register the keys to use for switching desktop with the arrow keys
 */
BOOL registerKeys()
{
  if(keyEnable && !keysRegistred) {
    keysRegistred = TRUE;
    setKeyMod();
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
void unRegisterKeys()
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
 * Register the hotkeys to use for switching desktop and the winlist hotkey
 */
BOOL registerHotKeys()
{
   if(hotKeyEnable && !hotKeysRegistred) {
      hotKeysRegistred = TRUE;
      if(hotkey1) {
         vw1 = GlobalAddAtom("atomKey1");
         if((RegisterHotKey(hWnd, vw1, hotKey2ModKey(hotkey1Mod) | hotkey1Win, hotkey1) == FALSE))
            return FALSE;
      }
      if(hotkey2) {
         vw2 = GlobalAddAtom("atomKey2");
         if((RegisterHotKey(hWnd, vw2, hotKey2ModKey(hotkey2Mod) | hotkey2Win, hotkey2) == FALSE))
            return FALSE;
      }
      if(hotkey3) {
         vw3 = GlobalAddAtom("atomKey3");
         if((RegisterHotKey(hWnd, vw3, hotKey2ModKey(hotkey3Mod) | hotkey3Win, hotkey3) == FALSE))
            return FALSE;
      }
      if(hotkey4) {
         vw4 = GlobalAddAtom("atomKey4");
         if((RegisterHotKey(hWnd, vw4, hotKey2ModKey(hotkey4Mod) | hotkey4Win, hotkey4) == FALSE))
            return FALSE;
      }
      if(hotkey5) {
         vw5 = GlobalAddAtom("atomKey5");
         if((RegisterHotKey(hWnd, vw5, hotKey2ModKey(hotkey5Mod) | hotkey5Win, hotkey5) == FALSE))
            return FALSE;
      }
      if(hotkey6) {
         vw6 = GlobalAddAtom("atomKey6");
         if((RegisterHotKey(hWnd, vw6, hotKey2ModKey(hotkey6Mod) | hotkey6Win, hotkey6) == FALSE))
            return FALSE;
      }
      if(hotkey7) {
         vw7 = GlobalAddAtom("atomKey7");
         if((RegisterHotKey(hWnd, vw7, hotKey2ModKey(hotkey7Mod) | hotkey7Win, hotkey7) == FALSE))
            return FALSE;
      }
      if(hotkey8) {
         vw8 = GlobalAddAtom("atomKey8");
         if((RegisterHotKey(hWnd, vw8, hotKey2ModKey(hotkey8Mod) | hotkey8Win, hotkey8) == FALSE))
            return FALSE;
      }
      if(hotkey9) {
         vw9 = GlobalAddAtom("atomKey9");
         if((RegisterHotKey(hWnd, vw9, hotKey2ModKey(hotkey9Mod) | hotkey9Win, hotkey9) == FALSE))
            return FALSE;
      }
      return TRUE;
   }
   return TRUE;
}

/*************************************************
 * Unregister the hot keys
 */
void unRegisterHotKeys()
{
  if(hotKeysRegistred) {
    hotKeysRegistred = FALSE;
    UnregisterHotKey(hWnd, vw1);
    UnregisterHotKey(hWnd, vw2);
    UnregisterHotKey(hWnd, vw3);
    UnregisterHotKey(hWnd, vw4);
    UnregisterHotKey(hWnd, vw5);
    UnregisterHotKey(hWnd, vw6);
    UnregisterHotKey(hWnd, vw7);
    UnregisterHotKey(hWnd, vw8);
    UnregisterHotKey(hWnd, vw9);
  }
}

/*************************************************
 * Register the sticky hot key, if defined
 */
BOOL registerStickyKey()
{
   if(!stickyKeyRegistered && (VW_STICKYMOD || VW_STICKYWIN) && VW_STICKY) {
      stickyKeyRegistered = TRUE;
      stickyKey = GlobalAddAtom("VWStickyKey");
      if((RegisterHotKey(hWnd, stickyKey, hotKey2ModKey(VW_STICKYMOD) | VW_STICKYWIN, VW_STICKY) == FALSE))
         return FALSE;
      else
         return TRUE;
   }
   return TRUE;
}

/*************************************************
 * Unregister the sticky hot key, if previosly registred
 */
void unRegisterStickyKey()
{
  if(stickyKeyRegistered) {
    stickyKeyRegistered = FALSE;
    UnregisterHotKey(hWnd, stickyKey);
  }
}

/*************************************************
 * Register the desktop cycling hot keys, if defined
 */
BOOL registerCyclingKeys()
{
  if(!cyclingKeysRegistered && cyclingKeysEnabled) {
     cyclingKeysRegistered = TRUE;
     cyclingKeyUp = GlobalAddAtom("VWCyclingKeyUp");
     cyclingKeyDown = GlobalAddAtom("VWCyclingKeyDown");
     if((RegisterHotKey(hWnd, cyclingKeyUp, hotKey2ModKey(hotCycleUpMod), hotCycleUp) == FALSE))
        return FALSE;
     if((RegisterHotKey(hWnd, cyclingKeyDown, hotKey2ModKey(hotCycleDownMod), hotCycleDown) == FALSE))
        return FALSE; 
  }
  return TRUE;
}

/*************************************************
 * Unregister the cycling  hot keys, if previosly registred
 */
void unRegisterCyclingKeys()
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
BOOL registerMenuHotKey()
{
   if(!menuHotKeyRegistered && hotkeyMenu) {
      menuHotKeyRegistered = TRUE;
      vwMenu = GlobalAddAtom("atomKeyMenu");
      if((RegisterHotKey(hWnd, vwMenu, hotKey2ModKey(hotkeyMenuMod) | 
                         hotkeyMenuWin, hotkeyMenu) == FALSE))
         return FALSE;
   }
   return TRUE;
}

/*************************************************
 * Unregister the window menu hot key, if previosly registred
 */
void unRegisterMenuHotKey()
{
   if(menuHotKeyRegistered) {
      menuHotKeyRegistered = FALSE;
      UnregisterHotKey(hWnd, vwMenu);
   }
}

/*************************************************
 * Main window callback, this is where all main window messages are taken care of
 */
LRESULT CALLBACK wndProc(HWND aHWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   POINT pt;
   HMENU hmenu, hpopup;
   int retItem;
   static UINT taskbarRestart; 
    
   switch (message) {
      case VW_MOUSEWARP:
         // Try to avoid switching if we press on the taskbar
         if((LOWORD(lParam) && (GetForegroundWindow() == FindWindow("Shell_traywnd", ""))))
            goto skipMouseWarp; // if so, skip whole sequence
         if(enabled) { // Is virtuawin enabled
            if(useMouseKey) { // Are we using a mouse key
               if(!HIWORD(GetAsyncKeyState(MOUSEKEY))) {
                  goto skipMouseWarp; // If key not pressed skip whole sequence
               }
            }
            // Suspend mouse thread during message processing, 
            // otherwise we might step over several desktops
            SuspendThread(mouseThread); 
            GetCursorPos(&pt);

            switch HIWORD(lParam) {
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
         // Sticky hot key
         if(wParam == stickyKey) {
            toggleActiveSticky();
            break;
         }
         // Desktop Hot keys
         else if(wParam == vw1) {
            gotoDesk(1);
            break;
         }
         else if(wParam == vw2) {
            gotoDesk(2);
            break;
         }
         else if(wParam == vw3) {
            gotoDesk(3);
            break;
         }
         else if(wParam == vw4) {
            gotoDesk(4);
            break;
         }
         else if(wParam == vw5) {
            gotoDesk(5);
            break;
         }
         else if(wParam == vw6) {
            gotoDesk(6);
            break;
         }
         else if(wParam == vw7) {
            gotoDesk(7);
            break;
         }
         else if(wParam == vw8) {
            gotoDesk(8);
            break;
         }
         else if(wParam == vw9) {
            gotoDesk(9);
            break;
         }
         else if(wParam == vwMenu && hotkeyMenuEn == TRUE) {
            if(enabled) {
               hpopup = createSortedWinList_cos();
               GetCursorPos(&pt);
               SetForegroundWindow(aHWnd);
               
               retItem = TrackPopupMenu(hpopup, TPM_RETURNCMD |  // Return menu code
                                        TPM_LEFTBUTTON, (pt.x-2), (pt.y-2), // screen coordinates
                                        0, aHWnd, NULL);
               if(retItem) {
                  lockMutex();
                  if(retItem < (2 * MAXWIN)) { // Sticky toggle
                     if(winList[retItem - MAXWIN].Sticky)
                        winList[retItem - MAXWIN].Sticky = FALSE;
                     else {
                        winList[retItem - MAXWIN].Sticky = TRUE; // mark sticky..
                        showHideWindow( &winList[retItem - MAXWIN], TRUE );
                     }
                  } else if(retItem < (MAXWIN * 3)) { // window access
                     gotoDesk(winList[retItem -  (2 * MAXWIN)].Desk);
                     forceForeground(winList[retItem - (2 * MAXWIN)].Handle);
                  } else { // Assign to this desktop
                     showHideWindow( &winList[retItem - (3 * MAXWIN)], TRUE );
                     forceForeground(winList[retItem - (3 * MAXWIN)].Handle);
                  }
                  releaseMutex();
               }
               
               PostMessage(aHWnd, 0, 0, 0);  // see above
               DestroyMenu(hpopup);       // Delete loaded menu and reclaim its resources
               clearBitmapVector();
            }
            
            break;
         }
         // Cycling hot keys
         else if(wParam == cyclingKeyUp) {
            if(currentDesk == (nDesksX * nDesksY)) {
               if(deskWrap)
                  gotoDesk(1);
            } else 
               gotoDesk(currentDesk + 1);
            break;
         }
         else if(wParam == cyclingKeyDown) {
            if(currentDesk == 1) {
               if(deskWrap)
                  gotoDesk(nDesksX * nDesksY);
            } else 
               gotoDesk(currentDesk - 1);
            break;
         }
         // Desk step hot keys
         switch HIWORD(lParam) {
            case VK_LEFT:
               stepLeft();
               break;
            case VK_RIGHT:
               stepRight();
               break;
            case VK_UP:
               if(invertY)
                  stepDown();
               else
                  stepUp();
               break;
            case VK_DOWN:
               if(invertY)
                  stepUp();
               else
                  stepDown();
               break;
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
               gotoDesk(wParam);
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
         WinHelp(aHWnd, vwHelp, HELP_CONTENTS, 0);
         return TRUE;
    
      case VW_GATHER:
         showAll();
         return TRUE;
         
      case VW_DESKX:
         return nDesksX;
    
      case VW_DESKY:
         return nDesksY;

      case VW_WINLIST:
      {
         // Send over the window list with WM_COPYDATA
         COPYDATASTRUCT cds;         
         cds.dwData = nWin;
         lockMutex();
         cds.cbData = sizeof(winList);
         cds.lpData = (void*)winList;
         releaseMutex();
         sendModuleMessage(WM_COPYDATA, 0, (LPARAM)&cds); 
         return TRUE;
      }
      
      case VW_CURDESK:
         return currentDesk;

      case VW_ASSIGNWIN:
         assignWindow((HWND)wParam, (int)lParam);
         return TRUE;
         
      case VW_SETSTICKY:
         setSticky((HWND)wParam, (int)lParam);
         return TRUE;
         
         // End plugin messages
    
      case WM_CREATE:		       // when main window is created
         // set the timer in ms
         SetTimer(aHWnd, 0x29a, 250, TimerProc); 
         // register message for explorer/systray crash restart
         // only works with >= IE4.0 
         taskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));
         return TRUE;

      case WM_ENDSESSION:
         if(wParam) {
            shutDown();
         }
         return TRUE;
  
      case WM_DESTROY:	  // when application is closed
         shutDown();            
         return TRUE;

      case UWM_SYSTRAY:		   // We are being notified of mouse activity over the icon
         switch (lParam) {
            case WM_RBUTTONUP:		   // Let's track a popup menu
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
                  case ID_EXIT:	// exit application
                     DestroyWindow(aHWnd);
                     break;
                  case ID_SETUP:	// show setup box
                     showSetup();
                     break;
                  case ID_GATHER:	// gather all windows
                     showAll();
                     break;
                  case ID_HELP:	// show help
                     WinHelp(aHWnd, vwHelp, HELP_CONTENTS, 0);
                     break;
                  case ID_DISABLE:
                     disableAll(aHWnd);
               }
               PostMessage(aHWnd, 0, 0, 0);	
               DestroyMenu(hpopup);  // Delete loaded menu and reclaim its resources
               DestroyMenu(hmenu);		
               break;

            case WM_LBUTTONDBLCLK:		// double click on icon
               showSetup();
               break;
      
            case WM_LBUTTONDOWN: // Show the window list
               if(enabled) {
                  hpopup = createSortedWinList_cos();
                  GetCursorPos(&pt);
                  SetForegroundWindow(aHWnd);
        
                  retItem = TrackPopupMenu(hpopup, TPM_RETURNCMD |  // Return menu code
                                           TPM_LEFTBUTTON, (pt.x-2), (pt.y-2), // screen coordinates
                                           0, aHWnd, NULL);
                  if(retItem) {
                     lockMutex();
                     if(retItem < (2 * MAXWIN)) { // Sticky toggle
                        if(winList[retItem - MAXWIN].Sticky)
                           winList[retItem - MAXWIN].Sticky = FALSE;
                        else {
                           winList[retItem - MAXWIN].Sticky = TRUE; // mark sticky..
                           showHideWindow( &winList[retItem - MAXWIN], TRUE );
                        }
                     } else if(retItem < (MAXWIN * 3)) { // window access
                        gotoDesk(winList[retItem -  (2 * MAXWIN)].Desk);
                        forceForeground(winList[retItem - (2 * MAXWIN)].Handle);
                     } else { // Assign to this desktop
                        showHideWindow( &winList[retItem - (3 * MAXWIN)], TRUE );
                        forceForeground(winList[retItem - (3 * MAXWIN)].Handle);
                     }
                     releaseMutex();
                  }
        
                  PostMessage(aHWnd, 0, 0, 0);  // see above
                  DestroyMenu(hpopup);	      // Delete loaded menu and reclaim its resources
                  clearBitmapVector();
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

/************************************************
 * Show the setup dialog and perform some stuff before and after display
 */
void showSetup()
{
   if(!setupOpen) { // Stupid fix, can't get this modal
      setupOpen = TRUE;
      // reload load current config
      readConfig();
      if(createPropertySheet(hInst, hWnd)) // Show the actual dialog
      {
         // User pressed OK, make changes persistent
         // Save the current config
         writeConfig();
         // Tell modules about the config change
         postModuleMessage(MOD_CFGCHANGE, 0, 0);
         // Reload taskbar if any changes to those settings
         getTaskbarLocation();
      }
      // Note! The setup dialog is responible for resetting all 
      // values if cancel is pressed

      // make sure that keys are alright
      unRegisterAllKeys();
      registerAllKeys();
      setMouseKey();
      // Set the mouse thread in correct state
      enableMouse(mouseEnable);
      setupOpen = FALSE;
   }
}

/************************************************
 * Convinient function for registering all hotkeys
 */
void registerAllKeys()
{
   if(!registerKeys())
      MessageBox(hWnd, "Invalid key modifier combination, check control keys!", 
                 NULL, MB_ICONWARNING);
   if(!registerHotKeys())
      MessageBox(hWnd, "Invalid key modifier combination, check hot keys!", 
                 NULL, MB_ICONWARNING);
   if(!registerStickyKey())
      MessageBox(hWnd, "Invalid key modifier combination, check sticky hot key!", 
                 NULL, MB_ICONWARNING);
   if(!registerCyclingKeys())
      MessageBox(hWnd, "Invalid key modifier combination, check cycling hot keys!", 
                 NULL, MB_ICONWARNING);
   if(!registerMenuHotKey())
      MessageBox(hWnd, "Invalid key modifier combination, check menu hot key!", 
                 NULL, MB_ICONWARNING);
}

/************************************************
 * Convinient function for unregistering all hotkeys
 */
void unRegisterAllKeys()
{
   unRegisterStickyKey();
   unRegisterHotKeys();
   unRegisterKeys();
   unRegisterCyclingKeys();
   unRegisterMenuHotKey();
}

/************************************************
 * Callback for the timer. Updates window list and other stuff 
 */
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
   lockMutex();
   /* We now own the mutex. */
   EnumWindows(enumWindowsProc, 0); // Get all windows
   /* Let other threads have a go. */
   releaseMutex();
   if (hMutex != (HANDLE)0) ReleaseMutex(hMutex);
   currentActive = GetForegroundWindow();
   if(userDefinedWin)
      findUserWindows();
   packList();	// Clean up the window list
  
   // save the desktop state every minute
   if(crashRecovery) {
      if(saveInterval < 0) {
         saveInterval = 240; // 250 * 240 = 1 min
         lockMutex();
         saveDesktopState(&nWin, winList);
         releaseMutex();
      }
      saveInterval--;
   }
}

/************************************************
 * Does necessary stuff before shutting down
 */
void shutDown()
{
   if(saveLayoutOnExit)
   {
      lockMutex();
      saveDesktopConfiguration(&nWin, winList); 
      releaseMutex();
   }
   writeDisabledList(&nOfModules, moduleList);
   unloadModules();
   showAll();	        // gather all windows on exit
   clearLock();       // Remove the lock file, don't bother if this fails
   unRegisterAllKeys();
   Shell_NotifyIcon(NIM_DELETE, &nIconD); // This removes the icon
   if(saveSticky)
   {
      lockMutex();
      saveStickyWindows(&nWin, winList);
      releaseMutex();
   }
   PostQuitMessage(0);
   KillTimer(hWnd, 0x29a);                // Remove the timer
}

/*************************************************
 * Sets the modifier key(s) for switching desktop with arrow keys
 */
void setKeyMod()
{
  MODKEY = modAlt | modShift | modCtrl | modWin;
}

/*************************************************
 * Sets the modifier key for switching desktop with number mouse
 */
void setMouseKey()
{
  if(mouseModAlt == TRUE)
    MOUSEKEY = VK_MENU;
  else if(mouseModShift == TRUE)
    MOUSEKEY = VK_SHIFT;
  else if(mouseModCtrl == TRUE)
    MOUSEKEY = VK_CONTROL;
}

/*************************************************
 * Helper function for all the step* functions below
 * Does the actual switching work 
 */
void stepDesk()
{
   int x;  
   lockMutex();
   for (x = 0; x < nWin ; ++x) {
      // Show these windows
      if(winList[x].Desk == currentDesk) {
         showHideWindow( &winList[x], TRUE );

         if(winList[x].Active && keepActive && !isDragging) {
            forceForeground(winList[x].Handle);
            topWindow = winList[x].Handle;
         }
      }
      // Hide these, not iconic or sticky
      else if(!IsIconic(winList[x].Handle) && !winList[x].Sticky) {
         showHideWindow( &winList[x], FALSE );
      }
      // Hide these, iconic but "Switch minimized" true
      else if(IsIconic(winList[x].Handle) && minSwitch && !winList[x].Sticky) {
         showHideWindow( &winList[x], FALSE );
      }
   }
   
   if(releaseFocus)     // Release focus maybe?
      SetForegroundWindow(releaseHnd);
   else if(topWindow)   // Raise the active window
      BringWindowToTop(topWindow);
   else {               // Ok, no active candidate, try the first sticky in list
      HWND tmpHwnd = GetTopWindow(NULL); 
      // We are trying to find the sticky window (if any) 
      // with the highest z-order
      while((tmpHwnd = GetNextWindow(tmpHwnd, GW_HWNDNEXT))) {
         for (x = 0; x < nWin; ++x) {
            if (winList[x].Sticky && tmpHwnd == winList[x].Handle) {
               SetForegroundWindow(winList[x].Handle);
               tmpHwnd = NULL; // Yes, brutal exit out of while loop
            }
         }
      }
   }
   releaseMutex();
   topWindow = NULL;
   
   if(refreshOnWarp) // Refresh the desktop 
      RedrawWindow( NULL, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN );
}

/*************************************************
 * Step on desk to the right
 */
int stepRight()
{
   if((currentDeskX + 1) > nDesksX) {
      if(deskWrap) {
         int retVal = gotoDeskXY(1, currentDeskY);
         if(retVal != 0)
            postModuleMessage(MOD_CHANGEDESK, MOD_STEPRIGHT, currentDesk);
         return retVal;
      }
      else
         return 0;
   }

   currentDeskX += 1;
   setIcon(calculateDesk());   
   stepDesk();
   
   postModuleMessage(MOD_CHANGEDESK, MOD_STEPRIGHT, currentDesk);
   return currentDeskX;
}

/*************************************************
 * Step one desk to the left
 */
int stepLeft()
{
   if((currentDeskX - 1) < 1) {
      if(deskWrap) {
         int retVal = gotoDeskXY(nDesksX, currentDeskY);
         if(retVal != 0)
            postModuleMessage(MOD_CHANGEDESK, MOD_STEPLEFT, currentDesk);
         return retVal;
      }
      else
         return 0;
   }
  
   currentDeskX -= 1;
   setIcon(calculateDesk());
   stepDesk();
  
   postModuleMessage(MOD_CHANGEDESK, MOD_STEPLEFT, currentDesk);
   return currentDeskX;
}

/*************************************************
 * Step one desk down
 */
int stepDown()
{
   if((currentDeskY + 1) > nDesksY) {
      if(deskWrap) {
         int retVal = gotoDeskXY(currentDeskX, 1);
         if(retVal != 0)
            postModuleMessage(MOD_CHANGEDESK, MOD_STEPDOWN, currentDesk);
         return retVal;
      }
      else
         return 0;
   }
  
   currentDeskY += 1;
   setIcon(calculateDesk());
   stepDesk();
  
   postModuleMessage(MOD_CHANGEDESK, MOD_STEPDOWN, currentDesk);
   return currentDeskY;
}

/*************************************************
 * Step one desk up
 */
int stepUp()
{
   if((currentDeskY - 1) < 1) {
      if(deskWrap) {
         int retVal = gotoDeskXY(currentDeskX, nDesksY);
         if(retVal != 0)
            postModuleMessage(MOD_CHANGEDESK, MOD_STEPUP, currentDesk);
         return retVal;
      }
      else
         return 0;
   }

   currentDeskY -= 1;
   setIcon(calculateDesk());
   stepDesk();
  
   postModuleMessage(MOD_CHANGEDESK, MOD_STEPUP, currentDesk);
   return currentDeskY;
}

/*************************************************
 * Goto the specified desktop specifying coordinates
 */
int gotoDeskXY(int deskX, int deskY)
{
   int tmpDesk;
   if(deskX > nDesksX || deskY > nDesksY)
      return 0;
  
   currentDeskX = deskX;
   currentDeskY = deskY;
   tmpDesk = gotoDesk(calculateDesk());
   if(tmpDesk != 0)
      setIcon(tmpDesk);
   
   return tmpDesk;
}

/*************************************************
 * Goto a specified desktop specifying desk number
 */
int gotoDesk(int theDesk)
{
   if(theDesk == currentDesk) // No use trying
      return 0;
   if (theDesk > (nDesksY * nDesksX)) // Hey, we can't go there
      return 0;

   currentDesk	= theDesk;

   nIconD.hIcon = icons[currentDesk];
   if( displayTaskbarIcon )
   Shell_NotifyIcon(NIM_MODIFY, &nIconD);

   /* Calculations for getting the x and y positions */
   currentDeskY = (int) ceil((double)currentDesk/(double)nDesksX);
   currentDeskX = nDesksX + currentDesk - (currentDeskY * nDesksX);

   stepDesk();
   
   postModuleMessage(MOD_CHANGEDESK, currentDesk, currentDesk);
   return theDesk;
}

/*************************************************
 * Sets the icon in the systray and updates the currentDesk variable
 */
void setIcon(int theNumber)
{
   nIconD.hIcon = icons[theNumber];
   if( displayTaskbarIcon )
      Shell_NotifyIcon(NIM_MODIFY, &nIconD);
   currentDesk = theNumber;
}

/*************************************************
 * Calculates currentDesktop 
 */
int calculateDesk()
{
   return (currentDeskY * nDesksX) - (nDesksX - currentDeskX);
}

/*************************************************
 * Forces a window into the foreground. Must be done in this way to avoid
 * the flashing in the taskbar insted of actually changing active window.
 */
void forceForeground(HWND theWin)
{
   DWORD ThreadID1;
   DWORD ThreadID2;

   /* Nothing to do if already in foreground */
   if(theWin == GetForegroundWindow()) {
      return;
   } else {
      /* Get the thread responsible for VirtuaWin,
         and the thread for the foreground window */
      ThreadID1 = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
      ThreadID2 = GetWindowThreadProcessId(hWnd, NULL);
      /* By sharing input state, threads share their concept of
         the active window */
      if(ThreadID1 != ThreadID2) {
         AttachThreadInput(ThreadID1, ThreadID2, TRUE);
         SetForegroundWindow(hWnd); // Set VirtuaWin active. Don't no why, but it seems to work
         AttachThreadInput(ThreadID1, ThreadID2, FALSE);
         SetForegroundWindow(theWin);
      } else {
         SetForegroundWindow(theWin);
      }
   }
}

/*************************************************
 * Inizialize the window list and gets the current active window
 */
void initData()
{
  int x;
  currentActive = GetForegroundWindow();
  lockMutex();
  for (x = 0; x < MAXWIN; ++x) {
    winList[x].Handle = NULL;
    winList[x].Active = FALSE;
    winList[x].Sticky = FALSE;
    winList[x].Sticky = TRUE;
    winList[x].Hidden = FALSE;
    winList[x].StyleFlags = 0;
    winList[x].Desk = currentDesk;
  }
  releaseMutex();
}

/*************************************************
 * Checks if a window is a previous saved sticky window
 */
BOOL checkIfSavedSticky(HWND hwnd)
{
   char className[51];
   int i;
   GetClassName(hwnd, className, 50);
   for(i = 0; i < curSticky; ++i) {
      if (!strncmp(stickyList[i].winClassName, className, 50)) {
         if(!permanentSticky)
         {
            free(stickyList[i].winClassName);
            stickyList[i].winClassName = "\n"; // Remove this from list, it is used
         }
         return TRUE;
      }
   }
   return FALSE;
}

BOOL checkIfSavedStickyString(char* className)
{
   int i;
   for(i = 0; i < curSticky; ++i) {
      if (!strncmp(stickyList[i].winClassName, className, 50)) {
         // Typically user windows will loose their stickiness if
         // minimized, therefore we do not remove their name from 
         // the list as done above.
         return TRUE;
      }
   }
   return FALSE;
}

/*************************************************
 * Add a window in the list
 */
inline void integrateWindow(HWND hwnd)
{
   int style = GetWindowLong(hwnd, GWL_STYLE);
   int exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);

   /* Criterias for a window to be handeled by VirtuaWin */
   if (nWin < MAXWIN-1 &&
       !(style & WS_CHILD) &&                                          // No child windows
       IsWindowVisible(hwnd) &&                                        // Must be visible
       (!GetParent(hwnd) || GetParent(hwnd) == GetDesktopWindow()) &&  // Only toplevel or owned by desktop
       !(exstyle & WS_EX_TOOLWINDOW) &&                                // No toolwindows
       (!GetWindow(hwnd, GW_OWNER) ||                                  // No windows that are owned by others, 
        !IsWindowVisible(GetWindow(hwnd, GW_OWNER))))                  // if the owner is not hidden
   {
      char buf[100];
      GetClassName(hwnd, buf, 99);

      if( isSpecialWindow( buf ) && trickyList)
      {
         winList[nWin].NormalHide = FALSE;
         winList[nWin].StyleFlags = exstyle;
      }
      else 
         winList[nWin].NormalHide = TRUE;
      winList[nWin].Handle = hwnd;
      if(useDeskAssignment) 
      {
         winList[nWin].Desk = checkIfAssignedDesktop(hwnd);
         if(winList[nWin].Desk != currentDesk)
            showHideWindow( &winList[nWin], FALSE );
      } 
      else 
      {
         winList[nWin].Desk = currentDesk;
      } 
      winList[nWin].Sticky = checkIfSavedSticky(hwnd);
      
      nWin++;
   }
}

/*************************************************
 * Searches for windows in user list
 */
void findUserWindows() 
{
  HWND tmpHnd;
  int i;
  for (i = 0; i < curUser; ++i) {
    if(userList[i].isClass) {
      tmpHnd = FindWindow(userList[i].winNameClass, NULL);
      if(!tmpHnd)
        userList[i].isClass = FALSE;
    }
    else {
      tmpHnd = FindWindow(NULL, userList[i].winNameClass);
      if(!tmpHnd)
        userList[i].isClass = TRUE;
    }
    
    lockMutex();
    if(!inWinList(tmpHnd)) {
      winList[nWin].Handle = tmpHnd;
      winList[nWin].Desk = currentDesk;
      winList[nWin].Sticky = checkIfSavedStickyString(userList[i].winNameClass);
      winList[nWin].NormalHide = TRUE;
      nWin++;
    }
    releaseMutex();
  }
}

/*************************************************
 * Callback function. Integrates all enumerated windows
 */
inline BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam) 
{
  if(nWin >= MAXWIN) {
    KillTimer(hWnd, 0x29a);
    enabled = FALSE;
    MessageBox(hWnd, "Oops! Maximum windows reached. \nVirtuaWin has been disabled. \nMail VirtuaWin@home.se and tell me this.", "VirtuaWin", MB_ICONWARNING);
    return FALSE;
  }
  
  if (!inWinList(hwnd))
    integrateWindow(hwnd);
  return TRUE;
}

/*************************************************
 * Returns true if a window is in the list
 */
inline BOOL inWinList(HWND hwnd)
{
  int index;
  if (!hwnd)
    return TRUE; // don't add

  for (index = 0; index < nWin; ++index)
    if (winList[index].Handle == hwnd)
      return TRUE;
  return FALSE;
}

/*************************************************
 * Makes all windows visible
 */
void showAll()
{
   int x;
   lockMutex();
   for (x = 0; x < MAXWIN; ++x) 
   {
      if (IsWindow(winList[x].Handle)) 
      {
         showHideWindow( &winList[x], TRUE );
      }
   }
   releaseMutex();
}

/*************************************************
 * Compact the window list, removes any destroyed windows
 * and makes sure that moved windows stays away from the taskbar
 */
void packList()
{
   int i;
   int j;
   lockMutex();
   for (i = 0; i < nWin; ++i) {
      // Windows can pop back into taksbar, make sure they keep away
      if(winList[i].NormalHide == FALSE && winList[i].Hidden == TRUE)
      {
         PostMessage( hwndTask, RM_Shellhook, 2, (LPARAM) winList[i].Handle);
      }
      // remove killed windows
      if(!IsWindow(winList[i].Handle) || 
         (!IsWindowVisible(winList[i].Handle) && winList[i].Desk == currentDesk)) {
         for (j = i; j < nWin - 1; ++j) {
            memcpy(&winList[j], &winList[j + 1], sizeof(windowType));
         }
         memset(&winList[nWin - 1], 0, sizeof(windowType));
         nWin--;
         continue;
      }
      
      // Update the current active window, only among the visible windows
      if( !(winList[i].Hidden) ) 
      {
         winList[i].Desk = currentDesk;
         if(winList[i].Handle == currentActive) // Is this the active one?
            winList[i].Active = TRUE;
         else
            winList[i].Active = FALSE;
      }
   }
   releaseMutex();
}

/*************************************************
 * createSortedWinList_cos creates a popup menu for the window-hotkey
 * which displays all windows in one list vertically seperated by a line.
 * first column is sticky, second is direct access and third is assign.
 * so you don't have to step through submenus.
 * 
 * Author: Christian Storm aka cosmic (Christian.Storm@Informatik.Uni-Oldenburg.de)
 */
HMENU createSortedWinList_cos()
{
    HMENU        hMenu;         // menu bar handle
    char title[35];
    MenuItem *items[MAXWIN], *item;
    char buff[31];
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
    for(i = 0; i < nWin; ++i)
    {
        GetWindowText(winList[i].Handle, buff, 30);
        sprintf(title, "%d - %s", winList[i].Desk, buff);
        item = malloc( sizeof(MenuItem) );
        item->name = strdup (title);
        
        HICON hSmallIcon = (HICON)GetClassLong(winList[i].Handle, GCL_HICON);
        item->icon = createBitmapIcon(hSmallIcon);

        item->desk = winList[i].Desk;
        item->sticky = winList[i].Sticky;
        item->id = i;
        items [i]   = item;
        items [i+1] = NULL;
    }
    items [i+2] = NULL; // just in case
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
                c = items [x]->desk; d=0;
            }
            if (!e && useTitle ) {
                AppendMenu(hMenu, MF_STRING, 0, "Sticky" );
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                e=1;
            }
            AppendMenu( hMenu,
                        MF_STRING | (items[x]->sticky ? MF_CHECKED: 0),
                        MAXWIN + (items[x]->id), items[x]->name );
            SetMenuItemBitmaps(hMenu, MAXWIN + (items[x]->id), MF_BYCOMMAND, items[x]->icon, 0);
            d=1;
        }
    }

    c=0; d=1; e=0;
    if(directMenu) {
        if (stickyMenu) menuBreak = TRUE;
        for (x=0; x < i; x++ )
        {
            if ((!c || c != items[x]->desk)&&d)
            {
                if(c) AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                c = items [x]->desk; d=0;
            }

            // accessing current desk - direct assign makes no sense
            if (items[x]->desk!=calculateDesk()) {
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
                AppendMenu( hMenu,
                            MF_STRING | (items[x]->sticky ? MF_CHECKED: 0),
                            2 * MAXWIN + (items[x]->id), items[x]->name );
                SetMenuItemBitmaps(hMenu, 2 * MAXWIN + (items[x]->id), MF_BYCOMMAND, items[x]->icon, 0);
                d=1;
            }
        }
    }

    c=0; d=1; e=0;
    if(assignMenu) {
        if (stickyMenu || directMenu) menuBreak=TRUE;
        for (x=0; x < i; x++ )
        {
            if ((!c || c != items[x]->desk)&&d)
            {
                if(c) AppendMenu(hMenu, MF_SEPARATOR, 0, NULL );
                c = items [x]->desk; d=0;
            }
      
            //sticky windows can't be assigned cause they're sticky :-) so leave them.out..
            //cannot assign to current Desktop
            if ((!items[x]->sticky)&&(items[x]->desk!=calculateDesk())) {
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
                AppendMenu( hMenu, MF_STRING, 3 * MAXWIN + (items[x]->id), items[x]->name );
                SetMenuItemBitmaps(hMenu, 3 * MAXWIN + (items[x]->id), MF_BYCOMMAND, items[x]->icon, 0);
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
 * Converts an icon to a bitmap representation
 */
HBITMAP createBitmapIcon(HICON anIcon)
{
    if(anIcon != NULL)
    {
        HDC aHDC = GetDC(hWnd);
        HDC aCHDC = CreateCompatibleDC(aHDC);
        HBITMAP hBMP = CreateCompatibleBitmap(aHDC, 16, 16);
        hBMP = SelectObject(aCHDC, hBMP);
        if(DrawIconEx(aCHDC, 0, 0, anIcon, 16, 16, 0, 0, DI_NORMAL) == 0)
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
    return 0;
}

/*************************************************
 * Deletes bitmaps allocated when displaying window menu
 */
void clearBitmapVector()
{
    while(vectorPosition > 0)
    {
        vectorPosition--;
        DeleteObject(iconReferenceVector[vectorPosition]);
    }
}

/*************************************************
 * Translates virtual key codes to "hotkey codes"
 */
WORD hotKey2ModKey(BYTE vModifiers)
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
 * Toggles the active window's stickiness
 */
void toggleActiveSticky()
{
   HWND tempHnd = GetForegroundWindow();
   int i;
   lockMutex();
   for (i = 0; i < nWin; ++i) {
      if(tempHnd == winList[i].Handle) {
         if(winList[i].Sticky)
            winList[i].Sticky = FALSE;
         else
            winList[i].Sticky = TRUE;
         return;
      }
   }
   releaseMutex();
}

/*************************************************
 * Tries to find windows saved before a crash by classname matching
 */
void recoverWindows()
{
   char* dummy = malloc(sizeof(char) * 80);
   int nOfRec = 0;
   HWND tmpHnd;
   char buff[27];
   FILE* fp;
    

   char VirtuaWinStateFile[MAX_PATH];
   GetFilename(vwSTATE, VirtuaWinStateFile);

   if((fp = fopen(VirtuaWinStateFile, "r"))) {
      while(!feof(fp)) {
         fscanf(fp, "%79s", dummy);
         if((strlen(dummy) != 0) && !feof(fp)) {
            tmpHnd = FindWindow(dummy, NULL);
            if(tmpHnd) {
               if ( !isSpecialWindow( dummy ) ) // Hidden window
               {
                  if(safeShowWindow(tmpHnd, SW_SHOWNA) == TRUE)
                     nOfRec++;
               }
               else // moved window
               {
                  // Move to an appropriate position
                  SetWindowPos( tmpHnd, 0, 10, 10, 0, 0, 
                                SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE );
                  // Notify taskbar of the change
                  PostMessage( hwndTask, RM_Shellhook, 1, (LPARAM) tmpHnd );
                  nOfRec++; // No way to tell if this went ok
               }
            }
         }
      }
      fclose(fp);
   }
   free(dummy);
   sprintf(buff, "%d windows was recovered.", nOfRec);
   MessageBox(hWnd, buff, "VirtuaWin", 0); 
}

/*************************************************
 * Checks if a window is an predifined desktop to go to
 */
int checkIfAssignedDesktop(HWND aHwnd)
{
   char className[51];
   int i;
   GetClassName(aHwnd, className, 50);
   for(i = 0; i < curAssigned; ++i) {
      if (!strncmp(assignedList[i].winClassName, className, 50)) {
         if(assignOnlyFirst) {
            free(assignedList[i].winClassName);
            assignedList[i].winClassName = "\n"; // Remove this from list, it is used
         }
         if( assignedList[i].desktop > (nDesksX * nDesksY))
            MessageBox(hWnd, "Tried to assign an application to an unavaliable desktop.\nIt will not be assigned.\nCheck desktop assignmet configuration.", "VirtuaWin", 0); 
         else
            return assignedList[i].desktop; // Yes, assign
      }
   }
   return currentDesk; // No assignment, return current
}

/************************************************
 * This function decides what switching technique that should be used
 * and calls the appropriate switching function
 */
void showHideWindow( windowType* aWindow, BOOL show )
{
   // Do nothing if we are dragging a window
   if( isDragging && ( aWindow->Handle == currentActive ))
      return;
   
   // Normal Case || alternative method disabled
   if( aWindow->NormalHide || !trickyWindows)
   {
      if( show )
      {
         if( safeShowWindow(aWindow->Handle, SW_SHOWNA ))
            aWindow->Hidden = FALSE;
      }
      else
      {         
         if( safeShowWindow(aWindow->Handle, SW_HIDE ))
            aWindow->Hidden = TRUE;
      }
   }
   // Tricky window
   else
   {
      moveShowWindow( aWindow, show );
   }
}

/************************************************
 * Wraps ShowWindow() and make sure that we won't hang on crashed applications
 * Instead we notify user by flashing the systray icon
 */
BOOL safeShowWindow(HWND theHwnd, int theState)
{
   if(SendMessageTimeout(theHwnd, (int)NULL, 0, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 2000, NULL)) {
      ShowWindow(theHwnd, theState);
      ShowOwnedPopups(theHwnd, theState);
      return TRUE;
   } else {
      if( displayTaskbarIcon )
         warningIcon(); // Flash the systray icon
      return FALSE;     // Probably hanged
   }
}

/************************************************
 * Moves a window either away from the visible area or back again. Some applications
 * needs this way of moving since they don't like to be hidden
 */
void moveShowWindow( windowType* aWindow, BOOL show )
{
   RECT aPosition;
   GetWindowRect( aWindow->Handle, &aPosition );

   if( show && aWindow->Hidden ) // Move window to visible area
   {
      // Restore the window mode
      SetWindowLong( aWindow->Handle, GWL_EXSTYLE, aWindow->StyleFlags );  
      // Notify taskbar of the change
      PostMessage( hwndTask, RM_Shellhook, 1, (LPARAM) aWindow->Handle );
      // Bring back to visible area, SWP_FRAMECHANGED makes it repaint 
      SetWindowPos( aWindow->Handle, 0, aPosition.left, aPosition.top - screenBottom - 10, 
                   0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE ); 
      // Notify taskbar of the change
      PostMessage( hwndTask, RM_Shellhook, 1, (LPARAM) aWindow->Handle );
      aWindow->Hidden = FALSE;
   }
   else if( !show && !aWindow->Hidden ) // Move away window
   {
      // Move the window off visible area
      SetWindowPos( aWindow->Handle, 0, aPosition.left, aPosition.top + screenBottom + 10, 
                   0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE );
      // This removes window from taskbar and alt+tab list
      SetWindowLong( aWindow->Handle, GWL_EXSTYLE, 
                     aWindow->StyleFlags & ((~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW) );
      // Notify taskbar of the change
      PostMessage( hwndTask, RM_Shellhook, 2, (LPARAM) aWindow->Handle);
      aWindow->Hidden = TRUE;
   }
}

/************************************************
 * Starts a timer that flashes the systray icon 5 times
 */
void warningIcon()
{
   SetTimer(hWnd, 0x30a, 500, FlashProc); // set the timer in ms
}

/************************************************
 * Callback for the flash timer
 */
VOID CALLBACK FlashProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
   static int count = 0;
   count++;
   if(count % 2)
      nIconD.hIcon = NULL; // No icon
   else 
      nIconD.hIcon = icons[currentDesk];
   Shell_NotifyIcon(NIM_MODIFY, &nIconD);
   
   if(count > 10) {
      KillTimer(hWnd, 0x30a);
      nIconD.hIcon = icons[currentDesk];
      Shell_NotifyIcon(NIM_MODIFY, &nIconD);
      count = 0;
   }
}

/************************************************
 * Checks if this window is saved as a tricky window that needs the 
 * "move technique" to be hidden.
 */
BOOL isSpecialWindow( char* className )
{
   int i;
   for( i = 0; i < curTricky; ++i ) 
   {
      if (!strncmp( trickyList[i].winClassName, className, 50 )) 
      {
         return TRUE;
      }
   }
   return FALSE;  
}

/************************************************
 * Tries to locate the handle to the taskbar
 */
void goGetTheTaskbarHandle()
{
   if(!noTaskbarCheck)
   {
      HWND hwndTray = FindWindowEx(NULL, NULL, "Shell_TrayWnd", NULL);
      HWND hwndBar = FindWindowEx(hwndTray, NULL, "ReBarWindow32", NULL );
      
      // Maybe "RebarWindow32" is not a child to "Shell_TrayWnd", then try this
      if( hwndBar == NULL )
         hwndBar = hwndTray;
      
      hwndTask = FindWindowEx(hwndBar, NULL, "MSTaskSwWClass", NULL);
      
      if( hwndTask == NULL )
         MessageBox(hWnd, "Could not locate handle to the taskbar.\n This will disable the ability to hide troublesome windows correctly.", "VirtuaWin", 0); 
   }
}

/************************************************
 * Get screen width and height and store values in
 * global variables
 */
void getScreenSize()
{
   screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
   screenRight = GetSystemMetrics(SM_CXVIRTUALSCREEN);
   screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
   screenBottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

/************************************************
 * Grabs and stores the taskbar coordinates
 */
void getTaskbarLocation()
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
 * Locks the window list protection
 */
void lockMutex()
{
   if (hMutex != (HANDLE)0 && WaitForSingleObject(hMutex,0) == WAIT_TIMEOUT) {
      WaitForSingleObject(hMutex,INFINITE);
   }
}

/************************************************
 * Releases the window list protection
 */
void releaseMutex()
{
   if (hMutex != (HANDLE)0) ReleaseMutex(hMutex);
}

/************************************************
 * Toggles the disabled state of VirtuaWin
 */
void disableAll(HWND aHWnd)
{
   if(enabled) {                                   // disable VirtuaWin
      strcpy(nIconD.szTip,"VirtuaWin - Disabled"); //Tooltip
      nIconD.hIcon = icons[0];
      Shell_NotifyIcon(NIM_MODIFY, &nIconD);
      unRegisterKeys();
      unRegisterHotKeys();
      unRegisterStickyKey();
      unRegisterCyclingKeys();
      unRegisterMenuHotKey();
      KillTimer(aHWnd, 0x29a);
      enabled = FALSE;
   } else {			        // Enable VirtuaWin
      strcpy(nIconD.szTip, appName);	// Tooltip
      setIcon(calculateDesk());
      if(!registerKeys())
         MessageBox(aHWnd, "Invalid key modifier combination, check control keys!", 
                    NULL, MB_ICONWARNING);
      if(!registerHotKeys())
         MessageBox(aHWnd, "Invalid key modifier combination, check hot keys!", 
                    NULL, MB_ICONWARNING);
      if(!registerStickyKey())
         MessageBox(aHWnd, "Invalid key modifier combination, check sticky hot key!", 
                    NULL, MB_ICONWARNING);
      if(!registerCyclingKeys())
         MessageBox(aHWnd, "Invalid key modifier combination, check cycling hot keys!", 
                    NULL, MB_ICONWARNING);
      if(!registerMenuHotKey())
         MessageBox(aHWnd, "Invalid key modifier combination, check menu hot key!", 
                    NULL, MB_ICONWARNING);
      SetTimer(aHWnd, 0x29a, 250, TimerProc);  // Set the timer in ms
      enabled = TRUE;
   }
}

/************************************************
 * Assigns a window to the specified desktop
 * Used by the module message VW_ASSIGNWIN
 */
void assignWindow(HWND theWin, int theDesk)
{
   int index;
   
   if((theDesk > (nDesksY * nDesksX)) || (theDesk < 1)) // Invalid desk
      return;

   for (index = 0; index < nWin; ++index)
      if (winList[index].Handle == theWin)
      {
         // found
         if(winList[index].Desk != theDesk)
         {
            lockMutex();
            showHideWindow( &winList[index], currentDesk == theDesk ? TRUE:FALSE );
            winList[index].Desk = theDesk;
            releaseMutex();
         }
      }
}

/************************************************
 * Changes sticky state of a window
 * Used by the module message VW_SETSTICKY
 */
void setSticky(HWND theWin, int state)
{
   int index;

   for (index = 0; index < nWin; ++index)
      if (winList[index].Handle == theWin)
      {
         lockMutex();
         // found
         if(state == 0) // disable
         {
            winList[index].Sticky = FALSE;
         }
         else if(state == 1) // enable
         {
            winList[index].Sticky = TRUE; 
            showHideWindow( &winList[index], TRUE );
         }
         else if(state == -1) // toggle
         {
            if(winList[index].Sticky)
               winList[index].Sticky = FALSE;
            else
            {
               winList[index].Sticky = TRUE;
               showHideWindow( &winList[index], TRUE );
            }
         }
         releaseMutex();
      }
}

/*
 * $Log$
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
