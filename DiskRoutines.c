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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lmcons.h>
#include <io.h>

#include "DiskRoutines.h"
#include "ConfigParameters.h"


/*************************************************
 * Routines for getting VirtuaWin path from the registry 
 * and fixup the different filenames
 */
void loadFilePaths()
{
   HKEY hkey = NULL;
   DWORD dwType;
   DWORD cbData = 0;
   DWORD dwUserNameLen = 0;
   LPSTR winCurrentUser;
   long lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\VirtuaWin\\Settings", 0,
                               KEY_READ, &hkey);
   if (hkey) {
      lResult = RegQueryValueEx(hkey, "Path", NULL, &dwType, NULL, &cbData);
      vwPath   = (LPSTR)malloc(cbData * sizeof(char));
      // Get current user data
      dwUserNameLen = ( (UNLEN + 1) * sizeof(char) );
      winCurrentUser = (LPSTR)malloc( dwUserNameLen );
      lResult = GetUserName( (LPBYTE) winCurrentUser, &dwUserNameLen );

      vwConfig = (LPSTR)malloc(cbData * sizeof(char) + dwUserNameLen * sizeof(char) + 14);
      vwList   = (LPSTR)malloc(cbData * sizeof(char) + 13);
      vwHelp   = (LPSTR)malloc(cbData * sizeof(char) + 10);
      vwSticky = (LPSTR)malloc(cbData * sizeof(char) + 11);
      vwTricky = (LPSTR)malloc(cbData * sizeof(char) + 11);
      vwState  = (LPSTR)malloc(cbData * sizeof(char) + 12);
      vwLock   = (LPSTR)malloc(cbData * sizeof(char) + 12);
      vwModules = (LPSTR)malloc(cbData * sizeof(char) + 15);
      vwDisabled = (LPSTR)malloc(cbData * sizeof(char) + 15);
      vwWindowsState = (LPSTR)malloc(cbData * sizeof(char) + 19);
      /* Get the key value. */
      lResult = RegQueryValueEx(hkey, "Path", NULL, &dwType,
                                (LPBYTE)vwPath, &cbData);

      sprintf(vwConfig, "%svwconfig.%s.cfg", vwPath, winCurrentUser);
      free(winCurrentUser);
      sprintf(vwList, "%suserlist.cfg", vwPath);
      sprintf(vwHelp, "%svirtuawin", vwPath);
      sprintf(vwSticky, "%ssticky.cfg", vwPath);
      sprintf(vwTricky, "%stricky.cfg", vwPath);
      sprintf(vwState, "%svwstate.cfg", vwPath);
      sprintf(vwLock, "%s.vwLock.cfg", vwPath);
      sprintf(vwModules, "%smodules\\*.exe", vwPath);
      sprintf(vwDisabled, "%svwDisabled.cfg", vwPath);
      sprintf(vwWindowsState, "%svwWindowsState.cfg", vwPath);
   } else {
      MessageBox(hWnd, "VirtuaWin is not correctly installed, try to reinstall.\nIf you still have problems, send a mail to \nvirtuawin@home.se", "Registry Error", MB_ICONWARNING);
      PostQuitMessage(0);
      //return 0;
   }
}

/*************************************************
 * Write out the disabled modules
 */
void writeDisabledList(int* theNOfModules, moduleType* theModList)
{
   FILE* fp;
   
   if(!(fp = fopen(vwDisabled, "w"))) {
      MessageBox(hWnd, "Error saving disabled module state", NULL, MB_ICONWARNING);
   } else {
      int i;
      for(i = 0; i < *theNOfModules; ++i) {
         if (theModList[i].Disabled) {
            fprintf(fp, "%s\n", &theModList[i].description);
         }
      }
      fclose(fp);
   }
}

/*************************************************
 * Loads module names that should be disabled 
 */
int loadDisabledModules(disModules* theDisList) 
{
   char dummy[81];
   FILE* fp;
   int nOfDisMod = 0;
   
   if(fp = fopen(vwDisabled, "r")) {
      while(!feof(fp)) {
         fgets(dummy, 80, fp);
         // Remove the newline
         if(dummy[strlen(dummy) - 1] == '\n') {
            dummy[strlen(dummy) - 1] = '\0';
         }
         if(nOfDisMod < (MAXMODULES * 2) && (strlen(dummy) != 0)) {
            strcpy(theDisList[nOfDisMod].moduleName, dummy);
            nOfDisMod++;
         } 
      }
      fclose(fp);
   } 
   return nOfDisMod;
}

/*************************************************
 * Loads window classnames from sticky file
 */
int loadStickyList(stickyType* theStickyList) 
{
   char dummy[80];
   FILE* fp;
   int nOfSticky = 0;
   
   if(fp = fopen(vwSticky, "r")) {
      while(!feof(fp)) {
         fscanf(fp, "%79s", &dummy);
         char* theReplaceString = replace(dummy, "££", " ");
         strcpy(dummy, theReplaceString);
         free(theReplaceString);
         if((strlen(dummy) != 0) && !feof(fp)) {
            theStickyList[nOfSticky].winClassName = malloc(sizeof(char) * strlen(dummy) + 1);
            strcpy(theStickyList[nOfSticky].winClassName, dummy);
            nOfSticky++;
         }
      }
      fclose(fp);
   }
   return nOfSticky;
}

/*************************************************
 * Writes down the classnames on the sticky windows on file
 */
void saveStickyWindows(int* theNOfWin, windowType* theWinList)
{
   char className[80];
   FILE* fp;
    
   if(!(fp = fopen(vwSticky, "w"))) {
      MessageBox(hWnd, "Error writing sticky file", NULL, MB_ICONWARNING);
   } else {
      int i;
      for(i = 0; i < *theNOfWin; ++i) {
         if (theWinList[i].Sticky) {
            GetClassName(theWinList[i].Handle, className, 79);
            char* theReplaceString = replace(className, " ", "££");
            fprintf(fp, "%s\n",  theReplaceString);
            free(theReplaceString);

         }
      }
      fclose(fp);
   }
}

/*************************************************
 * Loads window classnames from tricky file
 */
int loadTrickyList(stickyType* theTrickyList) 
{
   char dummy[80];
   FILE* fp;
   int nOfTricky = 0;
   
   if(fp = fopen(vwTricky, "r")) 
   {
      while(!feof(fp)) {
         fscanf(fp, "%79s", &dummy); 
         char* theReplaceString = replace(dummy, "££", " ");
         strcpy(dummy, theReplaceString);
         free(theReplaceString);
         if( (strlen(dummy) != 0) && !feof(fp) ) 
         {
            theTrickyList[nOfTricky].winClassName = malloc( sizeof(char) * strlen(dummy) + 1 );
            strcpy( theTrickyList[nOfTricky].winClassName, dummy );
            nOfTricky++;
         }
      }
      fclose(fp);
   }
   return nOfTricky;
}

/*************************************************
 * Writes down the classnames on the tricky windows on file
 */
void saveTrickyWindows(int* theNOfWin, windowType* theWinList)
{
/*
   char className[80];
   FILE* fp;
    
   if(!(fp = fopen(vwTricky, "w"))) {
      MessageBox(hWnd, "Error writing tricky file", NULL, MB_ICONWARNING);
   } else {
      int i;
      for(i = 0; i < *theNOfWin; ++i) {
         if (theWinList[i].Sticky) {
            GetClassName(theWinList[i].Handle, className, 79);
            char* theReplaceString = replace(className, " ", "££");
            fprintf(fp, "%s\n",  theReplaceString);
            free(theReplaceString);
         }
      }
      fclose(fp);
   }
*/
}

/*************************************************
 * Writes down the classnames of the windows currently in list
 * This method is used for the crash recovery routines, should not be 
 * mixed up with saveDesktopConfiguration()
 */
void saveDesktopState(int* theNOfWin, windowType* theWinList)
{
   char className[51];
   FILE* fp;
    
   if(!(fp = fopen(vwState, "wc"))) {
      MessageBox(hWnd, "Error writing state file", NULL, MB_ICONWARNING);
   } else {
      int i;
      for(i = 0; i < *theNOfWin; ++i) {
         GetClassName(theWinList[i].Handle, className, 50);
         char* theReplaceString = replace(className, " ", "££");
         fprintf(fp, "%s\n",  theReplaceString);
         free(theReplaceString);
      }
      fflush(fp); // Make sure the file is physically written to disk
      fclose(fp);
   }
}

/*************************************************
 * Writes down the current desktop layout to a file
 * 
 */
void saveDesktopConfiguration(int* theNOfWin, windowType* theWinList)
{
   char className[51];
   FILE* fp;
    
   if(!(fp = fopen(vwWindowsState, "w"))) {
      MessageBox(hWnd, "Error writing desktop configuration file", NULL, MB_ICONWARNING);
   } else {
      int i;
      for(i = 0; i < *theNOfWin; ++i) {
         GetClassName(theWinList[i].Handle, className, 50);
         char* theReplaceString = replace(className, " ", "££");
         fprintf(fp, "%s %d\n", theReplaceString, theWinList[i].Desk);
         free(theReplaceString);
      }
      fclose(fp);
   }
}

/*************************************************
 * Loads the list with classnames that has an desktop assigned
 */
int loadAssignedList(assignedType* theAssignList) 
{
   char dummy[51];
   FILE* fp;
   int curAssigned = 0;
   
   if(fp = fopen(vwWindowsState, "r")) {
      while(!feof(fp)) {
         fscanf(fp, "%s%i", &dummy, &theAssignList[curAssigned].desktop);
         char* theReplaceString = replace(dummy, "££", " ");
         strcpy(dummy, theReplaceString);
         free(theReplaceString);
         if((strlen(dummy) != 0) && !feof(fp)) {
            theAssignList[curAssigned].winClassName = malloc(sizeof(char) * strlen(dummy) + 1);
            strcpy(theAssignList[curAssigned].winClassName, dummy);
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
   char dummy[100];
   FILE* fp;
   int curUser = 0;
   
   if(fp = fopen(vwList, "r")) {
      while(!feof(fp)) {
         fgets(dummy, 99, fp);
         // Remove the newline
         if(dummy[strlen(dummy) - 1] == '\n') {
            dummy[strlen(dummy) - 1] = '\0';
         }
         if(curUser < MAXUSER && dummy[0] != ':' && (strlen(dummy) != 0) && !feof(fp)) {
            theUserList[curUser].winNameClass = malloc(sizeof(char) * strlen(dummy) + 1);
            strcpy(theUserList[curUser].winNameClass, dummy);
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
void writeConfig()
{
   FILE* fp;

   if((fp = fopen(vwConfig, "w")) == NULL) {
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
      fprintf(fp, "Hot_key_1# %i\n", hotkey1);
      fprintf(fp, "Hot_key_Mod1# %i\n", hotkey1Mod);
      fprintf(fp, "Hot_key_Win1# %i\n", hotkey1Win);
      fprintf(fp, "Hot_key_2# %i\n", hotkey2);
      fprintf(fp, "Hot_key_Mod2# %i\n", hotkey2Mod);
      fprintf(fp, "Hot_key_Win2# %i\n", hotkey2Win);
      fprintf(fp, "Hot_key_3# %i\n", hotkey3);
      fprintf(fp, "Hot_key_Mod3# %i\n", hotkey3Mod);
      fprintf(fp, "Hot_key_Win3# %i\n", hotkey3Win);
      fprintf(fp, "Hot_key_4# %i\n", hotkey4);
      fprintf(fp, "Hot_key_Mod4# %i\n", hotkey4Mod);
      fprintf(fp, "Hot_key_Win4# %i\n", hotkey4Win);
      fprintf(fp, "Hot_key_5# %i\n", hotkey5);
      fprintf(fp, "Hot_key_Mod5# %i\n", hotkey5Mod);
      fprintf(fp, "Hot_key_Win5# %i\n", hotkey5Win);
      fprintf(fp, "Hot_key_6# %i\n", hotkey6);
      fprintf(fp, "Hot_key_Mod6# %i\n", hotkey6Mod);
      fprintf(fp, "Hot_key_Win6# %i\n", hotkey6Win);
      fprintf(fp, "Hot_key_7# %i\n", hotkey7);
      fprintf(fp, "Hot_key_Mod7# %i\n", hotkey7Mod);
      fprintf(fp, "Hot_key_Win7# %i\n", hotkey7Win);
      fprintf(fp, "Hot_key_8# %i\n", hotkey8);
      fprintf(fp, "Hot_key_Mod8# %i\n", hotkey8Mod);
      fprintf(fp, "Hot_key_Win8# %i\n", hotkey8Win);
      fprintf(fp, "Hot_key_9# %i\n", hotkey9);
      fprintf(fp, "Hot_key_Mod9# %i\n", hotkey9Mod);
      fprintf(fp, "Hot_key_Win9# %i\n", hotkey9Win);
      fprintf(fp, "Mouse_control_key_support# %i\n", useMouseKey);
      fprintf(fp, "Mouse_key_alt# %i\n", mouseModAlt);
      fprintf(fp, "Mouse_key_shift# %i\n", mouseModShift);
      fprintf(fp, "Mouse_key_ctrl# %i\n", mouseModCtrl);
      fprintf(fp, "Save_sticky_info# %i\n", saveSticky);
      fprintf(fp, "Refresh_after_warp# %i\n", refreshOnWarp);
      fprintf(fp, "No_mouse_wrap# %i\n", noMouseWrap);
      fprintf(fp, "Sticky_modifier# %i\n", VW_STICKYMOD);
      fprintf(fp, "Sticky_key# %i\n", VW_STICKY);
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
      fprintf(fp, "Sticky_Win# %i\n", VW_STICKYWIN);
      fprintf(fp, "Taskbar_detection# %i\n", noTaskbarCheck);
      fprintf(fp, "Use_trickywindows# %i\n", trickyWindows);

      fclose(fp);
   }
}

/*************************************************
 * Reads a saved configuration from file
 */
void readConfig()
{
   char dummy[80];
   FILE* fp;

   if((fp = fopen(vwConfig, "r")) == NULL) {
      MessageBox(NULL, "Error reading config file. This is probably due to new user setup.\nA new config file will be created.", NULL, MB_ICONWARNING);
      // Try to create new file
      if((fp = fopen(vwConfig, "w")) == NULL) {
         MessageBox(NULL, "Error writing new config file. Check writepermissions.", NULL, MB_ICONWARNING);
      }
   } else {   
      fscanf(fp, "%s%i", &dummy, &mouseEnable);
      fscanf(fp, "%s%i", &dummy, &configMultiplier);
      fscanf(fp, "%s%i", &dummy, &keyEnable);
      fscanf(fp, "%s%i", &dummy, &releaseFocus);
      fscanf(fp, "%s%i", &dummy, &keepActive);
      fscanf(fp, "%s%i", &dummy, &modAlt);
      fscanf(fp, "%s%i", &dummy, &modShift);
      fscanf(fp, "%s%i", &dummy, &modCtrl);
      fscanf(fp, "%s%i", &dummy, &modWin);
      fscanf(fp, "%s%i", &dummy, &warpLength);
      fscanf(fp, "%s%i", &dummy, &minSwitch);
      fscanf(fp, "%s%i", &dummy, &taskBarWarp);
      fscanf(fp, "%s%i", &dummy, &nDesksY);
      fscanf(fp, "%s%i", &dummy, &nDesksX);
      fscanf(fp, "%s%i", &dummy, &hotKeyEnable);
      fscanf(fp, "%s%i", &dummy, &hotkey1);
      fscanf(fp, "%s%i", &dummy, &hotkey1Mod);
      fscanf(fp, "%s%i", &dummy, &hotkey1Win);
      fscanf(fp, "%s%i", &dummy, &hotkey2);
      fscanf(fp, "%s%i", &dummy, &hotkey2Mod);
      fscanf(fp, "%s%i", &dummy, &hotkey2Win);
      fscanf(fp, "%s%i", &dummy, &hotkey3);
      fscanf(fp, "%s%i", &dummy, &hotkey3Mod);
      fscanf(fp, "%s%i", &dummy, &hotkey3Win);
      fscanf(fp, "%s%i", &dummy, &hotkey4);
      fscanf(fp, "%s%i", &dummy, &hotkey4Mod);
      fscanf(fp, "%s%i", &dummy, &hotkey4Win);
      fscanf(fp, "%s%i", &dummy, &hotkey5);
      fscanf(fp, "%s%i", &dummy, &hotkey5Mod);
      fscanf(fp, "%s%i", &dummy, &hotkey5Win);
      fscanf(fp, "%s%i", &dummy, &hotkey6);
      fscanf(fp, "%s%i", &dummy, &hotkey6Mod);
      fscanf(fp, "%s%i", &dummy, &hotkey6Win);
      fscanf(fp, "%s%i", &dummy, &hotkey7);
      fscanf(fp, "%s%i", &dummy, &hotkey7Mod);
      fscanf(fp, "%s%i", &dummy, &hotkey7Win);
      fscanf(fp, "%s%i", &dummy, &hotkey8);
      fscanf(fp, "%s%i", &dummy, &hotkey8Mod);
      fscanf(fp, "%s%i", &dummy, &hotkey8Win);
      fscanf(fp, "%s%i", &dummy, &hotkey9);
      fscanf(fp, "%s%i", &dummy, &hotkey9Mod);
      fscanf(fp, "%s%i", &dummy, &hotkey9Win);
      fscanf(fp, "%s%i", &dummy, &useMouseKey);
      fscanf(fp, "%s%i", &dummy, &mouseModAlt);
      fscanf(fp, "%s%i", &dummy, &mouseModShift);
      fscanf(fp, "%s%i", &dummy, &mouseModCtrl);
      fscanf(fp, "%s%i", &dummy, &saveSticky);
      fscanf(fp, "%s%i", &dummy, &refreshOnWarp);
      fscanf(fp, "%s%i", &dummy, &noMouseWrap);
      fscanf(fp, "%s%i", &dummy, &VW_STICKYMOD);
      fscanf(fp, "%s%i", &dummy, &VW_STICKY);
      fscanf(fp, "%s%i", &dummy, &crashRecovery);
      fscanf(fp, "%s%i", &dummy, &deskWrap);
      fscanf(fp, "%s%i", &dummy, &invertY);
      fscanf(fp, "%s%i", &dummy, &stickyMenu);
      fscanf(fp, "%s%i", &dummy, &assignMenu);
      fscanf(fp, "%s%i", &dummy, &directMenu);
      fscanf(fp, "%s%i", &dummy, &useDeskAssignment);
      fscanf(fp, "%s%i", &dummy, &saveLayoutOnExit);
      fscanf(fp, "%s%i", &dummy, &assignOnlyFirst);
      fscanf(fp, "%s%i", &dummy, &cyclingKeysEnabled);
      fscanf(fp, "%s%i", &dummy, &hotCycleUp);
      fscanf(fp, "%s%i", &dummy, &hotCycleUpMod);
      fscanf(fp, "%s%i", &dummy, &hotCycleDown);
      fscanf(fp, "%s%i", &dummy, &hotCycleDownMod);
      fscanf(fp, "%s%i", &dummy, &hotkeyMenuEn);
      fscanf(fp, "%s%i", &dummy, &hotkeyMenu);
      fscanf(fp, "%s%i", &dummy, &hotkeyMenuMod);
      fscanf(fp, "%s%i", &dummy, &hotkeyMenuWin);
      fscanf(fp, "%s%i", &dummy, &displayTaskbarIcon);
      fscanf(fp, "%s%i", &dummy, &VW_STICKYWIN);
      fscanf(fp, "%s%i", &dummy, &noTaskbarCheck);
      fscanf(fp, "%s%i", &dummy, &trickyWindows);

      fclose(fp);
   }
}

/*************************************************
 * Replaces parts of a string with a new string
 */
char* replace(char *g_string, char *replace_from, char *replace_to)
{
   char *p, *p1, *return_str;
   int  i_diff;

   // the margin between the replace_from and replace_to;
   i_diff=strlen(replace_from) - strlen(replace_to); 
   return_str = (char*) malloc(strlen(g_string)+1); // Changed line

   if(return_str == NULL) 
      return g_string;
   return_str[0] = 0;

   p = g_string;

   for( ;; ) 
   {
      p1 = p;		           // old position
      p = strstr(p, replace_from); // next position
      if(p == NULL) 
      {
         strcat(return_str, p1);
         break;
      }
      while(p > p1) 
      {
         sprintf(return_str, "%s%c", return_str, *p1);
         p1++;
      }
      if(i_diff > 0)
      {
         // the changed length can be larger than _MAXLENGTH_(1000);
         return_str = (char*)realloc(return_str,strlen(g_string) + i_diff+1); 
         if (return_str == NULL) 
            return g_string;
      }
      strcat(return_str, replace_to);
      p += strlen(replace_from);	// new point position
   }
   return return_str;
}

/*************************************************
 * Check if we have a previous lock file, otherwise it creates it
 */
BOOL tryToLock()
{
   if(access(vwLock, 0 == -1)) {
      FILE* fp;
      if(!(fp = fopen(vwLock, "wc"))) {
         MessageBox(hWnd, "Error writing lock file", "VirtuaWin", MB_ICONWARNING);
         return TRUE;
      } else {
         fprintf(fp, "%s", "VirtuaWin LockFile");
      }
    
      fflush(fp); // Make sure the file is physically written to disk
      fclose(fp);
        
      return TRUE;
   } else {
      return FALSE; // We already had a lock file, probably due to a previous crash
   }
}

/*
 * $Log$
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
