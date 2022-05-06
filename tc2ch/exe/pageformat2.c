/*-------------------------------------------
  pageformat2.c
  "Detail of format" dialog
                       KAZUBON 1999
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg, LPARAM lParam);
static void OnOK(HWND hDlg);

extern BOOL b_EnglishMenu;
extern int Language_Offset;

/*------------------------------------------------
   dialog procedure of "Detail of format"
--------------------------------------------------*/
INT_PTR CALLBACK DlgProcFormat2(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg, lParam);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			switch (id)
			{
				case IDOK: OnOK(hDlg); // fall through
				case IDCANCEL: EndDialog(hDlg, id);
					break;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/*------------------------------------------------
  initialize the dialog
--------------------------------------------------*/
void OnInit(HWND hDlg, LPARAM lParam)
{
	char s[80], s2[11];
	int ilang;

	ilang = (int)lParam;

	// "AM Symbol" and "PM Symbol"
	CBResetContent(hDlg, IDC_AMSYMBOL);
	GetMyRegStr("Format", "AMsymbol", s, 80, "");
	if(s[0]) CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)s);
	GetLocaleInfoWA(ilang, LOCALE_S1159, s2, 10);
	if(s2[0] && strcmp(s, s2) != 0)
		CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)s2);
	if(strcmp(s, "AM") != 0 && strcmp(s2, "AM") != 0)
		CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)"AM");
	if(strcmp(s, "am") != 0 && strcmp(s2, "am") != 0)
		CBAddString(hDlg, IDC_AMSYMBOL, (LPARAM)"am");
	CBSetCurSel(hDlg, IDC_AMSYMBOL, 0);

	CBResetContent(hDlg, IDC_PMSYMBOL);
	GetMyRegStr("Format", "PMsymbol", s, 80, "");
	if(s[0]) CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)s);
	GetLocaleInfoWA(ilang, LOCALE_S2359, s2, 10);
	if(s2[0] && strcmp(s, s2) != 0)
		CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)s2);
	if(strcmp(s, "PM") != 0 && strcmp(s2, "PM") != 0)
		CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)"PM");
	if(strcmp(s, "pm") != 0 && strcmp(s2, "pm") != 0)
		CBAddString(hDlg, IDC_PMSYMBOL, (LPARAM)"pm");
	CBSetCurSel(hDlg, IDC_PMSYMBOL, 0);

	CheckDlgButton(hDlg, IDC_ZERO,
		GetMyRegLong("Format", "HourZero", FALSE));
}

void OnOK(HWND hDlg)
{
	char s[80];

	GetDlgItemText(hDlg, IDC_AMSYMBOL, s, 1024);
	SetMyRegStr("Format", "AMsymbol", s);
	GetDlgItemText(hDlg, IDC_PMSYMBOL, s, 1024);
	SetMyRegStr("Format", "PMsymbol", s);

	SetMyRegLong("Format", "HourZero",
		IsDlgButtonChecked(hDlg, IDC_ZERO));
}
