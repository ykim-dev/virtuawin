//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K
// 
//  This is a module for VirtuaWin that are used for 
//  restoring lost windows. 
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


#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "winlistres.h"
#include "../Messages.h"

HINSTANCE hInst;   // Instance handle
HWND hwndMain;	   // Main window handle
HWND vwHandle;     // Handle to VirtuaWin

typedef struct {
      HWND handle;
      char winName[104];
} winType;

winType windowList[999]; // should be enough
int noOfWin;
int curSel;

/* prototype for the dialog box function. */
static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/* Main message handler */
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

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
   wc.lpszClassName = "WinList.exe";
  
   if (!RegisterClass(&wc))
      return 0;
  
   return 1;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
          
  case MOD_INIT: // This must be taken care of in order to get the handle to VirtuaWin. 
    // The handle to VirtuaWin comes in the wParam 
    vwHandle = (HWND) wParam; // Should be some error handling here if NULL 
    break;
  case MOD_QUIT: // This must be handeled, otherwise VirtuaWin can't shut down the module 
    PostQuitMessage(0);
    break;
  case MOD_SETUP: // Optional
     DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), vwHandle, (DLGPROC) DialogFunc);
    break;
  case WM_DESTROY:
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
   if ((hwndMain = CreateWindow("WinList.exe", 
                                "WinList", 
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
  

   // main messge loop
   while (GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
   return msg.wParam;
}

/*************************************************
 * Callback function. Integrates all enumerated windows
 */
__inline BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam) 
{
   int style = GetWindowLong(hwnd, GWL_STYLE);
   int exstyle = GetWindowLong(hwnd, GWL_EXSTYLE);
   
   if (!(style & WS_CHILD) && (!GetParent(hwnd) || GetParent(hwnd) == GetDesktopWindow()) &&
       !(exstyle & WS_EX_TOOLWINDOW))
   {
      char tmpString[50];
      char tmpString2[50];
      // Add window to the windowlist
      if(!GetWindowText(hwnd, tmpString, 49))
         strcpy(tmpString, "<None>");
      
      GetClassName(hwnd, tmpString2, 49);
      sprintf( windowList[noOfWin].winName, "%s : (%s)", tmpString, tmpString2);
      
      windowList[noOfWin].handle = hwnd;
      noOfWin++;
   }
   
   return TRUE;
}

/*
 */
static int InitializeApp(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
   int index = 0;
   noOfWin = 0;
   EnumWindows(enumWindowsProc, 0);   // get all windows
   SendDlgItemMessage(hDlg, ID_WINLIST, LB_SETHORIZONTALEXTENT, 110, 0);
   for(;index < noOfWin; index++)
   {
      SendDlgItemMessage(hDlg, ID_WINLIST, LB_ADDSTRING, 0, (LONG)windowList[index].winName);
   }
   curSel = LB_ERR;
   return 1;
}

/*
  This is the main function for the dialog. 
*/
static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch (msg) {
      case WM_INITDIALOG:
         InitializeApp(hwndDlg, wParam, lParam);
         return TRUE;
        
      case WM_COMMAND:
         switch (LOWORD(wParam)) {
            case IDOK:
               curSel = SendDlgItemMessage(hwndDlg, ID_WINLIST, LB_GETCURSEL, 0, 0);
               if(curSel != LB_ERR)
                  ShowWindow(windowList[curSel].handle, SW_SHOWNA);
               return 1;
            case IDUNDO:
               if(curSel != LB_ERR)
               {
                  ShowWindow(windowList[curSel].handle, SW_HIDE);
                  curSel = LB_ERR;
               }
               else
                  MessageBox(vwHandle, "Cannot undo", "Undo", 0);
               return 1;
            case IDCANCEL:
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

/*
 * $Log$
 *
 */
