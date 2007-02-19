#ifndef _MESSAGES_H_
#define _MESSAGES_H_
/*************************************************************************
 *
 *  VirtuaWin - Virtual Desktop Manager (virtuawin.sourceforge.net)
 *  ConfigParameters.h - Dfinition of all module messages
 * 
 *  Copyright (c) 1999-2005 Johan Piculell
 *  Copyright (c) 2006 VirtuaWin (VirtuaWin@home.se)
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
 * This is a definition of all possible messages to send to VirtuaWin 
 * and the messages that VirtuaWin uses for module communication
 *
 * All messages starting with VW_ is for controlling VirtuaWin 
 * and messages starting with MOD_ is messages sent by VirtuaWin 
 * 
 * For example if you want to step one desktop to the left:
 * PostMessage(VirtuaWin HWND, VW_CHANGEDESK, VW_STEPLEFT, 0);
 * For messages where you expect a return value, use the SendMessage
 * function instead, see some win32 documentation for more info.
 * 
 * Note: the message numbers is not all in sequence!
 * 
 *************************************************************************/

/* Message, switch to a specified desktop, sent with following lParam or 1..9 */
#define VW_CHANGEDESK  (WM_USER + 10) 
#define VW_STEPLEFT    (WM_USER + 11)
#define VW_STEPRIGHT   (WM_USER + 12)
#define VW_STEPUP      (WM_USER + 13)
#define VW_STEPDOWN    (WM_USER + 14)
/* Message, close VirtuaWin */
#define VW_CLOSE       (WM_USER + 15) 
/* Message, display setup dialog */
#define VW_SETUP       (WM_USER + 16) 
/* Message, remove the systray icon */
#define VW_DELICON     (WM_USER + 17) 
/* Message, displays the systray icon */
#define VW_SHOWICON    (WM_USER + 18) 
/* Message, bring up the help */ 
#define VW_HELP        (WM_USER + 19) 
/* Message, gather all windows */
#define VW_GATHER      (WM_USER + 20) 
/* Message, retuns desktop width */
#define VW_DESKX       (WM_USER + 21)
/* Message, retuns desktop height */
#define VW_DESKY       (WM_USER + 22)
/* Message, require the window list from VirtuaWin. List will be returned with WM_COPYDATA */
#define VW_WINLIST     (WM_USER + 23)
/* Message, returns the current desktop number */
#define VW_CURDESK     (WM_USER + 24)
/* Message, assign a window to the specified desktop 
   wParam is the window handle (HWND) and lParam is the desktop number.
   If desk is -ve window is assigned to desk (-lParam) and Vw change to the desk. 
   Returns 0 if window was not found (i.e. not managed by VW), non-zero otherwise */
#define VW_ASSIGNWIN   (WM_USER + 25)
/* Message, set the sticky state of a window. wParam is the 
   window handle (HWND) and lParam should be -1 for toggle, 0 for unset
   and 1 for set sticky state. */
#define VW_SETSTICKY   (WM_USER + 26)
/* Message, make a window the foreground, only if visible 
   wParam is the window handle (HWND) */
#define VW_FOREGDWIN   (WM_USER + 27)
/* Message, return VirtuaWin's installation path. List will be returned with WM_COPYDATA */
#define VW_INSTALLPATH (WM_USER + 28)
/* Message, return VirtuaWin's user path. List will be returned with WM_COPYDATA */
#define VW_USERAPPPATH (WM_USER + 29)
/* Message, access a window, wParam is the window handle (HWND) and lParam is the method:
            0 - Use user's 'On hidden window activation' preference (ignore -> move)
            1 - Move window to this desk
            2 - Copy window to this disk
            3 - Change to window's desk
   Returns 0 if window was not found (i.e. not managed by VW), non-zero otherwise */
#define VW_ACCESSWIN   (WM_USER + 39)
/* Message, return the desk number of a window, 0 if not found (i.e. not managed) */
#define VW_GETWINDESK  (WM_USER + 40)
/* Message, Desk image generation message, the action of the message depends on the
 * value of wParam:
 *   0 - Returns the current status of image generation
 *   1 - Enables image generation, lParam specifies the required image height
 *       (aspect ratio is maintained). Returns 1 if generation is enabled.
 *   2 - Disables image generation. Note: as multiple modules may enable image
 *       generation an 'enable' counter is kept, each 'enable' message increments
 *       and each disable decrements this counter. Image generation only stops 
 *       when the counter returns to zero. Returns 1 if counter was decremented,
 *       0 if counter is already 0.
 *   3 - Updates the current desk's image if image generation is enabled, returns
 *       1 if successfully updated, 0 otherwise.
 *   4 - Returns the desk image height if enabled, 0 otherwise 
 *   5 - Returns the desk image width if enabled, 0 otherwise */ 
#define VW_DESKIMAGE   (WM_USER + 41)


/* Message, sent by VirtuaWin after a switch. lParam will contain current desktop number 
   if wParam isn't one of the following, then wParam will also contain current desktop.
   If desktop cycling is enabled, there will be two MOD_CHANGEDESK sent when going 
   "over the edge". The first one will have a MOD_STEP* parameter, and the second
   will have current desktop in both parameters.
*/
#define MOD_CHANGEDESK (WM_USER + 30) 
#define MOD_STEPLEFT   (WM_USER + 31)
#define MOD_STEPRIGHT  (WM_USER + 32)
#define MOD_STEPUP     (WM_USER + 33)
#define MOD_STEPDOWN   (WM_USER + 34)

/* Message, sent just after the module is started. wParam will contain VirtuaWin hWnd */ 
#define MOD_INIT       (WM_USER + 35)
/* Message, sent when VirtuaWin quits or reloads its modules */
#define MOD_QUIT       (WM_USER + 36)
/* Message, sent by VirtuaWin when setup button is pressed in "module tab" */
#define MOD_SETUP      (WM_USER + 37)
/* Message, sent by VirtuaWin when the configuration has been updated */
#define MOD_CFGCHANGE  (WM_USER + 38)

#endif
