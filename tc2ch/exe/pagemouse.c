/*-------------------------------------------
  pagemouse.c
　　「マウス操作」プロパティページ
　　KAZUBON 1997-1998
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnDestroy(HWND hDlg);
static void OnDropFilesChange(HWND hDlg);
static void OnMouseButton(HWND hDlg);
static void OnMouseClickTime(HWND hDlg, int id);
static void OnMouseFunc(HWND hDlg);
static void OnMouseFileChange(HWND hDlg);

static void OnSansho(HWND hDlg, WORD id);
static void InitMouseFuncList(HWND hDlg);

static char reg_section[] = "Mouse";

typedef struct {
	BOOL disable;
	int func[4];
	WORD hotkey[4];
	char format[4][256];
	char fname[4][256];
	int volume[4];
} CLICKDATA;
static CLICKDATA *pData = NULL;

//#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyClock = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

extern BOOL b_EnglishMenu;
extern int Language_Offset;

/*------------------------------------------------
　「マウス操作」ページ用ダイアログプロシージャ
--------------------------------------------------*/
BOOL CALLBACK PageMouseProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			switch(id)
			{
			// "Drop files"
			case IDC_DROPFILES:
				if(code == CBN_SELCHANGE)
				{
					OnDropFilesChange(hDlg);
					g_bApplyClock = TRUE;
				}
				break;
			case IDC_DROPFILESAPP:
				if(code == EN_CHANGE)
					SendPSChanged(hDlg);
				break;
			// "..."
			case IDC_DROPFILESAPPSANSHO:
			case IDC_MOUSEFILESANSHO:
				OnSansho(hDlg, id);
				break;
			//  "Button"
			case IDC_MOUSEBUTTON:
				if(code == CBN_SELCHANGE)
					OnMouseButton(hDlg);
				break;
			// single .... quadruple
			case IDC_RADSINGLE:
			case IDC_RADDOUBLE:
			case IDC_RADTRIPLE:
			case IDC_RADQUADRUPLE:
				OnMouseClickTime(hDlg, id);
				break;
			//  Mouse Function
			case IDC_MOUSEFUNC:
				if(code == CBN_SELCHANGE)
				{
					OnMouseFunc(hDlg);
					SendPSChanged(hDlg);
				}
				break;
			// Mouse Function - File
			case IDC_MOUSEFILE:
				if(code == EN_CHANGE)
				{
					OnMouseFileChange(hDlg);
					SendPSChanged(hDlg);
				}
				break;
			case IDC_RCLICKMENU:
				g_bApplyClock = TRUE;
				SendPSChanged(hDlg);
				break;
			case IDC_HOTKEY:
				OnMouseFunc(hDlg);
				SendPSChanged(hDlg);
				break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: My2chHelp(GetParent(hDlg)); break;
			}
			return TRUE;
		case WM_DESTROY:
			OnDestroy(hDlg);
			return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------
　ページの初期化
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	char s[256];
	char entry[20];
	BOOL b;
	int i, j;
	HWND hDlgPage;
	HFONT hfont;

	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont)
	{
		SendDlgItemMessage(hDlg, IDC_DROPFILESAPP,
			WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_MOUSEFILE,
			WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_TOOLTIP,
			WM_SETFONT, (WPARAM)hfont, 0);
	}

	for(i = IDS_NONE; i <= IDS_MOVETO; i++)
		CBAddString(hDlg, IDC_DROPFILES, (LPARAM)MyString(i));
	CBSetCurSel(hDlg, IDC_DROPFILES,
		GetMyRegLong(reg_section, "DropFiles", 0));
	GetMyRegStr(reg_section, "DropFilesApp", s, 256, "");
	SetDlgItemText(hDlg, IDC_DROPFILESAPP, s);

	pData = malloc(sizeof(CLICKDATA) * 28);

	for(i = 0; i < 28; i++)
	{
		for(j = 0; j < 4; j++)
		{
			wsprintf(entry, "%d%d", i, j+1);
			pData[i].disable = FALSE;
			pData[i].func[j] =
				GetMyRegLong(reg_section, entry, MOUSEFUNC_NONE);
			pData[i].format[j][0] = 0;
			pData[i].fname[j][0] = 0;
			pData[i].hotkey[j] = 0;
			if(i == IDS_HOTKEY - IDS_LEFTBUTTON)
			{
				wsprintf(entry, "%d%dHotkey", i, j+1);
				pData[i].hotkey[j] = (WORD)GetMyRegLong(reg_section, entry, 0);
			}

			else if(pData[i].func[j] == MOUSEFUNC_OPENFILE || pData[i].func[j] == MOUSEFUNC_FILELIST)
			{
				wsprintf(entry, "%d%dFile", i, j+1);
				GetMyRegStr(reg_section, entry, pData[i].fname[j], 256, "");
			}

		}
	}


	for(i = IDS_LEFTBUTTON; i <= IDS_SWHEEL2; i++)
		CBAddString(hDlg, IDC_MOUSEBUTTON, (LPARAM)MyString(i));
	AdjustDlgConboBoxDropDown(hDlg, IDC_MOUSEBUTTON, 22);

	CheckDlgButton(hDlg, IDC_RCLICKMENU,
		GetMyRegLong("Mouse", "RightClickMenu", TRUE));





	// set mouse functions to combo box
	InitMouseFuncList(hDlg);

	OnDropFilesChange(hDlg);
	CBSetCurSel(hDlg, IDC_MOUSEBUTTON, 0);
	OnMouseButton(hDlg);
}

/*------------------------------------------------
　更新
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[256], entry[20];
	int n;
	int i, j;

	n = CBGetCurSel(hDlg, IDC_DROPFILES);
	SetMyRegLong(reg_section, "DropFiles", n);
	GetDlgItemText(hDlg, IDC_DROPFILESAPP, s, 256);
	SetMyRegStr(reg_section, "DropFilesApp", s);

	SetMyRegLong("Mouse", "RightClickMenu",
		IsDlgButtonChecked(hDlg, IDC_RCLICKMENU));

	for(i = 0; i < 28; i++)
	{
		//if(i == 1) continue;
		for(j = 0; j < 4; j++)
		{
			wsprintf(entry, "%d%d", i, j+1);
			if(pData[i].func[j] >= 0)
				SetMyRegLong(reg_section, entry, pData[i].func[j]);
			else DelMyReg(reg_section, entry);
			if(i == IDS_HOTKEY - IDS_LEFTBUTTON)
			{
				wsprintf(entry, "%d%dHotkey", i, j+1);
				if (pData[i].hotkey[j])
					SetMyRegLong(reg_section, entry, pData[i].hotkey[j]);
				else DelMyReg(reg_section, entry);
			}
			if(pData[i].func[j] == MOUSEFUNC_OPENFILE || pData[i].func[j] == MOUSEFUNC_FILELIST)
			{
				wsprintf(entry, "%d%dFile", i, j+1);
				SetMyRegStr(reg_section, entry, pData[i].fname[j]);
			}

		}
	}
	ResetHotkey(g_hwndMain);
}

/*------------------------------------------------

--------------------------------------------------*/
void OnDestroy(HWND hDlg)
{
	UNREFERENCED_PARAMETER(hDlg);
	if(pData) free(pData);
}

/*------------------------------------------------
　「ファイルのドロップ」
--------------------------------------------------*/
void OnDropFilesChange(HWND hDlg)
{
	int i, n;
	n = CBGetCurSel(hDlg, IDC_DROPFILES);
	SetDlgItemText(hDlg, IDC_LABDROPFILESAPP,
		MyString(n >= 3?IDS_LABFOLDER:IDS_LABPROGRAM));
	for(i = IDC_LABDROPFILESAPP; i <= IDC_DROPFILESAPPSANSHO; i++)
		ShowDlgItem(hDlg, i, (2 <= n && n <= 4));

	SendPSChanged(hDlg);
}

/*------------------------------------------------
  "Button"
--------------------------------------------------*/
void OnMouseButton(HWND hDlg)
{
	int n, button, j;

	n = CBGetCurSel(hDlg, IDC_MOUSEBUTTON);
	button = n;
	//if(n > 0) button = n + 1;

	for(j = 0; j < 4; j++)
	{
		if(pData[button].func[j] >= 0) break;
	}
	if(j == 4) j = 0;
	CheckRadioButton(hDlg, IDC_RADSINGLE, IDC_RADQUADRUPLE,
		IDC_RADSINGLE + j);

	if (button == (IDS_HOTKEY - IDS_LEFTBUTTON))
	{
		ShowDlgItem(hDlg, IDC_HOTKEY, TRUE);
	}
	else
	{
		ShowDlgItem(hDlg, IDC_HOTKEY, FALSE);
	}

	if (button == (IDS_WHEEL1  - IDS_LEFTBUTTON) || button == (IDS_WHEEL2  - IDS_LEFTBUTTON)||
		button == (IDS_CWHEEL1 - IDS_LEFTBUTTON) || button == (IDS_CWHEEL2 - IDS_LEFTBUTTON)||
		button == (IDS_SWHEEL1 - IDS_LEFTBUTTON) || button == (IDS_SWHEEL2 - IDS_LEFTBUTTON))
	{
		EnableDlgItem(hDlg, IDC_RADDOUBLE, FALSE);
		EnableDlgItem(hDlg, IDC_RADTRIPLE, FALSE);
		EnableDlgItem(hDlg, IDC_RADQUADRUPLE, FALSE);
	}
	else
	{
		EnableDlgItem(hDlg, IDC_RADDOUBLE, TRUE);
		EnableDlgItem(hDlg, IDC_RADTRIPLE, TRUE);
		EnableDlgItem(hDlg, IDC_RADQUADRUPLE, TRUE);
	}

	OnMouseClickTime(hDlg, IDC_RADSINGLE + j);
}

/*------------------------------------------------
  "Single" ... "Quadruple"
--------------------------------------------------*/
void OnMouseClickTime(HWND hDlg, int id)
{
	int n, button;
	int click, i, count, func;

	n = CBGetCurSel(hDlg, IDC_MOUSEBUTTON);
	button = n;

	click = id - IDC_RADSINGLE;
	func = pData[button].func[click];

	count = CBGetCount(hDlg, IDC_MOUSEFUNC);
	for(i = 0; i < count; i++)
	{
		if(func == CBGetItemData(hDlg, IDC_MOUSEFUNC, i))
		{
			CBSetCurSel(hDlg, IDC_MOUSEFUNC, i);
			break;
		}
	}

	if (button == (IDS_HOTKEY - IDS_LEFTBUTTON))
		SendDlgItemMessage(hDlg, IDC_HOTKEY, HKM_SETHOTKEY, pData[button].hotkey[click], 0);
	else
		SendDlgItemMessage(hDlg, IDC_HOTKEY, HKM_SETHOTKEY, 0, 0);

	OnMouseFunc(hDlg);
}

/*------------------------------------------------
  Mouse Functions combo box
--------------------------------------------------*/
void OnMouseFunc(HWND hDlg)
{
	int n, button, j;
	int click, index, func;

	n = CBGetCurSel(hDlg, IDC_MOUSEBUTTON);
	button = n;

	for(j = 0; j < 4; j++)
	{
		if(IsDlgButtonChecked(hDlg, IDC_RADSINGLE + j))
			break;
	}
	if(j == 5) return;
	click = j;

	index = CBGetCurSel(hDlg, IDC_MOUSEFUNC);
	func = CBGetItemData(hDlg, IDC_MOUSEFUNC, index);
	pData[button].func[click] = func;

	if (button == (IDS_HOTKEY - IDS_LEFTBUTTON))
		pData[button].hotkey[click] = (WORD)SendDlgItemMessage(hDlg, IDC_HOTKEY, HKM_GETHOTKEY, 0, 0);

	ShowDlgItem(hDlg, IDC_LABMOUSEFILE,
		(func == MOUSEFUNC_OPENFILE||func == MOUSEFUNC_FILELIST
			));
	ShowDlgItem(hDlg, IDC_MOUSEFILE,
		(func == MOUSEFUNC_OPENFILE||func == MOUSEFUNC_FILELIST));
	ShowDlgItem(hDlg, IDC_MOUSEFILESANSHO, (func == MOUSEFUNC_OPENFILE || func == MOUSEFUNC_FILELIST));

	if(func == MOUSEFUNC_OPENFILE || func == MOUSEFUNC_FILELIST)
	{
		SetDlgItemText(hDlg, IDC_LABMOUSEFILE, MyString(IDS_FILE));
		SetDlgItemText(hDlg, IDC_MOUSEFILE, pData[button].fname[click]);
	}

}

/*------------------------------------------------
  Format/File - Mouse Function
--------------------------------------------------*/
void OnMouseFileChange(HWND hDlg)
{
	int n, button, j;
	int click, index, func;

	n = CBGetCurSel(hDlg, IDC_MOUSEBUTTON);
	button = n;

	for(j = 0; j < 4; j++)
	{
		if(IsDlgButtonChecked(hDlg, IDC_RADSINGLE + j))
			break;
	}
	if(j == 5) return;
	click = j;

	index = CBGetCurSel(hDlg, IDC_MOUSEFUNC);
	func = CBGetItemData(hDlg, IDC_MOUSEFUNC, index);

	if(func == MOUSEFUNC_OPENFILE || func == MOUSEFUNC_FILELIST)
		GetDlgItemText(hDlg, IDC_MOUSEFILE, pData[button].fname[click], 1024);
}

/*------------------------------------------------
　「...」　ファイルの参照
--------------------------------------------------*/
void OnSansho(HWND hDlg, WORD id)
{
	int n;
	char filter[80], deffile[MAX_PATH], fname[MAX_PATH];

	if(id == IDC_DROPFILESAPPSANSHO)
	{
		n = CBGetCurSel(hDlg, IDC_DROPFILES);
		if(n >= 3)
		{
			BROWSEINFO bi;
			LPITEMIDLIST pidl;
			memset(&bi, 0, sizeof(BROWSEINFO));
			bi.hwndOwner = hDlg;
			bi.ulFlags = BIF_RETURNONLYFSDIRS;
			pidl = SHBrowseForFolder(&bi);
			if(pidl)
			{
				SHGetPathFromIDList(pidl, fname);
				SetDlgItemText(hDlg, id - 1, fname);
				PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
				SendPSChanged(hDlg);
			}
			return;
		}
	}

	filter[0] = 0;
	if(id == IDC_DROPFILESAPPSANSHO)
	{
		str0cat(filter, MyString(IDS_PROGRAMFILE));
		str0cat(filter, "*.exe");
	}
	str0cat(filter, MyString(IDS_ALLFILE));
	str0cat(filter, "*.*");

	GetDlgItemText(hDlg, id - 1, deffile, MAX_PATH);

	if(!SelectMyFile(hDlg, filter, 0, deffile, fname)) // propsheet.c
		return;

	SetDlgItemText(hDlg, id - 1, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}


/*------------------------------------------------
  set mouse functions to combo box
--------------------------------------------------*/
void InitMouseFuncList(HWND hDlg)
{
	int i, index, cnt;
	MOUSE_FUNC_INFO *pmfl;
	cnt = GetMouseFuncCount();
	pmfl = GetMouseFuncList();
	for (i = 0; i < cnt; i++)
	{
		//リストの各項目を追加
		index = CBAddString(hDlg, IDC_MOUSEFUNC, (LPARAM)MyString(pmfl[i].idstring));
		CBSetItemData(hDlg, IDC_MOUSEFUNC, index, pmfl[i].mousefunc);
	}
	//リスト項目の表示数を指定
	AdjustDlgConboBoxDropDown(hDlg, IDC_MOUSEFUNC, 29);

}
