
#include "tclock.h"

static int MaxCol = 16;
extern int confNo;

/*------------------------------------------------
　「色」コンボボックスの初期化
--------------------------------------------------*/
void InitComboColor(HWND hDlg, int numcol, COMBOCOLOR *pCombocolor, int maxcol, BOOL ex_flg)
{
	COLORREF col = 0;
	int i, j;
	WORD id;
	char section[256];

	if (strcmp(pCombocolor[0].colname, "BackNetColSend") == 0) strcpy(section, "Graph");
	else if (strcmp(pCombocolor[0].colname, "BackColor") == 0) strcpy(section, "Color_Font");
	else if (strcmp(pCombocolor[0].colname, "Saturday_TextColor") == 0) strcpy(section, "Color_Font");
	else if (strcmp(pCombocolor[0].colname, "AClockHourHandColor") == 0) strcpy(section, "AnalogClock");
	else if (strcmp(pCombocolor[0].colname, "TipFontColor") == 0) strcpy(section, "Tooltip");
	else strcpy(section, "");


	//Windowsデフォルト16色+ボタンの色
	static COLORREF rgb[20] = {
		RGB(0,0,0),
		RGB(128,0,0),
		RGB(0,128,0),
		RGB(128,128,0),
		RGB(0,0,128),
		RGB(128,0,128),
		RGB(0,128,128),
		RGB(192,192,192),
		RGB(128,128,128),
		RGB(255,0,0),
		RGB(0,255,0),
		RGB(255,255,0),
		RGB(0,0,255),
		RGB(255,0,255),
		RGB(0,255,255),
		RGB(255,255,255),
		0x80000000|COLOR_3DFACE,
		0x80000000|COLOR_3DSHADOW,
		0x80000000|COLOR_3DHILIGHT,
		0x80000000|COLOR_BTNTEXT
	};

	MaxCol = maxcol;

	for(i = 0; i < numcol; i++)
	{
		id = pCombocolor[i].id;

		// WinXPのバグへの対処(VisualStyle使用時は0を追加できない)
		CBSetItemData(hDlg, id, CBAddString(hDlg, id, -1), rgb[0]);
		for(j = 1; j < maxcol; j++) //黒以外の残りの基本16色+ボタンの色
			CBAddString(hDlg, id, rgb[j]);

		//コンボボックスの色を選択
		if (ex_flg)
		{
			if (pCombocolor[i].defcol == 0xFFFFFFFF)
				col = GetMyRegLong(section, pCombocolor[i].colname, col);
			else
				col = GetMyRegLong(section, pCombocolor[i].colname, pCombocolor[i].defcol);
		}
		else
		{
			if (pCombocolor[i].defcol == 0xFFFFFFFF)
				col = GetMyRegLong(section, pCombocolor[i].colname, col);
			else
				col = GetMyRegLong(section, pCombocolor[i].colname, pCombocolor[i].defcol);
		}

		for(j = 0; j < maxcol; j++)
		{
			if(col == (COLORREF)CBGetItemData(hDlg, id, j))
				break;
		}
		if(j == maxcol) //20色中にないとき
			CBAddString(hDlg, id, col);
		CBSetCurSel(hDlg, id, j);

		//リスト項目の表示数を指定
		AdjustDlgConboBoxDropDown(hDlg, id, 10);
	}
}

/*------------------------------------------------
　「色」コンボボックスの高さの設定
--------------------------------------------------*/
void OnMeasureItemColorCombo(LPARAM lParam)
{
	LPMEASUREITEMSTRUCT pmis;

	pmis = (LPMEASUREITEMSTRUCT)lParam;
	pmis->itemHeight = 7 * HIWORD(GetDialogBaseUnits()) / 8;
}

/*------------------------------------------------
　「色」コンボボックスのオーナードロー
--------------------------------------------------*/
void OnDrawItemColorCombo(LPARAM lParam, int maxcol)
{
	LPDRAWITEMSTRUCT pdis;
	HBRUSH hbr;
	COLORREF col;
	TEXTMETRIC tm;
	int y;

	pdis = (LPDRAWITEMSTRUCT)lParam;

	if(IsWindowEnabled(pdis->hwndItem))
	{
		col = (COLORREF)pdis->itemData;
		if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);
	}
	else col = GetSysColor(COLOR_3DFACE);

	switch(pdis->itemAction)
	{
		case ODA_DRAWENTIRE:
		case ODA_SELECT:
		{
			hbr = CreateSolidBrush(col);
			FillRect(pdis->hDC, &pdis->rcItem, hbr);
			DeleteObject(hbr);

			// print color names
			if(maxcol > 16 && 16 <= pdis->itemID && pdis->itemID <= 19)
			{
				char s[80];
				strcpy(s, MyString(IDS_BTNFACE + pdis->itemID - 16));
				SetBkMode(pdis->hDC, TRANSPARENT);
				GetTextMetrics(pdis->hDC, &tm);
				if((GetRValue(col)+GetGValue(col)+GetBValue(col)) <= 384)
					SetTextColor(pdis->hDC, RGB(255,255,255));
				else
					SetTextColor(pdis->hDC, RGB(0,0,0));
				y = (pdis->rcItem.bottom - pdis->rcItem.top - tm.tmHeight)/2;
				TextOut(pdis->hDC, pdis->rcItem.left + 4, pdis->rcItem.top + y,
					s, strlen(s));
			}
			if(!(pdis->itemState & ODS_FOCUS)) break;
		}
		case ODA_FOCUS:
		{
			hbr = CreateSolidBrush((pdis->itemState & ODS_FOCUS)? 0 : col);
			FrameRect(pdis->hDC, &pdis->rcItem, hbr);
			DeleteObject(hbr);
			break;
		}
	}
}

/*------------------------------------------------
　色の選択 「...」ボタン
--------------------------------------------------*/
void OnChooseColor(HWND hDlg, WORD id, int maxcol)
{
	CHOOSECOLOR cc;
	COLORREF col, colarray[16];
	WORD idCombo;
	int i;

	idCombo = id - 1;

	//最初に選ばれている色
	col = (COLORREF)CBGetItemData(hDlg, idCombo, CBGetCurSel(hDlg, idCombo));
	if(col & 0x80000000) col = GetSysColor(col & 0x00ffffff);

	for(i = 0; i < 16; i++) colarray[i] = RGB(255,255,255);

	memset(&cc, 0, sizeof(CHOOSECOLOR));
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = hDlg;
	cc.hInstance = (HWND)g_hInst;
	cc.rgbResult = col;
	cc.lpCustColors = colarray;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if(!ChooseColor(&cc)) return;

	for(i = 0; i < 16; i++)
	{
		if(cc.rgbResult == (COLORREF)CBGetItemData(hDlg, idCombo, i))
			break;
	}
	if(i == 16) //基本16色ではないとき
	{
		if(CBGetCount(hDlg, idCombo) == maxcol)
			CBAddString(hDlg, idCombo, cc.rgbResult);
		else
			CBSetItemData(hDlg, idCombo, maxcol, cc.rgbResult);
		i = maxcol;
	}
	CBSetCurSel(hDlg, idCombo, i);

	PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);
}


