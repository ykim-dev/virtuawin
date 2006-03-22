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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lmcons.h>
#include <io.h>
#include <errno.h>
#include <assert.h>
#include <shlobj.h>  // for SHGetFolderPath
#include <winbase.h> // for GetTempPath
#include <stddef.h>  // for size_t
#include <direct.h>  // for mkdir
#include <winreg.h>  // For regisry calls

#include "VirtuaWin.h"
#include "DiskRoutines.h"
#include "ConfigParameters.h"

#define VIRTUAWIN_SUBDIR  "VirtuaWin"
#define LOCK_FILENAME     ".vwLock.cfg"

char *VirtuaWinPath=NULL ;
char *UserAppPath=NULL ;

/************************************************** 
 * Gets the local application settings path for the current user.
 * Calling function MUST pre-allocate the return string!
 */
static void getUserAppPath(char* path)
{
    LPITEMIDLIST idList ;
    int len ;
    
    path[0] = '\0' ;
    if(SUCCEEDED(SHGetSpecialFolderLocation(NULL,CSIDL_APPDATA,&idList)) && (idList != NULL))
    {
        IMalloc *im ;
        SHGetPathFromIDList(idList,path);
        if(SUCCEEDED(SHGetMalloc(&im)) && (im != NULL))
        {
            im->lpVtbl->Free(im,idList) ;
            im->lpVtbl->Release(im);
        }
    }
    
    if(path[0] != '\0')
    {
        len = strlen(path) ;
        if(path[len - 1] != '\\')
            path[len++] = '\\' ;
        strncpy(path+len,VIRTUAWIN_SUBDIR,MAX_PATH - len) ;
        if(((GetFileAttributes(path) & (0xf0000000|FILE_ATTRIBUTE_DIRECTORY)) != FILE_ATTRIBUTE_DIRECTORY) &&
           (CreateDirectory(path,NULL) == 0))
        {
            MessageBox(hWnd, "VirtuaWin cannot create its config directory.\nIf you continue to have problems, send e-mail to \nvirtuawin@home.se", "General Error", MB_ICONWARNING);
            exit(1) ;
        }
        len += strlen(path+len) ;
        path[len++] = '\\' ;
        path[len] = '\0' ;
    }
}

/************************************************
 * Generates a path + username for the requested file type, fills in
 * the supplied string parameter, that MUST be pre-allocated.  Caller
 * is responsible for memory allocation.  It is suggested that the
 * caller allocate the string on the stack, not dynamically on the
 * heap so that it is cleaned up automatically.  Strings can be up to
 * MAX_PATH in length.
 *
 * Return is 1 if successful, 0 otherwise.  (suggestion, convert to
 * type bool after porting to cpp)
 */

int GetFilename(eFileNames filetype, char* outStr)
{
    static char *subPath[vwFILE_COUNT] = {
        "userlist.cfg", "virtuawin", "tricky.cfg", "modules\\*.exe",
        "vwconfig.cfg", "sticky.cfg", "vwdisabled.cfg", "vwwindowsstate.cfg"
    };
    DWORD len ;
    
    if(UserAppPath == NULL)
    {
        /* initialization of paths, find the installation and user paths,
         * exit on failure - initialization happens so early on it is safe to
         * simply exit */
        char path[MAX_PATH], *ss ;
        
        GetModuleFileName(GetModuleHandle(NULL),path,MAX_PATH) ;
        ss = strrchr(path,'\\') ;
        ss[1] = '\0' ;
        VirtuaWinPath = strdup(path) ;
        getUserAppPath(path) ;  // for now I only want multiuser, we'll probably get rid of the registry entry
        if(path[0] == '\0')
            UserAppPath = VirtuaWinPath ;
        else
            UserAppPath = strdup(path) ;
        if((VirtuaWinPath == NULL) || (UserAppPath == NULL))
        {
            MessageBox(hWnd, "Memory resources appear to be very low, try rebooting.\nIf you still have problems, send a mail to \nvirtuawin@home.se", "General Error", MB_ICONWARNING);
            exit(1);
        }
    }
    
    if(filetype >= vwFILE_COUNT)
        return 0 ;
    strncpy(outStr,(filetype < vwCONFIG) ? VirtuaWinPath:UserAppPath, MAX_PATH);
    len = MAX_PATH - strlen(outStr) ;
    strncat(outStr,subPath[filetype],len) ;
    return 1;
}


/*************************************************
 * Write out the disabled modules
 */
void saveDisabledList(int theNOfModules, moduleType* theModList)
{
    FILE* fp;
    
    char DisabledFileList[MAX_PATH];
    GetFilename(vwDISABLED, DisabledFileList);
    
    if(!(fp = fopen(DisabledFileList, "w")))
        MessageBox(hWnd, "Error saving disabled module state", NULL, MB_ICONWARNING);
    else
    {
        int i;
        for(i = 0; i < theNOfModules; ++i)
            if(theModList[i].Disabled)
                fprintf(fp, "%s\n", (char*)&theModList[i].description);
        fclose(fp);
    }
}

/*************************************************
 * Loads module names that should be disabled 
 */
int loadDisabledModules(disModules *theDisList) 
{
    char buff[MAX_PATH];
    int len, nOfDisMod = 0;
    FILE *fp;
    
    GetFilename(vwDISABLED, buff);
    
    if((fp = fopen(buff,"r")) != NULL)
    {
        while(fgets(buff,MAX_PATH,fp) != NULL)
        {
            if((len = strlen(buff)) > 1)
            {
                if(len > vwMODULENAME_MAX)
                    buff[vwMODULENAME_MAX] = '\0' ;
                else if(buff[len-1] == '\n')
                    buff[len-1] = '\0' ;
                strcpy(theDisList[nOfDisMod++].moduleName,buff);
                if(nOfDisMod == (MAXMODULES * 2))
                    break ;
            }
        }
        fclose(fp);
    }
    return nOfDisMod;
}

/*************************************************
 * Loads window classnames from sticky file
 */
int loadStickyList(stickyType *theStickyList) 
{
    char buff[MAX_PATH];
    int len, nOfSticky = 0;
    FILE* fp;
    
    GetFilename(vwSTICKY,buff);
    
    if((fp = fopen(buff,"r")) != NULL)
    {
        while(fgets(buff,MAX_PATH,fp) != NULL)
        {
            if((len = strlen(buff)) > 1)
            {
                if(len > vwCLASSNAME_MAX)
                    buff[vwCLASSNAME_MAX] = '\0' ;
                else if(buff[len-1] == '\n')
                    buff[len-1] = '\0' ;
                if((theStickyList[nOfSticky].winClassName = strdup(buff)) != NULL)
                    nOfSticky++;
            }
        }
        fclose(fp);
    }
    return nOfSticky;
}

/*************************************************
 * Writes down the classnames of the sticky windows on file
 * File format:
 *   <WinClassName>\n
 *   <WinClassName>\n
 */
void saveStickyWindows(int theNOfWin, windowType *theWinList)
{
    char className[vwCLASSNAME_MAX+2];
    FILE* fp;
    
    char vwStickyList[MAX_PATH];
    GetFilename(vwSTICKY, vwStickyList);
    
    if((fp = fopen(vwStickyList, "w")) == NULL)
        MessageBox(hWnd, "Error writing sticky file", NULL, MB_ICONWARNING);
    else
    {
        int i;
        for(i = 0; i < theNOfWin; ++i)
        {
            if(theWinList[i].Sticky)
            {
                GetClassName(theWinList[i].Handle,className,vwCLASSNAME_MAX);
                className[vwCLASSNAME_MAX] = '\n' ;
                className[vwCLASSNAME_MAX+1] = '\0' ;
                fputs(className,fp) ;
            }
        }
        fclose(fp);
    }
}

/*************************************************
 * Loads window classnames from tricky file
 */
int loadTrickyList(stickyType *theTrickyList) 
{
    char buff[MAX_PATH];
    int len, nOfTricky = 0;
    FILE* fp;
    
    GetFilename(vwTRICKY, buff);
    if((fp = fopen(buff, "r")) != NULL) 
    {
        while(fgets(buff,MAX_PATH,fp) != NULL)
        {
            if((len = strlen(buff)) > 1)
            {
                if(len > vwCLASSNAME_MAX)
                    buff[vwCLASSNAME_MAX] = '\0' ;
                else if(buff[len-1] == '\n')
                    buff[len-1] = '\0' ;
                if((theTrickyList[nOfTricky].winClassName = strdup(buff)) != NULL)
                    nOfTricky++;
            }
        }
        fclose(fp);
    }
    return nOfTricky;
}


/*************************************************
 * Writes down the current desktop layout to a file
 * File format:
 *   <desk #> <WinClassName>\n
*   <desk #> <WinClassName>\n
*/
void saveAssignedList(int theNOfWin, windowType *theWinList)
{
    char buff[MAX_PATH];
    FILE *fp;
    
    GetFilename(vwWINDOWS_STATE,buff);
    if((fp = fopen(buff, "w")) == NULL)
        MessageBox(hWnd, "Error writing desktop configuration file", NULL, MB_ICONWARNING);
    else
    {
        int i;
        for(i = 0; i < theNOfWin; ++i)
        {
            GetClassName(theWinList[i].Handle,buff,vwCLASSNAME_MAX);
            buff[vwCLASSNAME_MAX] = '\0' ;
            fprintf(fp, "%d %s\n",theWinList[i].Desk,buff);
        }
        fclose(fp);
    }
}

/*************************************************
 * Loads the list with classnames that has an desktop assigned
 */
int loadAssignedList(assignedType *theAssignList) 
{
    char buff[MAX_PATH], className[MAX_PATH];
    int curAssigned = 0;
    FILE *fp;
    
    GetFilename(vwWINDOWS_STATE,buff) ;
    if((fp = fopen(buff, "r")) != NULL)
    {
        while(fgets(buff,MAX_PATH,fp) != NULL)
        {
            if(sscanf(buff, "%d %s",&theAssignList[curAssigned].desktop,className) == 2)
            {
                className[vwCLASSNAME_MAX] = '\0' ;
                if((className[0] != '\0') &&
                   ((theAssignList[curAssigned].winClassName = strdup(className)) != NULL))
                    curAssigned++;
            }
        }
        fclose(fp);
    }
    return curAssigned;
}

/*************************************************
 * Loads window titles/classnames from user file 
 */
int loadUserList(userType* theUserList) 
{
    char buff[MAX_PATH];
    FILE* fp;
    int curUser = 0;
    
    GetFilename(vwLIST, buff);
    
    if((fp = fopen(buff, "r")))
    {
        while(!feof(fp)) {
            fgets(buff, 99, fp);
            // Remove the newline
            if(buff[strlen(buff) - 1] == '\n') {
                buff[strlen(buff) - 1] = '\0';
            }
            if(curUser < MAXUSER && buff[0] != ':' && (strlen(buff) != 0) && !feof(fp)) {
                theUserList[curUser].winNameClass = malloc(sizeof(char) * strlen(buff) + 1);
                strcpy(theUserList[curUser].winNameClass, buff);
                theUserList[curUser].isClass = TRUE;
                curUser++;
            } 
        }
        fclose(fp);
    }
    
    return curUser;
}

/************************************************
 * Writes down the current configuration on file
 */
void writeConfig(void)
{
    FILE* fp;
    int ii ;
    
    char VWConfigFile[MAX_PATH];
    GetFilename(vwCONFIG, VWConfigFile);
    
    if((fp = fopen(VWConfigFile, "w")) == NULL) {
        MessageBox(NULL, "Error writing config file", NULL, MB_ICONWARNING);
    } else {
        fprintf(fp, "Mouse_warp# %i\n", mouseEnable);
        fprintf(fp, "Mouse_delay# %i\n", configMultiplier);
        fprintf(fp, "Key_support# %i\n", keyEnable);
        fprintf(fp, "Release_focus# %i\n", releaseFocus);
        fprintf(fp, "Keep_active# %i\n", keepActive);
        fprintf(fp, "Control_key_alt# %i\n", modAlt);
        fprintf(fp, "Control_key_shift# %i\n", modShift);
        fprintf(fp, "Control_key_ctrl# %i\n", modCtrl);
        fprintf(fp, "Control_key_win# %i\n", modWin);
        fprintf(fp, "Warp_jump# %i\n", warpLength);
        fprintf(fp, "Switch_minimized# %i\n", minSwitch);
        fprintf(fp, "Taskbar_warp# %i\n", taskBarWarp);
        fprintf(fp, "Desk_Ysize# %i\n", nDesksY);
        fprintf(fp, "Desk_Xsize# %i\n", nDesksX);
        fprintf(fp, "Hot_key_support# %i\n", hotKeyEnable);
        for(ii=1 ; ii<10 ; ii++) {
            fprintf(fp, "Hot_key_%d# %i\n", ii,deskHotkey[ii]);
            fprintf(fp, "Hot_key_Mod%d# %i\n", ii,deskHotkeyMod[ii]);
            fprintf(fp, "Hot_key_Win%d# %i\n", ii,deskHotkeyWin[ii]);
        }
        fprintf(fp, "Mouse_control_key_support# %i\n", useMouseKey);
        fprintf(fp, "Mouse_key_alt# %i\n", mouseModAlt);
        fprintf(fp, "Mouse_key_shift# %i\n", mouseModShift);
        fprintf(fp, "Mouse_key_ctrl# %i\n", mouseModCtrl);
        fprintf(fp, "Save_sticky_info# %i\n", saveSticky);
        fprintf(fp, "Refresh_after_warp# %i\n", refreshOnWarp);
        fprintf(fp, "No_mouse_wrap# %i\n", noMouseWrap);
        fprintf(fp, "Sticky_modifier# %i\n", hotkeyStickyMod);
        fprintf(fp, "Sticky_key# %i\n", hotkeySticky);
        fprintf(fp, "Crash_recovery# %i\n", crashRecovery);
        fprintf(fp, "Desktop_cycling# %i\n", deskWrap);
        fprintf(fp, "Invert_Y# %i\n", invertY);
        fprintf(fp, "WinMenu_sticky# %i\n", stickyMenu);
        fprintf(fp, "WinMenu_assign# %i\n", assignMenu);
        fprintf(fp, "WinMenu_direct# %i\n", directMenu);
        fprintf(fp, "Desktop_assignment# %i\n", useDeskAssignment);
        fprintf(fp, "Save_layout# %i\n", saveLayoutOnExit);
        fprintf(fp, "Assign_first# %i\n", assignOnlyFirst);
        fprintf(fp, "UseCyclingKeys# %i\n", cyclingKeysEnabled);
        fprintf(fp, "CycleUp# %i\n", hotCycleUp);
        fprintf(fp, "CycleUpMod# %i\n", hotCycleUpMod);
        fprintf(fp, "CycleDown# %i\n", hotCycleDown);
        fprintf(fp, "CycleDownMod# %i\n", hotCycleDownMod);
        fprintf(fp, "Hot_key_Menu_Support# %i\n", hotkeyMenuEn);
        fprintf(fp, "Hot_key_Menu# %i\n", hotkeyMenu);
        fprintf(fp, "Hot_key_ModMenu# %i\n", hotkeyMenuMod);
        fprintf(fp, "Hot_key_WinMenu# %i\n", hotkeyMenuWin);
        fprintf(fp, "Display_systray_icon# %i\n", displayTaskbarIcon);
        fprintf(fp, "Sticky_Win# %i\n", hotkeyStickyWin);
        fprintf(fp, "Taskbar_detection# %i\n", noTaskbarCheck);
        fprintf(fp, "Use_trickywindows# %i\n", trickyWindows);
        fprintf(fp, "XPStyleTaskbar# %i\n", taskbarOffset);
        fprintf(fp, "PermanentSticky# %i\n", permanentSticky);
        fprintf(fp, "CycleUpWin# %i\n", hotCycleUpWin);
        fprintf(fp, "CycleDownWin# %i\n", hotCycleDownWin);
        fprintf(fp, "Sticky_Win_En# %i\n", hotkeyStickyEn);
        fprintf(fp, "Hot_key_10# %i\n",deskHotkey[10]);
        fprintf(fp, "Hot_key_Mod10# %i\n",deskHotkeyMod[10]);
        fprintf(fp, "Hot_key_Win10# %i\n",deskHotkeyWin[10]);
        fclose(fp);
    }
}

/*************************************************
 * Reads a saved configuration from file
 */
void readConfig(void)
{
    char buff[MAX_PATH];
    FILE* fp;
    int ii;
    
    GetFilename(vwCONFIG, buff);
    if((fp = fopen(buff, "r")) == NULL)
    {
        MessageBox(NULL, "Error reading config file. This is probably due to new user setup.\nA new config file will be created.", NULL, MB_ICONWARNING);
        // Try to create new file
        if((fp = fopen(buff, "w")) == NULL)
            MessageBox(NULL, "Error writing new config file. Check writepermissions.", NULL, MB_ICONWARNING);
    }
    else
    {   
        fscanf(fp, "%s%i", buff, &mouseEnable);
        fscanf(fp, "%s%i", buff, &configMultiplier);
        fscanf(fp, "%s%i", buff, &keyEnable);
        fscanf(fp, "%s%i", buff, &releaseFocus);
        fscanf(fp, "%s%i", buff, &keepActive);
        fscanf(fp, "%s%i", buff, &modAlt);
        fscanf(fp, "%s%i", buff, &modShift);
        fscanf(fp, "%s%i", buff, &modCtrl);
        fscanf(fp, "%s%i", buff, &modWin);
        fscanf(fp, "%s%i", buff, &warpLength);
        fscanf(fp, "%s%i", buff, &minSwitch);
        fscanf(fp, "%s%i", buff, &taskBarWarp);
        fscanf(fp, "%s%i", buff, &nDesksY);
        fscanf(fp, "%s%i", buff, &nDesksX);
        fscanf(fp, "%s%i", buff, &hotKeyEnable);
        for(ii=1 ; ii<10 ; ii++)
        {
            fscanf(fp, "%s%i", buff, deskHotkey + ii);
            fscanf(fp, "%s%i", buff, deskHotkeyMod + ii);
            fscanf(fp, "%s%i", buff, deskHotkeyWin + ii);
        }
        fscanf(fp, "%s%i", buff, &useMouseKey);
        fscanf(fp, "%s%i", buff, &mouseModAlt);
        fscanf(fp, "%s%i", buff, &mouseModShift);
        fscanf(fp, "%s%i", buff, &mouseModCtrl);
        fscanf(fp, "%s%i", buff, &saveSticky);
        fscanf(fp, "%s%i", buff, &refreshOnWarp);
        fscanf(fp, "%s%i", buff, &noMouseWrap);
        fscanf(fp, "%s%i", buff, &hotkeyStickyMod);
        fscanf(fp, "%s%i", buff, &hotkeySticky);
        fscanf(fp, "%s%i", buff, &crashRecovery);
        fscanf(fp, "%s%i", buff, &deskWrap);
        fscanf(fp, "%s%i", buff, &invertY);
        fscanf(fp, "%s%hi", buff, &stickyMenu);
        fscanf(fp, "%s%hi", buff, &assignMenu);
        fscanf(fp, "%s%hi", buff, &directMenu);
        fscanf(fp, "%s%i", buff, &useDeskAssignment);
        fscanf(fp, "%s%i", buff, &saveLayoutOnExit);
        fscanf(fp, "%s%i", buff, &assignOnlyFirst);
        fscanf(fp, "%s%i", buff, &cyclingKeysEnabled);
        fscanf(fp, "%s%i", buff, &hotCycleUp);
        fscanf(fp, "%s%i", buff, &hotCycleUpMod);
        fscanf(fp, "%s%i", buff, &hotCycleDown);
        fscanf(fp, "%s%i", buff, &hotCycleDownMod);
        fscanf(fp, "%s%i", buff, &hotkeyMenuEn);
        fscanf(fp, "%s%i", buff, &hotkeyMenu);
        fscanf(fp, "%s%i", buff, &hotkeyMenuMod);
        fscanf(fp, "%s%i", buff, &hotkeyMenuWin);
        fscanf(fp, "%s%i", buff, &displayTaskbarIcon);
        fscanf(fp, "%s%i", buff, &hotkeyStickyWin);
        fscanf(fp, "%s%i", buff, &noTaskbarCheck);
        fscanf(fp, "%s%i", buff, &trickyWindows);
        fscanf(fp, "%s%i", buff, &taskbarOffset);
        fscanf(fp, "%s%i", buff, &permanentSticky);
        if(fscanf(fp, "%s%i", buff, &hotCycleUpWin) == 2)
        {
            fscanf(fp, "%s%i", buff, &hotCycleDownWin);
            fscanf(fp, "%s%i", buff, &hotkeyStickyEn);
            fscanf(fp, "%s%i", buff, deskHotkey + 10);
            fscanf(fp, "%s%i", buff, deskHotkeyMod + 10);
            fscanf(fp, "%s%i", buff, deskHotkeyWin + 10);
        }
        fclose(fp);
    }
}


/*************************************************
 * Check if we have a previous lock file, otherwise it creates it
 */
BOOL tryToLock(void)
{
    BOOL retval = FALSE;
    TCHAR lockFile[MAX_PATH];
    if(GetTempPath(MAX_PATH, lockFile))
    {
        strncat(lockFile, LOCK_FILENAME, MAX_PATH - strlen(lockFile));
        
        if(access(lockFile, 0 == -1)) 
        {
            FILE* fp;
            if(!(fp = fopen(lockFile, "wc"))) {
                MessageBox(hWnd, "Error writing lock file", "VirtuaWin", MB_ICONWARNING);
                return TRUE;
            } 
            else 
            {
                fprintf(fp, "%s", "VirtuaWin LockFile");
            }
            
            fflush(fp); // Make sure the file is physically written to disk
            fclose(fp);
            
            retval = TRUE;
        } 
        else 
        {
            retval = FALSE; // We already had a lock file, probably due to a previous crash
        }
    }
    
    return retval;
}

void clearLock(void)
{
    // returns void because there's not much that we can do if it fails anyhow.
    TCHAR lockFile[MAX_PATH];
    if(GetTempPath(MAX_PATH, lockFile))
    {
        strncat(lockFile, LOCK_FILENAME, MAX_PATH - strlen(lockFile));
        remove(lockFile);
    }
}

/*
 * $Log$
 * Revision 1.23  2005/03/10 08:10:18  rexkerr
 * Removed debugging code
 *
 * Revision 1.22  2005/03/10 08:02:10  rexkerr
 * Added multi-user support
 *
 * Revision 1.21  2004/04/10 10:20:01  jopi
 * Updated to compile with gcc/mingw
 *
 * Revision 1.20  2004/02/28 18:54:01  jopi
 * SF904069 Added possibility to choose if sticky should be permanent for all instances of the same classname.
 *
 * Revision 1.19  2004/01/10 11:15:52  jopi
 * Updated copyright for 2004
 *
 * Revision 1.18  2003/07/10 20:37:54  jopi
 * Made it possible to disable the multiuser config support with a registry value
 *
 * Revision 1.17  2003/06/24 19:49:08  jopi
 * SF693876 Fixed option to handle XP skinned style taskbars
 *
 * Revision 1.16  2003/01/27 20:22:56  jopi
 * Updated copyright header for 2003
 *
 * Revision 1.15  2002/12/23 15:42:29  jopi
 * Added config options to disable taskbar detection and the alternative hiding technique.
 *
 * Revision 1.14  2002/10/01 19:52:48  Johan Piculell
 * Fixed a memory leak
 *
 * Revision 1.13  2002/08/08 21:13:02  Johan Piculell
 * Fixed so that the recovery file is written with correct endlines.
 *
 * Revision 1.12  2002/06/01 21:15:23  Johan Piculell
 * Multiple fixes by Christian Storm.
 *
 * Revision 1.11  2002/02/14 21:23:39  Johan Piculell
 * Updated copyright header
 *
 * Revision 1.10  2001/12/01 00:05:52  Johan Piculell
 * Added alternative window hiding for troublesome windows like InternetExplorer
 *
 * Revision 1.9  2001/11/12 21:39:14  Johan Piculell
 * Added functionality for disabling the systray icon
 *
 * Revision 1.8  2001/11/12 18:21:52  Johan Piculell
 * Added support for classnames that contains spaces which will fix some
 * problems with desktop state save and sticky save.
 *
 * Revision 1.7  2001/02/05 21:13:07  Administrator
 * Updated copyright header
 *
 * Revision 1.6  2001/01/14 16:27:42  Administrator
 * Moved io.h include to DiskRoutines.c
 *
 * Revision 1.5  2001/01/12 18:11:26  Administrator
 * Moved some disk stuff from VirtuaWin to DiskRoutines
 *
 * Revision 1.4  2000/12/11 20:39:57  Administrator
 * Fixed a bug with the username lookup for config file, could go wrong sometimes
 *
 * Revision 1.3  2000/08/19 15:00:26  Administrator
 * Added multiple user setup support (Alasdair McCaig) and fixed creation of setup file if it don't exist
 *
 * Revision 1.2  2000/07/18 16:02:29  Administrator
 * Changed mail adress in error message
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  Administrator
 * Added first time
 *
 */
