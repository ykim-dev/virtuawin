/*
 *  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
 *  VWPreview.c - Module to display the time spent on each desk.
 * 
 *  Copyright (c) 2006-2008 VirtuaWin (VirtuaWin@home.se)
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 *  USA.
 * 
 *************************************************************************
 * 
 * Simple module skeleton for VirtuaWin. These are the minimal requirements.
 * It is a simple application with a hidden window that receives messages from virtuawin
 * Look in Messages.h to see what can be sent to and from VirtuaWin
 * 
 * Note that the classname must be the same as the filename including the '.exe'
 */
#include <windows.h>
#include <commctrl.h>
#include <string.h>
#include <stdio.h>
#include <tchar.h>
#include <math.h>
#include "../../Defines.h"
#include "../../Messages.h"
#include "Resource.h"

int initialised=-1 ;
HINSTANCE hInst;               /* Instance handle */
HWND wHnd;                     /* Hidden module window Handle */
HWND vwHandle;                 /* Handle to VirtuaWin */
HWND desktopHWnd ;
int  desktopSizeX ;
int  desktopSizeY ;
HWND vwpHnd=0;                   /* preview window handle */
HPEN curPen ;
HPEN nxtPen ;
TCHAR userAppPath[MAX_PATH] ;  /* User's config path */
TCHAR *userAppBase ;
int vwDesksN ;     
int vwDesksX ;     
int vwDesksY ;     
int vwDeskCur ;
int vwDeskNxt ;
int vwDeskSizeX ;
int vwDeskSizeY ;

#define vwpTYPE_WINDOW 0
#define vwpTYPE_FULL   1
#define vwpTYPE_COUNT  2
#define vwpZOOM_STEPS  10

#define vwpCMI_FULL    2021
#define vwpCMI_WINDOW  2022
#define vwpCMI_SEP     2023

int vwpType=0;                 /* preview window type */
int vwpBttnHeight=200 ;
int vwpOptimLayout[vwpTYPE_COUNT] = { 0, 1 } ;
int vwpZoomEffect[vwpTYPE_COUNT] = { 0, 1 } ;
vwUByte vwpCtlMenu[vwpTYPE_COUNT] = { 0, 1 } ;
vwUByte vwpHotkeyKey[vwpTYPE_COUNT] = { 0, 0 } ;
vwUByte vwpHotkeyMod[vwpTYPE_COUNT] = { 0, 0 } ;
ATOM    vwpHotkeyAtm[vwpTYPE_COUNT] ;
int vwpZoomStep=0 ;
RECT vwpZoomRect ;

int vwpBttnsX[vwpTYPE_COUNT] ;
int vwpBttnsY[vwpTYPE_COUNT] ;
int vwpBttnSizeX[vwpTYPE_COUNT] ;
int vwpBttnSizeY[vwpTYPE_COUNT] ;
int vwpFullOffsetX ;
int vwpFullOffsetY ;

HBITMAP dskBtmpImg[vwDESKTOP_MAX] ;
SIZE    dskBtmpSize[vwDESKTOP_MAX] ;

#define vwpDEBUG 0
#if vwpDEBUG
FILE *logfp ;
#endif

void
vwPreviewInit(void)
{
    vwMenuItemMsg mim ;
    COPYDATASTRUCT cds ;
    double sx, sy, vsb, osb ;
    int xx, yy, ox, oy ;
    RECT rc ;
    
    if(initialised < 0)
        return ;
    if(initialised > 0)
    {
        /* unregister everything first */
        if(initialised & 0x01)
            UnregisterHotKey(wHnd,vwpHotkeyAtm[vwpTYPE_FULL]) ;
        if(initialised & 0x10)
            UnregisterHotKey(wHnd,vwpHotkeyAtm[vwpTYPE_WINDOW]) ;
        /* 2nd time we have initialised so disable DESKIMAGE and remove control menu items first */
        SendMessage(vwHandle,VW_DESKIMAGE,2,0) ;
        if(initialised & 0x22)
            SendMessage(vwHandle,VW_CMENUITEM,(WPARAM) wHnd,0) ;
    }
    initialised = 0 ;
    if(vwpHotkeyKey[vwpTYPE_FULL] != 0)
    {
        if(RegisterHotKey(wHnd,vwpHotkeyAtm[vwpTYPE_FULL],(vwpHotkeyMod[vwpTYPE_FULL] & ~vwHOTKEY_EXT),vwpHotkeyKey[vwpTYPE_FULL]) == FALSE)
            MessageBox(wHnd,"Failed to register full screen preview hotkey","VWPreview Error", MB_ICONWARNING);
        else
            initialised |= 0x01 ;
    }
    if(vwpCtlMenu[vwpTYPE_FULL] != 0)
    {
        mim.position = vwpCMI_FULL ;
        mim.message = vwpCMI_FULL ;
        strcpy(mim.label,"Desktop Preview") ;
        cds.dwData = VW_CMENUITEM ;
        cds.cbData = sizeof(vwMenuItemMsg) ;
        cds.lpData = &mim ;
        if(!SendMessage(vwHandle,WM_COPYDATA,(WPARAM) wHnd,(LPARAM) &cds))
            MessageBox(wHnd,"Failed to install full screen preview menu item","VWPreview Error", MB_ICONWARNING);
        else
            initialised |= 0x02 ;
    }
    if(vwpHotkeyKey[vwpTYPE_WINDOW] != 0)
    {
        if(RegisterHotKey(wHnd,vwpHotkeyAtm[vwpTYPE_WINDOW],(vwpHotkeyMod[vwpTYPE_WINDOW] & ~vwHOTKEY_EXT),vwpHotkeyKey[vwpTYPE_WINDOW]) == FALSE)
            MessageBox(wHnd,"Failed to register full screen preview hotkey","VWPreview Error", MB_ICONWARNING);
        else
            initialised |= 0x10 ;
    }
    if(vwpCtlMenu[vwpTYPE_WINDOW] != 0)
    {
        mim.position = vwpCMI_WINDOW ;
        mim.message = vwpCMI_WINDOW ;
        strcpy(mim.label,"Select Desktop") ;
        cds.dwData = VW_CMENUITEM ;
        cds.cbData = sizeof(vwMenuItemMsg) ;
        cds.lpData = &mim ;
        if(!SendMessage(vwHandle,WM_COPYDATA,(WPARAM) wHnd,(LPARAM) &cds))
            MessageBox(wHnd,"Failed to install full screen preview menu item","VWPreview Error", MB_ICONWARNING);
        else
            initialised |= 0x20 ;
    }
    if(initialised & 0x22)
    {
        mim.position = vwpCMI_SEP ;
        mim.message = 0 ;
        mim.label[0] = 0 ;
        cds.dwData = VW_CMENUITEM ;
        cds.cbData = sizeof(vwMenuItemMsg) ;
        cds.lpData = &mim ;
        SendMessage(vwHandle,WM_COPYDATA,(WPARAM) wHnd,(LPARAM) &cds) ;
    }
    
    if((desktopHWnd = GetDesktopWindow()) == NULL)
        MessageBox(wHnd,"Failed to get desktop window handle","VWPreview Error", MB_ICONWARNING);
    GetClientRect(desktopHWnd,&rc) ;
    desktopSizeX = rc.right - rc.left ;
    desktopSizeY = rc.bottom - rc.top ;
    
    vsb = ((double) desktopSizeX) / ((double) (vwDesksX * vwDeskSizeX)) ;
    sy = ((double) desktopSizeY) / ((double) (vwDesksY * vwDeskSizeY)) ;
    if(sy < vsb)
        vsb = sy ;
    
    osb = vsb ;
    ox = vwDesksX ;
    oy = vwDesksY ;
    if(vwpOptimLayout[0] || vwpOptimLayout[1])
    {
        xx = vwDesksN ;
        do {
            yy = (int) ceil(((double) vwDesksN) / ((double) xx)) ;
            sx = ((double) desktopSizeX) / ((double) (xx * vwDeskSizeX)) ;
            sy = ((double) desktopSizeY) / ((double) (yy * vwDeskSizeY)) ;
            if(sy < sx)
                sx = sy ;
            if(sx > osb)
            {
                osb = sx ;
                ox = xx ;
                oy = yy ;
            }
        } while(--xx > 0) ;
    }
    if(vwpOptimLayout[vwpTYPE_FULL])
    {
        vwpBttnsX[vwpTYPE_FULL] = ox ;
        vwpBttnsY[vwpTYPE_FULL] = oy ;
        vwpBttnSizeX[vwpTYPE_FULL] = (int) (vwDeskSizeX * osb) ;
        vwpBttnSizeY[vwpTYPE_FULL] = (int) (vwDeskSizeY * osb) ;
    }
    else
    {
        vwpBttnsX[vwpTYPE_FULL] = vwDesksX ;
        vwpBttnsY[vwpTYPE_FULL] = vwDesksY ;
        vwpBttnSizeX[vwpTYPE_FULL] = (int) (vwDeskSizeX * vsb) ;
        vwpBttnSizeY[vwpTYPE_FULL] = (int) (vwDeskSizeY * vsb) ;
    }
    vwpFullOffsetX = (desktopSizeX - (vwpBttnsX[vwpTYPE_FULL]*vwpBttnSizeX[vwpTYPE_FULL])) >> 1 ;
    vwpFullOffsetY = (desktopSizeY - (vwpBttnsY[vwpTYPE_FULL]*vwpBttnSizeY[vwpTYPE_FULL])) >> 1 ;

    if(vwpOptimLayout[vwpTYPE_WINDOW])
    {
        vwpBttnsX[vwpTYPE_WINDOW] = ox ;
        vwpBttnsY[vwpTYPE_WINDOW] = oy ;
        vwpBttnSizeX[vwpTYPE_WINDOW] = (int) (vwDeskSizeX * osb) ;
        vwpBttnSizeY[vwpTYPE_WINDOW] = (int) (vwDeskSizeY * osb) ;
    }
    else
    {
        vwpBttnsX[vwpTYPE_WINDOW] = vwDesksX ;
        vwpBttnsY[vwpTYPE_WINDOW] = vwDesksY ;
        vwpBttnSizeX[vwpTYPE_WINDOW] = (int) (vwDeskSizeX * vsb) ;
        vwpBttnSizeY[vwpTYPE_WINDOW] = (int) (vwDeskSizeY * vsb) ;
    }
    if(vwpBttnSizeY[vwpTYPE_WINDOW] > vwpBttnHeight)
    {
        vwpBttnSizeX[vwpTYPE_WINDOW] = (vwpBttnHeight * vwDeskSizeX) / vwDeskSizeY ;
        vwpBttnSizeY[vwpTYPE_WINDOW] = vwpBttnHeight ;
    }
    
    if(initialised)
    {
        if(((initialised & 0xf0) == 0) ||
           (((initialised & 0x0f) != 0) && (vwpBttnSizeY[vwpTYPE_FULL] > vwpBttnSizeY[vwpTYPE_WINDOW])))
            oy = vwpBttnSizeY[vwpTYPE_FULL] ;
        else
            oy = vwpBttnSizeY[vwpTYPE_WINDOW] ;
        if(SendMessage(vwHandle,VW_DESKIMAGE,1,oy) == 0)
        {
            MessageBox(wHnd,_T("VirtuaWin failed to send the UserApp path."),_T("VWPreview Error"), MB_ICONWARNING);
            exit(1) ;
        }
    }
}

/* horrible macro, used as it makes the code more legible - use with care! */
#define vwpLoadConfigFileInt(fp,buff,tmpI,var) if(fscanf(fp, "%s%d", (char *) (buff), &(tmpI)) == 2) (var) = (tmpI)

static void
vwpLoadConfigFile(void)
{
    char buff[1024], *ss ;
    FILE *fp ;
    int ii, ver ;
    
    ss = userAppPath + strlen(userAppPath) ;
    strcpy(ss,"vwpreview.cfg") ;
    fp = fopen(userAppPath,"r") ;
    *ss = '\0' ;
    if(fp != NULL)
    {
        vwpLoadConfigFileInt(fp,buff,ii,ver) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpHotkeyMod[vwpTYPE_WINDOW]) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpHotkeyKey[vwpTYPE_WINDOW]) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpCtlMenu[vwpTYPE_WINDOW]) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpOptimLayout[vwpTYPE_WINDOW]) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpBttnHeight) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpHotkeyMod[vwpTYPE_FULL]) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpHotkeyKey[vwpTYPE_FULL]) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpCtlMenu[vwpTYPE_FULL]) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpOptimLayout[vwpTYPE_FULL]) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpZoomEffect[vwpTYPE_FULL]) ;
        fclose(fp) ;
    }
}

static void
vwpSaveConfigFile(void)
{
    char *ss ;
    FILE *fp ;
    
    ss = userAppPath + strlen(userAppPath) ;
    strcpy(ss,"vwpreview.cfg") ;
    fp = fopen(userAppPath,"w") ;
    *ss = '\0' ;
    if(fp != NULL)
    {
        fprintf(fp,"ver# 1\n") ;
        fprintf(fp,"winHKMod# %d\n",vwpHotkeyMod[vwpTYPE_WINDOW]) ;
        fprintf(fp,"winHKKey# %d\n",vwpHotkeyKey[vwpTYPE_WINDOW]) ;
        fprintf(fp,"winCMenu# %d\n",vwpCtlMenu[vwpTYPE_WINDOW]) ;
        fprintf(fp,"winOpLay# %d\n",vwpOptimLayout[vwpTYPE_WINDOW]) ;
        fprintf(fp,"winButHi# %d\n",vwpBttnHeight) ;
        fprintf(fp,"fulHKMod# %d\n",vwpHotkeyMod[vwpTYPE_FULL]) ;
        fprintf(fp,"fulHKKey# %d\n",vwpHotkeyKey[vwpTYPE_FULL]) ;
        fprintf(fp,"fulCMenu# %d\n",vwpCtlMenu[vwpTYPE_FULL]) ;
        fprintf(fp,"fulOpLay# %d\n",vwpOptimLayout[vwpTYPE_FULL]) ;
        fprintf(fp,"fulZmEff# %d\n",vwpZoomEffect[vwpTYPE_FULL]) ;
        fclose(fp) ;
    }
}

static BOOL CALLBACK
vwpSetupDialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    TCHAR buff[10] ;
    int ii ;
    
    switch(msg)
    {
    case WM_INITDIALOG:
        if(vwpHotkeyKey[vwpTYPE_WINDOW] != 0)
        {
            ii = 0 ;
            if(vwpHotkeyMod[vwpTYPE_WINDOW] & vwHOTKEY_ALT)
                ii |= HOTKEYF_ALT ;
            if(vwpHotkeyMod[vwpTYPE_WINDOW] & vwHOTKEY_CONTROL)
                ii |= HOTKEYF_CONTROL;
            if(vwpHotkeyMod[vwpTYPE_WINDOW] & vwHOTKEY_SHIFT)
                ii |= HOTKEYF_SHIFT;
            if(vwpHotkeyMod[vwpTYPE_WINDOW] & vwHOTKEY_EXT)
                ii |= HOTKEYF_EXT;
            SendDlgItemMessage(hwndDlg,IDC_WN_HOTKEY_ENT,HKM_SETHOTKEY,MAKEWORD(vwpHotkeyKey[vwpTYPE_WINDOW],ii), 0);
            SendDlgItemMessage(hwndDlg,IDC_WN_HOTKEY_WIN,BM_SETCHECK,((vwpHotkeyMod[vwpTYPE_WINDOW] & vwHOTKEY_WIN) != 0), 0);
        }
        if(vwpCtlMenu[vwpTYPE_WINDOW])
            SendDlgItemMessage(hwndDlg,IDC_WN_VWCONTROL,BM_SETCHECK,1,0) ;
        if(vwpOptimLayout[vwpTYPE_WINDOW])
            SendDlgItemMessage(hwndDlg,IDC_WN_OPTIMLAY,BM_SETCHECK,1,0) ;
        SetDlgItemInt(hwndDlg,IDC_WN_IMGHEIGHT,vwpBttnHeight,TRUE) ;
        
        if(vwpHotkeyKey[vwpTYPE_FULL] != 0)
        {
            ii = 0 ;
            if(vwpHotkeyMod[vwpTYPE_FULL] & vwHOTKEY_ALT)
                ii |= HOTKEYF_ALT ;
            if(vwpHotkeyMod[vwpTYPE_FULL] & vwHOTKEY_CONTROL)
                ii |= HOTKEYF_CONTROL;
            if(vwpHotkeyMod[vwpTYPE_FULL] & vwHOTKEY_SHIFT)
                ii |= HOTKEYF_SHIFT;
            if(vwpHotkeyMod[vwpTYPE_FULL] & vwHOTKEY_EXT)
                ii |= HOTKEYF_EXT;
            SendDlgItemMessage(hwndDlg,IDC_FS_HOTKEY_ENT,HKM_SETHOTKEY,MAKEWORD(vwpHotkeyKey[vwpTYPE_FULL],ii), 0);
            SendDlgItemMessage(hwndDlg,IDC_FS_HOTKEY_WIN,BM_SETCHECK,((vwpHotkeyMod[vwpTYPE_FULL] & vwHOTKEY_WIN) != 0), 0);
        }
        if(vwpCtlMenu[vwpTYPE_FULL])
            SendDlgItemMessage(hwndDlg,IDC_FS_VWCONTROL,BM_SETCHECK,1,0) ;
        if(vwpOptimLayout[vwpTYPE_FULL])
            SendDlgItemMessage(hwndDlg,IDC_FS_OPTIMLAY,BM_SETCHECK,1,0) ;
        if(vwpZoomEffect[vwpTYPE_FULL])
            SendDlgItemMessage(hwndDlg,IDC_FS_ZOOM,BM_SETCHECK,1,0) ;
        return 1 ;
        
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwndDlg,0) ;
            return 1 ;
        
        case ID_OK:
        case ID_APPLY:
            if((ii=SendDlgItemMessage(hwndDlg,IDC_WN_HOTKEY_ENT,HKM_GETHOTKEY,0,0)) > 0)
            {
                int mm=0, jj=HIBYTE(ii) ;
                if(jj & HOTKEYF_ALT)
                    mm |= vwHOTKEY_ALT ;
                if(jj & HOTKEYF_CONTROL)
                    mm |= vwHOTKEY_CONTROL;
                if(jj & HOTKEYF_SHIFT)
                    mm |= vwHOTKEY_SHIFT;
                if(jj & HOTKEYF_EXT)
                    mm |= vwHOTKEY_EXT;
                if(SendDlgItemMessage(hwndDlg,IDC_WN_HOTKEY_WIN,BM_GETCHECK,0,0) == BST_CHECKED)
                    mm |= vwHOTKEY_WIN ;
                vwpHotkeyMod[vwpTYPE_WINDOW] = mm ; 
                vwpHotkeyKey[vwpTYPE_WINDOW] = LOBYTE(ii) ;
            }
            else
                vwpHotkeyKey[vwpTYPE_WINDOW] = 0 ; 
            vwpCtlMenu[vwpTYPE_WINDOW] = (SendDlgItemMessage(hwndDlg,IDC_WN_VWCONTROL,BM_GETCHECK,0,0) == BST_CHECKED) ;
            vwpOptimLayout[vwpTYPE_WINDOW] = (SendDlgItemMessage(hwndDlg,IDC_WN_OPTIMLAY,BM_GETCHECK,0,0) == BST_CHECKED) ;
            GetDlgItemText(hwndDlg,IDC_WN_IMGHEIGHT,buff,10);
            if((ii = _ttoi(buff)) > 0)
                vwpBttnHeight = ii ;
            
            if((ii=SendDlgItemMessage(hwndDlg,IDC_FS_HOTKEY_ENT,HKM_GETHOTKEY,0,0)) > 0)
            {
                int mm=0, jj=HIBYTE(ii) ;
                if(jj & HOTKEYF_ALT)
                    mm |= vwHOTKEY_ALT ;
                if(jj & HOTKEYF_CONTROL)
                    mm |= vwHOTKEY_CONTROL;
                if(jj & HOTKEYF_SHIFT)
                    mm |= vwHOTKEY_SHIFT;
                if(jj & HOTKEYF_EXT)
                    mm |= vwHOTKEY_EXT;
                if(SendDlgItemMessage(hwndDlg,IDC_FS_HOTKEY_WIN,BM_GETCHECK,0,0) == BST_CHECKED)
                    mm |= vwHOTKEY_WIN ;
                vwpHotkeyMod[vwpTYPE_FULL] = mm ; 
                vwpHotkeyKey[vwpTYPE_FULL] = LOBYTE(ii) ;
            }
            else
                vwpHotkeyKey[vwpTYPE_FULL] = 0 ; 
            vwpCtlMenu[vwpTYPE_FULL] = (SendDlgItemMessage(hwndDlg,IDC_FS_VWCONTROL,BM_GETCHECK,0,0) == BST_CHECKED) ;
            vwpOptimLayout[vwpTYPE_FULL] = (SendDlgItemMessage(hwndDlg,IDC_FS_OPTIMLAY,BM_GETCHECK,0,0) == BST_CHECKED) ;
            vwpZoomEffect[vwpTYPE_FULL] = (SendDlgItemMessage(hwndDlg,IDC_FS_ZOOM,BM_GETCHECK,0,0) == BST_CHECKED) ;
            vwpSaveConfigFile() ;
            vwPreviewInit() ;
            if(LOWORD(wParam) == ID_OK)
                EndDialog(hwndDlg,0) ;
            return 1 ;
        }
        break;
        
    case WM_CLOSE:
        EndDialog(hwndDlg,0) ;
        return 1 ;
    }
    return 0 ;
}


void WINAPI
vwPreviewDraw(HDC hdc)
{
    int ii, xx, yy, bx, by, ox, oy, px, py, bsx, bsy ;
    HDC hdcMem;
    
    hdcMem = CreateCompatibleDC(hdc) ;
    if(vwpZoomStep)
    {
        ii = vwDeskCur - 1 ;
        SelectObject(hdcMem,dskBtmpImg[ii]) ;
        StretchBlt(hdc,vwpZoomRect.left,vwpZoomRect.top,vwpZoomRect.right-vwpZoomRect.left,vwpZoomRect.bottom-vwpZoomRect.top,
                   hdcMem,0,0,dskBtmpSize[ii].cx,dskBtmpSize[ii].cy,SRCCOPY) ;
    }
    else
    {
        HGDIOBJ pPen, pBsh ;
        
        bx = vwpBttnsX[vwpType] ;
        by = vwpBttnsY[vwpType] ;
        bsx = vwpBttnSizeX[vwpType] ;
        bsy = vwpBttnSizeY[vwpType] ;
        
        if(vwpType == vwpTYPE_WINDOW)
        {
            ox = 0 ;
            oy = 0 ;
        }
        else
        {
            ox = vwpFullOffsetX ;
            oy = vwpFullOffsetY ;
        }
        for(ii=0, yy=0, py=oy ; yy < by ; yy++,py+=bsy)
        {
            for(xx=0, px=ox ; xx < bx ; xx++,px+=bsx)
            {
                SelectObject(hdcMem,dskBtmpImg[ii]) ;
                StretchBlt(hdc,px,py,bsx,bsy,hdcMem,0,0,dskBtmpSize[ii].cx,dskBtmpSize[ii].cy,SRCCOPY) ;
                ii++ ;
                if((ii == vwDeskCur) || (ii == vwDeskNxt))
                {
                    pPen = SelectObject(hdc,(ii == vwDeskNxt) ? nxtPen:curPen) ;
                    pBsh = SelectObject(hdc,GetStockObject(HOLLOW_BRUSH)) ;
                    /* XOR no good on a grey background SetROP2(hdc,R2_XORPEN) ; */
                    Rectangle(hdc,px+1,py+1,px+bsx,py+bsy) ;
                    SelectObject(hdc,pPen) ;
                    SelectObject(hdc,pBsh) ;
                }
            }
        }
    }
    DeleteDC(hdcMem) ;
}

static VOID CALLBACK
vwPreviewZoom(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
#if vwpDEBUG
    {
        SYSTEMTIME stime;
    
        GetLocalTime (&stime);
        _ftprintf(logfp,_T("[%04d-%02d-%02d %02d:%02d:%02d:%03d] Zoom %d %x: %x %d %d\n"),
                  stime.wYear, stime.wMonth, stime.wDay,
                  stime.wHour, stime.wMinute, stime.wSecond, stime.wMilliseconds,vwpZoomStep,vwpHnd,hwnd,idEvent,dwTime) ;
        fflush(logfp) ;
    }
#endif
    if(vwpHnd == NULL)
    {
        KillTimer(vwpHnd,0x29a) ;
    }
    else if(vwpZoomStep >= vwpZOOM_STEPS)
    {
        KillTimer(vwpHnd,0x29a) ;
        DestroyWindow(vwpHnd) ;
        vwpZoomStep = 0 ;
        vwpHnd = NULL ;
    }
    else
    {
        int ii, bx, by, px, py, bsx, bsy ;
        
        if(vwpZoomStep == 0)
        {
            SetForegroundWindow(desktopHWnd) ;
            SetTimer(vwpHnd,0x29a,25,vwPreviewZoom) ;
        }

        ii = vwDeskCur - 1 ;
        bx = vwpBttnsX[vwpType] ;
        by = vwpBttnsY[vwpType] ;
        bsx = vwpBttnSizeX[vwpType] ;
        bsy = vwpBttnSizeY[vwpType] ;
        py = ii / bx ;
        px = ((ii - (py * bx)) * bsx) ;
        py = (py * bsy) ;
        if(vwpType != vwpTYPE_WINDOW)
        {
            px += vwpFullOffsetX ;
            py += vwpFullOffsetY ;
        }
        
        vwpZoomStep++ ;
        ii = vwpZOOM_STEPS - vwpZoomStep ;
        vwpZoomRect.left = (px*ii) / vwpZOOM_STEPS ;
        vwpZoomRect.top = (py*ii) / vwpZOOM_STEPS ;
        vwpZoomRect.right = desktopSizeX - (((desktopSizeX-(px+bsx))*ii)/vwpZOOM_STEPS) ;
        vwpZoomRect.bottom = desktopSizeY - (((desktopSizeY-(py+bsy))*ii)/vwpZOOM_STEPS) ;
        InvalidateRect(vwpHnd,&vwpZoomRect,FALSE) ;
        UpdateWindow(vwpHnd) ;
    }
#if vwpDEBUG
    _ftprintf(logfp,_T("Zoom end: %d %x\n"),vwpZoomStep,vwpHnd) ;
    fflush(logfp) ;
#endif
}

LRESULT CALLBACK 
vwPreviewDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
#if vwpDEBUG
    _ftprintf(logfp,_T("Message %x: %d %x %x\n"),hwnd,msg,wParam,lParam) ;
    fflush(logfp) ;
#endif
    switch (msg)
    {
    case WM_CREATE:
        {
            int ii, xx, yy, bx, by, bsx, bsy, ox, oy ;
            vwpHnd = hwnd ;
            /* Stop VW from managing this window! */
            SendMessage(vwHandle,VW_WINMANAGE,(WPARAM) vwpHnd,0) ;
            bx = vwpBttnsX[vwpType] ;
            by = vwpBttnsY[vwpType] ;
            bsx = vwpBttnSizeX[vwpType] ;
            bsy = vwpBttnSizeY[vwpType] ;
            xx = bx*bsx ;
            yy = by*bsy ;
            if(vwpType == vwpTYPE_WINDOW)
            {
                POINT pt ;
                RECT cr ;
                GetCursorPos(&pt);
                SetWindowPos(hwnd,HWND_TOPMOST,pt.x,pt.y,xx,yy,SWP_FRAMECHANGED) ;
                GetClientRect(hwnd,&cr) ;
#if vwpDEBUG
                _ftprintf(logfp,_T("Window: %dx%d : %d,%d,%d,%d : "),xx,yy,cr.left,cr.right,cr.top,cr.bottom) ;
#endif
                xx += xx - (cr.right - cr.left) ;
                yy += yy - (cr.bottom - cr.top) ;
                ox = pt.x - (xx >> 1) ;
                if((ox + xx) > desktopSizeX)
                    ox = desktopSizeX - xx ;
                if(ox < 0)
                    ox = 0 ;
                oy = pt.y - (yy >> 1) ;
                if((oy + yy) > desktopSizeY)
                    oy = desktopSizeY - yy ;
                if(oy < 0)
                    oy = 0 ;
#if vwpDEBUG
                _ftprintf(logfp,_T("%d,%d,%d,%d\n"),xx,yy,ox,oy) ;
                fflush(logfp) ;
#endif
                MoveWindow(hwnd,ox,oy,xx,yy,FALSE) ;
                ox = 0 ;
                oy = 0 ;
            }
            else
            {
                ii = GetWindowLong(hwnd,GWL_STYLE) ;
                ii &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU) ;
                SetWindowLong(hwnd,GWL_STYLE,ii) ;
                ii = GetWindowLong(hwnd,GWL_EXSTYLE) ;
                ii &= ~(WS_EX_DLGMODALFRAME|WS_EX_WINDOWEDGE) ;
                SetWindowLong(hwnd,GWL_EXSTYLE,ii) ;  
                SetWindowPos(hwnd,HWND_TOPMOST,0,0,desktopSizeX,desktopSizeY,SWP_FRAMECHANGED) ;
                ox = vwpFullOffsetX ;
                oy = vwpFullOffsetY ;
            }
            return 1;
        }
        
    case WM_ERASEBKGND:
        return 1 ;
        
    case WM_PAINT:
        {
            PAINTSTRUCT ps ;
            HDC hdc ;

            hdc = BeginPaint(hwnd,&ps) ;
            vwPreviewDraw(hdc) ;
            EndPaint(hwnd,&ps) ;
            return 0 ;
        }
        
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        if((vwpHnd != NULL) && !vwpZoomStep)
        {
            int xx = LOWORD(lParam) ;
            int yy = HIWORD(lParam) ;
            int ii ;
            if(vwpType != vwpTYPE_WINDOW)
            {
                xx -= vwpFullOffsetX ;
                yy -= vwpFullOffsetY ;
            }
            ii = ((yy / vwpBttnSizeY[vwpType]) * vwpBttnsX[vwpType]) + (xx / vwpBttnSizeX[vwpType]) + 1 ;
            if(msg == WM_LBUTTONDOWN)
            {
                vwDeskNxt = ii ;
                InvalidateRect(vwpHnd,NULL,FALSE) ;
            }
            else if(ii == vwDeskNxt)
                goto sel_desk ;
            return 0 ;
        }
    
    case WM_RBUTTONDOWN:
        return 0 ;
    
    case WM_RBUTTONUP:
        if(vwpHnd != NULL)
            DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), vwpHnd, (DLGPROC) vwpSetupDialogFunc);
        return 0 ;
        
    case WM_CHAR:
        if((vwpHnd != NULL) && !vwpZoomStep)
        {
            switch(wParam)
            {
            case VK_TAB:
                if(HIWORD(GetKeyState(VK_SHIFT)))
                {
                    if(--vwDeskNxt <= 0)
                        vwDeskNxt = vwDesksN ;
                }
                else if(++vwDeskNxt > vwDesksN)
                    vwDeskNxt = 1 ;
                InvalidateRect(vwpHnd,NULL,FALSE) ;
                return 0 ;
            
            case VK_RETURN:
sel_desk:
                if(vwDeskNxt != vwDeskCur)
                {
                    PostMessage(vwHandle,VW_CHANGEDESK,vwDeskNxt,0) ;
                    if(vwpZoomEffect[vwpType])
                    {
                        vwDeskCur = vwDeskNxt ;
                        vwpZoomStep = 0 ;
                        vwPreviewZoom(hwnd,0,0,0) ;
                        return 0 ;
                    }
                }
                /* no break */
            
            case VK_ESCAPE:
                DestroyWindow(hwnd) ;
                vwpHnd = NULL ;
                return 0 ;
            }
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
        
    case WM_DESTROY:
        {
            int ii ;
            
#if vwpDEBUG
            _ftprintf(logfp,_T("WM_DESTROY: %d %x %x %x\n"),vwpZoomStep,vwpHnd,hwnd,wParam) ;
            fflush(logfp) ;
#endif
            /* re-enable desk image capture */
            SendMessage(vwHandle,VW_DESKIMAGE,8,1) ;
            ii = vwDesksN ;
            while(--ii >= 0)
            {
                if(dskBtmpImg[ii])
                {
                    DeleteObject(dskBtmpImg[ii]) ;
                    dskBtmpImg[ii] = NULL ;
                }
            }
            vwpHnd = NULL ;
        }
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void
vwPreviewCreate(int type)
{
    BITMAPINFO bmi;
    HDC bitmapDC ;
    HDC deskDC ;
    HWND hwnd ;
    int ii ;
    
    /* check if a preview is already open */
    if(vwpHnd != NULL)
    {
        if(vwpType == type)
        {
            DestroyWindow(vwpHnd) ;
            vwpHnd = NULL ;
            return ;
        }
        if(vwpZoomStep)
        {
            KillTimer(vwpHnd,0x29a) ;
            vwpZoomStep = 0 ;
        }
        DestroyWindow(vwpHnd) ;
        vwpHnd = NULL ;
    }
    
    /* update image for current desktop */
    if(((deskDC = GetDC(desktopHWnd)) == NULL) ||
       ((bitmapDC = CreateCompatibleDC(deskDC)) == NULL))
    {
        MessageBox(wHnd,"Failed to create compatible DC","VWPreview Error", MB_ICONWARNING);
        return ;
    }
    vwpType = type ;
    vwpZoomStep = 0 ;
    vwDeskNxt = vwDeskCur ;
    SendMessage(vwHandle,VW_DESKIMAGE,3,0) ;
    
    ii = vwDesksN ;
    while(--ii >= 0)
    {
        if(dskBtmpImg[ii])
        {
            DeleteObject(dskBtmpImg[ii]) ;
            dskBtmpImg[ii] = NULL ;
        }
        _stprintf(userAppBase,_T("desk_%d.bmp"),ii+1) ;
        if((dskBtmpImg[ii] = (HBITMAP) LoadImage(NULL,userAppPath,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE)) == NULL)
        {
            MessageBox(wHnd,"Failed to load desktop image","VWPreview Error", MB_ICONWARNING);
            return ;
        }
        memset(&bmi,0,sizeof(bmi)) ;
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
        GetDIBits(bitmapDC,dskBtmpImg[ii],0,0,NULL,&bmi,DIB_RGB_COLORS) ;
        if(((dskBtmpSize[ii].cx = bmi.bmiHeader.biWidth) <= 0) || ((dskBtmpSize[ii].cy = bmi.bmiHeader.biHeight) <= 0))
        {
            MessageBox(wHnd,"Failed to get desktop image size","VWPreview Error", MB_ICONWARNING);
            return ;
        }
    }
    DeleteDC(bitmapDC);
    ReleaseDC(desktopHWnd,deskDC) ;
    /* disable desk image generation now to stop this preview window being captured */ 
    SendMessage(vwHandle,VW_DESKIMAGE,8,0) ;
    
    if(vwpType == vwpTYPE_WINDOW)
        ii = WS_POPUPWINDOW|WS_BORDER|WS_CAPTION|WS_OVERLAPPED|WS_VISIBLE ;
    else
        ii = WS_VISIBLE ;
    hwnd = CreateWindow("VWPreview","VirtuaWin Preview",ii,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,HWND_DESKTOP,NULL,hInst,NULL) ;
    if(hwnd != NULL)
    {
        SetForegroundWindow(hwnd) ; 
        SetFocus(hwnd) ;
    }
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case MOD_INIT: 
        /* This must be taken care of in order to get the handle to VirtuaWin. */
        /* The handle to VirtuaWin comes in the wParam */
        vwHandle = (HWND) wParam; /* Should be some error handling here if NULL */
        // If not yet initialized get the user path and initialize.
        if(initialised<0)
        {
            if(!SendMessage(vwHandle, VW_USERAPPPATH, (WPARAM) hwnd, 0) || (initialised < 0))
            {
                MessageBox(hwnd,_T("VirtuaWin failed to send the UserApp path."),_T("VWPreview Error"), MB_ICONWARNING);
                exit(1) ;
            }
        }
        /* no break */
    case MOD_CFGCHANGE:
        if(initialised >= 0)
        {
            vwDesksX = SendMessage(vwHandle,VW_DESKX,0,0) ;
            vwDesksY = SendMessage(vwHandle,VW_DESKY,0,0) ;
            vwDesksN = vwDesksX * vwDesksY ;
            vwDeskCur = SendMessage(vwHandle,VW_CURDESK,0,0) ;
            if(((vwDeskSizeX = SendMessage(vwHandle,VW_DESKIMAGE,7,0)) <= 0) ||
               ((vwDeskSizeY = SendMessage(vwHandle,VW_DESKIMAGE,6,0)) <= 0))
            {
                MessageBox(hwnd,_T("Incompatible version of VirtuaWin, please upgrade."),_T("VWPreview Error"), MB_ICONWARNING);
                exit(1) ;
            }
            vwPreviewInit() ;
        }
        break;
    
    case MOD_CHANGEDESK:
        if((vwpHnd != 0) && !vwpZoomStep)
        {
            DestroyWindow(vwpHnd) ;
            vwpHnd = NULL ;
        }
        vwDeskCur = lParam ;
        return TRUE ;
   case MOD_QUIT:
        /* This must be handled, otherwise VirtuaWin can't shut down the module */
        PostQuitMessage(0);
        break;
    case MOD_SETUP:
        if(wParam != 0)
            hwnd = (HWND) wParam ;
        else
            hwnd = (HWND) wHnd ;
        // reload current config
        vwpLoadConfigFile() ;
        DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), hwnd, (DLGPROC) vwpSetupDialogFunc);
        break;
    
    case VW_CMENUITEM:
        if((wParam == vwpCMI_FULL) || (wParam == vwpCMI_WINDOW))
           vwPreviewCreate(wParam == vwpCMI_FULL) ;
        break;
    
    case WM_COPYDATA:
        if(initialised < 0)
        {
            COPYDATASTRUCT *cds;         
            cds = (COPYDATASTRUCT *) lParam ;         
            if(cds->dwData == (0-VW_USERAPPPATH))
            {
                if((cds->cbData < 2) || (cds->lpData == NULL))
                    return FALSE ;
                initialised = 0 ;
                strcpy(userAppPath,(char *) cds->lpData) ;
                userAppBase = userAppPath + _tcslen(userAppPath) ;
                vwpLoadConfigFile() ;
            }
        }
        return TRUE ;
        
    case WM_HOTKEY:
        if((vwpHotkeyAtm[0] == wParam) || (vwpHotkeyAtm[1] == wParam))
           vwPreviewCreate(wParam == vwpHotkeyAtm[1]) ;
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
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    WNDCLASS wc ;
    MSG msg ;
    
#if vwpDEBUG
    logfp = fopen("c:\\temp\\VWPreview.log","w+") ;
#endif
    hInst = hInstance;
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.hInstance = hInstance ;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIRTUAWIN));
    /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
       this for locating the window */
    wc.lpszClassName = _T("VWPreview.exe");
    if(!RegisterClass(&wc))
        return 0;
  
    if((wHnd=CreateWindow(_T("VWPreview.exe"), 
                          _T("VWPreview"), 
                          WS_POPUP,
                          CW_USEDEFAULT, 
                          0, 
                          CW_USEDEFAULT, 
                          0,
                          NULL,
                          NULL,
                          hInstance,
                          NULL)) == (HWND) 0)
        return 0;
    
    memset(&wc, 0, sizeof(WNDCLASS));
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)vwPreviewDlgProc;
    wc.hInstance = hInstance ;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIRTUAWIN));
    wc.lpszClassName = _T("VWPreview");
    if(!RegisterClass(&wc))
        return 0;
    
    if(((vwpHotkeyAtm[0] = GlobalAddAtom("vwpAtom0")) == 0) ||
       ((vwpHotkeyAtm[1] = GlobalAddAtom("vwpAtom1")) == 0))
    {
        MessageBox(wHnd,"Failed to create hotkey atoms","VWPreview Error", MB_ICONWARNING);
        return 0 ;
    }
    curPen = CreatePen(PS_SOLID,2,RGB(128,128,128)) ;
    nxtPen = CreatePen(PS_SOLID,2,RGB(0,255,0)) ;
    
    /* main messge loop */
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    if(initialised)
        /* Unregister our interest in generating images */
        PostMessage(vwHandle,VW_DESKIMAGE,2,0) ;
    
    return msg.wParam;
}
