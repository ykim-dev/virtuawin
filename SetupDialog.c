//
//  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
//  SetupDialog.c - Setup Dialog routines.
// 
//  Copyright (c) 1999-2005 Johan Piculell
//  Copyright (c) 2006 VirtuaWin (VirtuaWin@home.se)
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

// Must compile with define of _WIN32_IE=0x0200 otherwise the setup dialog
// will not open on NT/95 without a patch (known MS issue) 
#define _WIN32_IE 0x0200

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
#include <shellapi.h>
#include <prsht.h>
#include <commctrl.h>

#define vwPROPSHEET_PAGE_COUNT 6

static int pageChangeMask=0 ;
static int pageApplyMask=0 ;
static HWND setupKeysHWnd=NULL;

static void vwSetupApply(HWND hDlg, int curPageMask)
{
    pageApplyMask |= curPageMask ;
    if(pageApplyMask == pageApplyMask)
    {
        // All pages have now got any changes from the GUI, save them and apply
        writeConfig();
        getWorkArea();
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

void initDeskHotkey(void)
{
    TCHAR buff[20] ;
    _stprintf(buff,_T("Desk %d"),currentDesk) ;
    SetDlgItemText(setupKeysHWnd, IDC_HOTDESKBTN, buff) ;
    SendDlgItemMessage(setupKeysHWnd, IDC_HOTDESK, HKM_SETHOTKEY, MAKEWORD(deskHotkey[currentDesk], deskHotkeyMod[currentDesk]), 0);
    SendDlgItemMessage(setupKeysHWnd, IDC_HOTDESKW, BM_SETCHECK, (deskHotkeyWin[currentDesk] != 0),0);
}

void storeDeskHotkey(void)
{
    WORD wRawHotKey;
    hotKeyEnable = (SendDlgItemMessage(setupKeysHWnd, IDC_HOTKEYS, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
    wRawHotKey = (WORD)SendDlgItemMessage(setupKeysHWnd, IDC_HOTDESK, HKM_GETHOTKEY, 0, 0);
    deskHotkey[currentDesk] = LOBYTE(wRawHotKey);
    deskHotkeyMod[currentDesk] = HIBYTE(wRawHotKey);
    if(SendDlgItemMessage(setupKeysHWnd, IDC_HOTDESKW, BM_GETCHECK, 0, 0) == BST_CHECKED)
        deskHotkeyWin[currentDesk] = MOD_WIN;
    else
        deskHotkeyWin[currentDesk] = FALSE;
}

/*************************************************
 * The "Key" tab callback
 * This is the firts callback to be called when the property sheet is created
 */
BOOL APIENTRY setupGeneral(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    static int tmpDesksY;
    static int tmpDesksX;
    RECT config_dlg_rect;
    WORD wRawHotKey;
    WORD wPar;
    int maxDesk ;
    
    switch (message) {
    case WM_INITDIALOG:
        {
            setupHWnd = GetParent(hDlg) ;
            setupKeysHWnd = hDlg ;
            /* the setup dialog will be automatically positioned top left of the primary, move this 40 pixels in */
            GetWindowRect(setupHWnd, &config_dlg_rect);
            config_dlg_rect.left += 40 ;
            config_dlg_rect.top  += 40 ;
            SetWindowPos(setupHWnd, 0, config_dlg_rect.left, config_dlg_rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
            
            SetDlgItemInt(hDlg, IDC_DESKY, nDesksY, FALSE);
            SetDlgItemInt(hDlg, IDC_DESKX, nDesksX, FALSE);
            tmpDesksY = nDesksY;
            tmpDesksX = nDesksX;
            // Set the spin buddy controls
            SendMessage(GetDlgItem(hDlg, IDC_SLIDERX), UDM_SETBUDDY, (LONG) GetDlgItem(hDlg, IDC_DESKX), 0L );
            SendMessage(GetDlgItem(hDlg, IDC_SLIDERY), UDM_SETBUDDY, (LONG) GetDlgItem(hDlg, IDC_DESKY), 0L );
            // Set spin ranges
            SendMessage(GetDlgItem(hDlg, IDC_SLIDERX), UDM_SETRANGE, 0L, MAKELONG(9, 1));
            SendMessage(GetDlgItem(hDlg, IDC_SLIDERY), UDM_SETRANGE, 0L, MAKELONG(9, 1));
            // Set init values
            SendMessage(GetDlgItem(hDlg, IDC_SLIDERX), UDM_SETPOS, 0L, MAKELONG( nDesksX, 0));
            SendMessage(GetDlgItem(hDlg, IDC_SLIDERY), UDM_SETPOS, 0L, MAKELONG( nDesksY, 0));
        
            /* Control keys */
            if(keyEnable)
                SendDlgItemMessage(hDlg, IDC_KEYS, BM_SETCHECK, 1,0);
            if(modAlt)
                SendDlgItemMessage(hDlg, IDC_ALT, BM_SETCHECK, 1,0);
            if(modShift)
                SendDlgItemMessage(hDlg, IDC_SHIFT, BM_SETCHECK, 1,0);
            if(modCtrl)
                SendDlgItemMessage(hDlg, IDC_CTRL, BM_SETCHECK, 1,0);
            if(modWin)
                SendDlgItemMessage(hDlg, IDC_WIN, BM_SETCHECK, 1,0);
            
            /* Cycling key */
            if(deskWrap)
                SendDlgItemMessage(hDlg, IDC_DESKCYCLE, BM_SETCHECK, 1, 0);
            if(cyclingKeysEnabled)
                SendDlgItemMessage(hDlg, IDC_CYCLINGKEYS, BM_SETCHECK, 1, 0);
            SendDlgItemMessage(hDlg, IDC_HOTCYCLEUP, HKM_SETHOTKEY, 
                               MAKEWORD(hotCycleUp, hotCycleUpMod), 0);
            if(hotCycleUpWin)
                SendDlgItemMessage(hDlg, IDC_HOTCYCLEUPW, BM_SETCHECK, 1, 0);
            SendDlgItemMessage(hDlg, IDC_HOTCYCLEDOWN, HKM_SETHOTKEY, 
                               MAKEWORD(hotCycleDown, hotCycleDownMod), 0);
            if(hotCycleDownWin)
                SendDlgItemMessage(hDlg, IDC_HOTCYCLEDOWNW, BM_SETCHECK, 1, 0);
            
            /* Hot keys */
            if(hotKeyEnable)
                SendDlgItemMessage(hDlg, IDC_HOTKEYS, BM_SETCHECK, 1,0);
            initDeskHotkey() ;
            return (TRUE);
        }
        
    case WM_NOTIFY:
        
        switch (((NMHDR FAR *) lParam)->code) {
        case PSN_SETACTIVE: // Getting focus
            // Initialize the controls.
            break;
        case PSN_APPLY: // Apply, OK
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
            else
                keyEnable = FALSE;
            
            // Desktop Cycling hot keys
            deskWrap = (SendDlgItemMessage(hDlg, IDC_DESKCYCLE, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            if(SendDlgItemMessage(hDlg, IDC_CYCLINGKEYS, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
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
            
            // Direct Desktop Access Hot keys
            storeDeskHotkey() ;
            
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
            showHelp(hDlg,6001);
            break;
        }
        break;
        
    case WM_COMMAND:
        wPar = LOWORD(wParam);
        if(wPar == IDC_DESKX)
        {
            pageChangeMask |= 0x01 ;
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            tmpDesksX = SendMessage(GetDlgItem(hDlg, IDC_SLIDERX), UDM_GETPOS, 0, 0);
            while ((tmpDesksX * tmpDesksY) > 9) {
                tmpDesksY--;
                SendMessage(GetDlgItem(hDlg, IDC_SLIDERY), UDM_SETPOS, 0L, MAKELONG( tmpDesksY, 0));
            }
        }
        else if(wPar == IDC_DESKY)
        {
            pageChangeMask |= 0x01 ;
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
            tmpDesksY = SendMessage(GetDlgItem(hDlg, IDC_SLIDERY), UDM_GETPOS, 0, 0);
            while ((tmpDesksY * tmpDesksX) > 9) {
                tmpDesksX--;
                SendMessage(GetDlgItem(hDlg, IDC_SLIDERX), UDM_SETPOS, 0L, MAKELONG( tmpDesksX, 0));
            }
        }
        else if(wPar == IDC_KEYS        || wPar == IDC_ALT          || wPar == IDC_SHIFT  ||
                wPar == IDC_CTRL        || wPar == IDC_WIN          || 
                wPar == IDC_DESKCYCLE   ||  wPar == IDC_CYCLINGKEYS || wPar == IDC_HOTCYCLEUP ||
                wPar == IDC_HOTCYCLEUPW || wPar == IDC_HOTCYCLEDOWN || wPar == IDC_HOTCYCLEDOWNW ||
                wPar == IDC_HOTKEYS     || wPar == IDC_HOTDESK      || wPar == IDC_HOTDESKW )
        {
            pageChangeMask |= 0x01 ;
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L); // Enable apply button
        }
        else if(wPar == IDC_HOTDESKBTN)
        {
            if((maxDesk = currentDesk + 1) > (tmpDesksX * tmpDesksY))
                maxDesk = 1 ;
            gotoDesk(maxDesk,TRUE);
        }
        break;
    }
    return (FALSE);
}

/*************************************************
 * The "Mouse" tab callback
 */
BOOL APIENTRY setupMouse(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    TCHAR buff[5];
    
    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, IDC_TIME, mouseDelay * 25, FALSE);
        SendDlgItemMessage(hDlg, IDC_SLIDER, TBM_SETRANGE, TRUE, MAKELONG(1, 80));
        SendDlgItemMessage(hDlg, IDC_SLIDER, TBM_SETPOS, TRUE, mouseDelay >> 1);
        SetDlgItemInt(hDlg, IDC_JUMP, warpLength, TRUE);
        
        if(mouseEnable) 
            SendDlgItemMessage(hDlg, IDC_ENABLEMOUSE, BM_SETCHECK, 1,0);
        if(useMouseKey)
            SendDlgItemMessage(hDlg, IDC_KEYCONTROL, BM_SETCHECK, 1,0);
        if(mouseModAlt)
            SendDlgItemMessage(hDlg, IDC_MALT, BM_SETCHECK, 1,0);
        if(mouseModShift)
            SendDlgItemMessage(hDlg, IDC_MSHIFT, BM_SETCHECK, 1,0);
        if(mouseModCtrl)
            SendDlgItemMessage(hDlg, IDC_MCTRL, BM_SETCHECK, 1,0);
        if(knockMode & 1) 
            SendDlgItemMessage(hDlg, IDC_KNOCKMODE1, BM_SETCHECK, 1,0);
        if(knockMode & 2) 
            SendDlgItemMessage(hDlg, IDC_KNOCKMODE2, BM_SETCHECK, 1,0);
        if(!noMouseWrap)
            SendDlgItemMessage(hDlg, IDC_MOUSEWRAP, BM_SETCHECK, 1,0);
        return TRUE;
        
    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {
        case PSN_SETACTIVE:
            // Initialize the controls. Only if we want to reinitialize on tab change.
            break;
            
        case PSN_APPLY:
            GetDlgItemText(hDlg, IDC_JUMP, buff, 4);
            warpLength = _ttoi(buff);
            mouseDelay = (SendDlgItemMessage(hDlg, IDC_SLIDER, TBM_GETPOS, 0, 0)) << 1 ;
            mouseEnable = (SendDlgItemMessage(hDlg, IDC_ENABLEMOUSE, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            noMouseWrap = (SendDlgItemMessage(hDlg, IDC_MOUSEWRAP, BM_GETCHECK, 0, 0) != BST_CHECKED) ;
            knockMode = (SendDlgItemMessage(hDlg, IDC_KNOCKMODE1, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            if(knockMode && (warpLength < 24))
            {
                knockMode = 0 ;
                SendDlgItemMessage(hDlg, IDC_KNOCKMODE1, BM_SETCHECK, 0,0);
                MessageBox(hDlg,_T("Mouse jump length is too small to support knocking, must be 24 or greater."),vwVIRTUAWIN_NAME _T(" Error"), MB_ICONERROR); 
            }
            if(SendDlgItemMessage(hDlg, IDC_KNOCKMODE2, BM_GETCHECK, 0, 0) == BST_CHECKED)
                knockMode |= 2 ;
                
            if(SendDlgItemMessage(hDlg, IDC_KEYCONTROL, BM_GETCHECK, 0, 0) == BST_CHECKED)
            {
                useMouseKey = TRUE;
                mouseModShift = FALSE;
                mouseModAlt = FALSE;
                mouseModCtrl = FALSE;
                if(SendDlgItemMessage(hDlg, IDC_MALT, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    mouseModAlt = TRUE;
                if(SendDlgItemMessage(hDlg, IDC_MSHIFT, BM_GETCHECK, 0, 0) == BST_CHECKED)
                    mouseModShift = TRUE;
                if(SendDlgItemMessage(hDlg, IDC_MCTRL, BM_GETCHECK, 0, 0) == BST_CHECKED)
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
            showHelp(hDlg,6002);
            break;
        }
        break;
        
    case WM_COMMAND:
        if(LOWORD(wParam) == IDC_JUMP     || LOWORD(wParam) == IDC_MOUSEWRAP ||
           LOWORD(wParam) == IDC_MALT     || LOWORD(wParam) == IDC_KEYCONTROL || 
           LOWORD(wParam) == IDC_MCTRL    || LOWORD(wParam) == IDC_ENABLEMOUSE || 
           LOWORD(wParam) == IDC_MSHIFT   || LOWORD(wParam) == IDC_KNOCKMODE1 || 
           LOWORD(wParam) == IDC_KNOCKMODE2)
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
static void setupModulesList(HWND hDlg)
{
    int index;
    TCHAR tmpName[128];
    
    SendDlgItemMessage(hDlg, IDC_MODLIST, LB_RESETCONTENT, 0, 0);
    for(index = 0; index < nOfModules; index++)
    {
        _tcsncpy(tmpName,moduleList[index].description,100) ;
        tmpName[100] = '\0' ;
        if(moduleList[index].Disabled)
            _tcscat(tmpName,_T(" (disabled)")) ;
        SendDlgItemMessage(hDlg, IDC_MODLIST, LB_ADDSTRING, 0, (LONG)tmpName);
    }
}

BOOL APIENTRY setupModules(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int index;
    
    switch (message)
    {
    case WM_INITDIALOG:
        setupModulesList(hDlg) ;
        return TRUE;
    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {
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
            showHelp(hDlg,6003);
            break;
        }
        break;
        
    case WM_COMMAND:
        if(LOWORD((wParam) == IDC_MODCONFIG || HIWORD(wParam) == LBN_DBLCLK ))
        {   // Show config
            int curSel = SendDlgItemMessage(hDlg, IDC_MODLIST, LB_GETCURSEL, 0, 0);
            if(curSel != LB_ERR)
                PostMessage(moduleList[curSel].Handle, MOD_SETUP, 0, 0);
        }
        else if(LOWORD((wParam) == IDC_MODRELOAD))
        {   // Reload
            saveDisabledList(nOfModules, moduleList);
            unloadModules();
            for(index = 0; index < MAXMODULES; index++)
            {
                moduleList[index].Handle = NULL;
                moduleList[index].description[0] = '\0';
            }
            nOfModules = 0;
            /* sleep for a second to allow the modules to exit cleanly */
            Sleep(1000) ;
            curDisabledMod = loadDisabledModules(disabledModules);
            loadModules();
            setupModulesList(hDlg) ;
        }
        else if(LOWORD((wParam) == IDC_MODDISABLE))
        {   // Enable/Disable
            int curSel = SendDlgItemMessage(hDlg, IDC_MODLIST, LB_GETCURSEL, 0, 0);
            if(curSel != LB_ERR)
            {
                if(moduleList[curSel].Disabled == FALSE)
                {   // let's disable
                    moduleList[curSel].Disabled = TRUE;
                    PostMessage(moduleList[curSel].Handle, MOD_QUIT, 0, 0);
                    setupModulesList(hDlg) ;
                }
                else
                {   // let's enable
                    MessageBox(hDlg,_T("Press reload or restart VirtuaWin to enable the module"),
                               vwVIRTUAWIN_NAME _T(" Note"), MB_ICONINFORMATION);
                    moduleList[curSel].Disabled = FALSE;
                    setupModulesList(hDlg) ;
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
BOOL APIENTRY setupMisc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    WORD wRawHotKey;
    
    switch (message) {
    case WM_INITDIALOG:
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
        if(hotkeyMenuEn)
            SendDlgItemMessage(hDlg, IDC_HOTMENUEN, BM_SETCHECK, 1, 0);
        SendDlgItemMessage(hDlg, IDC_HOTMENU, HKM_SETHOTKEY, MAKEWORD(hotkeyMenu, hotkeyMenuMod), 0);
        if(hotkeyMenuWin)
            SendDlgItemMessage(hDlg, IDC_HOTMENUW, BM_SETCHECK, 1, 0);
        if(hotkeyStickyEn)
            SendDlgItemMessage(hDlg, IDC_HOTSTICKYEN, BM_SETCHECK, 1, 0);
        SendDlgItemMessage(hDlg, IDC_HOTSTICKY, HKM_SETHOTKEY, MAKEWORD(hotkeySticky, hotkeyStickyMod), 0);
        if(hotkeyStickyWin)
            SendDlgItemMessage(hDlg, IDC_HOTSTICKYW, BM_SETCHECK, 1, 0);
        if(hotkeyDismissEn)
            SendDlgItemMessage(hDlg, IDC_HOTDISMISSEN, BM_SETCHECK, 1, 0);
        SendDlgItemMessage(hDlg, IDC_HOTDISMISS, HKM_SETHOTKEY, MAKEWORD(hotkeyDismiss, hotkeyDismissMod), 0);
        if(hotkeyDismissWin)
            SendDlgItemMessage(hDlg, IDC_HOTDISMISSW, BM_SETCHECK, 1, 0);
        return TRUE;
        
    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code) {
        case PSN_SETACTIVE:
            // Initialize the controls.
            break;
        case PSN_APPLY:
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
                hotkeyMenuEn = FALSE;
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
                hotkeyStickyEn = FALSE;
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
                hotkeyDismissEn = FALSE;
            vwSetupApply(hDlg,0x04) ;
            break;
        case PSN_KILLACTIVE:
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            return 1;
        case PSN_RESET:
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            break;
        case PSN_HELP:
            showHelp(hDlg,6004);
            break;
        }
        break;
        
    case WM_COMMAND:
        if( LOWORD(wParam) == IDC_STICKYSAVE    || LOWORD(wParam) == IDC_DISPLAYICON ||
            LOWORD(wParam) == IDC_MENUSTICKY    || LOWORD(wParam) == IDC_MENUACCESS ||
            LOWORD(wParam) == IDC_MENUASSIGN    || LOWORD(wParam) == IDC_USEASSIGN ||
            LOWORD(wParam) == IDC_FIRSTONLY     || LOWORD(wParam) == IDC_ASSIGNWINNOW ||
            LOWORD(wParam) == IDC_SAVEEXITSTATE ||
            LOWORD(wParam) == IDC_HOTMENUEN     || LOWORD(wParam) == IDC_HOTMENUW || 
            LOWORD(wParam) == IDC_HOTMENU       || LOWORD(wParam) == IDC_HOTSTICKYEN  ||
            LOWORD(wParam) == IDC_HOTSTICKY     || LOWORD(wParam) == IDC_HOTSTICKYW ||
            LOWORD(wParam) == IDC_HOTDISMISSEN  || LOWORD(wParam) == IDC_HOTDISMISS ||
            LOWORD(wParam) == IDC_HOTDISMISSW )
        {
            pageChangeMask |= 0x04 ;
            SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L);
        }
        else if (LOWORD(wParam) == IDC_SAVENOW)
            saveAssignedList(nWin,winList) ;
        else if(LOWORD(wParam) == IDC_SAVESTICKY)
            saveStickyWindows(nWin, winList);
        break;
    }
    return (FALSE);
}

/*************************************************
 * The "Expert" tab callback
 */
BOOL APIENTRY setupExpert(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    int newLogFlag ;
    
    switch (message)
    {
    case WM_INITDIALOG:
        SendDlgItemMessage(hDlg, IDC_PRESORDER, CB_ADDSTRING, 0, (LONG) _T("Taskbar order"));
        SendDlgItemMessage(hDlg, IDC_PRESORDER, CB_ADDSTRING, 0, (LONG) _T("Z order"));
        SendDlgItemMessage(hDlg, IDC_PRESORDER, CB_ADDSTRING, 0, (LONG) _T("Taskbar & Z order"));
        SendDlgItemMessage(hDlg, IDC_PRESORDER, CB_SETCURSEL, preserveZOrder, 0) ;
        SendDlgItemMessage(hDlg, IDC_HIDWINACT, CB_ADDSTRING, 0, (LONG) _T("Ignore the event"));
        SendDlgItemMessage(hDlg, IDC_HIDWINACT, CB_ADDSTRING, 0, (LONG) _T("Move window to current desktop"));
        SendDlgItemMessage(hDlg, IDC_HIDWINACT, CB_ADDSTRING, 0, (LONG) _T("Copy window to current desktop"));
        SendDlgItemMessage(hDlg, IDC_HIDWINACT, CB_ADDSTRING, 0, (LONG) _T("Change to window's desktop"));
        SendDlgItemMessage(hDlg, IDC_HIDWINACT, CB_SETCURSEL, hiddenWindowAct, 0) ;
        if(minSwitch)
            SendDlgItemMessage(hDlg, IDC_MINIMIZED, BM_SETCHECK, 1,0);
        if(releaseFocus)
            SendDlgItemMessage(hDlg, IDC_FOCUS, BM_SETCHECK, 1,0);
        if(refreshOnWarp)
            SendDlgItemMessage(hDlg, IDC_REFRESH, BM_SETCHECK, 1,0);
        if(invertY)
            SendDlgItemMessage(hDlg, IDC_INVERTY, BM_SETCHECK, 1,0);
        if(!displayTaskbarIcon)
            SendDlgItemMessage(hDlg, IDC_DISPLAYICON, BM_SETCHECK, 1,0);
        if(!noTaskbarCheck)
            SendDlgItemMessage(hDlg, IDC_TASKBARDETECT, BM_SETCHECK, 1,0);
        if(trickyWindows)
            SendDlgItemMessage(hDlg, IDC_TRICKYSUPPORT, BM_SETCHECK, 1,0);
        if(permanentSticky)
            SendDlgItemMessage(hDlg, IDC_PERMSTICKY, BM_SETCHECK, 1,0);
        if(vwLogFlag)
            SendDlgItemMessage(hDlg, IDC_DEBUGLOGGING, BM_SETCHECK, 1,0);
        return TRUE;
        
    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {
        case PSN_SETACTIVE:
            // Initialize the controls.
            break;
        case PSN_APPLY:
            preserveZOrder = SendDlgItemMessage(hDlg, IDC_PRESORDER, CB_GETCURSEL, 0, 0) ;
            hiddenWindowAct = SendDlgItemMessage(hDlg, IDC_HIDWINACT, CB_GETCURSEL, 0, 0) ;
            minSwitch = (SendDlgItemMessage(hDlg, IDC_MINIMIZED, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            releaseFocus = (SendDlgItemMessage(hDlg, IDC_FOCUS, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            refreshOnWarp = (SendDlgItemMessage(hDlg, IDC_REFRESH, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            invertY = (SendDlgItemMessage(hDlg, IDC_INVERTY, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            noTaskbarCheck = (SendDlgItemMessage(hDlg, IDC_TASKBARDETECT, BM_GETCHECK, 0, 0) != BST_CHECKED) ;
            trickyWindows = (SendDlgItemMessage(hDlg, IDC_TRICKYSUPPORT, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            permanentSticky = (SendDlgItemMessage(hDlg, IDC_PERMSTICKY, BM_GETCHECK, 0, 0) == BST_CHECKED) ;
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
            vwSetupApply(hDlg,0x08) ;
            newLogFlag = (SendDlgItemMessage(hDlg,IDC_DEBUGLOGGING,BM_GETCHECK, 0, 0) == BST_CHECKED) ;
            if(vwLogFlag != newLogFlag)
            {
                vwLogFlag = newLogFlag ;
                MessageBox(hDlg, vwVIRTUAWIN_NAME _T(" must be restarted for the debug logging change to take effect."),
                           vwVIRTUAWIN_NAME _T(" Note"), MB_ICONINFORMATION);
            }
            break;
        case PSN_KILLACTIVE:
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            return 1;
        case PSN_RESET:
            SetWindowLong(hDlg, DWL_MSGRESULT, FALSE);
            break;
        case PSN_HELP:
            showHelp(hDlg,6005);
            break;
        }
        break;
        
    case WM_COMMAND:
        if(LOWORD((wParam) == IDC_EXPLORECNFG))
        {   // Explore Config
            STARTUPINFO si;
            PROCESS_INFORMATION pi;  
            TCHAR cmdLn[MAX_PATH+9], *ss ;
            
            _tcscpy(cmdLn,_T("explorer ")) ;
            GetFilename(vwCONFIG,1,cmdLn+9) ;
            if((ss = _tcsrchr(cmdLn,'\\')) != NULL)
                *ss = '\0' ;
            memset(&si, 0, sizeof(si)); 
            si.cb = sizeof(si); 
            if(CreateProcess(NULL,cmdLn, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
            }
            else
            {
                TCHAR *lpszLastErrorMsg; 
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
                              GetLastError(), 
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language 
                              (TCHAR *) &lpszLastErrorMsg, 
                              0, 
                              NULL ); 
                
                _stprintf(cmdLn,_T("Failed to launch explorer.\n %s"),lpszLastErrorMsg);
                MessageBox(hWnd,cmdLn,vwVIRTUAWIN_NAME _T(" Error"),MB_ICONWARNING) ;
            }
        }
        else if(LOWORD(wParam) == IDC_FOCUS      || LOWORD(wParam) == IDC_TRICKYSUPPORT ||
                LOWORD(wParam) == IDC_MINIMIZED  || LOWORD(wParam) == IDC_DEBUGLOGGING ||
                LOWORD(wParam) == IDC_PERMSTICKY || LOWORD(wParam) == IDC_DISPLAYICON ||
                LOWORD(wParam) == IDC_INVERTY    || LOWORD(wParam) == IDC_TASKBARDETECT ||
                LOWORD(wParam) == IDC_REFRESH    || 
                (LOWORD(wParam) == IDC_PRESORDER && HIWORD(wParam) == LBN_SELCHANGE) ||
                (LOWORD(wParam) == IDC_HIDWINACT && HIWORD(wParam) == LBN_SELCHANGE) )
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
BOOL APIENTRY setupAbout(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    TCHAR license[] = _T("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\r\n \r\nThis program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. \r\n \r\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.");
    
    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemText(hDlg, IDC_LICENSE, license);
        return TRUE;
        
    case WM_CTLCOLORSTATIC:
        if((HWND)lParam == GetDlgItem(hDlg, IDC_MAILTO) ||
           (HWND)lParam == GetDlgItem(hDlg, IDC_HTTP))
        {
            SetBkMode((HDC)wParam, TRANSPARENT); // Don't overwrite background
            SetTextColor((HDC)wParam, RGB(0, 0, 255)); // Blue 
            return (BOOL) GetStockObject(HOLLOW_BRUSH);
        }
        return FALSE;
        
    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {
        case PSN_HELP:
            showHelp(hDlg,0);
            break;
        }
        break;
    
    case WM_COMMAND:
        if(LOWORD(wParam) == IDC_MAILTO)
        {
            HINSTANCE h = ShellExecute(NULL,_T("open"),_T("mailto:") vwVIRTUAWIN_EMAIL, 
                                       NULL, NULL, SW_SHOWNORMAL);
            if((UINT)h < 33)
                MessageBox(hDlg,_T("Error executing mail program."),vwVIRTUAWIN_NAME _T(" Error"),MB_ICONWARNING);
        }
        else if(LOWORD(wParam) == IDC_HTTP)
        {
            HINSTANCE h = ShellExecute(NULL,_T("open"),_T("http://virtuawin.sourceforge.net"), 
                                       NULL, NULL, SW_SHOWNORMAL);
            if((UINT)h < 33)
                MessageBox(hDlg,_T("Error open web link."),vwVIRTUAWIN_NAME _T(" Error"),MB_ICONWARNING);
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
void createPropertySheet(HINSTANCE theHinst, HWND theHwndOwner)
{
    PROPSHEETPAGE psp[vwPROPSHEET_PAGE_COUNT];
    PROPSHEETHEADER psh;
    
    int xIcon = GetSystemMetrics(SM_CXSMICON);
    int yIcon = GetSystemMetrics(SM_CYSMICON);
    
    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USETITLE|PSP_HASHELP;
    psp[0].hInstance = theHinst;
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_GENERAL);
    psp[0].pszIcon = NULL;
    psp[0].pfnDlgProc = setupGeneral;
    psp[0].pszTitle = _T("General");
    psp[0].lParam = 0;
    
    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_USETITLE|PSP_HASHELP;
    psp[1].hInstance = theHinst;
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_MOUSE);
    psp[1].pszIcon = NULL;
    psp[1].pfnDlgProc = setupMouse;
    psp[1].pszTitle = _T("Mouse");
    psp[1].lParam = 0;
    
    psp[2].dwSize = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags = PSP_USETITLE|PSP_HASHELP;
    psp[2].hInstance = theHinst;
    psp[2].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_MODULES);
    psp[2].pszIcon = NULL;
    psp[2].pfnDlgProc = setupModules;
    psp[2].pszTitle = _T("Modules");
    psp[2].lParam = 0;
    
    psp[3].dwSize = sizeof(PROPSHEETPAGE);
    psp[3].dwFlags = PSP_USETITLE|PSP_HASHELP;
    psp[3].hInstance = theHinst;
    psp[3].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_MISC);
    psp[3].pszIcon = NULL;
    psp[3].pfnDlgProc = setupMisc;
    psp[3].pszTitle = _T("Misc.");
    psp[3].lParam = 0;
    
    psp[4].dwSize = sizeof(PROPSHEETPAGE);
    psp[4].dwFlags = PSP_USETITLE|PSP_HASHELP;
    psp[4].hInstance = theHinst;
    psp[4].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_EXPERT);
    psp[4].pszIcon = NULL;
    psp[4].pfnDlgProc = setupExpert;
    psp[4].pszTitle = _T("Expert");
    psp[4].lParam = 0;
    
    psp[5].dwSize = sizeof(PROPSHEETPAGE);
    psp[5].dwFlags = PSP_USETITLE|PSP_HASHELP;
    psp[5].hInstance = theHinst;
    psp[5].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_ABOUT);
    psp[5].pszIcon = NULL;
    psp[5].pfnDlgProc = setupAbout;
    psp[5].pszTitle = _T("About");
    psp[5].lParam = 0;

    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_USECALLBACK | PSH_PROPSHEETPAGE | PSH_USEHICON ;
    psh.hwndParent = theHwndOwner;
    psh.hInstance = theHinst;
    psh.pszIcon = NULL;
    psh.pszCaption = _T("VirtuaWin - Properties") ;
    psh.nPages = vwPROPSHEET_PAGE_COUNT ;
    psh.ppsp = (LPCPROPSHEETPAGE) &psp;
    psh.nStartPage = 0;
    psh.hIcon = (HICON) LoadImage(theHinst, MAKEINTRESOURCE(IDI_VIRTUAWIN), IMAGE_ICON, xIcon, yIcon, 0);
    psh.pfnCallback = (PFNPROPSHEETCALLBACK)propCallBack;
    
    setupKeysHWnd = NULL;
    setupOpen = TRUE;
    PropertySheet(&psh);
    setupOpen = FALSE;
    setupHWnd = NULL;
}
