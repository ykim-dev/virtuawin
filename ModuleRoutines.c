//
//  VirtuaWin - Virtual Desktop Manager for Win9x/NT/Win2K/XP
// 
//  Copyright (c) 1999, 2000, 2001, 2002, 2003 Johan Piculell
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
#include "ModuleRoutines.h"
#include "Messages.h"
#include "ConfigParameters.h"
#include "Defines.h"
#include "ListStructures.h"


/*************************************************
 * Unloads all modules in the module list
 */
void unloadModules()
{
   sendModuleMessage(MOD_QUIT, 0, 0);
}

/*************************************************
 * Locates modules in "Modules" directory, that is 
 * all files with an .exe extension
 */
void loadModules()
{
   struct _finddata_t exe_file;
   long hFile;
  
   // Find first .exe file in modules directory
   if( (hFile = _findfirst(vwModules, &exe_file )) == -1L )
      return;
   else {
      addModule(&exe_file);
   }
   // Find the rest of the .exe files
   while( _findnext( hFile, &exe_file ) == 0 ) {
      addModule(&exe_file);
   }
  
   _findclose( hFile );
}

/*************************************************
 * Adds a module to a list, found by loadModules()
 */
void addModule(struct _finddata_t* aModule)
{
   char errMsg[150];
   HWND myModule;
   char tmpPath[100];
   STARTUPINFO si;
   PROCESS_INFORMATION pi;  
   sprintf(tmpPath, "%smodules\\", vwPath);
  
   if(nOfModules >= MAXMODULES) {
      sprintf(errMsg, "Max number of modules where added.\n'%s' won't be loaded.", aModule->name);
      MessageBox(hWnd, errMsg, "Warning",0 );
      return;
   }
  
   // Is the module disabled
   if(!checkDisabledList(aModule->name)) {
      if(myModule = FindWindow(aModule->name, NULL)) {
         sprintf(errMsg, "The module '%s' seems to already be running and will be re-used. \nThis is probably due to incorrect shutdown of VirtuaWin" , aModule->name);
         MessageBox(hWnd, errMsg, "Module warning", 0);
      } else {
         // Startup the module
         memset(&si, 0, sizeof(si)); 
         si.cb = sizeof(si); 
         if(!CreateProcess(strcat(tmpPath, aModule->name), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            LPTSTR  lpszLastErrorMsg; 
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
                          GetLastError(), 
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language 
                          (LPTSTR) &lpszLastErrorMsg, 
                          0, 
                          NULL ); 
            
            sprintf(errMsg, "Failed to load module '%s'.\n %s", aModule->name, lpszLastErrorMsg);
            MessageBox(hWnd, errMsg, "Module error",0 );
            return;
         }
         // Wait max 5 sec for the module to initialize itself
         WaitForInputIdle( pi.hProcess, 5000); 
         // Find the module with classname 
         myModule = FindWindow(aModule->name, NULL);
      }
      if(!myModule) {
         sprintf(errMsg, "Failed to load module '%s'.\n Maybe wrong class/filename.", aModule->name);
         MessageBox(hWnd, errMsg, "Module error",0 );
      } else {
         moduleList[nOfModules].Handle = myModule;
         moduleList[nOfModules].Disabled = FALSE;
         aModule->name[strlen(aModule->name)-4] = '\0'; // remove .exe
         strncpy(moduleList[nOfModules].description, aModule->name, 79);
         PostMessage(myModule, MOD_INIT, (WPARAM) hWnd , 0);
         nOfModules++;
      }
   } else { // Module disabled
      moduleList[nOfModules].Handle = NULL;
      moduleList[nOfModules].Disabled = TRUE;
      aModule->name[strlen(aModule->name)-4] = '\0'; // remove .exe
      strncpy(moduleList[nOfModules].description, aModule->name, 79);
      nOfModules++;
   }
}

/*************************************************
 * Checks if a module is disabled
 */
BOOL checkDisabledList(char* theModName)
{
  int modIndex;
  
  for (modIndex = 0; modIndex < curDisabledMod; ++modIndex) {
    if (!strncmp(disabledModules[modIndex].moduleName, theModName, (strlen(theModName) - 4)))
      return TRUE; // Module disabled
  }
  return FALSE;  // Not disabled
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
