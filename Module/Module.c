/*
 * Simple module skeleton for VirtuaWin. These are the minimal requirements.
 * It is a simple application with a hidden window that receives messages from virtuawin
 * Look in Messages.h to see what can be sent to and from VirtuaWin
 * 
 * Note that the classname must be the same as the filename including the '.exe'
 *
 * Created by jopi 1999
 * VirtuaWin@home.se
 * Feel free to use in any way you like(well, almost)
 */
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "Messages.h"

HINSTANCE hInst;   // Instance handle
HWND hwndMain;	   // Main window handle
HWND vwHandle;     // Handle to VirtuaWin

/* Main message handler */
LRESULT CALLBACK MainWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

/* Initializes the window */ 
static BOOL InitApplication(void)
{
  WNDCLASS wc;

  memset(&wc, 0, sizeof(WNDCLASS));
  wc.style = 0;
  wc.lpfnWndProc = (WNDPROC)MainWndProc;
  wc.hInstance = hInst;
  /* IMPORTANT! The classname must be the same as the filename since VirtuaWin uses 
     this for locating the window */
  wc.lpszClassName = "module.exe";
  
  if (!RegisterClass(&wc))
    return 0;
  
  return 1;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg) {
          
  case MOD_INIT: // This must be taken care of in order to get the handle to VirtuaWin. 
    // The handle to VirtuaWin comes in the wParam 
    vwHandle = (HWND) wParam; // Should be some error handling here if NULL 
    break;
  case MOD_QUIT: // This must be handeled, otherwise VirtuaWin can't shut down the module 
    PostQuitMessage(0);
    break;
  case MOD_SETUP: // Optional
    MessageBox(vwHandle, "No setup!", "Plugin", 0);
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
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
  MSG msg;
  hInst = hInstance;
  if (!InitApplication())
    return 0;
  
  // In this example, the window is never shown
  if ((hwndMain = CreateWindow("module.exe", 
                               "module", 
                               WS_POPUP,
                               CW_USEDEFAULT, 
                               0, 
                               CW_USEDEFAULT, 
                               0,
                               NULL,
                               NULL,
                               hInst,
                               NULL)) == (HWND)0)
    return 0;
  
  // main messge loop
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.wParam;
}

/*
 * $Log$
 * Revision 1.1.1.1  2000/06/03 15:44:50  jopi
 * Added first time
 *
 */
