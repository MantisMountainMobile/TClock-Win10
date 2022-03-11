/*-------------------------------------------
  pageabarmeter.c
     「メーターバー設定」
     by TTTT
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void ArrangeItems(HWND hDlg);

INT_PTR CALLBACK DlgProcBarmeterColor(HWND, UINT, WPARAM, LPARAM);
void CreateBarMeterColorDialog(int index);


__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyClock = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

extern char g_mydir[];

extern BOOL b_EnglishMenu;
extern int Language_Offset;

static HWND hDlgBarmeterColor;
int BarMeterColorDlgIndex = 0;

void CreateBarMeterColorDialog(int index)
{
	BarMeterColorDlgIndex = index;
	DialogBox(GetLangModule(), MAKEINTRESOURCE(0 + IDD_BARMETER_COLOR), g_hwndPropDlg, DlgProcBarmeterColor);
}



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
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			switch (id)
			{
			case IDC_BARVOLHORIZONTAL:
			case IDC_BARCPUHORIZONTAL:
			case IDC_BARBATHORIZONTAL:
			case IDC_BARNETHORIZONTAL:
				if (code == BN_CLICKED)
				{
					ArrangeItems(hDlg);
				}
				break;
			case IDC_BARMETERCOL_VOL:
			case IDC_BARMETERCOL_CPU:
			case IDC_BARMETERCOL_BAT:
			case IDC_BARMETERCOL_NET:
				if (code == BN_CLICKED)
				{
					CreateBarMeterColorDialog(id);
				}
				break;
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

	char tempStr[64];
	int tempWidth = 1000, tempHeight = 100;;

	tempWidth = (int)GetMyRegLong("Status_DoNotEdit", "ClockWidth", 0);
	tempHeight = (int)GetMyRegLong("Status_DoNotEdit", "ClockHeight", 0);
	wsprintf(tempStr, "Current Clock Width = %d, Clock Height = %d"
		, tempWidth, tempHeight);
	SendDlgItemMessage(hDlg, IDC_LABEL_HEIGHT, WM_SETTEXT, NULL, tempStr);


	CheckDlgButton(hDlg, IDC_USEBARVOL,
		GetMyRegLong("BarMeter", "UseBarMeterVL", FALSE));
	CheckDlgButton(hDlg, IDC_BARVOLHORIZONTAL,
		GetMyRegLong("BarMeter", "BarMeterVL_Horizontal", FALSE));


	SendDlgItemMessage(hDlg, IDC_SPGBARVOLRIGHT, UDM_SETRANGE, 0,
		MAKELONG(tempWidth, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLRIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterVL_Right", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLWIDTH, UDM_SETRANGE, 0,
		MAKELONG(tempWidth, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLWIDTH, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterVL_Width", 5));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLBOTTOM, UDM_SETRANGE, 0,
		MAKELONG(tempHeight, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLBOTTOM, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterVL_Bottom", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLHEIGHT, UDM_SETRANGE, 0,
		MAKELONG(tempHeight, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARVOLHEIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterVL_Top", 0));



	CheckDlgButton(hDlg, IDC_USEBARCPU,
		GetMyRegLong("BarMeter", "UseBarMeterCU", FALSE));
	CheckDlgButton(hDlg, IDC_BARCPUHORIZONTAL,
		GetMyRegLong("BarMeter", "BarMeterCU_Horizontal", FALSE));

	SendDlgItemMessage(hDlg, IDC_SPGBARCPURIGHT, UDM_SETRANGE, 0,
		MAKELONG(tempWidth, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPURIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Right", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUWIDTH, UDM_SETRANGE, 0,
		MAKELONG(tempWidth, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUWIDTH, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Width", 5));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUBOTTOM, UDM_SETRANGE, 0,
		MAKELONG(tempHeight, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUBOTTOM, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Bottom", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUHEIGHT, UDM_SETRANGE, 0,
		MAKELONG(tempHeight, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUHEIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Top", 0));



	CheckDlgButton(hDlg, IDC_USEBARCPU2,
		GetMyRegLong("BarMeter", "UseBarMeterGU", FALSE));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPURIGHT2, UDM_SETRANGE, 0,
		MAKELONG(tempWidth, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPURIGHT2, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterGU_Right", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUBOTTOM2, UDM_SETRANGE, 0,
		MAKELONG(tempHeight, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUBOTTOM2, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterGU_Bottom", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUHEIGHT2, UDM_SETRANGE, 0,
		MAKELONG(tempHeight, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARCPUHEIGHT2, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterGU_Top", 0));








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
		MAKELONG(tempWidth, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATRIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Right", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATWIDTH, UDM_SETRANGE, 0,
		MAKELONG(tempWidth, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATWIDTH, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Width", 5));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATBOTTOM, UDM_SETRANGE, 0,
		MAKELONG(tempHeight, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATBOTTOM, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Bottom", 290));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATHEIGHT, UDM_SETRANGE, 0,
		MAKELONG(tempHeight, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARBATHEIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Top", 0));


	CheckDlgButton(hDlg, IDC_USEBARNET,
		GetMyRegLong("BarMeter", "UseBarMeterNet", FALSE));

	CheckDlgButton(hDlg, IDC_BARNETLOGGRAPH, 
		GetMyRegLong("BarMeter", "BarMeterNet_LogGraph", FALSE));

	CheckDlgButton(hDlg, IDC_BARNETHORIZONTAL,
		GetMyRegLong("BarMeter", "BarMeterNet_Horizontal", FALSE));

	//CheckDlgButton(hDlg, IDC_BARNETFLOAT,
	//	GetMyRegLong("BarMeter", "BarMeterNet_Float", FALSE));

	SendDlgItemMessage(hDlg, IDC_SPGBARNETSENDRIGHT, UDM_SETRANGE, 0,
		MAKELONG(tempWidth, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETSENDRIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNetSend_Right", 300));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETSENDBOTTOM, UDM_SETRANGE, 0,
		MAKELONG(tempHeight, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETSENDBOTTOM, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNetSend_Bottom", 300));

	SendDlgItemMessage(hDlg, IDC_SPGBARNETRCVRIGHT, UDM_SETRANGE, 0,
		MAKELONG(tempWidth, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETRCVRIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNetRecv_Right", 310));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETRCVBOTTOM, UDM_SETRANGE, 0,
		MAKELONG(tempHeight, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETRCVBOTTOM, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNetRecv_Bottom", 310));

	SendDlgItemMessage(hDlg, IDC_SPGBARNETWIDTH, UDM_SETRANGE, 0,
		MAKELONG(tempWidth, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETWIDTH, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNet_Width", 5));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETHEIGHT, UDM_SETRANGE, 0,
		MAKELONG(tempHeight, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETHEIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNetRecv_Top", 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETHEIGHT2, UDM_SETRANGE, 0,
		MAKELONG(tempHeight, 0));
	SendDlgItemMessage(hDlg, IDC_SPGBARNETHEIGHT2, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("BarMeter", "BarMeterNetSend_Top", 0));


	CheckDlgButton(hDlg, IDC_BARHORIZ_LEFT_VL,
		GetMyRegLong("BarMeter", "BarMeterVL_HorizontalToLeft", FALSE));
	CheckDlgButton(hDlg, IDC_BARHORIZ_LEFT_BL,
		GetMyRegLong("BarMeter", "BarMeterBL_HorizontalToLeft", FALSE));
	CheckDlgButton(hDlg, IDC_BARHORIZ_LEFT_CU,
		GetMyRegLong("BarMeter", "BarMeterCU_HorizontalToLeft", FALSE));
	CheckDlgButton(hDlg, IDC_BARHORIZ_LEFT_NET,
		GetMyRegLong("BarMeter", "BarMeterNet_HorizontalToLeft", FALSE));



	ArrangeItems(hDlg);

}


static void ArrangeItems(HWND hDlg)
{
	if (IsDlgButtonChecked(hDlg, IDC_BARVOLHORIZONTAL)) {
		ShowDlgItem(hDlg, IDC_BARHORIZ_LEFT_VL, TRUE);
	}
	else {
		ShowDlgItem(hDlg, IDC_BARHORIZ_LEFT_VL, FALSE);
	}

	if (IsDlgButtonChecked(hDlg, IDC_BARBATHORIZONTAL)) {
		ShowDlgItem(hDlg, IDC_BARHORIZ_LEFT_BL, TRUE);
	}
	else {
		ShowDlgItem(hDlg, IDC_BARHORIZ_LEFT_BL, FALSE);
	}

	if (IsDlgButtonChecked(hDlg, IDC_BARCPUHORIZONTAL)) {
		ShowDlgItem(hDlg, IDC_BARHORIZ_LEFT_CU, TRUE);
	}
	else {
		ShowDlgItem(hDlg, IDC_BARHORIZ_LEFT_CU, FALSE);
	}

	if (IsDlgButtonChecked(hDlg, IDC_BARNETHORIZONTAL)) {
		ShowDlgItem(hDlg, IDC_BARHORIZ_LEFT_NET, TRUE);
	}
	else {
		ShowDlgItem(hDlg, IDC_BARHORIZ_LEFT_NET, FALSE);
	}

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
	SetMyRegLong("BarMeter", "BarMeterVL_Top",
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
	SetMyRegLong("BarMeter", "BarMeterCU_Top",
		SendDlgItemMessage(hDlg, IDC_SPGBARCPUHEIGHT, UDM_GETPOS, 0, 0));


	SetMyRegLong("BarMeter", "UseBarMeterGU",
		IsDlgButtonChecked(hDlg, IDC_USEBARCPU2));

	SetMyRegLong("BarMeter", "BarMeterGU_Right",
		SendDlgItemMessage(hDlg, IDC_SPGBARCPURIGHT2, UDM_GETPOS, 0, 0));

	SetMyRegLong("BarMeter", "BarMeterGU_Bottom",
		SendDlgItemMessage(hDlg, IDC_SPGBARCPUBOTTOM2, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterGU_Top",
		SendDlgItemMessage(hDlg, IDC_SPGBARCPUHEIGHT2, UDM_GETPOS, 0, 0));



	SetMyRegLong("BarMeter", "UseBarMeterBL",
		IsDlgButtonChecked(hDlg, IDC_USEBARBAT));
	SetMyRegLong("BarMeter", "BarMeterBL_Horizontal", IsDlgButtonChecked(hDlg, IDC_BARBATHORIZONTAL));

	SetMyRegLong("BarMeter", "BarMeterBL_Right",
		SendDlgItemMessage(hDlg, IDC_SPGBARBATRIGHT, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterBL_Width",
		SendDlgItemMessage(hDlg, IDC_SPGBARBATWIDTH, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterBL_Bottom",
		SendDlgItemMessage(hDlg, IDC_SPGBARBATBOTTOM, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterBL_Top",
		SendDlgItemMessage(hDlg, IDC_SPGBARBATHEIGHT, UDM_GETPOS, 0, 0));



	SetMyRegLong("BarMeter", "UseBarMeterNet",
		IsDlgButtonChecked(hDlg, IDC_USEBARNET));

	SetMyRegLong("BarMeter", "BarMeterNet_LogGraph", IsDlgButtonChecked(hDlg, IDC_BARNETLOGGRAPH));

	SetMyRegLong("BarMeter", "BarMeterNet_Horizontal", IsDlgButtonChecked(hDlg, IDC_BARNETHORIZONTAL));

	//SetMyRegLong("BarMeter", "BarMeterNet_Float", IsDlgButtonChecked(hDlg, IDC_BARNETFLOAT));

	SetMyRegLong("BarMeter", "BarMeterNet_Width",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETWIDTH, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterNetRecv_Top",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETHEIGHT, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterNetSend_Top",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETHEIGHT2, UDM_GETPOS, 0, 0));

	SetMyRegLong("BarMeter", "BarMeterNetSend_Right",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETSENDRIGHT, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterNetSend_Bottom",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETSENDBOTTOM, UDM_GETPOS, 0, 0));

	SetMyRegLong("BarMeter", "BarMeterNetRecv_Right",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETRCVRIGHT, UDM_GETPOS, 0, 0));
	SetMyRegLong("BarMeter", "BarMeterNetRecv_Bottom",
		SendDlgItemMessage(hDlg, IDC_SPGBARNETRCVBOTTOM, UDM_GETPOS, 0, 0));

	SetMyRegLong("BarMeter", "BarMeterVL_HorizontalToLeft", IsDlgButtonChecked(hDlg, IDC_BARHORIZ_LEFT_VL));
	SetMyRegLong("BarMeter", "BarMeterBL_HorizontalToLeft", IsDlgButtonChecked(hDlg, IDC_BARHORIZ_LEFT_BL));
	SetMyRegLong("BarMeter", "BarMeterCU_HorizontalToLeft", IsDlgButtonChecked(hDlg, IDC_BARHORIZ_LEFT_CU));
	SetMyRegLong("BarMeter", "BarMeterNet_HorizontalToLeft", IsDlgButtonChecked(hDlg, IDC_BARHORIZ_LEFT_NET));

}
