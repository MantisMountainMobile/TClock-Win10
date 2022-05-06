/*-------------------------------------------
  pageanalog.c
  "Analog Clock" page
  add by 505
---------------------------------------------*/

#include "tclock.h"
#define USE_ANALOG_CLOCK	1	// add by 505

#ifdef USE_ANALOG_CLOCK	// add by 505

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnBrowseAnalogClockBitmapFile(HWND hDlg);

//extern int confNo;

extern BOOL b_EnglishMenu;
extern int Language_Offset;

static COMBOCOLOR combocolor[2] = {
	{ IDC_COLHOUR,     "AClockHourHandColor",  RGB(255, 0, 0) },
	{ IDC_COLMIN,      "AClockMinHandColor",   RGB(0, 0, 255) }
};

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyClock = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
BOOL CALLBACK PageAnalogClockProc(HWND hDlg, UINT message,
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
			switch(id)
			{
			case IDC_COLMIN:
			case IDC_COLHOUR:
				if(code == CBN_SELCHANGE)
				{
					SendPSChanged(hDlg);
				}
				break;
			case IDC_CHOOSECOLMIN:
			case IDC_CHOOSECOLHOUR:
				OnChooseColor(hDlg, id, 16);
				SendPSChanged(hDlg);
				break;
			case IDC_ACLOCK_POS:
				if(code == CBN_SELCHANGE)
				{
					SendPSChanged(hDlg);
				}
				break;
			case IDC_CHECK_ACLOCK:
			case IDC_HOURHAND_BOLD:
			case IDC_MINHAND_BOLD:
				SendPSChanged(hDlg);
				break;
			case IDC_EDIT_ACLOCK_HPOS:
			case IDC_EDIT_ACLOCK_VPOS:
			case IDC_EDIT_ACLOCK_SIZE:
				if(code == EN_CHANGE)
				{
					SendPSChanged(hDlg);
				}
				break;
			case IDC_ACLOCKBMP:
				if(code == EN_CHANGE)
					SendPSChanged(hDlg);
				break;
			case IDC_ACLOCKBMPBROWSE:
				OnBrowseAnalogClockBitmapFile(hDlg);
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
		case WM_DESTROY:
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
	int index;
	char fname[MAX_PATH + 1];

	// setting of "min hand" and "hour hand"
	InitComboColor(hDlg, 2, combocolor, 16, TRUE);

	// if color depth is 256 or less
	hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
	if(GetDeviceCaps(hdc, BITSPIXEL) <= 8)
	{
		EnableDlgItem(hDlg, IDC_CHOOSECOLMIN, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSECOLHOUR, FALSE);
	}
	DeleteDC(hdc);

	CheckDlgButton(hDlg, IDC_HOURHAND_BOLD,
		GetMyRegLong("AnalogClock", "AnalogClockHourHandBold", FALSE));
	CheckDlgButton(hDlg, IDC_MINHAND_BOLD,
		GetMyRegLong("AnalogClock", "AnalogClockMinHandBold", FALSE));

	//「アナログ時計位置」の設定
	index = CBAddString(hDlg, IDC_ACLOCK_POS, (LPARAM)MyString(IDS_ACLOCKLEFT));
	index = CBAddString(hDlg, IDC_ACLOCK_POS, (LPARAM)MyString(IDS_ACLOCKRIGHT));
	index = CBAddString(hDlg, IDC_ACLOCK_POS, (LPARAM)MyString(IDS_ACLOCKMIDDLE));
	CBSetCurSel(hDlg, IDC_ACLOCK_POS,
		GetMyRegLong("AnalogClock", "AnalogClockPos", 0));

	CheckDlgButton(hDlg, IDC_CHECK_ACLOCK,
		GetMyRegLong("AnalogClock", "UseAnalogClock", FALSE));

	SendDlgItemMessage(hDlg, IDC_SPIN_ACLOCK_HPOS, UDM_SETRANGE, 0,
		MAKELONG(1000, -1000));
	SendDlgItemMessage(hDlg, IDC_SPIN_ACLOCK_HPOS, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("AnalogClock", "AnalogClockHPos", 0));

	SendDlgItemMessage(hDlg, IDC_SPIN_ACLOCK_VPOS, UDM_SETRANGE, 0,
		MAKELONG(1000, -1000));
	SendDlgItemMessage(hDlg, IDC_SPIN_ACLOCK_VPOS, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("AnalogClock", "AnalogClockVPos", 0));

	SendDlgItemMessage(hDlg, IDC_SPIN_ACLOCK_SIZE, UDM_SETRANGE, 0,
		MAKELONG(256, 0));
	SendDlgItemMessage(hDlg, IDC_SPIN_ACLOCK_SIZE, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("AnalogClock", "AnalogClockSize", 0));

	index = GetMyRegStr("AnalogClock", "AnalogClockBmp", fname, MAX_PATH, "..\\tclock.bmp");
	SetDlgItemText(hDlg, IDC_ACLOCKBMP, fname);
}

/*------------------------------------------------
  Apply
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	DWORD dw;
	char fname[MAX_PATH + 1];

	//短針の色
	dw = (DWORD)CBGetItemData(hDlg, IDC_COLHOUR, CBGetCurSel(hDlg, IDC_COLHOUR));
	SetMyRegLong("AnalogClock", "AClockHourHandColor", dw);

	//長針の色
	dw = (DWORD)CBGetItemData(hDlg, IDC_COLMIN, CBGetCurSel(hDlg, IDC_COLMIN));
	SetMyRegLong("AnalogClock", "AClockMinHandColor", dw);

	//短針、長針の太さ
	SetMyRegLong("AnalogClock", "AnalogClockHourHandBold", IsDlgButtonChecked(hDlg, IDC_HOURHAND_BOLD));
	SetMyRegLong("AnalogClock", "AnalogClockMinHandBold", IsDlgButtonChecked(hDlg, IDC_MINHAND_BOLD));

	//アナログ時計位置の保存
	SetMyRegLong("AnalogClock", "AnalogClockPos", CBGetCurSel(hDlg, IDC_ACLOCK_POS));

	SetMyRegLong("AnalogClock", "UseAnalogClock",
		IsDlgButtonChecked(hDlg, IDC_CHECK_ACLOCK));

	SetMyRegLong("AnalogClock", "AnalogClockHPos",
		SendDlgItemMessage(hDlg, IDC_SPIN_ACLOCK_HPOS, UDM_GETPOS, 0, 0));

	SetMyRegLong("AnalogClock", "AnalogClockVPos",
		SendDlgItemMessage(hDlg, IDC_SPIN_ACLOCK_VPOS, UDM_GETPOS, 0, 0));

	GetDlgItemText(hDlg, IDC_ACLOCKBMP, fname, MAX_PATH);
	SetMyRegStr("AnalogClock", "AnalogClockBmp", fname);

	SetMyRegLong("AnalogClock", "AnalogClockSize",
		SendDlgItemMessage(hDlg, IDC_SPIN_ACLOCK_SIZE, UDM_GETPOS, 0, 0));

}

static void OnBrowseAnalogClockBitmapFile(HWND hDlg)
{
	char filter[80];
	char deffile[MAX_PATH];
	char fname[MAX_PATH];

	filter[0] = filter[1] = 0;
	str0cat(filter, MyString(IDS_BMPFILE)); str0cat(filter, "*.bmp");
	str0cat(filter, MyString(IDS_ALLFILE)); str0cat(filter, "*.*");

	GetDlgItemText(hDlg, IDC_ACLOCKBMP, deffile, MAX_PATH);

	if (!SelectMyFile(hDlg, filter, 0, deffile, fname)) {// propsheet.c
		return;
	}

	SetDlgItemText(hDlg, IDC_ACLOCKBMP, fname);

	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);

	SendPSChanged(hDlg);
}

#endif //USE_ANALOG_CLOCK	// add by 505
