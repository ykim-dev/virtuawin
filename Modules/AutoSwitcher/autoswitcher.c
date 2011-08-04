//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K/XP
// 
//  This is a module for VirtuaWin that cycles desktops at a 
//  configurable interval
//  
//  Copyright (c) 1999-2005 Johan Piculell
//  Copyright (c) 2006-2011 VirtuaWin (VirtuaWin@home.se)
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
#include <tchar.h>
#include <commctrl.h>

#include "autoswitcherres.h"
#include "../../Messages.h"

#define vwDESKTOP_MAX 20 // be careful with this one, size might change in Defines.h (VirtuaWin core source)

int initialised = 0;
HINSTANCE hInst;   // Instance handle
HWND hwndMain;	   // Main window handle
HWND vwHandle;     // Handle to VirtuaWin
TCHAR *configFile;
ATOM hotKeyToggle;
UINT HOT_TOGGLE;
UINT HOT_TOGGLE_MOD;
BOOL enabled = TRUE;
int myNOfDesks;
int myCurDesk = 1;
int myNextCounter = 1;
int desktopDelay[vwDESKTOP_MAX]; 

/* prototype for the dialog box function. */
static BOOL CALLBACK DialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/* Main message handler */
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

void saveSettings();
void loadSettings();

void stepDesk()
{
   SendMessage(vwHandle, VW_CHANGEDESK, myCurDesk, 0);
   myCurDesk++;
   if(myCurDesk > myNOfDesks)
      myCurDesk = 1;
}

/*************************************************
 * Translates virtual key codes to "hotkey codes"
 */
UINT hotKey2ModKey(UINT vModifiers)
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
 * Registers the hotkey
 */
static void registerKey(void)
{
   if(HOT_TOGGLE)
   {
      hotKeyToggle = GlobalAddAtom(_T("hotKeyToggle"));
      if(RegisterHotKey(hwndMain, hotKeyToggle, hotKey2ModKey(HOT_TOGGLE_MOD), HOT_TOGGLE) == 0)
         MessageBox(hwndMain,_T("Invalid key modifier combination, check hot keys!"),
                    _T("AutoSwitcher Error"), MB_ICONWARNING);
   }
}

/*************************************************
 * Un-register the hotkey
 */
static void unregisterKey(void)
{
   UnregisterHotKey(hwndMain, hotKeyToggle);
}

/* Initializes the window */ 
static BOOL InitApplication(HWND hwnd, char *userAppPath)
{
   TCHAR buff[MAX_PATH];
   
   InitCommonControls();

#ifdef _UNICODE
   MultiByteToWideChar(CP_ACP,0,(char *) userAppPath,-1,buff,MAX_PATH) ;
#else
   strcpy(buff,userAppPath) ;
#endif
   _tcscat(buff,_T("autoswitcher_v2.cfg")) ;
   if((configFile = _tcsdup(buff)) == NULL)
   {
      MessageBox(hwnd,_T("Malloc failure"),_T("AutoSwitcher Error"), MB_ICONWARNING);
      exit(1) ;
   }
   
   loadSettings();
   registerKey();
   SetTimer(hwndMain, 0x50a, (desktopDelay[myCurDesk-1] * 1000), 0); 
   
   return 1;
}

//*************************************************

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   int deskX;
   int deskY;
   COPYDATASTRUCT *cds;

   switch (msg) {
      // from VirtuaWin, make sure we are in sync with current desktop
      case MOD_CHANGEDESK:
         KillTimer(hwndMain, 0x50a);
         myCurDesk = (int) lParam;
         SetTimer(hwndMain, 0x50a, (desktopDelay[myCurDesk-1] * 1000), 0); 
         // can't figure it out right now but a break here disturbs the switch timing
      // Time trigger
      case WM_TIMER:
         if(enabled)
            stepDesk();
         break;
      
      case WM_HOTKEY:
         if(wParam == hotKeyToggle)
         {
            if(enabled)
               enabled = FALSE;
            else
            {
               enabled = TRUE;
               stepDesk();
            }
         }
         break;

      case WM_COPYDATA:
         cds = (COPYDATASTRUCT *) lParam ;         
         if((cds->dwData == (0-VW_USERAPPPATH)) && !initialised)
         {
            if((cds->cbData < 2) || (cds->lpData == NULL))
               return FALSE ;
            initialised = 1 ;
            InitApplication(hwnd,(char *) cds->lpData) ;
         }
         return TRUE ;
         
      case MOD_INIT: // This must be taken care of in order to get the handle to VirtuaWin. 
         // The handle to VirtuaWin comes in the wParam 
         vwHandle = (HWND) wParam; // Should be some error handling here if NULL 
         deskX = SendMessage(vwHandle, VW_DESKX, 0, 0);
         deskY = SendMessage(vwHandle, VW_DESKY, 0, 0);
         myNOfDesks = deskX * deskY;
         myCurDesk = SendMessage(vwHandle, VW_CURDESK, 0, 0);
         if(!initialised)
         {
            SendMessage(vwHandle, VW_USERAPPPATH, (WPARAM) hwnd, 0) ;
            if(!initialised)
            {
               MessageBox(hwnd,_T("VirtuaWin failed to send the UserApp path."), _T("AutoSwitcher Error"), MB_ICONWARNING);
               exit(1) ;
            }
         }
         break;
      case MOD_QUIT: // This must be handeled, otherwise VirtuaWin can't shut down the module 
         PostQuitMessage(0);
         break;
      case MOD_SETUP:
         DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), vwHandle, (DLGPROC) DialogFunc);
         break;
      case WM_DESTROY:
         unregisterKey();
         KillTimer(hwndMain, 0x50a);
         PostQuitMessage(0);
         break;
      case MOD_CFGCHANGE:
         deskX = SendMessage(vwHandle, VW_DESKX, 0, 0);
         deskY = SendMessage(vwHandle, VW_DESKY, 0, 0);
         myNOfDesks = deskX * deskY;
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
   WNDCLASS wc;
   hInst = hInstance;
   
   memset(&wc, 0, sizeof(WNDCLASS));
   wc.style = 0;
   wc.lpfnWndProc = (WNDPROC)MainWndProc;
   wc.hInstance = hInst;
   /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
      this for locating the window */
   wc.lpszClassName = _T("AutoSwitcher.exe");
   
   if (!RegisterClass(&wc))
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
   TCHAR buff[11];
   WORD wRawHotKey;
   
   switch (msg) {
      case WM_INITDIALOG:
         KillTimer(hwndMain, 0x50a);
         myNextCounter = 1;
         SetDlgItemInt(hwndDlg, IDC_INTERVAL, desktopDelay[myNextCounter-1], TRUE);
         _stprintf(buff,_T("Display time of desktop %d:"), myNextCounter);
         SetDlgItemText(hwndDlg, IDC_DESKTOPLBL, buff) ;
         unregisterKey();
         SendDlgItemMessage(hwndDlg, IDC_HOTKEY_ENT, HKM_SETHOTKEY, 
                            MAKEWORD(HOT_TOGGLE, HOT_TOGGLE_MOD), 0);
         return TRUE;
         
      case WM_COMMAND:
         switch (LOWORD(wParam)) {
            case IDOK:
               GetDlgItemText(hwndDlg, IDC_INTERVAL, buff, 11);
               desktopDelay[myNextCounter-1] = strtol(buff, 0, 10);

               wRawHotKey = (WORD)SendDlgItemMessage(hwndDlg, IDC_HOTKEY_ENT, HKM_GETHOTKEY, 0, 0);
               HOT_TOGGLE = LOBYTE(wRawHotKey);
               HOT_TOGGLE_MOD = HIBYTE(wRawHotKey);
               
               saveSettings();
               registerKey();
               SetTimer(hwndMain, 0x50a, (desktopDelay[myCurDesk] * 1000), 0); 
               
               EndDialog(hwndDlg,0);
               return 1;
            case IDCANCEL:
               loadSettings();
               registerKey();
               SetTimer(hwndMain, 0x50a, (desktopDelay[myCurDesk] * 1000), 0); 
               EndDialog(hwndDlg,0);
               return 1;
            case IDC_DESKTOPBTN:
               GetDlgItemText(hwndDlg, IDC_INTERVAL, buff, 11);
               int value = strtol(buff, 0, 10);
               // Avoid 0 since that will cause out of control switching
               if(value >= 1)
               {
                  desktopDelay[myNextCounter-1] = value;
                  if(myNextCounter >= myNOfDesks)
                     myNextCounter = 0;
                  _stprintf(buff,_T("Display time of desktop %d:"), ++myNextCounter);
                  SetDlgItemText(hwndDlg, IDC_DESKTOPLBL, buff);
                  SetDlgItemInt(hwndDlg, IDC_INTERVAL, desktopDelay[myNextCounter-1], TRUE);
               }
               else
               {
                  _stprintf(buff,_T("Invalid value entered."));
                  SetDlgItemText(hwndDlg, IDC_DESKTOPLBL, buff);
               }
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
    int i;

    if((fp = _tfopen(configFile, _T("r"))))
    {
       for(i = 0; i < vwDESKTOP_MAX; i++)
       {
          fscanf(fp, "%s%d", dummy, &desktopDelay[i]);
       }
       fscanf(fp, "%s%i", dummy, &HOT_TOGGLE_MOD);
       fscanf(fp, "%s%i", dummy, &HOT_TOGGLE);
       fclose(fp);
    }
    else
    {
       // config not found, init with default values
       for(i = 0; i < vwDESKTOP_MAX; i++)
       {
          desktopDelay[i] = 10;
       }
    }
}

//*************************************************

void saveSettings()
{
   FILE* fp;
   if(!(fp = _tfopen(configFile,_T("w")))) 
   {
      MessageBox(hwndMain, _T("AutoSwitcher, Error writing config file"), _T("AutoSwitcher Error"), MB_ICONWARNING);
   } 
   else 
   {
      int i = 0;
      for(i = 0; i < vwDESKTOP_MAX; i++)
      {
         fprintf(fp, "interval# %d\n", desktopDelay[i]);
      }
      fprintf(fp, "toggle_mod# %i\n", HOT_TOGGLE_MOD);
      fprintf(fp, "toggle# %i\n", HOT_TOGGLE);
      fclose(fp);
   }
}

//*************************************************

