/*-------------------------------------------
  pageetc1.c
     �u���̂ق��\���ݒ�v
     by TT 20181103
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);


__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyClock = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

extern char g_mydir[];

/*------------------------------------------------
�@�u�o�[�W�������v�y�[�W�p�_�C�A���O�v���V�[�W��
--------------------------------------------------*/

INT_PTR CALLBACK PageEtc1Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
			//case IDC_EDITINI:
			//{
			//	char fname[1024];
			//	strcpy(fname, g_mydir);
			//	add_title(fname, "tclock-win10.ini");
			//	ShellExecute(NULL, "edit", fname, NULL, NULL, SW_SHOWNORMAL);
			//}
			//	break;
			//	//  readme.txt���J��
			//case IDC_README2:
			//{
			//	char fname[1024];
			//	strcpy(fname, g_mydir);
			//	add_title(fname, "readme.txt");
			//	ShellExecute(NULL, "open", fname, NULL, NULL, SW_SHOWNORMAL);
			//}
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
				My2chHelp(GetParent(hDlg), 12);
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
