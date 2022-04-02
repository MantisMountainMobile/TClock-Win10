/*-----------------------------------------------------
  tclock.c
  customize the tray clock
  KAZUBON 1997-2001
-------------------------------------------------------*/
#include "tcdll.h"
#include "resource.h"
#include <math.h>
//#include <physicalmonitorenumerationapi.h>

#include    <wingdi.h>
#include	<shellapi.h>

#pragma     comment(lib,"msimg32.lib")

#define GRADIENT_FILL_RECT_H    0x00000000
#define GRADIENT_FILL_RECT_V    0x00000001
#define MAX_PROCESSOR               64

//�O���t�̋L�^��
#define MAXGRAPHLOG 600
#define MAX_MYCOLORS 16

//IDTIMERDLL_CHECKNETSTAT, IDTIMERDLL_SYSINFO�̃I�t�Z�b�g(ms)	added by TTTT
#define OFFSETMS_TIMER_SYSINFO	200
#define OFFSETMS_TIMER_NETSTAT	500

char Ver_TClockWin10[16];
extern PSTR CreateFullPathName(HINSTANCE hmod, PSTR fname);
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);



//tcdll.h�Ɉړ�
//LRESULT CALLBACK WndProcSubClk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//void GetOrigSubClkDimensions(void);
//void StoreSpecificSubClockDimensions(int i);
//void ActivateSubClocks(void);
//void FindAllSubClocks(void);
//void DisableAllSubClocks(void);
//void DelayedUpdateSubClks(void);
//void DisableSpecificSubClock(int i);
//void SetAllSubClocks(void);
//void SetSpecificSubClock(int i);
//void CheckSubClocks(void);
//int GetSubClkIndexFromHWND(HWND tempHwndSubClk);
//void CalcSpecificSubClockSize(int i);
//BOOL GetInkWorkspaceSetting(void);

void InitClock();
void CreateClockDC(void);
void ReadData(void);
void InitSysInfo(void);
void OnTimer_Win10(void);	//added by TTTT
void OnTimerUpperTaskbar(void);
static void DrawClockFocusRect(HDC hdc);
void DrawClockSub(HDC hdc, SYSTEMTIME* pt, int beat100);
void DrawGraph(HDC hdc, int xclock, int yclock, int wclock, int hclock);
void UpdateSysRes(BOOL bbattery, BOOL bmem, BOOL bnet, BOOL bhdd, BOOL bcpu, BOOL bvol, BOOL bgpu, BOOL btemp);
void GetTimeZoneBias_Win10(void);
void getGraphVal();
void OntimerCheckNetStat_Win10(HWND hwnd); //added by TTTT
void SendStatusDLL2Main(void);
void GetPrevMainClockSize(void);
void CalcMainClockContentSize(void);
void CalcMainClockSize(void);
void RedrawMainTaskbar(void);
void RedrawTClock(void);
void SetTClockFont(void);
void GetTaskbarSize(void);
void RestartOnRefresh(void);
void GetMainClock(void);
void SetMainClockOnTasktray(void);
LRESULT CALLBACK SubclassTrayProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
BOOL IsVertTaskbar(HWND hwndTaskBarMain);
void DrawBarMeter(HWND hwnd, HDC hdc, int wclock, int hclock, int bar_right, int bar_left,
	int bar_bottom, int bar_top, int value, COLORREF color, BOOL b_Horizontal, BOOL b_ToLeft);
void DrawBarMeter2(HWND hwnd, HDC hdc, int wclock, int hclock, int bar_right, int bar_left,
	int bar_bottom, int bar_top, int value, COLORREF color, BOOL b_Horizontal);
void Textout_Tclock_Win10_3(int x, int y, char* sp, int len, int infoval);

COLORREF TextColorFromInfoVal(int infoval);
void ClearGraphData(void);

void GetTaskbarColor_Win11Type2(void);



//tcdll.h�Ɉړ�
//LRESULT CALLBACK SubclassTrayProc_Win11(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
//LRESULT CALLBACK WndProcWin11Notify(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//void CreateWin11MainClock(void);
//void CreateWin11Notify(void);
//void ReCreateWin11Notify(void);
//HWND CreateWin11SubClock(HWND tempHwndTaskbar);
//void DrawWin11Notify(void);
//void LoadBitMapWin11Notify(void);
//void SetMainClockOnTasktray_Win11(void);
//void GetWin11TaskbarType(void);
//void SetModifiedWidthWin11Tray(void);
//void GetWin11ElementSize(void);
//void GetWin11TrayWidth(void);





static BOOL bTooltipTimerStart = FALSE;
extern BOOL bEnableTooltip;



void CheckSafeMode_Win10(void);	// added by TTTT
BOOL b_ExistLTEProfile = FALSE;
BOOL b_ExistMeteredProfile = FALSE;

//////BOOL b_UsageRetrieveInternetProfile = TRUE;

int AdjustThreshold = 200;
BOOL b_FlagTimerAdjust = FALSE;

/*------------------------------------------------
  shared data among processes
--------------------------------------------------*/
#ifdef _MSC_VER
#pragma data_seg("MYDATA")
//dllmain�Ƌ��L����f�[�^(dll�̋��L�f�[�^�H)�͂���data_seg�̒��ɓ����K�v������A�܂��錾���ɏ��������K�v
HWND hwndTClockExeMain = NULL;
HWND hwndTaskBarMain = NULL;

char szShareBuf[81] = { 0 };
#pragma data_seg()
#else
extern HWND hwndTClockExeMain;
extern HWND hwndTaskBarMain;
#endif

HWND hwndTrayMain = NULL;
HWND hwndDesktop = NULL;

//�ȉ��̔z��̏�������FindAllSubClocks�̃��[�v�̒��ōs���B
HWND hwndTaskBarSubClk[MAX_SUBSCREEN];
HWND hwndClockSubClk[MAX_SUBSCREEN];
HWND hwndOriginalWin11SubClk[MAX_SUBSCREEN];
BOOL bEnableSpecificSubClk[MAX_SUBSCREEN];
BOOL bSuppressUpdateSubClk[MAX_SUBSCREEN];

int heightSubClock[MAX_SUBSCREEN];
int widthSubClock[MAX_SUBSCREEN];
int widthMainClockContent, heightMainClockContent;
int widthMainClockFrame, heightMainClockFrame;

BOOL bEnableSubClks = FALSE;
//BOOL bEnhanceSubClkOnDarkTray = FALSE;
//int indexSubClkToUpdate = 255;


/*------------------------------------------------
  globals
--------------------------------------------------*/
HANDLE hmod = 0;

HWND hwndClockMain = NULL;
HWND hwndTClockBarWin11 = NULL;
WNDPROC oldWndProc = NULL;
WNDPROC oldWndProcSub[MAX_SUBSCREEN] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
WNDPROC oldProcTaskbarContentBridge_Win11;

BOOL bTimer = FALSE;
BOOL bTimerDLLMainAdjust = FALSE;
HDC hdcClock = NULL;
HDC hdcClock_work = NULL;

HDC HDC_Stored_Desktop = NULL;
HDC HDC_Stored_ContentBridge_Win11 = NULL;
HDC HDC_Stored_TaskbarMain = NULL;


static HGDIOBJ hbmpClockOld = NULL;
HBITMAP hbmpClock = NULL;
HBITMAP hbm_DIBSection = NULL;
HBITMAP hbm_DIBSection_work = NULL;
BITMAPINFO bmi_MainClock;

HFONT hFon = NULL;
HFONT hFontNotify = NULL;

COLORREF colback, colback2, colfore;
BOOL fillbackcolor = FALSE;
DWORD grad;
BOOL bTimerCheckNetStat = FALSE;	//Added by TTTT
BOOL bTimerAdjust_SysInfo = FALSE;
BOOL bTimerAdjust_NetStat = FALSE;
COLORREF ColorWeekdayText = RGB(0, 0, 0);
COLORREF ColorSaturdayText = RGB(0, 0, 0);
COLORREF ColorSundayText = RGB(0, 0, 0);
COLORREF ColorHolidayText = RGB(0, 0, 0);
COLORREF ColorVPNText = RGB(0, 0, 0);
COLORREF textcol_DoWzone = RGB(0, 0, 0);
BOOL bSaturday = FALSE;
BOOL bSunday = FALSE;
BOOL bHoliday = FALSE;
BOOL b_DayChange = FALSE;

COLORREF originalColorTaskbar = RGB(0, 0, 0);
COLORREF originalColorTaskbar_ForWin11Notify = RGB(0, 0, 0);
COLORREF originalColorTaskbarEdge = RGB(0, 0, 0);

// add by 505 =================================
static BOOL bGetingFocus = FALSE;


enum {
	ANALOG_CLOCK_NOTREAD,
	ANALOG_CLOCK_NOTUSE,
	ANALOG_CLOCK_USE
};
enum {
	ANALOG_CLOCK_POS_LEFT,
	ANALOG_CLOCK_POS_RIGHT,
	ANALOG_CLOCK_POS_MIDDLE
};
#define ACLOCK_SIZE_CX	18
#define ACLOCK_SIZE_CY	18
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
int nAnalogClockUseFlag = ANALOG_CLOCK_NOTREAD;

SIZE sizeAClock = { 0, 0 };
int nAnalogClockPos = ANALOG_CLOCK_POS_LEFT;
static HDC hdcAnalogClock = NULL;
static HBITMAP hbmpAnalogClock = NULL;
static HBITMAP hbmpAnalogClockMask = NULL;
static HDC hdcAnalogClockMask  = NULL;
static LPBYTE lpbyAnalogClockMask = NULL;
static int nHourPenWidth = 2;
static int nMinPenWidth = 1;
static HPEN hpenHour = NULL;
static HPEN hpenMin = NULL;
static int nAnalogClockHPos = 0;
static int nAnalogClockVPos = 0;
static COLORREF colAClockHourHandColor = RGB(255, 0, 0);
static COLORREF colAClockMinHandColor = RGB(0, 0, 255);
static TCHAR szAnalogClockBmp[MAX_PATH] = "";
static SIZE  sizeAnalogBitmapSize = { 0, 0 };
static BOOL InitAnalogClockData(HWND hWnd);
static BOOL bGraphTimerStart = FALSE;
char format[1024];
BOOL bHour12, bHourZero;
SYSTEMTIME LastTime;
int beatLast = -1;
int bDispSecond = FALSE;
int nDispBeat = 0;
int nBlink = 0;		//Integer for TEST, This should be Zero
int nChime = 0;
int BlinksOnChime = 3;
int dwidth = 0, dheight = 0, dvpos = 0, dlineheight = 0, dclkvpos = 0;
BOOL bDispSysInfo = FALSE, bTimerSysInfo = FALSE;
BOOL bGetBattery = FALSE, bGetMem = FALSE,
bGetPm = FALSE, bGetNet = FALSE, bGetHdd = FALSE, bGetCpu = FALSE, bGetVol = FALSE, bGetGpu = FALSE, bGetTemp = FALSE;
int iFreeRes[3] = {0,0,0}, totalCPUUsage = 0, iBatteryLife = 0, iVolume = 0, totalGPUUsage = 0;
extern int CPUUsage[];
BOOL b_SoundCapability = TRUE;
int iCPUClock[MAX_PROCESSOR] = {0};
MEMORYSTATUSEX msMemory;
BOOL bClockShadow = FALSE;
BOOL bClockBorder = FALSE;
BOOL bVolRedraw = FALSE;
int nShadowRange = 1;
COLORREF colShadow;
int nTextPos = 0;
BOOL bRClickMenu = FALSE;
int tEdgeTop;
int tEdgeLeft;
int tEdgeBottom;
int tEdgeRight;

//Addby �Ђ܂���
BOOL checknet=FALSE;
BOOL bGraph = FALSE;
BOOL bLogGraph = FALSE;
BOOL bGraphTate = FALSE;
BOOL bReverseGraph = FALSE;
BOOL bGraphRedraw = FALSE;
BOOL bEnableGPUGraph = TRUE;
BOOL bUseBarMeterColForGraph = FALSE;
int NetGraphScaleRecv = 100;
int NetGraphScaleSend = 100;
int GraphL=0;
int GraphT=0;
int GraphR=0;
int GraphB=0;
int GraphType=1;
double sendlog[MAXGRAPHLOG+1] = { 0 };
double recvlog[MAXGRAPHLOG+1] = { 0 };
COLORREF ColSend,ColRecv,ColSR;
COLORREF ColorCPUGraph, ColorCPUGraph2, ColorGPUGraph;
int graphInterval = 1;
int graphMode = 1;
//int cpuHigh;

extern HWND hwndStart;
extern HWND hwndStartMenu;
extern int codepage;

// XButton Messages
#define WM_XBUTTONDOWN                  0x020B
#define WM_XBUTTONUP                    0x020C


// Added by TTTT for Win10AU (WIN10RS1) compatibility
#define SUBCLASSTRAY_ID		2
int     g_winver = WIN10;              // Windows version, currently set as WIN10(non AU)
BOOL    g_bVertTaskbar = FALSE;        // vertical taskbar ?
int     prevWidthMainClock;      // original clock width
int     prevHeightMainClock;      // original clock height
int		origSubClockWidth[MAX_SUBSCREEN], origSubClockHeight[MAX_SUBSCREEN];
int		widthSubTaskbar[MAX_SUBSCREEN], heightSubTaskbar[MAX_SUBSCREEN];



int g_InternetConnectStat_Win10 = -1;	//added for Internet Connectionstatus by TTTT
char icp_SSID_APName[32];
char activeSSID[32];
char activeAPName[32];

// IP addresses added by TTTT
char ipLTE[32];
char ipEther[32];
char ipWiFi[32];
char ipVPN[32];


int currentLTEProfNum = -1;
int previousLTEProfNum = 0;
int internetConnectProfNum = -1;
int profileNumber_WiFi = -1;
int profileNumber_Ethernet = -1;
int megabytesInGigaByte = 1000;
BOOL b_DebugLog = FALSE;
BOOL b_DebugLog_RegAccess = FALSE;
BOOL b_DebugLog_Specific = FALSE;

int WinBuildNumber = 0;
int Win11Type = 0;

char g_mydir_dll[MAX_PATH]; // path to tclock.exe

char strSoftEtherKeyword[32];
char strVPN_Keyword1[32];
char strVPN_Keyword2[32];
char strVPN_Keyword3[32];
char strVPN_Keyword4[32];
char strVPN_Keyword5[32];

char strVPN_Exclude1[32];
char strVPN_Exclude2[32];
char strVPN_Exclude3[32];
char strVPN_Exclude4[32];
char strVPN_Exclude5[32];

char strEthernet_Keyword1[32];
char strEthernet_Keyword2[32];
char strEthernet_Keyword3[32];
char strEthernet_Keyword4[32];
char strEthernet_Keyword5[32];

BOOL b_CompactMode = TRUE;
BOOL b_SafeMode = FALSE;
BOOL b_ExcessNetProfiles = FALSE;
BOOL muteStatus = FALSE;	//added by TTTT for volume

char strBuf[32];
char strLTE[32];
char charLTE[32];
char strMute[32];

BOOL b_MeteredNetNow = FALSE;
BOOL b_AutoClearLogFile = TRUE;
int AutoClearLogLines = 1000;
int LogLineCount = 1000;

char previousLTEProfName[256];
BOOL b_NormalLog = FALSE;		//added by TTTT

// For BarMeter Added by TTTT
//Color Chart
//���F0
//�D�F�F12632256
//�����D�F�F12632256
//���F16777215
//�ԁF255
//���F�F8388608
//�F16711680
//���F�F16776960
//�΁F65280
//���F�F65535
//�s���N�F16711935

BOOL b_UseBarMeterBL = FALSE;
BOOL b_BatteryLifeAvailable = TRUE;

COLORREF ColorBarMeterBL_Charge = RGB(255, 165, 0);
COLORREF ColorBarMeterBL_High = RGB(0, 0, 255);
COLORREF ColorBarMeterBL_Mid = RGB(0, 255, 0);
COLORREF ColorBarMeterBL_Low = RGB(255, 0, 0);
int BarMeterBL_Left = 230;
int BarMeterBL_Right = 220;
int BarMeterBL_Bottom = 0;
int BarMeterBL_Top = 0; 

int BarMeterBL_Threshold_High = 50;
int BarMeterBL_Threshold_Mid = 25;

int BarMeterCU_Threshold_High = 70;
int BarMeterCU_Threshold_Mid = 50;

BOOL b_BarMeterVL_Horizontal = FALSE;
BOOL b_BarMeterBL_Horizontal = FALSE;
BOOL b_BarMeterCU_Horizontal = FALSE;
BOOL b_BarMeterNet_Horizontal = FALSE;

BOOL b_BarMeterVL_HorizontalToLeft = FALSE;
BOOL b_BarMeterBL_HorizontalToLeft = FALSE;
BOOL b_BarMeterCU_HorizontalToLeft = FALSE;
BOOL b_BarMeterNet_HorizontalToLeft = FALSE;

BOOL b_UseBarMeterCU = FALSE;
COLORREF ColorBarMeterCU_High = RGB(255, 0, 0);
COLORREF ColorBarMeterCU_Mid = RGB(0, 0, 255);
COLORREF ColorBarMeterCU_Low = RGB(0, 255, 0);
int BarMeterCU_Left = 210;
int BarMeterCU_Right = 200;
int BarMeterCU_Bottom = 0;
int BarMeterCU_Top = 0;

BOOL b_UseBarMeterGU = FALSE;
COLORREF ColorBarMeterGPU = RGB(0, 255, 255);
int BarMeterGU_Left = 190;
int BarMeterGU_Right = 180;
int BarMeterGU_Bottom = 0;
int BarMeterGU_Top = 0;

BOOL b_UseBarMeterCore = FALSE;
int NumberBarMeterCore = 10;
COLORREF ColorBarMeterCore_High = RGB(255, 0, 0);
COLORREF ColorBarMeterCore_Mid = RGB(0, 0, 255);
COLORREF ColorBarMeterCore_Low = RGB(0, 255, 0);
int BarMeterCore_Left = 200;
int BarMeterCore_Right = 190;
int BarMeterCore_Bottom = 0;
int BarMeterCore_Top = 0;
int BarMeterCore_Pitch = 5;

BOOL b_UseBarMeterVL = FALSE;
COLORREF ColorBarMeterVL = RGB(0, 255, 0);
COLORREF ColorBarMeterVL_Mute = RGB(255, 0, 0);
int BarMeterVL_Left = 260; 
int BarMeterVL_Right = 250;
int BarMeterVL_Bottom = 0;
int BarMeterVL_Top = 0;

BOOL b_UseBarMeterNet = FALSE;
COLORREF ColorBarMeterNet_Recv = RGB(0, 255, 0);
COLORREF ColorBarMeterNet_Send = RGB(255, 0, 0);
int BarMeterNetRecv_Left = 305;
int BarMeterNetRecv_Right = 300;
int BarMeterNetRecv_Bottom = 0;
int BarMeterNetRecv_Top = -1;
int BarMeterNetSend_Left = 315;
int BarMeterNetSend_Right = 310;
int BarMeterNetSend_Bottom = 0;
int BarMeterNetSend_Top = 0;
BOOL b_BarMeterNet_LogGraph = FALSE;

extern BOOL b_Charging;

COLORREF MyColorTT[MAX_MYCOLORS];
COLORREF MyColorTT_CU(void);
COLORREF MyColorTT_VL(void);
COLORREF MyColorTT_BL(void);
COLORREF MyColorTT_GU(void);

COLORREF MyColorTT_Core(int iCPU);

int NetBarMeterSend, NetBarMeterRecv;
extern double net[];
int LogDigit = 4;
int LogDigit2 = 4;
int NetMIX_Length = 10;
int SSID_AP_Length = 10;
int TimerCountForSec = 1000;
int intervalTimerAdjust = 0;
BOOL b_InitialTimerAdjust = FALSE;
BOOL b_ModernStandbySupported = FALSE;
BOOL b_Sleeping = FALSE;

int offsetBottomOfMeter = 0;
BOOL IsHoliday_Win10(SYSTEMTIME* pt);

extern BOOL flag_VPN;
BOOL b_English = FALSE;

static RGBQUAD* m_color_start = NULL, *m_color_end;
static RGBQUAD* m_color_work_start = NULL, *m_color_work_end;

//Check_Light_Theme_Win10�p
//BOOL b_System_Light_Theme = FALSE;
//BOOL b_Apps_Light_Theme = FALSE;
//BOOL b_Transparency_Theme = FALSE;

char strAdditionalMountPath[10][64];
COLORREF colorBG_original = RGB(0, 0, 0);
int currentTimeZoneBiasMin = 0;
int timezoneBiasMin[256];
int iHourTransition = -1, iMinuteTransition = -1;
int originalWidthTaskbar = 0;
int originalHeightTaskbar = 0;
int originalPosYTaskbar = 0;

int widthTaskbar = 0;
int heightTaskbar = 0;
int posXTaskbar = 0;
int posYTaskbar = 0;
BOOL bAutoRestart = TRUE;

//Win11�Ή��֘A
//Win11�Ή�����E�B���h�E���[�h�t���O
BOOL bWin11Main = FALSE;
BOOL bWin11Sub = FALSE;

//Win11�p�֘A�E�B���h�E�n���h��
HWND hwndWin11ReBarWin = NULL;
HWND hwndWin11ContentBridge = NULL;
HWND hwndWin11InnerTrayContentBridge = NULL;
HWND hwndWin11Notify = NULL;

//�ʒm�E�B���h�E�p
//���p�ݒ�E�t���O
BOOL bUseWin11Notify = TRUE;		//���[�U�ݒ�
BOOL bEnabledWin11Notify = TRUE;	//���p���邩�ǂ���(�ʒm�E�B���h�E���̂͏�ɍ��)
//�ʒm�E�B���h�E�`��p
HDC hdcYesWin11Notify = NULL;
HDC hdcNoWin11Notify = NULL;
HDC hdcFocusWin11Notify = NULL;
HDC hdcWin11Notify = NULL;

HBITMAP hbm_DIBSection_YesWin11Notify = NULL;
HBITMAP hbm_DIBSection_NoWin11Notify = NULL;
HBITMAP hbm_DIBSection_FocusWin11Notify = NULL;
HBITMAP hbm_DIBSection_Win11Notify = NULL;
HBITMAP hbmpIconYesWin11Notify = NULL;
HBITMAP hbmpIconNoWin11Notify = NULL;
HBITMAP hbmpIconFocusWin11Notify = NULL;

RGBQUAD* m_color_Win11Notify_start = NULL;
RGBQUAD* m_color_Win11Notify_end = NULL;
RGBQUAD* m_color_YesWin11Notify_start = NULL;
RGBQUAD* m_color_YesWin11Notify_end = NULL;
RGBQUAD* m_color_NoWin11Notify_start = NULL;
RGBQUAD* m_color_NoWin11Notify_end = NULL;
RGBQUAD* m_color_FocusWin11Notify_start = NULL;
RGBQUAD* m_color_FocusWin11Notify_end = NULL;
COLORREF colWin11Notify = RGB(255, 255, 255);
//�����_�Œʒm���o�Ă��邩�ǂ���
BOOL bExistWin11Notify = FALSE;
//�ʒm�E�B���h�E�̃T�C�Y
int widthWin11Notify = 0;
int heightWin11Notify = 0;
//�ʒm�E�B���h�E���̃A�C�R������ʒu�ƃT�C�Y
POINT posNotifyIcon;
POINT posNotifyText;
int widthNotifyIcon, heightNotifyIcon;
int posXShowDesktopArea;

//���C�����v(����)�����p�ϐ�
int widthWin11Clock = 0;
int heightWin11Clock = 0;
//���i�T�C�Y(����Win11�ʒm�̈�̉�͗p)
int widthWin11Icon = 32;
int widthWin11Button = 30;
int origWidthWin11Tray = 0;
int origHeightWin11Tray = 0;
int defaultWin11ClockWidth = 999;
int defaultWin11NotificationWidth = 100;
int adjustWin11TrayYpos = 0;

//�����p
int adjustWin11TrayCutPosition = 0;
int adjustWin11DetectNotify = 0;
int adjustWin11ClockWidth = 0;
//�z�u�E�J�b�g�����̌��ʓ�����l
int cutOffWidthWin11Tray = 0;
int modifiedWidthWin11Tray = 250;
int posXMainClock = 0;

//�w�������^�X�N�o�[�x���ǂ���
int typeWin11Taskbar = 1;
//�w�������^�X�N�o�[�x�Ńg���C�ʒu�������邩�ǂ���
BOOL bAdjustTrayWin11SmallTaskbar = TRUE;

//�ʒm�̈�p�I�u�W�F�N�g
HBRUSH hBrushWin11Notify;
HPEN hPenWin11Notify;

//ContentBridge�ړ�����g�[�N��
//BOOL bTokenMoveContentBridge = FALSE;

//Focus Assist���
//https://stackoverflow.com/questions/53407374/is-there-a-way-to-detect-changes-in-focus-assist-formerly-quiet-hours-in-windo
//	not_supported = -2,
//	failed = -1,
//	off = 0,
//	priority_only = 1,
//	alarms_only = 2
int intWin11FocusAssist = 0;
int intWin11FocusAssistPrev = 99;
int intWin11NotificationNumber = 0;
int intWin11NotificationNumberPrev = 999;

int offsetClockMS = 0;


BOOL bUseAllColor = FALSE;
BOOL bUseVPNColor = FALSE;
BOOL bUseDateColor = FALSE;
BOOL bUseDowColor = FALSE;
BOOL bUseTimeColor = FALSE;

BOOL bShowWin11NotifyNumber = TRUE;



BOOL b_EnableChime = FALSE; 
BOOL b_EnableBlinkOnChime = FALSE;
static TCHAR szChimeWav[MAX_PATH] = "";
//int intVolChime = 100;
int intOffsetChimeSec = 0;
int chimeHourStart = 0;
int chimeHourEnd = 24;
BOOL b_EnableSecondaryChime = FALSE;
BOOL b_CuckooClock = FALSE;
int intOffsetSecondaryChimeSec = 1800;
static TCHAR szSecondaryChimeWav[MAX_PATH] = "";

extern int numThermalZone;
extern int indexSelectedThermalZone;
extern int pdhTemperature;

BOOL b_WININICHANGED = FALSE;


void GetMainClock(void)
{
	//�^�X�N�g���C�̃n���h���擾(Win10, 11����)
	hwndTrayMain = FindWindowEx(hwndTaskBarMain, NULL, "TrayNotifyWnd", "");

	//TrayClockWClass(Win10�܂ł̎��v)��Win11�ɂ͑��݂��Ȃ��BWin10�܂ł͂��̎��v���������Ďg���B
	hwndClockMain = FindWindowEx(hwndTrayMain, NULL, "TrayClockWClass", NULL);

	//Win11���ƈȉ��̏������������Ċ֘A�̃E�B���h�E�̃n���h�����擾����B
	if (!hwndClockMain) {
		hwndWin11ReBarWin = FindWindowEx(hwndTaskBarMain, NULL, "ReBarWindow32", NULL);		//Shell_TrayWnd�̒����ATrayNotifiWnd�̊O
		hwndWin11ContentBridge = FindWindowEx(hwndTaskBarMain, NULL, "Windows.UI.Composition.DesktopWindowContentBridge", NULL);
		hwndWin11InnerTrayContentBridge = FindWindowEx(hwndTrayMain, NULL, "Windows.UI.Composition.DesktopWindowContentBridge", NULL);
		bWin11Main = TRUE;


		//���C�����v�͐V�����E�B���h�E���^�X�N�o�[��(�g���C���ł͂Ȃ�)�ɍ��
		CreateWin11MainClock();
	}
}

/*------------------------------------------------
  initialize the clock
--------------------------------------------------*/
void InitClock()
{
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	char fname[MAX_PATH];
	BOOL b;
	DWORD dw;
	int nDelay;



	if(hwndClockMain != NULL) return;

	HookEnd();	//Ver 4.0.5.3���s�BInitclock�N�����Ă���͕s�v�Ȃ̂Ńt�b�N�O���B�s��o��Ȃ炱�̍s���폜����B

	GetModuleFileName(hmod, fname, MAX_PATH);


	GetTaskbarSize();
	originalWidthTaskbar = widthTaskbar;
	originalHeightTaskbar = heightTaskbar;
	originalPosYTaskbar = posYTaskbar;

	del_title(fname);
	add_title(fname, "tclock-win10.ini");
	hfind = FindFirstFile(fname, &fd);
	if(hfind != INVALID_HANDLE_VALUE)
	{
		g_bIniSetting = TRUE;
		strcpy(g_inifile, fname);
		FindClose(hfind);
	}

	g_winver = CheckWinVersion_Win10();

	if (Win11Type == 2) 
	{
		GetTaskbarColor_Win11Type2();
	}

	GetMainClock();	//hwndClockMain���Q�b�g, bWin11Main�͂����Ō��肳���B
	if (hwndClockMain == NULL) return;

	hwndDesktop = GetDesktopWindow();
	HDC_Stored_Desktop = GetDC(hwndDesktop);
	ReleaseDC(hwndDesktop, HDC_Stored_Desktop);


	HDC_Stored_TaskbarMain = GetDC(hwndTaskBarMain);
	ReleaseDC(hwndTaskBarMain, HDC_Stored_TaskbarMain);

	if (hwndWin11ContentBridge) {
		HDC_Stored_ContentBridge_Win11 = GetDC(hwndWin11ContentBridge);
		ReleaseDC(hwndWin11ContentBridge, HDC_Stored_ContentBridge_Win11);
	}


	//SafeMode�`�F�b�N
	CheckSafeMode_Win10();

	//���W�X�g���ǂݍ���
	ReadData();






	if (b_DebugLog) {
		writeDebugLog_Win10("[tclock.c][InitClock] bWin11Main =", bWin11Main);
	}

//	CpuMoni_start(); // cpu.c
	PerMoni_start(); // permon.c
	GPUMoni_start(); // gpumon.c
	TempMoni_start(); //tempmon.c
	Net_start();     // net.c

	CheckBatteryAvailability();

	//PostMessage(hwndTClockExeMain, WM_USER, 0, (LPARAM)hwnd);
	PostMessage(hwndTClockExeMain, WM_USER, 0, (LPARAM)hwndClockMain);


	InitAnalogClockData(hwndClockMain);


	GetTimeZoneBias_Win10();


	//�e��T�C�Y�̊m�ہBCalcMainClockSize�̓t�H���g�����łȂ��Ɛ��������ʂ������Ȃ��̂ŁAReadData����Ŏ��s����B
	//�܂��A�T�u�N���X������ƃ��b�Z�[�W�ɋ쓮����ē����n�߂�̂ŁA������O�Ɏ��s����B
	if (bWin11Main) {
		GetWin11ElementSize();
		GetWin11TrayWidth();	//����͏���N�����̂݁I
		SetModifiedWidthWin11Tray();
		CalcMainClockSize();
		if (Win11Type == 2) 
		{
			SetMainClockOnTasktray_Win11();
			COLORREF taskbarColor;
			HDC tempDC = NULL;
			tempDC = GetDC(hwndTaskBarMain);
			taskbarColor = GetBkColor(tempDC);
			ReleaseDC(hwndTaskBarMain, tempDC);
			if (b_DebugLog)writeDebugLog_Win10("[tclock.c] Taskbar Backcolor = ", taskbarColor);
		}
	}
	else {							//Win10
		GetPrevMainClockSize();
		CalcMainClockSize();
	}








	//�T�u�N���X��
	oldWndProc = (WNDPROC)GetWindowLongPtr(hwndClockMain, GWLP_WNDPROC);
	SubclassWindow(hwndClockMain, WndProc);

	if (bWin11Main)		//�����_��WndProc�͓���, Remove�����킹�邱�ƁB
	{

		if (hwndWin11Notify) SubclassWindow(hwndWin11Notify, WndProcWin11Notify);

		if (Win11Type == 2)
		{
			oldProcTaskbarContentBridge_Win11 = (WNDPROC)GetWindowLongPtr(hwndWin11ContentBridge, GWLP_WNDPROC);
			SubclassWindow(hwndWin11ContentBridge, WndProcTaskbarContentBridge_Win11);
		}
		else
		{
			SetWindowSubclass(hwndTrayMain, SubclassTrayProc_Win11,
				SUBCLASSTRAY_ID, (DWORD_PTR)hwndClockMain);
		}
	}
	else 
	{
		SetWindowSubclass(hwndTrayMain, SubclassTrayProc,
			SUBCLASSTRAY_ID, (DWORD_PTR)hwndClockMain);
	}
	//�_�u���N���b�N�󂯕t���Ȃ�
	SetClassLongPtr(hwndClockMain, GCL_STYLE,
	  GetClassLongPtr(hwndClockMain, GCL_STYLE) & ~CS_DBLCLKS);







	//�T�u�f�B�X�v���C��^�X�N�o�[���v�̃t�b�N
	//���C���^�X�N�o�[�����`�F�b�N�ƃT�u�N���b�N�̃T�u�N���X�����s����B
	if (bEnableSubClks) ActivateSubClocks();

	//��̏����Ɏ����ꂽ�̂ŁA���Ȃ��������Ƃ��m�F�ł���������B
	//if (bWin11Main)
	//{
	//	SetWindowSubclass(hwndTrayMain, SubclassTrayProc_Win11,
	//		SUBCLASSTRAY_ID, (DWORD_PTR)hwndClockMain);


	//}
	//else {
	//	SetWindowSubclass(hwndTrayMain, SubclassTrayProc,
	//		SUBCLASSTRAY_ID, (DWORD_PTR)hwndClockMain);
	//}



	b = GetMyRegLong("Mouse", "DropFiles", FALSE);
	DragAcceptFiles(hwndClockMain, b);


	//�^�X�N�o�[�̍X�V
	RedrawMainTaskbar();	//�������f�̂��߂ɕK�v�B�K�v�������Windows�̃��T�C�Y������ʂ���MainClock�̍Ĕz�u��T�C�Y�X�V�AhdcClock�č쐬�����s�����B

	//�c�[���`�b�v�쐬
	TooltipInit(hwndClockMain);


	if (b_DebugLog)writeDebugLog_Win10("[tclock.c] InitClock finished.", 999);

}

void RedrawTClock(void)
{
	if (b_DebugLog)writeDebugLog_Win10("[tclock.c] RedrawTClock called.", 999);
	HDC hdc;

	if (Win11Type == 2) {
		GetTaskbarColor_Win11Type2();
	}

	hdc = GetDC(hwndClockMain);

	if (hdc) {
		DrawClock_New(hdc, b_WININICHANGED);		//b_WININICHANGED = TRUE �̂Ƃ���Win11Type2�̏ꍇ�ɋ����I�ɒʒm���X�V����B
		ReleaseDC(hwndClockMain, hdc);
	}

	b_WININICHANGED = FALSE;
}

void RedrawMainTaskbar(void) 
{
	if (b_DebugLog)writeDebugLog_Win10("[tclock.c] RedrawMainTaskbar called.", 999);
	if (hwndTaskBarMain)
	{
		PostMessage(hwndTrayMain, WM_SIZE, SIZE_RESTORED, 0);
		PostMessage(hwndTaskBarMain, WM_SIZE, SIZE_RESTORED, 0);
		InvalidateRect(hwndTaskBarMain, NULL, TRUE);
	}
}


/*------------------------------------------------
  ending process
--------------------------------------------------*/
void DeleteClockRes(void)
{
	TooltipDeleteRes();
	if (hFon) DeleteObject(hFon); hFon = NULL;
	if (hFontNotify) DeleteObject(hFontNotify); hFontNotify = NULL;
	if (hdcClock) DeleteDC(hdcClock); hdcClock = NULL;
	if (hbmpClock) DeleteObject(hbmpClock); hbmpClock = NULL;
	if (hbmpClockOld) DeleteObject(hbmpClockOld); hbmpClockOld = NULL;
	if (hbm_DIBSection) DeleteObject(hbm_DIBSection); hbm_DIBSection = NULL;
	if (hdcClock_work) DeleteDC(hdcClock_work); hdcClock_work = NULL;
	if (hbm_DIBSection_work) DeleteObject(hbm_DIBSection_work); hbm_DIBSection_work = NULL;


	if (hpenHour) {
		DeleteObject(hpenHour); hpenHour = NULL;
	}
	if (hpenMin) {
		DeleteObject(hpenMin); hpenMin = NULL;
	}
	if (hdcAnalogClock) {
		DeleteDC(hdcAnalogClock);
		hdcAnalogClock = NULL;
	}
	if (hbmpAnalogClock) {
		DeleteObject(hbmpAnalogClock);
		hbmpAnalogClock = NULL;
	}
	if (hdcAnalogClockMask) {
		DeleteDC(hdcAnalogClockMask);
		hdcAnalogClockMask = NULL;
	}
	if (hbmpAnalogClockMask) {
		DeleteObject(hbmpAnalogClockMask);
		hbmpAnalogClockMask = NULL;
		lpbyAnalogClockMask = NULL;
	}

	if (hdcClock) {
		DeleteDC(hdcClock);
		DeleteObject(hbm_DIBSection);
	}

	if (hdcClock_work) {
		DeleteDC(hdcClock_work);
		DeleteObject(hbm_DIBSection_work);
	}

	if (hdcWin11Notify) {
		DeleteDC(hdcWin11Notify);
		DeleteObject(hbm_DIBSection_Win11Notify);
	}

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

	if (hbmpIconYesWin11Notify)
	{
		DeleteObject(hbmpIconYesWin11Notify);
	}

	if (hbmpIconNoWin11Notify)
	{
		DeleteObject(hbmpIconNoWin11Notify);
	}

	if (hbmpIconFocusWin11Notify)
	{
		DeleteObject(hbmpIconFocusWin11Notify);
	}


	

	if (!hBrushWin11Notify)DeleteObject(hBrushWin11Notify);
	if (!hPenWin11Notify)DeleteObject(hPenWin11Notify);

}

void EndClock(void)
{

	if (b_DebugLog)writeDebugLog_Win10("[tclock.c] EndClock called.", 999);

	if (!b_CompactMode) {
		newCodes_close_Win10();	//	-> Limited !b_CompactoMode to cope with Win10 April2018 Update
	}

	DragAcceptFiles(hwndClockMain, FALSE);


	//�T�u��ʂ̎��v�̃T�u�N���X�����������ă^�X�N�o�[�T�C�Y��߂��B
	DisableAllSubClocks();


	TooltipEnd(hwndClockMain);


	DeleteClockRes();
	EndNewAPI(hwndClockMain);
	EndSysres();
	FreeBatteryLife();
	FreeCpuClock();
//	CpuMoni_end(); // cpu.c
	PerMoni_end(); // permon.c
	GPUMoni_end(); // gpumon.c
	TempMoni_end();
	Net_end();     // net.c

	if(hwndClockMain && IsWindow(hwndClockMain))
	{
		if(bTimer) KillTimer(hwndClockMain, IDTIMERDLL_DLLMAIN); bTimer = FALSE;
		if(bTimerSysInfo) KillTimer(hwndClockMain, IDTIMERDLL_SYSINFO);
		if(bGraphTimerStart) KillTimer(hwndClockMain, IDTIMERDLL_GRAPH); bGraphTimerStart=FALSE;
		if(bTimerCheckNetStat) KillTimer(hwndClockMain, IDTIMERDLL_CHECKNETSTAT); bTimerCheckNetStat = FALSE;
		if(bTooltipTimerStart) KillTimer(hwndClockMain, IDTIMERDLL_TIP); bTooltipTimerStart=FALSE;
		KillTimer(hwndClockMain, IDTIMERDLL_DELEYED_RESPONSE);

		if (oldWndProc && (WNDPROC)WndProc == (WNDPROC)GetWindowLongPtr(hwndClockMain, GWLP_WNDPROC))
		{
			SubclassWindow(hwndClockMain, oldWndProc);
		}
		oldWndProc = NULL;
	}

	if (bWin11Main)
	{

		if (Win11Type == 2) {
			//if (bAdjustTrayWin11SmallTaskbar) {
			//	//�^�X�N�o�[����߂�(�ʒu�ƃT�C�Y)
			//	SetWindowPos(hwndTaskBarMain, NULL, 0, originalPosYTaskbar, originalWidthTaskbar, originalHeightTaskbar,
			//		SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOREDRAW | SWP_DEFERERASE);		//SWP_NOSENDCHANGING�����Ă����Ȃ��ƃ^�X�N�o�[�͈ړ����Ȃ��I
			//}
			//else {
				//�^�X�N�o�[����߂�(�T�C�Y�̂�)
				SetWindowPos(hwndTaskBarMain, NULL, 0, 0, originalWidthTaskbar, originalHeightTaskbar,
					SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOREDRAW | SWP_DEFERERASE);		//SWP_NOSENDCHANGING�����Ă����Ȃ��ƃ^�X�N�o�[�͈ړ����Ȃ��I
			//}
		}

		if ((Win11Type == 2) && oldProcTaskbarContentBridge_Win11 && IsWindow(hwndWin11ContentBridge))
			 //&& ((WNDPROC)WndProcTaskbarContentBridge_Win11 == (WNDPROC)GetWindowLongPtr(hwndWin11ContentBridge, GWLP_WNDPROC)))
		{
			SubclassWindow(hwndWin11ContentBridge, oldProcTaskbarContentBridge_Win11);
			oldProcTaskbarContentBridge_Win11 = NULL;
			ShowWindow(hwndWin11ContentBridge, SW_SHOW);	//��������Ȃ��ƃ^�X�N�o�[��������B
			ShowWindow(hwndTaskBarMain, SW_SHOW);	//��������Ȃ��ƃ^�X�N�o�[��������B
		}
		else if (Win11Type < 2)
		{
			RemoveWindowSubclass(hwndTrayMain, SubclassTrayProc_Win11,
				SUBCLASSTRAY_ID);
		}

		PostMessage(hwndClockMain, WM_CLOSE, 0, 0);
		UnregisterClass("TClockMain", hmod);

		SubclassWindow(hwndWin11Notify, DefWindowProc);	//��������Ȃ��ƒ��p��̃��[�U�C���^�[�t�F�[�X�ɍs���Ă��܂��B
		PostMessage(hwndWin11Notify, WM_CLOSE, 0, 0);
		UnregisterClass("TClockNotify", hmod);

		if (Win11Type == 2) {
//			SubclassWindow(hwndTClockBarWin11, DefWindowProc);
			PostMessage(hwndTClockBarWin11, WM_CLOSE, 0, 0);
			UnregisterClass("TClockBarWin11", hmod);
		}

	}else {
		RemoveWindowSubclass(hwndTrayMain, SubclassTrayProc,
			SUBCLASSTRAY_ID);
	}


	RedrawMainTaskbar();

	//���̌チ�C���E�B���h�E���I������̂�: PostMessage(hwndTClockExeMain, WM_USER+2, 0, 0);�����s
	//�������A����EndClock��WndProc��WM_CLOSE�ŌĂ΂ꂽ�ꍇ�ɂ̓��C������Ă΂�Ă���̂ŁA���C�����ŏI���������s����B
	//���̂��߁APostMessage�������ɃR�[�h���Ă͂����Ȃ��B

}

/*------------------------------------------------
  subclass procedure of the clock
--------------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND tempHwnd;
//	tempHwnd = hwnd;
	tempHwnd = hwndClockMain;

	//���̃R�[���o�b�N�֐���hwndClockMain�Ƃ̑g�ݍ��킹�łł������������삵�Ȃ��̂ŁA�����Ƃ���hwnd = hwndClockMain�����R�[���o�b�N�֐��Ȃ̂�tempHwnd�ŏ������Ă���
	//message�́A���^�]����WM_TIMER(275)�ȊO�͂߂����ɗ��Ȃ��B
//	if (b_DebugLog) writeDebugLog_Win10("[tclock.c][WndProc] Window Message was recevied, message = ", message);


	switch(message) // �c�[���`�b�v�Ή�
	{
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			//if (b_DebugLog) {
			//	writeDebugLog_Win10("[tclock.c][WndProc] Mouse Event received.", 999);
			//	writeDebugLog_Win10("[tclock.c][WndProc] xPos =", (int)GET_X_LPARAM(GetMessagePos()));
			//	writeDebugLog_Win10("[tclock.c][WndProc] yPos =", (int)GET_Y_LPARAM(GetMessagePos()));
			//}
			if (bEnableTooltip) {
					TooltipOnMouseEvent(tempHwnd, message, wParam, lParam, 1);
			}
			if (b_Sleeping)
			{
				b_Sleeping = FALSE;
				if (b_DebugLog) writeDebugLog_Win10("[tclock.c][WndProc] Awake from Sleep by Mouse Action", 999);
			}
			//if (Win11Type == 2)
			//{
			//	extern BOOL b_ShowingTClockBarWin11;
			//	if (b_ShowingTClockBarWin11) {
			//		ReturnToOriginalTaskBar();
			//	}
			//}
			break;
	}

	switch(message)
	{
		case WM_DESTROY:
			if (b_DebugLog) writeDebugLog_Win10("[tclock.c][WndProc()] WM_DESTROY received, bAutoRestart =", bAutoRestart);
			//���炩�̗��R��WM_CLOSE�ł͂Ȃ��A����DESTROY�������P�[�X�BWinExplorer��Destroy����Ƃ��ꂪ�Ă΂��BExplorer�N���b�V�����ɂ͂���͗��Ȃ��B
			if (bWin11Main) {
				if (bAutoRestart)
				{
					//�ċN������ꍇ
					RestartTClockFromDLL();
				}
				else
				{
					//�I������ꍇ�B���v������I������ꍇ�ɂ�EndClock->PostMessage��WM_USER+2�𑗐M
					EndClock();
					PostMessage(hwndTClockExeMain, WM_USER + 2, 0, 0);		//���̍s���R�����g�A�E�g�����Dll�������I������Bexemain����onTimerZombieChekck�Ŕ������āA�ݒ�ɉ����ďI��/�ċN�����Ă����B
				}
			}
			break;
		case (WM_USER + 100):
			// �e�E�B���h�E���瑗���A�T�C�Y��Ԃ����b�Z�[�W�B
			// Win10RS1(=AU)�ł́A���C���N���b�N�ł̓^�X�N�g���C�����̃��b�Z�[�W���󂯂đΉ�����(SubClassTrayProc)�̂ŁA�������Ȃ��ėǂ��B
			// LRESULT�`����LOWORD=���v�����AHIWORD=���v����ԓ�����Əꏊ���m�ۂ����B

			//if (g_winver&WIN10RS1) 
			break;

		// �V�X�e���̐ݒ�𔽉f����
		case WM_SYSCOLORCHANGE:
		case WM_WININICHANGE:
		case WM_TIMECHANGE:	
		case (WM_USER+101):		// �e�E�B���h�E���瑗����
		{
			CreateClockDC();


			//b_WININICHANGED = TRUE;		//�����ɂ��̃t���O�𗧂Ă�Ɖ�ʐݒ�X�V�O��ontimer_win10���Ă΂�Ă��܂��̂ŁA�ҋ@��(DelayedResponseToSysChange�Ɉړ�)
			if (b_DebugLog)writeDebugLog_Win10("[tclock.c][WndProc] WM_WININICHANGE etc received. message = ", message);
			SetTimer(hwndClockMain, IDTIMERDLL_DELEYED_RESPONSE, 500, NULL);

			return 0;
		}
		case WM_SIZE:
		{
			//[ToDo]Wind11�Ή���3.5.0.3����ς����̂�Win10�Ŗ��Ȃ����v�`�F�b�N�I�I ->�܂��������̂Ŗ߂����B
			//LRESULT ret;
			//CalcMainClockSize();
			//ret = oldWndProc(hwnd, message, wParam, lParam);
			//if (b_DebugLog) {
			//	writeDebugLog_Win10("[tclock.c][WndProc] ret from oldWndProc =", (int)ret);
			//}
			//return MAKELONG(widthMainClockFrame, heightMainClockFrame);
			break;
		}
		case WM_ERASEBKGND:
			break;
		case WM_SETFOCUS:
			bGetingFocus = TRUE;
			break;
		case WM_KILLFOCUS:
			bGetingFocus = FALSE;
			break;
		case WM_PAINT:
		{
			//RedrawTClock�Ə����͓�������WM_PAINT�ɑ΂��Ă�GetDC->BeginPaint�Ƃ��ɂ���K�v������炵��
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(tempHwnd, &ps);
			DrawClock_New(hdc, FALSE);
			EndPaint(tempHwnd, &ps);
			return 0;
		}
		case WM_TIMER:
//			if (b_DebugLog) writeDebugLog_Win10("[tclock.c][WndProc() WM_TIMER received with ID: ", (int)wParam);
			if (!b_Sleeping)
			{
				if (wParam == IDTIMERDLL_DLLMAIN)
				{
					OnTimer_Win10();
				}
				else if (wParam == IDTIMERDLL_SYSINFO)
				{
					UpdateSysRes(bGetBattery, bGetMem, bGetNet, bGetHdd, bGetCpu, bGetVol, bGetGpu, bGetTemp);
				}
				else if (wParam == IDTIMERDLL_CHECKNETSTAT)
				{
					OntimerCheckNetStat_Win10(tempHwnd);
				}
				else if (wParam == IDTIMERDLL_TIP) {
				//Ver 4.1�ȍ~��OnTimer_Win10����s�����ƂƂ���B�^�C�}�[���N�����Ȃ��悤�ɂ��Ă���B
//					TooltipOnTimer(tempHwnd);
				}
				else if (wParam == IDTIMERDLL_DELEYED_RESPONSE) {
					KillTimer(tempHwnd, IDTIMERDLL_DELEYED_RESPONSE);
					DelayedResponseToSyschange();
				}
				else if (wParam == IDTIMERDLL_WIN11TYPE2_SHOW_TASKBAR) {
					KillTimer(hwndClockMain, IDTIMERDLL_WIN11TYPE2_SHOW_TASKBAR);
					MoveWin11ContentBridge(3);
				}
			}
			else if (wParam == IDTIMERDLL_DLLMAIN)
			{
//				if (b_DebugLog) writeDebugLog_Win10("[tclock.c][WndProc] WM_TIMER actions cancelled because b_Sleeping = TRUE", 999);
			}
			return 0;
		// �}�E�X�_�E��
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
			if (message == WM_RBUTTONDOWN && (wParam & MK_LBUTTON || ((wParam&MK_CONTROL)&&(wParam&MK_SHIFT)) || bRClickMenu))
			{
				return 0;
			}

			PostMessage(hwndTClockExeMain, message, wParam, lParam);
			return 0;

		// �}�E�X�A�b�v
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
			if (message == WM_RBUTTONUP && (wParam & MK_LBUTTON || ((wParam&MK_CONTROL)&&(wParam&MK_SHIFT)) || bRClickMenu))
			{
				DWORD mp;
				mp = GetMessagePos();
				PostMessage(hwndTClockExeMain, WM_CONTEXTMENU, (WPARAM)tempHwnd, (LPARAM)mp);
				if (Win11Type == 2) {	//�N���b�N�ɂ��TClockBarWin11���^�X�N�o�[���O�ʂɗ��Ă��܂��̂�߂��B
					SetWindowPos(hwndTClockBarWin11, hwndTaskBarMain, 0, 0, 0, 0,
						SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_NOSENDCHANGING | SWP_SHOWWINDOW);
				}
				return 0;
			}

			PostMessage(hwndTClockExeMain, message, wParam, lParam);
			//// ���E�N���b�N��WM_CONTEXTMENU�ɕϊ�����Ȃ��悤��oldWndProc���Ă΂Ȃ�
			//// �Ă�ł��܂��ƓƎ����������蓖�Ă��Ƃ������j���[���J���Ă��܂�
			////if(message == WM_RBUTTONUP) break;
			return 0;

		case WM_MOUSEMOVE:
			return 0;
		case WM_CONTEXTMENU:   // �E�N���b�N���j���[
			// ApplicationKey�������ꂽ�Ƃ��ɂ���
			// �E�N���b�N�̏ꍇ�́��Ŏ��O�ŕϊ����Ȃ��Ƥ�Ǝ����������蓖�Ă��Ƃ���
			// ���j���[���J���Ă��܂�
			//if (bRClickMenu) {
				PostMessage(hwndTClockExeMain, message, wParam, lParam);
			//}
			return 0;
		case WM_NCHITTEST: // oldWndProc�ɏ��������Ȃ�
			return DefWindowProc(tempHwnd, message, wParam, lParam);
		case WM_MOUSEACTIVATE:
			return MA_ACTIVATE;
		// �t�@�C���̃h���b�v
		case WM_DROPFILES:
			PostMessage(hwndTClockExeMain, WM_DROPFILES, wParam, lParam);
			return 0;
		case WM_NOTIFY: //�c�[���`�b�v�̃e�L�X�g�\��
		{
			LRESULT lres;
			if (TooltipOnNotify(&lres, lParam)) return lres;
			break;
		}
		case WM_COMMAND:
			if (LOWORD(wParam) == IDC_EXIT) 
			{
				if (b_DebugLog) writeDebugLog_Win10("[tclock.c][WndProc()] WM_COMMAND with 102(IDC_EXIT) received", 999);
				EndClock();	// Modified by TTTT for Win10AU (WIN10RS1) compatibility
			}
			else if (LOWORD(wParam) == IDC_RESTART)
			{
				if (b_DebugLog) writeDebugLog_Win10("[tclock.c][WndProc()] WM_COMMAND with IDC_RESTART received", 999);
				RestartTClockFromDLL();	// Added by TTTT
			}
			else if (LOWORD(wParam) == CLOCKM_REQUEST_TEMPCOUNTERINFO)
			{
				return (numThermalZone * 200 + TempMoni_get((int)lParam));
			}
			else if (LOWORD(wParam) == CLOCKM_ZOMBIECHECK_CALL)
			{
				return 255;		//�Ăяo�����̓��C���v���O����(exemain.) 255��Ԃ��ƁA���̃R�[�h(WndProc)�������Ă��� = �������v�����݂���A�Ɣ��f�����B
			}
			else if (!b_CompactMode)
			{
				if (LOWORD(wParam) == CLOCKM_TOGGLE_BARMETER_VOLUME) toggleBarMeterFunc_Win10(0);// Added by TTTT
				if (LOWORD(wParam) == CLOCKM_TOGGLE_BARMETER_CPU) toggleBarMeterFunc_Win10(1);// Added by TTTT
				if (LOWORD(wParam) == CLOCKM_TOGGLE_BARMETER_BATTERY) toggleBarMeterFunc_Win10(2);// Added by TTTT
			}
			return 0;

		case CLOCKM_SLEEP_IN:
			if (b_DebugLog) writeDebugLog_Win10("[tclock.c][WndProc()] CLOCKM_SLEEP_IN received", 999);
			if (b_ModernStandbySupported) b_Sleeping = TRUE;
			return 0;

		case CLOCKM_SLEEP_AWAKE:
			if (b_DebugLog) writeDebugLog_Win10("[tclock.c][WndProc()] CLOCKM_SLEEP_AWAKE received", 999);
			b_Sleeping = FALSE;
			return 0;

		case CLOCKM_REFRESHCLOCK: // refresh the clock
		{
			if (b_DebugLog) writeDebugLog_Win10("[tclock.c][WndProc] CLOCKM_REFRESHCLOCK received", 999);
			RestartOnRefresh();
			return 0;
		}
		case CLOCKM_VISTACALENDAR:
			if (b_DebugLog) writeDebugLog_Win10("[tclock.c][WndProc() CLOCKM_VISTACALENDAR received", 999);
			if (bWin11Main)
			{
				ShellExecute(NULL, "open", "ms-actioncenter:", NULL, NULL, SW_SHOWNORMAL);
			}
			else
			{
				CallWindowProc(oldWndProc, tempHwnd, WM_LBUTTONDOWN, 0, 0);
				CallWindowProc(oldWndProc, tempHwnd, WM_LBUTTONUP, 0, 0);
			}
			return 0;

		case CLOCKM_SHOWAVAILABLENETWORKS:
			if (b_DebugLog) writeDebugLog_Win10("[tclock.c][WndProc() CLOCKM_SHOWAVAILABLENETWORKS received", 999);
			{
				ShellExecute(NULL, "open", "ms-availablenetworks:", NULL, NULL, SW_SHOWNORMAL);
			}
			return 0;

		case CLOCKM_SHOWPROFILELIST:
			saveAndOpenProfTable(TRUE);
			return 0;
		case CLOCKM_MOVEWIN11CONTENTBRIDGE:
			if (bWin11Main) MoveWin11ContentBridge((int)wParam);
			return 0;
		case CLOCKM_REFRESHTASKBAR: // refresh other elements than clock

			SetLayeredTaskbar(tempHwnd);
			RedrawMainTaskbar();	//�������f�̂��߂ɕK�v�B�K�v�������Windows�̃��T�C�Y������ʂ���MainClock�̍Ĕz�u��T�C�Y�X�V�AhdcClock�č쐬�����s�����B

			return 0;
		case WM_WINDOWPOSCHANGING:  // �T�C�Y����
			break;
	}
	return CallWindowProc(oldWndProc, tempHwnd, message, wParam, lParam);
}


void DelayedResponseToSyschange(void)
{
	if (b_DebugLog)writeDebugLog_Win10("[subclock.c] DelayedResponseToSyschange called.", 999);
	b_WININICHANGED = TRUE;	//����Ontimer_Win10->RedrawClock�̃^�C�~���O��Win11Notify�������X�V�����B


	if (bEnableSubClks) {
		CheckSubClocks();
		SetAllSubClocks();
	}
}

void RestartOnRefresh(void)
{

	if (b_DebugLog)writeDebugLog_Win10("[tclock.c] RestartOnRefresh called. bWin11Main =", bWin11Main);

	BOOL tempBool;
	//////b_ImmediateUsageUpdate = TRUE;

	tempBool = bEnableSubClks;

	ReadData();

	if (!tempBool && bEnableSubClks) {
		ActivateSubClocks();
	}
	else if (tempBool && !bEnableSubClks) {
		DisableAllSubClocks();
	}

	InitAnalogClockData(hwndClockMain);

	tempBool = GetMyRegLong("Mouse", "DropFiles", FALSE);
	DragAcceptFiles(hwndClockMain, tempBool);

	if (bWin11Main)
	{
		//�؂藎�Ƃ��ʒu�Ȃǂ̓E�B���h�E��񂪂Ȃ��ƌ��߂��Ȃ��̂ōēx�ĂԁB
		GetWin11ElementSize();
//		GetWin11TrayWidth();	//�Ă�ł�NG�B���łɃT�C�Y���ς���Ă���̂Ō�����擾���Ă��܂��B
		SetModifiedWidthWin11Tray();
		//CreateWin11NotifyDC();	//�s�v
		//Win11�ł�SetMainClockOnTasktray_Win11�����s����(Win11Type == 2�̏ꍇ�͕K�v)
		if (Win11Type == 2) {
			SetMainClockOnTasktray_Win11();
		}
		if (bEnabledWin11Notify) DrawWin11Notify(TRUE);	//�ʒm�A�C�R���̐F�̔��f�̂���
	}

	//��������T�u�N���b�N���N���A
	for (int i = 0; i < MAX_SUBSCREEN; i++)
	{
		if (bEnableSpecificSubClk[i]) {
			ClearSpecificSubClock(i);
			bSuppressUpdateSubClk[i] = TRUE;
		}
	}

	//�O���t�f�[�^�N���A
	ClearGraphData();

	RedrawMainTaskbar();

	RedrawTClock();

	TooltipOnRefresh(hwndClockMain);
}




/*------------------------------------------------
�@�ݒ�̓ǂݍ��݂ƃf�[�^�̏�����
--------------------------------------------------*/
void ReadData()
{
	char fontname[80];
	char strTemp[128];
	int fontsize;
	LONG weight, italic;
	SYSTEMTIME lt;
	DWORD dwInfoFormat;
	int resnetinterval;
	TCHAR fname[MAX_PATH];

	extern BOOL b_exist_DOWzone;
	b_exist_DOWzone = FALSE;

	b_DebugLog = GetMyRegLong(NULL, "DebugLog", FALSE);
	SetMyRegLong(NULL, "DebugLog", b_DebugLog);

	if (b_DebugLog) writeDebugLog_Win10("[tclock.c] ReadData called.", 999);



	UpdateSettingFile();

	SetMyRegLong("Status_DoNotEdit", "Win11TClockMain", bWin11Main);

	SetMyRegLong("Status_DoNotEdit", "WindowsType", Win11Type);

	GetModuleFileName(hmod, g_mydir_dll, MAX_PATH);		//ini�t�@�C���o�R�łȂ��f�B���N�g���擾����悤�� by TTTT
	del_title(g_mydir_dll);






	b_NormalLog = GetMyRegLong(NULL, "NormalLog", TRUE);

	bAutoRestart = GetMyRegLong(NULL, "AutoRestart", TRUE);

	b_DebugLog_RegAccess = GetMyRegLong(NULL, "DebugLog_RegAccess", FALSE);
	b_DebugLog_Specific = GetMyRegLong(NULL, "DebugLog_Specific", FALSE);

	b_AutoClearLogFile = GetMyRegLong(NULL, "AutoClearLogFile", TRUE);
	SetMyRegLong(NULL, "AutoClearLogFile", b_AutoClearLogFile);

	AutoClearLogLines = GetMyRegLong(NULL, "AutoClearLogLines", 1000);
	if (AutoClearLogLines < 100) AutoClearLogLines = 100;
	SetMyRegLong(NULL, "AutoClearLogLines", AutoClearLogLines);

	b_English = GetMyRegLong(NULL, "EnglishMenu", FALSE);



	bEnableSubClks = GetMyRegLong(NULL, "EnableOnSubDisplay", TRUE);
	SetMyRegLong(NULL, "EnableOnSubDisplay", bEnableSubClks);

	//bEnhanceSubClkOnDarkTray = GetMyRegLong(NULL, "EnhanceSubClkOnDarkTray", FALSE);
	//SetMyRegLong(NULL, "EnhanceSubClkOnDarkTray", bEnhanceSubClkOnDarkTray);

	offsetClockMS = (int)(short)GetMyRegLong(NULL, "OffsetClockMS", 0);
	SetMyRegLong(NULL, "OffsetClockMS", (int)(short)offsetClockMS);


	//Win11��ini�t�@�C���ǂݏo�����ɖ���������ꍇ�ɂ̓R�����g�A�E�g���O���B

//	if (bWin11Main) bEnableSubClks = FALSE;
	

	//LogLineCount = AutoClearLogLines;
	LogLineCount = 0;

	GetMyRegStr("Status_DoNotEdit", "Version", Ver_TClockWin10, 16, "Not Available");

	b_ExistLTEProfile = GetMyRegLong("Status_DoNotEdit", "ExistLTEProfile", FALSE);

	b_ExistMeteredProfile = GetMyRegLong("Status_DoNotEdit", "ExistMeteredProfile", FALSE);

	//���_���X�^���o�C�Ή��m�F
	b_ModernStandbySupported = CheckModernStandbyCapability_Win10();
	SetMyRegLong("Status_DoNotEdit", "ModernStandbySupported", b_ModernStandbySupported);

	//�ݒ�ԍ��擾


	colfore = (COLORREF)GetMyRegLong("Color_Font", "ForeColor", GetSysColor(COLOR_BTNTEXT));
//	if (colfore & 0x80000000) colfore = GetSysColor(colfore & 0x00ffffff);
	SetMyRegLong("Color_Font", "ForeColor", colfore);

	ColorWeekdayText = colfore;
	colWin11Notify = colfore;		//�ʒm�A�C�R���̓��C���e�L�X�g�Ɠ��F

	colback = (COLORREF)GetMyRegLong("Color_Font", "BackColor", GetSysColor(COLOR_3DFACE));
//	if (colback & 0x80000000) colback = GetSysColor(colback & 0x00ffffff);
	SetMyRegLong("Color_Font", "BackColor", colback);

	colback2 = (COLORREF)GetMyRegLong("Color_Font", "BackColor2", colback);
//	if (colback2 & 0x80000000) colback2 = GetSysColor(colback2 & 0x00ffffff);
	SetMyRegLong("Color_Font", "BackColor2", colback2);

	if (!GetMyRegLong("Color_Font", "UseBackColor2", FALSE))
	{
		colback2 = colback;
	}


	fillbackcolor = GetMyRegLong("Color_Font", "UseBackColor", TRUE);

	grad = GetMyRegLong("Color_Font", "GradDir", GRADIENT_FILL_RECT_H);

	bClockShadow = GetMyRegLong("Color_Font", "ForeColorShadow", FALSE);
	bClockBorder = GetMyRegLong("Color_Font", "ForeColorBorder", FALSE);

	colShadow = (COLORREF)GetMyRegLong("Color_Font", "ShadowColor", RGB(0, 0, 0));
	//if (colShadow & 0x80000000) colShadow = GetSysColor(colShadow & 0x00ffffff);
	SetMyRegLong("Color_Font", "ShadowColor", colShadow);

	nShadowRange = (int)(short)GetMyRegLong("Color_Font", "ClockShadowRange", 1);

	bRClickMenu = GetMyRegLong("Mouse", "RightClickMenu", TRUE);


	GetMyRegStr("Color_Font", "Font", fontname, 80, "");

	fontsize = GetMyRegLong("Color_Font", "FontSize", 9);
	weight = GetMyRegLong("Color_Font", "Bold", 0);
	if(weight) weight = FW_BOLD;
	else weight = 0;
	italic = GetMyRegLong("Color_Font", "Italic", 0);


	
	hFon = CreateMyFont(fontname, fontsize, weight, italic);
	SetTClockFont();

	TooltipReadData();

	//Ver 4.1�Ńc�[���`�b�v�X�V��OnTimer_Win10������s���邱�Ƃɂ����̂ŃR�����g�A�E�g
	//if (bTooltipTimerStart) KillTimer(hwndClockMain, IDTIMERDLL_TIP); bTooltipTimerStart = FALSE;
	//if (bEnableTooltip) {
	//	bTooltipTimerStart = SetTimer(hwndClockMain, IDTIMERDLL_TIP, 200, NULL) != 0;
	//}

	nTextPos = GetMyRegLong("Color_Font", "TextPos", 0);

	dwidth = (int)(short)GetMyRegLong("Color_Font", "ClockWidth", 0);
	dvpos = (int)(short)GetMyRegLong("Color_Font", "VertPos", 0);
	dlineheight = (int)(short)GetMyRegLong("Color_Font", "LineHeight", 0);

	ColorSaturdayText = (COLORREF)GetMyRegLong("Color_Font", "Saturday_TextColor", 0x00C8FFC8);
	//if (ColorSaturdayText & 0x80000000) ColorSaturdayText = GetSysColor(ColorSaturdayText & 0x00ffffff);
	SetMyRegLong("Color_Font", "Saturday_TextColor", ColorSaturdayText);

	ColorSundayText = (COLORREF)GetMyRegLong("Color_Font", "Sunday_TextColor", 0x00C8C8FF);
	//if (ColorSundayText & 0x80000000) ColorSundayText = GetSysColor(ColorSundayText & 0x00ffffff);
	SetMyRegLong("Color_Font", "Sunday_TextColor", ColorSundayText);

	ColorHolidayText = (COLORREF)GetMyRegLong("Color_Font", "Holiday_TextColor", 0x00C8C8FF);
	//if (ColorHolidayText & 0x80000000) ColorHolidayText = GetSysColor(ColorHolidayText & 0x00ffffff);
	SetMyRegLong("Color_Font", "Holiday_TextColor", ColorHolidayText);

	ColorVPNText = (COLORREF)GetMyRegLong("Color_Font", "VPN_TextColor", 0x00FFFF00);
	//if (ColorVPNText & 0x80000000) ColorVPNText = GetSysColor(ColorVPNText & 0x00ffffff);
	SetMyRegLong("Color_Font", "VPN_TextColor", ColorVPNText);

	bUseAllColor = GetMyRegLong("Color_Font", "UseAllColor", FALSE);
	SetMyRegLong("Color_Font", "UseAllColor", bUseAllColor);

	bUseVPNColor = GetMyRegLong("Color_Font", "UseVPNColor", FALSE);
	SetMyRegLong("Color_Font", "UseVPNColor", bUseVPNColor);

	bUseDateColor = GetMyRegLong("Color_Font", "UseDateColor", FALSE);
	SetMyRegLong("Color_Font", "UseDateColor", bUseDateColor);

	bUseDowColor = GetMyRegLong("Color_Font", "UseDowColor", FALSE);
	SetMyRegLong("Color_Font", "UseDowColor", bUseDowColor);

	bUseTimeColor = GetMyRegLong("Color_Font", "UseTimeColor", FALSE);
	SetMyRegLong("Color_Font", "UseTimeColor", bUseTimeColor);


	bGraph = GetMyRegLong("Graph", "BackNet", FALSE);
	SetMyRegLong("Graph", "BackNet", bGraph);

	bEnableGPUGraph = GetMyRegLong("Graph", "EnableGPUGraph", TRUE);
	SetMyRegLong("Graph", "EnableGPUGraph", bEnableGPUGraph);

	bUseBarMeterColForGraph = GetMyRegLong("Graph", "UseBarMeterColForGraph", FALSE);
	SetMyRegLong("Graph", "UseBarMeterColForGraph", bUseBarMeterColForGraph);

	bLogGraph = GetMyRegLong("Graph", "LogGraph", FALSE);
	bReverseGraph = GetMyRegLong("Graph", "ReverseGraph", FALSE);
	bGraphTate = GetMyRegLong("Graph", "GraphTate", FALSE);
	NetGraphScaleRecv = GetMyRegLong("Graph", "NetGraphScaleRecv", 100);
	NetGraphScaleSend = GetMyRegLong("Graph", "NetGraphScaleSend", 100);
	ColSend = GetMyRegLong("Graph", "BackNetColSend", RGB(255, 0, 0));
	ColRecv = GetMyRegLong("Graph", "BackNetColRecv", RGB(0, 255, 0));
	ColSR = GetMyRegLong("Graph", "BackNetColSR", 0x00800080);

	ColorCPUGraph = GetMyRegLong("Graph", "ColorCPUGraph", RGB(0, 255, 0));
	SetMyRegLong("Graph", "ColorCPUGraph", ColorCPUGraph);

	ColorCPUGraph2 = GetMyRegLong("Graph", "ColorCPUGraph2", RGB(255, 0, 0));
	SetMyRegLong("Graph", "ColorCPUGraph2", ColorCPUGraph2);

	ColorGPUGraph = GetMyRegLong("Graph", "ColorGPUGraph", RGB(255, 0, 255));
	SetMyRegLong("Graph", "ColorGPUGraph", ColorGPUGraph);


	graphInterval = 1;
	graphMode = GetMyRegLong("Graph", "GraphMode", 1);
	//cpuHigh = GetMyRegLong("Graph", "CpuHigh", 70);
	GraphL = GetMyRegLong("Graph", "GraphLeft", 0);
	GraphT = GetMyRegLong("Graph", "GraphTop", 0);
	GraphR = GetMyRegLong("Graph", "GraphRight", 1);
	GraphB = GetMyRegLong("Graph", "GraphBottom", 1);
	GraphType = GetMyRegLong("Graph", "GraphType", 1);

	if (bLogGraph)
	{
		if (NetGraphScaleRecv > 9) LogDigit = log10(NetGraphScaleRecv) + 1;
		if (NetGraphScaleSend > 9) LogDigit2 = log10(NetGraphScaleSend) + 1;
	}


	previousLTEProfNum = GetMyRegLong("Status_DoNotEdit", "PreviousLTEProfNumber", 0);
	SetMyRegLong("Status_DoNotEdit", "PreviousLTEProfNumber", previousLTEProfNum);

	GetMyRegStr("Status_DoNotEdit", "PreviousLTEProfName", previousLTEProfName, 256, "");
	SetMyRegStr("Status_DoNotEdit", "PreviousLTEProfName", previousLTEProfName);

	megabytesInGigaByte = GetMyRegLong("ETC", "MegabytesInGigaByte", 1000);
	if (megabytesInGigaByte != 1024) megabytesInGigaByte = 1000;
	SetMyRegLong("ETC", "MegabytesInGigaByte", megabytesInGigaByte);




	GetMyRegStr("VPN", "SoftEtherKeyword", strSoftEtherKeyword, 32, "VPN Client Adapter");
	SetMyRegStr("VPN", "SoftEtherKeyword", strSoftEtherKeyword);



	GetMyRegStr("VPN", "VPN_Keyword1", strVPN_Keyword1, 32, "");
	SetMyRegStr("VPN", "VPN_Keyword1", strVPN_Keyword1);
	if (strlen(strVPN_Keyword1) == 0) strcpy(strVPN_Keyword1, "VPN Adapter Keyword1");

	GetMyRegStr("VPN", "VPN_Keyword2", strVPN_Keyword2, 32, "");
	SetMyRegStr("VPN", "VPN_Keyword2", strVPN_Keyword2);
	if (strlen(strVPN_Keyword2) == 0) strcpy(strVPN_Keyword2, "VPN Adapter Keyword2");


	GetMyRegStr("VPN", "VPN_Keyword3", strVPN_Keyword3, 32, "");
	SetMyRegStr("VPN", "VPN_Keyword3", strVPN_Keyword3);
	if (strlen(strVPN_Keyword3) == 0) strcpy(strVPN_Keyword3, "VPN Adapter Keyword3");

	GetMyRegStr("VPN", "VPN_Keyword4", strVPN_Keyword4, 32, "");
	SetMyRegStr("VPN", "VPN_Keyword4", strVPN_Keyword4);
	if (strlen(strVPN_Keyword4) == 0) strcpy(strVPN_Keyword4, "VPN Adapter Keyword4");

	GetMyRegStr("VPN", "VPN_Keyword5", strVPN_Keyword5, 32, "");
	SetMyRegStr("VPN", "VPN_Keyword5", strVPN_Keyword5);
	if (strlen(strVPN_Keyword5) == 0) strcpy(strVPN_Keyword5, "VPN Adapter Keyword5");

	GetMyRegStr("VPN", "VPN_Exclude1", strVPN_Exclude1, 32, "");
	SetMyRegStr("VPN", "VPN_Exclude1", strVPN_Exclude1);
	if (strlen(strVPN_Exclude1) == 0) strcpy(strVPN_Exclude1, "VPN Adapter Exclude1");

	GetMyRegStr("VPN", "VPN_Exclude2", strVPN_Exclude2, 32, "");
	SetMyRegStr("VPN", "VPN_Exclude2", strVPN_Exclude2);
	if (strlen(strVPN_Exclude2) == 0) strcpy(strVPN_Exclude2, "VPN Adapter Exclude2");


	GetMyRegStr("VPN", "VPN_Exclude3", strVPN_Exclude3, 32, "");
	SetMyRegStr("VPN", "VPN_Exclude3", strVPN_Exclude3);
	if (strlen(strVPN_Exclude3) == 0) strcpy(strVPN_Exclude3, "VPN Adapter Exclude3");

	GetMyRegStr("VPN", "VPN_Exclude4", strVPN_Exclude4, 32, "");
	SetMyRegStr("VPN", "VPN_Exclude4", strVPN_Exclude4);
	if (strlen(strVPN_Exclude4) == 0) strcpy(strVPN_Exclude4, "VPN Adapter Exclude4");

	GetMyRegStr("VPN", "VPN_Exclude5", strVPN_Exclude5, 32, "");
	SetMyRegStr("VPN", "VPN_Exclude5", strVPN_Exclude5);
	if (strlen(strVPN_Exclude5) == 0) strcpy(strVPN_Exclude5, "VPN Adapter Exclude5");





	GetMyRegStr("ETC", "Ethernet_Keyword1", strEthernet_Keyword1, 32, "");
	SetMyRegStr("ETC", "Ethernet_Keyword1", strEthernet_Keyword1);
	if (strlen(strEthernet_Keyword1) == 0) strcpy(strEthernet_Keyword1, "Ethernet Adapter Keyword1");

	GetMyRegStr("ETC", "Ethernet_Keyword2", strEthernet_Keyword2, 32, "");
	SetMyRegStr("ETC", "Ethernet_Keyword2", strEthernet_Keyword2);
	if (strlen(strEthernet_Keyword2) == 0) strcpy(strEthernet_Keyword2, "Ethernet Adapter Keyword2");

	GetMyRegStr("ETC", "Ethernet_Keyword3", strEthernet_Keyword3, 32, "");
	SetMyRegStr("ETC", "Ethernet_Keyword3", strEthernet_Keyword3);
	if (strlen(strEthernet_Keyword3) == 0) strcpy(strEthernet_Keyword3, "Ethernet Adapter Keyword3");

	GetMyRegStr("ETC", "Ethernet_Keyword4", strEthernet_Keyword4, 32, "");
	SetMyRegStr("ETC", "Ethernet_Keyword4", strEthernet_Keyword4);
	if (strlen(strEthernet_Keyword4) == 0) strcpy(strEthernet_Keyword4, "Ethernet Adapter Keyword4");

	GetMyRegStr("ETC", "Ethernet_Keyword5", strEthernet_Keyword5, 32, "");
	SetMyRegStr("ETC", "Ethernet_Keyword5", strEthernet_Keyword5);
	if (strlen(strEthernet_Keyword5) == 0) strcpy(strEthernet_Keyword5, "Ethernet Adapter Keyword5");

	//CompactMode Added by TTTT
	b_CompactMode = GetMyRegLong(NULL, "CompactMode", TRUE);
	SetMyRegLong(NULL, "CompactMode", b_CompactMode);


	//SafeMode operation Added by TTTT
	b_SafeMode = GetMyRegLong("Status_DoNotEdit", "SafeMode", FALSE);
	if (b_DebugLog) writeDebugLog_Win10("[tclock.c][ReadData] b_SafeMode was retrieved from tclock-win10.ini as ", b_SafeMode);

	if (b_SafeMode)
	{
		b_CompactMode = TRUE;
	}

	GetMyRegStr("ETC", "LTEString", strLTE, 32, "LTE");
	if (strlen(strLTE) == 0) strcpy(strLTE, "LTE");
	SetMyRegStr("ETC", "LTEString", strLTE);

	GetMyRegStr("ETC", "LTEChar", strBuf, 32, "L");
	if (strlen(strBuf) == 0)
	{
		strcpy(charLTE, "L");
	}
	else
	{
		strncpy(charLTE, strBuf, 1);
	}
	SetMyRegStr("ETC", "LTEChar", charLTE);


	GetMyRegStr("ETC", "MuteString", strMute, 32, "*");
	if (strlen(strMute) == 0) strcpy(strMute, "*");
	SetMyRegStr("ETC", "MuteString", strMute);




	int BarMeterWidth;
	int BarMeterHeight;
	int BarMeterSpacing;

	
	b_BarMeterVL_Horizontal = GetMyRegLong("BarMeter", "BarMeterVL_Horizontal", FALSE);
	SetMyRegLong("BarMeter", "BarMeterVL_Horizontal", b_BarMeterVL_Horizontal);

	b_BarMeterBL_Horizontal = GetMyRegLong("BarMeter", "BarMeterBL_Horizontal", FALSE);
	SetMyRegLong("BarMeter", "BarMeterBL_Horizontal", b_BarMeterBL_Horizontal);
	
	b_BarMeterCU_Horizontal = GetMyRegLong("BarMeter", "BarMeterCU_Horizontal", FALSE);
	SetMyRegLong("BarMeter", "BarMeterCU_Horizontal", b_BarMeterCU_Horizontal);
	
	b_BarMeterNet_Horizontal = GetMyRegLong("BarMeter", "BarMeterNet_Horizontal", FALSE);
	SetMyRegLong("BarMeter", "BarMeterNet_Horizontal", b_BarMeterNet_Horizontal);

	
	b_BarMeterVL_HorizontalToLeft = GetMyRegLong("BarMeter", "BarMeterVL_HorizontalToLeft", FALSE);
	SetMyRegLong("BarMeter", "BarMeterVL_HorizontalToLeft", b_BarMeterVL_HorizontalToLeft);

	b_BarMeterBL_HorizontalToLeft = GetMyRegLong("BarMeter", "BarMeterBL_HorizontalToLeft", FALSE);
	SetMyRegLong("BarMeter", "BarMeterBL_HorizontalToLeft", b_BarMeterBL_HorizontalToLeft);

	b_BarMeterCU_HorizontalToLeft = GetMyRegLong("BarMeter", "BarMeterCU_HorizontalToLeft", FALSE);
	SetMyRegLong("BarMeter", "BarMeterCU_HorizontalToLeft", b_BarMeterCU_HorizontalToLeft);

	b_BarMeterNet_HorizontalToLeft = GetMyRegLong("BarMeter", "BarMeterNet_HorizontalToLeft", FALSE);
	SetMyRegLong("BarMeter", "BarMeterNet_HorizontalToLeft", b_BarMeterNet_HorizontalToLeft);



	b_UseBarMeterVL = GetMyRegLong("BarMeter", "UseBarMeterVL", 0);
	SetMyRegLong("BarMeter", "UseBarMeterVL", b_UseBarMeterVL);


	ColorBarMeterVL = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterVL", RGB(0, 255, 0));
	SetMyRegLong("BarMeter", "ColorBarMeterVL", ColorBarMeterVL);

	ColorBarMeterVL_Mute = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterVL_Mute", RGB(255, 0, 0));
	SetMyRegLong("BarMeter", "ColorBarMeterVL_Mute", ColorBarMeterVL_Mute);

	BarMeterVL_Right = (int)(short)GetMyRegLong("BarMeter", "BarMeterVL_Right", 290);
	SetMyRegLong("BarMeter", "BarMeterVL_Right", BarMeterVL_Right);

	BarMeterWidth = (int)(short)GetMyRegLong("BarMeter", "BarMeterVL_Width", 5);
	if (BarMeterWidth <= 0) BarMeterWidth = 5;
	BarMeterVL_Left = BarMeterVL_Right + BarMeterWidth;
	SetMyRegLong("BarMeter", "BarMeterVL_Width", BarMeterWidth);

	BarMeterVL_Bottom = (int)(short)GetMyRegLong("BarMeter", "BarMeterVL_Bottom", 0);
	if (BarMeterVL_Bottom < 0) BarMeterVL_Bottom = 0;
	SetMyRegLong("BarMeter", "BarMeterVL_Bottom", BarMeterVL_Bottom);

	BarMeterVL_Top = (int)(short)GetMyRegLong("BarMeter", "BarMeterVL_Top", 0);
	if (BarMeterVL_Top < 0) BarMeterVL_Top = 0;
	SetMyRegLong("BarMeter", "BarMeterVL_Top", BarMeterVL_Top);



	b_UseBarMeterBL = GetMyRegLong("BarMeter", "UseBarMeterBL", 0);
	SetMyRegLong("BarMeter", "UseBarMeterBL", b_UseBarMeterBL);


	ColorBarMeterBL_Charge = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterBL_Charge", RGB(255, 165, 0));
	SetMyRegLong("BarMeter", "ColorBarMeterBL_Charge", ColorBarMeterBL_Charge);

	ColorBarMeterBL_High = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterBL_High", RGB(0, 255, 0));
	SetMyRegLong("BarMeter", "ColorBarMeterBL_High", ColorBarMeterBL_High);

	ColorBarMeterBL_Mid = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterBL_Mid", RGB(255, 255, 0));
	SetMyRegLong("BarMeter", "ColorBarMeterBL_Mid", ColorBarMeterBL_Mid);

	ColorBarMeterBL_Low = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterBL_Low", RGB(255, 0, 0));
	SetMyRegLong("BarMeter", "ColorBarMeterBL_Low", ColorBarMeterBL_Low);

	BarMeterBL_Right = (int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Right", 210);
	SetMyRegLong("BarMeter", "BarMeterBL_Right", BarMeterBL_Right);	
	
	BarMeterWidth = (int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Width", 5);
	if (BarMeterWidth <= 0) BarMeterWidth = 5;
	BarMeterBL_Left = BarMeterBL_Right + BarMeterWidth;
	SetMyRegLong("BarMeter", "BarMeterBL_Width", BarMeterWidth);


	BarMeterBL_Bottom = (int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Bottom", 0);
	if (BarMeterBL_Bottom < 0) BarMeterBL_Bottom = 0;
	SetMyRegLong("BarMeter", "BarMeterBL_Bottom", BarMeterBL_Bottom);

	BarMeterBL_Top = (int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Top", 0);
	if (BarMeterBL_Top < 0) BarMeterBL_Top = 0;
	SetMyRegLong("BarMeter", "BarMeterBL_Top", BarMeterBL_Top);


	BarMeterBL_Threshold_High = (int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Threshold_High", 50);
	if (BarMeterBL_Threshold_High < 0) BarMeterBL_Threshold_High = 0;
	if (BarMeterBL_Threshold_High > 101) BarMeterBL_Threshold_High = 101;
	SetMyRegLong("BarMeter", "BarMeterBL_Threshold_High", BarMeterBL_Threshold_High);

	BarMeterBL_Threshold_Mid = (int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Threshold_Mid", 20);
	if (BarMeterBL_Threshold_Mid < 0) BarMeterBL_Threshold_Mid = 0;
	if (BarMeterBL_Threshold_Mid > 101) BarMeterBL_Threshold_Mid = 101;
	SetMyRegLong("BarMeter", "BarMeterBL_Threshold_Mid", BarMeterBL_Threshold_Mid);



	b_UseBarMeterCU = GetMyRegLong("BarMeter", "UseBarMeterCU", 0);
	SetMyRegLong("BarMeter", "UseBarMeterCU", b_UseBarMeterCU);

	ColorBarMeterCU_High = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterCU_High", RGB(255, 0, 0));
	SetMyRegLong("BarMeter", "ColorBarMeterCU_High", ColorBarMeterCU_High);

	ColorBarMeterCU_Mid = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterCU_Mid", RGB(255, 255, 0));
	SetMyRegLong("BarMeter", "ColorBarMeterCU_Mid", ColorBarMeterCU_Mid);

	ColorBarMeterCU_Low = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterCU_Low", RGB(0, 255, 0));
	SetMyRegLong("BarMeter", "ColorBarMeterCU_Low", ColorBarMeterCU_Low);


	BarMeterCU_Right = (int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Right", 170);
	SetMyRegLong("BarMeter", "BarMeterCU_Right", BarMeterCU_Right);

	BarMeterWidth = (int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Width", 5);
	if (BarMeterWidth <= 0) BarMeterWidth = 5;
	BarMeterCU_Left = BarMeterCU_Right + BarMeterWidth;
	SetMyRegLong("BarMeter", "BarMeterCU_Width", BarMeterWidth);

	BarMeterCU_Bottom = (int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Bottom", 0);
	if (BarMeterCU_Bottom < 0) BarMeterCU_Bottom = 0;
	SetMyRegLong("BarMeter", "BarMeterCU_Bottom", BarMeterCU_Bottom);

	BarMeterCU_Top = (int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Top", 0);
	if (BarMeterCU_Top < 0) BarMeterCU_Top = 0;
	SetMyRegLong("BarMeter", "BarMeterCU_Top", BarMeterCU_Top);


	BarMeterCU_Threshold_High = (int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Threshold_High", 70);
	if (BarMeterCU_Threshold_High < 0) BarMeterCU_Threshold_High = 0;
	if (BarMeterCU_Threshold_High > 101) BarMeterCU_Threshold_High = 101;
	SetMyRegLong("BarMeter", "BarMeterCU_Threshold_High", BarMeterCU_Threshold_High);

	BarMeterCU_Threshold_Mid = (int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Threshold_Mid", 50);
	if (BarMeterCU_Threshold_Mid < 0) BarMeterCU_Threshold_Mid = 0;
	if (BarMeterCU_Threshold_Mid > 101) BarMeterCU_Threshold_Mid = 101;
	SetMyRegLong("BarMeter", "BarMeterCU_Threshold_Mid", BarMeterCU_Threshold_Mid);




	b_UseBarMeterGU = GetMyRegLong("BarMeter", "UseBarMeterGU", 0);
	SetMyRegLong("BarMeter", "UseBarMeterGU", b_UseBarMeterGU);

	BarMeterGU_Right = (int)(short)GetMyRegLong("BarMeter", "BarMeterGU_Right", 175);
	SetMyRegLong("BarMeter", "BarMeterGU_Right", BarMeterGU_Right);

	BarMeterWidth = (int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Width", 5);
	BarMeterGU_Left = BarMeterGU_Right + BarMeterWidth;

	BarMeterGU_Bottom = (int)(short)GetMyRegLong("BarMeter", "BarMeterGU_Bottom", 0);
	if (BarMeterGU_Bottom < 0) BarMeterGU_Bottom = 0;
	SetMyRegLong("BarMeter", "BarMeterGU_Bottom", BarMeterGU_Bottom);

	BarMeterGU_Top = (int)(short)GetMyRegLong("BarMeter", "BarMeterGU_Top", 0);
	if (BarMeterGU_Top < 0) BarMeterGU_Top = 0;
	SetMyRegLong("BarMeter", "BarMeterGU_Top", BarMeterGU_Top);


	ColorBarMeterGPU = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterGPU", ColorGPUGraph);
	SetMyRegLong("BarMeter", "ColorBarMeterGPU", ColorBarMeterGPU);








	b_UseBarMeterCore = GetMyRegLong("BarMeter", "UseBarMeterCore", 0);
	SetMyRegLong("BarMeter", "UseBarMeterCore", b_UseBarMeterCore);

	NumberBarMeterCore = (int)(short)GetMyRegLong("BarMeter", "NumberBarMeterCore", 8);
	if (NumberBarMeterCore < 0)
	{
		NumberBarMeterCore = 0;
		b_UseBarMeterCore = FALSE;
	}
	SetMyRegLong("BarMeter", "NumberBarMeterCore", NumberBarMeterCore);


	ColorBarMeterCore_High = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterCore_High", RGB(255, 0, 0));
	SetMyRegLong("BarMeter", "ColorBarMeterCore_High", ColorBarMeterCore_High);

	ColorBarMeterCore_Mid = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterCore_Mid", RGB(255, 255, 0));
	SetMyRegLong("BarMeter", "ColorBarMeterCore_Mid", ColorBarMeterCore_Mid);

	ColorBarMeterCore_Low = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterCore_Low", RGB(0, 255, 0));
	SetMyRegLong("BarMeter", "ColorBarMeterCore_Low", ColorBarMeterCore_Low);



	BarMeterWidth = (int)(short)GetMyRegLong("BarMeter", "BarMeterCore_Width", 5);
	BarMeterSpacing = (int)(short)GetMyRegLong("BarMeter", "BarMeterCore_Spacing", 2);
	BarMeterCore_Left = (int)(short)GetMyRegLong("BarMeter", "BarMeterCore_Left", 0);

	if (BarMeterWidth <= 0) BarMeterWidth = BarMeterCU_Left - BarMeterCU_Right;
	if (BarMeterSpacing < 0) BarMeterSpacing = 0;

	SetMyRegLong("BarMeter", "BarMeterCore_Width", BarMeterWidth);
	SetMyRegLong("BarMeter", "BarMeterCore_Spacing", BarMeterSpacing);
	SetMyRegLong("BarMeter", "BarMeterCore_Left", BarMeterCore_Left);
	
	if (BarMeterCore_Left == 0)
	{
		BarMeterCore_Left = BarMeterCU_Right - BarMeterSpacing;
	}
	BarMeterCore_Right = BarMeterCore_Left - BarMeterWidth;
	BarMeterCore_Pitch = BarMeterWidth + BarMeterSpacing;

	BarMeterCore_Bottom = BarMeterCU_Bottom;
	BarMeterCore_Top = BarMeterCU_Top;







	b_UseBarMeterNet = GetMyRegLong("BarMeter", "UseBarMeterNet", 0);
	SetMyRegLong("BarMeter", "UseBarMeterNet", b_UseBarMeterNet);

	b_BarMeterNet_LogGraph = GetMyRegLong("BarMeter", "BarMeterNet_LogGraph", 0);
	SetMyRegLong("BarMeter", "BarMeterNet_LogGraph", b_BarMeterNet_LogGraph);

	ColorBarMeterNet_Send = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterNet_Send", ColSend);
	SetMyRegLong("BarMeter", "ColorBarMeterNet_Send", ColorBarMeterNet_Send);


	ColorBarMeterNet_Recv = (COLORREF)GetMyRegLong("BarMeter", "ColorBarMeterNet_Recv", ColRecv);
	SetMyRegLong("BarMeter", "ColorBarMeterNet_Recv", ColorBarMeterNet_Recv);

	BarMeterNetRecv_Right = (int)(short)GetMyRegLong("BarMeter", "BarMeterNetRecv_Right", 300);
	SetMyRegLong("BarMeter", "BarMeterNetRecv_Right", BarMeterNetRecv_Right);


	BarMeterNetRecv_Bottom = (int)(short)GetMyRegLong("BarMeter", "BarMeterNetRecv_Bottom", 0);
	if (BarMeterNetRecv_Bottom < 0) BarMeterNetRecv_Bottom = 0;
	SetMyRegLong("BarMeter", "BarMeterNetRecv_Bottom", BarMeterNetRecv_Bottom);

	BarMeterNetSend_Right = (int)(short)GetMyRegLong("BarMeter", "BarMeterNetSend_Right", 310);
	SetMyRegLong("BarMeter", "BarMeterNetSend_Right", BarMeterNetSend_Right);


	BarMeterNetSend_Bottom = (int)(short)GetMyRegLong("BarMeter", "BarMeterNetSend_Bottom", 0);
	if (BarMeterNetSend_Bottom < 0) BarMeterNetSend_Bottom = 0;
	SetMyRegLong("BarMeter", "BarMeterNetSend_Bottom", BarMeterNetSend_Bottom);

	BarMeterWidth = (int)(short)GetMyRegLong("BarMeter", "BarMeterNet_Width", 5);
	if (BarMeterWidth <= 0) BarMeterWidth = 5;
	BarMeterNetRecv_Left = BarMeterNetRecv_Right + BarMeterWidth;
	BarMeterNetSend_Left = BarMeterNetSend_Right + BarMeterWidth;
	SetMyRegLong("BarMeter", "BarMeterNet_Width", BarMeterWidth);

	BarMeterNetRecv_Top = (int)(short)GetMyRegLong("BarMeter", "BarMeterNetRecv_Top", 0);
	if (BarMeterNetRecv_Top < 0) BarMeterNetRecv_Top = 0;
	SetMyRegLong("BarMeter", "BarMeterNetRecv_Top", BarMeterNetRecv_Top);


	BarMeterNetSend_Top = (int)(short)GetMyRegLong("BarMeter", "BarMeterNetSend_Top", 0);
	if (BarMeterNetSend_Top < 0) BarMeterNetSend_Top = 0;
	SetMyRegLong("BarMeter", "BarMeterNetSend_Top", BarMeterNetSend_Top);

	SetMyRegLong("Status_DoNotEdit", "BatteryLifeAvailable", 1);

	NetMIX_Length = GetMyRegLong("ETC", "NetMIX_Length", 10);
	SetMyRegLong("ETC", "NetMIX_Length", NetMIX_Length);

	SSID_AP_Length = GetMyRegLong("ETC", "SSID_AP_Length", 10);
	SetMyRegLong("ETC", "SSID_AP_Length", SSID_AP_Length);



	strAdditionalMountPath[10][64];

	for (int i = 0; i < 10; i++) {
		sprintf(strTemp, "AdditionalMountPath%1d", i);
		GetMyRegStr("ETC", strTemp, strAdditionalMountPath[i], 64, "");
		SetMyRegStr("ETC", strTemp, strAdditionalMountPath[i]);
	}



	AdjustThreshold = (int)(short)GetMyRegLong(NULL, "AdjustThreshold", 200);
	if (AdjustThreshold < 100) AdjustThreshold = 100;
	if (AdjustThreshold > 500) AdjustThreshold = 500;
	SetMyRegLong(NULL, "AdjustThreshold", AdjustThreshold);


	TimerCountForSec = (int)(short)GetMyRegLong("Status_DoNotEdit", "TimerCountForSec", 1000);
	if (AdjustThreshold > 400) TimerCountForSec = 1000;
	else if (TimerCountForSec > 1030) TimerCountForSec = 1030;
	else if (TimerCountForSec < 970) TimerCountForSec = 970;
	SetMyRegLong("Status_DoNotEdit", "TimerCountForSec", TimerCountForSec);


	adjustWin11TrayCutPosition = (int)(short)GetMyRegLong("Win11", "AdjustCutTray", 0);
	SetMyRegLong("Win11", "AdjustCutTray", adjustWin11TrayCutPosition);

	adjustWin11ClockWidth = (int)(short)GetMyRegLong("Win11", "AdjustWin11ClockWidth", 0);
	SetMyRegLong("Win11", "AdjustWin11ClockWidth", adjustWin11ClockWidth);

	adjustWin11DetectNotify = (int)(short)GetMyRegLong("Win11", "AdjustDetectNotify", 0);
	SetMyRegLong("Win11", "AdjustDetectNotify", adjustWin11DetectNotify);

	bUseWin11Notify = GetMyRegLong("Win11", "UseTClockNotify", 1);
	SetMyRegLong("Win11", "UseTClockNotify", bUseWin11Notify);
	if (bUseWin11Notify && bWin11Main && hwndWin11Notify) {
		bEnabledWin11Notify = TRUE;
	}
	else
	{
		bEnabledWin11Notify = FALSE;
	}

	bAdjustTrayWin11SmallTaskbar = (BOOL)GetMyRegLong("Win11", "AdjustWin11IconPosition", 1);
	SetMyRegLong("Win11", "AdjustWin11IconPosition", bAdjustTrayWin11SmallTaskbar);


	bShowWin11NotifyNumber = (BOOL)GetMyRegLong("Win11", "ShowWin11NotifyNumber", 1);
	SetMyRegLong("Win11", "ShowWin11NotifyNumber", bShowWin11NotifyNumber);




	b_EnableChime = GetMyRegLong("Chime", "EnableChime", 0);
	SetMyRegLong("Chime", "EnableChime", b_EnableChime);

	//intVolChime = (int)(short)GetMyRegLong("Chime", "VolChime", 100);
	//SetMyRegLong("Chime", "VolChime", intVolChime);

	intOffsetChimeSec = (int)(short)GetMyRegLong("Chime", "OffsetChimeSec", 0);
	SetMyRegLong("Chime", "OffsetChimeSec", intOffsetChimeSec);

	chimeHourStart = (int)(short)GetMyRegLong("Chime", "ChimeHourStart", 0);
	if ((chimeHourStart < 0) || (chimeHourStart > 23)) chimeHourStart = 0;
	SetMyRegLong("Chime", "ChimeHourStart", chimeHourStart);

	chimeHourEnd = (int)(short)GetMyRegLong("Chime", "ChimeHourEnd", 24);
	if ((chimeHourEnd < 1) || (chimeHourEnd > 24)) chimeHourEnd = 0;
	SetMyRegLong("Chime", "ChimeHourEnd", chimeHourEnd);

	GetMyRegStr("Chime", "ChimeWav", fname, MAX_PATH, "C:\\Windows\\Media\\notify.wav");
	if (GetFileAttributes(fname) != 0xFFFFFFFF)
	{
		lstrcpy(szChimeWav, fname);
	}
	else {
		lstrcpy(szChimeWav, "C:\\Windows\\Media\\notify.wav");
	}
	SetMyRegStr("Chime", "ChimeWav", szChimeWav);

	b_EnableBlinkOnChime = GetMyRegLong("Chime", "EnableBlinkOnChime", 0);
	SetMyRegLong("Chime", "EnableBlinkOnChime", b_EnableBlinkOnChime);

	BlinksOnChime = (int)(short)GetMyRegLong("Chime", "BlinksOnChime", 3);
	SetMyRegLong("Chime", "BlinksOnChime", BlinksOnChime);

	b_EnableSecondaryChime = GetMyRegLong("Chime", "EnableSecondaryChime", 0);
	SetMyRegLong("Chime", "EnableSecondaryChime", b_EnableSecondaryChime);

	b_CuckooClock = GetMyRegLong("Chime", "CuckooClock", 0);
	SetMyRegLong("Chime", "CuckooClock", b_CuckooClock);

	intOffsetSecondaryChimeSec = (int)(short)GetMyRegLong("Chime", "OffsetSecondaryChimeSec", 1800);
	SetMyRegLong("Chime", "OffsetSecondaryChimeSec", intOffsetSecondaryChimeSec);

	GetMyRegStr("Chime", "SecondaryChimeWav", fname, MAX_PATH, "C:\\Windows\\Media\\chimes.wav");
	if (GetFileAttributes(fname) != 0xFFFFFFFF)
	{
		lstrcpy(szSecondaryChimeWav, fname);
	}
	else {
		lstrcpy(szSecondaryChimeWav, "C:\\Windows\\Media\\chimes.wav");
	}
	SetMyRegStr("Chime", "SecondaryChimeWav", szSecondaryChimeWav);



	indexSelectedThermalZone = GetMyRegLong("ETC", "SelectedThermalZone", 0);
	SetMyRegLong("ETC", "SelectedThermalZone", indexSelectedThermalZone);


	if(bGraphTimerStart) KillTimer(hwndClockMain, IDTIMERDLL_GRAPH); bGraphTimerStart = FALSE;
	if(bGraph)
		bGraphTimerStart = SetTimer(hwndClockMain, IDTIMERDLL_GRAPH, graphInterval*1000, NULL) != 0;

	GetMyRegStr("Format", "Format", format, 1024, "mm/dd ddd\\n hh:nn:ss ");

	// ���v�̏����͓ǂݍ��ݎ���<%�`%>��ǉ�����
	{
		char fmt_tmp[1024];
		strcpy(fmt_tmp,"<%");
		strcat(fmt_tmp,format);
		strcat(fmt_tmp,"%>");
		strcpy(format,fmt_tmp);
	}


	dwInfoFormat = FindFormat(format);
	bDispSecond = (dwInfoFormat&FORMAT_SECOND)? TRUE:FALSE;
	nDispBeat = dwInfoFormat & (FORMAT_BEAT1 | FORMAT_BEAT2);

	bHour12 = GetMyRegLong("Format", "Hour12", 0);
	bHourZero = GetMyRegLong("Format", "HourZero", 0);

	GetLocalTime(&lt);
	LastTime.wDay = lt.wDay;
	b_DayChange = TRUE;

	InitFormat(&lt);      // format.c

	//iClockWidth = -1;

	InitSysInfo();


	//Start Timers
	intervalTimerAdjust = 10;		// Enable first adjustment in 3 sec, added by TTTT	
	b_InitialTimerAdjust = TRUE;	// Do not change TimerCountForSec in the  first adjustment, added by TTTT
	SetTimer(hwndClockMain, IDTIMERDLL_DLLMAIN, TimerCountForSec, NULL);
	bTimer = TRUE;
	//Following Timers will be adjusted in the first call of the timeout functions
	//if (bTimerSysInfo) KillTimer(hwndClockMain, IDTIMERDLL_SYSINFO);
	//if (bTimerCheckNetStat) KillTimer(hwndClockMain, IDTIMERDLL_CHECKNETSTAT);

	SetTimer(hwndClockMain, IDTIMERDLL_SYSINFO, TimerCountForSec + OFFSETMS_TIMER_SYSINFO, NULL);
	bTimerSysInfo = TRUE;
	bTimerAdjust_SysInfo = TRUE;
	SetTimer(hwndClockMain, IDTIMERDLL_CHECKNETSTAT, TimerCountForSec + OFFSETMS_TIMER_NETSTAT, NULL);
	bTimerCheckNetStat = TRUE;
	bTimerAdjust_NetStat = TRUE;

	CleanSettingFile();

}

void InitSysInfo()
{
	DWORD dwInfoFormat, dwInfoTip;

	//if(bTimerSysInfo) KillTimer(hwndClockMain, IDTIMERDLL_SYSINFO);	// deactivated by TTTT
	bDispSysInfo = bTimerSysInfo = FALSE;

	bGetBattery = bGetMem = FALSE;
	memset(&msMemory, 0, sizeof(msMemory));

	dwInfoFormat = FindFormat(format);
	dwInfoTip = TooltipFindFormat();

	//bGetBattery = ((dwInfoFormat | dwInfoTip) & FORMAT_BATTERY) ? TRUE : FALSE;
	//if (dwInfoFormat & FORMAT_BATTERY) bDispSysInfo = TRUE;
	//if (b_UseBarMeterBL) bGetBattery = TRUE;	//Added by TTTT 2011028

	bGetBattery = TRUE; //Modified to check battery availability as default. Modified by TTTT

	bGetMem = ((dwInfoFormat | dwInfoTip) & FORMAT_MEMORY)? TRUE:FALSE;
	if(dwInfoFormat & FORMAT_MEMORY) bDispSysInfo = TRUE;

	bGetNet = (((dwInfoFormat | dwInfoTip) & FORMAT_NET)||(bGraph&&graphMode==1))? TRUE:FALSE;
	if(dwInfoFormat & FORMAT_NET) bDispSysInfo = TRUE;

	bGetHdd = ((dwInfoFormat | dwInfoTip) & FORMAT_HDD)? TRUE:FALSE;
	if(dwInfoFormat & FORMAT_HDD) bDispSysInfo = TRUE;

	bGetCpu = ((dwInfoFormat | dwInfoTip) & FORMAT_CPU)? TRUE:FALSE;
	if(dwInfoFormat & FORMAT_CPU) bDispSysInfo = TRUE;
	if (b_UseBarMeterCU) bGetCpu = TRUE;	//Added by TTTT 2011028
	if (bGraph && (graphMode == 2)) bGetCpu = TRUE;

	bGetVol = ((dwInfoFormat | dwInfoTip) & FORMAT_VOL)? TRUE:FALSE;
	if(dwInfoFormat & FORMAT_VOL) bDispSysInfo = TRUE;

	bGetGpu = ((dwInfoFormat | dwInfoTip) & FORMAT_GPU) ? TRUE : FALSE;
	if (dwInfoFormat & FORMAT_GPU) bDispSysInfo = TRUE;
	if (b_UseBarMeterGU) bGetGpu = TRUE;	//Added by TTTT 2011028
	if (bGraph && bEnableGPUGraph && (graphMode == 2)) bGetGpu = TRUE;

	bGetTemp = ((dwInfoFormat | dwInfoTip) & FORMAT_TEMP) ? TRUE : FALSE;
	if (dwInfoFormat & FORMAT_TEMP) bDispSysInfo = TRUE;

	if(bGetBattery || bGetMem || bGetNet || bGetHdd || bGetCpu || bGetVol || bGetGpu||bGetTemp)
	{
		UpdateSysRes(bGetBattery, bGetMem, bGetNet, bGetHdd, bGetCpu, bGetVol, bGetGpu, bGetTemp);
	}

}

/*------------------------------------------------
�@�`��p������DC�̍쐬
 hdcClock��hdcClock_work�����(��蒼��)
--------------------------------------------------*/
void CreateClockDC(void)
{
	RECT tempRect;
	//COLORREF col;
	HDC hdc;
	HWND tempHwnd;
	char s[1024];
	//int tempWidth, tempHeight;


	if (b_DebugLog)writeDebugLog_Win10("[tclock.c] CreateClockDC called", 999);


	CalcMainClockSize();

	//���ł�hdcClock������ꍇ�ɂ́A�����B
	if(hdcClock)
	{
		DeleteDC(hdcClock);
		hdcClock = NULL;
	}

	if (hdcClock_work)
	{
		DeleteDC(hdcClock_work);
		hdcClock_work = NULL;
	}

	//���ł�hbm_DIBSection�����݂���ꍇ�ɂ́A�N���A���ď����B
	if (hbm_DIBSection) {
		DeleteObject(hbm_DIBSection);
		hbm_DIBSection = NULL;
	}

	if (hbm_DIBSection_work) {
		DeleteObject(hbm_DIBSection_work);
		hbm_DIBSection_work = NULL;
	}



	tempHwnd = hwndClockMain;

	hdc = GetDC(tempHwnd);

	hdcClock = CreateCompatibleDC(hdc);
	if(!hdcClock)	//���s�����ꍇ�͒��߂�c�N����Ȃ��͂��B
	{
		if (b_DebugLog)writeDebugLog_Win10("[tclock.c][CreateClockDC] Failed to create hdcClock!", 999);
		ReleaseDC(tempHwnd, hdc);
		return;
	}

	hdcClock_work = CreateCompatibleDC(hdcClock);
	if (!hdcClock_work)
	{
		ReleaseDC(tempHwnd, hdc);
		return;
	}	

	static BITMAPINFO bmi = { { sizeof(BITMAPINFO),0,0,1,32,BI_RGB }, };
	bmi.bmiHeader.biWidth = widthMainClockFrame;
	bmi.bmiHeader.biHeight = heightMainClockFrame;
	bmi_MainClock = bmi;

	hbm_DIBSection = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&m_color_start, NULL, 0);
	m_color_end = m_color_start + (widthMainClockFrame * heightMainClockFrame);
	SelectObject(hdcClock, hbm_DIBSection);

	hbm_DIBSection_work = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&m_color_work_start, NULL, 0);
	m_color_work_end = m_color_work_start + (widthMainClockFrame * heightMainClockFrame);
	SelectObject(hdcClock_work, hbm_DIBSection_work);



	SetTClockFont();

	if (nTextPos == 1)
		SetTextAlign(hdcClock, TA_LEFT | TA_TOP);
	else if (nTextPos == 2)
		SetTextAlign(hdcClock, TA_RIGHT | TA_TOP);
	else
		SetTextAlign(hdcClock, TA_CENTER | TA_TOP);

	SetBkMode(hdcClock, TRANSPARENT);





	if (nTextPos == 1)
		SetTextAlign(hdcClock_work, TA_LEFT | TA_TOP);
	else if (nTextPos == 2)
		SetTextAlign(hdcClock_work, TA_RIGHT | TA_TOP);
	else
		SetTextAlign(hdcClock_work, TA_CENTER | TA_TOP);

	SetBkMode(hdcClock_work, TRANSPARENT);


	ReleaseDC(tempHwnd, hdc);
}



/*------------------------------------------------
   get date/time and beat to display
--------------------------------------------------*/
void GetDisplayTime(SYSTEMTIME* pt, int* beat100)
{
	FILETIME ft, lft;
	SYSTEMTIME lt;
	DWORDLONG tempDWL;

	GetSystemTimeAsFileTime(&ft);

	tempDWL = ((ULONGLONG)ft.dwHighDateTime << 32) + ft.dwLowDateTime;
	tempDWL += (offsetClockMS * 10000);


	ft.dwLowDateTime = (DWORD)tempDWL;
	ft.dwHighDateTime = (DWORD)(tempDWL >> 32);

	if(beat100)
	{
		DWORDLONG dl;
		SYSTEMTIME st;
		int sec;

		dl = *(DWORDLONG*)&ft + 36000000000;
		FileTimeToSystemTime((FILETIME*)&dl, &st);

		sec = st.wHour * 3600 + st.wMinute * 60 + st.wSecond;
		*beat100 = (sec * 1000) / 864;
	}

	FileTimeToLocalFileTime(&ft, &lft);
	FileTimeToSystemTime(&lft, &lt);
	memcpy(pt, &lt, sizeof(lt));
}

void PlayChime(BOOL b_sedondary)
{
	if (!b_sedondary) 
	{
		if (GetFileAttributes(szChimeWav) != 0xFFFFFFFF)
		{
			PlaySound(szChimeWav, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		}
	}
	else 
	{
		if (GetFileAttributes(szSecondaryChimeWav) != 0xFFFFFFFF)
		{
			PlaySound(szSecondaryChimeWav, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		}
	}
}

/*------------------------------------------------
�@WM_TIMER �̏��� New Version for TClock-Win10
 --------------------------------------------------*/
void OnTimer_Win10(void)
{
	SYSTEMTIME t;
	int beat100 = 0, tempInt = 0;
	//HDC hdc;
	BOOL bRedraw;


	if (Win11Type == 2) {
		CheckPixel_Win10(posXMainClock + 1, originalPosYTaskbar + 10);
	}






	GetDisplayTime(&t, nDispBeat ? (&beat100) : NULL);

	if (b_DebugLog)
	{
		if (t.wMilliseconds < 500) writeDebugLog_Win10("[tclock.c] OnTimer_Win10 called. Delay(ms) = ", t.wMilliseconds);
		else writeDebugLog_Win10("[tclock.c] OnTimer_Win10 called. Delay(ms) = -", (1000 - t.wMilliseconds));
	}

	intervalTimerAdjust++;


	if (t.wMilliseconds > AdjustThreshold && intervalTimerAdjust > 9) // suppress multiple adjust in short time
	{
		int ms_adjust;

		if (t.wMilliseconds < 500) ms_adjust = 1025 - t.wMilliseconds;
		else ms_adjust = 2025 - t.wMilliseconds;
		
		if (!b_InitialTimerAdjust)	//Do not change TimerCountForSec in the initial adjust.
		{
			if (t.wMilliseconds < 500)
			{
				TimerCountForSec--;
			}
			else
			{
				TimerCountForSec++;
			}
		}

		if (AdjustThreshold > 400) TimerCountForSec = 1000;
		else if (TimerCountForSec > 1030) TimerCountForSec = 1030;
		else if (TimerCountForSec < 970) TimerCountForSec = 970;


		KillTimer(hwndClockMain, IDTIMERDLL_DLLMAIN);
		SetTimer(hwndClockMain, IDTIMERDLL_DLLMAIN, ms_adjust, NULL);
		bTimerDLLMainAdjust = TRUE;
		b_FlagTimerAdjust = TRUE;

		SetMyRegLong("Status_DoNotEdit", "TimerCountForSec", TimerCountForSec);

		SetTimer(hwndClockMain, IDTIMERDLL_SYSINFO, ms_adjust + OFFSETMS_TIMER_SYSINFO, NULL);
		bTimerAdjust_SysInfo = TRUE;
		SetTimer(hwndClockMain, IDTIMERDLL_CHECKNETSTAT, ms_adjust + OFFSETMS_TIMER_NETSTAT, NULL);
		bTimerAdjust_NetStat = TRUE;

		if (b_DebugLog)
		{
			writeDebugLog_Win10("[OnTimer_Win10] IDTIMERDLL_DLLMAIN adjusted with TimerCountForSec =", TimerCountForSec);
			if (b_InitialTimerAdjust) writeDebugLog_Win10("[OnTimer_Win10] Startup Timer adjustment was performed.", 999);
				else writeDebugLog_Win10("[OnTimer_Win10] Seconds after the last Timer adjustment =", intervalTimerAdjust);
		}

		intervalTimerAdjust = 0;

	}
	else if (bTimerDLLMainAdjust)
	{
		KillTimer(hwndClockMain, IDTIMERDLL_DLLMAIN);
		bTimerDLLMainAdjust = FALSE;
		SetTimer(hwndClockMain, IDTIMERDLL_DLLMAIN, TimerCountForSec, NULL);
	}

	b_InitialTimerAdjust = FALSE;










	bRedraw = FALSE;

	if (bDispSecond) bRedraw = TRUE;
	else if (nDispBeat == FORMAT_BEAT1 && beatLast != (beat100 / 100)) bRedraw = TRUE;
	else if (nDispBeat == FORMAT_BEAT2 && beatLast != beat100) bRedraw = TRUE;
	else if (bDispSysInfo) bRedraw = TRUE;
	else if (LastTime.wHour != (int)t.wHour
		|| LastTime.wMinute != (int)t.wMinute) bRedraw = TRUE;
	else if (bGraphRedraw)
	{
		bGraphRedraw = FALSE;
		bRedraw = TRUE;
	}

	if (LastTime.wDay != t.wDay || LastTime.wMonth != t.wMonth ||
		LastTime.wYear != t.wYear)
	{
		InitFormat(&t); // format.c
		GetTimeZoneBias_Win10();
		b_DayChange = TRUE;
	}

	if (b_DayChange) {
		if (IsHoliday_Win10(&t)) {
			textcol_DoWzone = ColorHolidayText;
		}else if (t.wDayOfWeek == 0) {
			textcol_DoWzone = ColorSundayText;
		}else if (t.wDayOfWeek == 6) {
			textcol_DoWzone = ColorSaturdayText;
		}else {
			textcol_DoWzone = ColorWeekdayText;
		}
	}


	if (nBlink > 0) {
		nBlink--;
		bRedraw = TRUE;
	}

	tempInt = (3600 + t.wMinute * 60 + t.wSecond - intOffsetChimeSec) % 3600;
	if (tempInt == 0)
	{
		if (b_EnableChime) {
			BOOL bRing = FALSE;
			int tempInt = t.wHour;
			if (intOffsetChimeSec < 0) {
				tempInt++;
				if (tempInt == 24) tempInt = 0;
			}

			if ((tempInt >= chimeHourStart) && (tempInt <= chimeHourEnd)) {
				bRing = TRUE;
			}

			if (bRing) {
				if (b_CuckooClock)
				{
					nChime = (tempInt + 11) % 12 + 1;
					//if (intOffsetChimeSec < 0)
					//{
					//	nChime = t.wHour % 12 + 1;
					//}
					//else
					//{
					//	nChime = (t.wHour + 11) % 12 + 1;
					//}
				}
				else {
					nChime = 1;
				}
			}
		}
		if (b_EnableBlinkOnChime) {
			nBlink = BlinksOnChime * 2;
			bRedraw = TRUE;
		}
	}
	else
	{
		tempInt = (3600 + t.wMinute * 60 + t.wSecond - intOffsetSecondaryChimeSec) % 3600;

		if (tempInt == 0)
		{
			if (b_EnableChime && b_EnableSecondaryChime) {
				if (intOffsetSecondaryChimeSec < 0)
				{
					if (((t.wHour == 23) && (chimeHourStart == 0)) || ((chimeHourStart - t.wHour - 1) <= 0))
					{
						if ((chimeHourEnd - t.wHour) > 0)
						{
							PlayChime(TRUE);
						}
					}
				}
				else
				{
					if ((chimeHourStart - t.wHour) <= 0)
					{
						if (((chimeHourEnd - t.wHour) >= 0) || ((t.wHour == 0) && (chimeHourEnd == 24)))
						{
							PlayChime(TRUE);
						}
					}
				}
			}
		}
	}

	if (nChime > 0) {
		PlayChime(FALSE);
		nChime--;
	}


	memcpy(&LastTime, &t, sizeof(t));

	if (nDispBeat == FORMAT_BEAT1) beatLast = beat100 / 100;
	else if (nDispBeat == FORMAT_BEAT2) beatLast = beat100;



	if (bRedraw)
	{
		RedrawTClock();
	}

	//Ver 4.1�ȍ~��OnTimer_Win10����s�����ƂƂ���B
	if (bEnableTooltip) TooltipOnTimer(hwndClockMain, b_DayChange);

	//Ver4.0.4���݁Aexemain�ɂ�����OnTimerZombieCheck2�Ɠ�d�`�F�b�N�ɂȂ��Ă���̂ł����ꐮ�����K�v
	//������������̎d�g�݂͒�~���쓙�͎�������Ă��Ȃ��B
	SendStatusDLL2Main();	

	b_DayChange = FALSE;	//���t�X�V������1�񂾂�


	if (b_DebugLog) writeDebugLog_Win10("[tclock.c] OnTimer_Win10 finished.", 999);

}

/*------------------------------------------------
�@DLL�̓����Ԃ�Main�ɑ��M by TTTT
 --------------------------------------------------*/
void SendStatusDLL2Main(void)
{
	WPARAM tempwParam = 0;

	if (IsWindow(hwndTClockExeMain))
		PostMessage(hwndTClockExeMain, CLOCKM_DLLALIVE, tempwParam, 0);
}

void OnTimerUpperTaskbar(void)
{
	RECT rectTaskbar, rectWnd, rectClient;
	HWND hActiveWnd;
  


	if (!hwndTaskBarMain) return;
	if (!GetWindowRect(hwndTaskBarMain, &rectTaskbar)) return;

	hActiveWnd = GetForegroundWindow();
	if (!GetWindowRect(hActiveWnd, &rectWnd)) return;
	if (rectWnd.top == rectWnd.bottom) return;
	if (rectWnd.left == rectWnd.right) return;
	if (IsZoomed(hActiveWnd)
	 || !IsWindowVisible(hActiveWnd)
	 || (rectWnd.bottom-rectWnd.top) >= GetSystemMetrics(SM_CYFULLSCREEN)
	 || (rectWnd.right-rectWnd.left) >= GetSystemMetrics(SM_CXFULLSCREEN))
		return;
	//�u�f�X�N�g�b�v�̕\���v�ōŏ������ꂽ�E�B���h�E�𕜌������Ƃ���
	//�E�B���h�E�ʒu�����������Ȃ�����
	if (!GetClientRect(hActiveWnd, &rectClient)) return;
	if (rectClient.right == 0 && rectClient.bottom == 0) return;

	if (rectTaskbar.bottom <= (GetSystemMetrics(SM_CYSCREEN)/2))
	{
		//��^�X�N�o�[
		if (rectWnd.top < rectTaskbar.bottom)
		{
			MoveWindow(hActiveWnd, rectWnd.left, rectTaskbar.bottom,
				rectWnd.right-rectWnd.left, rectWnd.bottom-rectWnd.top, TRUE);
		}
	}
	else
	if (rectTaskbar.right <= (GetSystemMetrics(SM_CXSCREEN)/2))
	{
		//���^�X�N�o�[
		if (rectWnd.left < rectTaskbar.right)
		{
			MoveWindow(hActiveWnd, rectTaskbar.right, rectWnd.top,
				rectWnd.right-rectWnd.left, rectWnd.bottom-rectWnd.top, TRUE);
		}
	}
	else
	if (rectTaskbar.left >= (GetSystemMetrics(SM_CXSCREEN)/2))
	{
		//�E�^�X�N�o�[
		if (rectWnd.right > rectTaskbar.left)
		{
			MoveWindow(hActiveWnd, rectTaskbar.left-(rectWnd.right-rectWnd.left) , rectWnd.top,
				rectWnd.right-rectWnd.left, rectWnd.bottom-rectWnd.top, TRUE);
		}
	}
	else
	{
		//���^�X�N�o�[
		if (rectWnd.bottom > rectTaskbar.top)
		{
			MoveWindow(hActiveWnd, rectWnd.left, rectTaskbar.top-(rectWnd.bottom-rectWnd.top),
				rectWnd.right-rectWnd.left, rectWnd.bottom-rectWnd.top, TRUE);
		}
	}
}

/*------------------------------------------------
�@���v�̕`�� (2.5.0.2�ȍ~�ł͎g���ĂȂ��B
--------------------------------------------------*/
void DrawClock(HWND hwnd, HDC hdc)
{
	SYSTEMTIME t;
	int beat100 = 0;

	GetDisplayTime(&t, nDispBeat?(&beat100):NULL);
	DrawClockSub(hdc, &t, beat100);
	DrawClockFocusRect(hdc);
}

void DrawClock_New(HDC hdc, BOOL b_forceUpdateWin11Notify)
{
	SYSTEMTIME t;
	int beat100 = 0;


	//������Win11 Notification���`�悷��B
	//���Notification�������BWin11Type==2�ɂ����Ēʒm�X�V���������ꍇ�Ƀ��C���N���b�N�`������originalColorTaskbar_ForWin11Notify���X�V�����B
	if (bEnabledWin11Notify) DrawWin11Notify(b_forceUpdateWin11Notify);

	if (hdcClock) {
		GetDisplayTime(&t, nDispBeat ? (&beat100) : NULL);
		DrawClockSub(hdc, &t, beat100);
		//		DrawClockFocusRect(hdc);
		if (b_DebugLog)writeDebugLog_Win10("[tclock.c][DrawClock_New] Drawing completed.", 999);
	}
	else {
		if (b_DebugLog)writeDebugLog_Win10("[tclock.c][DrawClock_New] Draw was canceled because of No hdcClodk.", 999);
	}

}

static POINT ptMinHand[15] = {
	{  9,  1 },	//  0
	{ 10,  1 },	//  1
	{ 11,  2 },	//  2
	{ 12,  2 }, //  3
	{ 12,  3 }, //  4
	{ 13,  3 }, //  5
	{ 13,  3 }, //  6
	{ 14,  4 }, //  7
	{ 14,  4 }, //  8
	{ 15,  5 }, //  9
	{ 15,  5 }, // 10
	{ 16,  6 }, // 11
	{ 16,  6 }, // 12
	{ 17,  7 }, // 13
	{ 17,  8 }  // 14
};

static POINT ptHourHand[15] = {
	{  9,  1 },	//  0
	{ 10,  1 },	//  1
	{ 11,  2 },	//  2
	{ 12,  2 }, //  3
	{ 12,  3 }, //  4
	{ 13,  3 }, //  5
	{ 13,  3 }, //  6
	{ 14,  4 }, //  7
	{ 14,  4 }, //  8
	{ 15,  5 }, //  9
	{ 15,  5 }, // 10
	{ 16,  6 }, // 11
	{ 16,  6 }, // 12
	{ 17,  7 }, // 13
	{ 17,  8 }  // 14
};

static double ctbl[15] = {
	0.000000,
	0.104528,
	0.207912,
	0.309017,
	0.406737,
	0.500000,
	0.587785,
	0.669131,
	0.743145,
	0.809017,
	0.866025,
	0.913545,
	0.951057,
	0.978148,
	0.994522
};

static double stbl[15] = {
	0.000000,
	0.994522,
	0.978148,
	0.951057,
	0.913545,
	0.866025,
	0.809017,
	0.743145,
	0.669131,
	0.587785,
	0.500000,
	0.406737,
	0.309017,
	0.207912,
	0.104528
};

static void GetHnadLinePos(WORD index, WORD sector, POINT tbl[], POINT pos[], int dx, int dy)
{
	int cx, cy;
	int sx, sy;
	int x, y;

	x = tbl[index].x;
	y = tbl[index].y;
	cx = (sizeAClock.cx / 2);
	cy = (sizeAClock.cy / 2);
	sx = x;
	sy = y;
	switch (sector) {
	case 0:
		sx = x;
		sy = y;
		//cy += 1;
		break;
	case 1:
		sx = (sizeAClock.cx) - y;
		sy = x;
		//cx -= 1;
		break;
	case 2:
		sx = (sizeAClock.cx) - x;
		sy = (sizeAClock.cy)- y;
		//cy -= 1;
		break;
	case 3:
		sx = y;
		sy = (sizeAClock.cy) - x;
		//cx += 1;
		break;
	}
	pos[0].x = cx + dx;
	pos[0].y = cy + dy;
	pos[1].x = sx + dx;
	pos[1].y = sy + dy;
}

//�r�b�g�}�b�v�w�b�_�[�̃`�F�b�N
static BOOL CheckBitmapHeader(LPBYTE top, DWORD dwFileSize)
{
	LPBITMAPFILEHEADER lpbmfh;
	LPBITMAPINFOHEADER lpbmih;
	LPBYTE			  lpOffBits;
	RGBQUAD			 *lprgb;

	if (dwFileSize <= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) {
		// �T�C�Y�ُ�
		return FALSE;
	}

	lpbmfh = (BITMAPFILEHEADER *)top;
	if (lpbmfh->bfType != 0x4D42) { // BM
		//�w�b�_�[�ُ�
		return FALSE;
	}
	if (lpbmfh->bfSize <= sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) {
		// �T�C�Y�ُ�
		return FALSE;
	}
	lpOffBits = (LPBYTE)(top + lpbmfh->bfOffBits);
	lpbmih = (BITMAPINFOHEADER *)(top + sizeof(BITMAPFILEHEADER));
	lprgb = (RGBQUAD *)(lpbmih + sizeof(BITMAPINFOHEADER));

	if (lpbmih->biSize != sizeof(BITMAPINFOHEADER)) {
		// �r�b�g�}�b�v�C���t�H�w�b�_�[�T�C�Y�ُ�
		return FALSE;
	}

	if (lpbmih->biHeight == 0) {
		//�r�b�g�}�b�v�̍����ُ̈�
		return FALSE;
	}

	if (lpbmih->biWidth == 0) {
		//�r�b�g�}�b�v�̕��ُ̈�
		return FALSE;
	}
	switch (lpbmih->biBitCount) {
		case 1:
		case 4:
		case 8:
		case 16:
		case 24:
		case 32:
			break;
		default:
			//�s�N�Z���T�C�Y�ُ̈�
			return FALSE;
	}
	return TRUE;
}

static HBITMAP ReadBitmapData(HDC hDC, LPBYTE top, DWORD dwFileSize, SIZE *psize)
{
	LPBITMAPFILEHEADER lpbmfh;
	LPBITMAPINFO lpbmi;
	LPBITMAPINFOHEADER lpbmih;
	LPBYTE ptr;
	HBITMAP hBitmap;

	if (top == NULL) {
		return NULL;
	}
	if (!CheckBitmapHeader(top, dwFileSize)) {
		return NULL;
	}

	lpbmfh = (LPBITMAPFILEHEADER)top;
	lpbmih = (LPBITMAPINFOHEADER)(lpbmfh + 1);
	lpbmi = (LPBITMAPINFO)lpbmih;

	psize->cx = lpbmih->biWidth;
	psize->cy = lpbmih->biHeight;

	ptr = top + lpbmfh->bfOffBits;

	hBitmap = CreateDIBitmap(
		hDC,
		lpbmih,
		CBM_INIT,
		ptr,
		lpbmi,
		DIB_RGB_COLORS
	);
	if (hBitmap == NULL) {
		return NULL;
	}
	return hBitmap;
}

static HBITMAP ReadBitmapFile(HDC hDC, LPTSTR lpszFileName, SIZE *psize)
{
	HANDLE hFile;
	HANDLE hMapFile;
	LPBYTE ptr;
	DWORD dwFileSize;
	HBITMAP hBitmap;

	if (*lpszFileName == '\0') {
		return NULL;
	}
	hFile = CreateFile(
		lpszFileName,			// lpszName
		GENERIC_READ,			// fdwAccess
		FILE_SHARE_READ,		// fdwShareMode
		NULL,					// lpsa
		OPEN_EXISTING,			// fdwCreate
		FILE_ATTRIBUTE_NORMAL,	// fdwAttrsAndFlags
		NULL					// hTemplateFile
	);
	if (hFile == INVALID_HANDLE_VALUE) {
		return NULL;
	}

	dwFileSize = GetFileSize(hFile, NULL);
	if (dwFileSize == 0) {
		CloseHandle(hFile);
		return NULL;
	}

	hMapFile = CreateFileMapping(
		(HANDLE)hFile,
		NULL,
		PAGE_READONLY,
		0, 0,
		NULL
	);
	if (hMapFile == NULL) {
		CloseHandle(hFile);
		return NULL;
	}

	ptr = (LPBYTE)MapViewOfFile(
		hMapFile,
		FILE_MAP_READ,
		0, 0, 0
	);
	if (ptr == NULL) {
		CloseHandle(hMapFile);
		CloseHandle(hFile);
	}

	hBitmap = ReadBitmapData(hDC, ptr, dwFileSize, psize);

	UnmapViewOfFile(ptr);

	CloseHandle(hMapFile);

	CloseHandle(hFile);

	return hBitmap;
}

static HBITMAP Create24BitBitmap(HDC hDC, SIZE *psize, LPBYTE *ppImgBuf)
{
	HBITMAP hBitmap;
	BITMAPINFO bmi = {
		{	// bmiHeader
			sizeof(BITMAPINFOHEADER),	// DWORD	biSize;
			0,							// LONG		biWidth;
			0,							// LONG		biHeight;
			1,							// WORD		biPlanes;
			24,							// WORD		biBitCount
			BI_RGB,						// DWORD	biCompression;
			0,							// DWORD	biSizeImage;
			0,							// LONG		biXPelsPerMeter;
			0,							// LONG		biYPelsPerMeter;
			0,							// DWORD	biClrUsed;
			0							// DWORD	biClrImportant;
		}, {	// bmiColors
			{
				0x00, 0x00, 0x00
			}
		}
	};

	bmi.bmiHeader.biWidth = psize->cx;
	bmi.bmiHeader.biHeight = psize->cy;
	bmi.bmiHeader.biSizeImage = WIDTHBYTES(bmi.bmiHeader.biBitCount * psize->cx) * psize->cy;

	hBitmap = CreateDIBSection(
		hDC,
		&bmi,
		DIB_RGB_COLORS,
		ppImgBuf,
		NULL,
		0
	);
//	 GdiFlush();
	return hBitmap;
}

static VOID ModifyMaskImage(SIZE *psize, LPBYTE buf)
{
	int x, y;
	int bw;
	LPBYTE ptr;
	RGBTRIPLE *prgb;
	int cy;

	if (buf == NULL) {
		return;
	}

	bw = WIDTHBYTES(24 * psize->cx);

	ptr = buf;
	cy = psize->cy / 2;
	//mask
	for (y = 0; y < cy; y++) {
		prgb = (RGBTRIPLE *)ptr;
		for (x = 0; x < psize->cx; x++) {
			if (
				(prgb->rgbtBlue  == 0xFF) &&
				(prgb->rgbtGreen == 0x00) &&
				(prgb->rgbtRed   == 0xFF)
			) {
				prgb->rgbtBlue  = 0xFF;
				prgb->rgbtGreen = 0xFF;
				prgb->rgbtRed   = 0xFF;
			} else {
				prgb->rgbtBlue  = 0x00;
				prgb->rgbtGreen = 0x00;
				prgb->rgbtRed   = 0x00;
			}
			prgb++;
		}
		ptr += bw;
	}
	//invert
	for (y = 0; y < cy; y++) {
		prgb = (RGBTRIPLE *)ptr;
		for (x = 0; x < psize->cx; x++) {
			if (
				(prgb->rgbtBlue  == 0xFF) &&
				(prgb->rgbtGreen == 0x00) &&
				(prgb->rgbtRed   == 0xFF)
			) {
				prgb->rgbtBlue  = 0x00;
				prgb->rgbtGreen = 0x00;
				prgb->rgbtRed   = 0x00;
			}
			prgb++;
		}
		ptr += bw;
	}
}

static BOOL CreateAnalogClockDC(HWND hWnd, HDC hDC, LPTSTR fname)
{
	SIZE size;
	SIZE msize;
	PSTR path;



	if (hdcAnalogClock) {
		DeleteDC(hdcAnalogClock);
		hdcAnalogClock = NULL;
	}
	if (hbmpAnalogClock) {
		DeleteObject(hbmpAnalogClock);
		hbmpAnalogClock = NULL;
	}
	if (hdcAnalogClockMask) {
		DeleteDC(hdcAnalogClockMask);
		hdcAnalogClockMask = NULL;
	}
	if (hbmpAnalogClockMask) {
		DeleteObject(hbmpAnalogClockMask);
		hbmpAnalogClockMask = NULL;
		lpbyAnalogClockMask = NULL;
	}

	if (*fname == '\0') {
		return FALSE;
	}

	path = CreateFullPathName(hmod, fname);
	if (path == NULL) {
		hbmpAnalogClock = ReadBitmapFile(hDC, fname, &size);
	} else {
		hbmpAnalogClock = ReadBitmapFile(hDC, path, &size);
		free(path);
	}
	if (hbmpAnalogClock == NULL) {
		hdcAnalogClock = NULL;
		return TRUE;
	}
	sizeAnalogBitmapSize = size;
	msize = size;
	msize.cy *= 2;
	hbmpAnalogClockMask = Create24BitBitmap(hDC, &msize, &lpbyAnalogClockMask);
	if (hbmpAnalogClockMask == NULL) {
		lpbyAnalogClockMask = NULL;
		DeleteObject(hbmpAnalogClock);
		hbmpAnalogClock = NULL;
		return FALSE;
	}

	hdcAnalogClock = CreateCompatibleDC(hDC);
	if (hdcAnalogClock == NULL) {
		DeleteObject(hbmpAnalogClockMask);
		hbmpAnalogClockMask = NULL;
		lpbyAnalogClockMask = NULL;
		DeleteObject(hbmpAnalogClock);
		hbmpAnalogClock = NULL;
		return FALSE;
	}

	hdcAnalogClockMask = CreateCompatibleDC(hDC);
	if (hdcAnalogClockMask == NULL) {
		DeleteDC(hdcAnalogClock);
		hdcAnalogClock = NULL;
		DeleteObject(hbmpAnalogClockMask);
		hbmpAnalogClockMask = NULL;
		lpbyAnalogClockMask = NULL;
		DeleteObject(hbmpAnalogClock);
		hbmpAnalogClock = NULL;
		return FALSE;
	}

	SelectObject(hdcAnalogClock, hbmpAnalogClock);
	SelectObject(hdcAnalogClockMask, hbmpAnalogClockMask);

	BitBlt(hdcAnalogClockMask, 0,       0, size.cx, size.cy, hdcAnalogClock, 0, 0, SRCCOPY);
	BitBlt(hdcAnalogClockMask, 0, size.cy, size.cx, size.cy, hdcAnalogClock, 0, 0, SRCCOPY);

	ModifyMaskImage(&msize, lpbyAnalogClockMask);

	if (hdcAnalogClock) {
		DeleteDC(hdcAnalogClock);
		hdcAnalogClock = NULL;
	}
	if (hbmpAnalogClock) {
		DeleteObject(hbmpAnalogClock);
		hbmpAnalogClock = NULL;
	}

	return TRUE;
}

static VOID MakePosTable(int cx, int cy)
{
	int x, y;
	int x2, y2;
	double c, s;
	int i;
	int r;
	int sr;


	// x2 = x * cosA - y * sinA
	// y2 = x * sinA + y * cosA
	// ���W�A�� = (�p�x)���i��/180�j
	r = cx / 2;
	sr = r;

	x = sr;
	y = 0;
	for (i = 14; i > 0; i--) {
		c = ctbl[i];
		s = stbl[i];
		x2 = (int)(x * c - y * s);
		y2 = (int)(x * s + y * c);
		ptMinHand[i].x = x2 + r;
		ptMinHand[i].y = r - y2;
	}
	ptMinHand[0].x = r;
	ptMinHand[0].y = r - sr;

	r = cx / 2;
	sr = r * 7 / 10;

	x = sr;
	y = 0;
	for (i = 14; i > 0; i--) {
		c = ctbl[i];
		s = stbl[i];
		x2 = (int)(x * c - y * s);
		y2 = (int)(x * s + y * c);
		ptHourHand[i].x = x2 + r;
		ptHourHand[i].y = r - y2;
	}
	ptHourHand[0].x = r;
	ptHourHand[0].y = r - sr;
}

static VOID SetAnalogClockSize(SIZE *s)
{
	s->cy = s->cx = (int)(short)GetMyRegLong("AnalogClock", "AnalogClockSize", ACLOCK_SIZE_CX);
	if (s->cy <= 0) {
		s->cx = ACLOCK_SIZE_CX;
		s->cy = ACLOCK_SIZE_CY;
	}
	MakePosTable(s->cx, s->cy);
}

static BOOL InitAnalogClockData(HWND hWnd)
{
	//���ۂɂ�hWnd = hwndClockMain
	BOOL use;
	int confNo;
	TCHAR fname[MAX_PATH];
	HDC hDC;


	confNo = 1;

	use = GetMyRegLong("AnalogClock", "UseAnalogClock", FALSE);
	if (use == FALSE) {
		nAnalogClockUseFlag = ANALOG_CLOCK_NOTUSE;
		nAnalogClockPos = ANALOG_CLOCK_POS_MIDDLE;
		sizeAClock.cx = 0;
		sizeAClock.cy = 0;
		return FALSE;
	}

	nAnalogClockHPos = (int)(short)GetMyRegLong("AnalogClock", "AnalogClockHPos", 0);
	nAnalogClockVPos = (int)(short)GetMyRegLong("AnalogClock", "AnalogClockVPos", 0);
	nAnalogClockPos = (int)(short)GetMyRegLong("AnalogClock", "AnalogClockPos", 0);

	colAClockHourHandColor = (COLORREF)GetMyRegLong("AnalogClock", "AClockHourHandColor", RGB(255, 0, 0));
	//if (colAClockHourHandColor & 0x80000000) colAClockHourHandColor = GetSysColor(colAClockHourHandColor & 0x00ffffff);
	SetMyRegLong("AnalogClock", "AClockHourHandColor", colAClockHourHandColor);

	colAClockMinHandColor = (COLORREF)GetMyRegLong("AnalogClock", "AClockMinHandColor", RGB(0, 0, 255));
	//if (colAClockMinHandColor & 0x80000000) colAClockMinHandColor = GetSysColor(colAClockMinHandColor & 0x00ffffff);
	SetMyRegLong("AnalogClock", "AClockMinHandColor", colAClockMinHandColor);


	if (hpenHour) {
		DeleteObject(hpenHour);
	}
	if (hpenMin) {
		DeleteObject(hpenMin);
	}

	if (GetMyRegLong("AnalogClock", "AnalogClockHourHandBold", FALSE)) {
		nHourPenWidth = 2;
	} else {
		nHourPenWidth = 1;
	}
	if (GetMyRegLong("AnalogClock", "AnalogClockMinHandBold", FALSE)) {
		nMinPenWidth = 2;
	} else {
		nMinPenWidth = 1;
	}

	hpenHour = CreatePen(PS_SOLID, nHourPenWidth, colAClockHourHandColor);
	hpenMin = CreatePen(PS_SOLID, nMinPenWidth, colAClockMinHandColor);

	SetAnalogClockSize(&sizeAClock);

	GetMyRegStr("AnalogClock", "AnalogClockBmp", fname, MAX_PATH, "..\\tclock.bmp");
	lstrcpy(szAnalogClockBmp, fname);




	hDC = GetDC(hwndClockMain);

	CreateAnalogClockDC(hwndClockMain, hDC, szAnalogClockBmp);
	ReleaseDC(hwndClockMain, hDC);

	nAnalogClockUseFlag = ANALOG_CLOCK_USE;

	return TRUE;
}

static VOID DrawAnalogClockHand(HDC hDC, int dx, int dy, SYSTEMTIME* pt)
{
	HPEN hpenOld;
	WORD index;
	WORD sector;
	POINT posPoly[2];

	hpenOld = SelectObject(hDC, hpenHour);

	index = ((pt->wHour % 12) * 5) + (pt->wMinute / 12);
	sector = index / 15;
	index %= 15;
	GetHnadLinePos(index, sector, ptHourHand, posPoly, dx, dy);

	Polyline(hDC, posPoly, 2);

	SelectObject(hDC, hpenMin);

	index  = pt->wMinute % 15;
	sector = pt->wMinute / 15;
	GetHnadLinePos(index, sector, ptMinHand, posPoly, dx, dy);
	Polyline(hDC, posPoly, 2);

	SelectObject(hDC, hpenOld);
}

static VOID AnalogClockBlt(HDC hDC, int dx, int dy, SIZE *pdst, int sx, int sy, SIZE *psrc)
{
	if (
		(pdst->cx == sizeAnalogBitmapSize.cx) &&
		(pdst->cy == sizeAnalogBitmapSize.cy)
	) {
		BitBlt(
			hDC,
			dx, dy, pdst->cx, pdst->cy,
			hdcAnalogClockMask,
			sx, sy + sizeAnalogBitmapSize.cy,
			SRCAND
		);
		BitBlt(
			hDC,
			dx, dy, pdst->cx, pdst->cy,
			hdcAnalogClockMask,
			sx, sy,
			SRCINVERT
		);
	} else {
		StretchBlt(
			hDC,
			dx, dy, pdst->cx, pdst->cy,
			hdcAnalogClockMask,
			sx, sy + sizeAnalogBitmapSize.cy,
			psrc->cx, psrc->cy,
			SRCAND
		);
		StretchBlt(
			hDC,
			dx, dy,
			pdst->cx, pdst->cy,
			hdcAnalogClockMask,
			sx, sy,
			psrc->cx, psrc->cy,
			SRCINVERT
		);
	}
}

BOOL DrawAnalogClock(HDC hDC, SYSTEMTIME* pt, int xclock, int yclock, int wclock, int hclock)
{
//	RECT r;
	int sx, sy, dx, dy;
	SIZE clock_size;
	SIZE bitmapsize;



	if (hdcAnalogClockMask != NULL) {

		clock_size = sizeAClock;
		bitmapsize = sizeAnalogBitmapSize;

		if (nAnalogClockPos == ANALOG_CLOCK_POS_RIGHT) {
			sx = 0;
			dx = xclock + wclock - sizeAClock.cx + nAnalogClockHPos;
			if (dx >= xclock + wclock) {
				return FALSE;
			}
			if (dx < 0) {
				if (dx + sizeAClock.cx <= 0) {
					return FALSE;
				}
				sx = 0 - dx;
				clock_size.cx = clock_size.cx + dx;
				sx = sx * sizeAnalogBitmapSize.cx / sizeAClock.cx;
				dx = xclock;
				bitmapsize.cx = bitmapsize.cx - sx;
			}
		} else {
			if (nAnalogClockHPos < 0) {
				dx = xclock;
				if (nAnalogClockHPos + sizeAClock.cx <= 0) {
					return FALSE;
				}
				sx = 0 - nAnalogClockHPos;
				clock_size.cx = sizeAClock.cx + nAnalogClockHPos;
				sx = sx * sizeAnalogBitmapSize.cx / sizeAClock.cx;
				bitmapsize.cx = bitmapsize.cx - sx;
			} else {
				sx = 0;
				dx = xclock + nAnalogClockHPos;
			}
		}

		if (nAnalogClockVPos < 0) {
			dy = yclock;
			if (nAnalogClockVPos + sizeAClock.cy <= 0) {
				return FALSE;
			}
			sy = 0 - nAnalogClockVPos;
			clock_size.cy = sizeAClock.cy + nAnalogClockVPos;
			sy = sy * sizeAnalogBitmapSize.cy / sizeAClock.cy;
			bitmapsize.cy = bitmapsize.cy - sy;
		} else {
			sy = 0;
			dy = yclock + nAnalogClockVPos;
		}
		AnalogClockBlt(hDC, dx, dy, &clock_size, sx, sy, &bitmapsize);
	}

	if (nAnalogClockPos == ANALOG_CLOCK_POS_RIGHT) {
		dx = xclock + wclock - sizeAClock.cx + nAnalogClockHPos;
		dy = yclock + nAnalogClockVPos;
	} else {
		dx = xclock + nAnalogClockHPos;
		dy = yclock + nAnalogClockVPos;
	}
	DrawAnalogClockHand(hDC, dx, dy, pt);

	return TRUE;
}

static BOOL LocalDrawAnalogClock(HDC hDC, SYSTEMTIME* pt, int xclock, int yclock, int wclock, int hclock)
{
//	RECT r;
	static WORD lastSec = 0xFFFF;
	static WORD lastMin = 0xFFFF;
	static WORD lastHour = 0xFFFF;
	BOOL bRedraw;

	if (nAnalogClockUseFlag != ANALOG_CLOCK_USE) {
		return FALSE;
	}


	return DrawAnalogClock(hDC, pt, xclock, yclock, wclock, hclock);
}

static void DrawClockFocusRect(HDC hdc)
{
	if (bGetingFocus != FALSE) {
		RECT rc;

		GetClientRect(hwndClockMain, &rc);
		DrawFocusRect(hdc, &rc);
	}
}





void Textout_Tclock_Win10_3(int x, int y, char* sp, int len, int infoval)
{
	COLORREF textshadow, textcol_dow, textcol_temp;

	textshadow = TextColorFromInfoVal(99);
	textcol_dow = TextColorFromInfoVal(88);
	textcol_temp = TextColorFromInfoVal(infoval);

	if (fillbackcolor || (Win11Type == 2)) 
	{
		if (bClockShadow)
		{
			SetTextColor(hdcClock, textshadow);
			TextOut(hdcClock, x + nShadowRange, y + nShadowRange, sp, len);
		}
		if (bClockBorder)
		{
			SetTextColor(hdcClock, textshadow);
			TextOut(hdcClock, x - 1, y + 1, sp, len);
			TextOut(hdcClock, x + 1, y - 1, sp, len);
			TextOut(hdcClock, x + 1, y + 1, sp, len);
			TextOut(hdcClock, x, y - 1, sp, len);
			TextOut(hdcClock, x + 1, y, sp, len);
			TextOut(hdcClock, x - 1, y - 1, sp, len);
		}
		SetTextColor(hdcClock, textcol_temp);
		TextOut(hdcClock, x, y, sp, len);

	}
	else 
	{
		//3.4.5.2 (2021/10/14) by TTTT
		//hdcClock_work�ɂ������񏑂�����Ńt�H���g����̃����擾����
		//R�`���l��->textcol
		//G�`���l��->textcol_DoWzone
		//B�`���l��->textshadow
		//�̂��ꂼ��̃��l��������

		if (bClockShadow)
		{
			SetTextColor(hdcClock_work, RGB(0, 0, 255));
			TextOut(hdcClock_work, x + nShadowRange, y + nShadowRange, sp, len);
		}
		if (bClockBorder)
		{
			SetTextColor(hdcClock_work, RGB(0, 0, 255));
			TextOut(hdcClock_work, x - 1, y + 1, sp, len);
			TextOut(hdcClock_work, x + 1, y - 1, sp, len);
			TextOut(hdcClock_work, x + 1, y + 1, sp, len);
			TextOut(hdcClock_work, x, y - 1, sp, len);
			TextOut(hdcClock_work, x + 1, y, sp, len);
			TextOut(hdcClock_work, x - 1, y - 1, sp, len);
		}

		if (textcol_temp == textcol_dow) {
			SetTextColor(hdcClock_work, RGB(0, 255, 0));
		}
		else {
			SetTextColor(hdcClock_work, RGB(255, 0, 0));
		}

		TextOut(hdcClock_work, x, y, sp, len);
	}
}


COLORREF TextColorFromInfoVal(int infoval)
{
	COLORREF colRet;

	if (infoval == 99) {
		colRet = colShadow;
	}
	else if (infoval == 88) {
		colRet = textcol_DoWzone;
	}
	else if (flag_VPN && bUseVPNColor)
	{
		colRet = ColorVPNText;
	}
	else if (bUseAllColor)
	{
		colRet = textcol_DoWzone;
	}
	else if (infoval == 0x02)	//���t
	{
		if (bUseDateColor) {
			colRet = textcol_DoWzone;
		}
		else {
			colRet = ColorWeekdayText;
		}
	}
	else if (infoval == 0x04)	//�j��
	{
		if (bUseDowColor) {
			colRet = textcol_DoWzone;
		}
		else {
			colRet = ColorWeekdayText;
		}
	}
	else if (infoval == 0x08)	//����
	{
		if (bUseTimeColor) {
			colRet = textcol_DoWzone;
		}
		else {
			colRet = ColorWeekdayText;
		}
	}
	else
	{
		colRet = ColorWeekdayText;
	}

	if ((nBlink % 2) != 0)
	{
		colRet = (~colRet & 0xFFFFFF) + 0xFF000000;
	}

	return colRet;

}












/*------------------------------------------------
  draw the clock
--------------------------------------------------*/
void DrawClockSub(HDC hdc, SYSTEMTIME* pt, int beat100)
{
	BITMAP bmp;
	RECT rcFill,  rcClock;

	TEXTMETRIC tm;
	int hf, y, w;
	char s[1024], s_info[1024], *p, *sp, *p_info, *sp_info;
	//COLORREF s_col[1024];
	SIZE sz;
	int xclock, yclock, wclock, hclock, xsrc, ysrc, wsrc, hsrc;
	int xcenter;
	HRGN hRgn = NULL, hOldRgn = NULL;
	COLORREF textcol, textshadow, textcol_dow;

	RGBQUAD* color;
	RGBQUAD* color_work;

	extern BOOL b_exist_DOWzone;
	extern int pos_DOWzone;
	extern int length_DOWzone;
	extern int dow_DOWzone;








	//3.4.5.2 (2021/10/14) by TTTT
	//3.4.4.1�܂ł͈ȉ��̂悤�Ɏ擾�����w�i�F��w�i�Ƃ��Đݒ肵�Ă����BTextOut�̍ۂ̉��̐F�ɉe����^���邽�߁B
//	SetBkColor(hdcClock, colorBG_original);

	//3.4.5.2����́A���S�ȓ��ߍ����̏������ł���悤�ɂȂ����̂ŁA�^�����ő��v�ɂȂ���(�w�i�F�͎g��Ȃ��ł������ł���悤�ɂȂ����B
	//����ɕ����͂�������hdcClock_work�ɏo�͂���̂ňȉ��̍s�͎��ۂ͕s�v�ɂȂ����B�O���t��hdcClock�ɒ��ڏo�͂��邪�A���E���ł̒��ԐF�������Ȃ��̂Ŕw�i�ݒ���s�v
//	SetTextColor(hdcClock, textcol);
//	SetBkColor(hdcClock, RGB(0, 0, 0));


	SetBkColor(hdcClock_work, RGB(0,0,0));	//_work�̃r�b�g�}�b�v�͔w�i�����ɂ��ăt�H���g�̃����擾���邽�߂̃��[�N�G���A�Ƃ���B

	for (color = m_color_start; color < m_color_end; ++color) {
		//To write memory, the order is 0xRRrrggbb, different from colorref = 0x00bbggrr
		//rgbReserved(25-32�r�b�g)�̓A���t�@�l�Ƃ��ė��p����邪�A�����ł͂�������FF������BTextOut�֐���O���t�L�q�����ŏ����������N�����0�ɂȂ邱�Ƃ𗘗p���Č�̏������s���B


		//3.4.4.1�܂ł͈ȉ��̂悤�Ɏ擾�����w�i�F�����Ă����BTextOut�̍ۂ̉��̐F�ɉe����^���邽�߁B
	//	*(unsigned*)color = 0xFF000000 | (colorBG_original & 0x0000FF00) | ((colorBG_original & 0xFF) << 16) | ((colorBG_original >> 16) & 0xFF);
		//3.4.5.2 (2021/10/14)����́A���S�ȓ��ߍ����̏������ł���悤�ɂȂ����̂ŁA�^�����ő��v�ɂȂ���(�w�i�F�͎g��Ȃ��ł������ł���悤�ɂȂ����B
		*(unsigned*)color = 0xFF000000;
	}


	for (color = m_color_work_start; color < m_color_work_end; ++color) {
		//���ߕ\���p�̃r�b�g�}�b�v�ɂ����l�̏������{���B
		//To write memory, the order is 0xRRrrggbb, different from colorref = 0x00bbggrr
		//rgbReserved(25-32�r�b�g)�̓A���t�@�l�Ƃ��ė��p����邪�A�����ł͂�������FF������BTextOut�֐���O���t�L�q�����ŏ����������N�����0�ɂȂ邱�Ƃ𗘗p���Č�̏������s���B
		*(unsigned*)color = 0xFF000000;	
	}

	GetClientRect(hwndClockMain, &rcClock);


	if (!hdcClock) return;

	//FillClock();
	FillBack(hdcClock, widthMainClockFrame, heightMainClockFrame);

	if (Win11Type == 2)		//Win11Type2�ł̏�[�̐F���Ⴄ�������Č�����B
	{
		for (color = m_color_end - widthMainClockFrame; color <m_color_end ; ++color) {
			*(unsigned*)color = originalColorTaskbarEdge;
		}
	}
	

	for (int i = 0; i < 1024; i++)
	{
		*(s_info + i) = 0x00;
	}

	MakeFormat(s, s_info, pt, beat100, format);

	xclock = 0;
	yclock = 0;
	wclock = rcClock.right;
	hclock = rcClock.bottom;


	if (nAnalogClockUseFlag == ANALOG_CLOCK_USE) 
	{
		if (nAnalogClockPos == ANALOG_CLOCK_POS_MIDDLE) {
			if(nTextPos == 1) {
				xcenter = xclock;
			} else if(nTextPos == 2) {
				xcenter = wclock + xclock - nShadowRange;
			} else {
				xcenter = wclock / 2 + xclock;
			}
		} else if (nAnalogClockPos == ANALOG_CLOCK_POS_LEFT) {
			if(nTextPos == 1) {
				xcenter = xclock + sizeAClock.cx;
			} else if(nTextPos == 2) {
				xcenter = wclock + xclock - nShadowRange;
			} else {
				xcenter = (wclock - sizeAClock.cx) / 2 + xclock + sizeAClock.cx;
			}
		} else {
			if(nTextPos == 1) {
				xcenter = xclock;
			} else if(nTextPos == 2) {
				xcenter = wclock + xclock - nShadowRange - sizeAClock.cx;
			} else {
				xcenter = (wclock - sizeAClock.cx) / 2 + xclock;
			}
		}
	} else {
		if(nTextPos == 1) {
			xcenter = xclock;
		} else if(nTextPos == 2) {
			xcenter = wclock + xclock - nShadowRange;
		} else {
			xcenter = wclock / 2 + xclock;
		}
	}


	LocalDrawAnalogClock(hdcClock, pt, xclock, yclock, wclock, hclock);

	//3.4.5.2 (2021/10/14) by TTTT
	//�w�i���߃��[�h�ŃA�i���O���v�̎���̍��𓧉߂���B
	if (!fillbackcolor) {
		for (color = m_color_start; color < m_color_end; ++color) {
			if (*(unsigned*)color == 0x00000000) *(unsigned*)color = 0xFF000000;
		}
	}


	if (nAnalogClockUseFlag == ANALOG_CLOCK_USE)
	{
		if (nAnalogClockPos == ANALOG_CLOCK_POS_MIDDLE) {
			if (bGraph&&NetGraphScaleRecv>0 && (xclock + wclock)>0 && (yclock + hclock)>0)
				DrawGraph(hdcClock, xclock, yclock, wclock, hclock);
		}
		else {
			if (bGraph&&NetGraphScaleRecv>0 && (xclock + wclock)>0 && (yclock + hclock)>0)
				DrawGraph(hdcClock, xclock, yclock, wclock + sizeAClock.cx, hclock);
		}
	}
	else {
		if (bGraph&&NetGraphScaleRecv>0 && (xclock + wclock)>0 && (yclock + hclock)>0)
			DrawGraph(hdcClock, xclock, yclock, wclock, hclock);
	}



	//�O���t�ނ̕`�� on hdcClock
	if (b_UseBarMeterBL && b_BatteryLifeAvailable)
	{
		DrawBarMeter(hwndClockMain, hdcClock, wclock, hclock, BarMeterBL_Right, BarMeterBL_Left,
			BarMeterBL_Bottom, BarMeterBL_Top, iBatteryLife, MyColorTT_BL(), b_BarMeterBL_Horizontal, b_BarMeterBL_HorizontalToLeft);
	}


	if (b_UseBarMeterCU)
	{
		DrawBarMeter(hwndClockMain, hdcClock, wclock, hclock, BarMeterCU_Right, BarMeterCU_Left,
			BarMeterCU_Bottom, BarMeterCU_Top, totalCPUUsage, MyColorTT_CU(), b_BarMeterCU_Horizontal, b_BarMeterCU_HorizontalToLeft);
	}

	if (b_UseBarMeterGU)
	{
		DrawBarMeter(hwndClockMain, hdcClock, wclock, hclock, BarMeterGU_Right, BarMeterGU_Left,
			BarMeterGU_Bottom, BarMeterGU_Top, totalGPUUsage, MyColorTT_GU(), b_BarMeterCU_Horizontal, b_BarMeterCU_HorizontalToLeft);
	}

	if (!b_BarMeterCU_Horizontal && b_UseBarMeterCore)
	{
		for (int i = 0; i < NumberBarMeterCore; i++)
			DrawBarMeter(hwndClockMain, hdcClock, wclock, hclock, (BarMeterCore_Right - i * BarMeterCore_Pitch), (BarMeterCore_Left - i * BarMeterCore_Pitch),
				BarMeterCore_Bottom, BarMeterCore_Top, CPUUsage[i], MyColorTT_Core(CPUUsage[i]), b_BarMeterCU_Horizontal, b_BarMeterCU_HorizontalToLeft);

	}



	if (b_UseBarMeterVL)
	{
		DrawBarMeter(hwndClockMain, hdcClock, wclock, hclock, BarMeterVL_Right, BarMeterVL_Left,
			BarMeterVL_Bottom, BarMeterVL_Top, iVolume, MyColorTT_VL(), b_BarMeterVL_Horizontal, b_BarMeterVL_HorizontalToLeft);
	}


	if (b_UseBarMeterNet)
	{
		if (b_BarMeterNet_LogGraph)
		{
			if (net[2] == 0)
			{
				NetBarMeterRecv = 0;
			}
			else
			{
				NetBarMeterRecv = (int)((LogDigit - 1 + log10(net[2]) - log10(NetGraphScaleRecv * 1024)) * 100 / LogDigit);
			}
			if (net[3] == 0)
			{
				NetBarMeterSend = 0;
			}
			else
			{
				NetBarMeterSend = (int)((LogDigit2 - 1 + log10(net[3]) - log10(NetGraphScaleSend * 1024)) * 100 / LogDigit2);
			}
		}
		else
		{
			NetBarMeterRecv = (int)(net[2] / (NetGraphScaleRecv * 1024) * 100);
			NetBarMeterSend = (int)(net[3] / (NetGraphScaleSend * 1024) * 100);
		}



		{
			DrawBarMeter(hwndClockMain, hdcClock, wclock, hclock, BarMeterNetSend_Right, BarMeterNetSend_Left,
				BarMeterNetSend_Bottom, BarMeterNetSend_Top, NetBarMeterSend, ColorBarMeterNet_Send, b_BarMeterNet_Horizontal, b_BarMeterNet_HorizontalToLeft);
			DrawBarMeter(hwndClockMain, hdcClock, wclock, hclock, BarMeterNetRecv_Right, BarMeterNetRecv_Left,
				BarMeterNetRecv_Bottom, BarMeterNetRecv_Top, NetBarMeterRecv, ColorBarMeterNet_Recv, b_BarMeterNet_Horizontal, b_BarMeterNet_HorizontalToLeft);
		}
	}




	//�����̏o��



	//w = 0;
	GetTextMetrics(hdcClock, &tm);
	hf = tm.tmHeight - tm.tmInternalLeading;
	p = s;
	p_info = s_info;
	y = hf / 4 - tm.tmInternalLeading / 2 + yclock;
//	SIZE size1, size2, size3;
	//int xOffset;
	int currentzoneval;
	int zonelength;
	int tempXpos;
	int zonecount = 0;

	while (*p)
	{
		sp = p;
		sp_info = p_info;
		//xOffset = 0;

		while (*p && *p != 0x0d)	//�O�҂����������Ƃ��ɕ�����I���Ń��[�v���甲����B
		{
			p++;
			p_info++;
		}
		if (*p == 0x0d) {
			*p = 0;
			p += 2;
			*p_info = 0;
			p_info += 2;
		}		//���s�}�[�N�𕶎���I�[�}�[�N�ɂ�������u�� -> �ŏI�s�ȊO, p�͎��s�̐擪�A�h���X
		if (*p == 0 && sp == s)					//1�s�t�H�[�}�b�g�̏ꍇ
		{
			y = (hclock - tm.tmHeight) / 2 - tm.tmInternalLeading / 4 + yclock;
		}
		if (GetTextExtentPoint32(hdcClock, sp, strlen(sp), &sz) == 0)
		{
			sz.cx = (LONG)strlen(sp) * tm.tmAveCharWidth;
			sz.cy = tm.tmHeight;
		}
		//if (w < sz.cx) w = sz.cx;


		if (nTextPos == 1)
		{
			tempXpos = xcenter;
		}
		else if (nTextPos == 2)
		{
			tempXpos = xcenter - sz.cx;
		}
		else
		{
			tempXpos = xcenter - sz.cx / 2;
		}

		while (*sp_info)
		{
			zonecount++;

			currentzoneval = (int)*sp_info;
			zonelength = 0;
			while (*sp_info && ((int)*sp_info == currentzoneval))	//�s�[��*sp_info��0�Ȃ̂Ŏ����I�ɔ�����B
			{
				zonelength++;
				*sp_info++;
			}

			GetTextExtentPoint32(hdcClock, sp, zonelength, &sz);

			if (nTextPos == 1)
			{
				//tempXpos = xcenter;
			}
			else if (nTextPos == 2)
			{
				tempXpos += sz.cx;
			}
			else
			{
				tempXpos += sz.cx / 2;
			}

			Textout_Tclock_Win10_3(tempXpos, y + dvpos, sp, zonelength, currentzoneval);

			if (nTextPos == 1)
			{
				tempXpos += sz.cx;
			}
			else if (nTextPos == 2)
			{
				//tempXpos = xcenter + sz.cx;
			}
			else
			{
				tempXpos += sz.cx / 2;
			}


			sp = s + (sp_info - s_info);

		}

		y += hf; if (*p) y += 2 + dlineheight;
	}

//	if (b_DebugLog)writeDebugLog_Win10("[tclock.c][drawclocksub] Text out,zonecount =", zonecount);


	//3.4.5.2 (2021/10/14) by TTTT
	//���̎��_�ŁA�񓧉߂̏ꍇ��hdcClock�ɓ]�����ׂ��摜���o���オ���Ă���A���ׂẲ�f��rgbReserved = 0�ɂȂ��Ă���̂ŁA�����255�ɂ���΂悢�B
	//�w�i���߂̏ꍇ�́A�ȉ��̃v���Z�X�ɂȂ�B
	//hdcClock(�r�b�g�}�b�v�擪�A�h���X:m_color_start) �w�i�̓e�[�}�w�i�F+rgbReserved=255�A�O���t�������񂾂Ƃ��낾��rgbReserved��0�ɂȂ��Ă���B
	//hdcClock_work(�r�b�g�}�b�v�擪�A�h���X:m_color_work_start)�w�i�͍�+rgbReserved=255�A�����������񂾂Ƃ��낪rgbReserved��0�ɂȂ��Ă���B
	//r��0�łȂ��Ƃ����textcol, g��0�łȂ��Ƃ����textcol_WoDzone, b��0�łȂ��Ƃ����textshadow�̐F������ׂ��Ƃ���B
	//r, g, b�����Ƃ݂Ȃ��āAcolor_work�̃f�[�^����color�̃f�[�^������Ă����B


	textcol = TextColorFromInfoVal(1);	//����1�ŁAVPN�F�������͒ʏ�F�������I�ɓ�����B
	textshadow = TextColorFromInfoVal(99);
	textcol_dow = TextColorFromInfoVal(88);

	if (fillbackcolor || (Win11Type == 2)) {	//�w�i�񓧉߂̏ꍇ
		for (color = m_color_start; color < m_color_end; ++color) {
			color->rgbReserved = 255;
		}
	}
	else{			//�w�i���߂̏ꍇ
		unsigned channel;
		BYTE temp_alpha;
		BYTE textcol_r, textcol_g, textcol_b;
		textcol_r = GetRValue(textcol);
		textcol_g = GetGValue(textcol);
		textcol_b = GetBValue(textcol);

		BYTE textcol_dow_r, textcol_dow_g, textcol_dow_b;
		textcol_dow_r = GetRValue(textcol_dow);
		textcol_dow_g = GetGValue(textcol_dow);
		textcol_dow_b = GetBValue(textcol_dow);

		BYTE textshadow_r, textshadow_g, textshadow_b;
		textshadow_r = GetRValue(textshadow);
		textshadow_g = GetGValue(textshadow);
		textshadow_b = GetBValue(textshadow);

		for (color = m_color_start, color_work = m_color_work_start; color<m_color_end; ++color, ++color_work) {
			//BYTE temp_rgbReserved = color->rgbReserved;
//			if (color->rgbReserved == 255) {								//�O���t�h�b�g���Ȃ��Ƃ���
			if (color_work->rgbReserved == 0) {								//����������Ƃ���(���ӂ̒�P�x�֊s�܂Ŋ܂�)
				if (color_work->rgbRed > 10) {
					//			*(unsigned*)color = ((unsigned)(color_work->rgbRed)) << 24 | (textcol & 0x0000FF00) | ((textcol & 0xFF) << 16) | ((textcol >> 16) & 0xFF);
					temp_alpha = color_work->rgbRed;
					channel = textcol_r * temp_alpha / 255;
					color->rgbRed = (channel>255 ? 255 : (BYTE)channel);
					channel = textcol_g * temp_alpha / 255;
					color->rgbGreen = (channel>255 ? 255 : (BYTE)channel);
					channel = textcol_b * temp_alpha / 255;
					color->rgbBlue = (channel>255 ? 255 : (BYTE)channel);
					color->rgbReserved = temp_alpha;
				}
				else if (color_work->rgbGreen > 10) {
					//			*(unsigned*)color = ((unsigned)(color_work->rgbGreen)) << 24 | (textcol_dow & 0x0000FF00) | ((textcol_dow & 0xFF) << 16) | ((textcol_dow >> 16) & 0xFF);
					temp_alpha = color_work->rgbGreen;
					channel = textcol_dow_r * temp_alpha / 255;
					color->rgbRed = (channel>255 ? 255 : (BYTE)channel);
					channel = textcol_dow_g * temp_alpha / 255;
					color->rgbGreen = (channel>255 ? 255 : (BYTE)channel);
					channel = textcol_dow_b * temp_alpha / 255;
					color->rgbBlue = (channel>255 ? 255 : (BYTE)channel);
					color->rgbReserved = temp_alpha;
				}
				else if (color_work->rgbBlue > 10) {
					//			*(unsigned*)color = ((unsigned)(color_work->rgbBlue)) << 24 | (textshadow & 0x0000FF00) | ((textshadow & 0xFF) << 16) | ((textshadow >> 16) & 0xFF);
					temp_alpha = color_work->rgbBlue;
					channel = textshadow_r * temp_alpha / 255;
					color->rgbRed = (channel>255 ? 255 : (BYTE)channel);
					channel = textshadow_g * temp_alpha / 255;
					color->rgbGreen = (channel>255 ? 255 : (BYTE)channel);
					channel = textshadow_b * temp_alpha / 255;
					color->rgbBlue = (channel>255 ? 255 : (BYTE)channel);
					color->rgbReserved = temp_alpha;

				}
				else {
					*(unsigned*)color = 0x00000000;		//�����̎��ӂŋP�x���Ⴂ�Ƃ���=����
				}
			}
			else if (color->rgbReserved == 0) //�O���t�h�b�g������Ƃ���=�񓧖�
			{								
				color->rgbReserved = 255;	
			}
			else {//�O���t�h�b�g���Ȃ��Ƃ���=����
				*(unsigned*)color = 0x00000000;
			}
		}
	}

	//hdcClock_work�͂����ł�����ƂȂ̂ŁA�T�u�N���b�N�`��̍ۂ̓��ߗ��Z�o�p�ɍė��p����(�ԃ`���l������)
	for (color = m_color_start, color_work = m_color_work_start; color < m_color_end; ++color, ++color_work) {
		color_work->rgbRed = color->rgbReserved;
	}


	//202110�����_��Windows�ł�rgbReserved�𓧖��x(255:�񓧖�, 0:����,�������N���A)�Ƃ���hdc�ɃR�s�[������A���̂̂��̓��u�����h���Ă����B
	//������r, g, b�̒l�����炩���߃��������Ă����K�v������B
	//Alpha�u�����h�̐ݒ��AC_SRC_ALPHA�̂Ƃ���̐������Y������B
	//https://docs.microsoft.com/ja-jp/windows/win32/api/wingdi/ns-wingdi-blendfunction?redirectedfrom=MSDN

	//TransparentBlt�Ƃ������֐������邱�Ƃ��ォ��m�������A���łɂ��܂��s���Ă���̂ł�����Ȃ��B�������璲�ׂ邱�ƁB


	//�_�ŏ����́A�t�H���g�̂ݔ��]�Ƃ��āATextColorFromInfoVal()�̋@�\�Ƃ��Ď���
	BitBlt(hdc, 0, 0, widthMainClockFrame, heightMainClockFrame, hdcClock, 0, 0, SRCCOPY);

	HDC hdcSub = NULL;
	HDC hdcSubBuffer = NULL;
	HBITMAP hbm_tempDIBSection = NULL;
	RGBQUAD* temp_m_color_start = NULL, *temp_m_color_end;

	HBITMAP hbm_tempDIBSection2 = NULL;
	RGBQUAD* temp_m_color_start2 = NULL, *temp_m_color_end2;

	for (int i = 0; i < MAX_SUBSCREEN; i++) {
		if (bEnableSpecificSubClk[i]) {
			hdcSub = NULL;
			hdcSub = GetDC(hwndClockSubClk[i]);		//�T�u�f�B�X�v���C�̎��v�����݂����hdcSub�����݂��邱�ƂɂȂ�B
			if (hdcSub != NULL)
			{

				if (bWin11Sub && hwndOriginalWin11SubClk[i]) {
					if (IsWindowVisible(hwndOriginalWin11SubClk[i]))
					{
						if (b_DebugLog) writeDebugLog_Win10("[tclock.c][DrawClockSub] Original Win11 Clock on Subscreen is visible -> Hide, Index =", i);
						SetWindowVisible_Win10(hwndOriginalWin11SubClk[i], FALSE);
					}
				}



				if (bSuppressUpdateSubClk[i])
				{
					//HBRUSH tempHbrush;
					//tempHbrush = CreateSolidBrush(RGB(0, 0, 0));
					//SelectObject(hdcSub, tempHbrush);
					PatBlt(hdcSub, 0, 0, widthSubClock[i], heightSubClock[i], BLACKNESS);
					//DeleteObject(tempHbrush);
				}
				else
				{
				//StretchBlt�́AHALFTONE���[�h+SRCCOPY�łȂ��Ɖ掿�������邪�A���̑g�ݍ��킹����rgbReserved�͂��ׂ�0�ɂȂ��Ă��܂��B
				//rgbReserved�����߂邽�߂ɁAhdcClock_work�̐ԃ`���l�����g���ĕʓrStretcBlt�Ōv�Z����B


					hdcSubBuffer = CreateCompatibleDC(hdcSub);

					BITMAPINFO bmi = { { sizeof(BITMAPINFO),0,0,1,32,BI_RGB }, };
					bmi.bmiHeader.biWidth = widthSubClock[i];
					bmi.bmiHeader.biHeight = heightSubClock[i];

					hbm_tempDIBSection = CreateDIBSection(hdcSub, &bmi, DIB_RGB_COLORS, (void**)&temp_m_color_start, NULL, 0);
					temp_m_color_end = temp_m_color_start + (widthSubClock[i] * heightSubClock[i]);

					hbm_tempDIBSection2 = CreateDIBSection(hdcSub, &bmi, DIB_RGB_COLORS, (void**)&temp_m_color_start2, NULL, 0);
					temp_m_color_end2 = temp_m_color_start2 + (widthSubClock[i] * heightSubClock[i]);



					SelectObject(hdcSubBuffer, hbm_tempDIBSection);
					SetStretchBltMode(hdcSubBuffer, HALFTONE);
					StretchBlt(hdcSubBuffer, 0, 0, widthSubClock[i], heightSubClock[i], hdcClock_work, 0, 0, widthMainClockFrame, heightMainClockFrame, SRCCOPY);

					SelectObject(hdcSubBuffer, hbm_tempDIBSection2);
					SetStretchBltMode(hdcSubBuffer, HALFTONE);
					StretchBlt(hdcSubBuffer, 0, 0, widthSubClock[i], heightSubClock[i], hdcClock, 0, 0, widthMainClockFrame, heightMainClockFrame, SRCCOPY);

					for (color = temp_m_color_start, color_work = temp_m_color_start2; color < temp_m_color_end; ++color, ++color_work)
					{
						color_work->rgbReserved = color->rgbRed;
					}

					BitBlt(hdcSub, 0, 0, widthSubClock[i], heightSubClock[i], hdcSubBuffer, 0, 0, SRCCOPY);

				}

				ReleaseDC(hwndClockSubClk[i], hdcSub);
				DeleteDC(hdcSubBuffer);
				DeleteObject(hbm_tempDIBSection);
				DeleteObject(hbm_tempDIBSection2);
			}
			else {
				if (b_DebugLog)	writeDebugLog_Win10("[tclock.c][DrawClockSub] Clock on SubDisplay Disabled !! ID = ", i);
				DisableSpecificSubClock(i);
			}
		}
	}





	//�����Ă�����RedrawTClock�����s���Ă͂����Ȃ��I -> �R�[�h���Ŗ������[�v�ɂȂ�I�I
	//�����Ă�����RedrawMainTaskbar�����s���Ă͂����Ȃ��I -> Windows�o�R�Ŗ������[�v�ɂȂ�I�I

//NG!!!!!	RedrawMainTaskbar();	
}

/*------------------------------------------------
paint graph, added by TTTT
--------------------------------------------------*/
void DrawBarMeter(HWND hwnd, HDC hdc, int wclock, int hclock, int bar_right, int bar_left, int bar_bottom, int bar_top, int value, COLORREF color, BOOL b_Horizontal, BOOL b_ToLeft)
{
	//hwnd�͎g���Ă��Ȃ��B

	RECT barRect;
	HBRUSH hbr;
	int scale;

	if (bar_left > wclock || bar_right > wclock || bar_left <= bar_right) return;
	//if (bar_top >= 0 && bar_top < bar_bottom) return;
	if (bar_top >= hclock - bar_bottom) return;

	if (value < 0) return;

	if (value > 100) value = 100;

	barRect.bottom = hclock - bar_bottom - offsetBottomOfMeter;

	barRect.top = bar_top;

	barRect.left = wclock - bar_left;
	barRect.right = wclock - bar_right;

	if (b_Horizontal)
	{
		scale = barRect.left - barRect.right;
		if (b_ToLeft) {
			barRect.left = barRect.right + (scale * value / 100);
		}else{
			barRect.right = barRect.left - (scale * value / 100);
		}
	}
	else
	{
		scale = barRect.bottom - barRect.top;
		barRect.top = barRect.bottom - (scale * value / 100);
	}

	hbr = CreateSolidBrush(color);
	FillRect(hdc, &barRect, hbr);
	DeleteObject(hbr);
}


/*------------------------------------------------
paint graph, added by TTTT
--------------------------------------------------*/
void DrawBarMeter2(HWND hwnd, HDC hdc, int wclock, int hclock, int bar_right, int bar_left, int bar_bottom, int bar_top, int value, COLORREF color, BOOL b_Horizontal)
{
	//hwnd�͎g���Ă��Ȃ��B

	RECT barRect;
	HBRUSH hbr;
	int scale;

	if (bar_left > wclock || bar_right > wclock || bar_left <= bar_right) return;
	//if (bar_top >= 0 && bar_top < bar_bottom) return;
	if (bar_top >= hclock - bar_bottom) return;

	if (value < 0) return;

	if (value > 100) value = 100;

	barRect.bottom = hclock - bar_bottom - offsetBottomOfMeter;

	//if (bar_top < 0)
	//{
	//	barRect.top = 0;
	//}
	//else
	//{
	//	barRect.top = hclock - bar_top;
	//}

	barRect.top = bar_top;

	barRect.left = wclock - bar_left;
	barRect.right = wclock - bar_right;

	if (b_Horizontal)
	{
		scale = barRect.left - barRect.right;
		barRect.right = barRect.left - (scale * value / 100);
	}
	else
	{
		scale = barRect.bottom - barRect.top;
		barRect.top = barRect.bottom - (scale * value / 100);
	}

	hbr = CreateSolidBrush(color);
	FillRect(hdc, &barRect, hbr);
	DeleteObject(hbr);
}


void ClearGraphData(void) 
{
	for (int i = 0; i < MAXGRAPHLOG; i++)
	{
		recvlog[i] = 0;
		sendlog[i] = 0;
	}
}



/*------------------------------------------------
  paint graph
--------------------------------------------------*/
void DrawGraph(HDC hdc, int xclock, int yclock, int wclock, int hclock)
{
	int i, x, y, d;
	double one_dots = 0, one_dotr = 0;
	int graphSizeS;
	int graphSizeR;
	HPEN penSR, penR, penS;
	HPEN penCU_M,penCU_H;


	getGraphVal();

	//int LogDigit = 4;

	if(GraphL>wclock||GraphT>hclock)return;
	xclock+=GraphL;
	yclock+=GraphT;
	wclock-=GraphL + GraphR;
	hclock-=GraphT + GraphB + offsetBottomOfMeter;
	if(wclock > 0 && hclock > 0)
	{
		if (graphMode == 1)	//NET
		{
			penSR = CreatePen(PS_SOLID, 1, ColSR);
			penR = CreatePen(PS_SOLID, 1, ColRecv);
			penS = CreatePen(PS_SOLID, 1, ColSend);
		}
		else 
		{
			penSR = CreatePen(PS_SOLID, 1, ColorCPUGraph2);
			if (bUseBarMeterColForGraph) 
			{
				penR = CreatePen(PS_SOLID, 1, ColorBarMeterCU_Low);
				penCU_M = CreatePen(PS_SOLID, 1, ColorBarMeterCU_Mid);
				penCU_H = CreatePen(PS_SOLID, 1, ColorBarMeterCU_High);
				penS = CreatePen(PS_SOLID, 1, ColorBarMeterGPU);
			}
			else {
				penR = CreatePen(PS_SOLID, 1, ColorCPUGraph);
				penS = CreatePen(PS_SOLID, 1, ColorGPUGraph);
			}
		}
		HGDIOBJ oldPen = SelectObject(hdc, (HGDIOBJ)penSR);

		// Network
		if (graphMode == 1)
		{
			one_dotr = (double)(NetGraphScaleRecv  * 1024);
			one_dots = (double)(NetGraphScaleSend * 1024);
		}
		// CPU
		else if (graphMode == 2)
		{
			one_dotr = (double)100;
			one_dots = (double)100;
		}



		// ������
		if ( bGraphTate )
		{
			//one_dotr /= (double)(xclock + wclock);
			//one_dots /= (double)(xclock + wclock);
			one_dotr /= (double)wclock;
			one_dots /= (double)wclock;
			MoveToEx(hdc, (xclock + wclock) - 1, yclock, NULL);
			for(y = yclock;y < (yclock + hclock);y++)
			{
				if (bReverseGraph)
				{
					i = y - yclock;
					d = -1;
				}
				else
				{
					i = (yclock+hclock)-y-1;
					d = 1;
				}
				if(i >= 0 && i < MAXGRAPHLOG)
				{
					if(bLogGraph == TRUE && graphMode == 1)	//NET, Log
					{
						graphSizeS = (int)((LogDigit2 - 1 + log10(sendlog[i]) - log10(NetGraphScaleSend * 1024)) * wclock / LogDigit2);
						graphSizeR = (int)((LogDigit - 1 + log10(recvlog[i]) - log10(NetGraphScaleRecv * 1024)) * wclock / LogDigit);
						//if (graphSizeS < 0)graphSizeS = 0;
						//if (graphSizeR < 0)graphSizeR = 0;
					}
					else
					{
						graphSizeS = (int)(sendlog[i] / one_dots);
						graphSizeR = (int)(recvlog[i] / one_dotr);
					}
					if (graphSizeS < 0)graphSizeS = 0;
					if (graphSizeR < 0)graphSizeR = 0;

					if(GraphType == 1)	//�_�O���t
					{
//						MoveToEx(hdc, (xclock + wclock) - 1, y, NULL);
						MoveToEx(hdc, (xclock + wclock) , y, NULL);

						if (graphMode == 1) {			//Net

							if (graphSizeR > graphSizeS)
							{
								if (graphSizeS >= 0)
								{
									SelectObject(hdc, (HGDIOBJ)penS);
									LineTo(hdc, max((xclock + wclock) - graphSizeS, xclock), y);
								}
								if (graphSizeR > graphSizeS)
								{
									SelectObject(hdc, (HGDIOBJ)penR);
									LineTo(hdc, max((xclock + wclock) - graphSizeR, xclock), y);
								}
							}
							else
							{
								if (graphSizeR >= 0)
								{
									SelectObject(hdc, (HGDIOBJ)penR);
									LineTo(hdc, max((xclock + wclock) - graphSizeR, xclock), y);
								}
								if (graphSizeR < graphSizeS)
								{
									SelectObject(hdc, (HGDIOBJ)penS);
									LineTo(hdc, max((xclock + wclock) - graphSizeS, xclock), y);
								}
							}
						}
						else if (bEnableGPUGraph)	//CPU + GPU
						{
							if (graphSizeR > graphSizeS)
							{
								if (graphSizeS >= 0)
								{
									SelectObject(hdc, (HGDIOBJ)penS);
									LineTo(hdc, max((xclock + wclock) - graphSizeS, xclock), y);
								}
								if (graphSizeR > graphSizeS)
								{
									if (bUseBarMeterColForGraph) {
										if (recvlog[i] >= BarMeterCU_Threshold_High)
										{
											SelectObject(hdc, (HGDIOBJ)penCU_H);
										}
										else if (recvlog[i] >= BarMeterCU_Threshold_Mid)
										{
											SelectObject(hdc, (HGDIOBJ)penCU_M);
										}
										else
										{
											SelectObject(hdc, (HGDIOBJ)penR);
										}
									}
									else {
										SelectObject(hdc, (HGDIOBJ)penR);
									}
									LineTo(hdc, max((xclock + wclock) - graphSizeR, xclock), y);
								}
							}
							else
							{
								if (graphSizeR >= 0)
								{
									if (bUseBarMeterColForGraph) {
										if (recvlog[i] >= BarMeterCU_Threshold_High)
										{
											SelectObject(hdc, (HGDIOBJ)penCU_H);
										}
										else if (recvlog[i] >= BarMeterCU_Threshold_Mid)
										{
											SelectObject(hdc, (HGDIOBJ)penCU_M);
										}
										else
										{
											SelectObject(hdc, (HGDIOBJ)penR);
										}
									}
									else {
										SelectObject(hdc, (HGDIOBJ)penR);
									}
									LineTo(hdc, max((xclock + wclock) - graphSizeR, xclock), y);
								}
								if (graphSizeR < graphSizeS)
								{
									SelectObject(hdc, (HGDIOBJ)penS);
									LineTo(hdc, max((xclock + wclock) - graphSizeS, xclock), y);
								}
							}
						}
						else {			//Only CPU
							if (bUseBarMeterColForGraph) {
								if (totalCPUUsage >= BarMeterCU_Threshold_High)
								{
									SelectObject(hdc, (HGDIOBJ)penCU_H);
								}
								else if (totalCPUUsage >= BarMeterCU_Threshold_Mid)
								{
									SelectObject(hdc, (HGDIOBJ)penCU_M);
								}
								else
								{
									SelectObject(hdc, (HGDIOBJ)penR);
								}
							}
							else {
								SelectObject(hdc, (HGDIOBJ)penR);
							}
							LineTo(hdc, max((xclock + wclock) - graphSizeR, xclock), y);
						}

					}
					else if(GraphType==2)	//�܂��
					{
						if(i<MAXGRAPHLOG)
						{
							if(graphMode==1)	//Net
							{
								int SendThis;
								int RecvThis;
								int SendPrev;
								int RecvPrev;

								if (bLogGraph)
								{
									if (sendlog[i] == 0) SendThis = 0;
									else SendThis = (int)((LogDigit2 - 1 + log10(sendlog[i]) - log10(NetGraphScaleSend * 1024)) * wclock / LogDigit2);
									if (recvlog[i] == 0) RecvThis = 0;
									else RecvThis = (int)((LogDigit - 1 + log10(recvlog[i]) - log10(NetGraphScaleRecv * 1024)) * wclock / LogDigit);
									if (sendlog[i+1] == 0) SendPrev = 0;
									else SendPrev = (int)((LogDigit2 - 1 + log10(sendlog[i + 1]) - log10(NetGraphScaleSend * 1024)) * wclock / LogDigit2);
									if (recvlog[i+1] == 0) RecvPrev = 0;
									else RecvPrev = (int)((LogDigit - 1 + log10(recvlog[i + 1]) - log10(NetGraphScaleRecv * 1024)) * wclock / LogDigit);

									if (SendThis < 0)SendThis = 0;
									if (RecvThis < 0)RecvThis = 0;
									if (SendPrev < 0)SendPrev = 0;
									if (RecvPrev < 0)RecvPrev = 0;
								}
								else
								{
									SendThis = (int)(sendlog[i] / (NetGraphScaleSend * 1024) * wclock);
									RecvThis = (int)(recvlog[i] / (NetGraphScaleRecv * 1024) * wclock);
									SendPrev = (int)(sendlog[i + 1] / (NetGraphScaleSend * 1024) * wclock);
									RecvPrev = (int)(recvlog[i + 1] / (NetGraphScaleRecv * 1024) * wclock);
								}


								MoveToEx(hdc, max(xclock+wclock- SendPrev, xclock),y-d,NULL);
								SelectObject(hdc,(HGDIOBJ)penS);
								LineTo(hdc, max(xclock+wclock - SendThis, xclock),y);
								MoveToEx(hdc, max(xclock+wclock- RecvPrev, xclock),y-d,NULL);
								SelectObject(hdc,(HGDIOBJ)penR);
								LineTo(hdc, max(xclock+wclock- RecvThis, xclock),y);
							}
							else if(graphMode==2)	//CPU
							{
								if (bEnableGPUGraph) {
									MoveToEx(hdc, max((xclock + wclock) - (int)(sendlog[i + 1] / one_dots), xclock), y - d, NULL);
									SelectObject(hdc, (HGDIOBJ)penS);
									LineTo(hdc, max((xclock + wclock) - (int)(sendlog[i] / one_dots), xclock), y);
								}

								MoveToEx(hdc,max((xclock+wclock)-(int)(recvlog[i+1]/one_dotr), xclock),y-d,NULL);
								if (bUseBarMeterColForGraph) {
									if (recvlog[i] >= BarMeterCU_Threshold_High)
									{
										SelectObject(hdc, (HGDIOBJ)penCU_H);
									}
									else if (recvlog[i] >= BarMeterCU_Threshold_Mid)
									{
										SelectObject(hdc, (HGDIOBJ)penCU_M);
									}
									else
									{
										SelectObject(hdc, (HGDIOBJ)penR);
									}
								}
								else {
									SelectObject(hdc, (HGDIOBJ)penR);
								}
								LineTo(hdc,max((xclock+wclock)-(int)(recvlog[i]/one_dotr), xclock),y);

							}
						}
					}
				}
			}
		}

    // �c����
		else
		{
			//one_dotr/=(double)(yclock+hclock);
			//one_dots/=(double)(yclock+hclock);
			one_dotr /= (double)hclock;
			one_dots /= (double)hclock;
			for(x = xclock;x < (xclock + wclock);x++)
			{
				if (bReverseGraph)
				{
					i = x - xclock;
					d = -1;
				}
				else
				{
					i = (xclock+wclock)-x-1;
					d = 1;
				}
				if(i >= 0 && i < MAXGRAPHLOG)
				{
					if(bLogGraph == TRUE && graphMode == 1)
					{

						graphSizeS = (int)((LogDigit2 - 1 + log10(sendlog[i]) - log10(NetGraphScaleSend * 1024)) * hclock / LogDigit2);
						graphSizeR = (int)((LogDigit - 1 + log10(recvlog[i]) - log10(NetGraphScaleRecv * 1024)) * hclock / LogDigit);
						//if (graphSizeS < 0)graphSizeS = 0;
						//if (graphSizeR < 0)graphSizeR = 0;
					}
					else
					{
						graphSizeS = (int)(sendlog[i] / one_dots);
						graphSizeR = (int)(recvlog[i] / one_dotr);
					}
					if (graphSizeS < 0)graphSizeS = 0;
					if (graphSizeR < 0)graphSizeR = 0;

					if(GraphType==1)	//�_�O���t
					{
//						MoveToEx(hdc, x, (yclock + hclock) - 1,NULL);
						MoveToEx(hdc, x, (yclock + hclock) , NULL);

						if (graphMode == 1) {		//Net
							if (graphSizeR > graphSizeS)
							{
								if (graphSizeS >= 0)
								{
									SelectObject(hdc, (HGDIOBJ)penS);
									LineTo(hdc, x, max((yclock + hclock) - graphSizeS, yclock));
								}
								if (graphSizeR > graphSizeS)
								{
									SelectObject(hdc, (HGDIOBJ)penR);
									LineTo(hdc, x, max((yclock + hclock) - graphSizeR, yclock));
								}
							}
							else
							{
								if (graphSizeR >= 0)
								{
									SelectObject(hdc, (HGDIOBJ)penR);
									LineTo(hdc, x, max((yclock + hclock) - graphSizeR, yclock));
								}
								if (graphSizeR < graphSizeS)
								{
									SelectObject(hdc, (HGDIOBJ)penS);
									LineTo(hdc, x, max((yclock + hclock) - graphSizeS, yclock));
								}
							}
						}
						else if (bEnableGPUGraph)	//CPU + GPU
						{
							if (graphSizeR > graphSizeS)
							{
								if (graphSizeS >= 0)
								{
									SelectObject(hdc, (HGDIOBJ)penS);
									LineTo(hdc, x, max((yclock + hclock) - graphSizeS, yclock));
								}
								if (graphSizeR > graphSizeS)
								{
									if (bUseBarMeterColForGraph) {
										if (recvlog[i] >= BarMeterCU_Threshold_High)
										{
											SelectObject(hdc, (HGDIOBJ)penCU_H);
										}
										else if (recvlog[i] >= BarMeterCU_Threshold_Mid)
										{
											SelectObject(hdc, (HGDIOBJ)penCU_M);
										}
										else
										{
											SelectObject(hdc, (HGDIOBJ)penR);
										}
									}
									else {
										SelectObject(hdc, (HGDIOBJ)penR);
									}
									LineTo(hdc, x, max((yclock + hclock) - graphSizeR, yclock));
								}
							}
							else
							{
								if (graphSizeR >= 0)
								{
									if (bUseBarMeterColForGraph) {
										if (recvlog[i] >= BarMeterCU_Threshold_High)
										{
											SelectObject(hdc, (HGDIOBJ)penCU_H);
										}
										else if (recvlog[i] >= BarMeterCU_Threshold_Mid)
										{
											SelectObject(hdc, (HGDIOBJ)penCU_M);
										}
										else
										{
											SelectObject(hdc, (HGDIOBJ)penR);
										}
									}
									else {
										SelectObject(hdc, (HGDIOBJ)penR);
									}
									LineTo(hdc, x, max((yclock + hclock) - graphSizeR, yclock));
								}
								if (graphSizeR < graphSizeS)
								{
									SelectObject(hdc, (HGDIOBJ)penS);
									LineTo(hdc, x, max((yclock + hclock) - graphSizeS, yclock));
								}
							}
						}
						else 			//Only CPU
						{
							if (bUseBarMeterColForGraph) {
								if (recvlog[i] >= BarMeterCU_Threshold_High)
								{
									SelectObject(hdc, (HGDIOBJ)penCU_H);
								}
								else if (recvlog[i] >= BarMeterCU_Threshold_Mid)
								{
									SelectObject(hdc, (HGDIOBJ)penCU_M);
								}
								else
								{
									SelectObject(hdc, (HGDIOBJ)penR);
								}
							}
							else {
								SelectObject(hdc, (HGDIOBJ)penR);
							}
							LineTo(hdc, x, max((yclock + hclock) - graphSizeR, yclock));
						}
					}
					else if(GraphType==2)	//�܂��
					{
						if(i<MAXGRAPHLOG)
						{
							if(graphMode==1)	//Net
							{
								int SendThis;
								int RecvThis;
								int SendPrev;
								int RecvPrev;

								if (bLogGraph)
								{
									if (sendlog[i] == 0) SendThis = 0;
									else SendThis = (int)((LogDigit2 - 1 + log10(sendlog[i]) - log10(NetGraphScaleSend * 1024)) * hclock / LogDigit2);
									if (recvlog[i] == 0) RecvThis = 0;
									else RecvThis = (int)((LogDigit - 1 + log10(recvlog[i]) - log10(NetGraphScaleRecv * 1024)) * hclock / LogDigit);
									if (sendlog[i + 1] == 0) SendPrev = 0;
									else SendPrev = (int)((LogDigit2 - 1 + log10(sendlog[i + 1]) - log10(NetGraphScaleSend * 1024)) * hclock / LogDigit2);
									if (recvlog[i + 1] == 0) RecvPrev = 0;
									else RecvPrev = (int)((LogDigit - 1 + log10(recvlog[i + 1]) - log10(NetGraphScaleRecv * 1024)) * hclock / LogDigit);

									if (SendThis < 0)SendThis = 0;
									if (RecvThis < 0)RecvThis = 0;
									if (SendPrev < 0)SendPrev = 0;
									if (RecvPrev < 0)RecvPrev = 0;
								}
								else
								{
									SendThis = (int)(sendlog[i] / (NetGraphScaleSend * 1024) * hclock);
									RecvThis = (int)(recvlog[i] / (NetGraphScaleRecv * 1024) * hclock);
									SendPrev = (int)(sendlog[i+1] / (NetGraphScaleSend * 1024) * hclock);
									RecvPrev = (int)(recvlog[i+1] / (NetGraphScaleRecv * 1024) * hclock);
								}
								MoveToEx(hdc, x - d, max(yclock + hclock - SendPrev, yclock), NULL);
								SelectObject(hdc, (HGDIOBJ)penS);
								LineTo(hdc, x, max(yclock + hclock - SendThis, yclock));
								MoveToEx(hdc, x - d, max(yclock + hclock - RecvPrev, yclock), NULL);
								SelectObject(hdc, (HGDIOBJ)penR);
								LineTo(hdc, x, max(yclock + hclock - RecvThis, yclock));
							}
							else if(graphMode==2)	//CPU
							{
								if (bEnableGPUGraph) {
									MoveToEx(hdc, x - d, max((yclock + hclock) - (int)(sendlog[i + 1] / one_dots), yclock), NULL);
									SelectObject(hdc, (HGDIOBJ)penS);
									LineTo(hdc, x, max((yclock + hclock) - (int)(sendlog[i] / one_dots), yclock));
								}

								MoveToEx(hdc,x-d,max((yclock+hclock)-(int)(recvlog[i+1]/one_dotr), yclock),NULL);
								if (bUseBarMeterColForGraph) {
									if (recvlog[i] >= BarMeterCU_Threshold_High)
									{
										SelectObject(hdc, (HGDIOBJ)penCU_H);
									}
									else if (recvlog[i] >= BarMeterCU_Threshold_Mid)
									{
										SelectObject(hdc, (HGDIOBJ)penCU_M);
									}
									else
									{
										SelectObject(hdc, (HGDIOBJ)penR);
									}
								}
								else {
									SelectObject(hdc,(HGDIOBJ)penR);
								}
								LineTo(hdc,x,max((yclock+hclock)-(int)(recvlog[i]/one_dotr), yclock));

							}
						}
					}
				}
			}
		}
		SelectObject(hdc,oldPen);
		DeleteObject(penSR);
		DeleteObject(penR);
		DeleteObject(penS);
		if ((graphMode == 2) && (bUseBarMeterColForGraph)) {
			DeleteObject(penCU_M);
			DeleteObject(penCU_H);
		}
	}
}

void getGraphVal()
{
	int i;
	extern double net[];

	if (graphMode == 1)
	{

		for(i=MAXGRAPHLOG-1;i>=0;i--)
		{
			sendlog[i+1]=sendlog[i];
			recvlog[i+1]=recvlog[i];
		}
		sendlog[0] = net[3];
		recvlog[0] = net[2];


	}
	else if (graphMode == 2)
	{
		int cpu_u;
		for(i=MAXGRAPHLOG-1;i>=0;i--)
		{
			sendlog[i+1]=sendlog[i];
			recvlog[i+1]=recvlog[i];
		}
		recvlog[0] = (double)totalCPUUsage;
		sendlog[0] = (double)totalGPUUsage;

		//cpu_u = totalCPUUsage;
		//if (cpu_u >= cpuHigh)
		//{
		//	recvlog[0]=(double)cpu_u;
		//	sendlog[0]=cpuHigh;
		//}
		//else
		//{
		//	recvlog[0]=(double)cpu_u;
		//	sendlog[0]=(double)cpu_u;
		//}
	}
	bGraphRedraw = TRUE;
}




void FillBack(HDC hdcTarget, int width, int height) 
{
	HBRUSH hbr;
	COLORREF col;
	RECT tempRect;


	tempRect.left = 0;
	tempRect.right = width;
	tempRect.top = 0;
	tempRect.bottom = height;



	if (!fillbackcolor)
	{
		//�ȑO�͂����Ɏ��v�w�i��hdcClock�Ɏ��e���Ă��̏�ɏ������ރR�[�h�����������A2021�N10�����_�ŋ@�\���Ȃ����̂ɂȂ��Ă���B
		//Windows�̃^�X�N�o�[�\�������S�ɕς���Ă��Ă��̕�������������\���͂Ȃ��̂ŁA�폜
		//�������́A���v�̃r�b�g�}�b�v�𓧉ߍ����Ή��ɂ��Ď������Ă���B

		if (Win11Type == 2)
		{
			//Win11Type2�ł͓������ʂŃ^�X�N�o�[�F���ς��̂�TClock�̍��[�͖��b�F�����킹��B
			//����ŁATClock�E�[�ɂ͒ʒm�A�C�R���������āA���b�X�V�����킯�ł͂Ȃ��̂ŁA�����O���f�[�V�����ŕ`�悷��B
			//�ʒm�̍X�V������ꍇ�ɁA���E�̐F����v����B(�ʒm�̐F���^�X�N�o�[�̐F�ɍ��킹��)
			GradientFillBack(hdcTarget, width, height, originalColorTaskbar, originalColorTaskbar_ForWin11Notify, 0);
		}
	}
	else
	{
		if (colback == colback2)	
		{
			col = colback;
			hbr = CreateSolidBrush(col);
			FillRect(hdcTarget, &tempRect, hbr);
			DeleteObject(hbr);
		}
		else if ((width == widthWin11Notify) && (grad == 0) && bEnabledWin11Notify)	//���O���f��Win11Notify�̃E�B���h�E�̏ꍇ�͑�2�F�œh��Ԃ�
		{
			col = colback2;
			hbr = CreateSolidBrush(col);
			FillRect(hdcTarget, &tempRect, hbr);
			DeleteObject(hbr);
		}
		else
		{
			COLORREF col2;

			col = colback;
			col2 = colback2;

			GradientFillBack(hdcTarget, width, height, col, col2, grad);
		}
	}

}



//�g���Ă��Ȃ��B�K���ȃ^�C�~���O�ŏ����B
/*------------------------------------------------
  paint background of clock
--------------------------------------------------*/
void FillClock()
{
	HBRUSH hbr;
	COLORREF col;
	RECT tempRect;


	tempRect.left = 0;
	tempRect.right = widthMainClockFrame;
	tempRect.top = 0;
	tempRect.bottom = heightMainClockFrame;



	if (!fillbackcolor)
	{
		//�ȑO�͂����Ɏ��v�w�i��hdcClock�Ɏ��e���Ă��̏�ɏ������ރR�[�h�����������A2021�N10�����_�ŋ@�\���Ȃ����̂ɂȂ��Ă���B
		//Windows�̃^�X�N�o�[�\�������S�ɕς���Ă��Ă��̕�������������\���͂Ȃ��̂ŁA�폜
		//�������́A���v�̃r�b�g�}�b�v�𓧉ߍ����Ή��ɂ��Ď������Ă���B

		if (Win11Type == 2)
		{
			col = originalColorTaskbar;
			hbr = CreateSolidBrush(col);
			FillRect(hdcClock, &tempRect, hbr);
			DeleteObject(hbr);
		}
	}
	else 
	{
		if (colback == colback2)
		{
			col = colback;
			hbr = CreateSolidBrush(col);
			FillRect(hdcClock, &tempRect, hbr);
			DeleteObject(hbr);
		}
		else
		{
			COLORREF col2;

			col = colback;
			col2 = colback2;

			GradientFillBack(hdcClock, widthMainClockFrame, heightMainClockFrame, col, col2, grad);
		}
	}
}

// TClock�̒��g�̃T�C�Y���v�Z
// OnCalcRect�̃I���W�i���R�[�h�����p��
// ���v�G���A��菬�����ꍇ�̕␳���܂܂Ȃ��B�������`��̈�����߂�ɂ�
// CalcMainClockSize()���ĂԂ��ƁB���̊֐����Ă΂ꂽ��ŁA���v�G���A�ɍ����悤�ɓK�؂ȕ␳���Ȃ����B
// ���̊֐������ĂԂƁAwidthMainClockContent, heightMainClockContent�̓^�X�N�o�[��(�c�^�X�N�o�[)��^�X�N�o�[����(���^�X�N�o�[)��菬�����Ȃ��Ă��܂��̂ŁA
// ���ڌĂ΂Ȃ����ƁB
void CalcMainClockContentSize(void)
{
	SYSTEMTIME t;
	int beat100 = 0;
	LONG w, h;
	HDC hdc;
	HGDIOBJ hOldFont = NULL;
	TEXTMETRIC tm;
	char s[1024], s_info[1024], *p, *sp;
	SIZE sz;
	int hf;


	hdc = GetDC(hwndClockMain);

	if (hFon) hOldFont = SelectObject(hdc, hFon);	//�����Ńt�H���g��ݒ肵�Ă���B
	GetTextMetrics(hdc, &tm);

	GetDisplayTime(&t, nDispBeat ? (&beat100) : NULL);
	MakeFormat(s, s_info, &t, beat100, format);

	p = s; w = 0; h = 0;
	hf = tm.tmHeight - tm.tmInternalLeading;
	while (*p)
	{
		sp = p;
		while (*p && *p != 0x0d) p++;
		if (*p == 0x0d) { *p = 0; p += 2; }
		if (GetTextExtentPoint32(hdc, sp, strlen(sp), &sz) == 0)
			sz.cx = (LONG)strlen(sp) * tm.tmAveCharWidth;
		if (w < sz.cx) w = sz.cx;
		h += hf; if (*p) h += 2 + dlineheight;
	}

	if (nAnalogClockUseFlag == ANALOG_CLOCK_USE)
	{
		if (nAnalogClockPos != ANALOG_CLOCK_POS_MIDDLE) {
			w += sizeAClock.cx;
		}
	}

	w += tm.tmAveCharWidth * 2;
	w += dwidth;
	h += hf / 2 + dheight;
	if (bClockShadow)
	{
		h += nShadowRange; w += nShadowRange;
	}
	if (h < 4) h = 4;

//	if (hFon) SelectObject(hdc, hOldFont);		//���̐悸����TClock���ғ�����̂ŁA�߂��Ȃ��Ă悢�B
	ReleaseDC(hwndClockMain, hdc);



	widthMainClockContent = w;
	heightMainClockContent = h;

	//�^�X�N�o�[�̈��菬�����ꍇ�ɂ͏C������B
	if (g_bVertTaskbar) 
	{
		if (widthMainClockContent < widthMainClockFrame) widthMainClockContent = widthMainClockFrame;	//�c�^�X�N�o�[��蕝�������ꍇ�ɂ͕����^�X�N�o�[�ɂ��킹��B
	}
	else {
		if (heightMainClockContent < heightMainClockFrame) heightMainClockContent = heightMainClockFrame; //���^�X�N�o�[��荂���������ꍇ�ɂ͍������^�X�N�o�[�ɂ��킹��
	}

	if (b_DebugLog) {
		writeDebugLog_Win10("[tclock.c][CalcMainClockContentSize] Clock Content Width = ", w);
		writeDebugLog_Win10("[tclock.c][CalcMainClockContentSize] Clock Content Height = ", h);
	}

}

// TClock�̎��e�t���[���T�C�Y���v�Z
// CalcMainClockCOntetSize���ĂԂ̂�
// ���v�̃R���e���c�̃T�C�Y���v�Z���邱�ƂɂȂ�B
// �^�X�N�o�[�����̔�����s���B
// Frame��Content�Ɠ�����������
// �c�^�̏ꍇ�͍����͏�ɓ����ŁA����Content�̉E���؂��(Frame�͍�����؂�ڂ܂�)�\��������B
// ���^�̏ꍇ�͕��͏�ɓ����ŁA����Content�̉����؂��(Frame�͏ォ��؂�ڂ܂�)�\��������B
void CalcMainClockSize(void)
{

	g_bVertTaskbar = IsVertTaskbar(hwndTaskBarMain);

	RECT tempRect;
	GetWindowRect(hwndTaskBarMain, &tempRect);
	CalcMainClockContentSize();

	if (g_bVertTaskbar) {
		widthMainClockFrame = tempRect.right - tempRect.left;
		heightMainClockFrame = heightMainClockContent;
	}
	else {
		widthMainClockFrame = widthMainClockContent;
		heightMainClockFrame = tempRect.bottom - tempRect.top;
		//if (bAdjustTrayWin11SmallTaskbar && (Win11Type == 2))
		//{
		//	heightMainClockFrame = 2 * originalHeightTaskbar / 3;
		//}
	}

	SetMyRegLong("Status_DoNotEdit", "ClockWidth", widthMainClockFrame);
	SetMyRegLong("Status_DoNotEdit", "ClockHeight", heightMainClockFrame);

	if (b_DebugLog) {
		writeDebugLog_Win10("[tclock.c][CalcMainClockContentSize] Clock Frame Width = ", widthMainClockFrame);
		writeDebugLog_Win10("[tclock.c][CalcMainClockContentSize] Clock Frame Height = ", heightMainClockFrame);
	}

}

void UpdateSysRes(BOOL bbattery, BOOL bmem, BOOL bnet, BOOL bhdd, BOOL bcpu, BOOL bvol, BOOL bgpu, BOOL btemp)
{
	int i;



//	if (b_DebugLog) writeDebugLog_Win10("[tclock.c] UpdateSysRes called. ", 999);

	if (bTimerAdjust_SysInfo)
	{
		bTimerAdjust_SysInfo = FALSE;
		SetTimer(hwndClockMain, IDTIMERDLL_SYSINFO, TimerCountForSec, NULL);
	}

	if(bbattery)
	{
		iBatteryLife = GetBatteryLifePercent();

		if (iBatteryLife > 100 && b_BatteryLifeAvailable)
		{
			b_BatteryLifeAvailable = FALSE;
			SetMyRegLong("Status_DoNotEdit", "BatteryLifeAvailable", 0);
			if (b_UseBarMeterBL)
			{
				b_UseBarMeterBL = FALSE;
				SetMyRegLong("BarMeter", "UseBarMeterBL", 0);
			}
			bGetBattery = FALSE;
		}
	}



	if(bmem)
	{
		msMemory.dwLength = sizeof(msMemory);
		GlobalMemoryStatusEx(&msMemory);
	}

	if(bvol)
	{
		getMasterVolume_Win10();
	}


	//if(bpermon || bcpu)
	//{
	//	PerMoni_get();
	//}
	if (bcpu)
	{
		UpdateCpuClock();
		PerMoni_get();
	}

	if (bgpu)
	{
		GPUMoni_get();
	}

	if(bnet)
	{
		Net_get();
	}

	if(bhdd)
	{
		Hdd_get();
	}

	if (btemp)
	{
		pdhTemperature = TempMoni_get(indexSelectedThermalZone);
	}

	if (b_DebugLog) writeDebugLog_Win10("[tclock.c] UpdateSysRes finished. ", 999);


}

void OntimerCheckNetStat_Win10(HWND hwnd)
{
	int i;

	//if (b_DebugLog) writeDebugLog_Win10("[tclock.c] OntimerCheckNetStat_Win10 called. ", 999);

	if (bTimerAdjust_NetStat)
	{
		bTimerAdjust_NetStat = FALSE;
		SetTimer(hwndClockMain, IDTIMERDLL_CHECKNETSTAT, TimerCountForSec, NULL);
		//if (b_DebugLog) writeDebugLog_Win10("[OntimerCheckNetStat_Win10] IDTIMERDLL_CHECKNETSTAT was Adjusted with offset(ms) :", OFFSETMS_TIMER_NETSTAT);
	}


	if (!b_CompactMode)
	{
		chkInternetConnectionProfile_Win10(); // replaced by TTTT to check whether this chkDataPlanUsage_OK is needed or not.
	}

}

//added by TTTT 20200914
void GetTimeZoneBias_Win10(void)
{
	TIME_ZONE_INFORMATION tzi;
	DWORD dw;

	memset(&tzi, 0, sizeof(tzi));
	dw = GetTimeZoneInformation(&tzi);
	if (dw == TIME_ZONE_ID_STANDARD)
	{
		currentTimeZoneBiasMin = (int)tzi.StandardBias;
	}
	if (dw == TIME_ZONE_ID_DAYLIGHT)
	{
		currentTimeZoneBiasMin = (int)tzi.DaylightBias;
	}
	else {
		currentTimeZoneBiasMin = (int)tzi.Bias;
	}


//	currentTimeZoneBiasMin = (int)tzi.Bias;
	if (b_DebugLog) writeDebugLog_Win10("[tclock.c][GetTimeZoneBias_Win10] currentTimeZoneBiasMin = ", currentTimeZoneBiasMin);

}

// Added by TTTT for Win10AU (WIN10RS1) compatibility
// Imported from TClockLight-tclocklight-kt160911, dll/Wndproc.c
/*------------------------------------------------
subclass procedure of the tray
--------------------------------------------------*/
LRESULT CALLBACK SubclassTrayProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	//���̃R�[�h���ł�hwnd��hwndTrayMain�ł���A���̑����̏ꍇ(hwnd=hwndClockMain)�ƈقȂ�Ȃ̂Œ��ӂ��邱�ƁI
	//�^�X�N�g���C�̓��C���̃^�X�N�o�[�ɂ������݂��Ȃ��̂ŁA����ȊO�ɂ͎g���Ȃ��R�[���o�b�N�֐��ɂȂ��Ă���B

//	if (b_DebugLog)writeDebugLog_Win10("[tclock.c][SubclassTrayProc] Window Message was recevied, message = ", message);

	switch (message)
	{
		case (WM_USER + 100):	//1124
		{
			if (b_DebugLog)writeDebugLog_Win10("[tclock.c][SubclassTrayProc] WM_USER + 100 (1124) recevied.", 999);

			// �Ĕz�u�O�ɐe�E�B���h�E���瑗���A�T�C�Y��Ԃ����b�Z�[�W�B
			// DefSubClassTrayProc()���ĂԂ�LRESULT�`���ŁAWindows�W�����v���������ꍇ�̃T�C�Y���A���Ă���̂ŁA
			// ���������ꍇ�̃T�C�Y�ɍ����ւ��Ė߂��B
			// �������l��Ԃ��Ȃ��ƃ^�X�N�g���C�̃T�C�Y�����������Ȃ�B

			LRESULT ret;

			ret = DefSubclassProc(hwndTrayMain, message, wParam, lParam);




			if (g_bVertTaskbar)
			{
				ret = MAKELONG(LOWORD(ret),
					HIWORD(ret) + heightMainClockFrame - prevHeightMainClock);
			}
			else
			{
				ret = MAKELONG(LOWORD(ret) + widthMainClockFrame - prevWidthMainClock,
					HIWORD(ret));
			}

			if (b_DebugLog)
			{
				writeDebugLog_Win10("[tclock.c][SubclassTrayProc] Returned Width =", LOWORD(ret));
				writeDebugLog_Win10("[tclock.c][SubclassTrayProc] Returned Height =", HIWORD(ret));
			}

			return ret;
		}
		//		case (WM_NCCALCSIZE):	//�T�u�E�B���h�E�N���b�N��Win11��SubClassTrayProc_Win11�ɂ͓͂����AWin10�̃g���C�ɂ͂��̃��b�Z�[�W�͗��Ȃ�
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
			//	writeDebugLog_Win10("[tclock.c][SubclassTrayProc] WM_NOTIFY(78) received, with code =", nmh->code);
			//}

			if (nmh->code != PGN_CALCSIZE) break;

			ret = DefSubclassProc(hwndTrayMain, message, wParam, lParam);	//hwnd�𖾎��������A�@�\�͓����B

			if (b_DebugLog)writeDebugLog_Win10("[tclock.c][SubclassTrayProc] SetMainClockOnTasktray called, initiated by WM_NOTIFY(78) + PGN_CALCSIZE.", 999);

			SetMainClockOnTasktray();

			return ret;
		}
	}

	return DefSubclassProc(hwndTrayMain, message, wParam, lParam);
}

void GetPrevMainClockSize(void)
{
	RECT tempRect;

	GetWindowRect(hwndClockMain, &tempRect);
	prevWidthMainClock = tempRect.right - tempRect.left;
	prevHeightMainClock = tempRect.bottom - tempRect.top;

	if (b_DebugLog)
	{
		writeDebugLog_Win10("[tclock.c][GetPrevMainClockSize] prevWidthMainClock =", prevWidthMainClock);
		writeDebugLog_Win10("[tclock.c][GetPrevMainClockSize] prevHeightMainClock =", prevHeightMainClock);
	}

}

void SetTClockFont()
{
	if (hdcClock) {
		if (hFon) SelectObject(hdcClock, hFon);
	}

	if (hdcClock_work) {
		if (hFon) SelectObject(hdcClock_work, hFon);
	}
}

// Added by TTTT for Win10AU (WIN10RS1) compatibility
// Imported from TClockLight-tclocklight-kt160911, dll/Wndproc.c
/*------------------------------------------------
Rearrange the notify area
--------------------------------------------------*/
void SetMainClockOnTasktray(void)
{
	//���̊֐���SubClassTrayProc��WM_NOTIFY���炵���Ă΂�Ȃ�(��Ԃ��ێ����邱�ƁI)�B





	//Win10�ɂ�����^�X�N�g���C���Ĕz�u�̕��@
	//�^�X�N�g���C���ł͍����(0,0)�Ƃ��ăA�C�e�������ׂ��Ă��āA���̎��_�Ŏ��v(hwndClockMain)�̍���̍��W�͐�����
	//(������SubClassTrayProc��WM_USER + 100�ɑ΂���ԓ��Ő����������Ă����)�̂�,���v���ړ�����K�v�͂Ȃ��B
	//�������T�C�Y��Windows�W���̎��v�T�C�Y�ɋ����I�ɋ��߂��Ă���(=origClockWidth,origClockHeight)�B
	//�^�X�N�o�[�������Ŕ����ɕς��̂Œ��ӂ��K�v�����A���̊֐��R�[���̒��O��SubClassTrayProc���Ŏ擾�ł��Ă���͂�)�B
	//�g���C�S�̕��ƍŐV��origClockHeigt, origClockWidth�����������l�𐳂����߂��Ă���΂҂�������܂�͂��B
	//TClock�̃T�C�Y���C������ƁA�����荶(or��)�ɂ���g���C���̍\�����Ĕz�u���Ă��K�v������B
	//���̑���Ő��������܂�Ȃ���΁ASubClassTrayProc��WM_USER + 100�ɑ΂���߂��l(�v������^�X�N�g���C�T�C�Y)���Ԉ���Ă���B


	//����N�����ȊO�́Ag_OriginalClockWidth, g_OriginalClockHeight���������l���ێ�����Ă���Ɗ��҂����B


	if (b_DebugLog) writeDebugLog_Win10("[tclock.c] SetMainClockOnTasktray called. ", 999);
	POINT pos, shift;
	HWND tempHwnd;
	RECT tempRect;


	//�m�ۂ��ׂ����v�̃T�C�Y���擾
	GetPrevMainClockSize();
	CalcMainClockSize();

	//�ʒm�{�^���A�f�X�N�g�b�v�\���{�^���̈ړ����ׂ��������v�Z
	shift.x = widthMainClockFrame - prevWidthMainClock;
	shift.y = heightMainClockFrame - prevHeightMainClock;


	//�T�C�Y�Ƃ���ClockFrame���g���Ǝ��ی����Ă���T�C�Y�̃r�b�g�}�b�v����邱�ƂɂȂ�B
	SetWindowPos(hwndClockMain, NULL, 0, 0, widthMainClockFrame, heightMainClockFrame,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING);

	//�e�폈���̂��߃��C���N���b�N�ʒu���X�V
	GetWindowRect(hwndClockMain, &tempRect);
	posXMainClock = tempRect.left;


	//�ʒm�{�^���A�f�X�N�g�b�v�\���{�^�����ړ�
	tempHwnd = FindWindowEx(hwndTrayMain, NULL, "TrayButton", NULL);
	if (tempHwnd)
	{
		pos.x = 0;
		pos.y = 0;
		MapWindowPoints(tempHwnd, hwndTrayMain, &pos, 1);
		if (g_bVertTaskbar) {
			pos.y += shift.y;
		}
		else {
			pos.x += shift.x;
		}
		SetWindowPos(tempHwnd, NULL, pos.x, pos.y, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING);
	}


	tempHwnd = FindWindowEx(hwndTrayMain, NULL, "TrayShowDesktopButtonWClass", NULL);
	if (tempHwnd)
	{
		pos.x = 0;
		pos.y = 0;
		MapWindowPoints(tempHwnd, hwndTrayMain, &pos, 1);
		if (g_bVertTaskbar) {
			pos.y += shift.y;
		}
		else {
			pos.x += shift.x;
		}
		SetWindowPos(tempHwnd, NULL, pos.x, pos.y, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING);
	}


	//�T�C�Y�X�V������AhdcClock����蒼���悤�ɂ���B
	CreateClockDC();

	//	SetAllSubClocks();	//���C���N���b�N�̏�Ԃ��ς������A����T�u�N���b�N�����f������K�v����B
	//->�����Ɏ��s����Ƃ��܂��s���Ȃ����������J��Ԃ����̂Ńf�B���C�Ŏ��s
	if (bEnableSubClks) {
		SetTimer(hwndClockMain, IDTIMERDLL_DELEYED_RESPONSE, 500, NULL);
	}
}

void CheckPixel_Win10(int posX, int posY)
{
	HWND tempHwnd = NULL;
	HDC tempDC = NULL;
	RECT tempRect;
	COLORREF tempCol;

	tempHwnd = GetDesktopWindow();	//GetDesktopWindow()���f�X�N�g�b�v
	GetWindowRect(tempHwnd, &tempRect);
	tempDC = GetDC(tempHwnd);
	if (tempDC) {
		tempCol = GetPixel(tempDC, posX, posY);

		if (b_DebugLog) {
			writeDebugLog_Win10("[tclock.c][GetPixelForCheck_Win10] posX =", posX);
			writeDebugLog_Win10("[tclock.c][GetPixelForCheck_Win10] posY =", posY);
			writeDebugLog_Win10("[tclock.c][GetPixelForCheck_Win10] red =", GetRValue(tempCol));
			writeDebugLog_Win10("[tclock.c][GetPixelForCheck_Win10] green =", GetGValue(tempCol));
			writeDebugLog_Win10("[tclock.c][GetPixelForCheck_Win10] blue =", GetBValue(tempCol));
		}

		ReleaseDC(tempHwnd, tempDC);
	}
}

void GetTaskbarColor_Win11Type2(void)
{
	HWND tempHwnd = NULL;
	HDC tempDC = NULL;
	RECT tempRect;
	int posX, posY;

	GetWindowRect(hwndTaskBarMain, &tempRect);
	posX = (posXMainClock < 10 ? 0 : (posXMainClock - 10));
	posY = tempRect.top;


	tempHwnd = GetDesktopWindow();	//GetDesktopWindow()���f�X�N�g�b�v
	GetWindowRect(tempHwnd, &tempRect);
	tempDC = GetDC(tempHwnd);
	if (tempDC) {
//		originalColorTaskbar = GetPixel(tempDC, tempRect.left, tempRect.bottom - 1);	// bottom, right�� 1 �����Ȃ��Ƃ͂ݏo��CLR_INVALID�ɂȂ�B
//		originalColorTaskbarEdge = GetPixel(tempDC, tempRect.left, tempRect.bottom - originalHeightTaskbar);	// Taskbar��[1�h�b�g�����F���Ⴄ�E�E�E

		originalColorTaskbar = GetPixel(tempDC, posX, tempRect.bottom - 1);	// bottom, right�� 1 �����Ȃ��Ƃ͂ݏo��CLR_INVALID�ɂȂ�B
		originalColorTaskbarEdge = GetPixel(tempDC, posX, posY);	// Taskbar��[1�h�b�g�����F���Ⴄ�B

		ReleaseDC(tempHwnd, tempDC);

		if (b_DebugLog)writeDebugLog_Win10("[tclock.c][GetTaskbarColor_Win11Type2] originalColorTaskbar =", (int)originalColorTaskbar);
		if (b_DebugLog)writeDebugLog_Win10("[tclock.c][GetTaskbarColor_Win11Type2] originalColorTaskbarEdge =", (int)originalColorTaskbarEdge);

		if (originalColorTaskbar == CLR_INVALID) {
			if (b_DebugLog)writeDebugLog_Win10("[tclock.c][GetTaskbarColor_Win11Type2] Failed to get originalColorTaskbar", 999);
			originalColorTaskbar = RGB(0, 0, 0);
		}

		if (originalColorTaskbarEdge == CLR_INVALID) {
			if (b_DebugLog)writeDebugLog_Win10("[tclock.c][GetTaskbarColor_Win11Type2] Failed to get originalColorTaskbarEdge", 999);
			originalColorTaskbarEdge = RGB(0, 0, 0);
		}
	}
	else {
//		if (b_DebugLog)writeDebugLog_Win10("[tclock.c][GetTaskbarColor_Win11Type2] Failed to get HDC for Taskbar", 999);
	}
}

void GetTaskbarSize(void)
{
	RECT tempRect;

	GetWindowRect(hwndTaskBarMain, &tempRect);

	posXTaskbar = tempRect.left;
	posYTaskbar = tempRect.top;

	widthTaskbar = tempRect.right - tempRect.left;
	heightTaskbar = tempRect.bottom - tempRect.top;

	if (b_DebugLog){
		writeDebugLog_Win10("[tclock.c][GetTaskbarSize] widthTaskbar =", widthTaskbar);
		writeDebugLog_Win10("[tclock.c][GetTaskbarSize] heightTaskbar =", heightTaskbar);
	}

	if (heightTaskbar == 0) {
		if (b_DebugLog) writeDebugLog_Win10("[tclock.c][GetTaskbarSize] Failed to Get heightTaskbar. TClock will not work correctly....", 999);
		heightTaskbar = 48;	//������0�������ꍇ��100%�̏ꍇ�̒l������B�������Ȃ��Ə��������[�v����B�Ƃ͂��������Ȃ�Ɛ��퓮�삵�Ȃ��B
	}


}

// Added by TTTT for Win10AU (WIN10RS1) compatibility
// Imported from TClockLight-tclocklight-kt160911, dll/Wndproc.c
/*------------------------------------------------
vertical taskbar ?
--------------------------------------------------*/
BOOL IsVertTaskbar(HWND temphwndTaskBarMain)
{
	RECT tempRect;

	GetWindowRect(temphwndTaskBarMain, &tempRect);
	return (tempRect.bottom - tempRect.top) > (tempRect.right - tempRect.left);

}

/*------------------------------------------------
�@SafeMode����ƋN������(Tick)�ۑ�
 --------------------------------------------------*/
void CheckSafeMode_Win10(void)
{
	//hwnd�g���Ă��Ȃ�


	FILETIME currentLaunchFileTime;
	currentLaunchFileTime.dwHighDateTime = 0;
	currentLaunchFileTime.dwLowDateTime = 0;


	LONG currentLaunchTimeStamp = 0;
	LONG lastLaunchTimeStamp = 0;

	GetSystemTimeAsFileTime(&currentLaunchFileTime);
	currentLaunchTimeStamp = (((currentLaunchFileTime.dwHighDateTime & 0x000FFFFF) << 9)
			+ (currentLaunchFileTime.dwLowDateTime >> 23)) * 0.839;


	lastLaunchTimeStamp = GetMyRegLong("Status_DoNotEdit", "LastLaunchTimeStamp", 0);
	SetMyRegLong("Status_DoNotEdit", "LastLaunchTimeStamp", currentLaunchTimeStamp);
	LONG timeStampDifference = currentLaunchTimeStamp - lastLaunchTimeStamp;

	int count = GetMyRegLong("Status_DoNotEdit", "CountAutoRestart", 0);

	if ((timeStampDifference > 0) && (timeStampDifference < 20))
	{
		b_SafeMode = TRUE;
		SetMyRegLong("Status_DoNotEdit", "CountAutoRestart",  count + 1);
	}
	else {
		b_SafeMode = FALSE;
		SetMyRegLong("Status_DoNotEdit", "CountAutoRestart",  0);
	}



	b_ExcessNetProfiles = GetMyRegLong("Status_DoNotEdit", "ExcessNetProfiles", FALSE);
	SetMyRegLong("Status_DoNotEdit", "ExcessNetProfiles", FALSE);
	if (b_ExcessNetProfiles) b_SafeMode = TRUE;


	SetMyRegLong("Status_DoNotEdit", "SafeMode", b_SafeMode);
	
	b_DebugLog = GetMyRegLong(NULL, "DebugLog", FALSE);

	if (b_DebugLog) writeDebugLog_Win10("lastLaunchTimeStamp = ", lastLaunchTimeStamp);
	if (b_DebugLog) writeDebugLog_Win10("currentLaunchTimeStamp = ", currentLaunchTimeStamp);
	if (b_DebugLog) writeDebugLog_Win10("timeStampDifference = ", timeStampDifference);
	if (b_DebugLog) writeDebugLog_Win10("b_SafeMode = ", b_SafeMode);



}

COLORREF MyColorTT_Core(int iCPU)
{
	COLORREF returnvalue;
	if (iCPU > 66)
	{
		returnvalue = ColorBarMeterCore_High;
	}
	else if (iCPU > 33)
	{
		returnvalue = ColorBarMeterCore_Mid;
	}
	else if (iCPU > 0)
	{
		returnvalue = ColorBarMeterCore_Low;
	}
	return returnvalue;
}

COLORREF MyColorTT_GU(void)
{
	return ColorBarMeterGPU;
}

COLORREF MyColorTT_CU(void)
{
	COLORREF returnvalue;
	if (totalCPUUsage >= BarMeterCU_Threshold_High)
	{
		returnvalue = ColorBarMeterCU_High;
	}
	else if (totalCPUUsage >= BarMeterCU_Threshold_Mid)
	{
		returnvalue = ColorBarMeterCU_Mid;
	}
	else
	{
		returnvalue = ColorBarMeterCU_Low;
	}
	return returnvalue;
}

COLORREF MyColorTT_VL(void)
{
	COLORREF returnvalue;
		if (muteStatus)
		{
			returnvalue = ColorBarMeterVL_Mute;
		}
		else
		{
			returnvalue = ColorBarMeterVL;
		}
		return returnvalue;
}

COLORREF MyColorTT_BL(void)
{
	COLORREF returnvalue;

	if (b_Charging)
	{
		returnvalue = ColorBarMeterBL_Charge;
	}
	else if (iBatteryLife >= BarMeterBL_Threshold_High)
	{
		returnvalue = ColorBarMeterBL_High;
	}
	else if (iBatteryLife >= BarMeterBL_Threshold_Mid)
	{
			returnvalue = ColorBarMeterBL_Mid;
	}
	else if (iBatteryLife > 0)
	{
		returnvalue = ColorBarMeterBL_Low;
	}


	return returnvalue;
}

//Ver4.0.4���ݗ��p���Ă��Ȃ�
void checkDisplayStatus_Win10(void)
{
	DWORD dwFlags = 1;
	int iDevNum = 0;
	int activeDevNum = 0;
	int lastActiveDevNum = 0;
	BOOL primaryDevFlag[32];
	DISPLAY_DEVICE myDisplayDevice;
	myDisplayDevice.cb = sizeof(DISPLAY_DEVICE);
	DEVMODE mode;

	if (b_DebugLog) writeDebugLog_Win10("checkDisplayStatus_Win10 called", 999);


	while (EnumDisplayDevices(NULL, iDevNum, &myDisplayDevice, dwFlags))
	{
		iDevNum++;

		if (myDisplayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE)
		{
			activeDevNum++;
			lastActiveDevNum = iDevNum;
		}

		if (myDisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
		{
			primaryDevFlag[iDevNum] = TRUE;
		}
		else
		{
			primaryDevFlag[iDevNum] = FALSE;
		}


	}

}

void RestartTClockFromDLL(void)
{
	if (b_DebugLog) writeDebugLog_Win10("TClock will be restarted ...", 999);

	SetMyRegLong("Status_DoNotEdit", "LastLaunchTimeStamp", 0);
	char fname[MAX_PATH];
	strcpy(fname, g_mydir_dll);
	add_title(fname, "TClock-Win10.exe");
	ShellExecute(NULL, "open", fname, "/restart", NULL, SW_HIDE);
	EndClock();
}

BOOL IsHoliday_Win10(SYSTEMTIME* pt)
{
	if (b_English) return (FALSE);

	BOOL b_ret = FALSE;
	int Year, Month, Day, DoW;
	Year = pt->wYear;
	Month = pt->wMonth;
	Day = pt->wDay;
	DoW = pt->wDayOfWeek;

	switch(Month) {
		case 1:
			if (Day == 1 || (Day == 2 && DoW == 1)) b_ret = TRUE;	//���U
			if (Day > 7 && Day < 15 && DoW == 1) b_ret = TRUE;		//���l�̓�
			break;
		case 2:
			if (Day == 11 || (Day == 12 && DoW == 1)) b_ret = TRUE;	//�����L�O��
			if (Day == 23 || (Day == 24 && DoW == 1)) b_ret = TRUE;	//�V�c�a����
			break;
		case 3:		//�t���̓�
			if (Year == 2020 || Year == 2021 || Year == 2024 || Year == 2025 || Year == 2026 || Year == 2028 || Year == 2029 || Year == 2030)
				if (Day == 20 || (Day == 21 && DoW == 1)) b_ret = TRUE;
			if (Year == 2022 || Year == 2023 || Year == 2027)
				if (Day == 21 || (Day == 22 && DoW == 1)) b_ret = TRUE;
			break;
		case 4:
			if (Day == 29 || (Day == 30 && DoW == 1)) b_ret = TRUE;	//���a�̓�
			break;
		case 5:
			if (Day == 3) b_ret = TRUE;		//���@�L�O��
			if (Day == 4) b_ret = TRUE;		//�݂ǂ�̓�
			if (Day == 5) b_ret = TRUE;		//���ǂ��̓�
			if (Day == 6 && (DoW == 1 || DoW == 2 || DoW == 3)) b_ret = TRUE;	//�U�֋x��
			break;
		case 7:
			//if (Year == 2020) {
			//	if (Day == 23) b_ret = TRUE;
			//}
			//else {
				if (Day > 14 && Day < 22 && DoW == 1) b_ret = TRUE;		//�C�̓�
			//}
			break;
		case 8:
			//if (Year == 2020) {
			//	if (Day == 10) b_ret = TRUE;
			//}
			//else {
				if (Day == 11 || (Day == 12 && DoW == 1)) b_ret = TRUE;		//�R�̓�
			//}
			break;
		case 9:
			if (Day > 14 && Day < 22 && DoW == 1) b_ret = TRUE;		//�h�V�̓�
			//�ȉ��A�H���̓�
			if (Year == 2020 || Year == 2024 || Year == 2028)
				if (Day == 22 || (Day == 23 && DoW == 1)) b_ret = TRUE;
			if (Year == 2021 || Year == 2022 || Year == 2023 || Year == 2025 || Year == 2026 || Year == 2027 || Year == 2029 || Year == 2030)
				if (Day == 23 || (Day == 24 && DoW == 1)) b_ret = TRUE;
			break;
		case 10:
			if (Year != 2020) {
				if (Day > 7 && Day < 15 && DoW == 1) b_ret = TRUE;	//�X�|�[�c�̓�
			}
			break;
		case 11:
			if (Day == 3 || (Day == 4 && DoW == 1)) b_ret = TRUE;	//�����̓�
			if (Day == 23 || (Day == 24 && DoW == 1)) b_ret = TRUE;	//�ΘJ���ӂ̓�
	}
	return(b_ret);
}


void SetWindowVisible_Win10(HWND targetHWND, BOOL bVisibility)
{
	if (b_DebugLog) writeDebugLog_Win10("[tclock.c]SetWindowVisible_Win10 called. bVisibility = ", bVisibility);

	DWORD dwStyle = (DWORD)GetWindowLong(targetHWND, GWL_STYLE);

	if (!bVisibility)
	{
		SetWindowLongPtr(targetHWND, GWL_STYLE, dwStyle & ~WS_VISIBLE);
	}
	else
	{
		SetWindowLongPtr(targetHWND, GWL_STYLE, dwStyle | WS_VISIBLE);
	}
}



//�ȉ��̃R�[�h�͌��ݎg���Ă��Ȃ��B
//void Check_Light_Theme_Win10(void)
//{
//	if (b_DebugLog)
//	{
//		writeDebugLog_Win10("[tclock.c][Check_Light_Theme_Win10] Check_Light_Theme_Win10 called.", 999);
//	}
//
//
//	HKEY hkey;
//	DWORD reg_data;
//	DWORD regtype, size;
//	size = sizeof(DWORD);
//
//	b_System_Light_Theme = FALSE;
//	b_Apps_Light_Theme = FALSE;
//	b_Transparency_Theme = FALSE;
//
//	if (RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS) {
//		if (RegQueryValueEx(hkey, "AppsUseLightTheme", 0, &regtype, (LPBYTE)&reg_data, &size) == ERROR_SUCCESS)
//		{
//			if (reg_data == 1) b_Apps_Light_Theme = TRUE;
//			if (b_DebugLog)
//			{
//				writeDebugLog_Win10("[tclock.c][Check_Light_Theme_Win10] b_Apps_Light_Theme = ", b_Apps_Light_Theme);
//			}
//		}
//		if (RegQueryValueEx(hkey, "SystemUsesLightTheme", 0, &regtype, (LPBYTE)&reg_data, &size) == ERROR_SUCCESS)
//		{
//			if (reg_data == 1) b_System_Light_Theme = TRUE;
//			if (b_DebugLog)
//			{
//				writeDebugLog_Win10("[tclock.c][Check_Light_Theme_Win10] b_System_Light_Theme = ", b_System_Light_Theme);
//			}
//		}
//		if (RegQueryValueEx(hkey, "EnableTransparency", 0, &regtype, (LPBYTE)&reg_data, &size) == ERROR_SUCCESS)
//		{
//			if (reg_data == 1) b_Transparency_Theme = TRUE;
//			if (b_DebugLog)
//			{
//				writeDebugLog_Win10("[tclock.c][Check_Light_Theme_Win10] b_Transparency_Theme = ", b_Transparency_Theme);
//			}
//		}
//
//		RegCloseKey(hkey);
//	}
//}

