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
#define vwpTYPE_TRANS  2
#define vwpTYPE_COUNT  3

#define vwpEFFECT_TIME 200
#define vwpEFFECT_STEP 20

#define vwpEFFECT_INPROG  1
#define vwpEFFECT_CLOSE   2
#define vwpEFFECT_FULLCUR 4
#define vwpEFFECT_FULLNXT 8
#define vwpEFFECT_DRAWCUR 16
#define vwpEFFECT_DRAWNXT 32

#define vwpEFFECT_ZOOM_STATIC    0
#define vwpEFFECT_SPRING_NOWRAP  1
#define vwpEFFECT_SPRING_WRAP    2
#define vwpEFFECT_HSPRING_NOWRAP 3
#define vwpEFFECT_HSPRING_WRAP   4
#define vwpEFFECT_VSPRING_NOWRAP 5
#define vwpEFFECT_VSPRING_WRAP   6

#define vwpCMI_FULL    2021
#define vwpCMI_WINDOW  2022
#define vwpCMI_SEP     2023

vwUByte os9x=0;
int vwpType=0;                 /* preview window type */
int vwpBttnHeight=200 ;
int vwpOptimLayout[vwpTYPE_COUNT] = { 0, 1 } ;
vwUByte vwpCtlMenu[vwpTYPE_COUNT] = { 0, 1 } ;
vwUByte vwpHotkeyKey[vwpTYPE_COUNT] = { 0, 0 } ;
vwUByte vwpHotkeyMod[vwpTYPE_COUNT] = { 0, 0 } ;
ATOM    vwpHotkeyAtm[vwpTYPE_COUNT] ;
vwUByte vwpWinClose=1 ;
int     vwpWinUpdateTime=1000 ;
vwUByte vwpFullEffect=1 ;
vwUByte vwpFullUpdate=0 ;
vwUByte vwpFullUpdateCont=1 ;
vwUByte vwpFullUpdateChng=0 ;
int     vwpFullUpdateTime1=2000 ;
int     vwpFullUpdateTime2=10000 ;
int     vwpFullUpdatePause=0 ;
int     vwpFullUpdateCount ;     
vwUByte vwpTransitionEnable=0 ;
vwUByte vwpTransitionEffect=vwpEFFECT_SPRING_WRAP ;
vwUByte vwpFullSizeImages=0 ;
vwUByte vwpKeepImages=1 ;


int     vwpEffectFlag=0 ;
DWORD   vwpEffectStart ;
RECT    vwpEffectRectN ;
RECT    vwpEffectRectNS ;

int vwpBttnsX[vwpTYPE_COUNT] ;
int vwpBttnsY[vwpTYPE_COUNT] ;
int vwpBttnSizeX[vwpTYPE_COUNT] ;
int vwpBttnSizeY[vwpTYPE_COUNT] ;
int vwpFullOffsetX ;
int vwpFullOffsetY ;

HBITMAP dskBtmpImg[vwDESKTOP_MAX] ;
SIZE    dskBtmpSize[vwDESKTOP_MAX] ;
int     dskBtmpTime[vwDESKTOP_MAX] ;

#define vwpDEBUG 0
#if vwpDEBUG
FILE *logfp ;
#endif

int
vwPreviewGetFileTime(char *fname)
{
    HANDLE          *fh ;
    WIN32_FIND_DATA  fd ;
    int tstamp ;
    fh = FindFirstFile(fname,&fd) ;
    if(fh == INVALID_HANDLE_VALUE)
        return 0 ;

    tstamp = (fd.ftLastWriteTime.dwHighDateTime << 16) | (fd.ftLastWriteTime.dwLowDateTime >> 16) | 1 ;
    FindClose(fh) ;
    return tstamp ;
}

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
        if(initialised & 0x100)
            SendMessage(vwHandle,VW_ICHANGEDESK,2,(WPARAM) wHnd) ;
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
    if(vwpTransitionEnable != 0)
    {
        if((xx=SendMessage(vwHandle,VW_ICHANGEDESK,1,(LPARAM) wHnd)) == 2)
            initialised |= 0x100 ;
        else
            MessageBox(wHnd,(xx == 1) ? "Failed to install transition handler - another module has control":"Failed to install transition handler - incompatible version of VirtuaWin, please upgrade.","VWPreview Error", MB_ICONWARNING);
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
    vwpBttnsX[vwpTYPE_TRANS] = vwpBttnsX[vwpTYPE_FULL] ;
    vwpBttnsY[vwpTYPE_TRANS] = vwpBttnsY[vwpTYPE_FULL] ;
    vwpBttnSizeX[vwpTYPE_TRANS] = vwpBttnSizeX[vwpTYPE_FULL] ;
    vwpBttnSizeY[vwpTYPE_TRANS] = vwpBttnSizeY[vwpTYPE_FULL] ;

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
        if(vwpFullSizeImages)
            oy = vwDeskSizeY ;
        else if(((initialised & 0xf0) == 0) ||
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
    char buff[1024] ;
    FILE *fp ;
    int ii, ver ;
    
    strcpy(userAppBase,"vwpreview.cfg") ;
    fp = fopen(userAppPath,"r") ;
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
        vwpLoadConfigFileInt(fp,buff,ii,vwpFullEffect) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpFullUpdate) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpFullUpdateTime2) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpWinClose) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpWinUpdateTime) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpTransitionEnable) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpTransitionEffect) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpFullSizeImages) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpKeepImages) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpFullUpdatePause) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpFullUpdateCont) ;
        vwpLoadConfigFileInt(fp,buff,ii,vwpFullUpdateTime1) ;
        fclose(fp) ;
    }
}

static void
vwpSaveConfigFile(void)
{
    FILE *fp ;
    
    strcpy(userAppBase,"vwpreview.cfg") ;
    fp = fopen(userAppPath,"w") ;
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
        fprintf(fp,"fulEffec# %d\n",vwpFullEffect) ;
        fprintf(fp,"fulUpdat# %d\n",vwpFullUpdate) ;
        fprintf(fp,"fulUdTm2# %d\n",vwpFullUpdateTime2) ;
        fprintf(fp,"winClose# %d\n",vwpWinClose) ;
        fprintf(fp,"winUdTim# %d\n",vwpWinUpdateTime) ;
        fprintf(fp,"trnEnble# %d\n",vwpTransitionEnable) ;
        fprintf(fp,"trnEffct# %d\n",vwpTransitionEffect) ;
        fprintf(fp,"genFulSz# %d\n",vwpFullSizeImages) ;
        fprintf(fp,"genKepIm# %d\n",vwpKeepImages) ;
        fprintf(fp,"fulUdPas# %d\n",vwpFullUpdatePause) ;
        fprintf(fp,"fulUdCnt# %d\n",vwpFullUpdateCont) ;
        fprintf(fp,"fulUdTm1# %d\n",vwpFullUpdateTime1) ;
        fclose(fp) ;
    }
}

static BOOL CALLBACK
vwpSetupDialogFunc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    TCHAR buff[10] ;
    int ii ;
    
#if vwpDEBUG
    _ftprintf(logfp,_T("SetupMessage %x: %d %x %x\n"),hwndDlg,msg,wParam,lParam) ;
    fflush(logfp) ;
#endif
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
        if(vwpWinClose)
            SendDlgItemMessage(hwndDlg,IDC_WN_CLOSE,BM_SETCHECK,1,0) ;
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
        if(vwpFullEffect)
            SendDlgItemMessage(hwndDlg,IDC_FS_ZOOM,BM_SETCHECK,1,0) ;
        if(vwpFullUpdate)
            SendDlgItemMessage(hwndDlg,IDC_FS_UPDATE,BM_SETCHECK,1,0) ;
        if(vwpFullUpdateCont)
            SendDlgItemMessage(hwndDlg,IDC_FS_UDCONT,BM_SETCHECK,1,0) ;
        if(vwpTransitionEnable)
            SendDlgItemMessage(hwndDlg,IDC_TE_ENABLE,BM_SETCHECK,1,0) ;
        SendDlgItemMessage(hwndDlg,IDC_TE_EFFECT,CB_ADDSTRING,0,(LONG) _T("Zoom on from static position"));
        SendDlgItemMessage(hwndDlg,IDC_TE_EFFECT,CB_ADDSTRING,0,(LONG) _T("Spring on from side - no wrap"));
        SendDlgItemMessage(hwndDlg,IDC_TE_EFFECT,CB_ADDSTRING,0,(LONG) _T("Spring on from side - with wrap"));
        SendDlgItemMessage(hwndDlg,IDC_TE_EFFECT,CB_ADDSTRING,0,(LONG) _T("Horizontal spring - no wrap"));
        SendDlgItemMessage(hwndDlg,IDC_TE_EFFECT,CB_ADDSTRING,0,(LONG) _T("Horizontal spring - with wrap"));
        SendDlgItemMessage(hwndDlg,IDC_TE_EFFECT,CB_ADDSTRING,0,(LONG) _T("Vertical spring - no wrap"));
        SendDlgItemMessage(hwndDlg,IDC_TE_EFFECT,CB_ADDSTRING,0,(LONG) _T("Vertical spring - with wrap"));
        SendDlgItemMessage(hwndDlg,IDC_TE_EFFECT,CB_SETCURSEL,vwpTransitionEffect,0) ;
        if(vwpFullSizeImages)
            SendDlgItemMessage(hwndDlg,IDC_GS_FULLSIZE,BM_SETCHECK,1,0) ;
        if(vwpKeepImages)
            SendDlgItemMessage(hwndDlg,IDC_GS_KEEPIMGS,BM_SETCHECK,1,0) ;
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
            vwpWinClose = (SendDlgItemMessage(hwndDlg,IDC_WN_CLOSE,BM_GETCHECK,0,0) == BST_CHECKED) ;
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
            vwpFullEffect = (SendDlgItemMessage(hwndDlg,IDC_FS_ZOOM,BM_GETCHECK,0,0) == BST_CHECKED) ;
            vwpFullUpdate = (SendDlgItemMessage(hwndDlg,IDC_FS_UPDATE,BM_GETCHECK,0,0) == BST_CHECKED) ;
            vwpFullUpdateCont = (SendDlgItemMessage(hwndDlg,IDC_FS_UDCONT,BM_GETCHECK,0,0) == BST_CHECKED) ;
            vwpTransitionEnable = (SendDlgItemMessage(hwndDlg,IDC_TE_ENABLE,BM_GETCHECK,0,0) == BST_CHECKED) ;
            vwpTransitionEffect = (vwUByte) SendDlgItemMessage(hwndDlg,IDC_TE_EFFECT,CB_GETCURSEL,0,0) ;
            vwpFullSizeImages = (SendDlgItemMessage(hwndDlg,IDC_GS_FULLSIZE,BM_GETCHECK,0,0) == BST_CHECKED) ;
            vwpKeepImages = (SendDlgItemMessage(hwndDlg,IDC_GS_KEEPIMGS,BM_GETCHECK,0,0) == BST_CHECKED) ;
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

void
vwpSetupOpen(HWND pHwnd)
{
    // reload current config
    InitCommonControls() ;
    vwpLoadConfigFile() ;
    DialogBox(hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), pHwnd, (DLGPROC) vwpSetupDialogFunc) ;
}

static VOID CALLBACK
vwPreviewRenableImageCapture(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    KillTimer(hwnd,0x29b) ;
    /* re-enable desk image capture */
    if(vwpHnd == NULL)
        SendMessage(vwHandle,VW_DESKIMAGE,8,1) ;
}

void WINAPI
vwPreviewDraw(HDC hdc)
{
    int ii, xx, yy, bx, by, ox, oy, px, py, bsx, bsy ;
    HDC hdcMem;
    
    hdcMem = CreateCompatibleDC(hdc) ;
    if(os9x)
        SetStretchBltMode(hdc,COLORONCOLOR) ;
    else
    {
        SetStretchBltMode(hdc,HALFTONE) ;
        SetBrushOrgEx(hdc,0,0,0) ;
    }
    if(vwpEffectFlag == 0)
    {
        HGDIOBJ pPen, pBsh ;
        RECT rect;
        
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
            if(ox > 0)
            {
                rect.left   = 0 ;
                rect.top    = 0 ;
                rect.right  = ox ;
                rect.bottom = desktopSizeY ;
                FillRect(hdc,&rect,(HBRUSH) (COLOR_3DDKSHADOW+1)) ;
            }
            if(oy > 0)
            {
                rect.left   = 0 ;
                rect.top    = 0 ;
                rect.right  = desktopSizeX ;
                rect.bottom = oy ;
                FillRect(hdc,&rect,(HBRUSH) (COLOR_3DDKSHADOW+1)) ;
            }
        }
        for(ii=0, yy=0, py=oy ; yy < by ; yy++,py+=bsy)
        {
            for(xx=0, px=ox ; xx < bx ; xx++,px+=bsx)
            {
                if((ii < vwDesksN) && dskBtmpImg[ii])
                {
                    SelectObject(hdcMem,dskBtmpImg[ii]) ;
                    if((dskBtmpSize[ii].cx == bsx) && (dskBtmpSize[ii].cy == bsy))
                        BitBlt(hdc,px,py,bsx,bsy,hdcMem,0,0,SRCCOPY) ;
                    else
                        StretchBlt(hdc,px,py,bsx,bsy,hdcMem,0,0,dskBtmpSize[ii].cx,dskBtmpSize[ii].cy,SRCCOPY) ;
                }
                else
                {
                    rect.left   = px ;
                    rect.top    = py ;
                    rect.right  = px+bsx ;
                    rect.bottom = py+bsy ;
                    FillRect(hdc,&rect,(HBRUSH) (ii < vwDesksN) ? ((HBRUSH) (COLOR_BACKGROUND+1)):((HBRUSH) (COLOR_3DDKSHADOW+1))) ;
                }
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
        if(vwpType != vwpTYPE_WINDOW)
        {
            ox = vwpFullOffsetX ;
            oy = vwpFullOffsetY ;
            if(px < desktopSizeX)
            {
                rect.left   = px ;
                rect.top    = 0 ;
                rect.right  = desktopSizeX ;
                rect.bottom = desktopSizeY ;
                FillRect(hdc,&rect,(HBRUSH) (COLOR_3DDKSHADOW+1)) ;
            }
            if(py < desktopSizeY)
            {
                rect.left   = 0 ;
                rect.top    = py ;
                rect.right  = desktopSizeX ;
                rect.bottom = desktopSizeY ;
                FillRect(hdc,&rect,(HBRUSH) (COLOR_3DDKSHADOW+1)) ;
            }
        }
    }
    else if(vwpEffectFlag & vwpEFFECT_FULLNXT)
    {
        ii = vwDeskNxt - 1 ;
        SelectObject(hdcMem,dskBtmpImg[ii]) ;
        if((dskBtmpSize[ii].cx == desktopSizeX) && (dskBtmpSize[ii].cy == desktopSizeY))
            BitBlt(hdc,0,0,desktopSizeX,desktopSizeY,hdcMem,0,0,SRCCOPY) ;
        else
            StretchBlt(hdc,0,0,desktopSizeX,desktopSizeY,hdcMem,0,0,dskBtmpSize[ii].cx,dskBtmpSize[ii].cy,SRCCOPY) ;
    }
    else if(vwpType == vwpTYPE_TRANS)
    {
        if(vwpEffectFlag & vwpEFFECT_FULLCUR)
        {
            ii = vwDeskCur - 1 ;
            SelectObject(hdcMem,dskBtmpImg[ii]) ;
            if((dskBtmpSize[ii].cx == desktopSizeX) && (dskBtmpSize[ii].cy == desktopSizeY))
                BitBlt(hdc,0,0,desktopSizeX,desktopSizeY,hdcMem,0,0,SRCCOPY) ;
            else
                StretchBlt(hdc,0,0,desktopSizeX,desktopSizeY,hdcMem,0,0,dskBtmpSize[ii].cx,dskBtmpSize[ii].cy,SRCCOPY) ;
            vwpEffectFlag &= ~vwpEFFECT_FULLCUR ;
        }
        ii = vwDeskNxt - 1 ;
        xx = vwpEffectRectN.right - vwpEffectRectN.left ;
        yy = vwpEffectRectN.bottom - vwpEffectRectN.top ;
        SelectObject(hdcMem,dskBtmpImg[ii]) ;
        if((dskBtmpSize[ii].cx == xx) && (dskBtmpSize[ii].cy == yy))
            BitBlt(hdc,vwpEffectRectN.left,vwpEffectRectN.top,xx,yy,hdcMem,0,0,SRCCOPY) ;
        else
            StretchBlt(hdc,vwpEffectRectN.left,vwpEffectRectN.top,xx,yy,hdcMem,0,0,dskBtmpSize[ii].cx,dskBtmpSize[ii].cy,SRCCOPY) ;
    }
    else
    {
        ii = vwDeskNxt - 1 ;
        xx = vwpEffectRectN.right - vwpEffectRectN.left ;
        yy = vwpEffectRectN.bottom - vwpEffectRectN.top ;
        SelectObject(hdcMem,dskBtmpImg[ii]) ;
        if((dskBtmpSize[ii].cx == xx) && (dskBtmpSize[ii].cy == yy))
            BitBlt(hdc,vwpEffectRectN.left,vwpEffectRectN.top,xx,yy,hdcMem,0,0,SRCCOPY) ;
        else
            StretchBlt(hdc,vwpEffectRectN.left,vwpEffectRectN.top,xx,yy,hdcMem,0,0,dskBtmpSize[ii].cx,dskBtmpSize[ii].cy,SRCCOPY) ;
    }
    DeleteDC(hdcMem) ;
}

static VOID CALLBACK
vwPreviewEffect(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    DWORD ct ;
    
#if vwpDEBUG
    {
        SYSTEMTIME stime;
    
        GetLocalTime (&stime);
        _ftprintf(logfp,_T("[%04d-%02d-%02d %02d:%02d:%02d:%03d] Zoom %x %d: %d %d\n"),
                  stime.wYear, stime.wMonth, stime.wDay,
                  stime.wHour, stime.wMinute, stime.wSecond, stime.wMilliseconds,vwpHnd,vwpEffectFlag,vwpEffectStart,dwTime-vwpEffectStart) ;
        fflush(logfp) ;
    }
#endif
    if(vwpHnd == NULL)
    {
        KillTimer(vwpHnd,0x29a) ;
        return ;
    }
    if(dwTime == 0)
    {
        SetTimer(vwpHnd,0x29a,vwpEFFECT_STEP,vwPreviewEffect) ;
        vwpEffectStart = GetTickCount() - vwpEFFECT_STEP ;
        vwpEffectFlag = vwpEFFECT_INPROG ;
        ct = vwpEFFECT_STEP ;
        if(vwpType == vwpTYPE_TRANS)
        {
            int ii, jj, xx, yy ;
            if(vwpTransitionEffect == vwpEFFECT_ZOOM_STATIC)
            {
                ii = vwDeskNxt - 1 ;
                xx = (desktopSizeX * ((ii % vwDesksX) * 2 + 1)) / (vwDesksX * 2) ;
                yy = (desktopSizeY * ((ii / vwDesksX) * 2 + 1)) / (vwDesksY * 2) ;
                vwpEffectRectNS.left = xx ;
                vwpEffectRectNS.right = desktopSizeX - xx ;
                vwpEffectRectNS.top = yy ;
                vwpEffectRectNS.bottom = desktopSizeY - yy ;
            }
            else
            {
                if((vwpTransitionEffect == vwpEFFECT_HSPRING_WRAP) || (vwpTransitionEffect == vwpEFFECT_HSPRING_NOWRAP))
                {
                    xx = vwDeskNxt - vwDeskCur ;
                    if((vwpTransitionEffect == vwpEFFECT_HSPRING_WRAP) && (abs(xx) > (vwDesksN >> 1)))
                        xx = 0 - xx ;
                    yy = 0 ;
                }
                else if((vwpTransitionEffect == vwpEFFECT_VSPRING_WRAP) || (vwpTransitionEffect == vwpEFFECT_VSPRING_NOWRAP))
                {
                    xx = 0 ;
                    yy = vwDeskNxt - vwDeskCur ;
                    if((vwpTransitionEffect == vwpEFFECT_VSPRING_WRAP) && (abs(yy) > (vwDesksN >> 1)))
                        yy = 0 - yy ;
                }
                else
                {
                    ii = vwDeskNxt - 1 ;
                    jj = vwDeskCur - 1 ;
                    xx = (ii % vwDesksX) - (jj % vwDesksX) ;
                    yy = (ii / vwDesksX) - (jj / vwDesksX) ;
                    if(vwpTransitionEffect == vwpEFFECT_SPRING_WRAP)
                    {
                        if(abs(xx) > (vwDesksX >> 1))
                            xx = 0 - xx ;
                        if(abs(yy) > (vwDesksY >> 1))
                            yy = 0 - yy ;
                    }
                }
                if((xx == 0) && (yy == 0))
                    xx = 1 ;
                vwpEffectRectNS.left = 0 ;
                vwpEffectRectNS.right = 0 ;
                vwpEffectRectNS.top = 0 ;
                vwpEffectRectNS.bottom = 0 ;
                if(xx < 0)
                    vwpEffectRectNS.right = desktopSizeX ;
                else if(xx > 0)
                    vwpEffectRectNS.left = desktopSizeX ;
                if(yy < 0)
                    vwpEffectRectNS.bottom = desktopSizeY ;
                else if(yy > 0)
                    vwpEffectRectNS.top = desktopSizeY ;
            }
            vwpEffectFlag |= vwpEFFECT_FULLCUR ;
        }
        else
            SetForegroundWindow(desktopHWnd) ;
    }
    else
        ct = dwTime - vwpEffectStart ;
    if(ct >= vwpEFFECT_TIME)
    {
        KillTimer(vwpHnd,0x29a) ;
        if(vwpEffectFlag & vwpEFFECT_CLOSE)
        {
            DestroyWindow(vwpHnd) ;
            vwpEffectFlag = 0 ;
            vwpHnd = NULL ;
            return ;
        }
        InvalidateRect(vwpHnd,NULL,FALSE) ;
        vwpEffectFlag |= vwpEFFECT_CLOSE|vwpEFFECT_FULLNXT ;
    }
    else if(vwpType == vwpTYPE_TRANS)
    {
        int ii = vwpEFFECT_TIME - ct ;
        vwpEffectRectN.left = (vwpEffectRectNS.left*ii) / vwpEFFECT_TIME ;
        vwpEffectRectN.top = (vwpEffectRectNS.top*ii) / vwpEFFECT_TIME ;
        vwpEffectRectN.right = desktopSizeX - ((vwpEffectRectNS.right*ii)/vwpEFFECT_TIME) ;
        vwpEffectRectN.bottom = desktopSizeY - ((vwpEffectRectNS.bottom*ii)/vwpEFFECT_TIME) ;
        InvalidateRect(vwpHnd,&vwpEffectRectN,FALSE) ;
    }
    else
    {
        int ii, bx, by, px, py, bsx, bsy ;
        
        ii = vwDeskNxt - 1 ;
        bx = vwpBttnsX[vwpType] ;
        by = vwpBttnsY[vwpType] ;
        bsx = vwpBttnSizeX[vwpType] ;
        bsy = vwpBttnSizeY[vwpType] ;
        py = ii / bx ;
        px = ((ii - (py * bx)) * bsx) ;
        py = (py * bsy) ;
        
        ii = vwpEFFECT_TIME - ct ;
        vwpEffectRectN.left = (px*ii) / vwpEFFECT_TIME ;
        vwpEffectRectN.top = (py*ii) / vwpEFFECT_TIME ;
        vwpEffectRectN.right = desktopSizeX - (((desktopSizeX-(px+bsx))*ii)/vwpEFFECT_TIME) ;
        vwpEffectRectN.bottom = desktopSizeY - (((desktopSizeY-(py+bsy))*ii)/vwpEFFECT_TIME) ;
        InvalidateRect(vwpHnd,&vwpEffectRectN,FALSE) ;
    }
    UpdateWindow(vwpHnd) ;
#if vwpDEBUG
    _ftprintf(logfp,_T("Zoom end: %d %x\n"),vwpEffectStart,vwpHnd) ;
    fflush(logfp) ;
#endif
}

static VOID CALLBACK
vwPreviewUpdate(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
    BITMAPINFO bmi;
    HDC bitmapDC ;
    HDC deskDC ;
    int ii, ts ;
#if vwpDEBUG
    {
        SYSTEMTIME stime;
    
        GetLocalTime (&stime);
        _ftprintf(logfp,_T("[%04d-%02d-%02d %02d:%02d:%02d:%03d] Update %d %x: %x %d %d\n"),
                  stime.wYear, stime.wMonth, stime.wDay,
                  stime.wHour, stime.wMinute, stime.wSecond, stime.wMilliseconds,vwDeskCur,vwpHnd,hwnd,idEvent,dwTime) ;
        fflush(logfp) ;
    }
#endif
    if(((deskDC = GetDC(desktopHWnd)) == NULL) ||
       ((bitmapDC = CreateCompatibleDC(deskDC)) == NULL))
    {
        MessageBox(wHnd,"Failed to create compatible DC","VWPreview Error", MB_ICONWARNING);
        return ;
    }
    ii = vwDeskCur - 1 ;
    if(vwpType == vwpTYPE_WINDOW)
    {
        /* reset timer as changed after desktop change */
        SetTimer(vwpHnd,0x29a,vwpWinUpdateTime,vwPreviewUpdate) ;
        SendMessage(vwHandle,VW_DESKIMAGE,3,0) ;
    }
    else
    {
        vwpFullUpdateChng = 1 ;
        SetWindowPos(vwpHnd,0,0,0,0,0,(SWP_HIDEWINDOW | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE)) ;
        /* restore the focus */
        SendMessage(vwHandle,VW_FOREGDWIN,0,0) ;
        if(vwpFullUpdatePause > 0)
            Sleep(vwpFullUpdatePause) ;
        SendMessage(vwHandle,VW_DESKIMAGE,8,1) ;
        SendMessage(vwHandle,VW_DESKIMAGE,3,0) ;
        SendMessage(vwHandle,VW_DESKIMAGE,8,0) ;
        SetWindowPos(vwpHnd,HWND_TOPMOST,0,0,0,0,(SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE)) ;
        if(!vwpFullUpdateChng)
        {
#if vwpDEBUG
            _ftprintf(logfp,_T("Update exit: %x\n"),vwpHnd) ;
            fflush(logfp) ;
#endif
            return ;
        }
    }
    _stprintf(userAppBase,_T("desk_%d.bmp"),ii+1) ;
    if((dskBtmpImg[ii] == NULL) || (vwpKeepImages == 0) || ((ts=vwPreviewGetFileTime(userAppPath)) != dskBtmpTime[ii]))
    {
        if(dskBtmpImg[ii])
        {
            DeleteObject(dskBtmpImg[ii]) ;
            dskBtmpImg[ii] = NULL ;
        }
        if((dskBtmpImg[ii] = (HBITMAP) LoadImage(NULL,userAppPath,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE)) == NULL)
        {
            vwpFullUpdateChng = 0 ;
            MessageBox(wHnd,"Failed to load desktop image","VWPreview Error", MB_ICONWARNING);
            return ;
        }
        memset(&bmi,0,sizeof(bmi)) ;
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
        GetDIBits(bitmapDC,dskBtmpImg[ii],0,0,NULL,&bmi,DIB_RGB_COLORS) ;
        if(((dskBtmpSize[ii].cx = bmi.bmiHeader.biWidth) <= 0) || ((dskBtmpSize[ii].cy = bmi.bmiHeader.biHeight) <= 0))
        {
            vwpFullUpdateChng = 0 ;
            MessageBox(wHnd,"Failed to get desktop image size","VWPreview Error", MB_ICONWARNING);
            return ;
        }
        if(vwpKeepImages)
            dskBtmpTime[ii] = ts ;
        InvalidateRect(vwpHnd,NULL,FALSE) ;
        UpdateWindow(vwpHnd) ;
    }
    if((vwpType != vwpTYPE_WINDOW) && vwpFullUpdateChng)
    {
        if(++vwpFullUpdateCount == vwDesksN)
        {
            if(vwpFullUpdateCont)
                SetTimer(vwpHnd,0x29a,vwpFullUpdateTime2,vwPreviewUpdate) ;
            else
                KillTimer(vwpHnd,0x29a) ;
        }
        if((ii += 2) > vwDesksN)
            ii = 1 ;
        PostMessage(vwHandle,VW_CHANGEDESK,ii,0) ;
    }
#if vwpDEBUG
    _ftprintf(logfp,_T("Update end: %d %x\n"),vwpEffectFlag,vwpHnd) ;
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
                SetWindowPos(hwnd,NULL,pt.x,pt.y,xx,yy,SWP_FRAMECHANGED|SWP_HIDEWINDOW|SWP_NOZORDER) ;
                GetClientRect(hwnd,&cr) ;
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
                SetWindowPos(hwnd,HWND_TOPMOST,ox,oy,xx,yy,SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_SHOWWINDOW) ;
                ox = 0 ;
                oy = 0 ;
                if(!vwpWinClose)
                   SetTimer(vwpHnd,0x29a,vwpWinUpdateTime,vwPreviewUpdate) ;
            }
            else
            {
                ii = GetWindowLong(hwnd,GWL_STYLE) ;
                ii &= ~(WS_BORDER | WS_CAPTION | WS_SYSMENU) ;
                SetWindowLong(hwnd,GWL_STYLE,ii) ;
                ii = GetWindowLong(hwnd,GWL_EXSTYLE) ;
                ii &= ~(WS_EX_DLGMODALFRAME|WS_EX_WINDOWEDGE) ;
                SetWindowLong(hwnd,GWL_EXSTYLE,ii) ;  
                SetWindowPos(hwnd,HWND_TOPMOST,0,0,desktopSizeX,desktopSizeY,SWP_FRAMECHANGED|SWP_SHOWWINDOW) ;
                ox = vwpFullOffsetX ;
                oy = vwpFullOffsetY ;
                if(vwpType == vwpTYPE_TRANS)
                    vwPreviewEffect(hwnd,0,0,0) ;
                else if(vwpFullUpdate)
                {
                    vwpFullUpdateChng = 1 ;
                    if((ii = (vwDeskCur + 1)) > vwDesksN)
                        ii = 1 ;
                    PostMessage(vwHandle,VW_CHANGEDESK,ii,0) ;
                    SetTimer(vwpHnd,0x29a,vwpFullUpdateTime1,vwPreviewUpdate) ;
                    vwpFullUpdateCount = 1 ;
                }
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
        if((vwpHnd != NULL) && !vwpEffectFlag)
        {
            int xx = LOWORD(lParam) ;
            int yy = HIWORD(lParam) ;
            int ii ;
            if((vwpType != vwpTYPE_WINDOW) &&
               (((xx -= vwpFullOffsetX) < 0) || ((yy -= vwpFullOffsetY) < 0)))
                return 0 ;
            if(((xx /= vwpBttnSizeX[vwpType]) >= vwpBttnsX[vwpType]) ||
               ((yy /= vwpBttnSizeY[vwpType]) >= vwpBttnsY[vwpType]) ||
               ((ii = (yy * vwpBttnsX[vwpType]) + xx + 1) > vwDesksN))
                return 0 ;
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
            vwpSetupOpen(vwpHnd) ;
        return 0 ;
        
    case WM_CHAR:
        if((vwpHnd != NULL) && !vwpEffectFlag)
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
                vwpFullUpdateChng = 0 ;
                KillTimer(vwpHnd,0x29a) ;
                if(vwDeskNxt != vwDeskCur)
                {
                    PostMessage(vwHandle,VW_CHANGEDESK,vwDeskNxt,0) ;
                    if((vwpType == vwpTYPE_FULL) && vwpFullEffect)
                    {
                        vwDeskCur = vwDeskNxt ;
                        vwPreviewEffect(hwnd,0,0,0) ;
                        return 0 ;
                    }
                }
                if((vwpType == vwpTYPE_WINDOW) && !vwpWinClose)
                    return 0 ;
                /* no break */
            
            case VK_ESCAPE:
                vwpEffectFlag = 0 ;
                vwpFullUpdateChng = 0 ;
                KillTimer(vwpHnd,0x29a) ;
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
            _ftprintf(logfp,_T("WM_DESTROY: %d %x %x %x\n"),vwpEffectFlag,vwpHnd,hwnd,wParam) ;
            fflush(logfp) ;
#endif
            vwpHnd = NULL ;
            ii = vwDesksN ;
            if(vwpKeepImages == 0)
            {
                while(--ii >= 0)
                {
                    if(dskBtmpImg[ii])
                    {
                        DeleteObject(dskBtmpImg[ii]) ;
                        dskBtmpImg[ii] = NULL ;
                    }
                }
            }
            /* delay re-enable of desk image capture to ensure this window has gone */
            SetTimer(wHnd,0x29b,500,vwPreviewRenableImageCapture) ;
            /* give focus to something else */
            SendMessage(vwHandle,VW_FOREGDWIN,0,0) ;
        }
        /* no break */
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
    int ii, ts, loadMask ;
    
    /* check if a preview is already open */
    if(vwpHnd != NULL)
    {
        vwpEffectFlag = 0 ;
        vwpFullUpdateChng = 0 ;
        KillTimer(vwpHnd,0x29a) ;
        DestroyWindow(vwpHnd) ;
        vwpHnd = NULL ;
        if(vwpType == type)
            return ;
    }
    
    if(((deskDC = GetDC(desktopHWnd)) == NULL) ||
       ((bitmapDC = CreateCompatibleDC(deskDC)) == NULL))
    {
        MessageBox(wHnd,"Failed to create compatible DC","VWPreview Error", MB_ICONWARNING);
        return ;
    }
    vwpType = type ;
    if(vwpType == vwpTYPE_TRANS)
    {
        loadMask = (1 << (vwDeskCur - 1)) | (1 << (vwDeskNxt - 1)) ;
    }
    else
    {
        /* update image for current desktop */
        SendMessage(vwHandle,VW_DESKIMAGE,3,0) ;
        loadMask = 0xffffffff ;
        vwDeskNxt = vwDeskCur ;
    }
    ii = vwDesksN ;
    while(--ii >= 0)
    {
        if(loadMask & (1 << ii))
        {
            _stprintf(userAppBase,_T("desk_%d.bmp"),ii+1) ;
            if((vwpKeepImages == 0) || ((ts=vwPreviewGetFileTime(userAppPath)) != dskBtmpTime[ii]) || (dskBtmpImg[ii] == NULL))
            {
#if vwpDEBUG
                _ftprintf(logfp,_T("LoadImage %d: %x %d %x %x\n"),dskBtmpImg[ii],vwpKeepImages,ii,dskBtmpTime[ii],ts) ;
                fflush(logfp) ;
#endif
                if(dskBtmpImg[ii])
                {
                    DeleteObject(dskBtmpImg[ii]) ;
                    dskBtmpImg[ii] = NULL ;
                }
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
                if(vwpKeepImages)
                    dskBtmpTime[ii] = ts ;
            }
        }
    }
    DeleteDC(bitmapDC);
    ReleaseDC(desktopHWnd,deskDC) ;
    if((vwpType != vwpTYPE_TRANS) && ((vwpType != vwpTYPE_WINDOW) || vwpWinClose))
        /* disable desk image generation now to stop this preview window being captured */ 
        SendMessage(vwHandle,VW_DESKIMAGE,8,0) ;
    
    if(vwpType == vwpTYPE_WINDOW)
        ii = WS_BORDER | WS_CAPTION | WS_POPUPWINDOW ; //WS_POPUPWINDOW|WS_BORDER|WS_CAPTION|WS_OVERLAPPED|WS_VISIBLE ;
    else
        ii = 0 ; //WS_VISIBLE ;
    hwnd = CreateWindow("VWPreview","VirtuaWin Preview",ii,CW_USEDEFAULT,CW_USEDEFAULT,100,100,HWND_DESKTOP,NULL,hInst,NULL) ;
    if((hwnd != NULL) && (vwpType != vwpTYPE_TRANS))
    {
        /* try everything to get the current input focus so tab and escape work */
        SetForegroundWindow(hwnd) ; 
        SetFocus(hwnd) ;
        SendMessage(vwHandle,VW_FOREGDWIN,(WPARAM) hwnd,-1) ;
    }
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#if vwpDEBUG
    _ftprintf(logfp,_T("MainMessage %x: %d %x %x\n"),hwnd,msg,wParam,lParam) ;
    fflush(logfp) ;
#endif
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
        vwDeskCur = lParam ;
        if(vwpHnd != 0)
        {
            if(vwpEffectFlag)
            {
                if(vwpEffectFlag & vwpEFFECT_CLOSE)
                {
                    vwpEffectFlag = 0 ;
                    vwpFullUpdateChng = 0 ;
                    KillTimer(vwpHnd,0x29a) ;
                    DestroyWindow(vwpHnd) ;
                    vwpHnd = NULL ;
                }
                else
                    vwpEffectFlag |= vwpEFFECT_CLOSE ;
            }
            else if((vwpType == vwpTYPE_WINDOW) && !vwpWinClose)
            {
                SetTimer(vwpHnd,0x29a,250,vwPreviewUpdate) ;
                vwDeskNxt = lParam ;
            }
            else if(vwpFullUpdateChng)
                vwpFullUpdateChng = 0 ;
            else
            {
                KillTimer(vwpHnd,0x29a) ;
                DestroyWindow(vwpHnd) ;
                vwpHnd = NULL ;
            }
        }
        return TRUE ;
    
    case MOD_QUIT:
        /* This must be handled, otherwise VirtuaWin can't shut down the module */
        PostQuitMessage(0);
        break;
    
    case MOD_SETUP:
        vwpSetupOpen((wParam != 0) ? (HWND) wParam:(HWND) wHnd) ;
        break;
        
    case VW_ICHANGEDESK:
        vwDeskCur = wParam ;
        if((initialised <= 0) || ((vwpHnd != 0) && (vwpType == vwpTYPE_TRANS)))
            PostMessage(vwHandle,VW_ICHANGEDESK,3,0) ;
        else if((vwpHnd != 0) && (vwpType == vwpTYPE_FULL))
        {
            PostMessage(vwHandle,VW_ICHANGEDESK,3,0) ;
            if(!vwpFullUpdateChng)
            {
                vwDeskNxt = lParam ;
                vwPreviewEffect(hwnd,0,0,0) ;
            }
        }
        else
        {
            vwDeskNxt = lParam ;
            vwPreviewCreate(vwpTYPE_TRANS) ;
            PostMessage(vwHandle,VW_ICHANGEDESK,3,0) ;
        }
        return TRUE ;
    
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
    OSVERSIONINFO os;
    WNDCLASS wc ;
    MSG msg ;
    
    hInst = hInstance;
    
    os.dwOSVersionInfoSize = sizeof(os);
    GetVersionEx(&os);
    os9x = ((os.dwPlatformId == VER_PLATFORM_WIN32s) || (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)) ;
#if vwpDEBUG
    logfp = fopen("c:\\temp\\VWPreview.log","w+") ;
    _ftprintf(logfp,_T("Init: %x %d\n"),hInst,(int) os9x) ;
    fflush(logfp) ;
#endif
    
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
    wc.lpfnWndProc = (WNDPROC)vwPreviewDlgProc ;
    wc.hInstance = hInstance ;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIRTUAWIN)) ;
    wc.hCursor = LoadCursor(NULL,IDC_ARROW) ;
    wc.lpszClassName = _T("VWPreview") ;
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
    {
        /* Unregister our interest in generating images */
        PostMessage(vwHandle,VW_DESKIMAGE,2,0) ;
        while(--vwDesksN >= 0)
            if(dskBtmpImg[vwDesksN])
                DeleteObject(dskBtmpImg[vwDesksN]) ;
    }
    
    return msg.wParam;
}
