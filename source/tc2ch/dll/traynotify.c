/*-------------------------------------------
  traynotify.c
    Customize the tray in taskbar
    Kazubon 2001
---------------------------------------------*/
#include "tcdll.h"



LRESULT CALLBACK WndProcTrayNotify(HWND, UINT, WPARAM, LPARAM);
void InitTrayNotify(HWND hwnd);
void EndTrayNotify(void);
//BOOL bFillTray = FALSE;
//BOOL bSkinTray = FALSE;
static HWND hwndTrayNotify = NULL, hwndToolbar = NULL;
static WNDPROC oldWndProcTrayNotify = NULL;
static LONG_PTR oldClassStyleTrayNotify;
static LONG_PTR oldStyleTrayNotify;
static LONG_PTR oldExStyleTrayNotify;
//static BOOL bFlatTray = TRUE;
static HWND s_hwndClock;

/*--------------------------------------------------
  initialize
----------------------------------------------------*/
void InitTrayNotify(HWND hwndClock)
{
	HWND hwnd, hwnd2;
	char classname[80];

	EndTrayNotify();

	//bFillTray = GetMyRegLong(NULL, "FillTray", FALSE);
	//bFlatTray = GetMyRegLong(NULL, "FlatTray", TRUE);
	//bSkinTray = GetMyRegLong(NULL, "SkinTray", FALSE);

//	if(bSkinTray) bFillTray = FALSE; // ‚¿‚å‚Á‚Æ‹­ˆø‚ÉÝ’è‚Ì–µ‚‚ð‚È‚­‚·

	//if(!bFillTray && !bFlatTray && !bSkinTray) return ;
	//if (!bFillTray && !bFlatTray) return;

	return;

	// get window handle of TrayNotifyWnd
	hwndTrayNotify = GetParent(hwndClock);  // TrayNotifyWnd

	// search toolbar
	//if(bFillTray)
	//{
	//	hwndToolbar = NULL;
	//	hwnd = GetWindow(hwndTrayNotify, GW_CHILD);
	//	while(hwnd)
	//	{
	//		GetClassName(hwnd, classname, 80);
	//		if(lstrcmpi(classname, "ToolbarWindow32") == 0)
	//		{
	//			hwndToolbar = hwnd;
	//			break;
	//		}else if(lstrcmpi(classname, "SysPager") == 0)
	//		{
	//			hwnd2 = GetWindow(hwnd, GW_CHILD);
	//			while(hwnd2)
	//			{
	//				GetClassName(hwnd2, classname, 80);
	//				if(lstrcmpi(classname, "ToolbarWindow32") == 0)
	//				{
	//					hwndToolbar = hwnd2;
	//					break;
	//				}
	//			}
	//			hwnd2 = GetWindow(hwnd2, GW_HWNDNEXT);

	//		}
	//		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	//	}
	//	if(hwndToolbar == NULL)
	//	{
	//		bFillTray = FALSE;
	//	}
	//}

	//if(bFillTray)
	//{
	//	s_hwndClock = hwndClock;

	//	oldClassStyleTrayNotify = GetClassLongPtr(hwndTrayNotify, GCL_STYLE);
	//	SetClassLongPtr(hwndTrayNotify, GCL_STYLE,
	//		oldClassStyleTrayNotify|CS_HREDRAW|CS_VREDRAW);

	//	oldWndProcTrayNotify =
	//		(WNDPROC)GetWindowLongPtr(hwndTrayNotify, GWLP_WNDPROC);
	//	SubclassWindow(hwndTrayNotify, WndProcTrayNotify);

	//	oldStyleTrayNotify = GetWindowLongPtr(hwndTrayNotify, GWL_STYLE);
	//	SetWindowLongPtr(hwndTrayNotify, GWL_STYLE,
	//		oldStyleTrayNotify & ~(WS_CLIPCHILDREN|WS_CLIPSIBLINGS));
	//}

	//if(bFlatTray)
	//{
	//	oldExStyleTrayNotify = GetWindowLongPtr(hwndTrayNotify, GWL_EXSTYLE);
	//	SetWindowLongPtr(hwndTrayNotify, GWL_EXSTYLE,
	//		oldExStyleTrayNotify & ~WS_EX_STATICEDGE);
	//	SetWindowPos(hwndTrayNotify, NULL, 0, 0, 0, 0,
	//		SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
	//}

	//if(bFillTray || bFlatTray)
	//	InvalidateRect(hwndTrayNotify, NULL, TRUE);
//	if(bFillTray)
//		SendMessage(hwndToolbar, WM_SYSCOLORCHANGE, 0, 0);
	//if(bFillTray)
	//{
	//		SendMessage(GetParent(hwndToolbar), WM_SYSCOLORCHANGE, 0, 0);
	//}


}

/*--------------------------------------------------
  undo
----------------------------------------------------*/
void EndTrayNotify(void)
{
	//if(bFillTray && hwndTrayNotify && IsWindow(hwndTrayNotify))
	if (hwndTrayNotify && IsWindow(hwndTrayNotify))
	{
		SetWindowLongPtr(hwndTrayNotify, GWL_STYLE, oldStyleTrayNotify);
		if(oldWndProcTrayNotify)
			SubclassWindow(hwndTrayNotify, oldWndProcTrayNotify);

		SetClassLongPtr(hwndTrayNotify, GCL_STYLE, oldClassStyleTrayNotify);

		InvalidateRect(hwndTrayNotify, NULL, TRUE);
		SendMessage(hwndToolbar, WM_SYSCOLORCHANGE, 0, 0);
		InvalidateRect(hwndToolbar, NULL, TRUE);
	}

	//if(bFlatTray && hwndTrayNotify && IsWindow(hwndTrayNotify))
	if (hwndTrayNotify && IsWindow(hwndTrayNotify))
	{
		SetWindowLongPtr(hwndTrayNotify, GWL_EXSTYLE, oldExStyleTrayNotify);
		SetWindowPos(hwndTrayNotify, NULL, 0, 0, 0, 0,
			SWP_DRAWFRAME|SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);
	}

	hwndTrayNotify = NULL;
	hwndToolbar = NULL;
	oldWndProcTrayNotify = NULL;
	//bFillTray = bFlatTray = bSkinTray = FALSE;

}

extern HDC hdcClock;

/*------------------------------------------------
   subclass procedure of TrayNotifyWnd
--------------------------------------------------*/
LRESULT CALLBACK WndProcTrayNotify(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_ERASEBKGND:
		{
			RECT rc;
			//if(bNoClock) break;
			GetClientRect(hwnd, &rc);
			FillClock(hwnd, (HDC)wParam, &rc, 0);
			return 1;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			RECT rc;
			//if(bNoClock) break;
			hdc = BeginPaint(hwnd, &ps);
			GetClientRect(hwnd, &rc);
			FillClock(hwnd, hdc, &rc, 0);

			EndPaint(hwnd, &ps);
			return 0;
		}

		case WM_SIZE:
			//if(bNoClock) break;
			SendMessage(s_hwndClock, WM_SIZE, 0, 0);
			break;

		case WM_NOTIFY:
		{
			LPNMHDR pnmh;
			//if(bNoClock) break;
			pnmh = (LPNMHDR)lParam;
			if (pnmh->code == NM_CUSTOMDRAW && pnmh->idFrom == 0)
			{
				LPNMCUSTOMDRAW pnmcd;
				pnmcd = (LPNMCUSTOMDRAW)lParam;
				if (pnmcd->dwDrawStage  == CDDS_ITEMPREPAINT
					&& hdcClock != NULL)
				{
					POINT ptTray, ptToolbar;
					int x, y;
					ptTray.x = ptTray.y = 0;
					ClientToScreen(hwnd, &ptTray);
					ptToolbar.x = ptToolbar.y = 0;
					ClientToScreen(pnmh->hwndFrom, &ptToolbar);
					x = ptToolbar.x - ptTray.x;
					y = ptToolbar.y - ptTray.y;
					BitBlt(pnmcd->hdc, pnmcd->rc.left, pnmcd->rc.top,
						pnmcd->rc.right - pnmcd->rc.left,
						pnmcd->rc.bottom - pnmcd->rc.top,
						hdcClock, x + pnmcd->rc.left, y + pnmcd->rc.top,
						SRCCOPY);
				}
			}
			break;
		}
	}
	return CallWindowProc(oldWndProcTrayNotify, hwnd, message, wParam, lParam);
}
