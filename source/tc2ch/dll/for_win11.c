#include "tcdll.h"
#include "resource.h"









//非Win11専用
extern BOOL b_DebugLog;
extern HANDLE hmod;
extern HWND hwndClockMain;
extern HWND hwndTrayMain;
extern HWND hwndTClockExeMain;
extern HWND hwndTaskBarMain;
extern BOOL bEnableSubClks;
extern BOOL b_BatteryLifeAvailable;
extern int widthTaskbar;
extern int widthMainClockFrame;
extern int heightMainClockFrame;
extern int widthMainClockFrame;
extern int heightTaskbar;

//Win11対応関連
//Win11対応自作ウィンドウモードフラグ
extern BOOL bWin11Main;
extern BOOL bWin11Sub;

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
extern HBITMAP hbm_DIBSection_YesWin11Notify;
extern HBITMAP hbm_DIBSection_NoWin11Notify;
extern HBITMAP hbm_DIBSection_FocusWin11Notify;
extern HBITMAP hbmpIconYesWin11Notify;

extern RGBQUAD* m_color_YesWin11Notify_start;
extern RGBQUAD* m_color_YesWin11Notify_end;
extern RGBQUAD* m_color_NoWin11Notify_start;
extern RGBQUAD* m_color_NoWin11Notify_end;
extern RGBQUAD* m_color_FocusWin11Notify_start;
extern RGBQUAD* m_color_FocusWin11Notify_end;
extern COLORREF colWin11Notify;
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


void CreateWin11MainClock(void)
{

	//最初に残骸が残ってたら消す。
	HWND tempHwnd;
	while ((tempHwnd = FindWindowEx(hwndTaskBarMain, NULL, "TClockMain", NULL)) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);


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

	hwndClockMain = CreateWindowEx(WS_EX_TOPMOST, szClassName, "TClockMain",
		WS_VISIBLE | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hwndTaskBarMain, NULL, hmod, NULL);

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
	while ((tempHwnd = FindWindowEx(hwndTaskBarMain, NULL, "TClockNotify", NULL)) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);

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

	hwndWin11Notify = CreateWindowEx(WS_EX_TOPMOST, "TClockNotify", "TClockNotify",
		WS_VISIBLE | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hwndTaskBarMain, NULL, hmod, NULL);

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
	HBITMAP hbmpIconNoWin11Notify = NULL;
	HBITMAP hbmpIconFocusWin11Notify = NULL;

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
	bmi.bmiHeader.biWidth = widthNotifyIcon;
	bmi.bmiHeader.biHeight = heightNotifyIcon;


	//通知ありアイコンDC作成
	//DIB section 作成, 最初と最後のアドレス取得
	hbm_DIBSection_YesWin11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_YesWin11Notify_start, NULL, 0);
	m_color_YesWin11Notify_end = m_color_YesWin11Notify_start + (widthNotifyIcon * heightNotifyIcon);
	SelectObject(hdcYesWin11Notify, hbm_DIBSection_YesWin11Notify);

	//リソースビットマップ(白/透明)を枠にあわせてコピー。枠はGetWin11ElementSizeで決める)
	hdcYesWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcYesWin11Notify, hbm_DIBSection_YesWin11Notify);
	SetStretchBltMode(hdcYesWin11Notify, HALFTONE);

	tempDC = CreateCompatibleDC(hdc);
	SelectObject(tempDC, hbmpIconYesWin11Notify);

	GetObject(hbmpIconYesWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcYesWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);

	//ロードしたビットマップはもういらないので破棄
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
	}

	//通知なしアイコンDC作成
	//DIB section 作成, 最初と最後のアドレス取得
	hbm_DIBSection_NoWin11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_NoWin11Notify_start, NULL, 0);
	m_color_NoWin11Notify_end = m_color_NoWin11Notify_start + (widthNotifyIcon * heightNotifyIcon);
	SelectObject(hdcNoWin11Notify, hbm_DIBSection_NoWin11Notify);

	//リソースビットマップ(白/透明)を枠にあわせてコピー。枠はGetWin11ElementSizeで決める)
	hdcNoWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcNoWin11Notify, hbm_DIBSection_NoWin11Notify);
	SetStretchBltMode(hdcNoWin11Notify, HALFTONE);

	DeleteDC(tempDC);
	tempDC = CreateCompatibleDC(hdc);
	SelectObject(tempDC, hbmpIconNoWin11Notify);

	GetObject(hbmpIconNoWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcNoWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);

	//ロードしたビットマップはもういらないので破棄
	DeleteObject(hbmpIconNoWin11Notify);

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
	}

	//集中モードアイコンDC作成
	//DIB section 作成, 最初と最後のアドレス取得
	hbm_DIBSection_FocusWin11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_FocusWin11Notify_start, NULL, 0);
	m_color_FocusWin11Notify_end = m_color_FocusWin11Notify_start + (widthNotifyIcon * heightNotifyIcon);
	SelectObject(hdcFocusWin11Notify, hbm_DIBSection_FocusWin11Notify);

	//リソースビットマップ(白/透明)を枠にあわせてコピー。枠はGetWin11ElementSizeで決める)
	hdcFocusWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcFocusWin11Notify, hbm_DIBSection_FocusWin11Notify);
	SetStretchBltMode(hdcFocusWin11Notify, HALFTONE);

	tempDC = CreateCompatibleDC(hdc);
	SelectObject(tempDC, hbmpIconFocusWin11Notify);

	GetObject(hbmpIconFocusWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcFocusWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);

	//ロードしたビットマップはもういらないので破棄
	DeleteObject(hbmpIconFocusWin11Notify);

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

	if (typeWin11Taskbar == 0)
	{
		widthWin11Button = heightTaskbar * 3 / 4;	//『小さいタスクバー』の場合
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

	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][GetWin11TrayWidth] origWidthWin11Tray = ", origWidthWin11Tray);
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
		case (WM_USER + 100):
		{
			if (b_DebugLog)writeDebugLog_Win10("[fow_win11.c][SubclassTrayProc_Win11] WM_USER + 100 (1124) recevied.", 999);
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


			//再配置に先だって必ず呼ばれるわけではない。ここに処理を入れても確実に実行されない。

			return ret;
		}
		case (WM_NCCALCSIZE):
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
		case WM_NOTIFY:
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
	//messageは、定常運転時はWM_TIMER(275)以外はめったに来ない。

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
	BYTE tempR, tempG, tempB, tempAlpha;
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



	for (color = m_color_YesWin11Notify_start; color < m_color_YesWin11Notify_end; ++color)
	{
		color->rgbReserved = color->rgbRed;

		tempUnsigned = color->rgbRed * tempR / 255;
		color->rgbRed = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbGreen * tempG / 255;
		color->rgbGreen = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbBlue * tempB / 255;
		color->rgbBlue = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);
	}


	DeleteDC(tempDC);


	SetTextAlign(hdcYesWin11Notify, TA_CENTER | TA_BOTTOM);
	SelectObject(hdcYesWin11Notify, hFontNotify);
	SetBkMode(hdcYesWin11Notify, TRANSPARENT);
	SetTextColor(hdcYesWin11Notify, 0x00000000);
	char tempString[2];

	sprintf(tempString, "%d", num_notify);
	if (num_notify < 10) {
		TextOut(hdcYesWin11Notify, posNotifyText.x, posNotifyText.y - posNotifyIcon.y, tempString, 1);
	}
	else {
		TextOut(hdcYesWin11Notify, posNotifyText.x, posNotifyText.y - posNotifyIcon.y, tempString, 2);
	}
}


void DrawWin11Notify(BOOL b_forceUpdate)
{
	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] DrawWin11Notify called with bExistWin11Notify =", bExistWin11Notify);

	HDC hdc;
	BOOL b_update = FALSE;


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

		if (b_forceUpdate) {
			UpdateHdcYesWin11Notify(intWin11NotificationNumber);
			b_update = TRUE;
		}

		if (b_update)
		{
			SelectObject(hdc, hBrushWin11Notify);
			PatBlt(hdc, 0, 0, widthWin11Notify, heightWin11Notify, BLACKNESS);

			SelectObject(hdc, hPenWin11Notify);

			MoveToEx(hdc, posXShowDesktopArea, 0, NULL);
			LineTo(hdc, posXShowDesktopArea, heightWin11Notify);

			if (intWin11FocusAssist > 0)
			{
				BitBlt(hdc, posNotifyIcon.x, posNotifyIcon.y, widthNotifyIcon, heightNotifyIcon, hdcFocusWin11Notify, 0, 0, SRCCOPY);
			}
			else if (intWin11NotificationNumber > 0)
			{
				BitBlt(hdc, posNotifyIcon.x, posNotifyIcon.y, widthNotifyIcon, heightNotifyIcon, hdcYesWin11Notify, 0, 0, SRCCOPY);
			}
			else
			{
				BitBlt(hdc, posNotifyIcon.x, posNotifyIcon.y, widthNotifyIcon, heightNotifyIcon, hdcNoWin11Notify, 0, 0, SRCCOPY);
			}
		}



		ReleaseDC(hwndWin11Notify, hdc);;
	}

}


void SetModifiedWidthWin11Tray(void)
{
	//int tempWidth = 0;
	//POINT pos = { 0,0 };
	//HWND tempHwnd;
	//BOOL bPositioning = 0;
	//int tempDetectionPoint = 0;

	//tempHwnd = FindWindowEx(hwndTrayMain, NULL, "TrayInputIndicatorWClass", NULL);
	//MapWindowPoints(tempHwnd, hwndTrayMain, &pos, 1);

	//cutOffWidthWin11Tray = origWidthWin11Tray - pos.x;	//cutOffWidthWin11Trayが右から切断すべき長さ=時計部長さとして計算していく。
	//if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][SetModifiedWidthWin11Tray] TrayInput X =", pos.x);



	////まず、固定分を引いていく。

	////手動調節分を引く。IMEアイコンは通常アイコンと同じ幅なので後で自動で引かれる。
	//cutOffWidthWin11Tray -= adjustWin11TrayCutPosition;



	////WiFi+スピーカー+バッテリアイコン部を引く。
	//if (b_BatteryLifeAvailable) {
	//	//システムにバッテリが存在する場合にはWiFi+スピーカー+バッテリの3アイコンと推測されるのでさらにボタン10/3個分(実測値)を引く※。
	//	//アイコン1つwidthWin11Button *2/3, 隙間はwidthWin11Button * 1/3
	//	cutOffWidthWin11Tray -= widthWin11Button * 10 / 3;
	//}
	//else
	//{
	//	//デスクトップなど、WiFiアイコン部に2個しかない構成の場合は7 / 3 個分(実測値)を引く※。
	//	cutOffWidthWin11Tray -= widthWin11Button * 7 / 3;
	//}
	////※の値は正確な値を選ぶ必要がある。2021/11月現在の具体的な値はGetWin11ElementSize関数のコメントに記載


	////ここまで引いたら、cutOffWidthWin11Trayは
	////最小の時計の幅
	////右側の通知マーク(ボタン幅x1.2弱)
	////IMEアイコン、位置情報やソフトキーボードなどの臨時アイコン(アイコン幅 = widthWin11Icon)

	////アイコン幅(ボタンx4/3)単位で右部長さを減らしていく。
	//while ((cutOffWidthWin11Tray - defaultWin11ClockWidth) >= widthWin11Icon)
	//{
	//	cutOffWidthWin11Tray -= widthWin11Icon;		//右部の長さからアイコン1個分短くする。
	//}

	////残った長さは、時計(+通知)の部分だけになるので、元の長さからひけばうまくいくはず。
	//modifiedWidthWin11Tray = origWidthWin11Tray - cutOffWidthWin11Tray;

	//if (b_DebugLog)
	//{
	//	writeDebugLog_Win10("[for_win11.c][SetModifiedWidthWin11Tray] A: origWidthWin11Tray =", origWidthWin11Tray);
	//	writeDebugLog_Win10("[for_win11.c][SetModifiedWidthWin11Tray] defaultWin11ClockWidth =", defaultWin11ClockWidth);
	//	writeDebugLog_Win10("[for_win11.c][SetModifiedWidthWin11Tray] B: cutOffWidthWin11Tray =", cutOffWidthWin11Tray);
	//	writeDebugLog_Win10("[for_win11.c][SetModifiedWidthWin11Tray] A-B: modifiedWidthWin11Tray =", modifiedWidthWin11Tray);
	//}

	//tempDetectionPoint = widthWin11Button / 2;	//この値は実際には　ボタン幅 x 7/6以下なら効果は同じはず。adjustWin11DetectNotifyは余り意味がない。
	//if (cutOffWidthWin11Tray > (defaultWin11ClockWidth + tempDetectionPoint - adjustWin11DetectNotify))		//ここで普段より有意に長いか判定する。
	//{
	//	bExistWin11Notify = TRUE;
	//}
	//else
	//{
	//	bExistWin11Notify = FALSE;
	//}

	//	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][SetModifiedWidthWin11Tray] bExistWin11Notify is speculated to be ", bExistWin11Notify);



	//Ver 4.2.1以降は通知数が取得できるようになった。
	bExistWin11Notify = FALSE;
	cutOffWidthWin11Tray = defaultWin11ClockWidth;

	intWin11FocusAssist = GetFocusAssistState();
	intWin11NotificationNumber = GetNotificationNumber();

	if ((intWin11NotificationNumber > 0) || (intWin11FocusAssist > 0)) {
		bExistWin11Notify = TRUE;
		cutOffWidthWin11Tray += defaultWin11NotificationWidth;
	}

	modifiedWidthWin11Tray = origWidthWin11Tray - cutOffWidthWin11Tray;



}


//Win11でメインクロックの配置と周辺調整
void SetMainClockOnTasktray_Win11(void)
{

	//この関数はSubClassTrayProc_Win11のWM_NOTIFYからしか呼ばれない(状態を維持すること！)。


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
	SetWindowPos(hwndClockMain, HWND_TOPMOST, posXMainClock, 0, widthMainClockFrame, heightMainClockFrame,
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
//	ShowWindow(hwndClockMain, SW_SHOW);



	//一番上のContentBridgeクラス(2つあるが、トレイの中にあるほうではなく、タスクバー直下)をリサイズする。
	//ContentBridgeの操作はここでやっても上書きされる模様。
	//ここでは実行せず、下のほうに記述するPostmessage経由でMoveWin11ContentBridgeで調整する。
	//SetWindowPos(hwndWin11ContentBridge, NULL, 0, 0, posXMainClock, heightMainClockFrame,
	//	SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOREDRAW | SWP_DEFERERASE);

	//自作通知ウィンドウの場所を再設定する。
	if (hwndWin11Notify) {
		SetWindowPos(hwndWin11Notify, HWND_TOPMOST, posXMainClock + widthMainClockFrame, 0, widthWin11Notify, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
		ShowWindow(hwndWin11Notify, SW_SHOW);
	}

	//ContentBridgeの操作はここでやっても上書きされる模様。
	//あらためてSubClassTrayProc_Win11にWM_NCPAINTが来たタイミングで行う。
	//…だがこれでは小さいタスクバーに伴うアイコン下がりは残ってしまうので、↓のディレイ方式にする必要がある。
//	bTokenMoveContentBridge = TRUE;

	//ContentBridgeの移動は少し遅れて行う必要があるので、こちらでまとめて実行する。
//	DelayedMoveWin11ContentBridge();
	PostMessage(hwndClockMain, CLOCKM_MOVEWIN11CONTENTBRIDGE, 0, 0);	//タイマーを使うほどのディレイは不要で、Postmessage経由の実行でOK

	//サイズ更新したら、hdcClockを作り直すようにする。CreateClockDCはここから呼ぶだけで必要充分なはず。
	CreateClockDC();


	//ここでRedrawMainTaskbarを呼んではいけない！(たぶん)。ループが発生する。
	//InvalidateRectでの再描画ではループしないが、ちらつきの元なので避ける。
//		InvalidateRect(hwndTrayMain, NULL, TRUE);	
//	InvalidateRect(hwndTaskBarMain, NULL, TRUE);

	//	SetAllSubClocks();	//メインクロックの状態が変わったら、毎回サブクロックも反映させる必要あり。
	//->すぐに実行するとうまく行かない＆処理が繰り返されるのでディレイで実行
	//Ver 4.0.3では実効的には何も起こらない。
	if (bEnableSubClks) DelayedUpdateSubClks();
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

//void DelayedMoveWin11ContentBridge(void)
//{
////	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] DelayedMoveWin11ContentBridge called.", 999);
//	SetTimer(hwndClockMain, IDTIMERDLL_MOVEWIN11CONTENTBRIDGE, 10, NULL);		//10ms後に動作->実際にはタイマをつかわなくてPostmessageでOKだった。
//}


void MoveWin11ContentBridge(void)
{
	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] MoveWin11ContentBridge called.", 999);
	SetWindowPos(hwndWin11ContentBridge, NULL, 0, 0, posXMainClock, heightMainClockFrame,
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOREDRAW | SWP_DEFERERASE);

	if (adjustWin11TrayYpos != 0)
	{
		//改めて移動して、表示する。
		SetWindowPos(hwndWin11InnerTrayContentBridge, NULL, 0, - adjustWin11TrayYpos, modifiedWidthWin11Tray, heightMainClockFrame + adjustWin11TrayYpos,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);	// | SWP_SHOWWINDOW
		//InvalidateRect(hwndTrayMain, NULL, TRUE);
	}

	ShowWindow(hwndWin11InnerTrayContentBridge, SW_SHOW);
}

