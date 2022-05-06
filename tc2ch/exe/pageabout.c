/*-------------------------------------------
  pageabout.c
     �u�o�[�W�������v
     KAZUBON 1997-1998
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);

static void OnLinkClicked(HWND hDlg, UINT id);

static HFONT hfontLink;  //�����N���x���t�H���g
// �����N���x���p�T�u�N���X�v���V�[�W��
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
�@�u�o�[�W�������v�y�[�W�p�_�C�A���O�v���V�[�W��
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
				//  ini�t�@�C�����G�f�B�b�g
			case IDC_EDITINI:
			{
				char fname[1024];
				strcpy(fname, g_mydir);
				add_title(fname, "tclock-win10.ini");
				//ShellExecute(NULL, "edit", fname, NULL, NULL, SW_SHOWNORMAL);
				ShellExecute(NULL, "open", "notepad.exe", fname, NULL, SW_SHOWNORMAL);
			}
				break;
				//  readme.txt���J��
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
