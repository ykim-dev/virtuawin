//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K/XP
// 
//  This is a module for VirtuaWin that adds hot key support
//  for moving the current active window to next or previous desktop
//  
// 
//  Copyright (c) 1999, 2000, 2001, 2002, 2003, 2004 Johan Piculell
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
#include <string.h>
#include <stdio.h>
#include <commctrl.h>

#include "assignerres.h"
#include "../../Messages.h"

HINSTANCE hInst;   // Instance handle
HWND hwndMain;	   // Main window handle
HWND vwHandle;     // Handle to VirtuaWin
ATOM hotKeyUp;
ATOM hotKeyDown;
UINT HOT_NEXT;
UINT HOT_NEXT_MOD;
UINT HOT_PREV;
UINT HOT_PREV_MOD;
LPSTR vwPath;
LPSTR configFile;

/* prototype for the dialog box function. */
static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/* Main message handler */
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

void registerAssignment();
void unregisterAssignment();
void getConfigDir();
void saveSettings();
void loadSettings();
WORD hotKey2ModKey(BYTE vModifiers);

/* Initializes the window */ 
static BOOL InitApplication(void)
{
   WNDCLASS wc;

   memset(&wc, 0, sizeof(WNDCLASS));
   wc.style = 0;
   wc.lpfnWndProc = (WNDPROC)MainWndProc;
   wc.hInstance = hInst;
   /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
      this for locating the window */
   wc.lpszClassName = "VWAssigner.exe";
   
   if (!RegisterClass(&wc))
      return 0;
  
   return 1;
}

//*************************************************

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   HWND theActive = NULL;
   int theCurDesk;
   InitCommonControls();
   switch (msg) {
      case WM_HOTKEY:
         // Get the current desktop
         theCurDesk = SendMessage(vwHandle, VW_CURDESK, 0, 0);
         // Get the active window
         theActive = GetForegroundWindow();
         
         if(theActive)
         {
            if(wParam == hotKeyUp)
               PostMessage(vwHandle, VW_ASSIGNWIN, (WPARAM)theActive, theCurDesk + 1);
            else if(wParam == hotKeyDown)
               PostMessage(vwHandle, VW_ASSIGNWIN, (WPARAM)theActive, theCurDesk - 1);
         }
         break;  
      case MOD_INIT: // This must be taken care of in order to get the handle to VirtuaWin. 
         // The handle to VirtuaWin comes in the wParam 
         vwHandle = (HWND) wParam; // Should be some error handling here if NULL 
         break;
      case MOD_QUIT: // This must be handeled, otherwise VirtuaWin can't shut down the module 
         PostQuitMessage(0);
         break;
      case MOD_SETUP:
         DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), vwHandle, (DLGPROC) DialogFunc);
         break;
      case WM_DESTROY:
         unregisterAssignment();
         PostQuitMessage(0);
         break;
      default:
         return DefWindowProc(hwnd, msg, wParam, lParam);
   }
  
   return 0;
}

/*
 * Main startup function
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
   MSG msg;
   hInst = hInstance;
   if (!InitApplication())
      return 0;
  
   // the window is never shown
   if ((hwndMain = CreateWindow("VWAssigner.exe", 
                                "VWAssigner", 
                                WS_POPUP,
                                CW_USEDEFAULT, 
                                0, 
                                CW_USEDEFAULT, 
                                0,
                                NULL,
                                NULL,
                                hInst,
                                NULL)) == (HWND)0)
      return 0;

   getConfigDir();
   loadSettings();
   registerAssignment();
   // main messge loop
   while (GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
   return msg.wParam;
}

/*
  This is the main function for the dialog. 
*/
static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   WORD wRawHotKey;
   
   switch (msg) {
      case WM_INITDIALOG:
         unregisterAssignment();
         SendDlgItemMessage(hwndDlg, IDC_HOTNEXT, HKM_SETHOTKEY, 
                            MAKEWORD(HOT_NEXT, HOT_NEXT_MOD), 0);
         SendDlgItemMessage(hwndDlg, IDC_HOTPREV, HKM_SETHOTKEY, 
                            MAKEWORD(HOT_PREV, HOT_NEXT_MOD), 0);
         return TRUE;
         
      case WM_COMMAND:
         switch (LOWORD(wParam)) {
            case IDOK:
               wRawHotKey = (WORD)SendDlgItemMessage(hwndDlg, IDC_HOTNEXT, HKM_GETHOTKEY, 0, 0);
               HOT_NEXT = LOBYTE(wRawHotKey);
               HOT_NEXT_MOD = HIBYTE(wRawHotKey);
               
               wRawHotKey = (WORD)SendDlgItemMessage(hwndDlg, IDC_HOTPREV, HKM_GETHOTKEY, 0, 0);
               HOT_PREV = LOBYTE(wRawHotKey);
               HOT_PREV_MOD = HIBYTE(wRawHotKey);

               saveSettings();
               unregisterAssignment();
               registerAssignment();
               EndDialog(hwndDlg,0);
               return 1;
            case IDCANCEL:
               registerAssignment();
               EndDialog(hwndDlg,0);
               return 1;
         }
         break;
      
      case WM_CLOSE:
         EndDialog(hwndDlg,0);
         return TRUE;
	
   }
   return FALSE;
}

/*************************************************
 * Register the assignment hotkey
 */
void registerAssignment()
{
   if(HOT_NEXT)
   {
      hotKeyUp = GlobalAddAtom("AssignmentNext");
      if(RegisterHotKey(hwndMain, hotKeyUp, hotKey2ModKey(HOT_NEXT_MOD), HOT_NEXT) == 0)
         MessageBox(hwndMain, "Invalid key modifier combination, check hot keys!", 
                    NULL, MB_ICONWARNING);
   }
   if(HOT_PREV)
   {
      hotKeyDown = GlobalAddAtom("AssignmentPrev");
      if(RegisterHotKey(hwndMain, hotKeyDown, hotKey2ModKey(HOT_PREV_MOD), HOT_PREV) == 0)
         MessageBox(hwndMain, "Invalid key modifier combination, check hot keys!", 
                    NULL, MB_ICONWARNING);
   }
}

/*************************************************
 * Un-register the assignment hotkey
 */
void unregisterAssignment()
{
   UnregisterHotKey(hwndMain, hotKeyUp);
   UnregisterHotKey(hwndMain, hotKeyDown);
}

//*************************************************

void loadSettings()
{
   char* dummy = malloc(sizeof(char) * 80);
   FILE* fp;
   
   if((fp = fopen(configFile, "r")))
   {
      fscanf(fp, "%s%i", dummy, &HOT_NEXT_MOD);
      fscanf(fp, "%s%i", dummy, &HOT_NEXT);
      fscanf(fp, "%s%i", dummy, &HOT_PREV_MOD);
      fscanf(fp, "%s%i", dummy, &HOT_PREV);
      fclose(fp);
   }
   free(dummy);
}

//*************************************************

void saveSettings()
{
   FILE* fp;
   if(!(fp = fopen(configFile, "w"))) 
   {
      MessageBox(hwndMain, "Assigner", "Error writing config file", MB_ICONWARNING);
   } 
   else 
   {
      fprintf(fp, "next_mod# %i\n", HOT_NEXT_MOD);
      fprintf(fp, "next# %i\n", HOT_NEXT);
      fprintf(fp, "prev_mod# %i\n", HOT_PREV_MOD);
      fprintf(fp, "prev# %i\n", HOT_PREV);
      fclose(fp);
   }
}

//*************************************************

void getConfigDir()
{
   HKEY hkey = NULL;
   DWORD dwType;
   DWORD cbData = 0;
   
   RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\VirtuaWin\\Settings", 0,
                               KEY_READ, &hkey);
   if (hkey) 
   {
      RegQueryValueEx(hkey, "Path", NULL, &dwType, NULL, &cbData);
      vwPath   = (LPSTR)malloc(cbData * sizeof(char));
      RegQueryValueEx(hkey, "Path", NULL, &dwType, (LPBYTE)vwPath, &cbData);
      configFile = (LPSTR)malloc(cbData * sizeof(char) + 21);
      sprintf(configFile, "%smodules\\VWassigner.cfg", vwPath);
   }
   else
   {
      MessageBox(hwndMain, "Could not locate VirtuaWin module directory", "Registry Error", MB_ICONWARNING);
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

/*
 * $Log$
 * Revision 1.1  2003/06/26 19:27:40  jopi
 * Added new VWAssigner module
 *
 */
