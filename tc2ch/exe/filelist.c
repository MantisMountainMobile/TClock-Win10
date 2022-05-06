/*--------------------------------------------------
   filelist.c
      ファイルリスト表示
   ◆FDQ3TClock 2003
----------------------------------------------------*/

#include "tclock.h"

#ifndef TPM_RECURSE
#define TPM_RECURSE         0x0001L
#endif

typedef struct _LISTIDL
{
	UINT uMenuID;
	char path[MAX_PATH];
	struct _LISTIDL *next;
} LISTIDL, *PLISTIDL;

typedef struct _ODMENUSTATE
{
	LOGFONT tLogFont;
	int iIconSizeX;
	int iIconSizeY;
	DWORD dwMenuBackColor;
	DWORD dwMenuBackTextColor;
} ODMENUSTATE, *PODMENUSTATE;

typedef struct _ODMENUITEM
{
	UINT uMenuID;
	HICON hMenuIcon;
	char sMenuStr[_MAX_FNAME];
	struct _ODMENUITEM *next;
} ODMENUITEM, *PODMENUITEM;

typedef struct _USERMENU
{
	HMENU hMenu;
	char path[MAX_PATH];
	UINT uID;
	UINT st_uID;
	BOOL flg;
	struct _USERMENU *next;
} USERMENU, *PUSERMENU;

// globals
ODMENUSTATE g_tODMenuState;
PODMENUITEM g_ptODMenuItem = NULL;
PUSERMENU   g_ptUserMenu = NULL;
PLISTIDL    g_ptListIDL = NULL;
UINT uItemID;
BOOL getODFlg = FALSE;

char *Unicode2Ansi(LPCWSTR psUnicode)
{
	int iLengthW = lstrlenW(psUnicode);
	char *psAnsi = (char *)malloc(iLengthW * 2 + 1);

	if(psAnsi != NULL){
		ZeroMemory(psAnsi, iLengthW * 2 + 1);
		WideCharToMultiByte(CP_ACP, 0, psUnicode, iLengthW,
							psAnsi, iLengthW * 2, NULL, NULL);
	}
	return psAnsi;
}

BOOL AddMenuItem(HMENU hMenu, UINT uItemID, UINT uType, char *sDisplayName, HMENU hSubMenu)
{
	MENUITEMINFO tMenuItemInfo;

	ZeroMemory(&tMenuItemInfo, sizeof(tMenuItemInfo));
	tMenuItemInfo.cbSize = sizeof(tMenuItemInfo);
	tMenuItemInfo.fMask = MIIM_ID | MIIM_TYPE | MIIM_SUBMENU;
	tMenuItemInfo.fType = uType;
	tMenuItemInfo.wID = uItemID;
	tMenuItemInfo.hSubMenu = hSubMenu;
	tMenuItemInfo.dwTypeData = sDisplayName;

	return InsertMenuItem(hMenu, uItemID, FALSE, &tMenuItemInfo);
}

void DestroyUserMenu(void)
{
	PLISTIDL    pListIDL  = g_ptListIDL,    pTmpListIDL;
	PUSERMENU   pUserMenu = g_ptUserMenu,   pTempUserMenu;
	PODMENUITEM pMenuItem = g_ptODMenuItem, pTempMenuItem;

	while(pUserMenu != NULL){
		DestroyMenu(pUserMenu->hMenu);
		pTempUserMenu = pUserMenu->next;
		free(pUserMenu);
		pUserMenu = pTempUserMenu;
	}

	while(pListIDL != NULL){
		pTmpListIDL = pListIDL->next;
		free(pListIDL);
		pListIDL = pTmpListIDL;
	}

	while(pMenuItem != NULL){
		pTempMenuItem = pMenuItem->next;
		free(pMenuItem);
		pMenuItem = pTempMenuItem;
	}

	g_ptODMenuItem = NULL;
	g_ptUserMenu   = NULL;
	g_ptListIDL    = NULL;
}

void GetODMenuState()
{
	NONCLIENTMETRICS tNCMetrics;

	ZeroMemory(&g_tODMenuState, sizeof(ODMENUSTATE));
	tNCMetrics.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &tNCMetrics, 0);

	g_tODMenuState.tLogFont = tNCMetrics.lfMenuFont;
	g_tODMenuState.iIconSizeX = GetSystemMetrics(SM_CXSMICON);
	g_tODMenuState.iIconSizeY = GetSystemMetrics(SM_CYSMICON);
	g_tODMenuState.dwMenuBackColor = GetSysColor(COLOR_HIGHLIGHT);
	g_tODMenuState.dwMenuBackTextColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
}

LPITEMIDLIST GetNextIDL(LPCITEMIDLIST pidl)
{
	LPSTR lpMem=(LPSTR)pidl;
	lpMem+=pidl->mkid.cb;
	return (LPITEMIDLIST)lpMem;
}

static UINT GetSize(LPCITEMIDLIST pidl)
{
	UINT cbTotal = 0;
	if (pidl)
	{
		cbTotal += sizeof(pidl->mkid.cb);
		while (pidl->mkid.cb)
		{
			cbTotal += pidl->mkid.cb;
			pidl = GetNextIDL(pidl);
		}
	}
	return cbTotal;
}

static LPITEMIDLIST Create(UINT cbSize)
{
	LPMALLOC pMalloc;
	HRESULT  hr;
	LPITEMIDLIST pIDL = NULL;

	hr=SHGetMalloc(&pMalloc);
	if (FAILED(hr))
		return 0;
	pIDL = (LPITEMIDLIST)pMalloc->lpVtbl->Alloc(pMalloc, cbSize);
	if (pIDL)
		_fmemset(pIDL, 0, cbSize);
	if (pMalloc)
		pMalloc->lpVtbl->Release(pMalloc);
	return pIDL;
}

LPITEMIDLIST ConcatPidls(LPCITEMIDLIST pIDL1, LPCITEMIDLIST pIDL2)
{
	LPITEMIDLIST pIDLNew;
	UINT cb1;
	UINT cb2;

	if (pIDL1)
		cb1 = GetSize(pIDL1) - sizeof(pIDL1->mkid.cb);
	else
		cb1 = 0;
	cb2 = GetSize(pIDL2);
	pIDLNew = Create(cb1 + cb2);
	if (pIDLNew)
	{
		if (pIDL1)
			memcpy(pIDLNew, pIDL1, cb1);
		memcpy(((LPSTR)pIDLNew) + cb1, pIDL2, cb2);
	}
	return pIDLNew;
}

BOOL AddUserMenu(HMENU hMenu, LPMALLOC pMalloc, LPCTSTR path)
{
	int nResult = 0;
	char itemName[1024], tmpPath[MAX_PATH];
	OLECHAR ochPath[MAX_PATH];
	STRRET strret;
	SHFILEINFO    tSHFileInfo;
	LPITEMIDLIST  pItemID, pItemIDFile, pItemIDFolder;
	LPSHELLFOLDER pFolder = NULL, pDesktop = NULL;
	LPENUMIDLIST  pEnumIDFolder = NULL, pEnumIDFile = NULL;
	PLISTIDL      *pptListIDL    = &g_ptListIDL;
	PUSERMENU     *ppUserMenu    = &g_ptUserMenu;
	PODMENUITEM   *pptODMenuItem = &g_ptODMenuItem;

	//構造体連結リストの最後までいく。
	while((*pptODMenuItem) != NULL)
		pptODMenuItem = &(*pptODMenuItem)->next;
	while((*pptListIDL) != NULL)
		pptListIDL = &(*pptListIDL)->next;
	while((*ppUserMenu) != NULL)
		ppUserMenu = &(*ppUserMenu)->next;

	if(SHGetDesktopFolder(&pFolder) != NOERROR)
		return FALSE;
	if(SHGetDesktopFolder(&pDesktop) != NOERROR)
	{
		pFolder->lpVtbl->Release(pFolder);
		return FALSE;
	}

	MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,path,-1,ochPath,MAX_PATH);

	if (pDesktop->lpVtbl->ParseDisplayName(pDesktop, NULL, NULL, ochPath, NULL, &pItemIDFolder, NULL)!=NOERROR)
	{
		pItemIDFolder = NULL;
		pFolder->lpVtbl->Release(pFolder);
		pDesktop->lpVtbl->Release(pDesktop);
		return FALSE;
	}

	if (pDesktop->lpVtbl->BindToObject(pDesktop, pItemIDFolder, 0, &IID_IShellFolder, (LPVOID *)&pFolder)!=NOERROR)
	{
		pFolder->lpVtbl->Release(pFolder);
		pDesktop->lpVtbl->Release(pDesktop);
		return FALSE;
	}
	pDesktop->lpVtbl->Release(pDesktop);

	//フォルダをすべて取得
	if(pFolder->lpVtbl->EnumObjects(pFolder, NULL, SHCONTF_FOLDERS, &pEnumIDFolder) != NOERROR)
	{
		pFolder->lpVtbl->Release(pFolder);
		return FALSE;
	}

	while(pEnumIDFolder->lpVtbl->Next(pEnumIDFolder, 1, &pItemIDFile, NULL) == NOERROR)
	{
		if(pFolder->lpVtbl->GetDisplayNameOf(pFolder, pItemIDFile, SHGDN_NORMAL, &strret) == NOERROR)
		{
			if(strret.uType == STRRET_CSTR)
				strcpy(itemName, strret.cStr);
			else if(strret.uType == STRRET_OFFSET)
				strcpy(itemName, (char *)pItemIDFile + strret.uOffset);
			else if(strret.uType == STRRET_WSTR)
			{
				char *pStr = Unicode2Ansi(strret.pOleStr);
				pMalloc->lpVtbl->Free(pMalloc, strret.pOleStr);
				if(pStr != NULL){
					strcpy(itemName, pStr);
					free(pStr);
				}
				else{
					nResult = -1;
					break;
				}
			}
			pItemID = ConcatPidls(pItemIDFolder,pItemIDFile);
			if(!(SHGetPathFromIDList(pItemID,tmpPath)))
			{
				nResult = -1;
				break;
			}

			*pptListIDL = (PLISTIDL)malloc(sizeof(LISTIDL));
			if(*pptListIDL == NULL)
			{
				nResult = -1;
				break;
			}
			else
			{
				if(g_ptListIDL == NULL)
					g_ptListIDL = *pptListIDL;
			}
			(*pptListIDL)->uMenuID = uItemID;
			(*pptListIDL)->next = NULL;
			strcpy((*pptListIDL)->path,tmpPath);

			*pptODMenuItem = (PODMENUITEM)malloc(sizeof(ODMENUITEM));
			ZeroMemory(*pptODMenuItem, sizeof(ODMENUITEM));
			if(*pptODMenuItem == NULL)
			{
				nResult = -1;
				break;
			}
			else
			{
				if(g_ptODMenuItem == NULL)
					g_ptODMenuItem = *pptODMenuItem;
			}
			(*pptODMenuItem)->uMenuID = uItemID;
			(*pptODMenuItem)->next = NULL;

			ZeroMemory(&tSHFileInfo, sizeof(tSHFileInfo));

			SHGetFileInfo((LPCTSTR)pItemID, 0, &tSHFileInfo, sizeof(tSHFileInfo),
							SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON);

			(*pptODMenuItem)->hMenuIcon = tSHFileInfo.hIcon;

			// サブメニュー作成
			*ppUserMenu = (PUSERMENU)malloc(sizeof(USERMENU));
			if(*ppUserMenu == NULL){
				nResult = -1;
				break;
			}
			else
			{
				if (g_ptUserMenu == NULL)
					g_ptUserMenu = *ppUserMenu;
			}
			(*ppUserMenu)->hMenu = CreatePopupMenu();
			if((*ppUserMenu)->hMenu == NULL)
			{
				nResult = -1;
				break;
			}
			(*ppUserMenu)->uID   = uItemID;
			(*ppUserMenu)->flg   = FALSE;
			(*ppUserMenu)->next  = NULL;
			strcpy((*ppUserMenu)->path,tmpPath);

			AddMenuItem((*ppUserMenu)->hMenu, uItemID, MFT_STRING, "フォルダを開く", NULL);
			AddMenuItem(hMenu, uItemID, MFT_OWNERDRAW, itemName, (*ppUserMenu)->hMenu);
			strcpy((*pptODMenuItem)->sMenuStr, itemName);
		}
		else
		{
			nResult = -1;
			break;
		}

		pMalloc->lpVtbl->Free(pMalloc, pItemID);
		pMalloc->lpVtbl->Free(pMalloc, pItemIDFile);
		pptListIDL    = &(*pptListIDL)->next;
		ppUserMenu    = &(*ppUserMenu)->next;
		pptODMenuItem = &(*pptODMenuItem)->next;
		uItemID++;
	}
	if(pEnumIDFolder != NULL){
		pEnumIDFolder->lpVtbl->Release(pEnumIDFolder);
	}

	if (path[0] != 0)
	{
		//隠しファイル以外のファイルのみ
		if(pFolder->lpVtbl->EnumObjects(pFolder, NULL, SHCONTF_NONFOLDERS, &pEnumIDFile) != NOERROR){
			pFolder->lpVtbl->Release(pFolder);
			return FALSE;
		}

		while(pEnumIDFile->lpVtbl->Next(pEnumIDFile, 1, &pItemIDFile, NULL) == NOERROR){
			if(pFolder->lpVtbl->GetDisplayNameOf(pFolder, pItemIDFile, SHGDN_NORMAL, &strret) == NOERROR)
			{
				if(strret.uType == STRRET_CSTR)
					strcpy(itemName, strret.cStr);
				else if(strret.uType == STRRET_OFFSET)
					strcpy(itemName, (char *)pItemIDFile + strret.uOffset);
				else if(strret.uType == STRRET_WSTR)
				{
					char *pStr = Unicode2Ansi(strret.pOleStr);
					pMalloc->lpVtbl->Free(pMalloc, strret.pOleStr);
					if(pStr != NULL){
						strcpy(itemName, pStr);
						free(pStr);
					}
					else{
						nResult = -1;
						break;
					}
				}

				pItemID = ConcatPidls(pItemIDFolder,pItemIDFile);
				if(!(SHGetPathFromIDList(pItemID,tmpPath)))
				{
					nResult = -1;
					break;
				}

				*pptListIDL = (PLISTIDL)malloc(sizeof(LISTIDL));
				if(*pptListIDL == NULL)
				{
					nResult = -1;
					break;
				}
				(*pptListIDL)->uMenuID = uItemID;
				(*pptListIDL)->next = NULL;
				strcpy((*pptListIDL)->path,tmpPath);

				*pptODMenuItem = (PODMENUITEM)malloc(sizeof(ODMENUITEM));
				ZeroMemory(*pptODMenuItem, sizeof(ODMENUITEM));
				if(*pptODMenuItem == NULL)
				{
					nResult = -1;
					break;
				}
				else
				{
					if(g_ptODMenuItem == NULL)
						g_ptODMenuItem = *pptODMenuItem;
				}
				(*pptODMenuItem)->uMenuID = uItemID;
				(*pptODMenuItem)->next = NULL;

				ZeroMemory(&tSHFileInfo, sizeof(tSHFileInfo));

				SHGetFileInfo((LPCTSTR)pItemID, 0, &tSHFileInfo, sizeof(tSHFileInfo),
								SHGFI_PIDL | SHGFI_ICON | SHGFI_SMALLICON);
				(*pptODMenuItem)->hMenuIcon = tSHFileInfo.hIcon;

				AddMenuItem(hMenu, uItemID, MFT_OWNERDRAW, itemName, NULL);
				strcpy((*pptODMenuItem)->sMenuStr, itemName);
			}
			else{
				nResult = -1;
				break;
			}

			pMalloc->lpVtbl->Free(pMalloc, pItemID);
			pMalloc->lpVtbl->Free(pMalloc, pItemIDFile);
			pptListIDL    = &(*pptListIDL)->next;
			pptODMenuItem = &(*pptODMenuItem)->next;
			uItemID++;
		}
	}

	if(pEnumIDFile != NULL)
		pEnumIDFile->lpVtbl->Release(pEnumIDFile);

	if(pFolder != NULL)
		pFolder->lpVtbl->Release(pFolder);

	pMalloc->lpVtbl->Free(pMalloc, pItemIDFolder);

	if(nResult != 0)
		return FALSE;
	return TRUE;
}

void OnMeasureItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LPMEASUREITEMSTRUCT ptMeasure = (LPMEASUREITEMSTRUCT)lParam;
	HDC hDC;
	SIZE tSize;
	HFONT hFont, hOldFont;
	PODMENUITEM ptMenuItem = g_ptODMenuItem;

	UNREFERENCED_PARAMETER(wParam);
	hDC = GetDC(hwnd);
	hFont = CreateFontIndirect(&g_tODMenuState.tLogFont);
	hOldFont = (HFONT)SelectObject(hDC, hFont);

	while(ptMenuItem != NULL && ptMeasure->itemID != ptMenuItem->uMenuID){
		ptMenuItem = ptMenuItem->next;
	}

	GetTextExtentPoint32(hDC, ptMenuItem->sMenuStr,
						strlen(ptMenuItem->sMenuStr), &tSize);

	SelectObject(hDC, hOldFont);
	DeleteObject(hFont);

	ptMeasure->itemWidth = tSize.cx + g_tODMenuState.iIconSizeX + 14;
	ptMeasure->itemHeight = g_tODMenuState.iIconSizeY + 4;
	ReleaseDC(hwnd, hDC);
}


void OnDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT ptDrawItem = (LPDRAWITEMSTRUCT)lParam;
	HDC hDC;
	RECT tRect;
	RECT tRectText;
	HBRUSH hBrush;
	PODMENUITEM ptMenuItem = g_ptODMenuItem;

	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(hwnd);
	tRect = ptDrawItem->rcItem;
	hDC = ptDrawItem->hDC;

	if(ptDrawItem->itemState & ODS_SELECTED)
	{
		hBrush = CreateSolidBrush(g_tODMenuState.dwMenuBackColor);
		SetBkColor(hDC, g_tODMenuState.dwMenuBackColor);
		SetTextColor(hDC, g_tODMenuState.dwMenuBackTextColor);
	}
	else
		hBrush = CreateSolidBrush(GetBkColor(hDC));

	FillRect(hDC, &tRect, hBrush);
	DeleteObject(hBrush);

	while(ptMenuItem != NULL && ptDrawItem->itemID != ptMenuItem->uMenuID){
		ptMenuItem = ptMenuItem->next;
	}

	if(ptMenuItem->hMenuIcon != NULL){
		DrawIconEx(hDC, tRect.left + 2, tRect.top + 2, ptMenuItem->hMenuIcon,
					g_tODMenuState.iIconSizeX, g_tODMenuState.iIconSizeY,
					0, NULL, DI_NORMAL);
	}

	tRectText = tRect;
	tRectText.left += g_tODMenuState.iIconSizeX + 14;
	tRectText.top += 2;

	DrawText(hDC, ptMenuItem->sMenuStr, strlen(ptMenuItem->sMenuStr), &tRectText, DT_BOTTOM);
}

void OnMenuRButtonUp(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	PLISTIDL  pIDScan   = g_ptListIDL;
	PUSERMENU pMenuScan = g_ptUserMenu;
	UINT uID = (UINT)wParam;
	BOOL bmainMenu = TRUE;
	char path[MAX_PATH];

	//メニューを検索
	while(pMenuScan != NULL)
	{
		if (pMenuScan->hMenu == (HMENU)lParam)
		{
			if (uID == 0)
				uID = pMenuScan->uID;
			else
				uID += pMenuScan->st_uID - 1;
			bmainMenu = FALSE;
			break;
		}
		pMenuScan = pMenuScan->next;
	}
	if(bmainMenu)
		uID += 100;

	if(uID != 0)
	{
		while(pIDScan != NULL)
		{
			if(pIDScan->uMenuID == uID)
			{
				strcpy(path,pIDScan->path);
				break;
			}
			pIDScan = pIDScan->next;
		}
	}

	if (path[0] != 0)
	{
		HRESULT       hr;
		HMENU hFileMenu;
		LPMALLOC pMalloc;
		LPSHELLFOLDER pFolder = NULL, pDesktop = NULL;
		LPITEMIDLIST pAllIDL = NULL, pItemIDL = NULL, pParentIDL = NULL, pFileIDL = NULL;
		LPCONTEXTMENU pConMenu = NULL;
		CMINVOKECOMMANDINFO cmi;
		OLECHAR ochPath[MAX_PATH];
		POINT tPoint;
		UINT  uIDSysMenu;
		int iSize = 0;

		GetCursorPos(&tPoint);

		if(SHGetDesktopFolder(&pFolder) != NOERROR)
			return;
		if(SHGetDesktopFolder(&pDesktop) != NOERROR)
		{
			pFolder->lpVtbl->Release(pFolder);
			return;
		}
		if(SHGetMalloc(&pMalloc) != NOERROR)
		{
			pFolder->lpVtbl->Release(pFolder);
			pDesktop->lpVtbl->Release(pDesktop);
			return;
		}

		MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,path,-1,ochPath,MAX_PATH);
		if (pFolder->lpVtbl->ParseDisplayName(pFolder, NULL, NULL, ochPath, NULL, &pAllIDL, NULL)!=NOERROR)
		{
			pAllIDL = NULL;
			pFolder->lpVtbl->Release(pFolder);
			pMalloc->lpVtbl->Release(pMalloc);
			return;
		}

		pItemIDL = pAllIDL;
		while(((LPITEMIDLIST)((LPBYTE)pItemIDL + pItemIDL->mkid.cb))->mkid.cb)
		{
			iSize += pItemIDL->mkid.cb;
			pItemIDL = (LPITEMIDLIST)((LPBYTE)pItemIDL + pItemIDL->mkid.cb);
		}

		//ファイルのアイテムID
		pFileIDL = (LPITEMIDLIST)pMalloc->lpVtbl->Alloc(pMalloc, pItemIDL->mkid.cb + sizeof(USHORT));
		memcpy(pFileIDL, pItemIDL, pItemIDL->mkid.cb);
		memset((LPBYTE)pItemIDL + pItemIDL->mkid.cb, 0, sizeof(USHORT));

		//親ディレクトリのアイテムIDリスト
		pParentIDL = (LPITEMIDLIST)pMalloc->lpVtbl->Alloc(pMalloc, iSize + sizeof(USHORT));
		memcpy(pParentIDL, pAllIDL, iSize);
		memset((LPBYTE)pParentIDL + iSize, 0, sizeof(USHORT));
		pMalloc->lpVtbl->Free(pMalloc, pAllIDL);

		//親ディレクトリにバインド
		if (pDesktop->lpVtbl->BindToObject(pDesktop, pParentIDL, 0, &IID_IShellFolder, (LPVOID *)&pFolder)!=NOERROR)
		{
			pFolder->lpVtbl->Release(pFolder);
			pDesktop->lpVtbl->Release(pDesktop);
			pMalloc->lpVtbl->Free(pMalloc, pAllIDL);
			pMalloc->lpVtbl->Free(pMalloc, pParentIDL);
			pMalloc->lpVtbl->Release(pMalloc);
			return;
		}
		pMalloc->lpVtbl->Free(pMalloc, pParentIDL);
		pDesktop->lpVtbl->Release(pDesktop);

		//コンテキストメニューのインターフェイス取得
		hr = pFolder->lpVtbl->GetUIObjectOf(pFolder, hwnd, 1, (LPCITEMIDLIST *)&pFileIDL, &IID_IContextMenu, 0, (LPVOID *)&pConMenu);
		pMalloc->lpVtbl->Free(pMalloc, pFileIDL);
		pFolder->lpVtbl->Release(pFolder);
		pMalloc->lpVtbl->Release(pMalloc);

		if(SUCCEEDED(hr))
		{
			hFileMenu=CreatePopupMenu();
			if(hFileMenu)
			{
				hr = pConMenu->lpVtbl->QueryContextMenu(pConMenu, hFileMenu, 0, 1, 0x7fff, CMF_DEFAULTONLY|CMF_NORMAL); //ほかのを設定するとTrackPopupMenuで落ちる。
				if (SUCCEEDED(hr))
				{
					uIDSysMenu = TrackPopupMenu(hFileMenu,
												TPM_LEFTALIGN | TPM_RETURNCMD |
												TPM_RIGHTBUTTON | TPM_RECURSE,
												tPoint.x, tPoint.y, 0, hwnd, NULL);
					if (uIDSysMenu){
						cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
						cmi.fMask  = 0;
						cmi.hwnd   = hwnd;
						cmi.lpVerb = MAKEINTRESOURCE(uIDSysMenu-1);
						cmi.lpParameters = NULL;
						cmi.lpDirectory  = NULL;
						cmi.nShow        = SW_SHOWNORMAL;
						cmi.dwHotKey     = 0;
						cmi.hIcon        = NULL;
						pConMenu->lpVtbl->InvokeCommand(pConMenu, &cmi);
					}
				}
				DestroyMenu(hFileMenu);
			}
			pConMenu->lpVtbl->Release(pConMenu);
		}
	}
}


void OnInitMenuPopup(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	PUSERMENU pScan = g_ptUserMenu;
	LPMALLOC pMalloc;

	UNREFERENCED_PARAMETER(lParam);
	while(pScan != NULL)
	{
		if (pScan->hMenu == (HMENU)wParam && !pScan->flg)
		{
			if(SHGetMalloc(&pMalloc) == NOERROR)
			{
				pScan->flg = TRUE;
				pScan->st_uID = uItemID;
				AddUserMenu(pScan->hMenu, pMalloc, pScan->path);
				pMalloc->lpVtbl->Release(pMalloc);
				SetForegroundWindow98(hwnd);
				break;
			}
		}
		pScan = pScan->next;
	}
}

void showUserMenu(HWND hwnd, HWND hwndClicked, int xPos, int yPos, int btn, int clk)
{
	LPMALLOC pMalloc;

	UNREFERENCED_PARAMETER(hwndClicked);
	if(SHGetMalloc(&pMalloc) == NOERROR)
	{
		HMENU hMenu;
		UINT uID;
		PLISTIDL pScan;
		char fname[1024];
		char entry[20];

		wsprintf(entry, "%d%dFile", btn, clk);
		GetMyRegStr("Mouse", entry, fname, 1024, "");
		del_title(fname);

		hMenu = CreatePopupMenu();
		if(hMenu == NULL)
		{
			return;
		}

		uItemID = 100;
		if ( AddUserMenu(hMenu, pMalloc, fname))
		{
			if(hMenu != NULL)
			{
				if(!getODFlg++)
					GetODMenuState();
				SetForegroundWindow98(hwnd);

				uID = (UINT)TrackPopupMenu(
							hMenu,
							TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
							xPos, yPos, 0, hwnd, NULL);
				if(uID != 0)
				{
					pScan = g_ptListIDL;
					while(pScan != NULL)
					{
						if(pScan->uMenuID == uID)
						{
							ExecFile(hwnd, pScan->path);
							break;
						}
						pScan = pScan->next;
					}
				}
			}
		}
		DestroyUserMenu();
		DestroyMenu(hMenu);
	}
	pMalloc->lpVtbl->Release(pMalloc);
}
