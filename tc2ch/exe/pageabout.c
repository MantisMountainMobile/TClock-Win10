/*-------------------------------------------
  pageabout.c
     「バージョン情報」
     KAZUBON 1997-1998
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);

static void OnLinkClicked(HWND hDlg, UINT id);

static HFONT hfontLink;  //リンクラベルフォント
// リンクラベル用サブクラスプロシージャ
static WNDPROC oldLabProc = NULL;
LRESULT CALLBACK LabLinkProc(HWND, UINT, WPARAM, LPARAM);
static HCURSOR hCurHand = NULL;

//#define SendPSChanged(hDlg) SendMessage(GetParent(hDlg),PSM_CHANGED,(WPARAM)(hDlg),0)

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyClock = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

extern char g_mydir[];

extern BOOL b_EnglishMenu;
extern int Language_Offset;

/*------------------------------------------------
　「バージョン情報」ページ用ダイアログプロシージャ
--------------------------------------------------*/

INT_PTR CALLBACK PageAboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
			switch (id)
			{
				//  iniファイルをエディット
			case IDC_EDITINI:
			{
				char fname[1024];
				strcpy(fname, g_mydir);
				add_title(fname, "tclock-win10.ini");
				//ShellExecute(NULL, "edit", fname, NULL, NULL, SW_SHOWNORMAL);
				ShellExecute(NULL, "open", "notepad.exe", fname, NULL, SW_SHOWNORMAL);
			}
				break;
				//  readme.txtを開く
			case IDC_README2:
			{
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

			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch (((NMHDR *)lParam)->code)
			{
			case PSN_APPLY: 
				OnApply(hDlg); 
				break;
			case PSN_HELP:
				My2chHelp(GetParent(hDlg));
				break;
			}
			return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------
  initialize
--------------------------------------------------*/
static void OnInit(HWND hDlg)
{


}

/*------------------------------------------------
  "Apply" button
--------------------------------------------------*/
void OnApply(HWND hDlg)
{


}
