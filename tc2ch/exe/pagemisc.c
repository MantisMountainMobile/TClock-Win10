/*-------------------------------------------
  pagemisc.c
　　「その他」プロパティページ
　　KAZUBON 1997-1998
---------------------------------------------*/
#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);


static void OnStartup(HWND hDlg);
BOOL CreateLink(LPCSTR fname, LPCSTR dstpath, LPCSTR name);

#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)

extern BOOL b_EnglishMenu;
extern int Language_Offset;

/*------------------------------------------------
　「その他」ページ用ダイアログプロシージャ
--------------------------------------------------*/
BOOL CALLBACK PageMiscProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
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
			// 「スタートアップ」にショートカットをつくる
			case IDC_STARTUP:
				OnStartup(hDlg);
				break;
			case IDC_DELREG:
				{
					int r;
					r = DelRegAll();
					if (r == 0)
						MyMessageBox(hDlg,MyString(IDS_DELREGNGINI), "TClock-Win10",MB_OK,MB_ICONEXCLAMATION);
					else if (r == 1)
						MyMessageBox(hDlg,MyString(IDS_DELREGOK), "TClock-Win10",MB_OK,MB_ICONINFORMATION);
					else
						MyMessageBox(hDlg,MyString(IDS_DELREGNG),"TClock-Win10",MB_OK,MB_ICONEXCLAMATION);
				}
				break;
				//  readme.txtを開く
			case IDC_README1:
				My2chHelp(GetParent(hDlg));
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
	}
	return FALSE;
}

/*------------------------------------------------
　ページの初期化
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	char s[256], s1[32];

	GetMyRegStr("Status_DoNotEdit", "Version", s1, 16, "x.x.x.x");

	SendDlgItemMessage(hDlg, IDC_ABOUTICON, STM_SETIMAGE,
		IMAGE_ICON, (LPARAM)g_hIconTClock);

	wsprintf(s, "TClock-Win10 Ver %s", s1);

	SendDlgItemMessage(hDlg, IDC_STATIC_VersionText, WM_SETTEXT, 0, (LPARAM)s);

}

/*------------------------------------------------
　更新
--------------------------------------------------*/
void OnApply(HWND hDlg)
{

}

/*------------------------------------------------
　「スタートアップ」にショートカットをつくる
--------------------------------------------------*/
void OnStartup(HWND hDlg)
{
	LPITEMIDLIST pidl;
	char dstpath[MAX_PATH], myexe[MAX_PATH];

	if(SHGetSpecialFolderLocation(hDlg, CSIDL_STARTUP, &pidl) == NOERROR &&
		SHGetPathFromIDList(pidl, dstpath) == TRUE)
		;
	else return;

	if(MyMessageBox(hDlg, MyString(IDS_STARTUPLINK),
		"TClock-Win10", MB_YESNO, MB_ICONQUESTION) != IDYES) return;

	GetModuleFileName(GetModuleHandle(NULL), myexe, MAX_PATH);
	CreateLink(myexe, dstpath, "TClock-Win10");
}

/*------------------------------------------------
　ショートカットの作成
--------------------------------------------------*/
BOOL CreateLink(LPCSTR fname, LPCSTR dstpath, LPCSTR name)
{
	HRESULT hres;
	IShellLink* psl;

	CoInitialize(NULL);

	hres = CoCreateInstance(&CLSID_ShellLink, NULL,
		CLSCTX_INPROC_SERVER, &IID_IShellLink, &psl);
	if(SUCCEEDED(hres))
	{
		IPersistFile* ppf;
		char path[MAX_PATH];

		psl->lpVtbl->SetPath(psl, fname);
		psl->lpVtbl->SetDescription(psl, name);
		strcpy(path, fname);
		del_title(path);
		psl->lpVtbl->SetWorkingDirectory(psl, path);

		hres = psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile,
			&ppf);

		if(SUCCEEDED(hres))
		{
			WORD wsz[MAX_PATH];
			char lnkfile[MAX_PATH];
			strcpy(lnkfile, dstpath);
			add_title(lnkfile, (char*)name);
			strcat(lnkfile, ".lnk");

			MultiByteToWideChar(CP_ACP, 0, lnkfile, -1,
				wsz, MAX_PATH);

			hres = ppf->lpVtbl->Save(ppf, wsz, TRUE);
			ppf->lpVtbl->Release(ppf);
		}
		psl->lpVtbl->Release(psl);
	}
	CoUninitialize();

	if(SUCCEEDED(hres)) return TRUE;
	else return FALSE;
}
