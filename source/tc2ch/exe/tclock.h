/*-------------------------------------------
  tclock.h
      KAZUBON  1997-1999
---------------------------------------------*/


//#define _WIN32_IE 0x0500
//#define WINVER    0x0500

#define _WIN32_IE 0x0500
#define _WIN32_WINNT 0x0600
#define WINVER    0x0600

#define _CRT_SECURE_NO_DEPRECATE

#define LANGUAGE_OFFSET_JAPANESE 0	//Added by TTTT
#define LANGUAGE_OFFSET_ENGLISH 1000	//Added by TTTT

#pragma warning(push, 0)
# include <windows.h>
# include <windowsx.h>
# include <tchar.h>
# include <mmsystem.h>
# include <shlobj.h>
# include <shlwapi.h>	//Added by TTTT for  PathFileExists
#pragma warning(pop)


#include "resource.h"
#include "../language/resource.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "wsock32.lib")
//#pragma comment(lib, "imm32.lib")	//Added by TTTT use standard ImmDisableIME() in exemain.c, not same function defined in exemain.c
//#pragma comment(lib, "shell32.lib") //Added by TTTT for shellexecute

#ifdef _WIN64
//# if defined(_DEBUG)
//#  pragma comment(lib, "../../x64/debug/tcdll.lib")
//# else
#  pragma comment(lib, "../../x64/release/tcdll.lib")
//# endif // _DEBUG
#else
//# if defined(_DEBUG)
//#  pragma comment(lib, "../../debug/tcdll.lib")
//# else
#  pragma comment(lib, "../../release/tcdll.lib")
//# endif // _DEBUG
#endif  // _WIN64


// replacement of standard library's functions
#define malloc(s) HeapAlloc(GetProcessHeap(), 0, s)
#define free(p)   HeapFree(GetProcessHeap(), 0, p);





# undef memcpy
# define memcpy(d,s,l) r_memcpy(d,s,l)
# undef memset
# define memset(d,c,l) r_memset(d,c,l)
# undef strlen
# define strlen(s) lstrlen(s)
# undef strcpy
# define strcpy(d, s) lstrcpy(d, s)
# undef strcat
# define strcat(d, s) lstrcat(d, s)
void r_memcpy(void *d, const void *s, size_t l);
void r_memset(void *d, int c, size_t l);



#undef atoi
#define atoi(p) r_atoi(p)
#undef _strnicmp
#define _strnicmp(d,s,n) r_strnicmp(d,s,n)
#undef _stricmp
#define _stricmp(d,s) r_stricmp(d,s)
int r_atoi(const char *p);
int r_strnicmp(const char* d, const char* s, size_t n);
int r_stricmp(const char* d, const char* s);

// IDs for timer
#define IDTIMER_START       2
#define IDTIMER_MAIN        3
#define IDTIMER_MOUSE       4

#define IDTIMER_CREATE      7

#define IDTIMER_ZOMBIECHECK	5

#define IDTIMER_UPDATE_TEMP	6

// messages to send the clock
#define CLOCKM_REFRESHCLOCK   (WM_USER+1)
#define CLOCKM_REFRESHTASKBAR (WM_USER+2)


#define CLOCKM_VISTACALENDAR  (WM_USER+102)

#define CLOCKM_SHOWAVAILABLENETWORKS  (WM_USER+103)


#define CLOCKM_TOGGLE_AUTOPAUSE_CLOUDAPP	(WM_USER+201)	//Added by TTTT
#define CLOCKM_TOGGLE_DATAPLANFUNC	(WM_USER+202)	//Added by TTTT
#define CLOCKM_TOGGLE_BARMETER_VOLUME	(WM_USER+203)	//Added by TTTT
#define CLOCKM_TOGGLE_BARMETER_CPU		(WM_USER+204)	//Added by TTTT
#define CLOCKM_TOGGLE_BARMETER_BATTERY	(WM_USER+205)	//Added by TTTT
#define CLOCKM_DISPLAYSTATUS_CHECK	(WM_USER+206)	//Added by TTTT
#define CLOCKM_BGCOLOR_UPDATE	(WM_USER+207)	//Added by TTTT
#define CLOCKM_ZOMBIECHECK_CALL	(WM_USER+208)	//Added by TTTT



#define CLOCKM_SHOWPROFILELIST	(WM_USER+210)	//Added by TTTT
#define CLOCKM_DLLALIVE  (WM_USER+211)	//Added by TTTT

#define CLOCKM_SLEEP_IN  (WM_USER+220)	//Added by TTTT
#define CLOCKM_SLEEP_AWAKE  (WM_USER+221)	//Added by TTTT

#define CLOCKM_REQUEST_TEMPCOUNTERINFO	(WM_USER + 230)

#define CLOCKM_TRAYICONMSG	(WM_USER + 240)
#define ID_TRAYICON		100

// for mouse.c and pagemouce.c
#define MOUSEFUNC_NONE       -1
#define MOUSEFUNC_DATETIME    0

#define MOUSEFUNC_PROPERTY    10
#define MOUSEFUNC_VISTACALENDAR    11
#define MOUSEFUNC_SHOWAVAILABLENETWORKS    12

#define MOUSEFUNC_FILELIST    26
#define MOUSEFUNC_OPENFILE    100
#define MOUSEFUNC_TASKMGR	  200		//Added by TTTT
#define MOUSEFUNC_POWERPNL	  201		//Added by TTTT
#define MOUSEFUNC_DATAUSAGE	  202		//Added by TTTT

#define MOUSEFUNC_NETWORKSTG	  204		//Added by TTTT
#define MOUSEFUNC_NETWORKPNL	  205		//Added by TTTT
#define MOUSEFUNC_CONTROLPNL	206		//Added by TTTT
#define MOUSEFUNC_SETTING		207		//Added by TTTT
#define MOUSEFUNC_CMD			208		//added by TTTT

#define MOUSEFUNC_ALARM_CLOCK			209		//added by TTTT


#define MAX_AUTORESTART			2


typedef struct{
	BYTE	key;
	DWORD	flag;
} KEYEVENT,*LPKEYEVENT;

typedef struct{
	signed short mousefunc;
	signed short idstring;
} MOUSE_FUNC_INFO;

// exemain.c
extern HINSTANCE g_hInst;           // instance handle
extern HINSTANCE g_hInstResource;   // instance handle of language module
extern HWND      g_hwndMain;        // main window
extern HWND      g_hwndClock;       // clock window
extern HWND      g_hwndPropDlg;     // property sheet window
//extern HWND      g_hDlgTimer;       // timer dialog

//extern HICON     g_hIconTClock, g_hIconPlay, g_hIconStop, g_hIconDel;
                                    // icons to use frequently
extern HICON     g_hIconTClock;
extern char      g_mydir[];         // path to tclock.exe
extern char      g_langdllname[];   // language dll name
extern BOOL      g_bIniSetting;
extern char      g_inifile[];
MOUSE_FUNC_INFO *GetMouseFuncList(void);
int GetMouseFuncCount(void);
HINSTANCE LoadLanguageDLL(char *fname); // load language dll
HINSTANCE GetLangModule(void);      // instance handle of language dll

void My2chHelp(HWND hDlg);  // show 2ch help

//void CreateTClockTrayIcon(HWND hWnd, PNOTIFYICONDATA lp_notifyicon);	//20220323 Ver4.12, by MMM
void CreateTClockTrayIcon(BOOL bCreate);	//20220323 Ver4.12, by MMM


// alarm.c -> exemain.c
void GetFileAndOption(const char* command, char* fname, char* opt);
BOOL ExecFile(HWND hwnd, char* command);


// pagebarmeter.c
void CreateBarMeterColorDialog(int index);


// propsheet.c
extern BOOL g_bApplyClock;
extern BOOL g_bApplyTaskbar;
extern BOOL g_bApplyLangDLL;
void MyPropertyDialog(void);
BOOL SelectMyFile(HWND hDlg, const char *filter, DWORD nFilterIndex,
	const char *deffile, char *retfile);

// pageformat.c
void InitFormat(void);
void CreateFormat(char* s, int* checks);

// menu.c
void OnContextMenu(HWND hwnd, HWND hwndClicked, int xPos, int yPos);
void OnTClockCommand(HWND hwnd, WORD wID, WORD wCode);

// mouse.c
void InitMouseFunction(HWND hwnd);
void EndMouseFunction(HWND hwnd);
void ResetHotkey(HWND hwnd);
void OnHotkey(HWND hwnd, int id);
void ExecuteMouseFunction(HWND hwnd, LONG fnc, int btn, int clk);
void OnDropFiles(HWND hwnd, HDROP hdrop);
void OnMouseMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void OnTimerMouse(HWND hwnd);
void PushKeybd(LPKEYEVENT lpkey);

// utl.c
int atox(const char *p);
void add_title(char *path, char* titile);
void del_title(char *path);
void get_title(char* dst, const char *path);
int ext_cmp(const char *fname, const char *ext);
void parse(char *dst, char *src, int n);
void parsechar(char *dst, char *src, char ch, int n);
void str0cat(char* dst, const char* src);
char* MyString(UINT id);
int MyMessageBox(HWND hwnd, char* msg, char* title, UINT uType, UINT uBeep);
int GetLocaleInfoWA(int ilang, LCTYPE LCType, char* dst, int n);
DWORDLONG M32x32to64(DWORD a, DWORD b);
void SetForegroundWindow98(HWND hwnd);
//void WriteDebug(const char* s);	 //WriteDebug_New2()Ç…à⁄çs by TTTT
//void WriteDebug_New(const char* s);		//Added by TTTT
void WriteDebug_New2(const char* s);		//Added by TTTT
void WriteNormalLog(const char* s);		//Added by TTTT
void CheckNormalLog(void);		//Added by TTTT
int GetMyRegStr(char* section, char* entry, char* val, int cbData,
	char* defval);
LONG GetMyRegLong(char* section, char* entry, LONG defval);
BOOL SetMyRegStr(char* subkey, char* entry, char* val);
BOOL SetMyRegLong(char* subkey, char* entry, DWORD val);
BOOL DelMyReg(char* subkey, char* entry);
BOOL DelMyRegKey(char* subkey);
int DelRegAll(void);

HWND FindVistaCalenderWindow(void);	//originally in calendar.c


void AdjustConboBoxDropDown(HWND hComboBox, int nDropDownCount);



// tcdll.dllÇÃAPI
void WINAPI HookStart(HWND hwnd);
void WINAPI HookEnd(void);
int WINAPI CheckWinVersion_Win10(void);
BOOL WINAPI CheckModernStandbyCapability_Win10(void);

// Macros

#define EnableDlgItem(hDlg,id,b) EnableWindow(GetDlgItem((hDlg),(id)),(b))
#define ShowDlgItem(hDlg,id,b) ShowWindow(GetDlgItem((hDlg),(id)),(b)?SW_SHOW:SW_HIDE)
#define AdjustDlgConboBoxDropDown(hDlg,id,b) AdjustConboBoxDropDown(GetDlgItem((hDlg),(id)),(b))

#define CBAddString(hDlg,id,lParam) SendDlgItemMessage((hDlg),(id),CB_ADDSTRING,0,(lParam))
#define CBDeleteString(hDlg,id, i) SendDlgItemMessage((hDlg),(id),CB_DELETESTRING,(i),0)
#define CBFindString(hDlg,id,s) SendDlgItemMessage((hDlg),(id),CB_FINDSTRING,0,(LPARAM)(s))
#define CBFindStringExact(hDlg,id,s) SendDlgItemMessage((hDlg),(id),CB_FINDSTRINGEXACT,0,(LPARAM)(s))
#define CBGetCount(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_GETCOUNT,0,0)
#define CBGetCurSel(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_GETCURSEL,0,0)
#define CBGetItemData(hDlg,id,i) SendDlgItemMessage((hDlg),(id),CB_GETITEMDATA,(i),0)
#define CBGetLBText(hDlg,id,i,s) SendDlgItemMessage((hDlg),(id),CB_GETLBTEXT,(i),(LPARAM)(s))
#define CBInsertString(hDlg,id,i,s) SendDlgItemMessage((hDlg),(id),CB_INSERTSTRING,(i),(LPARAM)(s))
#define CBResetContent(hDlg,id) SendDlgItemMessage((hDlg),(id),CB_RESETCONTENT,0,0)
#define CBSetCurSel(hDlg,id,i) SendDlgItemMessage((hDlg),(id),CB_SETCURSEL,(i),0)
#define CBSetItemData(hDlg,id,i,lParam) SendDlgItemMessage((hDlg),(id),CB_SETITEMDATA,(i),(lParam))

// page.c
typedef struct {
	WORD id;
	char *colname;
	COLORREF defcol;
} COMBOCOLOR;

void ClearComboColor(HWND hDlg, WORD id);
void InitComboColor(HWND hDlg, int numcol, COMBOCOLOR *pCombocolor, int maxcol, BOOL ex_flg);
void OnMeasureItemColorCombo(LPARAM lParam);
void OnDrawItemColorCombo(LPARAM lParam, int maxcol);
void OnChooseColor(HWND hDlg, WORD id, int maxcol);

// filelist.c
void OnMeasureItem(HWND hwnd, WPARAM wParam, LPARAM lParam);
void OnDrawItem(HWND hwnd, WPARAM wParam, LPARAM lParam);
void OnInitMenuPopup(HWND hwnd, WPARAM wParam, LPARAM lParam);
void OnMenuRButtonUp(HWND hwnd, WPARAM wParam, LPARAM lParam);
void showUserMenu(HWND hwnd, HWND hwndClicked, int xPos, int yPos, int btn, int clk);


