#include "tcdll.h"
#include "resource.h"









//��Win11��p
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

//Win11�Ή��֘A
//Win11�Ή�����E�B���h�E���[�h�t���O
extern BOOL bWin11Main;
extern BOOL bWin11Sub;

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


void CreateWin11MainClock(void)
{

	//�ŏ��Ɏc�[���c���Ă�������B
	HWND tempHwnd;
	while ((tempHwnd = FindWindowEx(hwndTaskBarMain, NULL, "TClockMain", NULL)) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);


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

	hwndClockMain = CreateWindowEx(WS_EX_TOPMOST, szClassName, "TClockMain",
		WS_VISIBLE | WS_CHILD, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		hwndTaskBarMain, NULL, hmod, NULL);

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
	while ((tempHwnd = FindWindowEx(hwndTaskBarMain, NULL, "TClockNotify", NULL)) != NULL) PostMessage(tempHwnd, WM_CLOSE, 0, 0);

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
	bmi.bmiHeader.biWidth = widthNotifyIcon;
	bmi.bmiHeader.biHeight = heightNotifyIcon;


	//�ʒm����A�C�R��DC�쐬
	//DIB section �쐬, �ŏ��ƍŌ�̃A�h���X�擾
	hbm_DIBSection_YesWin11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_YesWin11Notify_start, NULL, 0);
	m_color_YesWin11Notify_end = m_color_YesWin11Notify_start + (widthNotifyIcon * heightNotifyIcon);
	SelectObject(hdcYesWin11Notify, hbm_DIBSection_YesWin11Notify);

	//���\�[�X�r�b�g�}�b�v(��/����)��g�ɂ��킹�ăR�s�[�B�g��GetWin11ElementSize�Ō��߂�)
	hdcYesWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcYesWin11Notify, hbm_DIBSection_YesWin11Notify);
	SetStretchBltMode(hdcYesWin11Notify, HALFTONE);

	tempDC = CreateCompatibleDC(hdc);
	SelectObject(tempDC, hbmpIconYesWin11Notify);

	GetObject(hbmpIconYesWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcYesWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);

	//���[�h�����r�b�g�}�b�v�͂�������Ȃ��̂Ŕj��
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
	}

	//�ʒm�Ȃ��A�C�R��DC�쐬
	//DIB section �쐬, �ŏ��ƍŌ�̃A�h���X�擾
	hbm_DIBSection_NoWin11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_NoWin11Notify_start, NULL, 0);
	m_color_NoWin11Notify_end = m_color_NoWin11Notify_start + (widthNotifyIcon * heightNotifyIcon);
	SelectObject(hdcNoWin11Notify, hbm_DIBSection_NoWin11Notify);

	//���\�[�X�r�b�g�}�b�v(��/����)��g�ɂ��킹�ăR�s�[�B�g��GetWin11ElementSize�Ō��߂�)
	hdcNoWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcNoWin11Notify, hbm_DIBSection_NoWin11Notify);
	SetStretchBltMode(hdcNoWin11Notify, HALFTONE);

	DeleteDC(tempDC);
	tempDC = CreateCompatibleDC(hdc);
	SelectObject(tempDC, hbmpIconNoWin11Notify);

	GetObject(hbmpIconNoWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcNoWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);

	//���[�h�����r�b�g�}�b�v�͂�������Ȃ��̂Ŕj��
	DeleteObject(hbmpIconNoWin11Notify);

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
	}

	//�W�����[�h�A�C�R��DC�쐬
	//DIB section �쐬, �ŏ��ƍŌ�̃A�h���X�擾
	hbm_DIBSection_FocusWin11Notify = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&m_color_FocusWin11Notify_start, NULL, 0);
	m_color_FocusWin11Notify_end = m_color_FocusWin11Notify_start + (widthNotifyIcon * heightNotifyIcon);
	SelectObject(hdcFocusWin11Notify, hbm_DIBSection_FocusWin11Notify);

	//���\�[�X�r�b�g�}�b�v(��/����)��g�ɂ��킹�ăR�s�[�B�g��GetWin11ElementSize�Ō��߂�)
	hdcFocusWin11Notify = CreateCompatibleDC(hdc);
	SelectObject(hdcFocusWin11Notify, hbm_DIBSection_FocusWin11Notify);
	SetStretchBltMode(hdcFocusWin11Notify, HALFTONE);

	tempDC = CreateCompatibleDC(hdc);
	SelectObject(tempDC, hbmpIconFocusWin11Notify);

	GetObject(hbmpIconFocusWin11Notify, sizeof(BITMAP), &tempBitmap);

	StretchBlt(hdcFocusWin11Notify, 0, 0, widthNotifyIcon, heightNotifyIcon, tempDC, 0, 0, tempBitmap.bmWidth, tempBitmap.bmHeight, SRCCOPY);

	//���[�h�����r�b�g�}�b�v�͂�������Ȃ��̂Ŕj��
	DeleteObject(hbmpIconFocusWin11Notify);

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

	if (typeWin11Taskbar == 0)
	{
		widthWin11Button = heightTaskbar * 3 / 4;	//�w�������^�X�N�o�[�x�̏ꍇ
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

	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][GetWin11TrayWidth] origWidthWin11Tray = ", origWidthWin11Tray);
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
		case (WM_USER + 100):
		{
			if (b_DebugLog)writeDebugLog_Win10("[fow_win11.c][SubclassTrayProc_Win11] WM_USER + 100 (1124) recevied.", 999);
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


			//�Ĕz�u�ɐ悾���ĕK���Ă΂��킯�ł͂Ȃ��B�����ɏ��������Ă��m���Ɏ��s����Ȃ��B

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
		case WM_NOTIFY:
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
	//message�́A���^�]����WM_TIMER(275)�ȊO�͂߂����ɗ��Ȃ��B

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

	//cutOffWidthWin11Tray = origWidthWin11Tray - pos.x;	//cutOffWidthWin11Tray���E����ؒf���ׂ�����=���v�������Ƃ��Čv�Z���Ă����B
	//if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][SetModifiedWidthWin11Tray] TrayInput X =", pos.x);



	////�܂��A�Œ蕪�������Ă����B

	////�蓮���ߕ��������BIME�A�C�R���͒ʏ�A�C�R���Ɠ������Ȃ̂Ō�Ŏ����ň������B
	//cutOffWidthWin11Tray -= adjustWin11TrayCutPosition;



	////WiFi+�X�s�[�J�[+�o�b�e���A�C�R�����������B
	//if (b_BatteryLifeAvailable) {
	//	//�V�X�e���Ƀo�b�e�������݂���ꍇ�ɂ�WiFi+�X�s�[�J�[+�o�b�e����3�A�C�R���Ɛ��������̂ł���Ƀ{�^��10/3��(�����l)���������B
	//	//�A�C�R��1��widthWin11Button *2/3, ���Ԃ�widthWin11Button * 1/3
	//	cutOffWidthWin11Tray -= widthWin11Button * 10 / 3;
	//}
	//else
	//{
	//	//�f�X�N�g�b�v�ȂǁAWiFi�A�C�R������2�����Ȃ��\���̏ꍇ��7 / 3 ��(�����l)���������B
	//	cutOffWidthWin11Tray -= widthWin11Button * 7 / 3;
	//}
	////���̒l�͐��m�Ȓl��I�ԕK�v������B2021/11�����݂̋�̓I�Ȓl��GetWin11ElementSize�֐��̃R�����g�ɋL��


	////�����܂ň�������AcutOffWidthWin11Tray��
	////�ŏ��̎��v�̕�
	////�E���̒ʒm�}�[�N(�{�^����x1.2��)
	////IME�A�C�R���A�ʒu����\�t�g�L�[�{�[�h�Ȃǂ̗Վ��A�C�R��(�A�C�R���� = widthWin11Icon)

	////�A�C�R����(�{�^��x4/3)�P�ʂŉE�����������炵�Ă����B
	//while ((cutOffWidthWin11Tray - defaultWin11ClockWidth) >= widthWin11Icon)
	//{
	//	cutOffWidthWin11Tray -= widthWin11Icon;		//�E���̒�������A�C�R��1���Z������B
	//}

	////�c���������́A���v(+�ʒm)�̕��������ɂȂ�̂ŁA���̒�������Ђ��΂��܂������͂��B
	//modifiedWidthWin11Tray = origWidthWin11Tray - cutOffWidthWin11Tray;

	//if (b_DebugLog)
	//{
	//	writeDebugLog_Win10("[for_win11.c][SetModifiedWidthWin11Tray] A: origWidthWin11Tray =", origWidthWin11Tray);
	//	writeDebugLog_Win10("[for_win11.c][SetModifiedWidthWin11Tray] defaultWin11ClockWidth =", defaultWin11ClockWidth);
	//	writeDebugLog_Win10("[for_win11.c][SetModifiedWidthWin11Tray] B: cutOffWidthWin11Tray =", cutOffWidthWin11Tray);
	//	writeDebugLog_Win10("[for_win11.c][SetModifiedWidthWin11Tray] A-B: modifiedWidthWin11Tray =", modifiedWidthWin11Tray);
	//}

	//tempDetectionPoint = widthWin11Button / 2;	//���̒l�͎��ۂɂ́@�{�^���� x 7/6�ȉ��Ȃ���ʂ͓����͂��BadjustWin11DetectNotify�͗]��Ӗ����Ȃ��B
	//if (cutOffWidthWin11Tray > (defaultWin11ClockWidth + tempDetectionPoint - adjustWin11DetectNotify))		//�����ŕ��i���L�ӂɒ��������肷��B
	//{
	//	bExistWin11Notify = TRUE;
	//}
	//else
	//{
	//	bExistWin11Notify = FALSE;
	//}

	//	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c][SetModifiedWidthWin11Tray] bExistWin11Notify is speculated to be ", bExistWin11Notify);



	//Ver 4.2.1�ȍ~�͒ʒm�����擾�ł���悤�ɂȂ����B
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


//Win11�Ń��C���N���b�N�̔z�u�Ǝ��Ӓ���
void SetMainClockOnTasktray_Win11(void)
{

	//���̊֐���SubClassTrayProc_Win11��WM_NOTIFY���炵���Ă΂�Ȃ�(��Ԃ��ێ����邱�ƁI)�B


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
	SetWindowPos(hwndClockMain, HWND_TOPMOST, posXMainClock, 0, widthMainClockFrame, heightMainClockFrame,
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
//	ShowWindow(hwndClockMain, SW_SHOW);



	//��ԏ��ContentBridge�N���X(2���邪�A�g���C�̒��ɂ���ق��ł͂Ȃ��A�^�X�N�o�[����)�����T�C�Y����B
	//ContentBridge�̑���͂����ł���Ă��㏑�������͗l�B
	//�����ł͎��s�����A���̂ق��ɋL�q����Postmessage�o�R��MoveWin11ContentBridge�Œ�������B
	//SetWindowPos(hwndWin11ContentBridge, NULL, 0, 0, posXMainClock, heightMainClockFrame,
	//	SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOREDRAW | SWP_DEFERERASE);

	//����ʒm�E�B���h�E�̏ꏊ���Đݒ肷��B
	if (hwndWin11Notify) {
		SetWindowPos(hwndWin11Notify, HWND_TOPMOST, posXMainClock + widthMainClockFrame, 0, widthWin11Notify, heightMainClockFrame,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
		ShowWindow(hwndWin11Notify, SW_SHOW);
	}

	//ContentBridge�̑���͂����ł���Ă��㏑�������͗l�B
	//���炽�߂�SubClassTrayProc_Win11��WM_NCPAINT�������^�C�~���O�ōs���B
	//�c��������ł͏������^�X�N�o�[�ɔ����A�C�R��������͎c���Ă��܂��̂ŁA���̃f�B���C�����ɂ���K�v������B
//	bTokenMoveContentBridge = TRUE;

	//ContentBridge�̈ړ��͏����x��čs���K�v������̂ŁA������ł܂Ƃ߂Ď��s����B
//	DelayedMoveWin11ContentBridge();
	PostMessage(hwndClockMain, CLOCKM_MOVEWIN11CONTENTBRIDGE, 0, 0);	//�^�C�}�[���g���قǂ̃f�B���C�͕s�v�ŁAPostmessage�o�R�̎��s��OK

	//�T�C�Y�X�V������AhdcClock����蒼���悤�ɂ���BCreateClockDC�͂�������ĂԂ����ŕK�v�[���Ȃ͂��B
	CreateClockDC();


	//������RedrawMainTaskbar���Ă�ł͂����Ȃ��I(���Ԃ�)�B���[�v����������B
	//InvalidateRect�ł̍ĕ`��ł̓��[�v���Ȃ����A������̌��Ȃ̂Ŕ�����B
//		InvalidateRect(hwndTrayMain, NULL, TRUE);	
//	InvalidateRect(hwndTaskBarMain, NULL, TRUE);

	//	SetAllSubClocks();	//���C���N���b�N�̏�Ԃ��ς������A����T�u�N���b�N�����f������K�v����B
	//->�����Ɏ��s����Ƃ��܂��s���Ȃ����������J��Ԃ����̂Ńf�B���C�Ŏ��s
	//Ver 4.0.3�ł͎����I�ɂ͉����N����Ȃ��B
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
//	SetTimer(hwndClockMain, IDTIMERDLL_MOVEWIN11CONTENTBRIDGE, 10, NULL);		//10ms��ɓ���->���ۂɂ̓^�C�}������Ȃ���Postmessage��OK�������B
//}


void MoveWin11ContentBridge(void)
{
	if (b_DebugLog)writeDebugLog_Win10("[for_win11.c] MoveWin11ContentBridge called.", 999);
	SetWindowPos(hwndWin11ContentBridge, NULL, 0, 0, posXMainClock, heightMainClockFrame,
		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOREDRAW | SWP_DEFERERASE);

	if (adjustWin11TrayYpos != 0)
	{
		//���߂Ĉړ����āA�\������B
		SetWindowPos(hwndWin11InnerTrayContentBridge, NULL, 0, - adjustWin11TrayYpos, modifiedWidthWin11Tray, heightMainClockFrame + adjustWin11TrayYpos,
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);	// | SWP_SHOWWINDOW
		//InvalidateRect(hwndTrayMain, NULL, TRUE);
	}

	ShowWindow(hwndWin11InnerTrayContentBridge, SW_SHOW);
}

