#ifndef _MESSAGES_H_
#define _MESSAGES_H_
/*************************************************************************
 * This is a definition of all possible messages to send to VirtuaWin 
 * and the messages that VirtuaWin uses for module communication
 *
 * Created by jopi 1999
 * VirtuaWin@home.se
 * Feel free to use in any way you like
 *************************************************************************
 * All messages starting with VW_ is for controlling VirtuaWin 
 * and messages starting with MOD_ is messages sent by VirtuaWin 
 * 
 * For example if you want to step one desktop to the left:
 * PostMessage(VirtuaWin HWND, VW_CHANGEDESK, VW_STEPLEFT, 0);
 * For messages where you expect a return value, use the SendMessage
 * function instead, see some win32 documentation for more info.
 *************************************************************************/

/* Message, switch to a specified desktop, sent with following lParam or 1..9 */
#define VW_CHANGEDESK  (WM_USER + 10) 
#define VW_STEPLEFT    (WM_USER + 11) // lParam
#define VW_STEPRIGHT   (WM_USER + 12) // lParam
#define VW_STEPUP      (WM_USER + 13) // lParam
#define VW_STEPDOWN    (WM_USER + 14) // lParam
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
   wParam is the window handle (HWND) and lParam is the desktop number */
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

/* Message, sent by VirtuaWin after a switch. lParam will contain current desktop number 
   if wParam isn't one of the following, then wParam will also contain current desktop.
   If desktop cycling is enabled, there will be two MOD_CHANGEDESK sent when going 
   "over the edge". The first one will have a MOD_STEP* parameter, and the second
   will have current desktop in both parameters.
*/
#define MOD_CHANGEDESK (WM_USER + 30) 
#define MOD_STEPLEFT   (WM_USER + 31) // wParam
#define MOD_STEPRIGHT  (WM_USER + 32) // wParam
#define MOD_STEPUP     (WM_USER + 33) // wParam
#define MOD_STEPDOWN   (WM_USER + 34) // wParam

/* Message, sent just after the module is started. wParam will contain VirtuaWin hWnd */ 
#define MOD_INIT      (WM_USER + 35)
/* Message, sent when VirtuaWin quits or reloads its modules */
#define MOD_QUIT      (WM_USER + 36)
/* Message, sent by VirtuaWin when setup button is pressed in "module tab" */
#define MOD_SETUP     (WM_USER + 37)
/* Message, sent by VirtuaWin when the configuration has been updated */
#define MOD_CFGCHANGE (WM_USER + 38)

#endif

/*
 * $Log$
 * Revision 1.7  2004/02/28 23:50:26  jopi
 * SF905625 Added module message for changing the sticky state of a window
 *
 * Revision 1.6  2003/11/05 15:46:34  jopi
 * Better description of ASSIGNWIN
 *
 * Revision 1.5  2003/06/26 19:56:52  jopi
 * Added module support for assigning a window to specified desktop
 *
 * Revision 1.4  2001/11/12 21:39:15  jopi
 * Added functionality for disabling the systray icon
 *
 * Revision 1.3  2001/01/12 16:58:11  jopi
 * Added module message for getting the current desktop number
 *
 * Revision 1.2  2000/07/18 16:03:34  jopi
 * Changed mail adress in error message
 *
 * Revision 1.1.1.1  2000/06/03 15:38:05  jopi
 * Added first time
 *
 */
