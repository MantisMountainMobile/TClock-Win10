/*-------------------------------------------
  pagetooltip.c
　　「ツールチップ」プロパティページ
　　KAZUBON 1997-1998 / 635@p5
---------------------------------------------*/

#include "tclock.h"


static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnDestroy(HWND hDlg);
static void OnSansho(HWND hDlg, WORD id);
static void OnTipSwitch(HWND hDlg, WORD id);
static void InitComboFontTip(HWND hDlg); //635@p5
static void SetComboFontSizeTip(HWND hDlg, BOOL bInit);
static HFONT hfontb;  // for IDC_BOLD
static HFONT hfonti;  // for IDC_ITALIC
static COMBOCOLOR combocolor[3] = {
	{ IDC_TFONCOL,      "TipFontColor",  0x80000000|COLOR_BTNTEXT },
	{ IDC_TBAKCOL,      "TipBakColor",   0x80000000|COLOR_INFOBK },
	{ IDC_TTITLECOL,      "TipTitleColor", 0x00FF0000 }
};


LOGFONT  logfont_sample;
int logfont_sample_height_tip;

extern BOOL b_EnglishMenu;
extern int Language_Offset;

void RefreshFontSample_ToolTip(HWND hDlg);

extern BOOL b_DebugLog;

//static char reg_section[] = "";

static HFONT hfont_sample_tip;

typedef struct {
	BOOL disable;
	int func[4];
	char format[4][256];
	char fname[4][256];
} CLICKDATA;
static CLICKDATA *pData = NULL;

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyTaskbar = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
　「ツールチップ」ページ用ダイアログプロシージャ
--------------------------------------------------*/
BOOL CALLBACK PageTooltipProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
			OnDrawItemColorCombo(lParam, 20);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			switch(id)
			{
			case IDC_TFONT:
				if(code == CBN_SELCHANGE)
				{
					SetComboFontSizeTip(hDlg, FALSE);
					SendPSChanged(hDlg);
					g_bApplyClock = TRUE;
				}
				break;
			case IDC_TFONTSIZE:
				if(code == CBN_SELCHANGE || code == CBN_EDITCHANGE)
				{
					RefreshFontSample_ToolTip(hDlg);
					SendPSChanged(hDlg);
					g_bApplyClock = TRUE;
				}
				break;
			case IDC_TBOLD:
			case IDC_TITALIC:
				RefreshFontSample_ToolTip(hDlg);
				SendPSChanged(hDlg);
				g_bApplyClock = TRUE;
				break;

			case IDC_ALPHATIP:
			case IDC_ALTIPSPIN:
			//case IDC_DISPT:
			case IDC_DISPTSPIN:
			case IDC_TIPENABLE:
			case IDC_TIPTATE:
			case IDC_UPDATETIP1:
			case IDC_UPDATETIP2:
			case IDC_UPDATETIP3:
			//case IDC_DISABLETIPCUSTOMDRAW:
			//case IDC_ENABLEDOUBLEBUFFERING:
			//case IDC_HTMLSIZEX:
			//case IDC_HTMLSIZEY:
				SendPSChanged(hDlg);
				g_bApplyClock = TRUE;
				break;
			case IDC_TIP2:
				OnTipSwitch(hDlg, id);
				break;
			case IDC_TIP3:
				SendPSChanged(hDlg);
				break;
			case IDC_TFONCOL:
			case IDC_TBAKCOL:
			case IDC_TTITLECOL:
			case IDC_TICON:
			case IDC_BALLOONFLG:
				if(code == CBN_SELCHANGE)
				{
					g_bApplyClock = TRUE;
					SendPSChanged(hDlg);
				}
				break;
			case IDC_Win10ITLE:
				if(code == EN_CHANGE)
				{
					g_bApplyClock = TRUE;
					SendPSChanged(hDlg);
				}
				break;
			case IDC_CHOOSETFONCOL:
			case IDC_CHOOSETBAKCOL:
			case IDC_CHOOSETTITLECOL:
				OnChooseColor(hDlg, id, 20);
				SendPSChanged(hDlg);
				break;
			case IDC_SANSHOTIP:
			case IDC_SANSHOTIP2:
			case IDC_SANSHOTIP3:
				OnSansho(hDlg, id);
				break;
			case IDC_TOOLTIP:
			case IDC_TOOLTIP2:
			case IDC_TOOLTIP3:
				if(code == EN_CHANGE)
				{
					g_bApplyClock = TRUE;
					SendPSChanged(hDlg);
				}
				break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: 
					OnApply(hDlg); 
					break;
				case PSN_HELP: 
					My2chHelp(GetParent(hDlg)); 
					break;
			}
			return TRUE;
		case WM_DESTROY:
			DeleteObject(hfontb);
			DeleteObject(hfonti);
			DeleteObject(hfont_sample_tip);
			OnDestroy(hDlg);
			return TRUE;
	}
	return FALSE;
}

/*------------------------------------------------
　ページの初期化
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	char s[1024];
	int i;
	HFONT hfont;
	LOGFONT logfont;
	DWORD dw;
	HDC hdc;
	BOOL bTip2;

	// setting of "background" and "text"
	InitComboColor(hDlg, 3, combocolor, 20, TRUE);

	// if color depth is 256 or less
	hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
	if(GetDeviceCaps(hdc, BITSPIXEL) <= 8)
	{
		EnableDlgItem(hDlg, IDC_CHOOSETFONCOL, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSETBAKCOL, FALSE);
		EnableDlgItem(hDlg, IDC_CHOOSETTITLECOL, FALSE);
	}
	DeleteDC(hdc);

	hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	if(hfont)
	{
		SendDlgItemMessage(hDlg, IDC_TOOLTIP,
			WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_TOOLTIP2,
			WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_TOOLTIP3,
			WM_SETFONT, (WPARAM)hfont, 0);
		SendDlgItemMessage(hDlg, IDC_TFONT,
			WM_SETFONT, (WPARAM)hfont, 0);

		GetObject(hfont, sizeof(LOGFONT), &logfont_sample);	//added by TTTT for sample
		logfont_sample_height_tip = logfont_sample.lfHeight;
	}

	//「フォント」の設定
	InitComboFontTip(hDlg);
	//「フォントサイズ」の設定
	SetComboFontSizeTip(hDlg, TRUE);


	for(i = IDS_TICONNO; i <= IDS_TICONERR; i++)
		CBAddString(hDlg, IDC_TICON, (LPARAM)MyString(i));
	CBSetCurSel(hDlg, IDC_TICON,
		GetMyRegLong("Tooltip", "TipIcon", 0));

	GetMyRegStr("Tooltip", "TipTitle", s, 300, "");
	SetDlgItemText(hDlg, IDC_Win10ITLE, s);
	//end

	GetMyRegStr("Tooltip", "Tooltip", s, 1024, "");
	if(s[0] == 0) strcpy(s, "TClock <%LDATE%>");
	SetDlgItemText(hDlg, IDC_TOOLTIP, s);
	GetMyRegStr("Tooltip", "Tooltip2", s, 1024, "");
	if(s[0] == 0) strcpy(s, "TClock <%LDATE%>");
	SetDlgItemText(hDlg, IDC_TOOLTIP2, s);
	GetMyRegStr("Tooltip", "Tooltip3", s, 1024, "");
	if(s[0] == 0) strcpy(s, "TClock <%LDATE%>");
	SetDlgItemText(hDlg, IDC_TOOLTIP3, s);

	bTip2 = GetMyRegLong("Tooltip", "Tip2Use", FALSE);
	CheckDlgButton(hDlg, IDC_TIP2,bTip2);
	CheckDlgButton(hDlg, IDC_TIP3,
		GetMyRegLong("Tooltip", "Tip3Use", FALSE));

	dw = GetMyRegLong("Tooltip", "Tip2Use", FALSE);

	CheckDlgButton(hDlg, IDC_UPDATETIP1, GetMyRegLong("Tooltip", "Tip1Update", FALSE));
	CheckDlgButton(hDlg, IDC_UPDATETIP2, GetMyRegLong("Tooltip", "Tip2Update", FALSE));
	CheckDlgButton(hDlg, IDC_UPDATETIP3, GetMyRegLong("Tooltip", "Tip3Update", FALSE));
	//CheckDlgButton(hDlg, IDC_DISABLETIPCUSTOMDRAW, GetMyRegLong("Tooltip", "TipDisableCustomDraw", TRUE));
	//CheckDlgButton(hDlg, IDC_ENABLEDOUBLEBUFFERING, GetMyRegLong("Tooltip", "TipEnableDoubleBuffering", FALSE));


	SetDlgItemText(hDlg, IDC_TOOLTIP3, s);
	if (!bTip2){
		CheckDlgButton(hDlg, IDC_TIP3,FALSE);
		EnableDlgItem(hDlg,IDC_TIP3,FALSE);
	}

	CheckDlgButton(hDlg, IDC_TIPENABLE,
		GetMyRegLong("Tooltip", "EnableTooltip", TRUE));

	CheckDlgButton(hDlg, IDC_TIPTATE,
		GetMyRegLong("Tooltip", "TipTateFlg", FALSE));
	CheckDlgButton(hDlg, IDC_TBOLD,
		GetMyRegLong("Tooltip", "TipBold", FALSE));
	CheckDlgButton(hDlg, IDC_TITALIC,
		GetMyRegLong("Tooltip", "TipItalic", FALSE));
	//for(i = IDS_TIPTYPENORMAL; i <= IDS_TIPTYPEIECOMP; i++)
	for (i = IDS_TIPTYPENORMAL; i <= IDS_TIPTYPEBALLOON; i++)
		CBAddString(hDlg, IDC_BALLOONFLG, (LPARAM)MyString(i));
	CBSetCurSel(hDlg, IDC_BALLOONFLG,
		GetMyRegLong("Tooltip", "BalloonFlg", 0));
	AdjustDlgConboBoxDropDown(hDlg, IDC_BALLOONFLG, 3);


	hfontb = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	GetObject(hfontb, sizeof(LOGFONT), &logfont);
	logfont.lfWeight = FW_BOLD;
	hfontb = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hDlg, IDC_TBOLD, WM_SETFONT, (WPARAM)hfontb, 0);

	logfont.lfWeight = FW_NORMAL;
	logfont.lfItalic = 1;
	hfonti = CreateFontIndirect(&logfont);
	SendDlgItemMessage(hDlg, IDC_TITALIC, WM_SETFONT, (WPARAM)hfonti, 0);


	dw = GetMyRegLong("Tooltip", "AlphaTip", 0);
	if(dw > 100) dw = 100;
	if(dw < 0  ) dw = 0;
	SendDlgItemMessage(hDlg,IDC_ALTIPSPIN,UDM_SETRANGE,0,
		(LPARAM) MAKELONG((short)100, (short)0));
	SendDlgItemMessage(hDlg, IDC_ALTIPSPIN, UDM_SETPOS, 0,
		(int)(short)dw);

	//dw = GetMyRegLong("Tooltip", "TipDispTime", 5);
	//if(dw > 30) dw = 30;
	//if(dw < 1  ) dw = 1;
	//SendDlgItemMessage(hDlg,IDC_DISPTSPIN,UDM_SETRANGE,0,
	//	(LPARAM) MAKELONG((short)30, (short)1));
	//SendDlgItemMessage(hDlg, IDC_DISPTSPIN, UDM_SETPOS, 0,
	//	(int)(short)dw);

}





/*------------------------------------------------
　更新
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	char s[1024];
	int n;
	DWORD dw;
	COLORREF col;

	GetDlgItemText(hDlg, IDC_TOOLTIP, s, 1024);
	SetMyRegStr("Tooltip", "Tooltip", s);
	GetDlgItemText(hDlg, IDC_TOOLTIP2, s, 1024);
	SetMyRegStr("Tooltip", "Tooltip2", s);
	GetDlgItemText(hDlg, IDC_TOOLTIP3, s, 1024);
	SetMyRegStr("Tooltip", "Tooltip3", s);

	SetMyRegLong("Tooltip", "Tip2Use", IsDlgButtonChecked(hDlg, IDC_TIP2));
	SetMyRegLong("Tooltip", "Tip3Use", IsDlgButtonChecked(hDlg, IDC_TIP3));
	SetMyRegLong("Tooltip", "TipTateFlg", IsDlgButtonChecked(hDlg, IDC_TIPTATE));
	SetMyRegLong("Tooltip", "EnableTooltip", IsDlgButtonChecked(hDlg, IDC_TIPENABLE));

	SetMyRegLong("Tooltip", "Tip1Update", IsDlgButtonChecked(hDlg, IDC_UPDATETIP1));
	SetMyRegLong("Tooltip", "Tip2Update", IsDlgButtonChecked(hDlg, IDC_UPDATETIP2));
	SetMyRegLong("Tooltip", "Tip3Update", IsDlgButtonChecked(hDlg, IDC_UPDATETIP3));
	//SetMyRegLong("Tooltip", "TipDisableCustomDraw", IsDlgButtonChecked(hDlg, IDC_DISABLETIPCUSTOMDRAW));
	//SetMyRegLong("Tooltip", "TipEnableDoubleBuffering", IsDlgButtonChecked(hDlg, IDC_ENABLEDOUBLEBUFFERING));

	//フォント名の保存
	CBGetLBText(hDlg, IDC_TFONT, CBGetCurSel(hDlg, IDC_TFONT), s);
	SetMyRegStr("Tooltip", "TipFont", s + 1); // +1 is to remove proportinal mark "*"

	//フォントサイズの保存
	//5以下のサイズは許可しない(数値でないものも含まれる)
	dw = GetDlgItemInt(hDlg, IDC_TFONTSIZE, NULL, FALSE);
	if(dw < 5)
	{
		dw = 9;
		SetDlgItemInt(hDlg, IDC_TFONTSIZE, dw, FALSE);
	}
	SetMyRegLong("Tooltip", "TipFontSize", dw);

	//「Bold」「Italic」の保存
	SetMyRegLong("Tooltip", "TipBold", IsDlgButtonChecked(hDlg, IDC_TBOLD));
	SetMyRegLong("Tooltip", "TipItalic", IsDlgButtonChecked(hDlg, IDC_TITALIC));
	SetMyRegLong("Tooltip", "BalloonFlg", CBGetCurSel(hDlg, IDC_BALLOONFLG));

	//dw = GetDlgItemInt(hDlg, IDC_ALPHATIP, NULL, FALSE);
	//if(dw > 100) dw = 100;
	//if(dw < 0  ) dw = 0;
	//SetDlgItemInt(hDlg, IDC_ALPHATIP, dw, FALSE);

	//SetMyRegLong("Tooltip", "AlphaTip", dw);

	//dw = GetDlgItemInt(hDlg, IDC_DISPT, NULL, FALSE);
	//if(dw > 30) dw = 30;
	//if(dw < 1  ) dw = 1;
	//SetDlgItemInt(hDlg, IDC_DISPT, dw, FALSE);

	//SetMyRegLong("Tooltip", "TipDispTime", dw);



	dw = (DWORD)CBGetItemData(hDlg, IDC_TFONCOL, CBGetCurSel(hDlg, IDC_TFONCOL));
	SetMyRegLong("Tooltip", "TipFontColor", dw);
	dw = (DWORD)CBGetItemData(hDlg, IDC_TBAKCOL, CBGetCurSel(hDlg, IDC_TBAKCOL));
	SetMyRegLong("Tooltip", "TipBakColor", dw);

	col = (COLORREF)CBGetItemData(hDlg, IDC_TTITLECOL, CBGetCurSel(hDlg, IDC_TTITLECOL));
	SetMyRegLong("Tooltip", "TipTitleColor", col);

	//n = CBGetCurSel(hDlg, IDC_TICON);
	//SetMyRegLong("Tooltip", "TipIcon", n);
	GetDlgItemText(hDlg, IDC_Win10ITLE, s, 300);
	SetMyRegStr("Tooltip", "TipTitle", s);


	//PostMessage(g_hwndMain, WM_TIMER, 6, NULL);


}

/*------------------------------------------------

--------------------------------------------------*/
void OnDestroy(HWND hDlg)
{
	UNREFERENCED_PARAMETER(hDlg);
	if(pData) free(pData);
}

/*------------------------------------------------
　「...」　ファイルの参照
--------------------------------------------------*/
void OnSansho(HWND hDlg, WORD id)
{
	char filter[80], deffile[MAX_PATH], fname[MAX_PATH], tipfname[MAX_PATH];

	filter[0] = 0;
	str0cat(filter, MyString(IDS_TIPFILEEXT));
	str0cat(filter, "*.txt");
	str0cat(filter, MyString(IDS_ALLFILE));
	str0cat(filter, "*.*");

	GetDlgItemText(hDlg, id - 1, deffile, MAX_PATH);

	if(!SelectMyFile(hDlg, filter, 0, deffile, fname)) // propsheet.c
		return;

	strcpy(tipfname,"file:");
	strcat(tipfname,fname);
	strcpy(fname,tipfname);

	SetDlgItemText(hDlg, id - 1, fname);
	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
	SendPSChanged(hDlg);
}

void OnTipSwitch(HWND hDlg, WORD id)
{
	UNREFERENCED_PARAMETER(id);
	if(IsDlgButtonChecked(hDlg, IDC_TIP2))
	{
		EnableDlgItem(hDlg,IDC_TIP3,TRUE);
	}
	else
	{
		CheckDlgButton(hDlg, IDC_TIP3,FALSE);
		EnableDlgItem(hDlg,IDC_TIP3,FALSE);
	}
	SendPSChanged(hDlg);
}

BOOL CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* pelf,
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo);
BOOL CALLBACK EnumSizeProcEx(ENUMLOGFONTEX* pelf,
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM hCombo);
int logpixelsyTip;

/*------------------------------------------------
   Initialization of "Font" combo box
--------------------------------------------------*/
void InitComboFontTip(HWND hDlg)
{
	HDC hdc;
	LOGFONT lf;
	HWND hcombo;
	char s[80], s1[81], s2[81];
	int i;

	hdc = GetDC(NULL);

	// Enumerate fonts and set in the combo box
	memset(&lf, 0, sizeof(LOGFONT));
	hcombo = GetDlgItem(hDlg, IDC_TFONT);
	lf.lfCharSet = (BYTE)GetTextCharset(hdc);  // MS UI Gothic, ...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	lf.lfCharSet = OEM_CHARSET;   // Small Fonts, Terminal...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	lf.lfCharSet = DEFAULT_CHARSET;  // Arial, Courier, Times New Roman, ...
	EnumFontFamiliesEx(hdc, &lf,
		(FONTENUMPROC)EnumFontFamExProc, (LPARAM)hcombo, 0);
	ReleaseDC(NULL, hdc);

	GetMyRegStr("Tooltip", "TipFont", s, 80, "");
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

	strcpy(s1, "*");
	strcat(s1, s);
	strcpy(s2, " ");
	strcat(s2, s);

	i = CBFindStringExact(hDlg, IDC_TFONT, s1);
	if (i == LB_ERR)
	{
		i = CBFindStringExact(hDlg, IDC_TFONT, s2);
		if (i == LB_ERR) i = 0;
	}

	CBSetCurSel(hDlg, IDC_TFONT, i);

	//リスト項目の表示数を指定
	AdjustDlgConboBoxDropDown(hDlg, IDC_TFONT, 12);
}

/*------------------------------------------------
　「フォントサイズ」コンボボックスの設定
--------------------------------------------------*/
void SetComboFontSizeTip(HWND hDlg, BOOL bInit)
{
	HDC hdc;
	char s[160];
	DWORD size;
	LOGFONT lf;

	//以前のsizeを保存
	if(bInit) // WM_INITDIALOGのとき
	{
		size = GetMyRegLong("Tooltip", "TipFontSize", 9);
		if(size == 0) size = 9;

	}
	else   // IDC_TFONTが変更されたとき
	{
		size = GetDlgItemInt(hDlg, IDC_TFONTSIZE, NULL, FALSE);
	}

	CBResetContent(hDlg, IDC_TFONTSIZE);

	hdc = GetDC(NULL);
	logpixelsyTip = GetDeviceCaps(hdc, LOGPIXELSY);

	logfont_sample_height_tip = logpixelsyTip * size / 72;	//Added by TTTT

	if (b_DebugLog)
	{
		char strTemp[64];
		wsprintf(strTemp, "[pagetootip.c][SetComboFontSizeTip] logfont_sample_height_tip = %d", logfont_sample_height_tip);
		WriteDebug_New2(strTemp);
	}

	// s = フォント名
	CBGetLBText(hDlg, IDC_TFONT, CBGetCurSel(hDlg, IDC_TFONT), (LPARAM)s);

	//フォントのサイズを列挙してコンボボックスに入れる
	memset(&lf, 0, sizeof(LOGFONT));
	strcpy(lf.lfFaceName, s + 1); // +1 is to remove proportinal mark "*"
	lf.lfCharSet = (BYTE)CBGetItemData(hDlg, IDC_TFONT, CBGetCurSel(hDlg, IDC_TFONT));
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumSizeProcEx,
		(LPARAM)GetDlgItem(hDlg, IDC_TFONTSIZE), 0);



	ReleaseDC(NULL, hdc);

	// sizeをセット
	SetDlgItemInt(hDlg, IDC_TFONTSIZE, size, FALSE);


	//リスト項目の表示数を指定
	AdjustDlgConboBoxDropDown(hDlg, IDC_TFONTSIZE, 10);

	RefreshFontSample_ToolTip(hDlg);

}

/*------------------------------------------------
Refresh Fontsample by TTTT
--------------------------------------------------*/
void RefreshFontSample_ToolTip(HWND hDlg)
{
	if (b_DebugLog) WriteDebug_New2("[pagetooltip.c] RefreshFontSample_ToolTip Called.");


	int size, temp;
	char strTemp[64];


	size = GetDlgItemInt(hDlg, IDC_TFONTSIZE, NULL, FALSE);

	CBGetLBText(hDlg, IDC_TFONTSIZE, CBGetCurSel(hDlg, IDC_TFONTSIZE), strTemp);
	temp = atoi(strTemp);
	if (temp > 0) size = temp;

	logfont_sample_height_tip = logpixelsyTip * size / 72;	//Added by TTTT

	logfont_sample.lfWeight = IsDlgButtonChecked(hDlg, IDC_TBOLD) * FW_BOLD;
	logfont_sample.lfItalic = IsDlgButtonChecked(hDlg, IDC_TITALIC);
	logfont_sample.lfHeight = logfont_sample_height_tip;
	logfont_sample.lfWidth = logfont_sample.lfEscapement = logfont_sample.lfOrientation = 0;

	DeleteObject(hfont_sample_tip);
	hfont_sample_tip = CreateFontIndirect(&logfont_sample);

	SendDlgItemMessage(hDlg, IDC_FONTSAMPLE_TOOLTIP, WM_SETFONT, hfont_sample_tip, TRUE);




}