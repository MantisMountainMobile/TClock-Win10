/*-------------------------------------------
�@TCDLL.H
�@KAZUBON 1997-2001
---------------------------------------------*/

	//#define _WIN32_IE 0x0500
	//#define WINVER    0x0500

#define _WIN32_IE 0x0500
#define _WIN32_WINNT 0x0600
#define WINVER    0x0600

#define _CRT_SECURE_NO_DEPRECATE

#pragma warning(push, 0)
# include <windows.h>
# include <windowsx.h>
# include <tchar.h>
# include <winreg.h>
# include <shellapi.h>
# include <winnls.h>
# include <commctrl.h>
# include <shlwapi.h>
# include <shlobj.h>
#pragma warning(pop)

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "comctl32.lib")	// Added by TTTT for Win10AU (WIN10RS1) compatibility
#pragma comment(lib, "Dxva2.lib")		//Added by TTTT for GetNumberOfPhysicalMonitorsFromHMONITOR function in tclock.c, myMonitorEnumProc() (Check Display Status) 

#if defined(_DEBUG)
# include <crtdbg.h>
#else
# define _ASSERTE(expr) ((void)0)
#endif

#ifdef _WIN64
# pragma comment(linker, "/base:0x0000070066060000")
#else
# pragma comment(linker,         "/base:0x66060000")
#endif // _WIN64


#define ENABLE_CHECK_SUBCLASS_NESTING 1

#define CLOCKM_REFRESHCLOCK   (WM_USER+1)
#define CLOCKM_REFRESHTASKBAR (WM_USER+2)


#define CLOCKM_VISTACALENDAR  (WM_USER+102)
//#define CLOCKM_VISTATOOLTIP   (WM_USER+103)
#define CLOCKM_SHOWAVAILABLENETWORKS  (WM_USER+103)

#define CLOCKM_TOGGLE_AUTOPAUSE_CLOUDAPP	(WM_USER+201)	//Added by TTTT
#define CLOCKM_TOGGLE_DATAPLANFUNC	(WM_USER+202)	//Added by TTTT

#define CLOCKM_TOGGLE_BARMETER_VOLUME	(WM_USER+203)	//Added by TTTT
#define CLOCKM_TOGGLE_BARMETER_CPU		(WM_USER+204)	//Added by TTTT
#define CLOCKM_TOGGLE_BARMETER_BATTERY	(WM_USER+205)	//Added by TTTT
#define CLOCKM_DISPLAYSTATUS_CHECK	(WM_USER+206)	//Added by TTTT
#define CLOCKM_BGCOLOR_UPDATE	(WM_USER+207)	//Added by TTTT
#define CLOCKM_ZOMBIECHECK_CALL	(WM_USER+208)	//Added by TTTT
#define CLOCKM_MOVEWIN11CONTENTBRIDGE (WM_USER+209)



#define CLOCKM_SHOWPROFILELIST	(WM_USER+210)	//Added by TTTT
#define CLOCKM_DLLALIVE  (WM_USER+211)	//Added by TTTT

#define CLOCKM_SLEEP_IN  (WM_USER+220)	//Added by TTTT
#define CLOCKM_SLEEP_AWAKE  (WM_USER+221)	//Added by TTTT

#define IDC_EXIT					102
#define IDC_RESTART					40007

#define AC_SRC_ALPHA                0x01

#define IDTIMERDLL_DLLMAIN			11
#define IDTIMERDLL_SYSINFO			12
#define IDTIMERDLL_UPDATESUBCLKS	13
#define IDTIMERDLL_CHECKNETSTAT		14
#define IDTIMERDLL_GRAPH			15
#define IDTIMERDLL_TIP				16
//#define IDTIMERDLL_MOVEWIN11CONTENTBRIDGE	17


//�T�u�X�N���[���^�X�N�o�[�̍ő又���� 20211107 TTTT
#define MAX_SUBSCREEN				8

// tclock.c
void DrawClock(HWND hwnd, HDC hdc);
void DrawClock_New(HDC hdc);
//void FillClock(HWND hwnd, HDC hdc, RECT *prc, int nblink);
void FillClock();
void checkDisplayStatus_Win10(void); //added by TTTT


// utl.c
extern BOOL g_bIniSetting;
extern char g_inifile[];
int _strncmp(const char* d, const char* s, size_t n);

HFONT CreateMyFont(char* fontname, int fontsize,
	LONG weight, LONG italic);

int ext_cmp(char *fname, char *ext);
//void add_title(char *path, char *title);
void del_title(char *path);



void parse(char *dst, char *src, int n);
char* MyString(UINT id);



// FORMAT.C
void InitFormat(SYSTEMTIME* lt);
void MakeFormat(char* s, char* s_info, SYSTEMTIME* pt, int beat100, char* fmt);
//void MakeFormat_Colored(char* s, COLORREF* s_col,  SYSTEMTIME* pt, int beat100, char* fmt);
#define FORMAT_SECOND    0x0001
#define FORMAT_SYSINFO   0x0002
#define FORMAT_BEAT1     0x0004
#define FORMAT_BEAT2     0x0008
#define FORMAT_BATTERY   0x0010
#define FORMAT_MEMORY    0x0020
#define FORMAT_MOTHERBRD 0x0040
#define FORMAT_PERMON    0x0080
#define FORMAT_NET       0x0100
#define FORMAT_HDD       0x0200
#define FORMAT_CPU       0x0400
#define FORMAT_VOL       0x0800
#define FORMAT_DISPLAY     0x1000
DWORD FindFormat(char* fmt);

SYSTEMTIME CalcTimeDifference_Win10(SYSTEMTIME*, int, int, BOOL);
SYSTEMTIME CalcTimeDifference_US_Win10(SYSTEMTIME*, int, int, BOOL);
SYSTEMTIME CalcTimeDifference_Europe_Win10(SYSTEMTIME*, int, int, BOOL);


//for_win11.c
void CreateWin11MainClock(void);
HWND CreateWin11SubClock(HWND tempHwndTaskbar);
void CreateWin11Notify(void);
void ReCreateWin11Notify(void);
void LoadBitMapWin11Notify(void);
void GetWin11ElementSize(void);
void GetWin11TrayWidth(void);
LRESULT CALLBACK SubclassTrayProc_Win11(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
LRESULT CALLBACK WndProcWin11Notify(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void DrawWin11Notify(BOOL b_forceUpdate);
void SetModifiedWidthWin11Tray(void);
void SetMainClockOnTasktray_Win11(void);
void GetWin11TaskbarType(void);
//void DelayedMoveWin11ContentBridge(void);
void MoveWin11ContentBridge(void);

//subclock.c
LRESULT CALLBACK WndProcSubClk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void GetOrigSubClkDimensions(void);
void StoreSpecificSubClockDimensions(int i);
void ActivateSubClocks(void);
void FindAllSubClocks(void);
void DisableAllSubClocks(void);
void DelayedUpdateSubClks(void);
void DisableSpecificSubClock(int i);
void SetAllSubClocks(void);
void SetSpecificSubClock(int i);
int GetSubClkIndexFromHWND(HWND tempHwndSubClk);
void CalcSpecificSubClockSize(int i);
void CheckSubClocks(void);
BOOL GetInkWorkspaceSetting(void);
void CreateMyNotificationFont(int fontwidth, int fontheigt);
void ClearSpecificSubClock(int i);



// traynotifywnd.c
//void InitTrayNotify(HWND hwnd);
//void EndTrayNotify(void);

// BMP.C
HBITMAP ReadBitmap(HWND hwnd, char* fname, BOOL b);

// newapi.c
void EndNewAPI(HWND hwnd);
void SetLayeredWindow(HWND hwnd, INT alphaTip, COLORREF colBack);
void SetLayeredTaskbar(HWND hwndClock);
//void InitLayeredStartMenu(HWND hwndClock);
//void SetLayeredStartMenu(HWND hwnd);
//void GradientFillClock(HDC hdc, RECT* prc, COLORREF col1, COLORREF col2, DWORD grad);
void GradientFillClock(COLORREF col1, COLORREF col2, DWORD grad);
HRESULT MyAlphaBlend(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,
  HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, BLENDFUNCTION blendFunction);
HRESULT MyTransparentBlt(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,
  HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, UINT crTransparent);
HRESULT MyDrawThemeParentBackground(HWND hwnd,HDC hdc, RECT* prc);

// sysres.c
//int GetFreeSystemResources(WORD wSysRes);
void EndSysres(void);

void InitBatteryLife(void);
int GetBatteryLifePercent(void);
void FreeBatteryLife(void);
void CheckBatteryAvailability(void);

void InitCpuClock(void);
void UpdateCpuClock(void);
void FreeCpuClock(void);

// cpu.c
void CpuMoni_start(void);
int CpuMoni_get(void);
void CpuMoni_end(void);


// permon.c
void PerMoni_start(void);
int PerMoni_get(void);
void PerMoni_end(void);

// net.c
void Net_start(void);
//void Net_restart(void);

void Net_getRecvSend_Win10(double*, double*, double*, double*, double*, double*, double*, double*);
void Net_get(void);
void Net_end(void);


void Net_UpdateInterfaceTable(void);

// hdd.c
void Hdd_get(void);

/* tooltip.c */
void TooltipInit(HWND hwnd);
void TooltipReadData(void);
void TooltipDeleteRes(void);
void TooltipEnd(HWND hwnd);
DWORD TooltipFindFormat(void);
void TooltipOnTimer(HWND hwnd);
void TooltipOnRefresh(HWND hwnd);
BOOL TooltipOnNotify(LRESULT *plRes, LPARAM lParam);
void TooltipOnMouseEvent(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

//void Check_Light_Theme_Win10(void);



// Added by TTTT for Win10AU (WIN10RS1) compatibility
// Windows version flag
#define WIN95    0x0001   // 95,98,Me
#define WIN98    0x0002   // 98,Me
#define WINME    0x0004   // Me
#define WINNT    0x0008   // NT4,2000,XP,Vista,Win7, ...
#define WIN2000  0x0010   // 2000,XP,Vista,Win7, ...
#define WINXP    0x0020   // XP,Vista,Win7, ...
#define WINVISTA 0x0040   // Vista,Win7, ...
#define WIN7     0x0080   // Win7, ...
#define WIN8     0x0100   // Win8, ...
#define WIN8_1   0x0200   // Win8.1, ...
#define WIN10    0x0400   // Win10,Win10TH2,Win10RS1
#define WIN10TH2 0x0800   // Win10TH2,Win10RS1
#define WIN10RS1 0x1000   // Win10RS1

// Added by TTTT for DataPlanInformation
#ifdef __cplusplus
extern "C" {
#endif
	void chkInternetConnectionProfile_Win10(void);

	void identifyInternetConnectProfNum_Win10(void);
	void newCodes_startup_Win10(void);
	void newCodes_close_Win10(void);
	void writeDebugLog_Win10(LPSTR s, int n);
	void getMasterVolume_Win10(void); //added for new iVolume function
	void initializeVolume_Win10(void); //added for new iVolume function
	int WINAPI CheckWinVersion_Win10(void);	//added for currently reliable versioncheck
	BOOL WINAPI CheckModernStandbyCapability_Win10(void);

	void toggleBarMeterFunc_Win10(int f);
	BOOL updateConnectProfsInfo_Win10(BOOL b_Detail);

	void saveAndOpenProfTable(BOOL b_Open);

	BOOL chkExistProf_Win10();

	void EndClock();
	void RestartTClockFromDLL(void);


	int GetMyRegStr(char* section, char* entry, char* val, int cbData,
		char* defval);
	BOOL SetMyRegStr(char* subkey, char* entry, char* val);


	void WriteNormalLog_DLL(const char* s);

	void SuspendClearLog_Win10(void);
	void RecoverClearLog_Win10(void);

	void WriteDebugDLL_New(LPSTR s);	//Adde by TTTT

	//Following lines have been moved from above lines for call from cpp codes.
	void add_title(char *path, char *title);
	LONG GetMyRegLong(char* section, char* entry, LONG defval);
	BOOL SetMyRegLong(char* subkey, char* entry, DWORD val);
	//void WriteDebug(LPSTR s); // WriteDebugDLL_New()�Ɉڍs by TTTT
	void get_title(char* dst, const char *path);	//	copied from utl.c in tclock by TTTT


	BOOL Net_getAdapterDescriptor(GUID targetGUID);

	int GetNotificationNumber(void);

	int GetFocusAssistState(void);


#ifdef __cplusplus
};
#endif