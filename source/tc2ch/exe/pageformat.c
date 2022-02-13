/*-------------------------------------------
  pageformat.c
  "Format" page of properties
                       KAZUBON 1997-1998
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnLocale(HWND hDlg);
static void OnCustom(HWND hDlg, BOOL bmouse);
static void On12Hour(HWND hDlg);
static void OnFormatCheck(HWND hDlg, WORD id);

static HWND hwndPage;
static int ilang;  // language code. ex) 0x411 - Japanese
static int idate;  // 0: mm/dd/yy 1: dd/mm/yy 2: yy/mm/dd
static BOOL bDayOfWeekIsLast;   // yy/mm/dd ddd
static BOOL bTimeMarkerIsFirst; // AM/PM hh:nn:ss
static char sMon[11];  //
static char *pCustomFormat = NULL;


extern BOOL b_EnglishMenu;
extern int Language_Offset;

INT_PTR CALLBACK DlgProcFormat2(HWND, UINT, WPARAM, LPARAM);

int exe_offsetClockMS = 0;

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyClock = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
   Dialog Procedure for the "Format" page
--------------------------------------------------*/
BOOL CALLBACK PageFormatProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
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
			switch(id)
			{
			case IDC_FORMAT_OFFSET_MS:
			case IDC_SPG_FORMAT_OFFSET_MS:
				SendPSChanged(hDlg);
				break;
			case IDC_LOCALE:
				if(code == CBN_SELCHANGE)
					OnLocale(hDlg);
				break;
			// format textbox
			case IDC_FORMAT:
				if(code == EN_CHANGE)
					SendPSChanged(hDlg);
				break;
			// "Custumize Format"
			case IDC_CUSTOM:
				OnCustom(hDlg, TRUE);
				break;
			// "12H"
			case IDC_12HOUR:
				On12Hour(hDlg);
				break;
			// "detail..."
			case IDC_FORMATDETAIL:
				{
				INT_PTR r;
				r = DialogBoxParam(GetLangModule(),
					MAKEINTRESOURCE(Language_Offset + IDD_FORMAT2),
					g_hwndPropDlg, DlgProcFormat2, (LPARAM)ilang);
				if(r == IDOK) SendPSChanged(hDlg);
				break;
				}
			// "year" -- "Internet Time"
			default:
				if(IDC_YEAR4 <= id && id <= IDC_AMPM)
					OnFormatCheck(hDlg, id);
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
			if(pCustomFormat) free(pCustomFormat); pCustomFormat = NULL;
			break;
	}
	return FALSE;
}

char *entrydate[] = { "Year4", "Year", "Month", "MonthS", "Day", "Weekday",
	"Hour", "Minute", "Second", "Kaigyo", "InternetTime",
	"AMPM", "Hour12", "Custom",  };
#define ENTRY(id) entrydate[(id)-IDC_YEAR4]

/*------------------------------------------------
  Initialize Locale Infomation
--------------------------------------------------*/
void InitLocale(HWND hwnd)
{
	char s[21];
	int i, sel;
	int aLangDayOfWeekIsLast[] =
		{ LANG_JAPANESE, 0 };
	int aTimeMarkerIsFirst[] =
		{ LANG_CHINESE, LANG_JAPANESE, 0 };

	if(hwnd)
	{
		sel = CBGetCurSel(hwnd, IDC_LOCALE);
		ilang = CBGetItemData(hwnd, IDC_LOCALE, sel);
	}
	else
	{
		ilang = GetMyRegLong("Format", "Locale", (int)GetUserDefaultLangID());
	}
	GetLocaleInfoWA(ilang, LOCALE_IDATE, s, 20);
	idate = atoi(s);
	GetLocaleInfoWA(ilang, LOCALE_SABBREVDAYNAME1, sMon, 10);

	bDayOfWeekIsLast = FALSE;
	for(i = 0; aLangDayOfWeekIsLast[i]; i++)
	{
		if((ilang & 0x00ff) == aLangDayOfWeekIsLast[i])
		{
			bDayOfWeekIsLast = TRUE; break;
		}
	}
	bTimeMarkerIsFirst = FALSE;
	for(i = 0; aTimeMarkerIsFirst[i]; i++)
	{
		if((ilang & 0x00ff) == aTimeMarkerIsFirst[i])
		{
			bTimeMarkerIsFirst = TRUE; break;
		}
	}
}

/*------------------------------------------------
  for EnumSystemLocales function
--------------------------------------------------*/
BOOL CALLBACK EnumLocalesProc(LPTSTR lpLocaleString)
{
	char s[81], s1[81], s2[81];
	int x, index;

	x = atox(lpLocaleString);
	if (b_EnglishMenu)
	{
		if (GetLocaleInfoWA(x, LOCALE_SENGLANGUAGE, s1, 40) > 0)
		{
			GetLocaleInfoWA(x, LOCALE_SENGCOUNTRY, s2, 40);
			wsprintf(s, "%s (%s)", s1, s2);
			index = CBAddString(hwndPage, IDC_LOCALE, (LPARAM)s);
		}
		else
			index = CBAddString(hwndPage, IDC_LOCALE, (LPARAM)lpLocaleString);
	}
	else
	{
		if (GetLocaleInfoWA(x, LOCALE_SLANGUAGE, s, 80) > 0)
			index = CBAddString(hwndPage, IDC_LOCALE, (LPARAM)s);
		else
			index = CBAddString(hwndPage, IDC_LOCALE, (LPARAM)lpLocaleString);
	}


	CBSetItemData(hwndPage, IDC_LOCALE, index, x);
	return TRUE;
}

/*------------------------------------------------
  Initialize the "Format" page
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	HFONT hfont;
	char s[1024];
	int i, count, nKaigyo;
	DWORD dw;

	hwndPage = hDlg;


	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont)
	{
		SendDlgItemMessage(hDlg, IDC_LOCALE, WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_AMSYMBOL, WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_PMSYMBOL, WM_SETFONT, (WPARAM)hfont, 0);
	}
	hfont = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
	if(hfont)
		SendDlgItemMessage(hDlg, IDC_FORMAT, WM_SETFONT, (WPARAM)hfont, 0);

	// Fill and select the "Locale" combobox
	EnumSystemLocales(EnumLocalesProc, LCID_INSTALLED);
	CBSetCurSel(hDlg, IDC_LOCALE, 0);
	ilang = GetMyRegLong("Format", "Locale", (int)GetUserDefaultLangID());
	count = CBGetCount(hDlg, IDC_LOCALE);
	for(i = 0; i < count; i++)
	{
		int x;
		x = CBGetItemData(hDlg, IDC_LOCALE, i);
		if(x == ilang)
		{
			CBSetCurSel(hDlg, IDC_LOCALE, i); break;
		}
	}
	//リスト項目の表示数を指定
	AdjustDlgConboBoxDropDown(hDlg, IDC_LOCALE, 13);

	InitLocale(hDlg);

	// "year" -- "second"
	for(i = IDC_YEAR4; i <= IDC_SECOND; i++)
	{
		CheckDlgButton(hDlg, i,
			GetMyRegLong("Format",  ENTRY(i), TRUE));
	}

	if(IsDlgButtonChecked(hDlg, IDC_YEAR))
		CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR);
	if(IsDlgButtonChecked(hDlg, IDC_YEAR4))
		CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR4);

	if(IsDlgButtonChecked(hDlg, IDC_MONTH))
		CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTH);
	if(IsDlgButtonChecked(hDlg, IDC_MONTHS))
		CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTHS);

	nKaigyo = GetMyRegLong("Format",  ENTRY(IDC_KAIGYO), -1);
	if(nKaigyo < 0)
	{
		RECT rc;
		HWND hwnd;
		nKaigyo = 1;
		hwnd = FindWindow("Shell_TrayWnd", NULL);
		if(hwnd != NULL)
		{
			GetClientRect(hwnd, &rc);
			// if the task bar is positioned horizontally
			if(rc.right > rc.bottom) nKaigyo = 0;
		}
	}
	CheckDlgButton(hDlg, IDC_KAIGYO, nKaigyo);

	// "Internet Time" -- "Customize format"
	for(i = IDC_INTERNETTIME; i <= IDC_CUSTOM; i++)
	{
		CheckDlgButton(hDlg, i,
			GetMyRegLong("Format",  ENTRY(i), FALSE));
	}


	GetMyRegStr("Format", "Format", s, 1024, "");
	SetDlgItemText(hDlg, IDC_FORMAT, s);

	pCustomFormat = malloc(1024);
	if(pCustomFormat)
		GetMyRegStr("Format", "CustomFormat", pCustomFormat, 1024, "");

	On12Hour(hDlg);
	OnCustom(hDlg, FALSE);

	exe_offsetClockMS = (int)(short)GetMyRegLong(NULL, "OffsetClockMS", 0);
	SendDlgItemMessage(hDlg, IDC_SPG_FORMAT_OFFSET_MS, UDM_SETRANGE, 0, MAKELONG(10000, -10000));
	SendDlgItemMessage(hDlg, IDC_SPG_FORMAT_OFFSET_MS, UDM_SETPOS, 0, exe_offsetClockMS);


}

/*------------------------------------------------
  "Apply" button
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[1024];
	int i;
	DWORD dw;

	SetMyRegLong("Format", "Locale",
		CBGetItemData(hDlg, IDC_LOCALE, CBGetCurSel(hDlg, IDC_LOCALE)));

	for(i = IDC_YEAR4; i <= IDC_CUSTOM; i++)
	{
		SetMyRegLong("Format", ENTRY(i), IsDlgButtonChecked(hDlg, i));
	}

	GetDlgItemText(hDlg, IDC_FORMAT, s, 1024);
	SetMyRegStr("Format", "Format", s);

	if(pCustomFormat)
	{
		if(IsDlgButtonChecked(hDlg, IDC_CUSTOM))
		{
			strcpy(pCustomFormat, s);
			SetMyRegStr("Format", "CustomFormat", pCustomFormat);
		}
	}

	SetMyRegLong(NULL, "OffsetClockMS", (int)(short)SendDlgItemMessage(hDlg, IDC_SPG_FORMAT_OFFSET_MS, UDM_GETPOS, 0, 0));

}

/*------------------------------------------------
  When changed "Locale" combobox
--------------------------------------------------*/
void OnLocale(HWND hDlg)
{
	InitLocale(hDlg);
	OnCustom(hDlg, FALSE);
}

/*------------------------------------------------
  "Customize format" checkbox
--------------------------------------------------*/
void OnCustom(HWND hDlg, BOOL bmouse)
{
	BOOL b;
	int i;

	b = IsDlgButtonChecked(hDlg, IDC_CUSTOM);
	EnableDlgItem(hDlg, IDC_FORMAT, b);

	for(i = IDC_YEAR4; i <= IDC_AMPM; i++)
		EnableDlgItem(hDlg, i, !b);

	if(pCustomFormat && bmouse)
	{
		if(b)
		{
			if(pCustomFormat[0])
				SetDlgItemText(hDlg, IDC_FORMAT, pCustomFormat);
		}
		else GetDlgItemText(hDlg, IDC_FORMAT, pCustomFormat, 1024);
	}

	if(!b) OnFormatCheck(hDlg, 0);

	SendPSChanged(hDlg);
}

/*------------------------------------------------
   "12H" combobox
--------------------------------------------------*/
void On12Hour(HWND hDlg)
{
	BOOL b;
	b = IsDlgButtonChecked(hDlg, IDC_12HOUR);
	if(!b)
	{
		CheckDlgButton(hDlg, IDC_AMPM, 0);
		if(!IsDlgButtonChecked(hDlg, IDC_CUSTOM))
			OnFormatCheck(hDlg, 0);
	}

	SendPSChanged(hDlg);
}

#define CHECKS(a) checks[(a)-IDC_YEAR4]

/*------------------------------------------------
  When clicked "year" -- "am/pm"
--------------------------------------------------*/
void OnFormatCheck(HWND hDlg, WORD id)
{
	char s[1024];
	int checks[15];
	int i;

	for(i = IDC_YEAR4; i <= IDC_AMPM; i++)
	{
		CHECKS(i) = IsDlgButtonChecked(hDlg, i);
	}

	if(id == IDC_YEAR4 || id == IDC_YEAR)
	{
		if(id == IDC_YEAR4 && CHECKS(IDC_YEAR4))
		{
			CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR4);
			CHECKS(IDC_YEAR) = FALSE;
		}
		if(id == IDC_YEAR && CHECKS(IDC_YEAR))
		{
			CheckRadioButton(hDlg, IDC_YEAR4, IDC_YEAR, IDC_YEAR);
			CHECKS(IDC_YEAR4) = FALSE;
		}
	}

	if(id == IDC_MONTH || id == IDC_MONTHS)
	{
		if(id == IDC_MONTH && CHECKS(IDC_MONTH))
		{
			CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTH);
			CHECKS(IDC_MONTHS) = FALSE;
		}
		if(id == IDC_MONTHS && CHECKS(IDC_MONTHS))
		{
			CheckRadioButton(hDlg, IDC_MONTH, IDC_MONTHS, IDC_MONTHS);
			CHECKS(IDC_MONTH) = FALSE;
		}
	}

	if(id == IDC_AMPM)
	{
		CheckDlgButton(hDlg, IDC_12HOUR, 1);
		On12Hour(hDlg);
	}

	CreateFormat(s, checks);
	SetDlgItemText(hDlg, IDC_FORMAT, s);
	SendPSChanged(hDlg);
}

/*------------------------------------------------
  Initialize a format string. Called from main.c
--------------------------------------------------*/
void InitFormat(void)
{
	char s[1024];
	int i, checks[15];
	RECT rc;
	HWND hwnd;
	BOOL b;

	if(GetMyRegLong("Format",  ENTRY(IDC_CUSTOM), FALSE))
		return;

	InitLocale(NULL);

	for(i = IDC_YEAR4; i <= IDC_SECOND; i++)
	{
		CHECKS(i) = GetMyRegLong("Format",  ENTRY(i), TRUE);
	}

	if(CHECKS(IDC_YEAR))  CHECKS(IDC_YEAR4) = FALSE;
	if(CHECKS(IDC_YEAR4)) CHECKS(IDC_YEAR) = FALSE;

	if(CHECKS(IDC_MONTH))  CHECKS(IDC_MONTHS) = FALSE;
	if(CHECKS(IDC_MONTHS)) CHECKS(IDC_MONTH) = FALSE;

	CHECKS(IDC_INTERNETTIME) = GetMyRegLong("Format",
		ENTRY(IDC_INTERNETTIME), FALSE);

	b = FALSE;
	hwnd = FindWindow("Shell_TrayWnd", NULL);
	if(hwnd != NULL)
	{
		GetClientRect(hwnd, &rc);
		if(rc.right < rc.bottom) b = TRUE;
	}
	CHECKS(IDC_KAIGYO) =
		GetMyRegLong("Format",  ENTRY(IDC_KAIGYO), b);
	CHECKS(IDC_AMPM) = GetMyRegLong("Format",  ENTRY(IDC_AMPM), FALSE);

	CreateFormat(s, checks);
	SetMyRegStr("Format", "Format", s);
}

/*------------------------------------------------
  Create a format string automatically
--------------------------------------------------*/
void CreateFormat(char* dst, int* checks)
{
	BOOL bdate = FALSE, btime = FALSE;
	int i;

	for(i = IDC_YEAR4; i <= IDC_WEEKDAY; i++)
	{
		if(CHECKS(i)) { bdate = TRUE; break; }
	}
	for(i = IDC_HOUR; i <= IDC_AMPM; i++)
	{
		if(CHECKS(i)) { btime = TRUE; break; }
	}

	dst[0] = 0;

	if(!bDayOfWeekIsLast && CHECKS(IDC_WEEKDAY))
	{
		strcat(dst, "ddd");
		for(i = IDC_YEAR4; i <= IDC_DAY; i++)
		{
			if(CHECKS(i))
			{
				if((ilang & 0x00ff) == LANG_CHINESE) strcat(dst, " ");
				else if(sMon[0] && sMon[ strlen(sMon) - 1 ] == '.')
					strcat(dst, " ");
				else strcat(dst, ", ");
				break;
			}
		}
	}

	if(idate == 0)
	{
		if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS))
		{
			if(CHECKS(IDC_MONTH)) strcat(dst, "mm");
			if(CHECKS(IDC_MONTHS)) strcat(dst, "mmm");
			if(CHECKS(IDC_DAY) || CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR))
			{
				if(CHECKS(IDC_MONTH)) strcat(dst, "/");
				else strcat(dst, " ");
			}
		}
		if(CHECKS(IDC_DAY))
		{
			strcat(dst, "dd");
			if(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR))
			{
				if(CHECKS(IDC_MONTH)) strcat(dst, "/");
				else strcat(dst, ", ");
			}
		}
		if(CHECKS(IDC_YEAR4)) strcat(dst, "yyyy");
		if(CHECKS(IDC_YEAR)) strcat(dst, "yy");
	}
	else if(idate == 1)
	{
		if(CHECKS(IDC_DAY))
		{
			strcat(dst, "dd");
			if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS))
			{
				if(CHECKS(IDC_MONTH)) strcat(dst, "/");
				else strcat(dst, " ");
			}
			else if(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR)) strcat(dst, "/");
		}
		if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS))
		{
			if(CHECKS(IDC_MONTH)) strcat(dst, "mm");
			if(CHECKS(IDC_MONTHS)) strcat(dst, "mmm");
			if(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR))
			{
				if(CHECKS(IDC_MONTH)) strcat(dst, "/");
				else strcat(dst, " ");
			}
		}
		if(CHECKS(IDC_YEAR4)) strcat(dst, "yyyy");
		if(CHECKS(IDC_YEAR)) strcat(dst, "yy");
	}
	else
	{
		if(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR))
		{
			if(CHECKS(IDC_YEAR4)) strcat(dst, "yyyy");
			if(CHECKS(IDC_YEAR)) strcat(dst, "yy");
			if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS)
				|| CHECKS(IDC_DAY))
			{
				if(CHECKS(IDC_MONTHS)) strcat(dst, " ");
				else strcat(dst, "/");
			}
		}
		if(CHECKS(IDC_MONTH) || CHECKS(IDC_MONTHS))
		{
			if(CHECKS(IDC_MONTH)) strcat(dst, "mm");
			if(CHECKS(IDC_MONTHS)) strcat(dst, "mmm");
			if(CHECKS(IDC_DAY))
			{
				if(CHECKS(IDC_MONTHS)) strcat(dst, " ");
				else strcat(dst, "/");
			}
		}
		if(CHECKS(IDC_DAY)) strcat(dst, "dd");
	}

	if(bDayOfWeekIsLast && CHECKS(IDC_WEEKDAY))
	{
		for(i = IDC_YEAR4; i <= IDC_DAY; i++)
		{
			if(CHECKS(i)) { strcat(dst, " "); break; }
		}
		strcat(dst, "ddd");
	}

	if(bdate && btime)
	{
		if(CHECKS(IDC_KAIGYO)) strcat(dst, "\\n");
		else
		{
			if(idate < 2 && CHECKS(IDC_MONTHS) &&
				(CHECKS(IDC_YEAR4) || CHECKS(IDC_YEAR)))
				strcat(dst, " ");
			strcat(dst, " ");
		}
	}

	if(bTimeMarkerIsFirst && CHECKS(IDC_AMPM))
	{
		strcat(dst, "tt");
		if(CHECKS(IDC_HOUR) || CHECKS(IDC_MINUTE) ||
			CHECKS(IDC_SECOND) || CHECKS(IDC_INTERNETTIME))
			strcat(dst, " ");
	}

	if(CHECKS(IDC_HOUR))
	{
		strcat(dst, "hh");
		if(CHECKS(IDC_MINUTE) || CHECKS(IDC_SECOND)) strcat(dst, ":");
		else if(CHECKS(IDC_INTERNETTIME) ||
			(!bTimeMarkerIsFirst && CHECKS(IDC_AMPM))) strcat(dst, " ");
	}
	if(CHECKS(IDC_MINUTE))
	{
		strcat(dst, "nn");
		if(CHECKS(IDC_SECOND)) strcat(dst, ":");
		else if(CHECKS(IDC_INTERNETTIME) ||
			(!bTimeMarkerIsFirst && CHECKS(IDC_AMPM))) strcat(dst, " ");
	}
	if(CHECKS(IDC_SECOND))
	{
		strcat(dst, "ss");
		if(CHECKS(IDC_INTERNETTIME) ||
			(!bTimeMarkerIsFirst && CHECKS(IDC_AMPM))) strcat(dst, " ");
	}

	if(!bTimeMarkerIsFirst && CHECKS(IDC_AMPM))
	{
		strcat(dst, "tt");
		if(CHECKS(IDC_INTERNETTIME)) strcat(dst, " ");
	}

	if(CHECKS(IDC_INTERNETTIME)) strcat(dst, "@@@");
}
