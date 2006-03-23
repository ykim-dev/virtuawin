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

// Standard includes
#include <io.h>
#include <stdio.h>
#include <string.h>

// Includes
#include "VirtuaWin.h"
#include "ModuleRoutines.h"
#include "Defines.h"
#include "Messages.h"
#include "ConfigParameters.h"
#include "ListStructures.h"
#include "DiskRoutines.h"


/*************************************************
 * Unloads all modules in the module list
 */
void unloadModules(void)
{
    sendModuleMessage(MOD_QUIT, 0, 0);
}

/*************************************************
 * Checks if a module is disabled
 */
static BOOL checkDisabledList(char* theModName)
{
    int modIndex;
    
    for (modIndex = 0; modIndex < curDisabledMod; ++modIndex)
        if (!strncmp(disabledModules[modIndex].moduleName, theModName, (strlen(theModName) - 4)))
            return TRUE; // Module disabled
    return FALSE;  // Not disabled
}

/*************************************************
 * Adds a module to a list, found by loadModules()
 */
static void addModule(char *moduleName, char *path)
{
    char tmpPath[MAX_PATH];
    char errMsg[150];
    HWND myModule;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;  
    int retVal = 1;
    
    if(nOfModules >= MAXMODULES)
    {
        sprintf(errMsg, "Max number of modules where added.\n'%s' won't be loaded.", moduleName);
        MessageBox(hWnd, errMsg, "Warning",0 );
        return;
    }
    
    // Is the module disabled
    if(!checkDisabledList(moduleName))
    {
        if((myModule = FindWindow(moduleName, NULL)))
        {
            sprintf(errMsg, "The module '%s' seems to already be running and will be re-used. \nThis is probably due to incorrect shutdown of VirtuaWin" , moduleName);
            MessageBox(hWnd, errMsg, "Module warning", 0);
        }
        else
        {
            // Startup the module
            strcpy(tmpPath,path) ;
            strcat(tmpPath,moduleName) ;
            memset(&si, 0, sizeof(si)); 
            si.cb = sizeof(si); 
            if(!CreateProcess(tmpPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
            {
                LPTSTR  lpszLastErrorMsg; 
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
                              GetLastError(), 
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language 
                              (LPTSTR) &lpszLastErrorMsg, 
                              0, 
                              NULL ); 
                
                sprintf(errMsg, "Failed to load module '%s'.\n %s", moduleName, lpszLastErrorMsg);
                MessageBox(hWnd, errMsg, "Module error",0 );
                return;
            }
            // Wait max 5 sec for the module to initialize itself
            retVal = WaitForInputIdle( pi.hProcess, 10000); 
            
            // Find the module with classname 
            myModule = FindWindow(moduleName, NULL);
        }
        if(!myModule)
        {
            sprintf(errMsg, "Failed to load module '%s'.\n Maybe wrong class/filename.\nErrcode %d", moduleName, retVal);
            MessageBox(hWnd, errMsg, "Module error",0 );
        }
        else
        {
            moduleList[nOfModules].Handle = myModule;
            moduleList[nOfModules].Disabled = FALSE;
            moduleName[strlen(moduleName)-4] = '\0'; // remove .exe
            strncpy(moduleList[nOfModules].description, moduleName, 79);
            PostMessage(myModule, MOD_INIT, (WPARAM) hWnd , 0);
            nOfModules++;
        }
    } 
    else
    { // Module disabled
        moduleList[nOfModules].Handle = NULL;
        moduleList[nOfModules].Disabled = TRUE;
        moduleName[strlen(moduleName)-4] = '\0'; // remove .exe
        strncpy(moduleList[nOfModules].description, moduleName, 79);
        nOfModules++;
    }
}

/*************************************************
 * Locates modules in "Modules" directory, that is 
 * all files with an .exe extension
 */
void loadModules(void)
{
    WIN32_FIND_DATA exe_file;
    char buff[MAX_PATH], *ss ;
    HANDLE hFile;
    
    GetFilename(vwMODULES,0,buff);
    
    // Find first .exe file in modules directory
    if((hFile = FindFirstFile(buff,&exe_file)) != INVALID_HANDLE_VALUE)
    {
        if((ss = strrchr(buff,'\\')) != NULL)
            ss[1] = '\0' ;
        do {
            addModule(exe_file.cFileName,buff);
        } while(FindNextFile(hFile,&exe_file)) ;
        
        FindClose(hFile);
    }
}

/*************************************************
 * Sends a message to all modules in the list
 */
void sendModuleMessage(UINT Msg, WPARAM wParam,	LPARAM lParam)
{
    int index;
    for(index = 0; index < nOfModules; ++index) {
        if(moduleList[index].Handle != NULL) 
            SendMessage(moduleList[index].Handle, Msg, wParam, lParam);
    }
}

/*************************************************
 * Posts a message to all modules in the list
 */
void postModuleMessage(UINT Msg, WPARAM wParam, LPARAM lParam)
{
    int index;
    for(index = 0; index < nOfModules; ++index) {
        if(moduleList[index].Handle != NULL)
            PostMessage(moduleList[index].Handle, Msg, wParam, lParam);
    }
}

/*
 * $Log$
 * Revision 1.11  2005/03/10 08:06:40  rexkerr
 * Fixed compile error in ModuleRoutines
 *
 * Revision 1.10  2004/04/10 10:20:01  jopi
 * Updated to compile with gcc/mingw
 *
 * Revision 1.9  2004/01/10 11:15:52  jopi
 * Updated copyright for 2004
 *
 * Revision 1.8  2003/02/28 16:59:43  jopi
 * Changed the wait time for module startup timeout to 10 secs instead of 5.
 *
 * Revision 1.7  2003/01/27 20:22:56  jopi
 * Updated copyright header for 2003
 *
 * Revision 1.6  2002/12/29 15:20:38  jopi
 * Fixed copyright info.
 *
 * Revision 1.5  2002/02/14 21:23:40  jopi
 * Updated copyright header
 *
 * Revision 1.4  2001/02/05 21:13:07  jopi
 * Updated copyright header
 *
 * Revision 1.3  2000/08/21 20:54:32  jopi
 * More than MAXMODULES modules would corrupt data and cause undefined behaviour, also increased value from 5 to 10
 *
 * Revision 1.2  2000/07/11 17:45:42  jopi
 * Added check for running modules. Modules will be re-used if already running
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  jopi
 * Added first time
 *
 */
