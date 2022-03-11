/*-----------------------------------------------------
  tooltip.c
  customize the tray clock
  KAZUBON 1997-2001
-------------------------------------------------------*/
#define COBJMACROS
#include "tcdll.h"

extern HANDLE hmod;
extern HWND hwndClockMain;

extern BOOL b_DebugLog;

extern void GetDisplayTime(SYSTEMTIME* pt, int* beat100);

#define GRADIENT_FILL_RECT_H    0x00000000
#define GRADIENT_FILL_RECT_V    0x00000001

#ifndef TTS_NOANIMATE
#define TTS_NOANIMATE 0x10
#endif
#ifndef TTS_NOFADE
#define TTS_NOFADE 0x20
#endif
#ifndef TTS_BALLOON
#define TTS_BALLOON 0x40
#endif
#ifndef TTM_SETTITLE
#define TTM_SETTITLE 0x420
#endif
#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED 0x80000
#endif
#ifndef WS_EX_COMPOSITED
#define WS_EX_COMPOSITED 0x02000000L
#endif

/* �c�[���`�b�v�̋��e�� */
#define LEN_TOOLTIP   10240

enum
{
	TOOLTIPTYPE_NORMAL = 0,
	TOOLTIPTYPE_BALLOON = 1,
};

static DWORD dwTooltipType = 0;
static DWORD dwTooltipTypeCur = 0;
//static int alphaTooltip = 255;
static int nTooltipIcon = 0;
static HWND hwndTooltip = NULL;
static int iTooltipSelected = 0;
static BOOL bTooltipCustomDrawDisable = FALSE;
static BOOL bTooltipUpdateEnable[3] = {FALSE, FALSE, FALSE};
//static BOOL bTooltipEnableDoubleBuffering = FALSE;
static BOOL bTooltipUpdated = FALSE;
static BOOL bTooltipShow = FALSE;
static BOOL bTooltipBalloon = FALSE;
static BOOL bTooltipHasTitle = FALSE;

static HFONT hFonTooltip = NULL;

static HFONT hFonTooltipTitle = NULL;

static int iTooltipDispIntervalCount = 0;
static int iTooltipDispInterval = 1;
static int iTooltipSizeX;
static int iTooltipSizeY;
static DWORD dwTooltipFonCol;
static DWORD dwTooltipTitleCol;
static DWORD dwTooltipBakCol;
static COLORREF colTooltipText, colTooltipBack, colTooltipTitle;

static char formatTooltip[LEN_TOOLTIP];
static WCHAR formatTooltipW[LEN_TOOLTIP];
static char titleTooltip[300] = { 0 };
static HRESULT hrOleInitialize = E_NOTIMPL;

static DWORD dwMousePosPrev = 0;
static int nTooltipHtmlDispCount = 0;
static HWND hwndHTMLParent = NULL;
static ATOM atomHTMLParent = 0;
static BOOL bEmbedBrowserObject = FALSE;

extern char g_mydir_dll[];	//added by TTTT

char fmtToolTip1[LEN_TOOLTIP];
char fmtToolTip2[LEN_TOOLTIP];
char fmtToolTip3[LEN_TOOLTIP];

BOOL bEnableTooltip = TRUE;
BOOL bTooltip2, bTooltip3, bTooltipTate;
char tiptitle[300];



static void TooltipApplySetting(void)
{
	COLORREF tcolf, tcolb;
	tcolf = dwTooltipFonCol; tcolb = dwTooltipBakCol;
	//if(tcolf & 0x80000000) tcolf = GetSysColor(tcolf & 0x00ffffff);
	//if(tcolb & 0x80000000) tcolb = GetSysColor(tcolb & 0x00ffffff);
	colTooltipText = tcolf;
	colTooltipBack = tcolb;
	switch(dwTooltipTypeCur)
	{
	case TOOLTIPTYPE_NORMAL:
	case TOOLTIPTYPE_BALLOON:
		if(hwndTooltip)
		{
			LONG_PTR exstyle;
			exstyle = GetWindowLongPtr(hwndTooltip, GWL_EXSTYLE) & ~WS_EX_COMPOSITED;
//			SetWindowLongPtr(hwndTooltip, GWL_EXSTYLE, exstyle | (bTooltipEnableDoubleBuffering ? WS_EX_COMPOSITED : 0));
			SetWindowLongPtr(hwndTooltip, GWL_EXSTYLE, exstyle);
			if (hFonTooltip) SendMessage(hwndTooltip, WM_SETFONT, (WPARAM)hFonTooltip, TRUE);
			SendMessage(hwndTooltip, TTM_SETTIPBKCOLOR, tcolb, 0);
			SendMessage(hwndTooltip, TTM_SETTIPTEXTCOLOR, tcolf, 0);
		}
		break;
	}
}

static void TooltipUpdateText(void);
static void TooltipUpdate2(HDC hdc, LPRECT lprcDraw, LPRECT lprect, UINT uDrawFlags);

static const CHAR cszWindowClassName[] = "__tclock html tooltip parent__";
static LRESULT CALLBACK MyWindowWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_ACTIVATE:
			break;
		default:
			return DefWindowProcA(hwnd, message, wParam, lParam);
	}
	return DefWindowProcA(hwnd, message, wParam, lParam);
}
static ATOM MyWindowRegisterClass(void)
{
	WNDCLASS wndcls;
	wndcls.style         = 0;
	wndcls.lpfnWndProc   = MyWindowWndProc;
	wndcls.cbClsExtra    = 0;
	wndcls.cbWndExtra    = 0;
	wndcls.hInstance     = GetModuleHandleA(NULL);
#pragma warning(push)
#pragma warning(disable: 4305)	// '�^�L���X�g' : 'LPSTR' ���� 'WORD' �֐؂�l�߂܂��B
	wndcls.hIcon         = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	wndcls.hCursor       = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
#pragma warning(pop)
	wndcls.hbrBackground = NULL;
	wndcls.lpszMenuName  = NULL;
	wndcls.lpszClassName = cszWindowClassName;
	return RegisterClass(&wndcls);
}
static void MyWindowUnregisterClass(void)
{
	UnregisterClass(cszWindowClassName, GetModuleHandleA(NULL));
}

/*------------------------------------------------
�@�c�[���`�b�v�E�B���h�E�̍쐬
--------------------------------------------------*/
void TooltipInit(HWND hwnd)
{
	TOOLINFO ti;
	extern int widthMainClockFrame, heightMainClockFrame;
	extern BOOL bWin11Main;

	dwTooltipTypeCur = dwTooltipType;
	switch(dwTooltipTypeCur)
	{
	case TOOLTIPTYPE_NORMAL:
	case TOOLTIPTYPE_BALLOON:
		//hwndTooltip = CreateWindowEx(WS_EX_LAYERED, TOOLTIPS_CLASS, (LPSTR)NULL,
		//	TTS_ALWAYSTIP | TTS_NOPREFIX | ((dwTooltipTypeCur == TOOLTIPTYPE_BALLOON) ? TTS_BALLOON : 0),
		//	CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		//	NULL, NULL, hmod, NULL);
		hwndTooltip = CreateWindowEx(0, TOOLTIPS_CLASS, (LPSTR)NULL,
			TTS_ALWAYSTIP | TTS_NOPREFIX | ((dwTooltipTypeCur == TOOLTIPTYPE_BALLOON) ? TTS_BALLOON : 0),
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, NULL, hmod, NULL);
		if(!hwndTooltip) return;

		SetWindowPos(hwndTooltip, HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);

		bTooltipBalloon = (GetWindowLongPtr(hwndTooltip, GWL_STYLE) & TTS_BALLOON) != 0;

		ti.cbSize = sizeof(TOOLINFO);
//		ti.hwnd = hwnd;		//�ȉ��̍s�Ɠ����e
		ti.hwnd = hwndClockMain;
		ti.uId = 1;

		if (bWin11Main) {
			ti.uFlags = 0;	//Win11�ł�TTF_SUBCLASS������ƏI�����ɃN���b�V������B�}�E�X���������ۂ�1���������͏o�Ȃ��͗l�H
		}
		else {
			ti.uFlags = TTF_SUBCLASS;	//TTF_SUBCLASS�����Ȃ��ƁA�}�E�X���������ۂ̍��W�����Ɉړ������ۂ�1�������(�_�ł���)�BVer4.1�ȍ~
		}

		ti.hinst = NULL;
		ti.lpszText = LPSTR_TEXTCALLBACK;
		ti.rect.left = 0;
		ti.rect.top = 0;
		//ti.rect.right = 480;
		//ti.rect.bottom = 480;

		//������Ă΂�鎞�_�Ń��C���N���b�N�T�C�Y���m�肵�Ă���͂��Ȃ̂ŁF
		ti.rect.right = widthMainClockFrame;
		ti.rect.bottom = heightMainClockFrame;


		TooltipApplySetting();
		TooltipUpdateText();

		SendMessage(hwndTooltip, TTM_SETMAXTIPWIDTH, 0, (LPARAM) LEN_TOOLTIP);
		SendMessage(hwndTooltip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO)&ti);
		SendMessage(hwndTooltip, TTM_ACTIVATE, TRUE, 0);
		break;
	}
}

void TooltipDeleteRes(void)
{
	if(hFonTooltip) DeleteObject(hFonTooltip); hFonTooltip = NULL;	//635@p5
	if (hFonTooltipTitle) DeleteObject(hFonTooltipTitle); hFonTooltipTitle = NULL;
}

void TooltipEnd(HWND hwnd)
{
	if (b_DebugLog)writeDebugLog_Win10("[tooltip.c] TooltipEnd called.", 999);
	switch(dwTooltipTypeCur)
	{
	case TOOLTIPTYPE_NORMAL:
	case TOOLTIPTYPE_BALLOON:
		if(hwndTooltip)
		{
			TOOLINFO ti;
			ti.cbSize = sizeof(TOOLINFO);
			ti.hwnd = hwnd;
			ti.uId = 1;
			SendMessage(hwndTooltip, TTM_ACTIVATE, FALSE, 0);
			SendMessage(hwndTooltip, TTM_DELTOOL, 0, (LPARAM) (LPTOOLINFO)&ti);
			DestroyWindow(hwndTooltip);
			hwndTooltip = NULL;
		}
		break;
	}
}

void TooltipOnRefresh(HWND hwnd)
{
	//if (dwTooltipTypeCur == dwTooltipType)
	//	TooltipApplySetting();
	//else
	{
		TooltipEnd(hwnd);
		TooltipInit(hwnd);
	}
}

//	2002/11/26 - H.N. Neri-uma added.
//	2002/11/27 - 635 modified.
//-------------------------------------------------
//	GetTooltipText
//		Tooltip�̃e�L�X�g���A���W�X�g������ł͂Ȃ��A�e�L�X�g�t�@�C������ǂݍ��݂܂��B
//	�����F	[char * pszText]	�ǂݍ��񂾕�������i�[����ϐ��ւ̃|�C���^
//	�߂�l�F0		�G���[�B�e�L�X�g�t�@�C�������݂��Ȃ������B�t�@�C���͑��݂������A�󂾂����B�Ȃ�
//			0�ȊO	����
//-------------------------------------------------
static BOOL GetTooltipText(PSTR pszText)
{
	HANDLE	hFile;
	char	szFilePath[MAX_PATH];	//	�e�L�X�g�t�@�C���̃p�X
	DWORD	dwReadSize;
	DWORD	dwFileSize;
	size_t	len;
	BOOL	bAbsPath;

//	{
//		GetCurrentDirectory(MAX_PATH, pszText);
//		return TRUE;
//	}

	if(!pszText)
	{
		//	�l���i�[����A�h���X�����݂��Ȃ��̂ŁAFALSE��Ԃ��ďI��
		return FALSE;
	}


	bAbsPath = FALSE;
	len = strlen(pszText);
	if (len >= 2) {
		if ((*pszText == '\\') && (*(pszText + 1) == '\\')) {
			//UNC name
			bAbsPath = TRUE;
		} else if (*(pszText + 1) == ':') {
			bAbsPath = TRUE;
		}
	}
	if (bAbsPath == FALSE) {
		// TClock�̈ʒu����p�X�Ƃ��Ďw�蕶����𑊑΃p�X�Ƃ��Ēǉ�
		GetModuleFileName(hmod, szFilePath, sizeof(szFilePath));
		del_title(szFilePath);
		if (len + strlen(szFilePath) >= MAX_PATH) {
			return FALSE;
		}
		add_title(szFilePath, pszText);
		*pszText = '\0';
	} else {
		//635@p5 �t�@�C�������擾���ăc�[���`�b�v���N���A�B
		strcpy(szFilePath, pszText);
		*pszText = '\0';
	}

	//	�t�@�C����ǂݍ���
	hFile = CreateFile(szFilePath,
					   GENERIC_READ,
					   FILE_SHARE_READ,
					   NULL,
					   OPEN_EXISTING,
					   FILE_ATTRIBUTE_NORMAL,
					   NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		//	�t�@�C�������݂��Ȃ��̂ŁAFALSE��Ԃ��ďI��
		return FALSE;
	}

	dwFileSize = GetFileSize(hFile, NULL);

	if ( dwFileSize > (DWORD)LEN_TOOLTIP)
		dwFileSize = (DWORD)(LEN_TOOLTIP-1);

	//	�t�@�C���ǂݍ���
	ReadFile(hFile, pszText, dwFileSize, &dwReadSize, NULL);
	//	�t�@�C�����N���[�Y
	CloseHandle(hFile);

	if(strcmp(pszText, "") == 0)
	{
		//	������擾�Ɏ��s�B���W�X�g���擾�֑��点�邽�߂�FALSE��Ԃ��ďI��
		return FALSE;
	}

	//	������̖����ɁA�����I��NULL String��t���i^^;
	pszText[dwFileSize] = '\0';

	//	���������̂�TRUE��Ԃ�
	return TRUE;
}






/*------------------------------------------------
draw tooltip	
Ver4.1�ȍ~�B
�V�X�e���`��E�^�C�g���ʕ`����g��Ȃ��O��̐V�����֐��B������͂��������}���ł���B
�T�C�Y�̓V�X�e�����Ő������ݒ肳��Ă���(�����X�V��ʂ���)���̂Ƃ��čĒ������s��Ȃ����Ƃŉ��̂�������}���\�B
--------------------------------------------------*/
static void TooltipUpdate2(HDC hdc, LPRECT lprcDraw, LPRECT lprect, UINT uDrawFlags)
{
//	if (b_DebugLog)writeDebugLog_Win10("[tooltip.c] TooltipUpdate2 called, lprcDraw =", lprcDraw);

	//HGDIOBJ hOldFont;
	HBRUSH hBrushTooltipBack = NULL, hBrushTooltipText = NULL;
	RECT rc, rcall;
	LPSTR pszText;
	int maxwidth, width, height, len;
	LONG top;


	//hBrushTooltipText = CreateSolidBrush(colTooltipText);
	hBrushTooltipBack = CreateSolidBrush(colTooltipBack);

	rcall = *lprcDraw;




	//if (b_DebugLog) {
	//	writeDebugLog_Win10("[tooltip.c][TooltipUpdate2] *lprcDraw.left =", rcall.left);
	//	writeDebugLog_Win10("[tooltip.c][TooltipUpdate2] *lprcDraw.right =", rcall.right);
	//	writeDebugLog_Win10("[tooltip.c][TooltipUpdate2] *lprcDraw.top =", rcall.top);
	//	writeDebugLog_Win10("[tooltip.c][TooltipUpdate2] *lprcDraw.bottom =", rcall.bottom);
	//}

	if (lprect)	//���̊֐��ł̓T�C�Y�ύX���s��Ȃ��̂ł��̂܂܃T�C�Y��Ԃ�
	{
		lprect->top = rcall.top;
		lprect->left = rcall.left;
		lprect->bottom = rcall.bottom;
		lprect->right = rcall.right;
	}





	SelectObject(hdc, hFonTooltip);

	rc = rcall;
	top = rcall.top;		//top�����̏o�͂�Y�ʒu(��)
	maxwidth = 0;

	HDC hdcTemp;
	hdcTemp = CreateCompatibleDC(hdc);
	if (hdcTemp)
	{
		//SelectObject(hdcTemp, hBrushTooltipText);
		SetBkColor(hdcTemp, colTooltipBack);
		SetBkMode(hdcTemp, TRANSPARENT);

		HBITMAP hbmpTemp;


		//�^�C�g���\��
		if (titleTooltip[0] != 0) {
			SelectObject(hdcTemp, hFonTooltipTitle);
			SetTextColor(hdcTemp, colTooltipTitle);

			pszText = titleTooltip;
				len = 0;

				while (pszText[len] && pszText[len] != '\r' && pszText[len] != '\n') len++;
				rc = rcall;
				rc.top = top;
				rc.right = 1980;	//���s���Ȃ��悤�ɉ��zRECT�𒷂�����B
				if (len)
				{
					height = DrawText(hdcTemp, pszText, len, &rc, uDrawFlags | DT_CALCRECT);
					width = rc.right - rc.left;
				}
				else
				{
					height = DrawText(hdcTemp, " ", 1, &rc, uDrawFlags | DT_CALCRECT);
				}

//				HBITMAP hbmpTemp;
				hbmpTemp = NULL;
				hbmpTemp = CreateCompatibleBitmap(hdc, rcall.right - rcall.left, height);
				if (hbmpTemp)
				{
					SelectObject(hdcTemp, hbmpTemp);

					rc.top = 0;
					rc.bottom = height;
					rc.left = 0;
					rc.right = rcall.right - rcall.left;

					FillRect(hdcTemp, &rc, hBrushTooltipBack);
					if (len)
					{
						rc.left = ((rcall.right - rcall.left) - width) / 2;
						//rc.right = ((rcall.right - rcall.left) + width) / 2;
						DrawText(hdcTemp, pszText, len, &rc, uDrawFlags | DT_SINGLELINE);
					}
					BitBlt(hdc, rcall.left, top, rcall.right - rcall.left, height, hdcTemp, 0, 0, SRCCOPY);
					DeleteObject(hbmpTemp);

				}
				top += height;
		}







		
		SelectObject(hdcTemp, hFonTooltip);
		SetTextColor(hdcTemp, colTooltipText);

		pszText = formatTooltip;

		if (titleTooltip[0] != 0) {		//�^�C�g���̈�m�ۗp�̉��s��2�폜����B
			if (*pszText == 0x0d/*'\r'*/) pszText++;
			if (*pszText == 0x0a/*'\n'*/) pszText++;
			if (*pszText == 0x0d/*'\r'*/) pszText++;
			if (*pszText == 0x0a/*'\n'*/) pszText++;
		}

		while (*pszText)
		{
			len = 0;

			while (pszText[len] && pszText[len] != '\r' && pszText[len] != '\n') len++;
			rc = rcall;
			rc.top = top;

			if (len)
			{
				height = DrawText(hdcTemp, pszText, len, &rc, uDrawFlags | DT_CALCRECT);
			}
			else
			{
				height = DrawText(hdcTemp, " ", 1, &rc, uDrawFlags | DT_CALCRECT);
			}

			//HBITMAP hbmpTemp;

			hbmpTemp = NULL;
			hbmpTemp = CreateCompatibleBitmap(hdc, rcall.right - rcall.left, height);
			if (hbmpTemp)
			{
				SelectObject(hdcTemp, hbmpTemp);

				rc.top = 0;
				rc.bottom = height;
				rc.left = 0;
				rc.right = rcall.right - rcall.left;

				FillRect(hdcTemp, &rc, hBrushTooltipBack);
				if (len)
				{
					DrawText(hdcTemp, pszText, len, &rc, uDrawFlags);
				}
				BitBlt(hdc, rcall.left, top, rcall.right - rcall.left, height, hdcTemp, 0, 0, SRCCOPY);
				DeleteObject(hbmpTemp);

			}
			top += height;
			pszText += len;

			if (*pszText == 0x0d/*'\r'*/) pszText++;
			if (*pszText == 0x0a/*'\n'*/) pszText++;	//���s1�X�L�b�v�����
		}
	}
	DeleteDC(hdcTemp);


	if (top < rcall.bottom)
	{
		rc.top = top;
		rc.bottom = rcall.bottom;
		rc.left = rcall.left;
		rc.right = rcall.right;
		FillRect(hdc, &rc, hBrushTooltipBack);
	}

	DeleteObject(hBrushTooltipBack);
	//DeleteObject(hBrushTooltipText);


}



/*------------------------------------------------
�@�c�[���`�b�v�̕\�����e�擾
--------------------------------------------------*/
static void TooltipUpdateText(void)
{

	SYSTEMTIME t;
	int beat100;
	char fmt[LEN_TOOLTIP], s[LEN_TOOLTIP], s_info[LEN_TOOLTIP], tipt[300], tipt_info[300], ini_key[10];
	COLORREF s_col[LEN_TOOLTIP],tipt_col[300];
	DWORD dw;
	RECT rcClock;
	int clLen, mPos;


	bTooltipUpdated = TRUE;
	if (bTooltip2)
	{

		GetWindowRect(hwndClockMain, &rcClock);
		dw = GetMessagePos();
		if (bTooltipTate)
		{
			clLen = rcClock.bottom - rcClock.top;
			mPos  = GET_Y_LPARAM(dw) - rcClock.top;
		}
		else
		{
			clLen = rcClock.right - rcClock.left;
			mPos  = GET_X_LPARAM(dw) - rcClock.left;
		}
		if (bTooltip3)
		{
			if ( mPos > (2*clLen/3) )
				iTooltipSelected = 2;
			if ( mPos <= (2*clLen/3) && mPos >= (clLen/3) )
				iTooltipSelected = 1;
			if ( mPos < (clLen/3) )
				iTooltipSelected = 0;
		}
		else
		{
			if ( mPos > (clLen/2) )
				iTooltipSelected = 1;
			if ( mPos <= (clLen/2) )
				iTooltipSelected = 0;
		}
	}
	else
	{
		iTooltipSelected = 0;
	}


	switch (iTooltipSelected) {
	case 0:
		strcpy(fmt, fmtToolTip1);
		break;
	case 1:
		strcpy(fmt, fmtToolTip2);
		break;
	case 2:
		strcpy(fmt, fmtToolTip3);
		break;
	}

	if(fmt[0] == 'f' && fmt[1] == 'i' && fmt[2] == 'l' && fmt[3] == 'e' && fmt[4] == ':')
	{
		memmove( fmt, fmt + 5, (size_t)((strchr(fmt,'\0')-1)-fmt));
		if(!GetTooltipText(fmt)){
			strcpy(fmt, "�t�@�C���擾���s");
		}
	}
	if(fmt[0] == 0)
	{
		strcpy(fmt, "TClock <%LDATE%>");
	}

	GetDisplayTime(&t, &beat100);
	MakeFormat(s, s_info, &t, beat100, fmt);
	//strcpy(formatTooltip, s);

	if(tiptitle[0] != 0)
	{
		MakeFormat(tipt, tipt_info, &t, beat100, tiptitle);
		strcpy(titleTooltip, tipt);
		sprintf(formatTooltip, "\n\n\n%s", s);	//�^�C�g��������ꍇ�͍����m�ۂ̂��߂̋�sx3������B
	}
	else
	{
		strcpy(titleTooltip, "");
		strcpy(formatTooltip, s);
	}
}

/*------------------------------------------------
�@�c�[���`�b�v�̕\���X�V
--------------------------------------------------*/
void TooltipOnTimer(HWND hwnd)
{

	//Ver 4.1�ȍ~��OnTimer_Win10����s�����ƂƂ���B
	//���̂��߂̌���200ms���Ƃ̃J�E���g�͖�����
	//if (++iTooltipDispIntervalCount < iTooltipDispInterval * 5) return;		//200ms�P�ʂȂ̂�5���1�񂵂��ʉ߂��Ȃ��B
	//iTooltipDispIntervalCount = 0;

	if (b_DebugLog)writeDebugLog_Win10("[tooltip.c] TooltipOnTimer called, bTooltipShow =", bTooltipShow);

	if (!bTooltipShow) return;

	if (!bTooltipUpdateEnable[iTooltipSelected]) return;

	switch(dwTooltipTypeCur)
	{
	case TOOLTIPTYPE_NORMAL:
	case TOOLTIPTYPE_BALLOON:
		if (hwndTooltip)
		{
			TooltipUpdateText();


			TOOLINFO ti;

			ti.cbSize = sizeof(TOOLINFO);
			ti.hwnd = hwnd;
			ti.uId = 1;
			SendMessage(hwndTooltip, TTM_GETTOOLINFO, 0, (LPARAM)(LPTOOLINFO)&ti);
//			ti.uFlags = 0;
//			ti.hinst = NULL;
//			ti.lpszText = LPSTR_TEXTCALLBACK;
			SendMessage(hwndTooltip, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&ti);
		}
		break;
	}
}

void TooltipReadData(void)
{
	char fontname[80];
	int fontsize;
	LONG weight, italic;

	GetMyRegStr("Tooltip", "TipFont", fontname, 80, "");
	if (fontname[0] == 0) 
	{
		HFONT hfont;
		hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if(hfont)
		{
			LOGFONT lf;
			GetObject(hfont, sizeof(lf),(LPVOID)&lf);
			strcpy(fontname, lf.lfFaceName);
		}
	}

	fontsize = GetMyRegLong("Tooltip", "TipFontSize", 9);
	weight = GetMyRegLong("Tooltip", "TipBold", 0);
	if(weight) weight = FW_BOLD;
	else weight = 0;
	italic = GetMyRegLong("Tooltip", "TipItalic", 0);

	if(hFonTooltip) DeleteObject(hFonTooltip);
	hFonTooltip = CreateMyFont(fontname, fontsize, weight, italic);

	if (hFonTooltipTitle) DeleteObject(hFonTooltipTitle);
	hFonTooltipTitle = CreateMyFont("Yu Gothic UI", 12, FW_REGULAR, 0);

	dwTooltipType = GetMyRegLong("Tooltip", "BalloonFlg", 1);

	bTooltipCustomDrawDisable = FALSE;

	bTooltipUpdateEnable[0] = GetMyRegLong("Tooltip", "Tip1Update", 0);
	bTooltipUpdateEnable[1] = GetMyRegLong("Tooltip", "Tip2Update", 0);
	bTooltipUpdateEnable[2] = GetMyRegLong("Tooltip", "Tip3Update", 0);

	bTooltip2 = GetMyRegLong("Tooltip", "Tip2Use", FALSE);
	bTooltip3 = GetMyRegLong("Tooltip", "Tip3Use", FALSE);
	bTooltipTate = GetMyRegLong("Tooltip", "TipTateFlg", FALSE);

	bEnableTooltip = GetMyRegLong("Tooltip", "EnableTooltip", TRUE);
	SetMyRegLong("Tooltip", "EnableTooltip", bEnableTooltip);

	GetMyRegStr("Tooltip", "Tooltip", fmtToolTip1, sizeof(fmtToolTip1), "");
	GetMyRegStr("Tooltip", "Tooltip2", fmtToolTip2, sizeof(fmtToolTip1), "");
	GetMyRegStr("Tooltip", "Tooltip3", fmtToolTip3, sizeof(fmtToolTip1), "");

	GetMyRegStr("Tooltip", "TipTitle", tiptitle, 300, "TClock-Win10 <%VerTC%>");
	SetMyRegStr("Tooltip", "TipTitle", tiptitle);

	dwTooltipFonCol = GetMyRegLong("Tooltip", "TipFontColor", 0x80000000 | COLOR_INFOTEXT);
	if (dwTooltipFonCol & 0x80000000) dwTooltipFonCol = GetSysColor(dwTooltipFonCol & 0x00ffffff);
	SetMyRegLong("Tooltip", "TipFontColor", dwTooltipFonCol);

	dwTooltipBakCol = GetMyRegLong("Tooltip", "TipBakColor", 0x80000000 | COLOR_INFOBK);
	if (dwTooltipBakCol & 0x80000000) dwTooltipBakCol = GetSysColor(dwTooltipBakCol & 0x00ffffff);
	SetMyRegLong("Tooltip", "TipBakColor", dwTooltipBakCol);


	colTooltipTitle = GetMyRegLong("Tooltip", "TipTitleColor", RGB(0,0,255));
	SetMyRegLong("Tooltip", "TipTitleColor", colTooltipTitle);

	//bTooltipEnableDoubleBuffering = GetMyRegLong("Tooltip", "TipEnableDoubleBuffering", FALSE);
	//bTooltipEnableDoubleBuffering = FALSE;

	//nTooltipIcon = GetMyRegLong("Tooltip", "TipIcon", 0);


	iTooltipDispInterval = 1;

}

BOOL TooltipOnNotify(LRESULT *plRes, LPARAM lParam)
{
	if (b_DebugLog)writeDebugLog_Win10("[tooltip.c] TooltipOnNotify called with code=", ((LPNMHDR)lParam)->code);




	if (!hwndTooltip || hwndTooltip != ((LPNMHDR)lParam)->hwndFrom) return FALSE;
	switch (((LPNMHDR)lParam)->code)
	{
		case NM_CUSTOMDRAW:
//			if (b_DebugLog)writeDebugLog_Win10("[tooltip.c][TooltipOnNotify] NM_CUSTOMDRAW received.", ((LPNMHDR)lParam)->code);
			if (bTooltipCustomDrawDisable) break;
			if (((LPNMCUSTOMDRAW)lParam)->dwDrawStage == CDDS_PREPAINT)
			{
//				if (b_DebugLog)writeDebugLog_Win10("[tooltip.c][TooltipOnNotify] dwDrawStage == CDDS_PREPAINT.", 999);
				if (((LPNMCUSTOMDRAW)lParam)->rc.top)
				{
					RECT rc;
					TooltipUpdate2(((LPNMCUSTOMDRAW)lParam)->hdc, &((LPNMCUSTOMDRAW)lParam)->rc, &rc, ((LPNMTTCUSTOMDRAW)lParam)->uDrawFlags);
//					ExcludeClipRect(((LPNMCUSTOMDRAW)lParam)->hdc, rc.left, rc.top, rc.right, rc.bottom);
					ExcludeClipRect(((LPNMCUSTOMDRAW)lParam)->hdc, ((LPNMCUSTOMDRAW)lParam)->rc.left, ((LPNMCUSTOMDRAW)lParam)->rc.top, ((LPNMCUSTOMDRAW)lParam)->rc.right, ((LPNMCUSTOMDRAW)lParam)->rc.bottom);
				}
				else
				{
					ExcludeClipRect(((LPNMCUSTOMDRAW)lParam)->hdc, ((LPNMCUSTOMDRAW)lParam)->rc.left, ((LPNMCUSTOMDRAW)lParam)->rc.top, ((LPNMCUSTOMDRAW)lParam)->rc.right, ((LPNMCUSTOMDRAW)lParam)->rc.bottom);
				}
				*plRes = CDRF_NOTIFYPOSTPAINT;
				return TRUE;
			}
			if (((LPNMCUSTOMDRAW)lParam)->dwDrawStage == CDDS_POSTPAINT)
			{
//				if (b_DebugLog)writeDebugLog_Win10("[tooltip.c][TooltipOnNotify] dwDrawStage == CDDS_POSTPAINT.", 999);
				HRGN hrgn = CreateRectRgn(((LPNMCUSTOMDRAW)lParam)->rc.left, ((LPNMCUSTOMDRAW)lParam)->rc.top, ((LPNMCUSTOMDRAW)lParam)->rc.right, ((LPNMCUSTOMDRAW)lParam)->rc.bottom);
				if (hrgn)
				{
					ExtSelectClipRgn(((LPNMCUSTOMDRAW)lParam)->hdc, hrgn, RGN_OR);
					DeleteObject(hrgn);
				}
				*plRes = 0;
				return TRUE;
			}
//			if (b_DebugLog)writeDebugLog_Win10("[tooltip.c][TooltipOnNotify] dwDrawStage == OTHER.", 999);
			break;
		case TTN_NEEDTEXTW:
//			if (b_DebugLog)writeDebugLog_Win10("[tooltip.c][TooltipOnNotify] TTN_NEEDTEXTW received.", ((LPNMHDR)lParam)->code);
			if (!bTooltipShow || !bTooltipUpdated)
			{
				TooltipUpdateText();
			}
			MultiByteToWideChar(CP_ACP, 0, formatTooltip, -1,
				formatTooltipW, sizeof(formatTooltipW)/sizeof(WCHAR));
			((LPTOOLTIPTEXTW)lParam)->lpszText = formatTooltipW;
			*plRes = 0;
			return TRUE;
		case TTN_NEEDTEXT:
//			if (b_DebugLog)writeDebugLog_Win10("[tooltip.c][TooltipOnNotify] TTN_NEEDTEXT received.", ((LPNMHDR)lParam)->code);
			if (!bTooltipShow || !bTooltipUpdated)
			{
				TooltipUpdateText();
			}
			((LPTOOLTIPTEXT)lParam)->lpszText = formatTooltip;
			*plRes = 0;
			return TRUE;
		case TTN_SHOW:
//			if (b_DebugLog)writeDebugLog_Win10("[tooltip.c][TooltipOnNotify] TTN_SHOW received.", ((LPNMHDR)lParam)->code);
			if (!bTooltipShow)
			{
				TooltipApplySetting();
				TooltipUpdateText();
				SetWindowPos(hwndTooltip, HWND_TOPMOST, 0, 0, 0, 0,
					SWP_NOSIZE|SWP_NOMOVE|SWP_NOACTIVATE);
				bTooltipShow = TRUE;
			}
			break;
		case TTN_POP:
//			if (b_DebugLog)writeDebugLog_Win10("[tooltip.c][TooltipOnNotify] TTN_POP received.", ((LPNMHDR)lParam)->code);
			bTooltipShow = FALSE;
			bTooltipUpdated = FALSE;

			break;
	}
	return FALSE;
}

DWORD TooltipFindFormat(void)
{
	DWORD dwInfoTooltip = 0;
	char tip[LEN_TOOLTIP];
	char tip_temp[LEN_TOOLTIP];
	char tip_title[LEN_TOOLTIP];
	char tip_path[LEN_TOOLTIP];


	strcpy(tip, fmtToolTip1);
	//�t�@�C���w��ł������ꍇ
	if(tip[0] == 'f' && tip[1] == 'i' && tip[2] == 'l' && tip[3] == 'e' && tip[4] == ':')
	{
		//Path��؂�o��
		memmove( tip, tip + 5, (size_t)((strchr(tip,'\0')-1)-tip));
		strcpy(tip_path, tip);
		del_title(tip_path);
		get_title(tip_title, tip);
		if (b_DebugLog)
		{
			writeDebugLog_Win10("Tooltip1 setting retrieved", 999);
			char strDebug[1024];

			wsprintf(strDebug, "%s%s", "[tooltip.c][TooltipFindFormat] tip: ", tip);
			writeDebugLog_Win10(strDebug, 999);

			wsprintf(strDebug, "%s%s", "[tooltip.c][TooltipFindFormat] tip_path: ", tip_path);
			writeDebugLog_Win10(strDebug, 999);

			wsprintf(strDebug, "%s%s", "[tooltip.c][TooltipFindFormat] tip_title: ", tip_title);
			writeDebugLog_Win10(strDebug, 999);
		}
		if (!strcmp(tip_path, g_mydir_dll) || !GetTooltipText(tip))		//Path���A�v���p�X�Ɠ��ꂩ�H
		{
			//Path���A�v���p�X�Ɠ���A�������̓t�@�C���擾���s�̏ꍇ�̓p�X���폜����B
			if (b_DebugLog) writeDebugLog_Win10("tip_path and g_mydir_dll are identical", 999);
			strcpy(tip_temp, "file:");
			strcat(tip_temp, tip_title);
			strcpy(tip, tip_title);
			if (!GetTooltipText(tip))
			{
				//�t�@�C�������݂��Ȃ��ꍇ�̓f�t�H���g�t�@�C�����ɒ����ă��g���C
				strcpy(tip, "tclock_tooltip1.txt");
				if (!GetTooltipText(tip))
				{
					strcpy(tip, "tclock_tooltip.txt");
					if (!GetTooltipText(tip))
					{
						strcpy(tip, "�t�@�C���擾���s(ToolTip1)");
						//SetMyRegStr("Tooltip", "Tooltip", "TClock <%LDATE%>");
					}
					else
					{
						SetMyRegStr("Tooltip", "Tooltip", "file:tclock_tooltip.txt");
					}
				}
				else
				{
					SetMyRegStr("Tooltip", "Tooltip", "file:tclock_tooltip1.txt");
				}
			}
			else
			{
			SetMyRegStr("Tooltip", "Tooltip", tip_temp);
			}
		}
	}
	dwInfoTooltip |= FindFormat(tip);
	//	Tooltip�̃e�L�X�g�����W�X�g������ǂݍ���
	//GetMyRegStr("Tooltip", "Tooltip2", tip, sizeof(tip), "");
	strcpy(tip, fmtToolTip2);
	//�t�@�C���w��ł������ꍇ
	if(tip[0] == 'f' && tip[1] == 'i' && tip[2] == 'l' && tip[3] == 'e' && tip[4] == ':')
	{
		//Path��؂�o��
		memmove( tip, tip + 5, (size_t)((strchr(tip,'\0')-1)-tip));
		strcpy(tip_path, tip);
		del_title(tip_path);
		get_title(tip_title, tip);
		if (b_DebugLog)
		{
			writeDebugLog_Win10("Tooltip2 setting retrieved", 999);
			char strDebug[1024];

			wsprintf(strDebug, "%s%s", "[tooltip.c][TooltipFindFormat] tip: ", tip);
			writeDebugLog_Win10(strDebug, 999);

			wsprintf(strDebug, "%s%s", "[tooltip.c][TooltipFindFormat] tip_path: ", tip_path);
			writeDebugLog_Win10(strDebug, 999);

			wsprintf(strDebug, "%s%s", "[tooltip.c][TooltipFindFormat] tip_title: ", tip_title);
			writeDebugLog_Win10(strDebug, 999);
		}
		if (!strcmp(tip_path, g_mydir_dll) || !GetTooltipText(tip))		//Path���A�v���p�X�Ɠ��ꂩ�H
		{
			//Path���A�v���p�X�Ɠ���A�������̓t�@�C���擾���s�̏ꍇ�̓p�X���폜����B
			if (b_DebugLog) writeDebugLog_Win10("tip_path and g_mydir_dll are identical", 999);
			strcpy(tip_temp, "file:");
			strcat(tip_temp, tip_title);
			strcpy(tip, tip_title);
			if (!GetTooltipText(tip))
			{
				//�t�@�C�������݂��Ȃ��ꍇ�̓f�t�H���g�t�@�C�����ɒ����ă��g���C
				strcpy(tip, "tclock_tooltip2.txt");
				if (!GetTooltipText(tip))
				{
					strcpy(tip, "�t�@�C���擾���s(ToolTip2)");
					//SetMyRegStr("Tooltip", "Tooltip2", "TClock <%LDATE%>");
				}
				else
				{
					SetMyRegStr("Tooltip", "Tooltip2", "file:tclock_tooltip2.txt");
				}
			}
			else
			{
				SetMyRegStr("Tooltip", "Tooltip2", tip_temp);
			}
		}
	}
	dwInfoTooltip |= FindFormat(tip);
	//	Tooltip�̃e�L�X�g�����W�X�g������ǂݍ���
	//GetMyRegStr("Tooltip", "Tooltip3", tip, sizeof(tip), "");
	strcpy(tip, fmtToolTip3);
	//�t�@�C���w��ł������ꍇ
	if(tip[0] == 'f' && tip[1] == 'i' && tip[2] == 'l' && tip[3] == 'e' && tip[4] == ':')
	{
		//Path��؂�o��
		memmove( tip, tip + 5, (size_t)((strchr(tip,'\0')-1)-tip));
		strcpy(tip_path, tip);
		del_title(tip_path);
		get_title(tip_title, tip);
		if (b_DebugLog)
		{
			writeDebugLog_Win10("Tooltip3 setting retrieved", 999);
			char strDebug[1024];

			wsprintf(strDebug, "%s%s", "[tooltip.c][TooltipFindFormat] tip: ", tip);
			writeDebugLog_Win10(strDebug, 999);

			wsprintf(strDebug, "%s%s", "[tooltip.c][TooltipFindFormat] tip_path: ", tip_path);
			writeDebugLog_Win10(strDebug, 999);

			wsprintf(strDebug, "%s%s", "[tooltip.c][TooltipFindFormat] tip_title: ", tip_title);
			writeDebugLog_Win10(strDebug, 999);
		}
		if (!strcmp(tip_path, g_mydir_dll) || !GetTooltipText(tip))		//Path���A�v���p�X�Ɠ��ꂩ�H
		{
			//Path���A�v���p�X�Ɠ���A�������̓t�@�C���擾���s�̏ꍇ�̓p�X���폜����B
			if (b_DebugLog) writeDebugLog_Win10("tip_path and g_mydir_dll are identical", 999);
			strcpy(tip_temp, "file:");
			strcat(tip_temp, tip_title);
			strcpy(tip, tip_title);
			if (!GetTooltipText(tip))
			{
				//�t�@�C�������݂��Ȃ��ꍇ�̓f�t�H���g�t�@�C�����ɒ����ă��g���C
				strcpy(tip, "tclock_tooltip3.txt");
				if (!GetTooltipText(tip))
				{
					strcpy(tip, "�t�@�C���擾���s(ToolTip3)");
				}
				else
				{
					SetMyRegStr("Tooltip", "Tooltip3", "file:tclock_tooltip2.txt");
				}
			}
			else
			{
				SetMyRegStr("Tooltip", "Tooltip3", tip_temp);
			}
		}
	}
	dwInfoTooltip |= FindFormat(tip);

	strcpy(tip, tiptitle);
	dwInfoTooltip |= FindFormat(tip);
	return dwInfoTooltip;
}

void TooltipOnMouseEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//	if (b_DebugLog)writeDebugLog_Win10("[tooltip.c] TooltipOnMouseEvent called", 999);




	MSG msg;
	int tempInt;
	extern posXMainClock;

	switch(dwTooltipTypeCur)
	{
	case TOOLTIPTYPE_NORMAL:
	case TOOLTIPTYPE_BALLOON:
		msg.hwnd = hwnd;
		msg.message = message;
		msg.wParam = wParam;
		msg.lParam = lParam;		//���΍��W
		msg.time = GetMessageTime();
		msg.pt.x = GET_X_LPARAM(GetMessagePos());		//��΍��W
		msg.pt.y = GET_Y_LPARAM(GetMessagePos());		//��΍��W


		//if (b_DebugLog) {
		//	writeDebugLog_Win10("[tooltip.c][TooltipOnMouseevent] time =", msg.time);
		//	writeDebugLog_Win10("[tooltip.c][TooltipOnMouseevent] lParam.HIWORD =", HIWORD(msg.lParam));
		//	writeDebugLog_Win10("[tooltip.c][TooltipOnMouseevent] lParam.LOWORD =", LOWORD(msg.lParam));
		//	writeDebugLog_Win10("[tooltip.c][TooltipOnMouseevent] Xpos =", msg.pt.x);
		//	writeDebugLog_Win10("[tooltip.c][TooltipOnMouseevent] Ypos =", msg.pt.y);
		//}

		//�}�E�X��TClock�����ō��Ɉړ�����ꍇ�A�ǂ�����Ă��c�[���`�b�v���ŏ��Ɉ������邽�߁A�_�ł��Ă��܂��B
		//�����ō��W���׍H���Ă����ۂɂ͐V���Ƀ}�E�X���W���擾���ď������邽�߉����ł��Ȃ��B
		//���ǂ́A�c�[���o�^�̍ۂ�ti.uFlags��TTF_SUBCLASS����ꂽ����������B


		if(hwndTooltip)
		{
			SendMessage(hwndTooltip, TTM_RELAYEVENT, 0, (LPARAM)&msg);
		}
		break;
	}
}
