/*-------------------------------------------
  pagedataplan.c
     「データ利用状況設定」
     by TTTT
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

//BOOL b_exe_Win11Main = FALSE;
BOOL b_exe_UseSubClks = TRUE;
//BOOL b_exe_UseWin11Notify = TRUE;
//int exe_AdjustTrayCutPosition = 0;
//int exe_AdjustWin11ClockWidth = 0;
//int exe_AdjutDetectNotify = 0;

extern BOOL b_AutoRestart;

extern BOOL b_EnglishMenu;
extern int Language_Offset;


/*------------------------------------------------
　「バージョン情報」ページ用ダイアログプロシージャ
--------------------------------------------------*/

INT_PTR CALLBACK PageDataPlanProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
	char str[32];

	CheckDlgButton(hDlg, IDC_ETC_AUTORESTART, b_AutoRestart);
	
	b_exe_UseSubClks = GetMyRegLong(NULL, "EnableOnSubDisplay", TRUE);
	CheckDlgButton(hDlg, IDC_USE_SUBCLKS, b_exe_UseSubClks);

	//CheckDlgButton(hDlg, IDC_ENHANCE_SUBCLK, GetMyRegLong(NULL, "EnhanceSubClkOnDarkTray", FALSE));


	GetMyRegStr("ETC", "Ethernet_Keyword1", str, 32, "");
	SetDlgItemText(hDlg, IDC_ETHERNET_KEYWORD1, str);

	GetMyRegStr("ETC", "Ethernet_Keyword2", str, 32, "");
	SetDlgItemText(hDlg, IDC_ETHERNET_KEYWORD2, str);

	GetMyRegStr("ETC", "Ethernet_Keyword3", str, 32, "");
	SetDlgItemText(hDlg, IDC_ETHERNET_KEYWORD3, str);

	GetMyRegStr("ETC", "Ethernet_Keyword4", str, 32, "");
	SetDlgItemText(hDlg, IDC_ETHERNET_KEYWORD4, str);

	GetMyRegStr("ETC", "Ethernet_Keyword5", str, 32, "");
	SetDlgItemText(hDlg, IDC_ETHERNET_KEYWORD5, str);


	GetMyRegStr("VPN", "VPN_Keyword1", str, 32, "");
	SetDlgItemText(hDlg, IDC_VPN_KEYWORD1, str);

	GetMyRegStr("VPN", "VPN_Keyword2", str, 32, "");
	SetDlgItemText(hDlg, IDC_VPN_KEYWORD2, str);

	GetMyRegStr("VPN", "VPN_Keyword3", str, 32, "");
	SetDlgItemText(hDlg, IDC_VPN_KEYWORD3, str);

	GetMyRegStr("VPN", "VPN_Keyword4", str, 32, "");
	SetDlgItemText(hDlg, IDC_VPN_KEYWORD4, str);

	GetMyRegStr("VPN", "VPN_Keyword5", str, 32, "");
	SetDlgItemText(hDlg, IDC_VPN_KEYWORD5, str);


	GetMyRegStr("VPN", "VPN_Exclude1", str, 32, "");
	SetDlgItemText(hDlg, IDC_VPN_EXCLUDE1, str);

	GetMyRegStr("VPN", "VPN_Exclude2", str, 32, "");
	SetDlgItemText(hDlg, IDC_VPN_EXCLUDE2, str);

	GetMyRegStr("VPN", "VPN_Exclude3", str, 32, "");
	SetDlgItemText(hDlg, IDC_VPN_EXCLUDE3, str);

	GetMyRegStr("VPN", "VPN_Exclude4", str, 32, "");
	SetDlgItemText(hDlg, IDC_VPN_EXCLUDE4, str);

	GetMyRegStr("VPN", "VPN_Exclude5", str, 32, "");
	SetDlgItemText(hDlg, IDC_VPN_EXCLUDE5, str);



}

/*------------------------------------------------
  "Apply" button
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char str[32];

	SetMyRegLong(NULL, "EnableOnSubDisplay", IsDlgButtonChecked(hDlg, IDC_USE_SUBCLKS));

	//SetMyRegLong(NULL, "EnhanceSubClkOnDarkTray", IsDlgButtonChecked(hDlg, IDC_ENHANCE_SUBCLK));

	b_AutoRestart = IsDlgButtonChecked(hDlg, IDC_ETC_AUTORESTART);
	SetMyRegLong(NULL, "AutoRestart", b_AutoRestart);




	GetDlgItemText(hDlg, IDC_ETHERNET_KEYWORD1, str, 32);
	SetMyRegStr("ETC", "Ethernet_Keyword1", str);

	GetDlgItemText(hDlg, IDC_ETHERNET_KEYWORD2, str, 32);
	SetMyRegStr("ETC", "Ethernet_Keyword2", str);

	GetDlgItemText(hDlg, IDC_ETHERNET_KEYWORD3, str, 32);
	SetMyRegStr("ETC", "Ethernet_Keyword3", str);

	GetDlgItemText(hDlg, IDC_ETHERNET_KEYWORD4, str, 32);
	SetMyRegStr("ETC", "Ethernet_Keyword4", str);

	GetDlgItemText(hDlg, IDC_ETHERNET_KEYWORD5, str, 32);
	SetMyRegStr("ETC", "Ethernet_Keyword5", str);


	GetDlgItemText(hDlg, IDC_VPN_KEYWORD1, str, 32);
	SetMyRegStr("VPN", "VPN_Keyword1", str);

	GetDlgItemText(hDlg, IDC_VPN_KEYWORD2, str, 32);
	SetMyRegStr("VPN", "VPN_Keyword2", str);

	GetDlgItemText(hDlg, IDC_VPN_KEYWORD3, str, 32);
	SetMyRegStr("VPN", "VPN_Keyword3", str);

	GetDlgItemText(hDlg, IDC_VPN_KEYWORD4, str, 32);
	SetMyRegStr("VPN", "VPN_Keyword4", str);

	GetDlgItemText(hDlg, IDC_VPN_KEYWORD5, str, 32);
	SetMyRegStr("VPN", "VPN_Keyword5", str);


	GetDlgItemText(hDlg, IDC_VPN_EXCLUDE1, str, 32);
	SetMyRegStr("VPN", "VPN_Exclude1", str);

	GetDlgItemText(hDlg, IDC_VPN_EXCLUDE2, str, 32);
	SetMyRegStr("VPN", "VPN_Exclude2", str);

	GetDlgItemText(hDlg, IDC_VPN_EXCLUDE3, str, 32);
	SetMyRegStr("VPN", "VPN_Exclude3", str);

	GetDlgItemText(hDlg, IDC_VPN_EXCLUDE4, str, 32);
	SetMyRegStr("VPN", "VPN_Exclude4", str);

	GetDlgItemText(hDlg, IDC_VPN_EXCLUDE5, str, 32);
	SetMyRegStr("VPN", "VPN_Exclude5", str);
}

