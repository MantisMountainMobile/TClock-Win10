#include "tcdll.h"
#include "resource.h"





#define TIMEOUT_MS_TCLOCKBARWIN11	100



//��Win11��p
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

//Win11�Ή��֘A
//Win11�Ή�����E�B���h�E���[�h�t���O
extern BOOL bWin11Main;
extern BOOL bWin11Sub;

extern int Win11Type;

//Win11�p�֘A�E�B���h�E�n���h��
extern HWND hwndWin11ReBarWin;
extern HWND hwndWin11ContentBridge;
extern HWND hwndWin11InnerTrayContentBridge;
extern HWND hwndWin11Notify;

//�ʒm�E�B���h�E�p
//���p�ݒ�E�t���O
extern BOOL bUseWin11Notify;		//���[�U�ݒ�
extern BOOL bEnabledWin11Notify;	//���p���邩�ǂ���(�ʒm�E�B���h�E���̂͏�ɍ��)
									//�ʒm�E�B���h�E�`��p
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

//�����_�Œʒm���o�Ă��邩�ǂ���
extern BOOL bExistWin11Notify;
//�ʒm�E�B���h�E�̃T�C�Y
extern int widthWin11Notify;
extern int heightWin11Notify;
//�ʒm�E�B���h�E���̃A�C�R������ʒu�ƃT�C�Y
extern POINT posNotifyIcon;
extern int widthNotifyIcon;
extern int heightNotifyIcon;
extern int posXShowDesktopArea;
extern POINT posNotifyText;

//���C�����v(����)�����p�ϐ�
extern int widthWin11Clock;
extern int heightWin11Clock;
//���i�T�C�Y(����Win11�ʒm�̈�̉�͗p)
extern int widthWin11Icon;
extern int widthWin11Button;
extern int origWidthWin11Tray;
extern int origHeightWin11Tray;
extern int defaultWin11ClockWidth;
extern int defaultWin11NotificationWidth;
extern int adjustWin11TrayYpos;

//�����p
extern int adjustWin11TrayCutPosition;
extern int adjustWin11DetectNotify;
extern int adjustWin11ClockWidth;
//�z�u�E�J�b�g�����̌��ʓ�����l
extern int cutOffWidthWin11Tray;
extern int modifiedWidthWin11Tray;
extern int posXMainClock;

//�w�������^�X�N�o�[�x���ǂ���
extern int typeWin11Taskbar;
//�w�������^�X�N�o�[�x�Ńg���C�ʒu�������邩�ǂ���
extern BOOL bAdjustTrayWin11SmallTaskbar;

//�ʒm�̈�p�I�u�W�F�N�g
extern HBRUSH hBrushWin11Notify;
extern HPEN hPenWin11Notify;


//Type2

extern WNDPROC oldProcTaskbarContentBridge_Win11;

BOOL fillbackWin11NotifyIconInvert = FALSE;

int  modifiedWidthTaskbar_Win11Type2 = 0;

BOOL b_ShowingTClockBarWin11 = FALSE;


//extern BOOL bTokenMoveContentBridge;

//Focus Assist���
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
	//�ŏ��Ɏc�[���c���Ă�������B
	HWND tempHwnd;
	if(tempHwnd = FindWindow("TClockBarWin11", NULL) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);


	//Win11Type2�p��TClockBar�̃E�B���h�E�����삷��B
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

	//Win11Type2�̏ꍇ�͑S�̂����e����TClockBarWin11�����
	if (Win11Type == 2) {
		CreateTClockBarWin11Type2();
	}


	//�ŏ��Ɏc�[���c���Ă�������B
	HWND tempHwnd;
	if (Win11Type < 2){
		while ((tempHwnd = FindWindowEx(hwndTaskBarMain, NULL, "TClockMain", NULL)) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);
	}


	//Win11�p��TClock�̃E�B���h�E�����삷��B
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
		CreateWin11Notify();		//�g��Ȃ��ꍇ������Ă����B
	}
}

HWND CreateWin11SubClock(HWND tempHwndTaskbar)
{


	//�ŏ��Ɏc�[���c���Ă�������B
	HWND tempHwnd;
	if ((tempHwnd = FindWindowEx(tempHwndTaskbar, NULL, "TClockSub", NULL)) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);

	WNDCLASS classTClockWin11Sub;
	TCHAR szClassName[] = TEXT("TClockSub");

	//Win11�p��TClockSub�̃N���X�����삷��B
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

	//�ŏ��Ɏc�[���c���Ă�������B
	HWND tempHwnd;
	if (Win11Type < 2) {
		while ((tempHwnd = FindWindowEx(hwndTaskBarMain, NULL, "TClockNotify", NULL)) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);
	}

	//Win11�p��TClock�̃E�B���h�E�����삷��B
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

	//�ŏ��Ɏc�[���c���Ă�������B
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


	//�r�b�g�}�b�v�̏���
	//��->�w�i
	//��(�~������)->�A�C�R���F
	//�T�C�Y:120 x 160����{�����A����Ă�OK

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

	hbmpIconNoWin11Notify = LoadBitmap(hmod, IDB_BITMAP1);		//�u�����N
	hbmpIconFocusWin11Notify = LoadBitmap(hmod, IDB_BITMAP3);	//�O�����}�[�N

	if (bShowWin11NotifyNumber) {
		hbmpIconYesWin11Notify = LoadBitmap(hmod, IDB_BITMAP4);		//�h��Ԃ�(��������p)
	}
	else {
		hbmpIconYesWin11Notify = LoadBitmap(hmod, IDB_BITMAP2);		//!�}�[�N
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
	

	//Notification�N���A�E�`��p�I�u�W�F�N�g�w��
	if (!hBrushWin11Notify)DeleteObject(hBrushWin11Notify);
	hBrushWin11Notify = CreateSolidBrush(RGB(0, 0, 0));
	SelectObject(hdc, hBrushWin11Notify);

	if (!hPenWin11Notify)DeleteObject(hPenWin11Notify);
	hPenWin11Notify = CreatePen(PS_SOLID, 1, colWin11Notify);
	SelectObject(hdc, hPenWin11Notify);


	//DIB section�쐬�pbitmap�\����
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


	//�ʒm����A�C�R��DC�쐬
	//DIB section �쐬, �ŏ��ƍŌ�̃A�h���X�擾
	hbm_DIBSection_YesWin11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_YesWin11Notify_start, NULL, 0);
	m_color_YesWin11Notify_end = m_color_YesWin11Notify_start + (widthNotifyIcon * heightNotifyIcon);

	//���\�[�X�r�b�g�}�b�v(��/����)��g�ɂ��킹�ăR�s�[�B�g��GetWin11ElementSize�Ō��߂�)
	hdcYesWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcYesWin11Notify, hbm_DIBSection_YesWin11Notify);
	SetStretchBltMode(hdcYesWin11Notify, HALFTONE);

	tempDC = CreateCompatibleDC(hdc);
	SelectObject(tempDC, hbmpIconYesWin11Notify);

	GetObject(hbmpIconYesWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcYesWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);

	//���[�h�����r�b�g�}�b�v�͂�������Ȃ��̂Ŕj��->�Ō�܂ŗ��p�ɕύX
//	DeleteObject(hbmpIconYesWin11Notify);

	for (color = m_color_YesWin11Notify_start; color < m_color_YesWin11Notify_end; ++color)
	{



		//�F�����݂���|�C���g��Alpha���P�x�l�ɂ���(����/���r�b�g�}�b�v�ɂ��Ă����΂ǂ̐F����ł�����)�B�������Ȃ��ƍ����\������Ȃ�
		color->rgbReserved = color->rgbRed;

		tempUnsigned = color->rgbRed * tempR / 255;
		color->rgbRed = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbGreen * tempG / 255;
		color->rgbGreen = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbBlue * tempB / 255;
		color->rgbBlue = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		if (fillbackWin11NotifyIconInvert)	//fillbackcolor�ŐF���Â��Ƃ��̂݁A�w�i�𔒂ɔ��]����B
		{
			if (color->rgbReserved == 0)
			{
				color->rgbRed = 255;
				color->rgbGreen = 255;
				color->rgbBlue = 255;
			}
		}

	}

	//�ʒm�Ȃ��A�C�R��DC�쐬
	//DIB section �쐬, �ŏ��ƍŌ�̃A�h���X�擾
	hbm_DIBSection_NoWin11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_NoWin11Notify_start, NULL, 0);
	m_color_NoWin11Notify_end = m_color_NoWin11Notify_start + (widthNotifyIcon * heightNotifyIcon);

	//���\�[�X�r�b�g�}�b�v(��/����)��g�ɂ��킹�ăR�s�[�B�g��GetWin11ElementSize�Ō��߂�)
	hdcNoWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcNoWin11Notify, hbm_DIBSection_NoWin11Notify);
	SetStretchBltMode(hdcNoWin11Notify, HALFTONE);

	DeleteDC(tempDC);
	tempDC = CreateCompatibleDC(hdc);
	SelectObject(tempDC, hbmpIconNoWin11Notify);

	GetObject(hbmpIconNoWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcNoWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);

	//���[�h�����r�b�g�}�b�v�͂�������Ȃ��̂Ŕj��->�Ō�܂ŗ��p�ɕύX
//	DeleteObject(hbmpIconNoWin11Notify);

	for (color = m_color_NoWin11Notify_start; color < m_color_NoWin11Notify_end; ++color)
	{
		//�F�����݂���|�C���g��Alpha���P�x�l�ɂ���(����/���r�b�g�}�b�v�ɂ��Ă����΂ǂ̐F����ł�����)�B�������Ȃ��ƍ����\������Ȃ�
		color->rgbReserved = color->rgbRed;

		tempUnsigned = color->rgbRed * tempR / 255;
		color->rgbRed = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbGreen * tempG / 255;
		color->rgbGreen = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbBlue * tempB / 255;
		color->rgbBlue = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		if (fillbackWin11NotifyIconInvert)	//fillbackcolor�ŐF���Â��Ƃ��̂݁A�w�i�𔒂ɔ��]����B
		{
			if (color->rgbReserved == 0)
			{
				color->rgbRed = 255;
				color->rgbGreen = 255;
				color->rgbBlue = 255;
			}
		}
	}

	//�W�����[�h�A�C�R��DC�쐬
	//DIB section �쐬, �ŏ��ƍŌ�̃A�h���X�擾
	hbm_DIBSection_FocusWin11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_FocusWin11Notify_start, NULL, 0);
	m_color_FocusWin11Notify_end = m_color_FocusWin11Notify_start + (widthNotifyIcon * heightNotifyIcon);

	//���\�[�X�r�b�g�}�b�v(��/����)��g�ɂ��킹�ăR�s�[�B�g��GetWin11ElementSize�Ō��߂�)
	hdcFocusWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcFocusWin11Notify, hbm_DIBSection_FocusWin11Notify);
	SetStretchBltMode(hdcFocusWin11Notify, HALFTONE);

	tempDC = CreateCompatibleDC(hdc);
	SelectObject(tempDC, hbmpIconFocusWin11Notify);

	GetObject(hbmpIconFocusWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcFocusWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);

	//���[�h�����r�b�g�}�b�v�͂�������Ȃ��̂Ŕj��->�Ō�܂ŗ��p�ɕύX
//	DeleteObject(hbmpIconFocusWin11Notify);

	for (color = m_color_FocusWin11Notify_start; color < m_color_FocusWin11Notify_end; ++color)
	{
		//�F�����݂���|�C���g��Alpha���P�x�l�ɂ���(����/���r�b�g�}�b�v�ɂ��Ă����΂ǂ̐F����ł�����)�B�������Ȃ��ƍ����\������Ȃ�
		color->rgbReserved = color->rgbRed;

		tempUnsigned = color->rgbRed * tempR / 255;
		color->rgbRed = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbGreen * tempG / 255;
		color->rgbGreen = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		tempUnsigned = color->rgbBlue * tempB / 255;
		color->rgbBlue = (tempUnsigned>255 ? 255 : (BYTE)tempUnsigned);

		if (fillbackWin11NotifyIconInvert)	//fillbackcolor�ŐF���Â��Ƃ��̂݁A�w�i�𔒂ɔ��]����B
		{
			if (color->rgbReserved == 0)
			{
				color->rgbRed = 255;
				color->rgbGreen = 255;
				color->rgbBlue = 255;
			}
		}

	}


	//�ꎞDC�̍폜�ƒʒm�E�B���h�EDC�̃����[�X
	DeleteDC(tempDC);
	ReleaseDC(hwndWin11Notify, hdc);

}


void GetWin11ElementSize(void)
{
	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] GetWin11ElementSize called.", 999);
	//RECT tempRect;

	GetTaskbarSize();
	GetWin11TaskbarType(); //���W�X�g���ŏ������^�X�N�o�[�ݒ�����Ă��邩�ǂ����m�F

						   //���̓A�v���A�C�R���W�J�p�{�^��������ɂ��Ă����̂ŁAwidthWin11Button����ɂȂ��Ă���B
						   //�W�J�{�^�����Ȃ��ꍇ�����邱�Ƃ����������̂ŁA�����l��heightTaskbar / 2�Ɉڍs�����B

						   //Win11(2021/11�����_)�̃^�X�N�g���C�A�C�R���T�C�Y(�W���^�X�N�o�[)
						   //���g�嗦100%�̏ꍇ�̒l�B�{���ɂ��킹�Ēl���ς��(��:200%�Ȃ�{�̒l)
						   //��WinExplorer�Œ��ׂ��ꍇ�ɂ́A���ۂ̔{���ɂ�炸���L��100%���Z�̒l��������B
						   //�^�X�N�o�[����: 48
						   //�A�v���A�C�R���W�J�{�^��(�A�v���A�C�R��1�Ɠ���)��	: 24
						   //IME�A�^�b�`�L�[�{�[�h�A�ʒu��񓙂�OS�A�C�R��: 32
						   //���v��: 88 = 24 * 11 / 3
						   //���v�ɒʒm���o���ꍇ�̍L���蕝: 28
						   //���A�C�R����32�ƒʒm��28�̍��ŁA�ʒm������s���K�v������A�Ƃ������ƁB
						   //WiFi���A�C�R����3����ꍇ: 80 (�A�C�R��1��16 x 3 + ���Ԃ�[:8 x 4)
						   //WiFi���A�C�R����3����ꍇ: 56 (�A�C�R��1��16 x 2 + ���Ԃ�[:8 x 3)

						   //Win11(2021/11�����_)�̃^�X�N�g���C�A�C�R���T�C�Y(���W�X�g������ɂ��w�������^�X�N�o�[�x)
						   //���g�嗦100%�̏ꍇ�̒l�B�{���ɂ��킹�Ēl���ς��(��:200%�Ȃ�{�̒l)
						   //�^�X�N�o�[����: 32
						   //�A�v���A�C�R���W�J�{�^��(�A�v���A�C�R��1�Ɠ���)��	: 24
						   //IME�A�^�b�`�L�[�{�[�h�A�ʒu��񓙂�OS�A�C�R��: 32
						   //���v��: 88
						   //���v�ɒʒm���o���ꍇ�̍L���蕝: 28
						   //WiFi���A�C�R����3����ꍇ:  80 (�A�C�R��1��16 x 3 + ���Ԃ�[:8 x 4)
						   //WiFi���A�C�R����3����ꍇ:  56 (�A�C�R��1��16 x 2 + ���Ԃ�[:8 x 3)

						   //->[���_] �������^�X�N�o�[�̏ꍇ�̓^�X�N�o�[�������ς�邾���A

	adjustWin11TrayYpos = 0;

	if ((typeWin11Taskbar == 0) && (Win11Type < 2))	//���W�X�g����TaskbarSi�̐ݒ�̓r���h22579����(Win11Type == 1)�łȂ��Ɩ���
	{
		widthWin11Button = heightTaskbar * 3 / 4;
		if (bAdjustTrayWin11SmallTaskbar) adjustWin11TrayYpos = heightTaskbar / 4;
	}
	else
	{
		widthWin11Button = heightTaskbar / 2;		//�W���̍����̃^�X�N�o�[�̏ꍇ
	}

	widthWin11Icon = widthWin11Button * 4 / 3;

	SetMyRegLong("Status_DoNotEdit", "Win11IconSize", widthWin11Icon);	//�ݒ�̍ۂ̏���l���߂邽�߂Ƀt�@�C���ɕۑ�����B

//	defaultWin11ClockWidth = widthWin11Button * 11 / 3 - (widthWin11Button / 15) + adjustWin11ClockWidth;		//(widthWin11Button / 15)�͌v�Z��̌덷�̋z���p�B�����Ŏ蓮�����l��K�p����B

	//Ver4.2.1�ȍ~
	defaultWin11ClockWidth = widthWin11Button * 11 / 3 + adjustWin11TrayCutPosition;
	defaultWin11NotificationWidth = widthWin11Button * 7 / 6;

	if (b_DebugLog) {
		writeDebugLog_Win10("[for_win11.c][GetWin11ElementSize] (From [Taskbar Height]) widthWin11Button = ", widthWin11Button);
		writeDebugLog_Win10("[for_win11.c][GetWin11ElementSize] (From [Taskbar Height]) widthWin11Icon = ", widthWin11Icon);
		writeDebugLog_Win10("[for_win11.c][GetWin11ElementSize] (From [Taskbar Height]) defaultWin11ClockWidth = ", defaultWin11ClockWidth);
	}


	if (bEnabledWin11Notify)
	{
//		widthWin11Notify = widthWin11Button * 12 / 10;		//����͎����Ō��߂Ă悢�l
		widthWin11Notify = widthWin11Icon;		//����͎����Ō��߂Ă悢�l
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

		LoadBitMapWin11Notify();		//�ʒm�A�C�R���T�C�Y�����܂�Ȃ��ƃ��[�h�ł��Ȃ��B


		//�ʒm���\���p�̃t�H���g�쐬�E�ʒu����
		posNotifyText.x = widthNotifyIcon / 2;
		if (typeWin11Taskbar == 0)	//�������^�X�N�o�[�̏ꍇ
		{
			posNotifyText.y = heightWin11Notify * 3 / 4;
			CreateMyNotificationFont(heightWin11Notify / 2, widthNotifyIcon / 5);
		}
		else	//�������^�X�N�o�[�łȂ��ꍇ
		{
			posNotifyText.y = heightWin11Notify * 7 / 10;
			CreateMyNotificationFont(heightWin11Notify * 2 / 5, widthNotifyIcon / 5);
		}


	}
	else {		//�g��Ȃ��ꍇ�͒P�ɃT�C�Y���[���ɂ��ăE�B���h�E�͎c���Ă����B
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


	tempDeskotopHwnd = GetDesktopWindow();	//GetDesktopWindow()���f�X�N�g�b�v
	
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



//	DesktopDirectDraw_Win11();		//�Ƃ肠����TClock�������I�Ƀf�X�N�g�b�v�ɕ\������B

/*
0��Ԃ��������15(WM_PAINT)�����X�����ă��[�v����B�����Ȃ��Ƃ�15��oldProcTaskbarContentBridge_Win11�ɓn���Ȃ���΂����Ȃ��B

���b�Z�[�W(�W���߂�l��)

�E�C���h�E�T�C�Y�����܂ނ���
70		WM_WINDOWPOSCHANGING
131		WM_NCCALCSIZE
71		WM_WINDOWPOSCHANGED
5		WM_SIZE


�E�B���h�E�T�C�Y�����܂܂Ȃ�����
133	 	WM_NCPAINT
15		WM_PAINT (�ʂ���, BeginPaint&EndPaint���Ȃ��ƌĂ΂ꑱ����)
20 		WM_ERASEBKGND
26		WM_WININICHANGE
24		WM_SHOWWINDOW	�V�X�e���ɂ��^�X�N�o�[�T�C�Y���߂��ꂽ�Ƃ���1��͂��BSetMainClockOnTaskTray_Win11�ɂ��^�X�N�o�[�Z�k�ł͏o�Ȃ����߃��[�v���Ȃ��B
528		WM_PARENTNOTIFY

�E�B���h�E�T�C�Y�����܂ނ��s���Ȃ���
738		�Ǝ����b�Z�[�W�H��ʃX�P�[�����O��ύX����Ɠ͂������i�͓͂��Ȃ�
739		�Ǝ����b�Z�[�W�H��ʃX�P�[�����O��ύX����Ɠ͂������i�͓͂��Ȃ�

*/

	//���낢��u���b�N�����݂����c
	//71���~�߂�Ƃ��܂��T�C�Y�������ł��Ȃ��Ȃ�B
	//15���~�߂�Ɩ������[�v��NG
	//����ȊO�͎~�߂Ă��A����̃A���S���Y���ł͓��Ɍ��ʂȂ��B
	//�f�X�N�g�b�v���ڕ`��ł�������P���݂��邪���S�ł͂Ȃ��B

	//�T�C�Y���߂̍Ō�́A
	//WM_WINDOWPOSCHANGED(71)���͂�
	//oldProcTaskbarContentBridge_Win11�̓��삩��WM_SIZE(5)����������
	//WM_SIZE�ɑ΂���oldProcTaskbarContentBridge_Win11�̓��삪����������߂��Ă���WM_WINDOWPOSCHANGED���I������B
	//���̎��_�ŃT�C�Y�������������Ă���̂ŁATClock�p�̈ړ����s���B���������ڌĂ�ł͑�������̂ŁA
	//PostMessage(hwndClockMain, CLOCKM_MOVEWIN11CONTENTBRIDGE, 0, 0);�ŌĂ�

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

	//		if ((message == 71))	//WM_WINDOWPOSCHANGED, ������u���b�N����Ƃ��܂������Ȃ�
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

	//if (message == 15) {		//WM_PAINT��BeginPaint/EndPaint���Ȃ��ƌĂ΂ꑱ����̂ł���ŃX�L�b�v����B
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
		else if ( tempWidth == originalWidthTaskbar)	//�z�u���߂��ꂽ���A�̃v���Z�X�̍Ō�ɐ�������B���ۂɂ�originalWidthTaskbar�ɖ߂邪�A����ȊO�̃P�[�X�ɂ��Ή��ł���
		{
			if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][WndProcTaskbarContentBridge_Win11] SetMainClockOnTaskTray_Win11(operation = 0) called due to ContentBridge resize, Size =", (int)(*pwinpos).cx);
			SwitchToTClockBarWin11();
			PostMessage(hwndClockMain, CLOCKM_MOVEWIN11CONTENTBRIDGE, 0, 0);	//Postmessage�o�R��SetMainClockOnTaskTray_Win11�����s����B
//			SetMainClockOnTasktray_Win11();			//����ȕ��ɒ��ڌĂԂƑ������Ă����->���s�A�ɂȂ�B
		}
		else 
		{
			if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][WndProcTaskbarContentBridge_Win11] SetMainClockOnTaskTray_Win11(operation = 1) called due to ContentBridge resize, Size =", (int)(*pwinpos).cx);
			PostMessage(hwndClockMain, CLOCKM_MOVEWIN11CONTENTBRIDGE, 1, 0);	//Postmessage�o�R��SetMainClockOnTaskTray_Win11�����s����B
		}

	}
	else if (message == 70)	//�ʒm�����ς��ƂȂ������ꂪ�͂��悤�Ȃ̂ŁA�ʒm�����ω����Ă��Ȃ����`�F�b�N����B
	{
		if (SetModifiedWidthWin11Tray())	//���̊֐��͒ʒm�A�C�R���̗L�����ω�������TRUE�ɂȂ�B
		{
			if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][WndProcTaskbarContentBridge_Win11] SetMainClockOnTaskTray_Win11 called due to Notification change.", 999);
			PostMessage(hwndClockMain, CLOCKM_MOVEWIN11CONTENTBRIDGE, 0, 0);	//Postmessage�o�R��SetMainClockOnTaskTray_Win11�����s����B
			//			SetMainClockOnTasktray_Win11(); ����ȕ��ɒ��ڌĂԂƃ^�C�~���O������Ă��܂������Ȃ�
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

//	DesktopDirectDraw_Win11();		//�O�̂���TClock�������I�Ƀf�X�N�g�b�v�ɕ\������B


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
	//���̃R�[�h���ł�hwnd��hwndTrayMain�ł���A���̑����̏ꍇ(hwnd=hwndClockMain)�ƈقȂ�Ȃ̂Œ��ӂ��邱�ƁI
	//�^�X�N�g���C�̓��C���̃^�X�N�o�[�ɂ������݂��Ȃ��̂ŁA����ȊO�ɂ͎g���Ȃ��R�[���o�b�N�֐��ɂȂ��Ă���B

//	if (b_DebugLog) writeDebugLog_Win10("[for_win11.c][SubclassTrayProc_Win11] Window Message was recevied, message = ", message);



	//Win11�ł́ATClock�̓^�X�N�g���C�̊O�ɂ���B
	//���̃v���V�[�W���ł́A�^�X�N�g���C�̖{���̒������擾���A���̂����E�̎��v��؂藎�Ƃ����߂̑�����s���Ă���B

	switch (message)
	{
		case (WM_USER + 100):	//1124
		{
			if (b_DebugLog)writeDebugLog_Win10("[fow_win11.c][SubclassTrayProc_Win11] WM_USER + 100 (1124) recevied.", 999);
			//Win10�̏ꍇ
			// �Ĕz�u�O�ɐe�E�B���h�E���瑗���A�T�C�Y��Ԃ����b�Z�[�W�B
			// DefSubClassTrayProc()���ĂԂ�LRESULT�`���ŁAWindows�W�����v���������ꍇ�̃T�C�Y���A���Ă���̂ŁA
			// ���������ꍇ�̃T�C�Y�ɍ����ւ��Ė߂��B
			// �������l��Ԃ��Ȃ��ƃ^�X�N�g���C�̃T�C�Y�����������Ȃ�B

			//Win11�̏ꍇ
			//IME�A�C�R����荶�̃A�C�R�����ύX�����ꍇ�ɂ� WININICHANGE������s����B
			//���̂��ƁAIME�A�C�R���̕ύX�̏ꍇ���A�ʒu���A�C�R���\���̏ꍇ���A
			//WM_USER+100 ��1�� -> WM_NCCALC(131) ��2��͂�


			LRESULT ret;


			ret = DefSubclassProc(hwndTrayMain, message, wParam, lParam);

			//����œ�����̂�Button + Pager�̕��ł���A�W�����v�ƃl�b�g���̃A�C�R���ނ̕��͊܂܂�ĂȂ��悤���B
			//�\�t�g�E�F�A�L�[�{�[�h�A�C�R�����́A�N�����l���������܂ܔ��f����Ȃ�(�Ȃ����͕s��)�B
			//�Ȃ̂ł��܂�g���Ȃ��B

			//Win10�̏ꍇ�ƈقȂ�A�����ŉ��ς�������Ԃ��Ă͂����Ȃ��B�g���C���Ĕz�u���N�����ăA�C�R�����d�Ȃ��Ă��܂��B
			//���v��؂藎�Ƃ��̂́ASetMainClockOnTasktray_Win11�ōs���B


			//�Ĕz�u�ɐ悾���ĕK���Ă΂��킯�ł͂Ȃ��B�����ɏ��������Ă��m���Ɏ��s����Ȃ�

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

				//�Ĕz�u�ɐ悾���ĕK���Ă΂��͗l�B

				//���̃��b�Z�[�W�ł́A�w�����炱�̒l�ɕύX���悤�Ƃ���x�T�C�Y���擾�ł���B���ꂪmodifiedWidthWin11Tray�ƈقȂ�ꍇ�ɂ͋����I�ɖ߂���悤�Ƃ��Ă��邱�ƂɂȂ邪�A
				//���̃T�C�Y�͌���̃g���C�̍\���̕��Ƃ������ƂɂȂ�̂ŁAorigWidthWin11Tray�̃A�b�v�f�[�g�Ɏg����B


				if (newWidth != modifiedWidthWin11Tray) {	//modifiedWidthWin11Tray�ւ̕ύX�ɔ����ė����ꍇ�ɂ͖�������B
					origWidthWin11Tray = newWidth;
					SetModifiedWidthWin11Tray();
				}
				if (b_DebugLog)
				{
					writeDebugLog_Win10("[for_win11.c][SubclassTrayProc_Win11] origWidthWin11Tray =", origWidthWin11Tray);
				}

			}

			//Small Taskbar�̃A�C�R��������}��
//			ShowWindow(hwndWin11InnerTrayContentBridge, SW_HIDE);

			break;
		}
		//case WM_PAINT:
		//case WM_NCPAINT:
		//{
		//	//ContentBridge��SetMainClock...�ňړ����Ă��Ĕz�u����Ă��܂��B
		//	if (bTokenMoveContentBridge) {
		//		bTokenMoveContentBridge = FALSE;
		//		MoveWin11ContentBridge();
		//	}
		//	break;
		//}
		case WM_NOTIFY:		//78
		{
			// �Ĕz�u������������e�E�B���h�E���瑗����B
			// DefSubClassTrayProc()���ĂԂ�LRESULT�`���ŕԓ����ׂ��R�[�h�A���Ă���̂ŁA���̂܂ܖ߂���OK�̂悤���B
			// ���̎��_�Ŏ��v�͋����I�ɕW��Windows���v�̃T�C�Y�ɕύX����āA�^�X�N�g���C�����ɃA�C�e�������񂾏��(�g���C�̃T�C�Y���̂�WM_USER+100�ɐ������Ԃ��Ă���Ίm�ۂ���Ă���)�B
			// hwndClockMain�̃T�C�Y��������T�C�Y�ɏC�����āA�ʒm�̈�Ȃǂ̏ꏊ���C������K�v����(SetMainClockOnTasktray���ĂԁB

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
			ret = DefSubclassProc(hwndTrayMain, message, wParam, lParam);	//hwnd�𖾎��������A�@�\�͓����B
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

	//���̃R�[���o�b�N�֐���hwndWin11Notification�Ƃ̑g�ݍ��킹�łł������������삵�Ȃ��̂ŁA�����Ƃ���hwnd = hwndWin11Notify�����R�[���o�b�N�֐��Ȃ̂�tempHwnd�ŏ������Ă���


//	if (b_DebugLog) writeDebugLog_Win10("[for_win11.c][WndProcWin11Notify] Window Message was recevied, message = ", message);


	switch (message) //Win11�^�X�N�g���C�ʒm�A�C�R�����ɑ����]��
	{
		case WM_LBUTTONDOWN:
			//�����ɁA�����ꂽ�ꏊ��posXShowDesktopArea���E�Ȃ�f�X�N�g�b�v��\������A�Ƃ����̂���������*****
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
			PostMessage(hwndTClockExeMain, WM_CONTEXTMENU, wParam, (LPARAM)MAKELONG(pos.x, pos.y));		//�������ꏊ�Ƀ��j���[���o�����߂Ɉʒu������ʏ�̐�΍��W�ɕς��đ���K�v������B
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

	//	if (fillbackWin11NotifyIconInvert)	//fillbackcolor�ŐF���Â��Ƃ��̂݁A�w�i�𔒂ɔ��]����B
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

		if (fillbackWin11NotifyIconInvert)	//fillbackcolor�ŐF���Â��Ƃ��̂݁A�w�i�𔒂ɔ��]����B
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

	if (!hdc)	//bWin11Main�͎���(�Ă΂��O�ɔ���ς�)�B
	{	//�ʒm�E�B���h�E��������Ă������蒼���B
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

			if (Win11Type == 2) {		//Win11Type2�ł̏�[���C���Č�
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


BOOL SetModifiedWidthWin11Tray(void)	//�߂�l�̓A�C�R���̗L�����ω��������ǂ����F�ω�������TRUE
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


//Win11�Ń��C���N���b�N�̔z�u�Ǝ��Ӓ���
void SetMainClockOnTasktray_Win11(void)
{

	//���̊֐���Win11Type < 2 (�r���h22579����)�ł�SubClassTrayProc_Win11��WM_NOTIFY���炵���Ă΂�Ȃ�(��Ԃ��ێ����邱�ƁI)�B
	//���̏ꍇCLOCKM_MOVEWIN11CONTENTBRIDGE�͂��̊֐����瓊����B
	//����AWin11Type == 2�ł́A���C���N���b�N�E�B���h�E�֓�����CLOCKM_MOVEWIN11CONTENTBRIDGE���b�Z�[�W�o�R�ł�����ɗ���B�Ă΂�鏇�Ԃ��t�Ȃ̂ł��ꂮ������ӂ���B
	//Win11Type == 2��CLOCKM_MOVEWIN11CONTENTBRIDGE���b�Z�[�W�𓊂���Ɩ������[�v����I


	//Win11�ɂ�����^�X�N�g���C���Ĕz�u�̕��@
	//Win11�ł�TClockWin11�Ƃ����N���X���^�X�N�o�[�����ɍ���āA�����烏�[�N�A�C�R���Q�A�g���C�G���A(���Ɉړ�)�ATClock�̏��ɕ��ׂ��Ԃ��ێ�����B


	if (b_DebugLog) writeDebugLog_Win10("[for_win11.c] SetMainClockOnTasktray_Win11 called. ", 999);
	char tempClassName[32];

	//�m�ۂ��ׂ����v�̃T�C�Y���擾
	CalcMainClockSize();

	//�^�X�N�o�[�����擾(Win11�͂߂����ɕς��Ȃ����A�O�̂��߁B����͕K�v)
	GetTaskbarSize();

	//Win11�^�X�N�g���C�؂藎�Ƃ���������
	SetModifiedWidthWin11Tray();


	if (Win11Type == 2)
	{
		
		//�^�X�N�o�[��Z�k����B
		//SWP_NOSENDCHANGING�����Ă����Ȃ��ƃ^�X�N�o�[�͈ړ����Ȃ�(�����I�ɖ߂����̂��H)
		//SWP_NOTOPMOST�����Ă�������TClock��O�ɏo�����A����͂�����TOPMOST�ɂȂ�B
		modifiedWidthTaskbar_Win11Type2 = originalWidthTaskbar - widthMainClockFrame - widthWin11Notify + cutOffWidthWin11Tray;
		SetWindowPos(hwndTaskBarMain, NULL, 0, 0, modifiedWidthTaskbar_Win11Type2, heightTaskbar,
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING);

		//TClock�̍��[��X���W�����߂�B
		posXMainClock = originalWidthTaskbar - widthMainClockFrame - widthWin11Notify;

		SetWindowPos(hwndWin11ContentBridge, NULL, 0, 0, posXMainClock, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_HIDEWINDOW);

		//TClockBarWin11��ݒ肷��B
		SetWindowPos(hwndTClockBarWin11, hwndTaskBarMain, 0, posYTaskbar, originalWidthTaskbar, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);

		//TClock�̃E�B���h�E������̏ꏊ�Ɉړ�����B
		//SetWindowPos(hwndClockMain, HWND_TOPMOST, posXMainClock, 0, widthMainClockFrame, heightMainClockFrame,
		//	SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
		SetWindowPos(hwndClockMain, NULL, posXMainClock, 0, widthMainClockFrame, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);

		//����ʒm�E�B���h�E�̏ꏊ���Đݒ肷��B
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

		//�g���C���ATClock�̃E�B���h�E�����Ɉړ����āA�E���̎��v�̈悪�����Ȃ��Ȃ�悤�Ƀ��T�C�Y����B
		SetWindowPos(hwndTrayMain, NULL, tempX, 0, modifiedWidthWin11Tray, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);


		//�w�������^�X�N�o�[�x�̏ꍇ�Ƀg���C�A�C�R������ɃV�t�g����B
		//ContentBridge�̑���͂����ł���Ă��㏑�������͗l�B������h�~�̂��߂Ɉꎞ�I�ɏ������Ƃ͂ł����B
		//���̂ق��ɋL�q����Postmessage�o�R��MoveWin11ContentBridge�Œ�������B
		//if (adjustWin11TrayYpos != 0)
		//{
		//	SetWindowPos(hwndWin11InnerTrayContentBridge, NULL, 0, - adjustWin11TrayYpos, modifiedWidthWin11Tray, heightMainClockFrame + adjustWin11TrayYpos,
		//		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_HIDEWINDOW);
		//}
		if (adjustWin11TrayYpos != 0) ShowWindow(hwndWin11InnerTrayContentBridge, SW_HIDE);


		//�A�v���A�C�R������A�g���C�̍��[�܂łɃ��T�C�Y����
		SetWindowPos(hwndWin11ReBarWin, NULL, 0, 0, tempX, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);

		//TClock�̍��[��X���W�����߂�B
		posXMainClock = widthTaskbar - widthMainClockFrame - widthWin11Notify;

		//TClock�̃E�B���h�E������̏ꏊ�Ɉړ�����B
		SetWindowPos(hwndClockMain, NULL, posXMainClock, 0, widthMainClockFrame, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);


		//��ԏ��ContentBridge�N���X(2���邪�A�g���C�̒��ɂ���ق��ł͂Ȃ��A�^�X�N�o�[����)�����T�C�Y����B
		//ContentBridge�̑���͂����ł���Ă��㏑�������͗l�B
		//�����ł͎��s�����A���̂ق��ɋL�q����Postmessage�o�R��MoveWin11ContentBridge�Œ�������B
		//SetWindowPos(hwndWin11ContentBridge, NULL, 0, 0, posXMainClock, heightMainClockFrame,
		//	SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOREDRAW | SWP_DEFERERASE);

		//����ʒm�E�B���h�E�̏ꏊ���Đݒ肷��B
		if (hwndWin11Notify) {
			SetWindowPos(hwndWin11Notify, NULL, posXMainClock + widthMainClockFrame, 0, widthWin11Notify, heightMainClockFrame,
				SWP_NOACTIVATE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
			ShowWindow(hwndWin11Notify, SW_SHOW);
		}

		//ContentBridge�̑���͂����ł���Ă��㏑�������͗l�B
		//���炽�߂�SubClassTrayProc_Win11��WM_NCPAINT�������^�C�~���O�ōs���B
		//�c��������ł͏������^�X�N�o�[�ɔ����A�C�R��������͎c���Ă��܂��̂ŁA���̃f�B���C�����ɂ���K�v������B
		//	bTokenMoveContentBridge = TRUE;

		//ContentBridge�̈ړ��͏����x��čs���K�v������̂ŁA������ł܂Ƃ߂Ď��s����B
		PostMessage(hwndClockMain, CLOCKM_MOVEWIN11CONTENTBRIDGE, 0, 0);	//�^�C�}�[���g���قǂ̃f�B���C�͕s�v�ŁAPostmessage�o�R�̎��s��OK
	}



	//�T�C�Y�X�V������AhdcClock����蒼���悤�ɂ���B
	CreateClockDC();


	//������RedrawMainTaskbar���Ă�ł͂����Ȃ��I(���Ԃ�)�B���[�v����������B
	//InvalidateRect�ł̍ĕ`��ł̓��[�v���Ȃ����A������̌��Ȃ̂Ŕ�����B
//		InvalidateRect(hwndTrayMain, NULL, TRUE);	
//	InvalidateRect(hwndTaskBarMain, NULL, TRUE);

	//	SetAllSubClocks();	//���C���N���b�N�̏�Ԃ��ς������A����T�u�N���b�N�����f������K�v����B
	//->�����Ɏ��s����Ƃ��܂��s���Ȃ����������J��Ԃ����̂Ńf�B���C�Ŏ��s

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


void MoveWin11ContentBridge(int operation)	//Win11 Type2 (build 22579����т���ȍ~)�ł�hwndWin11ContentBridge�̏�ɃA�C�R�����ڂ�悤�ɂȂ��Ă��āA���̏����ł͌��A�C�R���Q���؂�Č����Ȃ��Ȃ�B
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
			//���߂Ĉړ����āA�\������B
			SetWindowPos(hwndWin11InnerTrayContentBridge, NULL, 0, -adjustWin11TrayYpos, modifiedWidthWin11Tray, heightMainClockFrame + adjustWin11TrayYpos,
				SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);	// | SWP_SHOWWINDOW
																		//InvalidateRect(hwndTrayMain, NULL, TRUE);
		}

		ShowWindow(hwndWin11InnerTrayContentBridge, SW_SHOW);
	}

}

