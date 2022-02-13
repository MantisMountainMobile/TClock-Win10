/*-------------------------------------------
  pageabarmeter.c
     「メーターバー設定」
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

extern BOOL b_EnglishMenu;
extern int Language_Offset;

/*------------------------------------------------
　「バージョン情報」ページ用ダイアログプロシージャ
--------------------------------------------------*/

INT_PTR CALLBACK PageBarmeterProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
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

	CheckDlgButton(hDlg, IDC_USEBARVOL,
		GetMyRegLong("BarMeter", "UseBarMeterVL", FALSE));
	CheckDlgButton(hDlg, IDC_BARVOLHORIZONTAL,
		GetMyRegLong("BarMeter", "BarMeterVL_Horizontal", FALSE));


	SendDlgItemMessage(hDlg, IDC_SPGBARVOLRIGHT, UDM_SETRANGE, 0,
		MAKELONG(500, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLRIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterVL_Right", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLWIDTH, UDM_SETRANGE, 0,
		MAKELONG(500, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLWIDTH, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterVL_Width", 5));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLBOTTOM, UDM_SETRANGE, 0,
		MAKELONG(100, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLBOTTOM, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterVL_Bottom", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLHEIGHT, UDM_SETRANGE, 0,
		MAKELONG(100, -1));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLHEIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterVL_Height", -1));

	CheckDlgButton(hDlg, IDC_USEBARCPU,
		GetMyRegLong("BarMeter", "UseBarMeterCU", FALSE));
	CheckDlgButton(hDlg, IDC_BARCPUHORIZONTAL,
		GetMyRegLong("BarMeter", "BarMeterCU_Horizontal", FALSE));

	SendDlgItemMessage(hDlg, IDC_SPGBARCPURIGHT, UDM_SETRANGE, 0,
		MAKELONG(500, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPURIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Right", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUWIDTH, UDM_SETRANGE, 0,
		MAKELONG(500, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUWIDTH, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Width", 5));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUBOTTOM, UDM_SETRANGE, 0,
		MAKELONG(100, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUBOTTOM, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Bottom", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUHEIGHT, UDM_SETRANGE, 0,
		MAKELONG(100, -1));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUHEIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Height", -1));

	CheckDlgButton(hDlg, IDC_USEBARBAT,
		GetMyRegLong("BarMeter", "UseBarMeterBL", FALSE));
	CheckDlgButton(hDlg, IDC_BARBATHORIZONTAL,
		GetMyRegLong("BarMeter", "BarMeterBL_Horizontal", FALSE));


	if (!GetMyRegLong("Status_DoNotEdit", "BatteryLifeAvailable", TRUE))
	{
		EnableDlgItem(hDlg, IDC_USEBARBAT, FALSE);
	}
	else
	{
		EnableDlgItem(hDlg, IDC_USEBARBAT, TRUE);
	}

	SendDlgItemMessage(hDlg, IDC_SPGBARBATRIGHT, UDM_SETRANGE, 0,
		MAKELONG(500, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATRIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Right", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATWIDTH, UDM_SETRANGE, 0,
		MAKELONG(500, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATWIDTH, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Width", 5));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATBOTTOM, UDM_SETRANGE, 0,
		MAKELONG(100, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATBOTTOM, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Bottom", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATHEIGHT, UDM_SETRANGE, 0,
		MAKELONG(100, -1));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATHEIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Height", -1));


	CheckDlgButton(hDlg, IDC_USEBARNET,
		GetMyRegLong("BarMeter", "UseBarMeterNet", FALSE));

	CheckDlgButton(hDlg, IDC_BARNETLOGGRAPH, 
		GetMyRegLong("BarMeter", "BarMeterNet_LogGraph", FALSE));

	CheckDlgButton(hDlg, IDC_BARNETHORIZONTAL,
		GetMyRegLong("BarMeter", "BarMeterNet_Horizontal", FALSE));

	//CheckDlgButton(hDlg, IDC_BARNETFLOAT,
	//	GetMyRegLong("BarMeter", "BarMeterNet_Float", FALSE));

	SendDlgItemMessage(hDlg, IDC_SPGBARNETSENDRIGHT, UDM_SETRANGE, 0,
		MAKELONG(500, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETSENDRIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNetSend_Right", 300));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETSENDBOTTOM, UDM_SETRANGE, 0,
		MAKELONG(100, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETSENDBOTTOM, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNetSend_Bottom", 300));

	SendDlgItemMessage(hDlg, IDC_SPGBARNETRCVRIGHT, UDM_SETRANGE, 0,
		MAKELONG(500, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETRCVRIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNetRecv_Right", 310));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETRCVBOTTOM, UDM_SETRANGE, 0,
		MAKELONG(100, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETRCVBOTTOM, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNetRecv_Bottom", 310));

	SendDlgItemMessage(hDlg, IDC_SPGBARNETWIDTH, UDM_SETRANGE, 0,
		MAKELONG(500, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETWIDTH, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNet_Width", 5));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETHEIGHT, UDM_SETRANGE, 0,
		MAKELONG(100, -1));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETHEIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNet_Height", -1));




}

/*------------------------------------------------
  "Apply" button
--------------------------------------------------*/
void OnApply(HWND hDlg)
{

	SetMyRegLong("BarMeter", "UseBarMeterVL",
		IsDlgButtonChecked(hDlg, IDC_USEBARVOL));
	SetMyRegLong("BarMeter", "BarMeterVL_Horizontal", IsDlgButtonChecked(hDlg, IDC_BARVOLHORIZONTAL));

	SetMyRegLong("BarMeter", "BarMeterVL_Right",
		SendDlgItemMessage(hDlg, IDC_SPGBARVOLRIGHT, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterVL_Width",
		SendDlgItemMessage(hDlg, IDC_SPGBARVOLWIDTH, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterVL_Bottom",
		SendDlgItemMessage(hDlg, IDC_SPGBARVOLBOTTOM, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterVL_Height",
		SendDlgItemMessage(hDlg, IDC_SPGBARVOLHEIGHT, UDM_GETPOS, 0, 0));


	SetMyRegLong("BarMeter", "UseBarMeterCU",
		IsDlgButtonChecked(hDlg, IDC_USEBARCPU));
	SetMyRegLong("BarMeter", "BarMeterCU_Horizontal", IsDlgButtonChecked(hDlg, IDC_BARCPUHORIZONTAL));

	SetMyRegLong("BarMeter", "BarMeterCU_Right",
		SendDlgItemMessage(hDlg, IDC_SPGBARCPURIGHT, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterCU_Width",
		SendDlgItemMessage(hDlg, IDC_SPGBARCPUWIDTH, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterCU_Bottom",
		SendDlgItemMessage(hDlg, IDC_SPGBARCPUBOTTOM, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterCU_Height",
		SendDlgItemMessage(hDlg, IDC_SPGBARCPUHEIGHT, UDM_GETPOS, 0, 0));


	SetMyRegLong("BarMeter", "UseBarMeterBL",
		IsDlgButtonChecked(hDlg, IDC_USEBARBAT));
	SetMyRegLong("BarMeter", "BarMeterBL_Horizontal", IsDlgButtonChecked(hDlg, IDC_BARBATHORIZONTAL));

	SetMyRegLong("BarMeter", "BarMeterBL_Right",
		SendDlgItemMessage(hDlg, IDC_SPGBARBATRIGHT, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterBL_Width",
		SendDlgItemMessage(hDlg, IDC_SPGBARBATWIDTH, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterBL_Bottom",
		SendDlgItemMessage(hDlg, IDC_SPGBARBATBOTTOM, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterBL_Height",
		SendDlgItemMessage(hDlg, IDC_SPGBARBATHEIGHT, UDM_GETPOS, 0, 0));



	SetMyRegLong("BarMeter", "UseBarMeterNet",
		IsDlgButtonChecked(hDlg, IDC_USEBARNET));

	SetMyRegLong("BarMeter", "BarMeterNet_LogGraph", IsDlgButtonChecked(hDlg, IDC_BARNETLOGGRAPH));

	SetMyRegLong("BarMeter", "BarMeterNet_Horizontal", IsDlgButtonChecked(hDlg, IDC_BARNETHORIZONTAL));

	//SetMyRegLong("BarMeter", "BarMeterNet_Float", IsDlgButtonChecked(hDlg, IDC_BARNETFLOAT));

	SetMyRegLong("BarMeter", "BarMeterNet_Width",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETWIDTH, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterNet_Height",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETHEIGHT, UDM_GETPOS, 0, 0));

	SetMyRegLong("BarMeter", "BarMeterNetSend_Right",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETSENDRIGHT, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterNetSend_Bottom",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETSENDBOTTOM, UDM_GETPOS, 0, 0));

	SetMyRegLong("BarMeter", "BarMeterNetRecv_Right",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETRCVRIGHT, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterNetRecv_Bottom",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETRCVBOTTOM, UDM_GETPOS, 0, 0));



}
