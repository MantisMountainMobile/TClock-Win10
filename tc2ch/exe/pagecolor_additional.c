/*-------------------------------------------
  pagecolor.c
  "Color and Font" page
  KAZUBON 1997-1998
---------------------------------------------*/


#include "tclock.h"


static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);


extern BOOL b_EnglishMenu;
extern int Language_Offset;

extern BOOL b_DebugLog;

static COMBOCOLOR combocolor[4] = {
	{ IDC_COL_SAT,      "Saturday_TextColor",   0x00C8FFC8},
	{ IDC_COL_SUN,     "Sunday_TextColor",  0x00C8C8FF },
	{ IDC_COL_HOLIDAY,     "Holiday_TextColor",  0x00C8C8FF },
	{ IDC_COL_VPN,      "VPN_TextColor",   0x00FFFF00 },
};

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyClock = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
BOOL CALLBACK PageColorAdditionalProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_MEASUREITEM:
			OnMeasureItemColorCombo(lParam);
			return TRUE;
		case WM_DRAWITEM:
			OnDrawItemColorCombo(lParam, 16);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			SendPSChanged(hDlg);

			switch(id)
			{
			case IDC_COL_SAT:
			case IDC_COL_SUN:
			case IDC_COL_HOLIDAY:
			case IDC_COL_VPN:
			case IDC_TEXTPOS:
				if(code == CBN_SELCHANGE)
				{
					SendPSChanged(hDlg);
				}
				break;
			case IDC_CHOOSECOL_SAT:
			case IDC_CHOOSECOL_SUN:
			case IDC_CHOOSECOL_HOLIDAY:
			case IDC_CHOOSECOL_VPN:
				OnChooseColor(hDlg, id, 16);
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
			DeleteObject(combocolor);
			break;
	}
	return FALSE;
}

/*------------------------------------------------
  Initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{

	HDC hdc;
	LOGFONT logfont;
	//HFONT hfont;
	DWORD dwVer;
	int index;

	InitComboColor(hDlg, 4, combocolor, 16, TRUE);

	// if color depth is 256 or less
	hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
	if(GetDeviceCaps(hdc, BITSPIXEL) <= 8)
	{
		EnableDlgItem(hDlg, IDC_CHOOSECOL_SAT, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSECOL_SUN, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSECOL_HOLIDAY, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSECOL_VPN, FALSE);
	}
	DeleteDC(hdc);
	
	CheckDlgButton(hDlg, IDC_CHKCOL_DATE, GetMyRegLong("Color_Font", "UseDateColor", TRUE));
	CheckDlgButton(hDlg, IDC_CHKCOL_DOW, GetMyRegLong("Color_Font", "UseDowColor", TRUE));
	CheckDlgButton(hDlg, IDC_CHKCOL_TIME, GetMyRegLong("Color_Font", "UseTimeColor", TRUE));
	CheckDlgButton(hDlg, IDC_CHKCOL_ALL, GetMyRegLong("Color_Font", "UseAllColor", TRUE));
	CheckDlgButton(hDlg, IDC_CHKCOL_VPN, GetMyRegLong("Color_Font", "UseVPNColor", TRUE));
}

/*------------------------------------------------
  Apply
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	DWORD dw;

	SetMyRegLong("Color_Font", "UseDateColor", IsDlgButtonChecked(hDlg, IDC_CHKCOL_DATE));
	SetMyRegLong("Color_Font", "UseDowColor", IsDlgButtonChecked(hDlg, IDC_CHKCOL_DOW));
	SetMyRegLong("Color_Font", "UseTimeColor", IsDlgButtonChecked(hDlg, IDC_CHKCOL_TIME));
	SetMyRegLong("Color_Font", "UseAllColor", IsDlgButtonChecked(hDlg, IDC_CHKCOL_ALL));

	SetMyRegLong("Color_Font", "UseVPNColor", IsDlgButtonChecked(hDlg, IDC_CHKCOL_VPN));

	dw = (DWORD)CBGetItemData(hDlg, IDC_COL_SAT, CBGetCurSel(hDlg, IDC_COL_SAT));
	SetMyRegLong("Color_Font", "Saturday_TextColor", dw);

	dw = (DWORD)CBGetItemData(hDlg, IDC_COL_SUN, CBGetCurSel(hDlg, IDC_COL_SUN));
	SetMyRegLong("Color_Font", "Sunday_TextColor", dw);

	dw = (DWORD)CBGetItemData(hDlg, IDC_COL_HOLIDAY, CBGetCurSel(hDlg, IDC_COL_HOLIDAY));
	SetMyRegLong("Color_Font", "Holiday_TextColor", dw);

	dw = (DWORD)CBGetItemData(hDlg, IDC_COL_VPN, CBGetCurSel(hDlg, IDC_COL_VPN));
	SetMyRegLong("Color_Font", "VPN_TextColor", dw);


//	char s[80];
//
//	//背景色の保存
//	SetMyRegLong("Color_Font", "UseBackColor",
//		IsDlgButtonChecked(hDlg, IDC_CHKCOLOR));
//	dw = (DWORD)CBGetItemData(hDlg, IDC_COLBACK, CBGetCurSel(hDlg, IDC_COLBACK));
//	SetMyRegLong("Color_Font", "BackColor", dw);
//
//	SetMyRegLong("Color_Font", "UseBackColor2",
//		IsDlgButtonChecked(hDlg, IDC_CHKCOLOR2));
//	dw = (DWORD)CBGetItemData(hDlg, IDC_COLBACK2, CBGetCurSel(hDlg, IDC_COLBACK2));
//	SetMyRegLong("Color_Font", "BackColor2", dw);
//	SetMyRegLong("Color_Font", "GradDir", IsDlgButtonChecked(hDlg, IDC_CHKCOLORV));
//
//	//文字色の保存
//	dw = (DWORD)CBGetItemData(hDlg, IDC_COLFORE, CBGetCurSel(hDlg, IDC_COLFORE));
//	SetMyRegLong("Color_Font", "ForeColor", dw);
//
//	//影の保存
//	SetMyRegLong("Color_Font", "ForeColorShadow",
//		IsDlgButtonChecked(hDlg, IDC_CHKCLKSHADOW));
//	SetMyRegLong("Color_Font", "ForeColorBORDER",
//		IsDlgButtonChecked(hDlg, IDC_CHKCLKBORDER));
//	dw = (DWORD)CBGetItemData(hDlg, IDC_COLCLKSHADOW, CBGetCurSel(hDlg, IDC_COLCLKSHADOW));
//	SetMyRegLong("Color_Font", "ShadowColor", dw);
//	SetMyRegLong("Color_Font", "ClockShadowRange",
//		SendDlgItemMessage(hDlg, IDC_SPINCLKSHADOW, UDM_GETPOS, 0, 0));
//
//
//	//フォント名の保存
//	CBGetLBText(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT), s);
//	//SetMyRegStr("Color_Font", "Font", s);
//
//	SetMyRegStr("Color_Font", "Font", s+1);	//先頭マークを除去して記録する場合
//
//	//フォントサイズの保存
//	//5以下のサイズは許可しない(数値でないものも含まれる)
//	dw = GetDlgItemInt(hDlg, IDC_FONTSIZE, NULL, FALSE);
//	if(dw < 5)
//	{
//		dw = 9;
//		SetDlgItemInt(hDlg, IDC_FONTSIZE, dw, FALSE);
//	}
//	SetMyRegLong("Color_Font", "FontSize", dw);
//
//	//テキスト位置の保存
//	SetMyRegLong("Color_Font", "TextPos", CBGetCurSel(hDlg, IDC_TEXTPOS));
//
//	//「Bold」「Italic」の保存
//	SetMyRegLong("Color_Font", "Bold", IsDlgButtonChecked(hDlg, IDC_BOLD));
//	SetMyRegLong("Color_Font", "Italic", IsDlgButtonChecked(hDlg, IDC_ITALIC));
//
//	//SetMyRegLong("Color_Font", "ClockHeight",
//	//	SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_GETPOS, 0, 0));
//	SetMyRegLong("Color_Font", "ClockWidth",
//		SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_GETPOS, 0, 0));
//	SetMyRegLong("Color_Font", "VertPos",
//		SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_GETPOS, 0, 0));
//	SetMyRegLong("Color_Font", "LineHeight",
//		SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_GETPOS, 0, 0));
}

