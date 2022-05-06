/*-------------------------------------------
  propdlg.c
  show "properties for TClock"
---------------------------------------------*/

//#define NONAMELESSUNION
#include "tclock.h"

#define MAX_PAGE  20

INT_PTR CALLBACK PropertyDialog(HWND, UINT, WPARAM, LPARAM);

void SetMyDialgPos(HWND hwnd);

// dialog procedures of each page
INT_PTR CALLBACK PageColorProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageFormatProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageMouseProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageTooltipProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageGraphProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageMiscProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageAnalogClockProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageAboutProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageChimeProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK PageBarmeterProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageEtcProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageKeywordProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageAppControlProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageEtc1Proc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageWin11Proc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK PageColorAdditionalProc(HWND, UINT, WPARAM, LPARAM);




// TV_INSERTSTRUCTではなぜかエラーがでた
typedef struct{
	HTREEITEM hParent;
	HTREEITEM hInsertAfter;
	TV_ITEM item;
} _TV_INSERTSTRUCT;


static HHOOK hookMsgfilter = 0;

static WNDPROC oldWndProc; // to save default window procedure
static int startpage = 0;  // page to open first

BOOL g_bApplyClock = FALSE;
BOOL g_bApplyTaskbar = FALSE;
BOOL g_bApplyLangDLL = FALSE;

// menu.c
extern HMENU g_hMenu;

extern BOOL b_EnglishMenu;
extern int Language_Offset;




/*-------------------------------------------
  show property sheet
---------------------------------------------*/
void MyPropertyDialog(void)
{
	g_bApplyClock = FALSE;
	g_bApplyTaskbar = FALSE;
	g_bApplyLangDLL = FALSE;

	if(g_hwndPropDlg && IsWindow(g_hwndPropDlg))
		;
	else {
		g_hwndPropDlg = CreateDialog(GetLangModule(), MAKEINTRESOURCE(Language_Offset + IDD_PROPERTY), g_hwndMain, PropertyDialog);
	}
	ShowWindow(g_hwndPropDlg, SW_SHOW);
	UpdateWindow(g_hwndPropDlg);

	SetForegroundWindow98(g_hwndPropDlg);
}

static VOID SetPageDlgPos(HWND hParent, HWND hDlg)
{
	LONG DlgBase;
	WORD DlgBaseH;
	HWND hTree;
	RECT rect;
	POINT pos;

	hTree = GetDlgItem(hParent, IDC_TREE);
	GetWindowRect(hTree, &rect);
	pos.x = rect.right;
	pos.y = rect.top;
	ScreenToClient(hParent, &pos);

	DlgBase = GetDialogBaseUnits();
	DlgBaseH = LOWORD(DlgBase);

	pos.x = pos.x + (DlgBaseH / 4);
	pos.y = pos.y;
	SetWindowPos(hDlg, NULL, pos.x, pos.y, 0, 0, SWP_NOSIZE);
}

static VOID CreatePageDialog(HWND hParent, HWND hDlg[], BOOL bDlgFlg[], int index, WORD wID, DLGPROC dlgprc)
{
	HINSTANCE hInst;

	if (bDlgFlg[index]) {
		return;
	}
	hInst   = GetLangModule();
	hDlg[index] = CreateDialog(hInst, MAKEINTRESOURCE(wID), hParent, dlgprc);
	SetPageDlgPos(hParent, hDlg[index]);

	bDlgFlg[index] = TRUE;
}

/*-------------------------------------------
  Property dialog
---------------------------------------------*/
INT_PTR CALLBACK PropertyDialog(HWND hDwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hTree;
	static HWND hDlg[MAX_PAGE];
	static BOOL bDlgFlg[MAX_PAGE];
	static HWND *hNowDlg;
	_TV_INSERTSTRUCT tv;
	HTREEITEM hParent[6], hChild[MAX_PAGE];
	NM_TREEVIEW *pNMTV;
//	HINSTANCE hInst;
	static int nowDlg;
	int i;

	switch(message){
		case WM_INITDIALOG:
			InitCommonControls();

			hTree = GetDlgItem(hDwnd, IDC_TREE);
			memset(&tv, 0, sizeof(_TV_INSERTSTRUCT));

			//設定ダイアログ左メニューの順序は、数字ではなく、以下の行の順番で決まっている。

			tv.hInsertAfter = TVI_LAST;
			tv.hParent = TVI_ROOT;
			tv.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
			tv.item.state = TVIS_EXPANDED;
			tv.item.stateMask = TVIS_EXPANDED;

			tv.item.pszText = MyString(IDS_CLOCK);
			tv.item.lParam = 0;
			hParent[0] = TreeView_InsertItem(hTree, &tv);

			tv.item.lParam = 1;
			tv.item.pszText = MyString(IDS_TOOLTIP);
			hParent[1] = TreeView_InsertItem(hTree, &tv);

			tv.item.lParam = 6;
			tv.item.pszText = MyString(IDS_PROP_MOUSE);
			hParent[6] = TreeView_InsertItem(hTree, &tv);

			tv.item.lParam = 7;
			tv.item.pszText = MyString(IDS_PROP_ETC);
			hParent[7] = TreeView_InsertItem(hTree, &tv);

			tv.item.lParam = 2;
			tv.item.pszText = MyString(IDS_PROP_KEYWORDS);
			hParent[2] = TreeView_InsertItem(hTree, &tv);

			tv.item.lParam = 5;
			tv.item.pszText = MyString(IDS_PROP_WIN11);
			hParent[5] = TreeView_InsertItem(hTree, &tv);

			tv.item.lParam = 3;
			tv.item.pszText = MyString(IDS_ABOUT);
			hParent[3] = TreeView_InsertItem(hTree, &tv);

			tv.item.lParam = 4;
			tv.item.pszText = MyString(IDS_MISC);
			hParent[4] = TreeView_InsertItem(hTree, &tv);



			tv.hParent = hParent[0];
			tv.item.mask = TVIF_TEXT | TVIF_PARAM;

			tv.item.lParam = 100;
			tv.item.pszText = MyString(IDS_PROP_COLOR);
			hChild[0] = TreeView_InsertItem(hTree, &tv);

			tv.item.lParam = 106;
			tv.item.pszText = MyString(IDS_PROP_COLOR_ADDITIONAL);
			hChild[6] = TreeView_InsertItem(hTree, &tv);

			tv.item.lParam = 101;
			tv.item.pszText = MyString(IDS_PROP_FORMAT);
			hChild[1] = TreeView_InsertItem(hTree, &tv);

			tv.item.lParam = 107;
			tv.item.pszText = MyString(IDS_PROP_CHIME);
			hChild[7] = TreeView_InsertItem(hTree, &tv);

			tv.item.lParam = 103;
			tv.item.pszText = MyString(IDS_PROP_GRAPH);
			hChild[3] = TreeView_InsertItem(hTree, &tv);

			//	BarMeter設定	20181103
			tv.item.lParam = 105;
			tv.item.pszText = MyString(IDS_BARMETER);
			hChild[5] = TreeView_InsertItem(hTree, &tv);


			//tv.item.lParam = 102;
			//tv.item.pszText = MyString(IDS_PROP_MOUSE);
			//hChild[2] = TreeView_InsertItem(hTree, &tv);


			tv.item.lParam = 104;
			tv.item.pszText = MyString(IDS_PROP_ANALOG);
			hChild[4] = TreeView_InsertItem(hTree, &tv);



			CreatePageDialog(hDwnd, hDlg, bDlgFlg, 0, Language_Offset + IDD_PAGECOLOR, PageColorProc);
			nowDlg = startpage;
			//nowDlg = 0;
			hNowDlg = &hDlg[nowDlg];
			ShowWindow(*hNowDlg, SW_SHOW);
			UpdateWindow(*hNowDlg);

			if (nowDlg < 9)
				TreeView_SelectItem(hTree, hChild[nowDlg]);
			else
				TreeView_SelectItem(hTree, hParent[nowDlg - 10]);


			break;

		case WM_SHOWWINDOW: // adjust the window position
			SetMyDialgPos(hDwnd);
			break;

		case WM_NOTIFY:
			pNMTV = (NM_TREEVIEW *)lParam;
			switch (pNMTV->hdr.code){
				case TVN_SELCHANGED:
					ShowWindow(*hNowDlg, SW_HIDE);
					UpdateWindow(*hNowDlg);
					nowDlg = (int)pNMTV->itemNew.lParam;
					switch (nowDlg)		//nowDlgはここのところだけはTree選択のバッファとして利用されているが、このswitchを出るときにもともとのnowDlgとしての意味に戻っている。
					{
						case 0:
							nowDlg = 0;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGECOLOR, PageColorProc);
							break;

						case 1:
							nowDlg = 11;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGETOOLTIP, PageTooltipProc);
							break;

						case 2:
							nowDlg = 12;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGE_KEYWORDS, PageKeywordProc);
							break;

						case 3:
							nowDlg = 13;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGEABOUT, PageAboutProc);
							break;

						case 4:
							nowDlg = 14;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGEMISC, PageMiscProc);
							break;

						case 5:
							nowDlg = 15;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGE_WIN11, PageWin11Proc);
							break;

						case 6:
							nowDlg = 16;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGEMOUSE, PageMouseProc);
							break;

						case 7:
							nowDlg = 17;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGE_ETC, PageEtcProc);
							break;

						case 100:
							//nowDlg -= 10;
							nowDlg = 0;		//PAGECOLORの親と同じ
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGECOLOR, PageColorProc);
							break;
						case 101:
							//nowDlg -= 10;
							nowDlg = 1;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGEFORMAT, PageFormatProc);
							break;

						//case 102:
						//	//nowDlg -= 10;
						//	nowDlg = 2;
						//	CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGEMOUSE, PageMouseProc);
						//	break;
						case 103:
							//nowDlg -= 10;
							nowDlg = 3;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGEGRAPH, PageGraphProc);
							break;
						case 104:
							//nowDlg -= 10;
							nowDlg = 4;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGEANALOG, PageAnalogClockProc);
							break;

						case 105:
							nowDlg = 5;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGEBARMETER, PageBarmeterProc);
							break;

						case 106:
							nowDlg = 6;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGECOLOR_ADDITIONAL, PageColorAdditionalProc);
							break;

						case 107:
							nowDlg = 7;
							CreatePageDialog(hDwnd, hDlg, bDlgFlg, nowDlg, Language_Offset + IDD_PAGECHIME, PageChimeProc);
							break;

						default:
							//nowDlg -= 10;
							nowDlg = 0;
							break;
					}
					hNowDlg = &hDlg[nowDlg];
					ShowWindow(*hNowDlg, SW_SHOW);
					UpdateWindow(*hNowDlg);
					break;
			}
			break;

		case WM_COMMAND:
			// apply settings
			if(LOWORD(wParam) == IDOK || LOWORD(wParam) == ID_APPLY)
			{
				NMHDR lp;
				lp.code = PSN_APPLY;
				SendMessage(hDlg[nowDlg], WM_NOTIFY, 0, (LPARAM)&lp);
				if(g_bApplyClock)
				{
					PostMessage(g_hwndClock, CLOCKM_REFRESHCLOCK, 0, 0);
					g_bApplyClock = FALSE;
				}
				if(g_bApplyTaskbar)
				{
					PostMessage(g_hwndClock, CLOCKM_REFRESHTASKBAR, 0, 0);
					g_bApplyTaskbar = FALSE;
				}
			}
			if(LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				startpage = nowDlg;

				// reload language dll
				if(g_bApplyLangDLL)
				{
					char fname[MAX_PATH];
					HINSTANCE hInst;
					hInst = LoadLanguageDLL(fname);
					if(hInst != NULL)
					{
						if(g_hMenu) DestroyMenu(g_hMenu);
						g_hMenu = NULL;
						if(g_hInstResource) FreeLibrary(g_hInstResource);
						g_hInstResource = hInst;
						strcpy(g_langdllname, fname);
					}
				}
				for (i = 0; i < MAX_PAGE; i++) {
					bDlgFlg[i] = FALSE;
				}
				DestroyWindow(hDwnd);
				g_hwndPropDlg = NULL;
			}
			// HELP
			if(LOWORD(wParam) == ID_HELP){
				char fname[1024];
				strcpy(fname, g_mydir);
				if (b_EnglishMenu) {
					add_title(fname, "readme_en.txt");
				}
				else {
					add_title(fname, "readme_jp.txt");
				}
				ShellExecute(NULL, "open", "notepad.exe", fname, NULL, SW_SHOWNORMAL);
			}
			break;

		// close by "x" button
		case WM_SYSCOMMAND:
			if((wParam & 0xfff0) == SC_CLOSE)
				PostMessage(hDwnd, WM_COMMAND, IDCANCEL, 0);
			break;
	}

	return 0;
}



/*------------------------------------------------
   adjust the window position
--------------------------------------------------*/
void SetMyDialgPos(HWND hwnd)
{
	HWND hwndTray;
	RECT rc, rcTray;
	int wscreen, hscreen, wProp, hProp;
	int x, y;

	GetWindowRect(hwnd, &rc);
	wProp = rc.right - rc.left;
	hProp = rc.bottom - rc.top;

	wscreen = GetSystemMetrics(SM_CXSCREEN);
	hscreen = GetSystemMetrics(SM_CYSCREEN);

	hwndTray = FindWindow("Shell_TrayWnd", NULL);
	if(hwndTray == NULL) return;
	GetWindowRect(hwndTray, &rcTray);
	if(rcTray.right - rcTray.left >
		rcTray.bottom - rcTray.top)
	{
		x = wscreen - wProp - 32;
		if(rcTray.top < hscreen / 2)
			y = rcTray.bottom + 2;
		else
			y = rcTray.top - hProp - 32;
		if(y < 0) y = 0;
	}
	else
	{
		y = hscreen - hProp - 2;
		if(rcTray.left < wscreen / 2)
			x = rcTray.right + 2;
		else
			x = rcTray.left - wProp - 2;
		if(x < 0) x = 0;
	}

	MoveWindow(hwnd, x, y, wProp, hProp, FALSE);
}

/*------------------------------------------------
   select file
--------------------------------------------------*/
BOOL SelectMyFile(HWND hDlg, const char *filter, DWORD nFilterIndex,
	const char *deffile, char *retfile)
{
	OPENFILENAME ofn;
	char fname[MAX_PATH], ftitle[MAX_PATH], initdir[MAX_PATH];
	BOOL r;

	memset(&ofn, '\0', sizeof(OPENFILENAME));

	strcpy(initdir, g_mydir);
	if(deffile[0])
	{
		WIN32_FIND_DATA fd;
		HANDLE hfind;
		hfind = FindFirstFile(deffile, &fd);
		if(hfind != INVALID_HANDLE_VALUE)
		{
			FindClose(hfind);
			strcpy(initdir, deffile);
			del_title(initdir);
		}
	}

	fname[0] = 0;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hDlg;
	ofn.hInstance = g_hInst;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = nFilterIndex;
	ofn.lpstrFile= fname;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = ftitle;
	ofn.nMaxFileTitle = MAX_PATH;
	ofn.lpstrInitialDir = initdir;

	ofn.Flags = OFN_HIDEREADONLY | OFN_EXPLORER ;
	r = GetOpenFileName(&ofn);
	if(!r) return r;

	strcpy(retfile, ofn.lpstrFile);

	return r;
}

