#include "tcdll.h"
#include "resource.h"





#define TIMEOUT_MS_TCLOCKBARWIN11	100



//非Win11専用
extern BOOL b_DebugLog;
extern HANDLE hmod;
extern HWND hwndClockMain;
extern HWND hwndTrayMain;
extern HWND hwndTClockExeMain;
extern HWND hwndTaskBarMain;
extern HWND hwndTClockBarWin11;
extern HWND hwndDesktop;

extern BOOL bEnableSubClks;
extern BOOL b_BatteryLifeAvailable;
extern int widthTaskbar;
extern int widthMainClockFrame;
extern int heightMainClockFrame;
extern int widthMainClockFrame;
extern int heightTaskbar;

extern int posXTaskbar;
extern int posYTaskbar;

extern int originalWidthTaskbar;
extern int originalHeightTaskbar;
extern int originalPosYTaskbar;

//Win11対応関連
//Win11対応自作ウィンドウモードフラグ
extern BOOL bWin11Main;
extern BOOL bWin11Sub;

extern int Win11Type;

//Win11用関連ウィンドウハンドル
extern HWND hwndWin11ReBarWin;
extern HWND hwndWin11ContentBridge;
extern HWND hwndWin11InnerTrayContentBridge;
extern HWND hwndWin11Notify;

//通知ウィンドウ用
//利用設定・フラグ
extern BOOL bUseWin11Notify;		//ユーザ設定
extern BOOL bEnabledWin11Notify;	//利用するかどうか(通知ウィンドウ自体は常に作る)
									//通知ウィンドウ描画用
extern HDC hdcYesWin11Notify;
extern HDC hdcNoWin11Notify;
extern HDC hdcFocusWin11Notify;
extern HDC hdcWin11Notify;

extern HBITMAP hbm_DIBSection_YesWin11Notify;
extern HBITMAP hbm_DIBSection_NoWin11Notify;
extern HBITMAP hbm_DIBSection_FocusWin11Notify;
extern HBITMAP hbm_DIBSection_Win11Notify;
extern HBITMAP hbmpIconYesWin11Notify;
extern HBITMAP hbmpIconNoWin11Notify;
extern HBITMAP hbmpIconFocusWin11Notify;

extern RGBQUAD* m_color_Win11Notify_start;
extern RGBQUAD* m_color_Win11Notify_end;
extern RGBQUAD* m_color_YesWin11Notify_start;
extern RGBQUAD* m_color_YesWin11Notify_end;
extern RGBQUAD* m_color_NoWin11Notify_start;
extern RGBQUAD* m_color_NoWin11Notify_end;
extern RGBQUAD* m_color_FocusWin11Notify_start;
extern RGBQUAD* m_color_FocusWin11Notify_end;
extern COLORREF colWin11Notify;

extern BOOL fillbackcolor;

//現時点で通知が出ているかどうか
extern BOOL bExistWin11Notify;
//通知ウィンドウのサイズ
extern int widthWin11Notify;
extern int heightWin11Notify;
//通知ウィンドウ内のアイコン左上位置とサイズ
extern POINT posNotifyIcon;
extern int widthNotifyIcon;
extern int heightNotifyIcon;
extern int posXShowDesktopArea;
extern POINT posNotifyText;

//メイン時計(自作)処理用変数
extern int widthWin11Clock;
extern int heightWin11Clock;
//部品サイズ(元のWin11通知領域の解析用)
extern int widthWin11Icon;
extern int widthWin11Button;
extern int origWidthWin11Tray;
extern int origHeightWin11Tray;
extern int defaultWin11ClockWidth;
extern int defaultWin11NotificationWidth;
extern int adjustWin11TrayYpos;

//調整用
extern int adjustWin11TrayCutPosition;
extern int adjustWin11DetectNotify;
extern int adjustWin11ClockWidth;
//配置・カット処理の結果得られる値
extern int cutOffWidthWin11Tray;
extern int modifiedWidthWin11Tray;
extern int posXMainClock;

//『小さいタスクバー』かどうか
extern int typeWin11Taskbar;
//『小さいタスクバー』でトレイ位置調整するかどうか
extern BOOL bAdjustTrayWin11SmallTaskbar;

//通知領域用オブジェクト
extern HBRUSH hBrushWin11Notify;
extern HPEN hPenWin11Notify;


//Type2

extern WNDPROC oldProcTaskbarContentBridge_Win11;

BOOL fillbackWin11NotifyIconInvert = FALSE;

int  modifiedWidthTaskbar_Win11Type2 = 0;

BOOL b_ShowingTClockBarWin11 = FALSE;


//extern BOOL bTokenMoveContentBridge;

//Focus Assist状態
//https://stackoverflow.com/questions/53407374/is-there-a-way-to-detect-changes-in-focus-assist-formerly-quiet-hours-in-windo
//	not_supported = -2,
//	failed = -1,
//	off = 0,
//	priority_only = 1,
//	alarms_only = 2
extern int intWin11FocusAssist;
extern int intWin11FocusAssistPrev;

extern int intWin11NotificationNumber;
extern int intWin11NotificationNumberPrev;


extern HFONT hFontNotify;
extern BOOL bShowWin11NotifyNumber;


//BOOL bEnableContentBridgeResize = TRUE;


void CreateTClockBarWin11Type2(void)
{
	//最初に残骸が残ってたら消す。
	HWND tempHwnd;
	if(tempHwnd = FindWindow("TClockBarWin11", NULL) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);


	//Win11Type2用のTClockBarのウィンドウを自作する。
	WNDCLASS classTClockBarWin11;
	TCHAR szClassName[] = TEXT("TClockBarWin11");

	// register a window class
	classTClockBarWin11.style = CS_HREDRAW | CS_VREDRAW;		// | CS_OWNDC;
	classTClockBarWin11.lpfnWndProc = DefWindowProc;
	classTClockBarWin11.cbClsExtra = 0;
	classTClockBarWin11.cbWndExtra = 0;
	classTClockBarWin11.hInstance = hmod;
	classTClockBarWin11.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	classTClockBarWin11.hCursor = LoadCursor(NULL, IDC_ARROW);
	classTClockBarWin11.hbrBackground = NULL;
	classTClockBarWin11.lpszMenuName = NULL;
	classTClockBarWin11.lpszClassName = szClassName;

	RegisterClass(&classTClockBarWin11);

	hwndTClockBarWin11 = CreateWindowEx(WS_EX_TOOLWINDOW, szClassName, "TClockBarWin11",
		WS_POPUP | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hmod, NULL);

	if (hwndTClockBarWin11) {
		//SubclassWindow(hwndTClockBarWin11, WndProcTClockBar_Win11);
		ShowWindow(hwndTClockBarWin11, SW_SHOW);
	}
}




void CreateWin11MainClock(void)
{

	//Win11Type2の場合は全体を収容するTClockBarWin11を作る
	if (Win11Type == 2) {
		CreateTClockBarWin11Type2();
	}


	//最初に残骸が残ってたら消す。
	HWND tempHwnd;
	if (Win11Type < 2){
		while ((tempHwnd = FindWindowEx(hwndTaskBarMain, NULL, "TClockMain", NULL)) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);
	}


	//Win11用のTClockのウィンドウを自作する。
	WNDCLASS classTClockWin11;
	TCHAR szClassName[] = TEXT("TClockMain");

	// register a window class
	classTClockWin11.style = CS_HREDRAW | CS_VREDRAW;		// | CS_OWNDC;
	classTClockWin11.lpfnWndProc = DefWindowProc;
	classTClockWin11.cbClsExtra = 0;
	classTClockWin11.cbWndExtra = 0;
	classTClockWin11.hInstance = hmod;
	classTClockWin11.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	classTClockWin11.hCursor = LoadCursor(NULL, IDC_ARROW);
	classTClockWin11.hbrBackground = NULL;
	classTClockWin11.lpszMenuName = NULL;
	classTClockWin11.lpszClassName = szClassName;

	RegisterClass(&classTClockWin11);

	if (Win11Type == 2) {
		hwndClockMain = CreateWindowEx(WS_EX_TOOLWINDOW, szClassName, "TClockMain",
			WS_VISIBLE | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			hwndTClockBarWin11, NULL, hmod, NULL);
		//hwndClockMain = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, szClassName, "TClockMain",
		//	WS_POPUP | WS_VISIBLE , CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		//	NULL, NULL, hmod, NULL);
	}
	else {
		hwndClockMain = CreateWindowEx(WS_EX_TOPMOST, szClassName, "TClockMain",
			WS_VISIBLE | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			hwndTaskBarMain, NULL, hmod, NULL);
	}


	if (hwndClockMain) {
		ShowWindow(hwndClockMain, SW_SHOW);
		CreateWin11Notify();		//使わない場合も作っておく。
	}
}

HWND CreateWin11SubClock(HWND tempHwndTaskbar)
{


	//最初に残骸が残ってたら消す。
	HWND tempHwnd;
	if ((tempHwnd = FindWindowEx(tempHwndTaskbar, NULL, "TClockSub", NULL)) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);

	WNDCLASS classTClockWin11Sub;
	TCHAR szClassName[] = TEXT("TClockSub");

	//Win11用のTClockSubのクラスを自作する。
	if (!bWin11Sub)
	{
		// register a window class
		classTClockWin11Sub.style = CS_HREDRAW | CS_VREDRAW;		// | CS_OWNDC;
		classTClockWin11Sub.lpfnWndProc = DefWindowProc;
		classTClockWin11Sub.cbClsExtra = 0;
		classTClockWin11Sub.cbWndExtra = 0;
		classTClockWin11Sub.hInstance = hmod;
		classTClockWin11Sub.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
		classTClockWin11Sub.hCursor = LoadCursor(NULL, IDC_ARROW);
		classTClockWin11Sub.hbrBackground = NULL;
		classTClockWin11Sub.lpszMenuName = NULL;
		classTClockWin11Sub.lpszClassName = szClassName;

		RegisterClass(&classTClockWin11Sub);

		bWin11Sub = TRUE;
	}

	tempHwnd = NULL;

	tempHwnd = CreateWindowEx(WS_EX_TOPMOST, "TClockSub", "TClockSub",
		WS_VISIBLE | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		tempHwndTaskbar, NULL, hmod, NULL);

	if (tempHwnd) {
		ShowWindow(tempHwnd, SW_SHOW);
	}

	return tempHwnd;
}

void CreateWin11Notify(void)
{

	//最初に残骸が残ってたら消す。
	HWND tempHwnd;
	if (Win11Type < 2) {
		while ((tempHwnd = FindWindowEx(hwndTaskBarMain, NULL, "TClockNotify", NULL)) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);
	}

	//Win11用のTClockのウィンドウを自作する。
	WNDCLASS classTClockWin11Notify;
	TCHAR szClassName[] = TEXT("TClockNotify");

	// register a window class
	classTClockWin11Notify.style = CS_HREDRAW | CS_VREDRAW;		// | CS_OWNDC;
	classTClockWin11Notify.lpfnWndProc = DefWindowProc;
	classTClockWin11Notify.cbClsExtra = 0;
	classTClockWin11Notify.cbWndExtra = 0;
	classTClockWin11Notify.hInstance = hmod;
	classTClockWin11Notify.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	classTClockWin11Notify.hCursor = LoadCursor(NULL, IDC_ARROW);
	classTClockWin11Notify.hbrBackground = NULL;
	classTClockWin11Notify.lpszMenuName = NULL;
	classTClockWin11Notify.lpszClassName = szClassName;

	RegisterClass(&classTClockWin11Notify);

	if (Win11Type == 2) {
		hwndWin11Notify = CreateWindowEx(WS_EX_TOOLWINDOW, "TClockNotify", "TClockNotify",
			WS_VISIBLE | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			hwndTClockBarWin11, NULL, hmod, NULL);
		//hwndWin11Notify = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, "TClockNotify", "TClockNotify",
		//	WS_POPUP | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		//	NULL, NULL, hmod, NULL);
	}
	else {
		hwndWin11Notify = CreateWindowEx(WS_EX_TOPMOST, "TClockNotify", "TClockNotify",
			WS_VISIBLE | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			hwndTaskBarMain, NULL, hmod, NULL);
	}

	if (hwndWin11Notify) {
		ShowWindow(hwndWin11Notify, SW_SHOW);
	}
}

void ReCreateWin11Notify(void)
{
	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] ReCreateWin11Notify called.", 999);

	//最初に残骸が残ってたら消す。
	HWND tempHwnd;
	while ((tempHwnd = FindWindowEx(hwndTaskBarMain, NULL, "TClockNotify", NULL)) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);

	hwndWin11Notify = NULL;

	hwndWin11Notify = CreateWindowEx(WS_EX_TOPMOST, "TClockNotify", "TClockNotify",
		WS_VISIBLE | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hwndTaskBarMain, NULL, hmod, NULL);

	if (hwndWin11Notify) {
		ShowWindow(hwndWin11Notify, SW_SHOW);
		SubclassWindow(hwndWin11Notify, WndProcWin11Notify);
		SetMainClockOnTasktray_Win11();
	}
}



void CreateMyNotificationFont(int fontheight, int fontwidth)
{
	//char fontname[80];
	//GetMyRegStr("Color_Font", "Font", fontname, 80, "");
//	hFontNotify = CreateMyFont(fontname, 9, FW_BOLD, FALSE);

	LOGFONT lf;

	lf.lfHeight = fontheight;
	lf.lfWidth = 0;
	lf.lfEscapement = lf.lfOrientation = 0;
	lf.lfWeight = FW_BOLD;
	lf.lfItalic = 0;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	lf.lfCharSet = ANSI_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	strcpy(lf.lfFaceName, "Segoe UI");

	hFontNotify = CreateFontIndirect(&lf);
}



void LoadBitMapWin11Notify(void)
{
	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] LoadBitMapWin11Notify called.", 999);


	//ビットマップの条件
	//黒->背景
	//白(諧調あり)->アイコン色
	//サイズ:120 x 160が基本だが、違ってもOK

//	HBITMAP hbmpIconYesWin11Notify = NULL;
	//HBITMAP hbmpIconNoWin11Notify = NULL;
	//HBITMAP hbmpIconFocusWin11Notify = NULL;

	HDC hdc, tempDC;
	BYTE tempR, tempG, tempB, tempAlpha;
	unsigned tempUnsigned;
	RGBQUAD* color;
	BITMAP tempBitmap;

	if (hdcYesWin11Notify) {
		DeleteDC(hdcYesWin11Notify);
		DeleteObject(hbm_DIBSection_YesWin11Notify);
	}

	if (hdcNoWin11Notify) {
		DeleteDC(hdcNoWin11Notify);
		DeleteObject(hbm_DIBSection_NoWin11Notify);
	}

	if (hdcFocusWin11Notify) {
		DeleteDC(hdcFocusWin11Notify);
		DeleteObject(hbm_DIBSection_FocusWin11Notify);
	}

	if (hdcWin11Notify) {
		DeleteDC(hdcWin11Notify);
		DeleteObject(hbm_DIBSection_Win11Notify);
	}

	hbmpIconNoWin11Notify = LoadBitmap(hmod, IDB_BITMAP1);		//ブランク
	hbmpIconFocusWin11Notify = LoadBitmap(hmod, IDB_BITMAP3);	//三日月マーク

	if (bShowWin11NotifyNumber) {
		hbmpIconYesWin11Notify = LoadBitmap(hmod, IDB_BITMAP4);		//塗りつぶし(文字入れ用)
	}
	else {
		hbmpIconYesWin11Notify = LoadBitmap(hmod, IDB_BITMAP2);		//!マーク
	}


	tempR = GetRValue(colWin11Notify);
	tempG = GetGValue(colWin11Notify);
	tempB = GetBValue(colWin11Notify);


	if ((fillbackcolor || (Win11Type == 2)) && (tempR + tempG + tempB < 255))
	{
		fillbackWin11NotifyIconInvert = TRUE;
	}
	else {
		fillbackWin11NotifyIconInvert = FALSE;
	}

	if (hwndWin11Notify) {
		if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][LoadBitMapWin11Notify] No hwndWin11Notify, canceled.", 999);
	}

	hdc = GetDC(hwndWin11Notify);

//	CreateMyNotificationFont(heightWin11Notify / 2, widthNotifyIcon / 5);
	

	//Notificationクリア・描画用オブジェクト指定
	if (!hBrushWin11Notify)DeleteObject(hBrushWin11Notify);
	hBrushWin11Notify = CreateSolidBrush(RGB(0, 0, 0));
	SelectObject(hdc, hBrushWin11Notify);

	if (!hPenWin11Notify)DeleteObject(hPenWin11Notify);
	hPenWin11Notify = CreatePen(PS_SOLID, 1, colWin11Notify);
	SelectObject(hdc, hPenWin11Notify);


	//DIB section作成用bitmap構造体
	static BITMAPINFO bmi = { { sizeof(BITMAPINFO),0,0,1,32,BI_RGB }, };

	
	bmi.bmiHeader.biWidth = widthWin11Notify;
	bmi.bmiHeader.biHeight = heightWin11Notify;

	hbm_DIBSection_Win11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_Win11Notify_start, NULL, 0);
	m_color_Win11Notify_end = m_color_Win11Notify_start + (widthWin11Notify * heightWin11Notify);
	hdcWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcWin11Notify, hbm_DIBSection_Win11Notify);
	SetStretchBltMode(hdcWin11Notify, HALFTONE);

	bmi.bmiHeader.biWidth = widthNotifyIcon;
	bmi.bmiHeader.biHeight = heightNotifyIcon;


	//通知ありアイコンDC作成
	//DIB section 作成, 最初と最後のアドレス取得
	hbm_DIBSection_YesWin11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_YesWin11Notify_start, NULL, 0);
	m_color_YesWin11Notify_end = m_color_YesWin11Notify_start + (widthNotifyIcon * heightNotifyIcon);

	//リソースビットマップ(白/透明)を枠にあわせてコピー。枠はGetWin11ElementSizeで決める)
	hdcYesWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcYesWin11Notify, hbm_DIBSection_YesWin11Notify);
	SetStretchBltMode(hdcYesWin11Notify, HALFTONE);

	tempDC = CreateCompatibleDC(hdc);
	SelectObject(tempDC, hbmpIconYesWin11Notify);

	GetObject(hbmpIconYesWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcYesWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);

	//ロードしたビットマップはもういらないので破棄->最後まで利用に変更
//	DeleteObject(hbmpIconYesWin11Notify);

	for (color = m_color_YesWin11Notify_start; color < m_color_YesWin11Notify_end; ++color)
	{



		//色が存在するポイントはAlphaを輝度値にする(元を白/黒ビットマップにしておけばどの色からでも取れる)。こうしないと黒が表示されない
		color->rgbReserved = color->rgbRed;

		tempUnsigned = color->rgbRed * tempR / 255;
		color->rgbRed = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbGreen * tempG / 255;
		color->rgbGreen = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbBlue * tempB / 255;
		color->rgbBlue = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		if (fillbackWin11NotifyIconInvert)	//fillbackcolorで色が暗いときのみ、背景を白に反転する。
		{
			if (color->rgbReserved == 0)
			{
				color->rgbRed = 255;
				color->rgbGreen = 255;
				color->rgbBlue = 255;
			}
		}

	}

	//通知なしアイコンDC作成
	//DIB section 作成, 最初と最後のアドレス取得
	hbm_DIBSection_NoWin11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_NoWin11Notify_start, NULL, 0);
	m_color_NoWin11Notify_end = m_color_NoWin11Notify_start + (widthNotifyIcon * heightNotifyIcon);

	//リソースビットマップ(白/透明)を枠にあわせてコピー。枠はGetWin11ElementSizeで決める)
	hdcNoWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcNoWin11Notify, hbm_DIBSection_NoWin11Notify);
	SetStretchBltMode(hdcNoWin11Notify, HALFTONE);

	DeleteDC(tempDC);
	tempDC = CreateCompatibleDC(hdc);
	SelectObject(tempDC, hbmpIconNoWin11Notify);

	GetObject(hbmpIconNoWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcNoWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);

	//ロードしたビットマップはもういらないので破棄->最後まで利用に変更
//	DeleteObject(hbmpIconNoWin11Notify);

	for (color = m_color_NoWin11Notify_start; color < m_color_NoWin11Notify_end; ++color)
	{
		//色が存在するポイントはAlphaを輝度値にする(元を白/黒ビットマップにしておけばどの色からでも取れる)。こうしないと黒が表示されない
		color->rgbReserved = color->rgbRed;

		tempUnsigned = color->rgbRed * tempR / 255;
		color->rgbRed = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbGreen * tempG / 255;
		color->rgbGreen = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbBlue * tempB / 255;
		color->rgbBlue = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		if (fillbackWin11NotifyIconInvert)	//fillbackcolorで色が暗いときのみ、背景を白に反転する。
		{
			if (color->rgbReserved == 0)
			{
				color->rgbRed = 255;
				color->rgbGreen = 255;
				color->rgbBlue = 255;
			}
		}
	}

	//集中モードアイコンDC作成
	//DIB section 作成, 最初と最後のアドレス取得
	hbm_DIBSection_FocusWin11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_FocusWin11Notify_start, NULL, 0);
	m_color_FocusWin11Notify_end = m_color_FocusWin11Notify_start + (widthNotifyIcon * heightNotifyIcon);

	//リソースビットマップ(白/透明)を枠にあわせてコピー。枠はGetWin11ElementSizeで決める)
	hdcFocusWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcFocusWin11Notify, hbm_DIBSection_FocusWin11Notify);
	SetStretchBltMode(hdcFocusWin11Notify, HALFTONE);

	tempDC = CreateCompatibleDC(hdc);
	SelectObject(tempDC, hbmpIconFocusWin11Notify);

	GetObject(hbmpIconFocusWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcFocusWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);

	//ロードしたビットマップはもういらないので破棄->最後まで利用に変更
//	DeleteObject(hbmpIconFocusWin11Notify);

	for (color = m_color_FocusWin11Notify_start; color < m_color_FocusWin11Notify_end; ++color)
	{
		//色が存在するポイントはAlphaを輝度値にする(元を白/黒ビットマップにしておけばどの色からでも取れる)。こうしないと黒が表示されない
		color->rgbReserved = color->rgbRed;

		tempUnsigned = color->rgbRed * tempR / 255;
		color->rgbRed = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbGreen * tempG / 255;
		color->rgbGreen = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbBlue * tempB / 255;
		color->rgbBlue = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		if (fillbackWin11NotifyIconInvert)	//fillbackcolorで色が暗いときのみ、背景を白に反転する。
		{
			if (color->rgbReserved == 0)
			{
				color->rgbRed = 255;
				color->rgbGreen = 255;
				color->rgbBlue = 255;
			}
		}

	}


	//一時DCの削除と通知ウィンドウDCのリリース
	DeleteDC(tempDC);
	ReleaseDC(hwndWin11Notify, hdc);

}


void GetWin11ElementSize(void)
{
	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] GetWin11ElementSize called.", 999);
	//RECT tempRect;

	GetTaskbarSize();
	GetWin11TaskbarType(); //レジストリで小さいタスクバー設定をしているかどうか確認

						   //元はアプリアイコン展開用ボタン幅を基準にしていたので、widthWin11Buttonが基準になっている。
						   //展開ボタンがない場合があることが判明したので、同じ値のheightTaskbar / 2に移行した。

						   //Win11(2021/11月時点)のタスクトレイアイコンサイズ(標準タスクバー)
						   //※拡大率100%の場合の値。倍率にあわせて値が変わる(例:200%なら倍の値)
						   //※WinExplorerで調べた場合には、実際の倍率によらず下記の100%換算の値が得られる。
						   //タスクバー高さ: 48
						   //アプリアイコン展開ボタン(アプリアイコン1個と同じ)幅	: 24
						   //IME、タッチキーボード、位置情報等のOSアイコン: 32
						   //時計幅: 88 = 24 * 11 / 3
						   //時計に通知が出た場合の広がり幅: 28
						   //※アイコン幅32と通知幅28の差で、通知判定を行う必要がある、ということ。
						   //WiFi等アイコンが3つある場合: 80 (アイコン1個16 x 3 + 隙間や端:8 x 4)
						   //WiFi等アイコンが3つある場合: 56 (アイコン1個16 x 2 + 隙間や端:8 x 3)

						   //Win11(2021/11月時点)のタスクトレイアイコンサイズ(レジストリ操作による『小さいタスクバー』)
						   //※拡大率100%の場合の値。倍率にあわせて値が変わる(例:200%なら倍の値)
						   //タスクバー高さ: 32
						   //アプリアイコン展開ボタン(アプリアイコン1個と同じ)幅	: 24
						   //IME、タッチキーボード、位置情報等のOSアイコン: 32
						   //時計幅: 88
						   //時計に通知が出た場合の広がり幅: 28
						   //WiFi等アイコンが3つある場合:  80 (アイコン1個16 x 3 + 隙間や端:8 x 4)
						   //WiFi等アイコンが3つある場合:  56 (アイコン1個16 x 2 + 隙間や端:8 x 3)

						   //->[結論] 小さいタスクバーの場合はタスクバー高さが変わるだけ、

	adjustWin11TrayYpos = 0;

	if ((typeWin11Taskbar == 0) && (Win11Type < 2))	//レジストリのTaskbarSiの設定はビルド22579未満(Win11Type == 1)でないと無効
	{
		widthWin11Button = heightTaskbar * 3 / 4;
		if (bAdjustTrayWin11SmallTaskbar) adjustWin11TrayYpos = heightTaskbar / 4;
	}
	else
	{
		widthWin11Button = heightTaskbar / 2;		//標準の高さのタスクバーの場合
	}

	widthWin11Icon = widthWin11Button * 4 / 3;

	SetMyRegLong("Status_DoNotEdit", "Win11IconSize", widthWin11Icon);	//設定の際の上限値決めるためにファイルに保存する。

//	defaultWin11ClockWidth = widthWin11Button * 11 / 3 - (widthWin11Button / 15) + adjustWin11ClockWidth;		//(widthWin11Button / 15)は計算上の誤差の吸収用。ここで手動調整値を適用する。

	//Ver4.2.1以降
	defaultWin11ClockWidth = widthWin11Button * 11 / 3 + adjustWin11TrayCutPosition;
	defaultWin11NotificationWidth = widthWin11Button * 7 / 6;

	if (b_DebugLog) {
		writeDebugLog_Win10("[for_win11.c][GetWin11ElementSize] (From [Taskbar Height]) widthWin11Button = ", widthWin11Button);
		writeDebugLog_Win10("[for_win11.c][GetWin11ElementSize] (From [Taskbar Height]) widthWin11Icon = ", widthWin11Icon);
		writeDebugLog_Win10("[for_win11.c][GetWin11ElementSize] (From [Taskbar Height]) defaultWin11ClockWidth = ", defaultWin11ClockWidth);
	}


	if (bEnabledWin11Notify)
	{
//		widthWin11Notify = widthWin11Button * 12 / 10;		//これは自分で決めてよい値
		widthWin11Notify = widthWin11Icon;		//これは自分で決めてよい値
		heightWin11Notify = heightTaskbar;

		//posNotifyIcon.x = widthWin11Notify * 1 / 10;
		//widthNotifyIcon = widthWin11Notify * 8 / 10;

		posNotifyIcon.x = widthWin11Button * 0 / 3;		//0
		widthNotifyIcon = widthWin11Button * 3 / 3;		//24
		posXShowDesktopArea = widthWin11Button * 9 / 8;

		if (typeWin11Taskbar == 0)
		{
			posNotifyIcon.y = heightWin11Notify * 0 / 10;
			heightNotifyIcon = heightWin11Notify * 10 / 10;
		}
		else
		{
			posNotifyIcon.y = heightWin11Notify * 1 / 10;
			heightNotifyIcon = heightWin11Notify * 8 / 10;
		}

		LoadBitMapWin11Notify();		//通知アイコンサイズが決まらないとロードできない。


		//通知数表示用のフォント作成・位置決定
		posNotifyText.x = widthNotifyIcon / 2;
		if (typeWin11Taskbar == 0)	//小さいタスクバーの場合
		{
			posNotifyText.y = heightWin11Notify * 3 / 4;
			CreateMyNotificationFont(heightWin11Notify / 2, widthNotifyIcon / 5);
		}
		else	//小さいタスクバーでない場合
		{
			posNotifyText.y = heightWin11Notify * 7 / 10;
			CreateMyNotificationFont(heightWin11Notify * 2 / 5, widthNotifyIcon / 5);
		}


	}
	else {		//使わない場合は単にサイズをゼロにしてウィンドウは残しておく。
		widthWin11Notify = 0;
		heightWin11Notify = 0;
	}

}


void GetWin11TrayWidth(void)
{
	RECT tempRect;
	GetWindowRect(hwndTrayMain, &tempRect);
	origWidthWin11Tray = tempRect.right - tempRect.left;

	HWND tempHwnd;
	POINT pos = { 0,0 };
	tempHwnd = FindWindowEx(hwndTrayMain, NULL, "TrayInputIndicatorWClass", NULL);
	MapWindowPoints(tempHwnd, hwndTrayMain, &pos, 1);

	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][GetWin11TrayWidth] origWidthWin11Tray =", origWidthWin11Tray);
}


void SwitchToTClockBarWin11(void)
{


	if (Win11Type < 2) return;

	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] SwitchToTClockBarWin11 called.", 999);


	HDC desktopDC = NULL;
	HDC tclockBarDC = NULL;

	DesktopDirectDraw_Win11();

	if (!b_ShowingTClockBarWin11)
	{
		b_ShowingTClockBarWin11 = TRUE;
		if (desktopDC = GetDC(hwndDesktop)) {
			if (tclockBarDC = GetDC(hwndTClockBarWin11)) {

				BitBlt(tclockBarDC, 0, 0, posXMainClock, heightMainClockFrame, desktopDC, 0, posYTaskbar, SRCCOPY);
				ReleaseDC(hwndTClockBarWin11, tclockBarDC);
			}

			ReleaseDC(hwndDesktop, desktopDC);
		}
	}

	ShowWindow(hwndWin11ContentBridge, SW_HIDE);

	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] SwitchToTClockBarWin11 finished.", 999);

}

void ReturnToOriginalTaskBar(void)
{
	b_ShowingTClockBarWin11 = FALSE;

	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] ReturnToOriginalTaskBar called.", 999);

	ShowWindow(hwndWin11ContentBridge, SW_SHOW);

}



void DesktopDirectDraw_Win11(void)
{
	HWND tempDeskotopHwnd = NULL;
	HDC desktopDC = NULL;
	HDC tclockDC = NULL;
	HDC notifyDC = NULL;
	extern HDC hdcClock;

	if (b_DebugLog) {
		writeDebugLog_Win10("[for_win11.c] DesktopDirectDraw_Win11 called.", 999);
	}


	tempDeskotopHwnd = GetDesktopWindow();	//GetDesktopWindow()がデスクトップ
	
	if (desktopDC = GetDC(tempDeskotopHwnd)) {
		if (tclockDC = GetDC(hwndClockMain)) {
			
			BitBlt(desktopDC, posXMainClock, originalPosYTaskbar, widthMainClockFrame, heightMainClockFrame, tclockDC, 0, 0, SRCCOPY);
			ReleaseDC(hwndClockMain, tclockDC);
		}

		ReleaseDC(tempDeskotopHwnd, desktopDC);
	}
}


LRESULT CALLBACK WndProcTClockBar_Win11(HWND tempHwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (b_DebugLog) {
		writeDebugLog_Win10("[for_win11.c][WndProcTClockBar_Win11] Window Message was recevied, message =", message);
	}

	switch (message)
	{
	case WM_MOUSEMOVE:
	case WM_MOUSEHOVER:
	case WM_MOUSELEAVE:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		return SendMessage(hwndTaskBarMain, message, wParam, lParam);
	}

	return DefWindowProc(tempHwnd, message, wParam, lParam);
}



LRESULT CALLBACK WndProcTaskbarContentBridge_Win11(HWND tempHwnd, UINT message, WPARAM wParam, LPARAM lParam)
{



//	DesktopDirectDraw_Win11();		//とりあえずTClockを強制的にデスクトップに表示する。

/*
0を返し続けると15(WM_PAINT)が延々続いてループする。すくなくとも15はoldProcTaskbarContentBridge_Win11に渡さなければいけない。

メッセージ(標準戻り値例)

ウインドウサイズ情報を含むもの
70		WM_WINDOWPOSCHANGING
131		WM_NCCALCSIZE
71		WM_WINDOWPOSCHANGED
5		WM_SIZE


ウィンドウサイズ情報を含まないもの
133	 	WM_NCPAINT
15		WM_PAINT (通すか, BeginPaint&EndPaintしないと呼ばれ続ける)
20 		WM_ERASEBKGND
26		WM_WININICHANGE
24		WM_SHOWWINDOW	システムによりタスクバーサイズが戻されたときに1回届く。SetMainClockOnTaskTray_Win11によるタスクバー短縮では出ないためループしない。
528		WM_PARENTNOTIFY

ウィンドウサイズ情報を含むか不明なもの
738		独自メッセージ？画面スケーリングを変更すると届くが普段は届かない
739		独自メッセージ？画面スケーリングを変更すると届くが普段は届かない

*/

	//いろいろブロックを試みたが…
	//71を止めるとうまくサイズ調整ができなくなる。
	//15を止めると無限ループでNG
	//それ以外は止めても、現状のアルゴリズムでは特に効果ない。
	//デスクトップ直接描画でちらつき改善がみられるが完全ではない。

	//サイズ調節の最後は、
	//WM_WINDOWPOSCHANGED(71)が届く
	//oldProcTaskbarContentBridge_Win11の動作からWM_SIZE(5)が発生する
	//WM_SIZEに対するoldProcTaskbarContentBridge_Win11の動作が完了したら戻ってきてWM_WINDOWPOSCHANGEDが終了する。
	//この時点でサイズ調整が完了しているので、TClock用の移動を行う。ただし直接呼んでは早すぎるので、
	//PostMessage(hwndClockMain, CLOCKM_MOVEWIN11CONTENTBRIDGE, 0, 0);で呼ぶ

	//if (b_DebugLog){
	//	writeDebugLog_Win10("[for_win11.c][WndProcTaskbarContentBridge_Win11] Window Message was recevied, message =", message);
	//	writeDebugLog_Win10("[for_win11.c][WndProcTaskbarContentBridge_Win11] wParam =", (int)wParam);
	//	writeDebugLog_Win10("[for_win11.c][WndProcTaskbarContentBridge_Win11] lParam =", (int)lParam);
	//	CheckPixel_Win10(2081, posYTaskbar + 10);
	//	writeDebugLog_Win10("[for_win11.c][WndProcTaskbarContentBridge_Win11] b_ShowingTClockBarWin11 =", b_ShowingTClockBarWin11);

	//	if (message == 528)	//WM_PARENTNOTIFY
	//	{
	//		writeDebugLog_Win10("[for_win11.c] WM_PARENTNOTIFY(528)", 999);
	//		writeDebugLog_Win10("[for_win11.c] posX =", GET_X_LPARAM(lParam));
	//		writeDebugLog_Win10("[for_win11.c] posY =", GET_Y_LPARAM(lParam));
	//	}

	//		if (message == 131)	//WM_NCCALCSIZE
	//		{
	//			writeDebugLog_Win10("[for_win11.c] WM_NCCALCSIZE(131)", 999);
	//			NCCALCSIZE_PARAMS* pncsp = (NCCALCSIZE_PARAMS*)lParam;
	//			int newWidth = (int)(*pncsp).rgrc[0].right - (int)(*pncsp).rgrc[0].left;
	//			int newHeight = (int)(*pncsp).rgrc[0].bottom - (int)(*pncsp).rgrc[0].top;
	//			writeDebugLog_Win10("[for_win11.c] newWidth =", newWidth);
	//			writeDebugLog_Win10("[for_win11.c] newHeight =", newHeight);
	//		}

	//		if (message == 5)	//WM_SIZE
	//		{
	//			writeDebugLog_Win10("[for_win11.c] WM_SIZE(5)", 999);
	//			writeDebugLog_Win10("[for_win11.c] Width =", (int)(lParam & 0xFFFF));
	//			writeDebugLog_Win10("[for_win11.c] Height =", (int)((lParam >> 16) & 0xFFFF));
	//		}

	//		if ((message == 70))	//WM_WINDOWPOSCHANGING
	//		{
	//			writeDebugLog_Win10("[for_win11.c] WM_WINDOWPOSCHANGING(70)", 999);
	//			WINDOWPOS* pwinpos = (WINDOWPOS*)lParam;
	//			writeDebugLog_Win10("[for_win11.c] x =", (int)(*pwinpos).x);
	//			writeDebugLog_Win10("[for_win11.c] y =", (int)(*pwinpos).y);
	//			writeDebugLog_Win10("[for_win11.c] cx =", (int)(*pwinpos).cx);
	//			writeDebugLog_Win10("[for_win11.c] cy =", (int)(*pwinpos).cy);
	//		}

	//		if ((message == 71))	//WM_WINDOWPOSCHANGED, これをブロックするとうまく動かない
	//		{
	//			writeDebugLog_Win10("[for_win11.c] WM_WINDOWPOSCHANGED(71)", 999);
	//			WINDOWPOS* pwinpos = (WINDOWPOS*)lParam;
	//			writeDebugLog_Win10("[for_win11.c] x =", (int)(*pwinpos).x);
	//			writeDebugLog_Win10("[for_win11.c] y =", (int)(*pwinpos).y);
	//			writeDebugLog_Win10("[for_win11.c] cx =", (int)(*pwinpos).cx);
	//			writeDebugLog_Win10("[for_win11.c] cy =", (int)(*pwinpos).cy);

	//		}

	//		if (message == WM_NCPAINT)
	//		{
	//			HDC tempHDC;
	//			tempHDC = GetDCEx(tempHwnd, (HRGN)wParam, DCX_WINDOW | DCX_INTERSECTRGN);
	//			writeDebugLog_Win10("[for_win11.c] WM_NCPAINT(133), Target HDC =", (int)tempHDC);
	//			ReleaseDC(tempHwnd, tempHDC);
	//		}

	//		if (message == WM_PAINT)
	//		{
	//			writeDebugLog_Win10("[for_win11.c] WM_PAINT(15).", 999);
	//		}

	//		if (message == WM_ERASEBKGND)
	//		{
	//			writeDebugLog_Win10("[for_win11.c] WM_ERASEBKGND(20), Target HDC =", (int)wParam);
	//		}
	//		
	//		extern HDC HDC_Stored_Desktop;
	//		extern HDC HDC_Stored_TaskbarMain;
	//		extern HDC HDC_Stored_ContentBridge_Win11;
	//		extern HWND hwndDesktop;

	//		writeDebugLog_Win10("[for_win11.c] hwndDesktop =", (int)hwndDesktop);
	//		writeDebugLog_Win10("[for_win11.c] HDC_Stored_Desktop =", (int)HDC_Stored_Desktop);
	//		writeDebugLog_Win10("[for_win11.c] hwndTaskBarMain =", (int)hwndTaskBarMain);
	//		writeDebugLog_Win10("[for_win11.c] HDC_Stored_TaskbarMain =", (int)HDC_Stored_TaskbarMain);
	//		writeDebugLog_Win10("[for_win11.c] hwndWin11ContentBridge =", (int)hwndWin11ContentBridge);
	//		writeDebugLog_Win10("[for_win11.c] HDC_Stored_ContentBridge_Win11 =", (int)HDC_Stored_ContentBridge_Win11);
	//}




	LRESULT ret = 0;

	//if (message == 15) {		//WM_PAINTはBeginPaint/EndPaintしないと呼ばれ続けるのでこれでスキップする。
	//	//https://docs.microsoft.com/ja-jp/windows/win32/gdi/wm-paint
	//	PAINTSTRUCT ps;
	//	HDC hdc = BeginPaint(tempHwnd, &ps);
	//	if (b_DebugLog) {
	//		writeDebugLog_Win10("[for_win11.c] WM_PAINT(15).", 999);
	//		writeDebugLog_Win10("[for_win11.c] Target HDC =", (int)hdc);
	//		writeDebugLog_Win10("[for_win11.c] rcPaint.left =", ps.rcPaint.left);
	//		writeDebugLog_Win10("[for_win11.c] rcPaint.top =", ps.rcPaint.top);
	//		writeDebugLog_Win10("[for_win11.c] rcPaint.right =", ps.rcPaint.right);
	//		writeDebugLog_Win10("[for_win11.c] rcPaint.bottom =", ps.rcPaint.bottom);
	//	}

	//		ReleaseDC(tempHwnd, hdc);	
	//}

	ret = CallWindowProc(oldProcTaskbarContentBridge_Win11, tempHwnd, message, wParam, lParam);


	if (b_DebugLog) {
		writeDebugLog_Win10("[for_win11.c][WndProcTaskbarContentBridge_Win11] Ret =", ret);
	}

	if (message == 71)		//WM_WINDOWPOSCHANGED
	{
		WINDOWPOS* pwinpos = (WINDOWPOS*)lParam;
		int tempWidth = (int)(*pwinpos).cx;
		if (b_DebugLog) {
			writeDebugLog_Win10("[for_win11.c] cx after oldProcTaskbarContentBridge_Win11 =", tempWidth);
			writeDebugLog_Win10("[for_win11.c] posXMainClock =", posXMainClock);
		}


		if (tempWidth == posXMainClock) 
		{
			if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][WndProcTaskbarContentBridge_Win11] Replacement of ContentBridege completed.", 999);
		}
		else if ( tempWidth == originalWidthTaskbar)	//配置が戻されたら一連のプロセスの最後に成立する。実際にはoriginalWidthTaskbarに戻るが、それ以外のケースにも対応できる
		{
			if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][WndProcTaskbarContentBridge_Win11] SetMainClockOnTaskTray_Win11(operation = 0) called due to ContentBridge resize, Size =", (int)(*pwinpos).cx);
			SwitchToTClockBarWin11();
			PostMessage(hwndClockMain, CLOCKM_MOVEWIN11CONTENTBRIDGE, 0, 0);	//Postmessage経由でSetMainClockOnTaskTray_Win11を実行する。
//			SetMainClockOnTasktray_Win11();			//こんな風に直接呼ぶと早すぎてちらつき->失敗、になる。
		}
		else 
		{
			if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][WndProcTaskbarContentBridge_Win11] SetMainClockOnTaskTray_Win11(operation = 1) called due to ContentBridge resize, Size =", (int)(*pwinpos).cx);
			PostMessage(hwndClockMain, CLOCKM_MOVEWIN11CONTENTBRIDGE, 1, 0);	//Postmessage経由でSetMainClockOnTaskTray_Win11を実行する。
		}

	}
	else if (message == 70)	//通知数が変わるとなぜかこれが届くようなので、通知数が変化していないかチェックする。
	{
		if (SetModifiedWidthWin11Tray())	//この関数は通知アイコンの有無が変化したらTRUEになる。
		{
			if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][WndProcTaskbarContentBridge_Win11] SetMainClockOnTaskTray_Win11 called due to Notification change.", 999);
			PostMessage(hwndClockMain, CLOCKM_MOVEWIN11CONTENTBRIDGE, 0, 0);	//Postmessage経由でSetMainClockOnTaskTray_Win11を実行する。
			//			SetMainClockOnTasktray_Win11(); こんな風に直接呼ぶとタイミングがずれてうまくいかない
		}
	}
	else if (message == 131)	//WM_NCCALCSIZE
	{
		NCCALCSIZE_PARAMS* pncsp = (NCCALCSIZE_PARAMS*)lParam;
		int newWidth = (int)(*pncsp).rgrc[0].right - (int)(*pncsp).rgrc[0].left;
		if (newWidth == originalWidthTaskbar) {
			SwitchToTClockBarWin11();
		}
	}
	else if (message == 528)	//WM_NCCALCSIZE
	{
		SwitchToTClockBarWin11();
	}

//	DesktopDirectDraw_Win11();		//念のためTClockを強制的にデスクトップに表示する。


	SetTimer(hwndClockMain, IDTIMERDLL_WIN11TYPE2_SHOW_TASKBAR, TIMEOUT_MS_TCLOCKBARWIN11, NULL);

	return ret;
}

// Added by TTTT for Win10AU (WIN10RS1) compatibility
// Imported from TClockLight-tclocklight-kt160911, dll/Wndproc.c
/*------------------------------------------------
subclass procedure of the tray
--------------------------------------------------*/
LRESULT CALLBACK SubclassTrayProc_Win11(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	//このコード内ではhwndがhwndTrayMainであり、他の多くの場合(hwnd=hwndClockMain)と異なるなので注意すること！
	//タスクトレイはメインのタスクバーにしか存在しないので、それ以外には使えないコールバック関数になっている。

//	if (b_DebugLog) writeDebugLog_Win10("[for_win11.c][SubclassTrayProc_Win11] Window Message was recevied, message = ", message);



	//Win11では、TClockはタスクトレイの外にある。
	//このプロシージャでは、タスクトレイの本来の長さを取得し、そのうち右の時計を切り落とすための操作を行っている。

	switch (message)
	{
		case (WM_USER + 100):	//1124
		{
			if (b_DebugLog)writeDebugLog_Win10("[fow_win11.c][SubclassTrayProc_Win11] WM_USER + 100 (1124) recevied.", 999);
			//Win10の場合
			// 再配置前に親ウィンドウから送られ、サイズを返すメッセージ。
			// DefSubClassTrayProc()を呼ぶとLRESULT形式で、Windows標準時計が入った場合のサイズが帰ってくるので、
			// 改造した場合のサイズに差し替えて戻す。
			// 正しい値を返さないとタスクトレイのサイズがおかしくなる。

			//Win11の場合
			//IMEアイコンより左のアイコンが変更される場合には WININICHANGEが一回先行する。
			//そのあと、IMEアイコンの変更の場合も、位置情報アイコン表示の場合も、
			//WM_USER+100 が1回 -> WM_NCCALC(131) が2回届く


			LRESULT ret;


			ret = DefSubclassProc(hwndTrayMain, message, wParam, lParam);

			//これで得られるのはButton + Pagerの幅であり、標準時計とネット等のアイコン類の幅は含まれてないようだ。
			//ソフトウェアキーボードアイコン分は、起動時値が入ったまま反映されない(なぜかは不明)。
			//なのであまり使えない。

			//Win10の場合と異なり、ここで改変した幅を返してはいけない。トレイ内再配置が起こってアイコンが重なってしまう。
			//時計を切り落とすのは、SetMainClockOnTasktray_Win11で行う。


			//再配置に先だって必ず呼ばれるわけではない。ここに処理を入れても確実に実行されない

			return ret;
		}
		case (WM_NCCALCSIZE):	//131
		{
			int i, newWidth;
			LRESULT ret;
			{
				NCCALCSIZE_PARAMS* pncsp = (NCCALCSIZE_PARAMS*)lParam;
				newWidth = (int)(*pncsp).rgrc[0].right - (int)(*pncsp).rgrc[0].left;
				//			newHeight = (int)(*pncsp).rgrc[0].bottom - (int)(*pncsp).rgrc[0].top;

				if (b_DebugLog) {
					writeDebugLog_Win10("[for_win11.c][SubclassTrayProc_Win11] WM_NCCALCSIZE received, wParam = ", wParam);
					writeDebugLog_Win10("[for_win11.c][SubclassTrayProc_Win11] newWidth = ", newWidth);
				}

				//再配置に先だって必ず呼ばれる模様。

				//このメッセージでは、『今からこの値に変更しようとする』サイズが取得できる。これがmodifiedWidthWin11Trayと異なる場合には強制的に戻されようとしていることになるが、
				//そのサイズは現状のトレイの構成の幅ということになるので、origWidthWin11Trayのアップデートに使える。


				if (newWidth != modifiedWidthWin11Tray) {	//modifiedWidthWin11Trayへの変更に伴って来た場合には無視する。
					origWidthWin11Tray = newWidth;
					SetModifiedWidthWin11Tray();
				}
				if (b_DebugLog)
				{
					writeDebugLog_Win10("[for_win11.c][SubclassTrayProc_Win11] origWidthWin11Tray =", origWidthWin11Tray);
				}

			}

			//Small Taskbarのアイコンちらつき抑制
//			ShowWindow(hwndWin11InnerTrayContentBridge, SW_HIDE);

			break;
		}
		//case WM_PAINT:
		//case WM_NCPAINT:
		//{
		//	//ContentBridgeはSetMainClock...で移動しても再配置されてしまう。
		//	if (bTokenMoveContentBridge) {
		//		bTokenMoveContentBridge = FALSE;
		//		MoveWin11ContentBridge();
		//	}
		//	break;
		//}
		case WM_NOTIFY:		//78
		{
			// 再配置が発生したら親ウィンドウから送られる。
			// DefSubClassTrayProc()を呼ぶとLRESULT形式で返答すべきコード帰ってくるので、そのまま戻せばOKのようだ。
			// この時点で時計は強制的に標準Windows時計のサイズに変更されて、タスクトレイ左上基準にアイテムが並んだ状態(トレイのサイズ自体はWM_USER+100に正しく返していれば確保されている)。
			// hwndClockMainのサイズを改造後サイズに修正して、通知領域などの場所を修正する必要あり(SetMainClockOnTasktrayを呼ぶ。

			LRESULT ret;
			NMHDR *nmh = (NMHDR*)lParam;

			//if (b_DebugLog)
			//{
			//	writeDebugLog_Win10("[for_win11.c][SubclassTrayProc_Win11] WM_NOTIFY(78) received, with code =", nmh->code);
			//}

			if (nmh->code != PGN_CALCSIZE) {
				break;
			}
			if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][SubclassTrayProc_Win11] SetMainClockOnTasktray_Win11 called by WM_NOTIFY(78) + PGN_CALCSIZE, origWidthWin11Tray = ", origWidthWin11Tray);
			SetMainClockOnTasktray_Win11();
			ret = DefSubclassProc(hwndTrayMain, message, wParam, lParam);	//hwndを明示したが、機能は同じ。
			return ret;
		}
	}

	return DefSubclassProc(hwndTrayMain, message, wParam, lParam);
}








/*------------------------------------------------
subclass procedure of the Win11 Tclock Notify Window (2021/11)
--------------------------------------------------*/
LRESULT CALLBACK WndProcWin11Notify(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND tempHwnd;
	tempHwnd = hwnd;

	//このコールバック関数はhwndWin11Notificationとの組み合わせででしか正しく動作しないので、原則としてhwnd = hwndWin11Notifyだがコールバック関数なのでtempHwndで処理している


//	if (b_DebugLog) writeDebugLog_Win10("[for_win11.c][WndProcWin11Notify] Window Message was recevied, message = ", message);


	switch (message) //Win11タスクトレイ通知アイコン部に操作を転送
	{
		case WM_LBUTTONDOWN:
			//ここに、押された場所がposXShowDesktopAreaより右ならデスクトップを表示する、というのを実装する*****
			if (GET_X_LPARAM(lParam) > posXShowDesktopArea) {
				//https://www.ka-net.org/blog/?p=8432
				//http://katsura-kotonoha.sakura.ne.jp/prog/win/tip00024.shtml
				//https://andromemo.blog.jp/archives/35317591.html
				if (b_DebugLog)("[for_win11.c][WndProcWin11Notify] ToggleDesktop caled.", 999);
				ShellExecute(NULL, "open", "explorer.exe", "shell:::{3080F90D-D7AD-11D9-BD98-0000947B0257}", NULL, SW_SHOWNORMAL);
			}
			else {
				ShellExecute(NULL, "open", "ms-actioncenter:", NULL, NULL, SW_SHOWNORMAL);
			}
			break;
		case WM_RBUTTONDOWN:
		{
			POINT pos = { 0,0 };
			MapWindowPoints(hwndWin11Notify, GetDesktopWindow(), &pos, 1);
			PostMessage(hwndTClockExeMain, WM_CONTEXTMENU, wParam, (LPARAM)MAKELONG(pos.x, pos.y));		//正しい場所にメニューを出すために位置情報を画面上の絶対座標に変えて送る必要がある。
			return 0;
		}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}







void UpdateHdcYesWin11Notify(int num_notify)
{
	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c]UpdateHdcYesWin11Notify called with num_notify =", num_notify);

	HDC tempDC;
	BYTE tempR, tempG, tempB;
	BITMAP tempBitmap;
	unsigned tempUnsigned;
	RGBQUAD* color;

	if (!hdcYesWin11Notify || !hbmpIconYesWin11Notify)return;

//	hbmpIconYesWin11Notify = LoadBitmap(hmod, IDB_BITMAP4);	

	tempR = GetRValue(colWin11Notify);
	tempG = GetGValue(colWin11Notify);
	tempB = GetBValue(colWin11Notify);

	tempDC = CreateCompatibleDC(hdcYesWin11Notify);
	SelectObject(tempDC, hbmpIconYesWin11Notify);
	GetObject(hbmpIconYesWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcYesWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);



	//for (color = m_color_YesWin11Notify_start; color < m_color_YesWin11Notify_end; ++color)
	//{
	//	color->rgbReserved = color->rgbRed;

	//	tempUnsigned = color->rgbRed * tempR / 255;
	//	color->rgbRed = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

	//	tempUnsigned = color->rgbGreen * tempG / 255;
	//	color->rgbGreen = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

	//	tempUnsigned = color->rgbBlue * tempB / 255;
	//	color->rgbBlue = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

	//	if (fillbackWin11NotifyIconInvert)	//fillbackcolorで色が暗いときのみ、背景を白に反転する。
	//	{
	//		if (color->rgbReserved == 0)
	//		{
	//			color->rgbRed = 255;
	//			color->rgbGreen = 255;
	//			color->rgbBlue = 255;
	//		}
	//	}
	//}


	DeleteDC(tempDC);


	SetTextAlign(hdcYesWin11Notify, TA_CENTER | TA_BOTTOM);
	SelectObject(hdcYesWin11Notify, hFontNotify);
	SetBkMode(hdcYesWin11Notify, TRANSPARENT);
	//if (fillbackWin11NotifyIconInvert) {
	//	SetTextColor(hdcYesWin11Notify, 0x00FFFFFF);
	//}
	//else {
		SetTextColor(hdcYesWin11Notify, 0x00000000);
	//}
	char tempString[2];

	sprintf(tempString, "%d", num_notify);
	if (num_notify < 10) {
		TextOut(hdcYesWin11Notify, posNotifyText.x, posNotifyText.y - posNotifyIcon.y, tempString, 1);
	}
	else {
		TextOut(hdcYesWin11Notify, posNotifyText.x, posNotifyText.y - posNotifyIcon.y, tempString, 2);
	}


	for (color = m_color_YesWin11Notify_start; color < m_color_YesWin11Notify_end; ++color)
	{
		color->rgbReserved = color->rgbRed;

		tempUnsigned = color->rgbRed * tempR / 255;
		color->rgbRed = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbGreen * tempG / 255;
		color->rgbGreen = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbBlue * tempB / 255;
		color->rgbBlue = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		if (fillbackWin11NotifyIconInvert)	//fillbackcolorで色が暗いときのみ、背景を白に反転する。
		{
			if (color->rgbReserved == 0)
			{
				color->rgbRed = 255;
				color->rgbGreen = 255;
				color->rgbBlue = 255;
			}
		}
	}



}


void DrawWin11Notify(BOOL b_forceUpdate)
{
	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] DrawWin11Notify called with bExistWin11Notify =", bExistWin11Notify);

	HDC hdc;
	BOOL b_update = b_forceUpdate;
	extern COLORREF originalColorTaskbar_ForWin11Notify;
	extern COLORREF originalColorTaskbar;
	extern COLORREF originalColorTaskbarEdge;

	hdc = GetDC(hwndWin11Notify);

	if (!hdc)	//bWin11Mainは自明(呼ばれる前に判定済み)。
	{	//通知ウィンドウが消されていたら作り直す。
		ReCreateWin11Notify();
	}
	else 
	{
		intWin11FocusAssist = GetFocusAssistState();
		intWin11NotificationNumber = GetNotificationNumber();

		//for test
//		intWin11NotificationNumber = rand() % 100;

		if (intWin11FocusAssistPrev != intWin11FocusAssist)
		{
			b_update = TRUE;
			intWin11FocusAssistPrev = intWin11FocusAssist;
		}

		if (bShowWin11NotifyNumber && (intWin11NotificationNumberPrev != intWin11NotificationNumber))
		{
			b_update = TRUE;
			UpdateHdcYesWin11Notify(intWin11NotificationNumber);
			intWin11NotificationNumberPrev = intWin11NotificationNumber;
		}

		//if (b_forceUpdate) {
		//	UpdateHdcYesWin11Notify(intWin11NotificationNumber);
		//	b_update = TRUE;
		//}

		if (b_update)
		{
			if (fillbackcolor) 
			{
				FillBack(hdcWin11Notify, widthWin11Notify, heightWin11Notify);
			}
			else 
			{
				if (Win11Type == 2)
				{
					originalColorTaskbar_ForWin11Notify = originalColorTaskbar;
					FillBack(hdcWin11Notify, widthWin11Notify, heightWin11Notify);
				}
				else
				{
					SelectObject(hdcWin11Notify, hBrushWin11Notify);
					PatBlt(hdcWin11Notify, 0, 0, widthWin11Notify, heightWin11Notify, BLACKNESS);
				}
			}
			
			hPenWin11Notify = CreatePen(PS_SOLID, 1, colWin11Notify);
			SelectObject(hdcWin11Notify, hPenWin11Notify);
			MoveToEx(hdcWin11Notify, posXShowDesktopArea, 0, NULL);
			LineTo(hdcWin11Notify, posXShowDesktopArea, heightWin11Notify);

			if (Win11Type == 2) {		//Win11Type2での上端ライン再現
				hPenWin11Notify = CreatePen(PS_SOLID, 1, originalColorTaskbarEdge);
				SelectObject(hdcWin11Notify, hPenWin11Notify);
				MoveToEx(hdcWin11Notify, 0, 0, NULL);
				LineTo(hdcWin11Notify, widthWin11Notify, 0);
			}


			if (fillbackcolor || (Win11Type == 2)) {

				if (intWin11FocusAssist > 0)
				{
					TransparentBlt(hdcWin11Notify, posNotifyIcon.x, posNotifyIcon.y, widthNotifyIcon, heightNotifyIcon, hdcFocusWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, (fillbackWin11NotifyIconInvert ? RGB(255, 255, 255) : RGB(0, 0, 0)));
				}
				else if (intWin11NotificationNumber > 0)
				{
					TransparentBlt(hdcWin11Notify, posNotifyIcon.x, posNotifyIcon.y, widthNotifyIcon, heightNotifyIcon, hdcYesWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, (fillbackWin11NotifyIconInvert ? RGB(255, 255, 255) : RGB(0, 0, 0)));
				}
				else
				{
					TransparentBlt(hdcWin11Notify, posNotifyIcon.x, posNotifyIcon.y, widthNotifyIcon, heightNotifyIcon, hdcNoWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, (fillbackWin11NotifyIconInvert ? RGB(255, 255, 255) : RGB(0, 0, 0)));

				}

				RGBQUAD* color;
				for (color = m_color_Win11Notify_start; color < m_color_Win11Notify_end; ++color) {
					color->rgbReserved = 255;
				}
			}
			else {
				if (intWin11FocusAssist > 0)
				{
					BitBlt(hdcWin11Notify, posNotifyIcon.x, posNotifyIcon.y, widthNotifyIcon, heightNotifyIcon, hdcFocusWin11Notify, 0, 0, SRCCOPY);
				}
				else if (intWin11NotificationNumber > 0)
				{
					BitBlt(hdcWin11Notify, posNotifyIcon.x, posNotifyIcon.y, widthNotifyIcon, heightNotifyIcon, hdcYesWin11Notify, 0, 0, SRCCOPY);
				}
				else
				{
					BitBlt(hdcWin11Notify, posNotifyIcon.x, posNotifyIcon.y, widthNotifyIcon, heightNotifyIcon, hdcNoWin11Notify, 0, 0, SRCCOPY);
				}
			}

			BitBlt(hdc, 0, 0, widthWin11Notify, heightWin11Notify, hdcWin11Notify, 0, 0, SRCCOPY);
		}



		ReleaseDC(hwndWin11Notify, hdc);;
	}

}


BOOL SetModifiedWidthWin11Tray(void)	//戻り値はアイコンの有無が変化したかどうか：変化したらTRUE
{
	BOOL bPrev;
	bPrev = bExistWin11Notify;


	bExistWin11Notify = FALSE;
	cutOffWidthWin11Tray = defaultWin11ClockWidth;

	intWin11FocusAssist = GetFocusAssistState();
	intWin11NotificationNumber = GetNotificationNumber();

	if ((intWin11NotificationNumber > 0) || (intWin11FocusAssist > 0)) {
		bExistWin11Notify = TRUE;
		cutOffWidthWin11Tray += defaultWin11NotificationWidth;
	}

	modifiedWidthWin11Tray = origWidthWin11Tray - cutOffWidthWin11Tray;

	return (bPrev != bExistWin11Notify);
}


//Win11でメインクロックの配置と周辺調整
void SetMainClockOnTasktray_Win11(void)
{

	//この関数はWin11Type < 2 (ビルド22579未満)ではSubClassTrayProc_Win11のWM_NOTIFYからしか呼ばれない(状態を維持すること！)。
	//その場合CLOCKM_MOVEWIN11CONTENTBRIDGEはこの関数から投げる。
	//一方、Win11Type == 2では、メインクロックウィンドウへ投げたCLOCKM_MOVEWIN11CONTENTBRIDGEメッセージ経由でこちらに来る。呼ばれる順番が逆なのでくれぐれも注意する。
	//Win11Type == 2でCLOCKM_MOVEWIN11CONTENTBRIDGEメッセージを投げると無限ループする！


	//Win11におけるタスクトレイ内再配置の方法
	//Win11ではTClockWin11というクラスをタスクバー直下に作って、左からワークアイコン群、トレイエリア(左に移動)、TClockの順に並べる状態を維持する。


	if (b_DebugLog) writeDebugLog_Win10("[for_win11.c] SetMainClockOnTasktray_Win11 called. ", 999);
	char tempClassName[32];

	//確保すべき時計のサイズを取得
	CalcMainClockSize();

	//タスクバー幅を取得(Win11はめったに変わらないが、念のため。初回は必要)
	GetTaskbarSize();

	//Win11タスクトレイ切り落とし幅を決定
	SetModifiedWidthWin11Tray();


	if (Win11Type == 2)
	{
		
		//タスクバーを短縮する。
		//SWP_NOSENDCHANGINGを入れておかないとタスクバーは移動しない(強制的に戻されるのか？)
		//SWP_NOTOPMOSTを入れていったんTClockを前に出すが、これはいずれTOPMOSTになる。
		modifiedWidthTaskbar_Win11Type2 = originalWidthTaskbar - widthMainClockFrame - widthWin11Notify + cutOffWidthWin11Tray;
		SetWindowPos(hwndTaskBarMain, NULL, 0, 0, modifiedWidthTaskbar_Win11Type2, heightTaskbar,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING);

		//TClockの左端のX座標を求める。
		posXMainClock = originalWidthTaskbar - widthMainClockFrame - widthWin11Notify;

		SetWindowPos(hwndWin11ContentBridge, NULL, 0, 0, posXMainClock, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_HIDEWINDOW);

		//TClockBarWin11を設定する。
		SetWindowPos(hwndTClockBarWin11, hwndTaskBarMain, 0, posYTaskbar, originalWidthTaskbar, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);

		//TClockのウィンドウを所定の場所に移動する。
		//SetWindowPos(hwndClockMain, HWND_TOPMOST, posXMainClock, 0, widthMainClockFrame, heightMainClockFrame,
		//	SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
		SetWindowPos(hwndClockMain, NULL, posXMainClock, 0, widthMainClockFrame, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);

		//自作通知ウィンドウの場所を再設定する。
		if (hwndWin11Notify) {
			//SetWindowPos(hwndWin11Notify, HWND_TOPMOST, posXMainClock + widthMainClockFrame, 0, widthWin11Notify, heightMainClockFrame,
			//	SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
			SetWindowPos(hwndWin11Notify, NULL, posXMainClock + widthMainClockFrame, 0, widthWin11Notify, heightMainClockFrame,
				SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
		}
	}
	else
	{
		int tempX = widthTaskbar - widthMainClockFrame - modifiedWidthWin11Tray - widthWin11Notify;

		//トレイを、TClockのウィンドウ分左に移動して、右側の時計領域が見えなくなるようにリサイズする。
		SetWindowPos(hwndTrayMain, NULL, tempX, 0, modifiedWidthWin11Tray, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);


		//『小さいタスクバー』の場合にトレイアイコンを上にシフトする。
		//ContentBridgeの操作はここでやっても上書きされる模様。ちらつき防止のために一時的に消すことはできた。
		//下のほうに記述するPostmessage経由でMoveWin11ContentBridgeで調整する。
		//if (adjustWin11TrayYpos != 0)
		//{
		//	SetWindowPos(hwndWin11InnerTrayContentBridge, NULL, 0, - adjustWin11TrayYpos, modifiedWidthWin11Tray, heightMainClockFrame + adjustWin11TrayYpos,
		//		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_HIDEWINDOW);
		//}
		if (adjustWin11TrayYpos != 0) ShowWindow(hwndWin11InnerTrayContentBridge, SW_HIDE);


		//アプリアイコン列を、トレイの左端までにリサイズする
		SetWindowPos(hwndWin11ReBarWin, NULL, 0, 0, tempX, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);

		//TClockの左端のX座標を求める。
		posXMainClock = widthTaskbar - widthMainClockFrame - widthWin11Notify;

		//TClockのウィンドウを所定の場所に移動する。
		SetWindowPos(hwndClockMain, NULL, posXMainClock, 0, widthMainClockFrame, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);


		//一番上のContentBridgeクラス(2つあるが、トレイの中にあるほうではなく、タスクバー直下)をリサイズする。
		//ContentBridgeの操作はここでやっても上書きされる模様。
		//ここでは実行せず、下のほうに記述するPostmessage経由でMoveWin11ContentBridgeで調整する。
		//SetWindowPos(hwndWin11ContentBridge, NULL, 0, 0, posXMainClock, heightMainClockFrame,
		//	SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOREDRAW | SWP_DEFERERASE);

		//自作通知ウィンドウの場所を再設定する。
		if (hwndWin11Notify) {
			SetWindowPos(hwndWin11Notify, NULL, posXMainClock + widthMainClockFrame, 0, widthWin11Notify, heightMainClockFrame,
				SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
			ShowWindow(hwndWin11Notify, SW_SHOW);
		}

		//ContentBridgeの操作はここでやっても上書きされる模様。
		//あらためてSubClassTrayProc_Win11にWM_NCPAINTが来たタイミングで行う。
		//…だがこれでは小さいタスクバーに伴うアイコン下がりは残ってしまうので、↓のディレイ方式にする必要がある。
		//	bTokenMoveContentBridge = TRUE;

		//ContentBridgeの移動は少し遅れて行う必要があるので、こちらでまとめて実行する。
		PostMessage(hwndClockMain, CLOCKM_MOVEWIN11CONTENTBRIDGE, 0, 0);	//タイマーを使うほどのディレイは不要で、Postmessage経由の実行でOK
	}



	//サイズ更新したら、hdcClockを作り直すようにする。
	CreateClockDC();


	//ここでRedrawMainTaskbarを呼んではいけない！(たぶん)。ループが発生する。
	//InvalidateRectでの再描画ではループしないが、ちらつきの元なので避ける。
//		InvalidateRect(hwndTrayMain, NULL, TRUE);	
//	InvalidateRect(hwndTaskBarMain, NULL, TRUE);

	//	SetAllSubClocks();	//メインクロックの状態が変わったら、毎回サブクロックも反映させる必要あり。
	//->すぐに実行するとうまく行かない＆処理が繰り返されるのでディレイで実行

	if (bEnableSubClks){
		SetTimer(hwndClockMain, IDTIMERDLL_DELEYED_RESPONSE, 500, NULL);
	}
}


void GetWin11TaskbarType(void)
{
	HKEY hkey;
	DWORD reg_data;
	DWORD regtype, size;
	size = sizeof(DWORD);
	typeWin11Taskbar = 1;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS) {
		if (RegQueryValueEx(hkey, "TaskbarSi", NULL, &regtype, (LPBYTE)&reg_data, &size) == ERROR_SUCCESS)
		{
			typeWin11Taskbar = (int)reg_data;
			if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][GetWin11TaskbarType] Registry access succeeded, typeWin11Taskbar =", typeWin11Taskbar);
		}
		else
		{
			if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][GetWin11TaskbarType] DWORD\"TaskbarSi\" does not exist.", 999);
		}
		RegCloseKey(hkey);
	}
	else
	{
		if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][GetWin11TaskbarType] Registry access failed...", 999);
	}
}


void MoveWin11ContentBridge(int operation)	//Win11 Type2 (build 22579およびそれ以降)ではhwndWin11ContentBridgeの上にアイコンが載るようになっていて、この処理では元アイコン群が切れて見えなくなる。
{
	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] MoveWin11ContentBridge called. operation =", operation);

	if (Win11Type == 2) {
		switch(operation) 
		{
			case 0:
				SetMainClockOnTasktray_Win11();
				break;
			case 1:
				SetWindowPos(hwndWin11ContentBridge, NULL, 0, 0, posXMainClock, heightMainClockFrame,
					SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_HIDEWINDOW);
				break;
			case 2:
				break;
			case 3:
				ReturnToOriginalTaskBar();
				break;
			default:
				break;
		}
	}
	else {
		SetWindowPos(hwndWin11ContentBridge, NULL, 0, 0, posXMainClock, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOREDRAW | SWP_DEFERERASE);

		if (adjustWin11TrayYpos != 0)
		{
			//改めて移動して、表示する。
			SetWindowPos(hwndWin11InnerTrayContentBridge, NULL, 0, -adjustWin11TrayYpos, modifiedWidthWin11Tray, heightMainClockFrame + adjustWin11TrayYpos,
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);	// | SWP_SHOWWINDOW
																		//InvalidateRect(hwndTrayMain, NULL, TRUE);
		}

		ShowWindow(hwndWin11InnerTrayContentBridge, SW_SHOW);
	}

}

