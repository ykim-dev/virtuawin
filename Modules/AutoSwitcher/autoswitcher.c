//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K/XP
// 
//  This is a module for VirtuaWin that cycles desktops at a 
//  configurable interval
//  
// 
//  Copyright (c) 2003 Johan Piculell
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

#include "autoswitcherres.h"
#include "../../Messages.h"

HINSTANCE hInst;   // Instance handle
HWND hwndMain;	   // Main window handle
HWND vwHandle;     // Handle to VirtuaWin
LPSTR vwPath;
LPSTR configFile;
int myInterval = 2;
int myNOfDesks;
int myCurDesk = 1;

/* prototype for the dialog box function. */
static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/* Main message handler */
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

void getConfigDir();
void saveSettings();
void loadSettings();

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
   wc.lpszClassName = "AutoSwitcher.exe";
   
   if (!RegisterClass(&wc))
      return 0;
  
   return 1;
}

//*************************************************

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   int deskX;
   int deskY;
   
   switch (msg) {
      case WM_TIMER:
         SendMessage(vwHandle, VW_CHANGEDESK, myCurDesk, 0);
         myCurDesk++;
         if(myCurDesk > myNOfDesks)
            myCurDesk = 1;
         break;
      case MOD_INIT: // This must be taken care of in order to get the handle to VirtuaWin. 
         // The handle to VirtuaWin comes in the wParam 
         vwHandle = (HWND) wParam; // Should be some error handling here if NULL 
         deskX = SendMessage(vwHandle, VW_DESKX, 0, 0);
         deskY = SendMessage(vwHandle, VW_DESKY, 0, 0);
         myNOfDesks = deskX * deskY;
         break;
      case MOD_QUIT: // This must be handeled, otherwise VirtuaWin can't shut down the module 
         PostQuitMessage(0);
         break;
      case MOD_SETUP:
         DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), vwHandle, (DLGPROC) DialogFunc);
         break;
      case WM_DESTROY:
         KillTimer(hwndMain, 0x50a);
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
   if ((hwndMain = CreateWindow("AutoSwitcher.exe", 
                                "AutoSwitcher", 
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
   SetTimer(hwndMain, 0x50a, (myInterval * 1000), 0); 
   
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
   char buff[5];
   
   switch (msg) {
      case WM_INITDIALOG:
         KillTimer(hwndMain, 0x50a);
         SetDlgItemInt(hwndDlg, IDC_INTERVAL, myInterval, TRUE);
         return TRUE;
         
      case WM_COMMAND:
         switch (LOWORD(wParam)) {
            case IDOK:
               GetDlgItemText(hwndDlg, IDC_INTERVAL, buff, 4);
               myInterval = atoi(buff);
               saveSettings();
               SetTimer(hwndMain, 0x50a, (myInterval * 1000), 0); 
               EndDialog(hwndDlg,0);
               return 1;
            case IDCANCEL:
               SetTimer(hwndMain, 0x50a, (myInterval * 1000), 0); 
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

//*************************************************

void loadSettings()
{
   char dummy[80];
   FILE* fp;
   
   if((fp = fopen(configFile, "r")))
   {
      fscanf(fp, "%s%i", &dummy, &myInterval);
      fclose(fp);
   }
}

//*************************************************

void saveSettings()
{
   FILE* fp;
   if(!(fp = fopen(configFile, "w"))) 
   {
      MessageBox("AutoSwitcher", "Error writing config file", NULL, MB_ICONWARNING);
   } 
   else 
   {
      fprintf(fp, "interval# %i\n", myInterval);
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
      sprintf(configFile, "%smodules\\AutoSwitcher.cfg", vwPath);
   }
   else
   {
      MessageBox(hwndMain, "Could not locate VirtuaWin module directory", "Registry Error", MB_ICONWARNING);
   }
}

/*
 * $Log$
 *
 */
