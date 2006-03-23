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
#include "SetupDialog.h"
#include "Resource.h"
#include "ListStructures.h"
#include "Messages.h"
#include "DiskRoutines.h"
#include "ModuleRoutines.h"
#include "ConfigParameters.h"

// Standard includes
#include <stdlib.h>
#include <stdio.h>
#include <shellapi.h>
#include <prsht.h>
#include <commctrl.h>

static int pageChangeMask=0 ;
static int pageApplyMask=0 ;

static void vwSetupApply(HWND hDlg, int curPageMask)
{
    pageApplyMask |= curPageMask ;
    if(pageApplyMask == pageApplyMask)
    {
        // All pages have now got any changes from the GUI, save them and apply
        writeConfig();
        getTaskbarLocation();
        unRegisterAllKeys();
        registerAllKeys();
        enableMouse(mouseEnable);
        setMouseKey();
        // Tell modules about the config change
        postModuleMessage(MOD_CFGCHANGE, 0, 0);
        pageChangeMask = 0 ;
        pageApplyMask = 0 ;
    }
}

/*************************************************
 * The "Key" tab callback
 * This is the firts callback to be called when the property sheet is created
 */
BOOL APIENTRY keys(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    WORD wRawHotKey;
    WORD wPar;
    RECT config_dlg_rect;
    
    switch (message) {
    case WM_INITDIALOG:

         GetWindowRect(GetParent(hDlg), &config_dlg_rect);

         // Reposition the dialog to the center of the monitor that
         // was selected
            
         // GetSystemMetrics returns the primary monitor for backward
         // compatiblity.  I used this rather than GetMonitorInfo
         // because some versions of MingW don't have that function
         // and because those other functions only work on 98+ and
         // 2000+
         int monitor_width  = GetSystemMetrics(SM_CXSCREEN); // get the width of the PRIMARY display monitor
         int monitor_height = GetSystemMetrics(SM_CYSCREEN); // get the height of the PRIMARY display monitor
         int dialog_width   = config_dlg_rect.right  - config_dlg_rect.left;
         int dialog_height  = config_dlg_rect.bottom - config_dlg_rect.top;

         config_dlg_rect.left = (monitor_width  - dialog_width)  / 2;
         config_dlg_rect.top  = (monitor_height - dialog_height) / 2;

         SetWindowPos(GetParent(hDlg), 0, config_dlg_rect.left, config_dlg_rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    
        /* Control keys */
        if(keyEnable) {
            SendDlgItemMessage(hDlg, IDC_KEYS, BM_SETCHECK, 1,0);
        }
        if(modAlt)
            SendDlgItemMessage(hDlg, IDC_ALT, BM_SETCHECK, 1,0);
        if(modShift)
            SendDlgItemMessage(hDlg, IDC_SHIFT, BM_SETCHECK, 1,0);
        if(modCtrl)
            SendDlgItemMessage(hDlg, IDC_CTRL, BM_SETCHECK, 1,0);
        if(modWin)
            SendDlgItemMessage(hDlg, IDC_WIN, BM_SETCHECK, 1,0);
        
        /* Cycling key */
        SendDlgItemMessage(hDlg, IDC_CYCLINGKEYS, BM_SETCHECK,(cyclingKeysEnabled != 0),0);
        SendDlgItemMessage(hDlg, IDC_HOTCYCLEUP, HKM_SETHOTKEY, 
                           MAKEWORD(hotCycleUp, hotCycleUpMod), 0);
        SendDlgItemMessage(hDlg, IDC_HOTCYCLEUPW, BM_SETCHECK,(hotCycleUpWin != 0),0);
        SendDlgItemMessage(hDlg, IDC_HOTCYCLEDOWN, HKM_SETHOTKEY, 
                           MAKEWORD(hotCycleDown, hotCycleDownMod), 0);
        SendDlgItemMessage(hDlg, IDC_HOTCYCLEDOWNW, BM_SETCHECK,(hotCycleDownWin != 0),0);
        
        /* Hot keys */
        if(hotKeyEnable) {
            SendDlgItemMessage(hDlg, IDC_HOTKEYS, BM_SETCHECK, 1,0);
        }
        if(deskHotkeyWin[1])
            SendDlgItemMessage(hDlg, IDC_HOT1W, BM_SETCHECK, 1,0);
        SendDlgItemMessage(hDlg, IDC_HOT1, HKM_SETHOTKEY, MAKEWORD(deskHotkey[1], deskHotkeyMod[1]), 0);
        if(deskHotkeyWin[2])
            SendDlgItemMessage(hDlg, IDC_HOT2W, BM_SETCHECK, 1,0);
        SendDlgItemMessage(hDlg, IDC_HOT2, HKM_SETHOTKEY, MAKEWORD(deskHotkey[2], deskHotkeyMod[2]), 0);
        if(deskHotkeyWin[3])
            SendDlgItemMessage(hDlg, IDC_HOT3W, BM_SETCHECK, 1,0);
        SendDlgItemMessage(hDlg, IDC_HOT3, HKM_SETHOTKEY, MAKEWORD(deskHotkey[3], deskHotkeyMod[3]), 0);
        if(deskHotkeyWin[4])
            SendDlgItemMessage(hDlg, IDC_HOT4W, BM_SETCHECK, 1,0);
        SendDlgItemMessage(hDlg, IDC_HOT4, HKM_SETHOTKEY, MAKEWORD(deskHotkey[4], deskHotkeyMod[4]), 0);
        if(deskHotkeyWin[5])
            SendDlgItemMessage(hDlg, IDC_HOT5W, BM_SETCHECK, 1,0);
        SendDlgItemMessage(hDlg, IDC_HOT5, HKM_SETHOTKEY, MAKEWORD(deskHotkey[5], deskHotkeyMod[5]), 0);
        if(deskHotkeyWin[6])
            SendDlgItemMessage(hDlg, IDC_HOT6W, BM_SETCHECK, 1,0);
        SendDlgItemMessage(hDlg, IDC_HOT6, HKM_SETHOTKEY, MAKEWORD(deskHotkey[6], deskHotkeyMod[6]), 0);
        if(deskHotkeyWin[7])
            SendDlgItemMessage(hDlg, IDC_HOT7W, BM_SETCHECK, 1,0);
        SendDlgItemMessage(hDlg, IDC_HOT7, HKM_SETHOTKEY, MAKEWORD(deskHotkey[7], deskHotkeyMod[7]), 0);
        if(deskHotkeyWin[8])
            SendDlgItemMessage(hDlg, IDC_HOT8W, BM_SETCHECK, 1,0);
        SendDlgItemMessage(hDlg, IDC_HOT8, HKM_SETHOTKEY, MAKEWORD(deskHotkey[8], deskHotkeyMod[8]), 0);
        if(deskHotkeyWin[9])
            SendDlgItemMessage(hDlg, IDC_HOT9W, BM_SETCHECK, 1,0);
        SendDlgItemMessage(hDlg, IDC_HOT9, HKM_SETHOTKEY, MAKEWORD(deskHotkey[9], deskHotkeyMod[9]), 0);
        return (TRUE);
        
    case WM_NOTIFY:
        
        switch (((NMHDR FAR *) lParam)->code) {
        case PSN_SETACTIVE: // Getting focus
            // Initialize the controls.
            break;
        case PSN_APPLY: // Apply, OK
            // Cycle hot keys
            if(SendDlgItemMessage(hDlg, IDC_CYCLINGKEYS, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                cyclingKeysEnabled = TRUE;
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOTCYCLEUP, HKM_GETHOTKEY, 0, 0);
                hotCycleUp = LOBYTE(wRawHotKey);
                hotCycleUpMod = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOTCYCLEUPW, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    hotCycleUpWin = MOD_WIN;
                else
                    hotCycleUpWin = FALSE;
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOTCYCLEDOWN, HKM_GETHOTKEY, 0, 0);
                hotCycleDown = LOBYTE(wRawHotKey);
                hotCycleDownMod = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOTCYCLEDOWNW, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    hotCycleDownWin = MOD_WIN;
                else
                    hotCycleDownWin = FALSE;
            }
            else
                cyclingKeysEnabled = FALSE;
            
            // Hot key controls
            if(SendDlgItemMessage(hDlg, IDC_HOTKEYS, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                hotKeyEnable = TRUE;
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOT1, HKM_GETHOTKEY, 0, 0);
                deskHotkey[1] = LOBYTE(wRawHotKey);
                deskHotkeyMod[1] = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOT1W, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    deskHotkeyWin[1] = MOD_WIN;
                else
                    deskHotkeyWin[1] = FALSE;
                
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOT2, HKM_GETHOTKEY, 0, 0);
                deskHotkey[2] = LOBYTE(wRawHotKey);
                deskHotkeyMod[2] = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOT2W, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    deskHotkeyWin[2] = MOD_WIN;
                else
                    deskHotkeyWin[2] = FALSE;
                
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOT3, HKM_GETHOTKEY, 0, 0);
                deskHotkey[3] = LOBYTE(wRawHotKey);
                deskHotkeyMod[3] = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOT3W, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    deskHotkeyWin[3] = MOD_WIN;
                else
                    deskHotkeyWin[3] = FALSE;
                
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOT4, HKM_GETHOTKEY, 0, 0);
                deskHotkey[4] = LOBYTE(wRawHotKey);
                deskHotkeyMod[4] = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOT4W, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    deskHotkeyWin[4] = MOD_WIN;
                else
                    deskHotkeyWin[4] = FALSE;
                
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOT5, HKM_GETHOTKEY, 0, 0);
                deskHotkey[5] = LOBYTE(wRawHotKey);
                deskHotkeyMod[5] = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOT5W, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    deskHotkeyWin[5] = MOD_WIN;
                else
                    deskHotkeyWin[5] = FALSE;
                
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOT6, HKM_GETHOTKEY, 0, 0);
                deskHotkey[6] = LOBYTE(wRawHotKey);
                deskHotkeyMod[6] = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOT6W, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    deskHotkeyWin[6] = MOD_WIN;
                else
                    deskHotkeyWin[6] = FALSE;
                
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOT7, HKM_GETHOTKEY, 0, 0);
                deskHotkey[7] = LOBYTE(wRawHotKey);
                deskHotkeyMod[7] = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOT7W, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    deskHotkeyWin[7] = MOD_WIN;
                else
                    deskHotkeyWin[7] = FALSE;
                
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOT8, HKM_GETHOTKEY, 0, 0);
                deskHotkey[8] = LOBYTE(wRawHotKey);
                deskHotkeyMod[8] = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOT8W, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    deskHotkeyWin[8] = MOD_WIN;
                else
                    deskHotkeyWin[8] = FALSE;
                
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOT9, HKM_GETHOTKEY, 0, 0);
                deskHotkey[9] = LOBYTE(wRawHotKey);
                deskHotkeyMod[9] = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOT9W, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    deskHotkeyWin[9] = MOD_WIN;
                else
                    deskHotkeyWin[9] = FALSE;
            }
            hotKeyEnable = FALSE;
            
            // Control keys
            if(SendDlgItemMessage(hDlg, IDC_KEYS, BM_GETCHECK, 0, 0) == BST_CHECKED) {
                keyEnable = TRUE;
                if(SendDlgItemMessage(hDlg, IDC_ALT, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    modAlt = MOD_ALT;
                else
                    modAlt = FALSE;
                if(SendDlgItemMessage(hDlg, IDC_SHIFT, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    modShift = MOD_SHIFT;
                else
                    modShift = FALSE;
                if(SendDlgItemMessage(hDlg, IDC_CTRL, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    modCtrl = MOD_CONTROL;
                else
                    modCtrl = FALSE;
                if(SendDlgItemMessage(hDlg, IDC_WIN, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    modWin = MOD_WIN;
                else
                    modWin = FALSE;
            }
            else {
                keyEnable = FALSE;
            }
            
            // Hot keys
            if(SendDlgItemMessage(hDlg, IDC_HOTKEYS, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                hotKeyEnable = TRUE;
                if(SendDlgItemMessage(hDlg, IDC_ALTHOT, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    hotModAlt = MOD_ALT;
                else
                    hotModAlt = FALSE;
                if(SendDlgItemMessage(hDlg, IDC_SHIFTHOT, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    hotModShift = MOD_SHIFT;
                else
                    hotModShift = FALSE;
                if(SendDlgItemMessage(hDlg, IDC_CTRLHOT, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    hotModCtrl = MOD_CONTROL;
                else
                    hotModCtrl = FALSE;
                if(SendDlgItemMessage(hDlg, IDC_WINHOT, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    hotModWin = MOD_WIN;
                else
                    hotModWin = FALSE;
            }
            else {
                hotKeyEnable = FALSE;
            }
            vwSetupApply(hDlg,0x01) ;
            SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
            break;
        case PSN_KILLACTIVE: // Switch tab sheet
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            return 1;
        case PSN_RESET: // Cancel
            // Reset to the original values.
            readConfig();
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            break;
        case PSN_HELP:
            showHelp(hDlg);
            break;
        }
        
    case WM_COMMAND:
        wPar = LOWORD(wParam);
        if(wPar == IDC_KEYS || wPar == IDC_ALT || wPar == IDC_SHIFT  ||
           wPar == IDC_CTRL || wPar == IDC_WIN  || wPar == IDC_HOTKEYS ||
           wPar == IDC_HOT1 || wPar == IDC_HOT2 || wPar == IDC_HOT3 ||
           wPar == IDC_HOT4 || wPar == IDC_HOT5 || wPar == IDC_HOT6 ||
           wPar == IDC_HOT7 || wPar == IDC_HOT8 || wPar == IDC_HOT9 ||
           wPar == IDC_HOT1W || wPar == IDC_HOT2W || wPar == IDC_HOT3W ||
           wPar == IDC_HOT4W || wPar == IDC_HOT5W || wPar == IDC_HOT6W ||
           wPar == IDC_HOT7W || wPar == IDC_HOT8W || wPar == IDC_HOT9W ||
           wPar == IDC_CYCLINGKEYS || wPar == IDC_HOTCYCLEUP || wPar == IDC_HOTCYCLEUPW ||
           wPar == IDC_HOTCYCLEDOWN || wPar == IDC_HOTCYCLEDOWNW )
        {
            pageChangeMask |= 0x01 ;
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L); // Enable apply button
        }
        break;
    }
    return (FALSE);
}

/*************************************************
 * The "Mouse" tab callback
 */
BOOL APIENTRY mouse(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    char buff[5];
    
    switch (message) {
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, IDC_TIME, configMultiplier * 50, FALSE);
        SendDlgItemMessage(hDlg, IDC_SLIDER, TBM_SETRANGE, TRUE, MAKELONG(1, 80));
        SendDlgItemMessage(hDlg, IDC_SLIDER, TBM_SETPOS, TRUE, configMultiplier);
        SetDlgItemInt(hDlg, IDC_JUMP, warpLength, TRUE);
        
        /* Mouse Control keys */
        if(useMouseKey) {
            SendDlgItemMessage(hDlg, IDC_KEYCONTROL, BM_SETCHECK, 1,0);
        }
        if(mouseModAlt)
            SendDlgItemMessage(hDlg, IDC_ALT, BM_SETCHECK, 1,0);
        if(mouseModShift)
            SendDlgItemMessage(hDlg, IDC_SHIFT, BM_SETCHECK, 1,0);
        if(mouseModCtrl)
            SendDlgItemMessage(hDlg, IDC_CTRL, BM_SETCHECK, 1,0);
        if(mouseEnable) 
            SendDlgItemMessage(hDlg, IDC_MOUSWARP, BM_SETCHECK, 1,0);
        if(taskBarWarp)
            SendDlgItemMessage(hDlg, IDC_TASKBAR, BM_SETCHECK, 1,0);
        if(noMouseWrap)
            SendDlgItemMessage(hDlg, IDC_MOUSEWRAP, BM_SETCHECK, 1,0);
        
        return TRUE;
        
    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code) {
        case PSN_SETACTIVE:
            // Initialize the controls. Only if we want to reinitialize on tab change.
            break;
            
        case PSN_APPLY:
            GetDlgItemText(hDlg, IDC_JUMP, buff, 4);
            warpLength = atoi(buff);
            configMultiplier = SendDlgItemMessage(hDlg, IDC_SLIDER, TBM_GETPOS, 0, 0);
            if(SendDlgItemMessage(hDlg, IDC_MOUSWARP, BM_GETCHECK, 0, 0) == BST_CHECKED)
                mouseEnable = TRUE;
            else
                mouseEnable = FALSE;
            
            if(SendDlgItemMessage(hDlg, IDC_TASKBAR, BM_GETCHECK, 0, 0) == BST_CHECKED)
                taskBarWarp = TRUE;
            else
                taskBarWarp = FALSE;
            if(SendDlgItemMessage(hDlg, IDC_MOUSEWRAP, BM_GETCHECK, 0, 0) == BST_CHECKED)
                noMouseWrap = TRUE;
            else
                noMouseWrap= FALSE;
            
            if(SendDlgItemMessage(hDlg, IDC_KEYCONTROL, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                useMouseKey = TRUE;
                mouseModShift = FALSE;
                mouseModAlt = FALSE;
                mouseModCtrl = FALSE;
                if(SendDlgItemMessage(hDlg, IDC_ALT, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    mouseModAlt = TRUE;
                if(SendDlgItemMessage(hDlg, IDC_SHIFT, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    mouseModShift = TRUE;
                if(SendDlgItemMessage(hDlg, IDC_CTRL, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    mouseModCtrl = TRUE;
            }
            else
                useMouseKey = FALSE;
            vwSetupApply(hDlg,0x02) ;
            SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
            break;
        case PSN_KILLACTIVE:
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            return 1;
        case PSN_RESET:
            // Reset to the original values.
            readConfig();
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            break;
        case PSN_HELP:
            showHelp(hDlg);
            break;
        }
        
    case WM_COMMAND:
        if(LOWORD(wParam) == IDC_MOUSWARP || LOWORD(wParam) == IDC_TASKBAR || 
           LOWORD(wParam) == IDC_JUMP || LOWORD(wParam) == IDC_KEYCONTROL || 
           LOWORD(wParam) == IDC_CTRL || LOWORD(wParam) == IDC_ALT || 
           LOWORD(wParam) == IDC_SHIFT || 
           LOWORD(wParam) == IDC_MOUSEWRAP)
        {
            pageChangeMask |= 0x02 ;
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L); // Enable apply
        }
        break;
        
    case WM_HSCROLL:
        pageChangeMask |= 0x02 ;
        SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
        SetDlgItemInt(hDlg, IDC_TIME, (SendDlgItemMessage(hDlg, IDC_SLIDER, TBM_GETPOS, 0, 0) * 50), TRUE);
        break;
    }
    return FALSE;
}

/*************************************************
 * The "Modules" tab callback
 */
BOOL APIENTRY modules(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int index;
    char tmpName[80];
    
    switch (message) {
    case WM_INITDIALOG:
        for(index = 0; index < nOfModules; index++) {
            if(moduleList[index].Disabled)
                sprintf(tmpName, "* %s", moduleList[index].description);
            else
                sprintf(tmpName, "%s", moduleList[index].description);
            SendDlgItemMessage(hDlg, IDC_MODLIST, LB_ADDSTRING, 0, (LONG)tmpName);
        }
        return TRUE;
    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code) {
        case PSN_SETACTIVE:
            // Initialize the controls.
            break;
        case PSN_APPLY:
            SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
            break;
        case PSN_KILLACTIVE:
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            return 1;
        case PSN_RESET:
            // Reset to the original values.
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            break;
        case PSN_HELP:
            showHelp(hDlg);
            break;
        }
        
    case WM_COMMAND:
        if(LOWORD((wParam) == IDC_MODCONFIG || HIWORD(wParam) == LBN_DBLCLK )) { // Show config
            int curSel = SendDlgItemMessage(hDlg, IDC_MODLIST, LB_GETCURSEL, 0, 0);
            if(curSel != LB_ERR)
                PostMessage(moduleList[curSel].Handle, MOD_SETUP, 0, 0);
        } else if(LOWORD((wParam) == IDC_MODRELOAD)) { // Reload
            SendDlgItemMessage(hDlg, IDC_MODLIST, LB_RESETCONTENT, 0, 0);
            saveDisabledList(nOfModules, moduleList);
            unloadModules();
            for(index = 0; index < MAXMODULES; index++) {
                moduleList[index].Handle = NULL;
                moduleList[index].description[0] = '\0';
            }
            nOfModules = 0;
            curDisabledMod = loadDisabledModules(disabledModules);
            loadModules();
            for(index = 0; index < nOfModules; index ++) {
                if(moduleList[index].Disabled)
                    sprintf(tmpName, "* %s", moduleList[index].description);
                else
                    sprintf(tmpName, "%s", moduleList[index].description);
                SendDlgItemMessage(hDlg, IDC_MODLIST, LB_ADDSTRING, 0, (LONG)tmpName);
            }
        } else if(LOWORD((wParam) == IDC_MODDISABLE)) { // Enable/Disable
            int curSel = SendDlgItemMessage(hDlg, IDC_MODLIST, LB_GETCURSEL, 0, 0);
            if(curSel != LB_ERR) {
                if(moduleList[curSel].Disabled == FALSE) { // let's disable
                    moduleList[curSel].Disabled = TRUE;
                    PostMessage(moduleList[curSel].Handle, MOD_QUIT, 0, 0);
                    SendDlgItemMessage(hDlg, IDC_MODLIST, LB_RESETCONTENT, 0, 0);
                    for(index = 0; index < nOfModules; index ++) {
                        if(moduleList[index].Disabled)
                            sprintf(tmpName, "* %s", moduleList[index].description);
                        else
                            sprintf(tmpName, "%s", moduleList[index].description);
                        SendDlgItemMessage(hDlg, IDC_MODLIST, LB_ADDSTRING, 0, (LONG)tmpName);
                    }
                } else { // let's enable
                    MessageBox(hDlg, "Press reload or restart VirtuaWin to enable the module", "Note!", 0);
                    moduleList[curSel].Disabled = FALSE;
                    SendDlgItemMessage(hDlg, IDC_MODLIST, LB_RESETCONTENT, 0, 0);
                    for(index = 0; index < nOfModules; index ++) {
                        if(moduleList[index].Disabled)
                            sprintf(tmpName, "* %s", moduleList[index].description);
                        else
                            sprintf(tmpName, "%s", moduleList[index].description);
                        SendDlgItemMessage(hDlg, IDC_MODLIST, LB_ADDSTRING, 0, (LONG)tmpName);
                    }
                }
            }
        }
        break;
    }
    
    return (FALSE);
}

/*************************************************
 * The "Misc." tab callback
 */
BOOL APIENTRY misc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static int tmpDesksY;
    static int tmpDesksX;
    static HWND xBuddy;
    static HWND yBuddy;
    static int spinPressed;
    WORD wRawHotKey;
    int maxDesk ;
    
    switch (message) {
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, IDC_DESKY, nDesksY, FALSE);
        SetDlgItemInt(hDlg, IDC_DESKX, nDesksX, FALSE);
        tmpDesksY = nDesksY;
        tmpDesksX = nDesksX;
        // Get a handle to desktop edit controls 
        xBuddy = GetDlgItem(hDlg, IDC_DESKX);
        yBuddy = GetDlgItem(hDlg, IDC_DESKY);
        // Set the spin buddy controls
        SendMessage(GetDlgItem(hDlg, IDC_SLIDERX), UDM_SETBUDDY, (LONG)xBuddy, 0L );
        SendMessage(GetDlgItem(hDlg, IDC_SLIDERY), UDM_SETBUDDY, (LONG)yBuddy, 0L );
        // Set spin ranges
        SendMessage(GetDlgItem(hDlg, IDC_SLIDERX), UDM_SETRANGE, 0L, MAKELONG(9, 1));
        SendMessage(GetDlgItem(hDlg, IDC_SLIDERY), UDM_SETRANGE, 0L, MAKELONG(9, 1));
        // Set init values
        SendMessage(GetDlgItem(hDlg, IDC_SLIDERX), UDM_SETPOS, 0L, MAKELONG( nDesksX, 0));
        SendMessage(GetDlgItem(hDlg, IDC_SLIDERY), UDM_SETPOS, 0L, MAKELONG( nDesksY, 0));
        
        if(saveSticky)
            SendDlgItemMessage(hDlg, IDC_STICKYSAVE, BM_SETCHECK, 1,0);
        if(stickyMenu) 
            SendDlgItemMessage(hDlg, IDC_MENUSTICKY, BM_SETCHECK, 1,0);
        if(assignMenu)
            SendDlgItemMessage(hDlg, IDC_MENUASSIGN, BM_SETCHECK, 1,0);
        if(directMenu)
            SendDlgItemMessage(hDlg, IDC_MENUACCESS, BM_SETCHECK, 1,0);
        if(useDeskAssignment)
            SendDlgItemMessage(hDlg, IDC_USEASSIGN, BM_SETCHECK, 1,0);
        if(saveLayoutOnExit)
            SendDlgItemMessage(hDlg, IDC_SAVEEXITSTATE, BM_SETCHECK, 1,0);
        if(assignOnlyFirst)
            SendDlgItemMessage(hDlg, IDC_FIRSTONLY, BM_SETCHECK, 1,0);
        if(assignImmediately)
            SendDlgItemMessage(hDlg, IDC_ASSIGNWINNOW, BM_SETCHECK, 1,0);
        SendDlgItemMessage(hDlg, IDC_HOTMENUEN, BM_SETCHECK,(hotkeyMenuEn != 0),0);
        SendDlgItemMessage(hDlg, IDC_HOTMENU, HKM_SETHOTKEY, MAKEWORD(hotkeyMenu, hotkeyMenuMod), 0);
        SendDlgItemMessage(hDlg, IDC_HOTMENUW, BM_SETCHECK,(hotkeyMenuWin != 0),0);
        SendDlgItemMessage(hDlg, IDC_HOTSTICKYEN, BM_SETCHECK,(hotkeyStickyEn != 0),0);
        SendDlgItemMessage(hDlg, IDC_HOTSTICKY, HKM_SETHOTKEY, MAKEWORD(hotkeySticky, hotkeyStickyMod), 0);
        SendDlgItemMessage(hDlg, IDC_HOTSTICKYW, BM_SETCHECK,(hotkeyStickyWin != 0),0);
        return TRUE;
        
    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code) {
        case PSN_SETACTIVE:
            // Initialize the controls.
            break;
        case PSN_APPLY:
            tmpDesksY = GetDlgItemInt(hDlg, IDC_DESKY, NULL, FALSE);
            tmpDesksX = GetDlgItemInt(hDlg, IDC_DESKX, NULL, FALSE);
            maxDesk = tmpDesksX * tmpDesksY ;
            if(maxDesk < (nDesksX * nDesksY))
            {
                int index, count=0 ;
                
                if((currentDesk > maxDesk) && (currentDesk < vwDESK_PRIVATE1))
                    // user is on an invalid desk, move
                    gotoDesk(1,TRUE);
                index = nWin ;
                while(--index >= 0)
                {
                    if((winList[index].Desk > maxDesk) && (winList[index].Desk < vwDESK_PRIVATE1) && !winList[index].Sticky)
                    {
                        // This window is on an invalid desk, move
                        assignWindow(winList[index].Handle,1,TRUE);
                        count++ ;
                        // must start again as the list will have been updated
                        index = nWin ; 
                    }
                }
            }
            nDesksY = tmpDesksY;
            nDesksX = tmpDesksX;
            reLoadIcons();              
            
            if(SendDlgItemMessage(hDlg, IDC_STICKYSAVE, BM_GETCHECK, 0, 0) == BST_CHECKED)
                saveSticky = TRUE;
            else
                saveSticky = FALSE;
            if(SendDlgItemMessage(hDlg, IDC_MENUSTICKY, BM_GETCHECK, 0, 0) == BST_CHECKED)
                stickyMenu = 1;
            else 
                stickyMenu = 0;
            if(SendDlgItemMessage(hDlg, IDC_MENUACCESS, BM_GETCHECK, 0, 0) == BST_CHECKED)
                directMenu = 1;
            else
                directMenu = 0;
            if(SendDlgItemMessage(hDlg, IDC_MENUASSIGN, BM_GETCHECK, 0, 0) == BST_CHECKED)
                assignMenu = 1;
            else
                assignMenu = 0;
            if(SendDlgItemMessage(hDlg, IDC_USEASSIGN, BM_GETCHECK, 0, 0) == BST_CHECKED)
                useDeskAssignment = TRUE;
            else
                useDeskAssignment = FALSE;
            if(SendDlgItemMessage(hDlg, IDC_FIRSTONLY, BM_GETCHECK, 0, 0) == BST_CHECKED)
                assignOnlyFirst = TRUE;
            else
                assignOnlyFirst = FALSE;
            assignImmediately = (SendDlgItemMessage(hDlg, IDC_ASSIGNWINNOW, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            if(SendDlgItemMessage(hDlg, IDC_SAVEEXITSTATE, BM_GETCHECK, 0, 0) == BST_CHECKED)
                saveLayoutOnExit = TRUE;
            else
                saveLayoutOnExit= FALSE;
            if(SendDlgItemMessage(hDlg, IDC_HOTMENUEN, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                hotkeyMenuEn = TRUE;
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOTMENU, HKM_GETHOTKEY, 0, 0);
                hotkeyMenu = LOBYTE(wRawHotKey);
                hotkeyMenuMod = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOTMENUW, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    hotkeyMenuWin = MOD_WIN;
                else
                    hotkeyMenuWin = FALSE;
            }
            else
            {
                hotkeyMenuEn = FALSE;
            }
            if(SendDlgItemMessage(hDlg, IDC_HOTSTICKYEN, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                hotkeyStickyEn = TRUE;
                SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
                // Sticky hot key control
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOTSTICKY, HKM_GETHOTKEY, 0, 0);
                hotkeySticky = LOBYTE(wRawHotKey);
                hotkeyStickyMod = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOTSTICKYW, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    hotkeyStickyWin = MOD_WIN;
                else
                    hotkeyStickyWin = FALSE;
            }
            else
            {
                hotkeyStickyEn = FALSE;
            }
            vwSetupApply(hDlg,0x04) ;
            break;
        case PSN_KILLACTIVE:
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            return 1;
        case PSN_RESET:
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            break;
        case PSN_HELP:
            showHelp(hDlg);
            break;
        }
        
    case WM_COMMAND:
        if( LOWORD(wParam) == IDC_STICKYSAVE || LOWORD(wParam) == IDC_DISPLAYICON ||
            LOWORD(wParam) == IDC_MENUSTICKY || LOWORD(wParam) == IDC_MENUACCESS ||
            LOWORD(wParam) == IDC_MENUASSIGN || LOWORD(wParam) == IDC_USEASSIGN ||
            LOWORD(wParam) == IDC_FIRSTONLY  || LOWORD(wParam) == IDC_ASSIGNWINNOW ||
            LOWORD(wParam) == IDC_SAVEEXITSTATE ||
            LOWORD(wParam) == IDC_HOTMENUEN  || LOWORD(wParam) == IDC_HOTMENUW || 
            LOWORD(wParam) == IDC_HOTMENU    || LOWORD(wParam) == IDC_HOTSTICKYEN  ||
            LOWORD(wParam) == IDC_HOTSTICKY  || LOWORD(wParam) == IDC_HOTSTICKYW )
        {
            pageChangeMask |= 0x04 ;
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
        }
        if (LOWORD(wParam) == IDC_SAVENOW) {
            saveAssignedList(nWin,winList) ;
        }
        else if(LOWORD(wParam) == IDC_SAVESTICKY) {
            saveStickyWindows(nWin, winList);
        }
        if (LOWORD(wParam) == IDC_SLIDERX)
            spinPressed = 1;
        else if (LOWORD(wParam) == IDC_SLIDERY)
            spinPressed = 2;
        
        if (LOWORD(wParam) == IDC_DESKX && spinPressed == 1)
        {
            pageChangeMask |= 0x04 ;
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            tmpDesksX = SendMessage(GetDlgItem(hDlg, IDC_SLIDERX), UDM_GETPOS, 0, 0);
            while ((tmpDesksX * tmpDesksY) > 9) {
                tmpDesksY--;
                SendMessage(GetDlgItem(hDlg, IDC_SLIDERY), UDM_SETPOS, 0L, MAKELONG( tmpDesksY, 0));
            }
        }
        else if(LOWORD(wParam) == IDC_DESKY && spinPressed == 2)
        {
            pageChangeMask |= 0x04 ;
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            tmpDesksY = SendMessage(GetDlgItem(hDlg, IDC_SLIDERY), UDM_GETPOS, 0, 0);
            while ((tmpDesksY * tmpDesksX) > 9) {
                tmpDesksX--;
                SendMessage(GetDlgItem(hDlg, IDC_SLIDERX), UDM_SETPOS, 0L, MAKELONG( tmpDesksX, 0));
            }
        }
        break;
    }
    return (FALSE);
}

/*************************************************
 * The "Expert" tab callback
 */
BOOL APIENTRY expert(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    switch (message) {
    case WM_INITDIALOG:
        if(minSwitch)
            SendDlgItemMessage(hDlg, IDC_MINIMIZED, BM_SETCHECK, 1,0);
        if(releaseFocus)
            SendDlgItemMessage(hDlg, IDC_FOCUS, BM_SETCHECK, 1,0);
        if(refreshOnWarp)
            SendDlgItemMessage(hDlg, IDC_REFRESH, BM_SETCHECK, 1,0);
        if(preserveZOrder)
            SendDlgItemMessage(hDlg, IDC_PRESZORDER, BM_SETCHECK, 1,0);
        if(deskWrap)
            SendDlgItemMessage(hDlg, IDC_DESKCYCLE, BM_SETCHECK, 1,0);
        if(invertY)
            SendDlgItemMessage(hDlg, IDC_INVERTY, BM_SETCHECK, 1,0);
        if(!displayTaskbarIcon)
            SendDlgItemMessage(hDlg, IDC_DISPLAYICON, BM_SETCHECK, 1,0);
        if(!noTaskbarCheck)
            SendDlgItemMessage(hDlg, IDC_TASKBARDETECT, BM_SETCHECK, 1,0);
        if(trickyWindows)
            SendDlgItemMessage(hDlg, IDC_TRICKYSUPPORT, BM_SETCHECK, 1,0);
        if(!taskbarOffset)
            SendDlgItemMessage(hDlg, IDC_XPSTYLETASKBAR, BM_SETCHECK, 1,0);
        if(permanentSticky)
            SendDlgItemMessage(hDlg, IDC_PERMSTICKY, BM_SETCHECK, 1,0);
        if(hiddenWindowRaise)
            SendDlgItemMessage(hDlg, IDC_POPUPRHWIN, BM_SETCHECK, 1,0);
        if(hiddenWindowPopup)
            SendDlgItemMessage(hDlg, IDC_HWINPOPUP, BM_SETCHECK, 1,0);
        SendDlgItemMessage(hDlg, IDC_HOTDISMISSEN, BM_SETCHECK,(hotkeyDismissEn != 0),0);
        SendDlgItemMessage(hDlg, IDC_HOTDISMISS, HKM_SETHOTKEY, MAKEWORD(hotkeyDismiss, hotkeyDismissMod), 0);
        SendDlgItemMessage(hDlg, IDC_HOTDISMISSW, BM_SETCHECK,(hotkeyDismissWin != 0),0);
        return TRUE;
        
    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code) {
        case PSN_SETACTIVE:
            // Initialize the controls.
            break;
        case PSN_APPLY:
            minSwitch = (SendDlgItemMessage(hDlg, IDC_MINIMIZED, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            releaseFocus = (SendDlgItemMessage(hDlg, IDC_FOCUS, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            refreshOnWarp = (SendDlgItemMessage(hDlg, IDC_REFRESH, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            preserveZOrder = (SendDlgItemMessage(hDlg, IDC_PRESZORDER, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            deskWrap = (SendDlgItemMessage(hDlg, IDC_DESKCYCLE, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            invertY = (SendDlgItemMessage(hDlg, IDC_INVERTY, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            noTaskbarCheck = (SendDlgItemMessage(hDlg, IDC_TASKBARDETECT, BM_GETCHECK, 0, 0) != BST_CHECKED) ;
            trickyWindows = (SendDlgItemMessage(hDlg, IDC_TRICKYSUPPORT, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            if(SendDlgItemMessage(hDlg, IDC_XPSTYLETASKBAR, BM_GETCHECK, 0, 0) == BST_CHECKED)
                taskbarOffset = 0;
            else
                taskbarOffset = 3;
            permanentSticky = (SendDlgItemMessage(hDlg, IDC_PERMSTICKY, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            hiddenWindowRaise = (SendDlgItemMessage(hDlg, IDC_POPUPRHWIN, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            hiddenWindowPopup = (SendDlgItemMessage(hDlg, IDC_HWINPOPUP, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            if(SendDlgItemMessage(hDlg, IDC_DISPLAYICON, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                displayTaskbarIcon = FALSE;
                PostMessage(hWnd, VW_DELICON, 0, 0);
            }
            else
            {
                displayTaskbarIcon = TRUE;
                PostMessage(hWnd, VW_SHOWICON, 0, 0);
            }
            if(SendDlgItemMessage(hDlg, IDC_HOTDISMISSEN, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                WORD wRawHotKey;
                hotkeyDismissEn = TRUE;
                SetWindowLong(hDlg, DWL_MSGRESULT, TRUE);
                wRawHotKey = (WORD)SendDlgItemMessage(hDlg, IDC_HOTDISMISS, HKM_GETHOTKEY, 0, 0);
                hotkeyDismiss = LOBYTE(wRawHotKey);
                hotkeyDismissMod = HIBYTE(wRawHotKey);
                if(SendDlgItemMessage(hDlg, IDC_HOTDISMISSW, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    hotkeyDismissWin = MOD_WIN;
                else
                    hotkeyDismissWin = FALSE;
            }
            else
            {
                hotkeyDismissEn = FALSE;
            }
            vwSetupApply(hDlg,0x08) ;
            break;
        case PSN_KILLACTIVE:
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            return 1;
        case PSN_RESET:
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            break;
        case PSN_HELP:
            showHelp(hDlg);
            break;
        }
        
    case WM_COMMAND:
        if(LOWORD(wParam) == IDC_FOCUS      || LOWORD(wParam) == IDC_TRICKYSUPPORT ||
           LOWORD(wParam) == IDC_MINIMIZED  || LOWORD(wParam) == IDC_REFRESH ||
           LOWORD(wParam) == IDC_DESKCYCLE  || LOWORD(wParam) == IDC_INVERTY ||
           LOWORD(wParam) == IDC_PERMSTICKY || LOWORD(wParam) == IDC_DISPLAYICON ||
           LOWORD(wParam) == IDC_PRESZORDER || LOWORD(wParam) == IDC_TASKBARDETECT ||
           LOWORD(wParam) == IDC_HWINPOPUP  || LOWORD(wParam) == IDC_XPSTYLETASKBAR ||
           LOWORD(wParam) == IDC_POPUPRHWIN || LOWORD(wParam) == IDC_HOTDISMISSEN  ||
           LOWORD(wParam) == IDC_HOTDISMISS || LOWORD(wParam) == IDC_HOTDISMISSW )
        {
            pageChangeMask |= 0x08 ;
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
        }
        break;
    }
    return (FALSE);
}

/*************************************************
 * The "About" tab callback
 */
BOOL APIENTRY about(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    char license[] = "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\r\n \r\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. \r\n \r\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.";
    
    switch (message) {
    case WM_INITDIALOG:
        SetDlgItemText(hDlg, IDC_LICENSE, license);
        return TRUE;
        
    case WM_CTLCOLORSTATIC:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_MAILTO) ||
           (HWND)lParam == GetDlgItem(hDlg, IDC_HTTP)) {
            SetBkMode((HDC)wParam, TRANSPARENT); // Don't overwrite background
            SetTextColor((HDC)wParam, RGB(0, 0, 255)); // Blue 
            return (BOOL) GetStockObject(HOLLOW_BRUSH);
        } else {
            return FALSE;
        }
        
    case WM_COMMAND:
        if(LOWORD(wParam) == IDC_MAILTO) {
            HINSTANCE h = ShellExecute(NULL, "open", "mailto:" vwVIRTUAWIN_EMAIL, 
                                       NULL, NULL, SW_SHOWNORMAL);
            if ((UINT)h < 33) {
                MessageBox(hDlg, "Error executing mail program.", "VirtuawWin", MB_ICONWARNING);
            }  
        }
        else if(LOWORD(wParam) == IDC_HTTP) {
            HINSTANCE h = ShellExecute(NULL, "open", "http://virtuawin.sourceforge.net", 
                                       NULL, NULL, SW_SHOWNORMAL);
            if ((UINT)h < 33) {
                MessageBox(hDlg, "Error open web link.", "VirtuawWin", MB_ICONWARNING);
            }
        }
        return TRUE;
    }
    return FALSE;
}

/*************************************************
 * Initialize callback function for the property sheet
 * Used for removing the "?" in the title bar
 */
int CALLBACK propCallBack( HWND hwndDlg, UINT uMsg, LPARAM lParam )
{
    DLGTEMPLATE* ptr;
    
    switch(uMsg)
    {
    case PSCB_PRECREATE:
        
        // Removes the question mark button in the title bar
        ptr = (DLGTEMPLATE*)lParam;
        ptr->style = ptr->style ^ (DS_CONTEXTHELP | DS_CENTER); 
        
        break;  
    }
    return 0;
}

/*************************************************
 * Creates the property sheet that holds the setup dialog
 */
int createPropertySheet(HINSTANCE theHinst, HWND theHwndOwner)
{
    PROPSHEETPAGE psp[6]; // How many sheets
    PROPSHEETHEADER psh;
    
    int xIcon = GetSystemMetrics(SM_CXSMICON);
    int yIcon = GetSystemMetrics(SM_CYSMICON);
    
    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE|PSP_HASHELP;
    psp[0].hInstance = theHinst;
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_KEYS);
    psp[0].pszIcon = NULL;
    psp[0].pfnDlgProc = keys;
    psp[0].pszTitle = "Keys";
    psp[0].lParam = 0;
    
    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_USETITLE|PSP_HASHELP;
    psp[1].hInstance = theHinst;
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_MOUSE);
    psp[1].pszIcon = NULL;
    psp[1].pfnDlgProc = mouse;
    psp[1].pszTitle = "Mouse";
    psp[1].lParam = 0;
    
    psp[2].dwSize = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags = PSP_USETITLE|PSP_HASHELP;
    psp[2].hInstance = theHinst;
    psp[2].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_MODULES);
    psp[2].pszIcon = NULL;
    psp[2].pfnDlgProc = modules;
    psp[2].pszTitle = "Modules";
    psp[2].lParam = 0;
    
    psp[3].dwSize = sizeof(PROPSHEETPAGE);
    psp[3].dwFlags = PSP_USETITLE|PSP_HASHELP;
    psp[3].hInstance = theHinst;
    psp[3].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_MISC);
    psp[3].pszIcon = NULL;
    psp[3].pfnDlgProc = misc;
    psp[3].pszTitle = "Misc.";
    psp[3].lParam = 0;
    
    psp[4].dwSize = sizeof(PROPSHEETPAGE);
    psp[4].dwFlags = PSP_USETITLE|PSP_HASHELP;
    psp[4].hInstance = theHinst;
    psp[4].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_EXPERT);
    psp[4].pszIcon = NULL;
    psp[4].pfnDlgProc = expert;
    psp[4].pszTitle = "Expert";
    psp[4].lParam = 0;
    
    psp[5].dwSize = sizeof(PROPSHEETPAGE);
    psp[5].dwFlags = PSP_USETITLE|PSP_HASHELP;
    psp[5].hInstance = theHinst;
    psp[5].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_ABOUT);
    psp[5].pszIcon = NULL;
    psp[5].pfnDlgProc = about;
    psp[5].pszTitle = "About";
    psp[5].lParam = 0;
    
    psh.dwSize = sizeof(PROPSHEETHEADER);
    // the use of Apply button is dangerous as it does not save the config
    // properly, this is only done when Okay is pressed, if the user selects
    // Apply and then Cancel their settings are lost which is bad behaviour
    // so remove the Apply button 
    psh.dwFlags = PSH_USECALLBACK | PSH_PROPSHEETPAGE | PSH_USEHICON ;
    psh.hwndParent = theHwndOwner;
    psh.hInstance = theHinst;
    psh.pszIcon = NULL;
    psh.pszCaption = (LPSTR) "VirtuaWin - Properties";
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;
    psh.nStartPage = 0;
    psh.hIcon = (HICON) LoadImage(theHinst, MAKEINTRESOURCE(IDI_VIRTWIN), IMAGE_ICON, xIcon, yIcon, 0);
    psh.pfnCallback = (PFNPROPSHEETCALLBACK)propCallBack;
    
    return (PropertySheet(&psh));
}

/*
 * $Log$
 * Revision 1.26  2005/08/10 15:10:45  rexkerr	
 * Made the setup dialog center itself on the primary monitor so that it worked on dual monitor systems.  Prior to this change it was centering itself on the centerline between the two monitors if the secondary monitor was to the left or under the primary monitor, and half way off of the edge of the displays if the secondary monitor was to the right or above the primary monitor.	
 *
 * Revision 1.25  2005/07/21 19:34:47  jopi
 * Removed popup message when configuring only one desktop
 *
 * Revision 1.24  2004/04/10 10:20:01  jopi
 * Updated to compile with gcc/mingw
 *
 * Revision 1.23  2004/02/28 18:54:01  jopi
 * SF904069 Added possibility to choose if sticky should be permanent for all instances of the same classname.
 *
 * Revision 1.22  2004/01/10 11:15:52  jopi
 * Updated copyright for 2004
 *
 * Revision 1.21  2003/09/25 16:41:03  jopi
 * SF770859 Window menu heading will not be displayed if only one meny is used
 *
 * Revision 1.20  2003/07/08 21:24:24  jopi
 * Corrected the web link
 *
 * Revision 1.19  2003/07/08 21:18:11  jopi
 * Changed the module enable message
 *
 * Revision 1.18  2003/06/24 19:49:08  jopi
 * SF693876 Fixed option to handle XP skinned style taskbars
 *
 * Revision 1.17  2003/04/23 19:36:01  jopi
 * SF723880, Changed the mouse control checkboxes to radiobuttons.
 *
 * Revision 1.16  2003/04/09 17:46:20  jopi
 * Small error made a checkbox work incorrectly, introduced with the "expert" tab.
 *
 * Revision 1.15  2003/01/27 20:22:58  jopi
 * Updated copyright header for 2003
 *
 * Revision 1.14  2002/12/23 15:42:27  jopi
 * Added config options to disable taskbar detection and the alternative hiding technique.
 *
 * Revision 1.13  2002/12/23 14:16:47  jopi
 * Added a new setup tab, "expert" and moved some settings from misc.
 *
 * Revision 1.12  2002/06/01 21:15:22  Johan Piculell
 * Multiple fixes by Christian Storm.
 *
 * Revision 1.11  2002/06/01 19:33:34  Johan Piculell
 * *** empty log message ***
 *
 * Revision 1.10  2002/02/14 21:23:40  Johan Piculell
 * Updated copyright header
 *
 * Revision 1.9  2001/12/24 10:11:01  Johan Piculell
 * Added doubleclick support in the module listbox for bringing up the config window.
 *
 * Revision 1.8  2001/11/12 21:39:14  Johan Piculell
 * Added functionality for disabling the systray icon
 *
 * Revision 1.7  2001/02/10 11:11:53  Administrator
 * Removed the context help icon since there is no functionality for this
 *
 * Revision 1.6  2001/02/05 21:13:08  Administrator
 * Updated copyright header
 *
 * Revision 1.5  2001/01/29 21:09:34  Administrator
 * Changed web adress
 *
 * Revision 1.4  2001/01/28 16:26:56  Administrator
 * Configuration behaviour change. It is now possible to test all settings by using apply and all changes will be rollbacked if cancel is pressed
 *
 * Revision 1.3  2001/01/12 18:14:34  Administrator
 * Modules will now get a notification when desktop layout has changed since we might have a new current desktop number after a change. Also fixed so that config update notification is sent upon apply and only when something has changed upon hitting ok. Config file will also be written upon every apply and not if cancel is selected
 *
 * Revision 1.2  2000/08/18 23:43:07  Administrator
 *  Minor modifications by Matti Jagula <matti@proekspert.ee> List of modifications follows: Added window title sorting in popup menus (Assign, Direct, Sticky) Added some controls to Setup Misc tab and support for calling the popup menus from keyboard.
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  Administrator
 * Added first time
 *
 */
