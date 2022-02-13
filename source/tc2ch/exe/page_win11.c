/*-------------------------------------------
  pagedataplan.c
     「データ利用状況設定」
     by TTTT
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
//////void DefineDaysRange_Win10(HWND hDlg);

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyClock = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

extern char g_mydir[];

BOOL b_exe_Win11Main = FALSE;
//BOOL b_exe_UseSubClks = TRUE;
BOOL b_exe_UseWin11Notify = TRUE;
int exe_AdjustTrayCutPosition = 0;
int exe_AdjustWin11ClockWidth = 0;
int exe_AdjutDetectNotify = 0;
BOOL b_exe_AdjustTrayWin11SmallTaskbar = TRUE;


//extern BOOL b_AutoRestart;

extern BOOL b_EnglishMenu;
extern int Language_Offset;


/*------------------------------------------------
　「バージョン情報」ページ用ダイアログプロシージャ
--------------------------------------------------*/

INT_PTR CALLBACK PageWin11Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
			//case IDC_SPGDATAPLANMONTH:
			//case IDC_DATAPLANMONTH:
			//	month = SendDlgItemMessage(hDlg, IDC_SPGDATAPLANMONTH, UDM_GETPOS, 0, 0);
			//	//////DefineDaysRange_Win10(hDlg);
			//	break;
			//case IDC_DATAPLANFIXEDDAY:
			//	b_FixedDayInMonth = IsDlgButtonChecked(hDlg, IDC_DATAPLANFIXEDDAY);
			//	break;
			//case IDC_DATAPLANGB1024MB:
			//	b_1024MBinGB = IsDlgButtonChecked(hDlg, IDC_DATAPLANGB1024MB);
			//	break;
			//case IDC_DATAPLANGB1000MB:
			//	b_1024MBinGB = !IsDlgButtonChecked(hDlg, IDC_DATAPLANGB1000MB);
			//	break;
			//case IDC_DATAPLANAUTOFIND:
			//	DataPlanUsageRetrieveMode = 0;
			//	break;
			//case IDC_DATAPLANALWAYSACTIVE:
			//	DataPlanUsageRetrieveMode = 1;
			//	break;
			//case IDC_DATAPLANSUGGESTPROF:
			//	DataPlanUsageRetrieveMode = 2;
			//	break;
			//case IDC_PROFILELIST:
			//	{
			//		PostMessage(g_hwndClock, CLOCKM_SHOWPROFILELIST, 0, 0);
			//	}
			//	break;
			}
			SendPSChanged(hDlg);
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

//	CheckDlgButton(hDlg, IDC_ETC_AUTORESTART, b_AutoRestart);
	
	//b_exe_UseSubClks = GetMyRegLong(NULL, "EnableOnSubDisplay", TRUE);
	//CheckDlgButton(hDlg, IDC_USE_SUBCLKS, b_exe_UseSubClks);

	b_exe_Win11Main = GetMyRegLong("Status_DoNotEdit", "Win11TClockMain", 9);



	if (b_exe_Win11Main)
	{
		b_exe_UseWin11Notify = (BOOL)GetMyRegLong("Win11", "UseTClockNotify", 1);
		CheckDlgButton(hDlg, IDC_ETC_USE_WIN11NOTIFY, b_exe_UseWin11Notify);

		CheckDlgButton(hDlg, IDC_ETC_SHOW_WIN11NOTIFY_NUMBER, (BOOL)GetMyRegLong("Win11", "ShowWin11NotifyNumber", 1));


		int IconSize = GetMyRegLong("Status_DoNotEdit", "Win11IconSize", 99);

		exe_AdjustTrayCutPosition = (int)(short)GetMyRegLong("Win11", "AdjustCutTray", 0);
		SendDlgItemMessage(hDlg, IDC_SPG_ETC_CUTPOSITION, UDM_SETRANGE, 0, MAKELONG(IconSize, -IconSize));
		SendDlgItemMessage(hDlg, IDC_SPG_ETC_CUTPOSITION, UDM_SETPOS, 0, exe_AdjustTrayCutPosition);

		exe_AdjustWin11ClockWidth = (int)(short)GetMyRegLong("Win11", "AdjustWin11ClockWidth", 0);
		SendDlgItemMessage(hDlg, IDC_SPG_ETC_CUT_LIMIT, UDM_SETRANGE, 0, MAKELONG(IconSize, -IconSize));
		SendDlgItemMessage(hDlg, IDC_SPG_ETC_CUT_LIMIT, UDM_SETPOS, 0, exe_AdjustWin11ClockWidth);

		exe_AdjutDetectNotify = (int)(short)GetMyRegLong("Win11", "AdjustDetectNotify", 0);
		SendDlgItemMessage(hDlg, IDC_SPG_ETC_NOTIFY_DETECTPOS, UDM_SETRANGE, 0, MAKELONG(IconSize, -IconSize));
		SendDlgItemMessage(hDlg, IDC_SPG_ETC_NOTIFY_DETECTPOS, UDM_SETPOS, 0, exe_AdjutDetectNotify);

		EnableDlgItem(hDlg, IDC_SPG_ETC_CUT_LIMIT, FALSE);
		EnableDlgItem(hDlg, IDC_ETC_CUT_LIMIT, FALSE);
		EnableDlgItem(hDlg, IDC_SPG_ETC_NOTIFY_DETECTPOS, FALSE);
		EnableDlgItem(hDlg, IDC_ETC_NOTIFY_DETECTPOS, FALSE);

		b_exe_AdjustTrayWin11SmallTaskbar = (BOOL)GetMyRegLong("Win11", "AdjustWin11IconPosition", 1);
		CheckDlgButton(hDlg, IDC_ETC_ADJUST_WIN11_SMALLTASKBAR, (BOOL)GetMyRegLong("Win11", "AdjustWin11IconPosition", 1));

	}
	else {
		EnableDlgItem(hDlg, IDC_ETC_USE_WIN11NOTIFY, FALSE);
		EnableDlgItem(hDlg, IDC_ETC_SHOW_WIN11NOTIFY_NUMBER, FALSE);
		EnableDlgItem(hDlg, IDC_SPG_ETC_CUTPOSITION, FALSE);
		EnableDlgItem(hDlg, IDC_ETC_CUTPOSITION, FALSE);
		EnableDlgItem(hDlg, IDC_SPG_ETC_CUT_LIMIT, FALSE);
		EnableDlgItem(hDlg, IDC_ETC_CUT_LIMIT, FALSE);
		EnableDlgItem(hDlg, IDC_SPG_ETC_NOTIFY_DETECTPOS, FALSE);
		EnableDlgItem(hDlg, IDC_ETC_NOTIFY_DETECTPOS, FALSE);

		EnableDlgItem(hDlg, IDC_ETC_ADJUST_WIN11_SMALLTASKBAR, FALSE);
	}


}

/*------------------------------------------------
  "Apply" button
--------------------------------------------------*/
void OnApply(HWND hDlg)
{


	//SetMyRegLong(NULL, "EnableOnSubDisplay", IsDlgButtonChecked(hDlg, IDC_USE_SUBCLKS));

	//b_AutoRestart = IsDlgButtonChecked(hDlg, IDC_ETC_AUTORESTART);
	//SetMyRegLong(NULL, "AutoRestart", b_AutoRestart);

	SetMyRegLong("Win11", "UseTClockNotify", IsDlgButtonChecked(hDlg, IDC_ETC_USE_WIN11NOTIFY));
	SetMyRegLong("Win11", "ShowWin11NotifyNumber", IsDlgButtonChecked(hDlg, IDC_ETC_SHOW_WIN11NOTIFY_NUMBER));


	SetMyRegLong("Win11", "AdjustCutTray", (int)(short)SendDlgItemMessage(hDlg, IDC_SPG_ETC_CUTPOSITION, UDM_GETPOS, 0, 0));
	SetMyRegLong("Win11", "AdjustWin11ClockWidth", (int)(short)SendDlgItemMessage(hDlg, IDC_SPG_ETC_CUT_LIMIT, UDM_GETPOS, 0, 0));
	SetMyRegLong("Win11", "AdjustDetectNotify", (int)(short)SendDlgItemMessage(hDlg, IDC_SPG_ETC_NOTIFY_DETECTPOS, UDM_GETPOS, 0, 0));

	SetMyRegLong("Win11", "AdjustWin11IconPosition", IsDlgButtonChecked(hDlg, IDC_ETC_ADJUST_WIN11_SMALLTASKBAR));

}

