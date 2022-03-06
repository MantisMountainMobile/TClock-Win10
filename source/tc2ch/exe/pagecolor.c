/*-------------------------------------------
  pagecolor.c
  "Color and Font" page
  KAZUBON 1997-1998
---------------------------------------------*/


#include "tclock.h"


static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnCheckColor2(HWND hDlg);
static void OnCheckShadow(HWND hDlg, int id);
static void InitComboFont(HWND hDlg);
static void SetComboFontSize(HWND hDlg, BOOL bInit);
static HFONT hfontb;  // for IDC_BOLD
static HFONT hfonti;  // for IDC_ITALIC
static BOOL UseGrad = FALSE;
int confNo;

LOGFONT  logfont_sample;
int logfont_sample_height;

extern BOOL b_EnglishMenu;
extern int Language_Offset;

void RefreshFontSample(HWND hDlg, BOOL bInit);

extern BOOL b_DebugLog;

static HFONT hfont_sample;

static COMBOCOLOR combocolor[4] = {
	{ IDC_COLBACK,      "BackColor",   0x80000000|COLOR_3DFACE },
	{ IDC_COLBACK2,     "BackColor2",  0xFFFFFFFF },
	{ IDC_COLFORE,      "ForeColor",   0x80000000|COLOR_BTNTEXT },
	{ IDC_COLCLKSHADOW, "ShadowColor", RGB(0, 0, 0) }
};

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyClock = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
BOOL CALLBACK PageColorProc(HWND hDlg, UINT message,
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
			case IDC_FONT:
				if(code == CBN_SELCHANGE)
				{
					SetComboFontSize(hDlg, FALSE);
					SendPSChanged(hDlg);
				}
				break;
			case IDC_COLBACK:
			case IDC_COLBACK2:
			case IDC_COLFORE:
			case IDC_COLCLKSHADOW:
			case IDC_TEXTPOS:
				if(code == CBN_SELCHANGE)
				{
					SendPSChanged(hDlg);
				}
				break;
			case IDC_FONTSIZE:
				if (code == CBN_SELCHANGE || code == CBN_EDITCHANGE)
				{
					RefreshFontSample(hDlg, FALSE);
					//SetComboFontSize(hDlg, FALSE);	//これはうまくいかない。
					SendPSChanged(hDlg);
				}
				break;
			case IDC_CHKCOLOR:
			case IDC_CHKCOLOR2:
				OnCheckColor2(hDlg);
				break;
			case IDC_CHKCLKSHADOW:
			case IDC_CHKCLKBORDER:
				OnCheckShadow(hDlg, id);
				break;
			case IDC_CHOOSECOLBACK:
			case IDC_CHOOSECOLBACK2:
			case IDC_CHOOSECOLFORE:
			case IDC_CHOOSECOLCLKSHADOW:
				OnChooseColor(hDlg, id, 16);
				SendPSChanged(hDlg);
				break;
			case IDC_BOLD:
			case IDC_ITALIC:
				RefreshFontSample(hDlg, FALSE);
				SendPSChanged(hDlg);
				break;
			case IDC_CHKCOLORV:
				SendPSChanged(hDlg);
				break;
			//case IDC_CLOCKHEIGHT:
			case IDC_CLOCKWIDTH:
			case IDC_VERTPOS:
			case IDC_LINEHEIGHT:
			case IDC_CLKSHADOWRANGE:
				if(code == EN_CHANGE)
				{
					SendPSChanged(hDlg);
				}
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
			DeleteObject(hfontb);
			DeleteObject(hfonti);
			DeleteObject(hfont_sample);
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


	confNo = 1;

	// setting of "background" and "text"
	InitComboColor(hDlg, 4, combocolor, 16, TRUE);

	// if color depth is 256 or less
	hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
	if(GetDeviceCaps(hdc, BITSPIXEL) <= 8)
	{
		EnableDlgItem(hDlg, IDC_CHOOSECOLBACK, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSECOLBACK2, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSECOLFORE, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSECOLCLKSHADOW, FALSE);
	}
	DeleteDC(hdc);
	
	hfontb = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if (hfontb)
	{
		SendDlgItemMessage(hDlg, IDC_FONT, WM_SETFONT, (WPARAM)hfontb, 0);
		GetObject(hfontb, sizeof(LOGFONT), &logfont_sample);	//added by TTTT for sample
		logfont_sample_height = logfont_sample.lfHeight;
	}







	//「フォント」の設定
	InitComboFont(hDlg);




	//「フォントサイズ」の設定
	SetComboFontSize(hDlg, TRUE);

	//RefreshFontSample(hDlg);



	//「テキストの位置」の設定
	index = CBAddString(hDlg, IDC_TEXTPOS, (LPARAM)MyString(IDS_TEXTCENTER));
//	CBSetItemData(hDlg, IDC_TEXTPOS, index, 0);
	index = CBAddString(hDlg, IDC_TEXTPOS, (LPARAM)MyString(IDS_TEXTLEFT));
//	CBSetItemData(hDlg, IDC_TEXTPOS, index, 1);
	index = CBAddString(hDlg, IDC_TEXTPOS, (LPARAM)MyString(IDS_TEXTRIGHT));
//	CBSetItemData(hDlg, IDC_TEXTPOS, index, 2);


	CBSetCurSel(hDlg, IDC_TEXTPOS,
		GetMyRegLong("Color_Font", "TextPos", 0));
	{
		CheckDlgButton(hDlg, IDC_CHKCOLOR,
			GetMyRegLong("Color_Font", "UseBackColor", TRUE));
	}
	CheckDlgButton(hDlg, IDC_CHKCOLOR2,
		GetMyRegLong("Color_Font", "UseBackColor2", FALSE));
	CheckDlgButton(hDlg, IDC_CHKCOLORV,
		GetMyRegLong("Color_Font", "GradDir", FALSE));


	{
		UseGrad = TRUE;
	}




	OnCheckColor2(hDlg);

	CheckDlgButton(hDlg, IDC_CHKCLKSHADOW,
		GetMyRegLong("Color_Font", "ForeColorShadow", FALSE));
	CheckDlgButton(hDlg, IDC_CHKCLKBORDER,
		GetMyRegLong("Color_Font", "ForeColorBorder", FALSE));
	SendDlgItemMessage(hDlg, IDC_SPINCLKSHADOW, UDM_SETRANGE, 0,
		MAKELONG(10, 0));
	SendDlgItemMessage(hDlg, IDC_SPINCLKSHADOW, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Color_Font", "ClockShadowRange", 1));
	OnCheckShadow(hDlg, 0);

	//「Bold」「Italic」の設定
	CheckDlgButton(hDlg, IDC_BOLD,
		GetMyRegLong("Color_Font", "Bold", FALSE));
	CheckDlgButton(hDlg, IDC_ITALIC,
		GetMyRegLong("Color_Font", "Italic", FALSE));

	hfontb = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	GetObject(hfontb, sizeof(LOGFONT), &logfont);
	logfont.lfWeight = FW_BOLD;
	hfontb = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hDlg, IDC_BOLD, WM_SETFONT, (WPARAM)hfontb, 0);

	logfont.lfWeight = FW_NORMAL;
	logfont.lfItalic = 1;
	hfonti = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hDlg, IDC_ITALIC, WM_SETFONT, (WPARAM)hfonti, 0);

	//SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_SETRANGE, 0,
	//	MAKELONG(32, -32));
	//SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_SETPOS, 0,
	//	(int)(short)GetMyRegLong("Color_Font", "ClockHeight", 0));
	SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_SETRANGE, 0,
		MAKELONG(32, -32));
	SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Color_Font", "ClockWidth", 0));
	SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_SETRANGE, 0,
		MAKELONG(32, -32));
	SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Color_Font", "VertPos", 0));
	SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_SETRANGE, 0,
		MAKELONG(32, -32));
	SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Color_Font", "LineHeight", 0));
}

/*------------------------------------------------
  Apply
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	DWORD dw;
	char s[80];

	//背景色の保存
	SetMyRegLong("Color_Font", "UseBackColor",
		IsDlgButtonChecked(hDlg, IDC_CHKCOLOR));
	dw = (DWORD)CBGetItemData(hDlg, IDC_COLBACK, CBGetCurSel(hDlg, IDC_COLBACK));
	SetMyRegLong("Color_Font", "BackColor", dw);

	SetMyRegLong("Color_Font", "UseBackColor2",
		IsDlgButtonChecked(hDlg, IDC_CHKCOLOR2));
	dw = (DWORD)CBGetItemData(hDlg, IDC_COLBACK2, CBGetCurSel(hDlg, IDC_COLBACK2));
	SetMyRegLong("Color_Font", "BackColor2", dw);
	SetMyRegLong("Color_Font", "GradDir", IsDlgButtonChecked(hDlg, IDC_CHKCOLORV));

	//文字色の保存
	dw = (DWORD)CBGetItemData(hDlg, IDC_COLFORE, CBGetCurSel(hDlg, IDC_COLFORE));
	SetMyRegLong("Color_Font", "ForeColor", dw);

	//影の保存
	SetMyRegLong("Color_Font", "ForeColorShadow",
		IsDlgButtonChecked(hDlg, IDC_CHKCLKSHADOW));
	SetMyRegLong("Color_Font", "ForeColorBORDER",
		IsDlgButtonChecked(hDlg, IDC_CHKCLKBORDER));
	dw = (DWORD)CBGetItemData(hDlg, IDC_COLCLKSHADOW, CBGetCurSel(hDlg, IDC_COLCLKSHADOW));
	SetMyRegLong("Color_Font", "ShadowColor", dw);
	SetMyRegLong("Color_Font", "ClockShadowRange",
		SendDlgItemMessage(hDlg, IDC_SPINCLKSHADOW, UDM_GETPOS, 0, 0));


	//フォント名の保存
	CBGetLBText(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT), s);
	//SetMyRegStr("Color_Font", "Font", s);

	SetMyRegStr("Color_Font", "Font", s+1);	//先頭マークを除去して記録する場合

	//フォントサイズの保存
	//5以下のサイズは許可しない(数値でないものも含まれる)
	dw = GetDlgItemInt(hDlg, IDC_FONTSIZE, NULL, FALSE);
	if(dw < 5)
	{
		dw = 9;
		SetDlgItemInt(hDlg, IDC_FONTSIZE, dw, FALSE);
	}
	SetMyRegLong("Color_Font", "FontSize", dw);

	//テキスト位置の保存
	SetMyRegLong("Color_Font", "TextPos", CBGetCurSel(hDlg, IDC_TEXTPOS));

	//「Bold」「Italic」の保存
	SetMyRegLong("Color_Font", "Bold", IsDlgButtonChecked(hDlg, IDC_BOLD));
	SetMyRegLong("Color_Font", "Italic", IsDlgButtonChecked(hDlg, IDC_ITALIC));

	//SetMyRegLong("Color_Font", "ClockHeight",
	//	SendDlgItemMessage(hDlg, IDC_SPINCHEIGHT, UDM_GETPOS, 0, 0));
	SetMyRegLong("Color_Font", "ClockWidth",
		SendDlgItemMessage(hDlg, IDC_SPINCWIDTH, UDM_GETPOS, 0, 0));
	SetMyRegLong("Color_Font", "VertPos",
		SendDlgItemMessage(hDlg, IDC_SPINVPOS, UDM_GETPOS, 0, 0));
	SetMyRegLong("Color_Font", "LineHeight",
		SendDlgItemMessage(hDlg, IDC_SPINLHEIGHT, UDM_GETPOS, 0, 0));
}

/*------------------------------------------------
  enable/disable to use "background 2"
--------------------------------------------------*/
void OnCheckColor2(HWND hDlg)
{
	BOOL b;
	b = IsDlgButtonChecked(hDlg, IDC_CHKCOLOR);
	if (b == FALSE)
	{
		EnableDlgItem(hDlg, IDC_COLBACK, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSECOLBACK, FALSE);
		EnableDlgItem(hDlg, IDC_CHKCOLOR2, FALSE);
		EnableDlgItem(hDlg, IDC_COLBACK2, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSECOLBACK2, FALSE);
		EnableDlgItem(hDlg, IDC_CHKCOLORV, FALSE);
	}
	else
	{
		EnableDlgItem(hDlg, IDC_COLBACK, TRUE);
		EnableDlgItem(hDlg, IDC_CHOOSECOLBACK, TRUE);

		if (UseGrad == TRUE)
		{
			EnableDlgItem(hDlg, IDC_CHKCOLOR2, TRUE);
			b = IsDlgButtonChecked(hDlg, IDC_CHKCOLOR2);
			EnableDlgItem(hDlg, IDC_COLBACK2, b);
			EnableDlgItem(hDlg, IDC_CHOOSECOLBACK2, b);
			EnableDlgItem(hDlg, IDC_CHKCOLORV, b);
		}
		else
		{
			EnableDlgItem(hDlg, IDC_CHKCOLOR2, FALSE);
			EnableDlgItem(hDlg, IDC_COLBACK2, FALSE);
			EnableDlgItem(hDlg, IDC_CHOOSECOLBACK2, FALSE);
			EnableDlgItem(hDlg, IDC_CHKCOLORV, FALSE);
		}
	}
	SendPSChanged(hDlg);
}

/*------------------------------------------------
  enable/disable to use "shadow"
--------------------------------------------------*/
void OnCheckShadow(HWND hDlg, int id)
{
	BOOL b;
	if (id == 0)
	{
		if (id == 0 && IsDlgButtonChecked(hDlg, IDC_CHKCLKSHADOW))
			id = IDC_CHKCLKSHADOW;
		else if (id == 0 && IsDlgButtonChecked(hDlg, IDC_CHKCLKBORDER))
			id = IDC_CHKCLKBORDER;
	}

	if (id == IDC_CHKCLKSHADOW)
	{
		b = IsDlgButtonChecked(hDlg, IDC_CHKCLKSHADOW);
		CheckDlgButton(hDlg, IDC_CHKCLKBORDER,FALSE);
		EnableDlgItem(hDlg, IDC_COLCLKSHADOW, b);
		EnableDlgItem(hDlg, IDC_CHOOSECOLCLKSHADOW, b);
		EnableDlgItem(hDlg, IDC_STATICSHADOW, b);
		EnableDlgItem(hDlg, IDC_CLKSHADOWRANGE, b);
		EnableDlgItem(hDlg, IDC_SPINCLKSHADOW, b);
	}
	else if (id == IDC_CHKCLKBORDER)
	{
		b = IsDlgButtonChecked(hDlg, IDC_CHKCLKBORDER);
		CheckDlgButton(hDlg, IDC_CHKCLKSHADOW,FALSE);
		EnableDlgItem(hDlg, IDC_COLCLKSHADOW, b);
		EnableDlgItem(hDlg, IDC_CHOOSECOLCLKSHADOW, b);
		EnableDlgItem(hDlg, IDC_STATICSHADOW, FALSE);
		EnableDlgItem(hDlg, IDC_CLKSHADOWRANGE, FALSE);
		EnableDlgItem(hDlg, IDC_SPINCLKSHADOW, FALSE);
	}
	else
	{
		EnableDlgItem(hDlg, IDC_COLCLKSHADOW, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSECOLCLKSHADOW, FALSE);
		EnableDlgItem(hDlg, IDC_STATICSHADOW, FALSE);
		EnableDlgItem(hDlg, IDC_CLKSHADOWRANGE, FALSE);
		EnableDlgItem(hDlg, IDC_SPINCLKSHADOW, FALSE);
	}
	SendPSChanged(hDlg);
}

// 参考：Visual C++ 4.x のサンプルWORDPADのFORMATBA.CPP

BOOL CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* pelf,
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo);
BOOL CALLBACK EnumSizeProcEx(ENUMLOGFONTEX* pelf,
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo);
int nFontSizes[] =
	{8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};
int logpixelsy;

/*------------------------------------------------
   Initialization of "Font" combo box
--------------------------------------------------*/
void InitComboFont(HWND hDlg)
{
	HDC hdc;
	LOGFONT lf;
	HWND hcombo;
	char s[80], s1[81], s2[81];
	int i;

	hdc = GetDC(NULL);

	// Enumerate fonts and set in the combo box
	memset(&lf, 0, sizeof(LOGFONT));
	hcombo = GetDlgItem(hDlg, IDC_FONT);


	lf.lfCharSet = DEFAULT_CHARSET;  // Arial, Courier, Times New Roman, ...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);


	ReleaseDC(NULL, hdc);

	GetMyRegStr("Color_Font", "Font", s, 80, "");
	if(s[0] == 0)
	{
		HFONT hfont;
		hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if(hfont)
		{
			GetObject(hfont, sizeof(lf),(LPVOID)&lf);
			strcpy(s, lf.lfFaceName);
		}
	}
	//i = CBFindStringExact(hDlg, IDC_FONT, s);
	//if(i == LB_ERR) i = 0;

	strcpy(s1, "*");
	strcat(s1, s);
	strcpy(s2, " ");
	strcat(s2, s);

	i = CBFindStringExact(hDlg, IDC_FONT, s1);
	if (i == LB_ERR)
	{
		i = CBFindStringExact(hDlg, IDC_FONT, s2);
		if (i == LB_ERR) i = 0;
	}

	CBSetCurSel(hDlg, IDC_FONT, i);


	//リスト項目の表示数を指定
	AdjustDlgConboBoxDropDown(hDlg, IDC_FONT, 12);
}

/*------------------------------------------------
　「フォントサイズ」コンボボックスの設定
--------------------------------------------------*/
void SetComboFontSize(HWND hDlg, BOOL bInit)
{
	HDC hdc;
	char s[160];
	DWORD size;
	LOGFONT lf;

	//以前のsizeを保存
	if(bInit) // WM_INITDIALOGのとき
	{
		size = GetMyRegLong("Color_Font", "FontSize", 9);
		if(size == 0) size = 9;
	}
	else   // IDC_FONTが変更されたとき
	{
		size = GetDlgItemInt(hDlg, IDC_FONTSIZE, NULL, FALSE);
	}

	CBResetContent(hDlg, IDC_FONTSIZE);

	hdc = GetDC(NULL);
	logpixelsy = GetDeviceCaps(hdc, LOGPIXELSY);

	logfont_sample_height = logpixelsy * size / 72;	//Added by TTTT

	if (b_DebugLog)
	{
		char strTemp[64];
		wsprintf(strTemp, "[pagecolor.c][SetComboFontSizeTip] logfont_sample_height = %d", logfont_sample_height);
		WriteDebug_New2(strTemp);
	}

	// s = フォント名
	CBGetLBText(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT), (LPARAM)s);

	//フォントのサイズを列挙してコンボボックスに入れる
	memset(&lf, 0, sizeof(LOGFONT));
	strcpy(lf.lfFaceName, s + 1); // +1 is to remove proportional mark
	lf.lfCharSet = (BYTE)CBGetItemData(hDlg, IDC_FONT, CBGetCurSel(hDlg, IDC_FONT));
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumSizeProcEx,
		(LPARAM)GetDlgItem(hDlg, IDC_FONTSIZE), 0);




	ReleaseDC(NULL, hdc);

	// sizeをセット
	SetDlgItemInt(hDlg, IDC_FONTSIZE, size, FALSE);

	//リスト項目の表示数を指定
	AdjustDlgConboBoxDropDown(hDlg, IDC_FONTSIZE, 5);

	RefreshFontSample(hDlg, bInit);



}

/*------------------------------------------------
Refresh Fontsample by TTTT
--------------------------------------------------*/
void RefreshFontSample(HWND hDlg, BOOL bInit)
{
	if (b_DebugLog) WriteDebug_New2("[pagecolor.c] RefreshFontSample Called.");

	int size, temp;
	char strTemp[64];


	size = GetDlgItemInt(hDlg, IDC_FONTSIZE, NULL, FALSE);

	CBGetLBText(hDlg, IDC_FONTSIZE, CBGetCurSel(hDlg, IDC_FONTSIZE), strTemp);
	temp = atoi(strTemp);
	if (temp > 0) size = temp;

	logfont_sample_height = logpixelsy * size / 72;	//Added by TTTT


	logfont_sample.lfWeight = IsDlgButtonChecked(hDlg, IDC_BOLD) * FW_BOLD;
	logfont_sample.lfItalic = IsDlgButtonChecked(hDlg, IDC_ITALIC);
	logfont_sample.lfHeight = logfont_sample_height;
	logfont_sample.lfWidth = logfont_sample.lfEscapement = logfont_sample.lfOrientation = 0;


	//DeleteObject(hfont_sample);
	hfont_sample = CreateFontIndirect(&logfont_sample);

	SendDlgItemMessage(hDlg, IDC_FONTSAMPLE, WM_SETFONT, hfont_sample, TRUE);


}



/*------------------------------------------------
  Callback function for enumerating fonts.
  To set a font name in the combo box.
--------------------------------------------------*/
BOOL CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* pelf,
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo)
{
	UNREFERENCED_PARAMETER(lpntm);
	UNREFERENCED_PARAMETER(FontType);



	// if(FontType & RASTER_FONTTYPE) return 1;
	if(pelf->elfLogFont.lfFaceName[0] != '@' 
		&& pelf->elfLogFont.lfFaceName[0] != '&'
		&& pelf->elfLogFont.lfFaceName[0] != '$'
		&& pelf->elfLogFont.lfFaceName[1] != '@'
		&& pelf->elfLogFont.lfFaceName[1] != '$'
		&& SendMessage((HWND)hCombo, CB_FINDSTRINGEXACT, 0,
			(LPARAM)pelf->elfLogFont.lfFaceName) == LB_ERR)
	{
		{
			int index;
			TCHAR strFont[256];
			TCHAR strType[64];


			// add "*" for proportinal fonts

			if (!(lpntm->ntmTm.tmPitchAndFamily & TMPF_FIXED_PITCH)) strcpy(strType, "*");
			else strcpy(strType, " ");


			wsprintf(strFont, "%s%s", strType, pelf->elfLogFont.lfFaceName);

			if (SendMessage((HWND)hCombo, CB_FINDSTRINGEXACT, 0,
				(LPARAM)strFont) == LB_ERR)
			{
				index = SendMessage((HWND)hCombo, CB_ADDSTRING, 0, strFont);

				if (index >= 0)
				{
					SendMessage((HWND)hCombo, CB_SETITEMDATA,
						index, (LPARAM)pelf->elfLogFont.lfCharSet);

				}

			}
		}

	}
	return 1;
}

/*------------------------------------------------
　フォントの列挙コールバック
　コンボボックスにフォントサイズを入れる
--------------------------------------------------*/
BOOL CALLBACK EnumSizeProcEx(ENUMLOGFONTEX* pelf,
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo)
{
	char s[80];
	int num, i, count;


	logfont_sample = pelf->elfLogFont;

	UNREFERENCED_PARAMETER(pelf);
	//トゥルータイプフォントまたは、
	//トゥルータイプでもラスタフォントでもない場合
	if((FontType & TRUETYPE_FONTTYPE) ||
		!( (FontType & TRUETYPE_FONTTYPE) || (FontType & RASTER_FONTTYPE) ))
	{
		// nFontSizesの数字をそのまま入れる
		for (i = 0; i < 16; i++)
		{
			wsprintf(s, "%d", nFontSizes[i]);
			SendMessage((HWND)hCombo, CB_ADDSTRING, 0, (LPARAM)s);
		}
		return FALSE;
	}

	//それ以外の場合、１つ１つ数字を入れていく
	num = (lpntm->ntmTm.tmHeight - lpntm->ntmTm.tmInternalLeading) * 72 / logpixelsy;
	count = SendMessage((HWND)hCombo, CB_GETCOUNT, 0, 0);
	for(i = 0; i < count; i++)
	{
		SendMessage((HWND)hCombo, CB_GETLBTEXT, i, (LPARAM)s);
		if(num == atoi(s)) return TRUE;
		else if(num < atoi(s)) 
		{
			wsprintf(s, "%d", num);
			SendMessage((HWND)hCombo, CB_INSERTSTRING, i, (LPARAM)s);
			return TRUE;
		}
	}
	wsprintf(s, "%d", num);
	SendMessage((HWND)hCombo, CB_ADDSTRING, 0, (LPARAM)s);
	return TRUE;
}

