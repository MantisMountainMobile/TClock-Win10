/*-------------------------------------------------------------
  main.c
  WinMain, window procedure, and functions for initializing
---------------------------------------------------------------*/

#include "tclock.h"
#include <winver.h>


// Globals
HINSTANCE g_hInst;           // instance handle
HINSTANCE g_hInstResource;   // instance handle of language module
HWND      g_hwndMain = 0;    // main window
HWND      g_hwndClock;       // clock window
HWND      g_hwndPropDlg;     // property sheet window

HICON     g_hIconTClock;
char      g_mydir[MAX_PATH]; // path to tclock.exe
char      g_langdllname[MAX_PATH];  // language dll name
BOOL      g_bIniSetting = TRUE;

char      g_inifile[MAX_PATH];		//フルパスつき*.iniファイル名

BOOL	b_DisplayChanged = FALSE;






// スワップアウトさせる /WS:AGGRESSIVE			
// 20181125 Ver3.3.2.1でコードからはコメントアウトして様子を見る。スワップアウトしても0.3-2MB程度が、スワップしなければ最大3.5MB程度。
#define DO_WS_AGGRESSIVE() \
          SetProcessWorkingSetSize(GetCurrentProcess(), (SIZE_T)-1, (SIZE_T)-1);



#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL  0x020A
#endif
#ifndef WM_MENURBUTTONUP
#define WM_MENURBUTTONUP 0x0122
#endif


static DWORD exeVersionM = 0;
static DWORD exeVersionL = 0;
char exeVersionString[32];

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	//tclock.exe本体のウィンドウプロシージャコールバック

char szClassName[] = "TClockMainClass"; // window class name
char szWindowText[] = "TClock";         // caption of the window	(TClock-Win10にする？)

static BOOL bMenuOpened = FALSE;
static BOOL bDestroy = FALSE;

void CheckCommandLine(HWND hwnd);
static void OnTimerMain(HWND hwnd);
static void InitError(int n);
static BOOL CheckTCDLL(void);
static BOOL CheckDLL(char *fname);
static void CheckRegistry(void);
static BOOL IsWow64(void);
static BOOL CheckRegistry_Win10(void); //Added by TTTT
static void CreateDefaultIniFile_Win10(char *fname); //Added by TTTT
void getExeVersion(char *fname); //Added by TTTT
void SetIdlePriority(void);		//Added by TTTT

//void OnTimerZombieCheck(HWND hwnd); //Added by TTTT
void OnTimerZombieCheck2(HWND hwnd);


void OnDLLAliveMessage(WPARAM tempwParam); //Added by TTTT

void TerminateTClock(HWND hwnd);

void TerminateTClockFromDLL(HWND hwnd);

BOOL WaitQuitPrevTClock(int cycle);

static BOOL IsUserAdmin(void);


static BOOL AddMessageFilters(void);

static UINT s_uTaskbarRestart = 0;
static BOOL bcontractTimer = FALSE;
//static int nCountFindingClock = -1;

BOOL b_DebugLog = FALSE;		//added by TTTT
BOOL b_DebugLog_RegAccess = FALSE;
BOOL b_DebugLog_Specific = FALSE;

BOOL b_NormalLog = FALSE;		//added by TTTT


BOOL b_EnglishMenu =FALSE;		//Added by TTTT


// XButton Messages
#define WM_XBUTTONDOWN 0x020B
#define WM_XBUTTONUP   0x020C

// menu.c
extern HMENU g_hMenu;

BOOL b_AutoRestart = TRUE;

BOOL b_UnplugDriveAvailable = FALSE;
void CheckUnplugDrive(void);

int Language_Offset = 0;


//BOOL b_AcceptRisk;

//BOOL b_RestartDLL = FALSE;
BOOL b_RestartNOW = FALSE;

BOOL b_Exit = FALSE;

int zombieCheckInterval = 5;
int TaskbarThreadID = 0;

BOOL b_FlagDLLAlive = TRUE;

LONG lastFileTimeDLLAlive = 0;

HWND hwndTaskBar_Prev = NULL;


BOOL b_ModernStandbySupported = FALSE;

int countRestart = 0;

/*-------------------------------------------------------
    mouse function list
---------------------------------------------------------*/
static MOUSE_FUNC_INFO mouse_func_list[] = {
	{	MOUSEFUNC_NONE,			IDS_NONE},
	{ MOUSEFUNC_VISTACALENDAR,		IDS_VISTACALENDAR },
	{ MOUSEFUNC_ALARM_CLOCK,		IDS_ALARM_CLOCK2 },
	{ MOUSEFUNC_SHOWAVAILABLENETWORKS,		IDS_SHOWAVAILABLENETWORKS },
	{ MOUSEFUNC_TASKMGR,		IDS_TASKMGR },	//Added by TTTT
	{ MOUSEFUNC_CMD,			IDS_CMD },		//Added by TTTT
	{ MOUSEFUNC_CONTROLPNL,		IDS_CONTROLPNL },	//Added by TTTT
	{ MOUSEFUNC_POWERPNL,		IDS_POWERPNL },	//Added by TTTT
	{ MOUSEFUNC_NETWORKPNL,	IDS_NETWORKPNL },	//Added by TTTT
	{ MOUSEFUNC_SETTING,		IDS_SETTING },	//Added by TTTT
	{	MOUSEFUNC_NETWORKSTG,	IDS_NETWORKSTG},	//Added by TTTT
	{ MOUSEFUNC_DATAUSAGE,	IDS_DATAUSAGE },	//Added by TTTT
	{ MOUSEFUNC_DATETIME,		IDS_PROPDATE },
	{	MOUSEFUNC_OPENFILE,		IDS_OPENFILE},
	{	MOUSEFUNC_FILELIST,		IDS_FILELIST},
	{	MOUSEFUNC_PROPERTY,		IDS_PROPERTY}
};

MOUSE_FUNC_INFO *GetMouseFuncList(void)
{
	return mouse_func_list;
}
int GetMouseFuncCount(void)
{
	return sizeof(mouse_func_list) / sizeof(MOUSE_FUNC_INFO);
}



/*-------------------------------------------------------
Wait Until Previous TClock Process Disappear
---------------------------------------------------------*/
BOOL WaitQuitPrevTClock(int cycle)
{
	BOOL ret = FALSE;
	HWND hwnd;

	for (int i = 0; i < cycle; i++)
	{
		hwnd = FindWindow(szClassName, szWindowText);		//プロセスがすでに起動していたらhwnd != NULLになる
		if (hwnd == NULL) return FALSE;
		Sleep(100);
	}

	MessageBox(NULL, "再起動を試みましたが、既存のTClock-Win10のプロセス終了に失敗しました。現時点で正常に時計が改造されていない場合は、タスクマネージャーからTClock-Win10のプロセスを強制終了してください。\n\nFailed to terminate the previous TClock-Win10 proccess. If you don't see the modified Clock on Taskbar, please kill the previous TClock-Win10 in the Taskmanager.",
		"TClock-Win10", MB_ICONEXCLAMATION | MB_SETFOREGROUND);

	return TRUE;
}


/*-------------------------------------------------------
Check UnplugDrive.exe availability
---------------------------------------------------------*/
void CheckUnplugDrive(void)
{

	char fname[MAX_PATH];
	strcpy(fname, g_mydir);
	add_title(fname, "UnplugDrive.exe");

	//b_UnplugDriveAvailable = TRUE;

	b_UnplugDriveAvailable = PathFileExists(fname);

}



/*-------------------------------------------------------
Status of DLL by TTTT
---------------------------------------------------------*/
void OnDLLAliveMessage(WPARAM tempwParam)
{
	GetSystemTimeAsFileTime(&lastFileTimeDLLAlive);
}


/*-------------------------------------------
   main routine
---------------------------------------------*/
static UINT WINAPI TclockExeMain(void)
{
	MSG msg;
	WNDCLASS wndclass;
	HWND hwnd;

	//CheckCommandLine(hwnd);



	//if (b_RestartDLL)
	//{
	//	hwnd = FindWindow(szClassName, szWindowText);		//プロセスがすでに起動していたらhwnd != NULLになる
	//	for (int i = 0; i < 10; i++)
	//	{
	//		if (hwnd != NULL) Sleep(500);
	//		hwnd = FindWindow(szClassName, szWindowText);		//プロセスがすでに起動していたらhwnd != NULLになる
	//	}
	//	if (hwnd != NULL)
	//	{
	//		MessageBox(NULL, "既存のTClock-Win10のプロセス終了に時間がかかっています。『OK』を押しても再起動しない場合にはタスクマネージャーからTClock-Win10のプロセスを強制終了してください。\n\nTerminating Previous TClock-Win10 is taking a long time. If you do not have the restarted TClock-Win10 even after clicking \"OK\", please kill the previous TClock-Win10 in the Taskmanager.",
	//			"TClock-Win10", MB_ICONEXCLAMATION | MB_SETFOREGROUND);
	//		hwnd = FindWindow(szClassName, szWindowText);
	//		if (hwnd != NULL) return 1;
	//	}
	//}




	// check wow64
	if (IsWow64()) {
		MessageBox(NULL, "本実行ファイルは32bit (x86)バイナリです。\n64bit環境ではx64バイナリを使用する必要があります。\n\nThis is 32bit (x86) binary.\nx64 binary is required for 64bit Windows.",
			"TClock-Win10", MB_ICONERROR | MB_SETFOREGROUND);
		return 1;
	}



	// for Vista 
	if (IsUserAdmin()) {		//管理者権限があれば
		AddMessageFilters();	//メッセージフィルタ(ウィンドウプロシージャの)を設定する(AddMessageFilters)
	}	



	// Call WINAPI CheckWinVersion_Win10() in tcdll.dll
	if (CheckWinVersion_Win10() < 0x0400) // = WIN10, 1024
	{
		MessageBox(NULL, "本アプリケーションはWindows10以降用です。\n\nThis application works on Windows 10(Anniversary Update) or later.",
			"TClock-Win10", MB_ICONERROR | MB_SETFOREGROUND);
		return 1;
	}




	// get the path where .exe is positioned
	GetModuleFileName(g_hInst, g_mydir, MAX_PATH);	//この時点ではフルパス付きのtclock実行ファイル名を取得
	char fname[MAX_PATH];
	strcpy(fname, g_mydir);		//exeのファイル名がついたままのg_mydirをfnameに入れて
	getExeVersion(fname);		//fileバージョンを取得してexeVersionM, exeVersionLを取得->DLLバージョンチェックに利用
	del_title(g_mydir);		//g_mydirはこれ以降、TClockのフォルダへのパスとして利用される



	//CheckRegistry();
	if (!CheckRegistry_Win10()) {		//名前にはRegistryとあるが、iniファイルを探し、なければ作成する関数
		MessageBox(NULL, "tclock-win10.iniが見当たらず、また作成に失敗しました。アプリケーションを終了します。\n\nCould not access / create tclock-win10.ini.",
			"TClock-Win10", MB_ICONERROR | MB_SETFOREGROUND);
		return 1;
	}




	// not to execute the program twice
	hwnd = FindWindow(szClassName, szWindowText);
	if(hwnd != NULL)				//すでにプロセスが起動していれば、	
	{
		CheckCommandLine(hwnd);		//コマンドラインオプションをチェック("/exit"の場合のため)して	

		if (b_Exit) return 1;
		else if (b_RestartNOW)
		{
			//SetMyRegLong("Status_DoNotEdit", "LastLaunchTimeStamp", 0);
			PostMessage(hwnd, WM_CLOSE, 0, 0);		//メインウィンドウにWM_CLOSE(102)を送出する。時々失敗するが、だいたいうまくいく。
			if (WaitQuitPrevTClock(50))	return 1;
		}
		else
		{
			int reply = MessageBox(NULL, "TClockのプロセスが稼働中です。再起動しますか？\n『OK』を選ぶと現在のプロセスを終了して新プロセスで再起動します。\n『キャンセル』を選ぶと現在のプロセスを維持します。\n\nPrevious TClock process is still running. Will you restart TClock?\nChoosing:\n\"OK\" initiates restarting from existing TClock Process.\n\"Cancel\" simply aborts this new process",
				"TClock-Win10", MB_ICONEXCLAMATION | MB_OKCANCEL | MB_DEFBUTTON1 | MB_SETFOREGROUND);
			if (reply == IDOK)
			{
				SetMyRegLong("Status_DoNotEdit", "LastLaunchTimeStamp", 0);
				PostMessage(hwnd, WM_CLOSE, 0, 0);		//メインウィンドウにWM_CLOSE(102)を送出する。時々失敗するが、だいたいうまくいく。			
				if (WaitQuitPrevTClock(50))	return 1;
				b_RestartNOW = TRUE;	//NormalLogへの連絡のために再利用。
			}
			else
			{
				return 1;					//終了する
			}
		}
	}






	b_DebugLog = GetMyRegLong(NULL, "DebugLog", FALSE);
	SetMyRegLong(NULL, "DebugLog", b_DebugLog);

	b_DebugLog_RegAccess = GetMyRegLong(NULL, "DebugLog_RegAccess", FALSE);

	b_DebugLog_Specific = GetMyRegLong(NULL, "DebugLog_Specific", FALSE);

	b_NormalLog = GetMyRegLong(NULL, "NormalLog", TRUE);
	SetMyRegLong(NULL, "NormalLog", b_NormalLog);





	zombieCheckInterval = GetMyRegLong("ETC", "ZombieCheckInterval", 10);
	if (zombieCheckInterval < 5) zombieCheckInterval = 5;
	if (zombieCheckInterval > 300) zombieCheckInterval = 300;	
	SetMyRegLong("ETC", "ZombieCheckInterval", zombieCheckInterval);

	if (b_DebugLog) WriteDebug_New2("[exemain.c][TclockExeMain] TclockMain started");

	if (b_NormalLog)
	{
		CheckNormalLog();
		if (b_RestartNOW) WriteNormalLog("TClock-Win10 restarted");
		else WriteNormalLog("TClock-Win10 newly started");
	}

	b_AutoRestart = GetMyRegLong(NULL, "AutoRestart", TRUE);
	SetMyRegLong(NULL, "AutoRestart", b_AutoRestart);

	//起動時に前回終了時の連続リスタート回数を取得する
	countRestart = GetMyRegLong("Status_DoNotEdit", "CountAutoRestart", 0);
	if (countRestart >= MAX_AUTORESTART) {
		MessageBox(NULL, "クラッシュループを検出しました。アプリケーションを終了します。\n\nTClock is terminated because of repeting crash.",
			"TClock-Win10", MB_ICONERROR | MB_SETFOREGROUND);
		SetMyRegLong("Status_DoNotEdit", "CountAutoRestart", 0);
		return 1;
	}

	b_EnglishMenu = GetMyRegLong(NULL, "EnglishMenu", FALSE);
	SetMyRegLong(NULL, "EnglishMenu", b_EnglishMenu);
	if (b_EnglishMenu)
	{
		Language_Offset = LANGUAGE_OFFSET_ENGLISH;
	}
	else
	{
		Language_Offset = 0;
	}




	InitializeMenuItems();

	if(!CheckTCDLL()) { return 1; }	//tclock.dllのバージョンチェック

	g_hInstResource = LoadLanguageDLL(g_langdllname);		//langja.dllのロードを試みる
	if(g_hInstResource == NULL) { return 1; }				//langja.dllがロードできなければ停止

	CheckUnplugDrive();


	InitCommonControls();

	// Message of the taskbar recreating
	// Special thanks to Mr.Inuya
	//https://isobe.exblog.jp/113279/
	s_uTaskbarRestart = RegisterWindowMessage("TaskbarCreated");

	g_hIconTClock = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));

	//g_hwndPropDlg = g_hDlgTimer = NULL;
	g_hwndPropDlg = NULL;

	// register a window class
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = g_hInst;
	wndclass.hIcon         = g_hIconTClock;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szClassName;
	RegisterClass(&wndclass);

	if (b_DebugLog) WriteDebug_New2("[exemain.c][TclockExeMain] Window Class Registered");


	// create a hidden window
	//DO_WS_AGGRESSIVE();	// Comment out by TTTT 20181125
	hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, szClassName, szWindowText,		//ここでxzClassName, szWindowText等を登録して、hwndを取得
		0, CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
		NULL, NULL, g_hInst, NULL);
	//ShowWindow(hwnd, SW_SHOW);	//見えないウィンドウが存在している。この行のコメントアウトを外すと見える。これが"hwnd"
	//UpdateWindow(hwnd);


	if (b_DebugLog) WriteDebug_New2("[exemain.c][TclockExeMain] Hidden Main Window Created");


	if(OleInitialize(NULL) != S_OK){	//STA（シングルスレッドアパートメント）スレッドとして初期化し、OLE用の追加処理を行う…らしい。
		MessageBox(NULL, "OLEの初期化に失敗しました。\n\nFailed to initialize OLE.", "TClock-Win10", MB_ICONERROR);
	}

	g_hwndMain = hwnd;	//メイン隠しウィンドウのハンドルをグローバル変数のg_hwndMainにコピー

	SetIdlePriority();	//デフォルトではIDLE_PRIORITY_CLASSとする	added by TTTT

	CheckCommandLine(hwnd);		//コマンドラインチェック。この時点ではタスクトレイの改造は行っていない。この中で開始ウェイトも設定されている(?)

	//b_RestartDLL = FALSE;		//どうせこのあと使わないが、気持ち悪いのでクリア
	
	HPOWERNOTIFY handle_PowerNotify;
	b_ModernStandbySupported = CheckModernStandbyCapability_Win10();
	if (b_ModernStandbySupported)
	{
		handle_PowerNotify = RegisterPowerSettingNotification(hwnd, &GUID_CONSOLE_DISPLAY_STATE, DEVICE_NOTIFY_WINDOW_HANDLE);
	}

	while(GetMessage(&msg, NULL, 0, 0))		//キューからメッセージを受け取るGetMessageの戻り値が0になる(＝WM_QUITがポストされる) まで、
											//取得したメッセージをウィンドウプロシージャに送りつづける
	{
		if(g_hwndPropDlg && IsWindow(g_hwndPropDlg)
			&& IsDialogMessage(g_hwndPropDlg, &msg))	//g_hwndPropDlgのメッセージは無視
			;
		//else if(g_hDlgTimer && IsWindow(g_hDlgTimer)
		//	&& IsDialogMessage(g_hDlgTimer, &msg))		//g_hDlgTimerのメッセージは無視
		//	;
		else		//それ以外は以下の2関数で処理
		{
			TranslateMessage(&msg);	//TranslateMessage: 仮想キーメッセージを文字メッセージへ変換(?)
			DispatchMessage(&msg);	// DispatchMessageで受け取ったメッセージをウィンドウプロシージャ(?)に送出.
		}
	}

	if (b_DebugLog) WriteDebug_New2("[exemain.c][TclockExeMain] Got out from the main message loop");

	if(g_hMenu) DestroyMenu(g_hMenu);				//右クリックメニュ表示中なら、メニューを消す

	if(g_hInstResource) FreeLibrary(g_hInstResource);		//langja.dllをアンロード

	UnregisterClass(szClassName, g_hInst);	// for TTBASE …と書いてあったが、たぶん必要 

	if (b_ModernStandbySupported) UnregisterPowerSettingNotification(handle_PowerNotify);

	return (UINT)msg.wParam;
}




/*-------------------------------------------
Set process priority idle as default
---------------------------------------------*/
void SetIdlePriority(void)
{
	HANDLE op = OpenProcess(PROCESS_ALL_ACCESS, TRUE, GetCurrentProcessId());
	SetPriorityClass(op, IDLE_PRIORITY_CLASS);
	Sleep(10);
}



/*-------------------------------------------
   Command Line Option
   /prop : Show TClock Properties
   /exit : Exit TClock
   //http://tclock2ch.no.land.to/help2ch/about.html
---------------------------------------------*/
void CheckCommandLine(HWND hwnd)
{

	char *p;
	p = GetCommandLine();
	while(*p)
	{
		if(*p == '/')
		{
			p++;
			if(_strnicmp(p, "prop", 4) == 0)	//propオプション：プロパティを開く
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with  prop option");
				PostMessage(hwnd, WM_COMMAND, IDC_SHOWPROP, 0);
				p += 4;
			}
			//else if (_strnicmp(p, "restartdll", 10) == 0)	//exitオプション：終了処理を行う
			//{
			//	//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with  restartdll option");
			//	b_RestartDLL = TRUE;
			//	p += 10;
			//}
			else if (_strnicmp(p, "restart", 7) == 0)	//exitオプション：終了処理を行う
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with  restart option");
				b_RestartNOW = TRUE;
				p += 10;
			}
			else if(_strnicmp(p, "exit", 4) == 0)	//exitオプション：終了処理を行う
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with  exit option");
				b_Exit = TRUE;
				PostMessage(hwnd, WM_CLOSE, 0, 0);		//メインウィンドウにWM_CLOSE(102)を送出する
				p += 4;
			}
			else if(_strnicmp(p, "nowait", 6) == 0)	//nowaitオプション：遅延スタートを無視
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with nowait option");
				KillTimer(hwnd, IDTIMER_START);				//現在動いているIDTIMER_STARTを停止
				SetTimer(hwnd, IDTIMER_START, 100, NULL);	//100msウェイトでIDTIMER_STARTを開始, タイムアウト時にはメッセージ送出
				p += 6;
			}
			else if(_strnicmp(p, "idle", 4) == 0)	//idleオプション：優先度をIDLEにして起動
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with  idle option");
				HANDLE op = OpenProcess(PROCESS_ALL_ACCESS, TRUE, GetCurrentProcessId());
				SetPriorityClass(op, IDLE_PRIORITY_CLASS);
				Sleep(10);
				p += 4;
			}
			else if (_strnicmp(p, "normal", 4) == 0)	//normalオプション：優先度をNORMALにして起動
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with  normal option");
				HANDLE op = OpenProcess(PROCESS_ALL_ACCESS, TRUE, GetCurrentProcessId());
				SetPriorityClass(op, NORMAL_PRIORITY_CLASS);
				Sleep(10);
				p += 6;
			}
		}
		p++;
	}

}


/*-------------------------------------------
   the window procedure	
---------------------------------------------*/
LRESULT CALLBACK WndProc(HWND hwnd,	UINT message, WPARAM wParam, LPARAM lParam)	//messageループの中のDispatchMessage()からのメッセージを受けてる？
{
	switch (message)
	{
		case WM_CREATE:
		{
			//if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_CREATE received");
			int nDelay;
			bDestroy = FALSE;
			InitFormat(); // initialize a Date/Time format
			nDelay = GetMyRegLong(NULL, "DelayStart", 0);		//設定ファイルから遅延スタート秒数を読み込み
			if(nDelay > 0)
			{
				SetTimer(hwnd, IDTIMER_START, nDelay * 1000, NULL);		//タイマーを起動(タイムアウト時にメインウィンドウあてWM_TIMER, IDTIMER_START)メッセージ
				bcontractTimer = TRUE;									//起動タイマー動作中フラグTRUE
			}
			else SendMessage(hwnd, WM_TIMER, IDTIMER_START, 0);			//タイマーの代わりにメインウィンドウに即刻IDTIMER_STARTメッセージ
			InitMouseFunction(hwnd);
			SetTimer(hwnd, IDTIMER_MAIN, 1000, NULL);		//メインループタイマー(IDTIMER_MAIN)周期を1秒に設定, 現時点では何の処理も行っていない。
			SetTimer(hwnd, IDTIMER_CREATE, 5000, NULL);		//クリエイトタイマー(IDTIMER_CREATE)周期を5秒に設定
			//DO_WS_AGGRESSIVE(); // Comment out by TTTT 20181125
			return 0;
		}
		case WM_TIMER:		//WM_TIMERに対する処理
			if(wParam == IDTIMER_START)		//起動用タイマーのタイムアウトの処理
			{
				if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_TIMER(IDTIMER_START) received");
				if(bcontractTimer) KillTimer(hwnd, wParam);		//タイマー停止
				bcontractTimer = FALSE;							//起動タイマー動作中フラグFALSE
				HookStart(hwnd);				// install a hook	dllmain.cの中にある。重要。タスクトレイのメッセージをフック。コア機能の起動

				SetTimer(hwnd, IDTIMER_ZOMBIECHECK, zombieCheckInterval * 1000, NULL);	//

				//nCountFindingClock = 0;			// 時計カウンタ(エラー検出用？)
				//DO_WS_AGGRESSIVE(); // Comment out by TTTT 20181125
			}
			else if (wParam == IDTIMER_MAIN)	//メインループタイマー(デフォルト1秒)のタイムアウトの処理
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_TIMER(IDTIMER_MAIN) received");
				OnTimerMain(hwnd);
			}
			else if (wParam == IDTIMER_MOUSE)
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_TIMER(IDTIMER_MOUSE) received");
				OnTimerMouse(hwnd);
			}
			else if(wParam == IDTIMER_CREATE)	//クリエイトタイマー(デフォルト5秒)のタイムアウトの処理
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_TIMER(IDTIMER_CREATE) received");
				SetTimer(hwnd, IDTIMER_CREATE, 60000, NULL);	//以降はタイムアウトを60秒に場合
				//DO_WS_AGGRESSIVE(); // Comment out by TTTT 20181125
			}
			else if (wParam == IDTIMER_ZOMBIECHECK)	//ゾンビチェックのタイムアウトの処理 by TTTT 20181125
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_TIMER(IDTIMER_ZOMBIECHECK) received");
//				OnTimerZombieCheck(hwnd);
				OnTimerZombieCheck2(hwnd);
			}
			return 0;
		case WM_CLOSE:		//手動での修了処理はこちらから行う。
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_CLOSE received");
			TerminateTClock(hwnd);
			return 0;
		case WM_DESTROY:	//終了時処理。実際にはすべて終わってから届くようになっている。
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_DESTROY received");
			return 0;
		case WM_ENDSESSION:	//セッション終了時のTClock終了はこちらから
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_ENDSESSION received");
			if(wParam) TerminateTClock(hwnd);
			break;
		case WM_QUERYENDSESSION:	//セッション終了時の事前確認のようなもの。修了処理はWM_ENDSESSIONに実装する。
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_QUERYENDSESSION received");
			if (b_NormalLog) WriteNormalLog("Exit TClock-Win10 by EndSession.");
			break;		//DefWindowProcが1を返してくれるので任せる。
		case WM_PAINT:
		{
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_PAINT received");
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
			return 0;
		}

		// Messages sent/posted from tclock.dll
		case WM_USER:
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_USER received");	//DLL Window生成時にハンドルを通知
			//nCountFindingClock = -1;
			g_hwndClock = (HWND)lParam;
			return 0;
		case (WM_USER+1):   // error
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_USER+1 received");	//DLL Window生成時のエラー
			//nCountFindingClock = -1;
			InitError((int)lParam);
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		case (WM_USER+2):   // exit (from tclock.c EndClock())	このコードはVer4.0.3以降では修了処理として呼ばれなくなっている(はず)。適当な時期に削除すること。
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_USER+2 received");

			TerminateTClockFromDLL(hwnd);	//tcdlll(tclock.cから呼ばれるときはFromDLLを実行する)
			////タイマ機能のなごり
			////if(g_hDlgTimer && IsWindow(g_hDlgTimer))
			////	PostMessage(g_hDlgTimer, WM_CLOSE, 0, 0);
			////g_hDlgTimer = NULL;
			return 0;
		case CLOCKM_DLLALIVE:
			OnDLLAliveMessage(wParam);	//wParamに情報を入れる想定(未使用)
			return 0;
		case WM_WININICHANGE:		//画面テーマが変わった時の対応
		{
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_WININICHANGE received");
			PostMessage(g_hwndClock, WM_COMMAND, (WPARAM)CLOCKM_BGCOLOR_UPDATE, 0);
			return 0;
		}
		case WM_SYSCOLORCHANGE:		//SYSCOLORが変わった場合は、アイコン名背景透明化のタイマー(IDTIMER_DESKTOPICON)を作動させるだけ。不要か。
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_SYSCOLORCHANGE received");
			PostMessage(hwnd, WM_USER+10, 1,0);
			return 0;
		case (WM_USER+10):		//WM_USER+10はDESKCAL関連のようなので、削除してもよいか。そもそもフィルタされて届かないかも。
		{
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_USER+10 received");
			return 0;
		}

		// return from power saving
		case WM_POWERBROADCAST:		//これもカレンダー関係のみのコード。不要か。
		{
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_POWERBROADCAST received");
			if (wParam == PBT_APMPOWERSTATUSCHANGE)
			{
				if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] wParam: PBT_APMPOWERSTATUSCHANGE");
			}
			else if (wParam == PBT_APMRESUMEAUTOMATIC)
			{
				if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] wParam: PBT_APMRESUMEAUTOMATIC");
			}
			else if (wParam == PBT_APMRESUMESUSPEND)
			{
				if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] wParam: PBT_APMRESUMESUSPEND");
			}
			else if (wParam == PBT_APMSUSPEND)
			{
				if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] wParam: PBT_APMSUSPEND");
			}
			else if (wParam == PBT_POWERSETTINGCHANGE)
			{
				if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] wParam: PBT_POWERSETTINGCHANGE");
				//if (*((POWERBROADCAST_SETTING*)lParam)->PowerSetting == GUID_CONSOLE_DISPLAY_STATE)
				//{

				//}

				if (*((POWERBROADCAST_SETTING*)lParam)->Data == 0x0)
				{
					if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] lParam->Data == 0 : Sleep in");
					PostMessage(g_hwndClock, CLOCKM_SLEEP_IN, 0, 0);
				}
				else if (*((POWERBROADCAST_SETTING*)lParam)->Data == 0x1)
				{
					if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] lParam->Data == 1 : Awake from Sleep");
					PostMessage(g_hwndClock, CLOCKM_SLEEP_AWAKE, 0, 0);
				}
			}
			break;
		}

		// context menu
		case WM_COMMAND:	//右クリックメニューからコマンド実行された場合はここを通ってmenu.cのコードに届けられる模様
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_COMMAND received");
			OnTClockCommand(hwnd, LOWORD(wParam), HIWORD(wParam)); // menu.c
			//DO_WS_AGGRESSIVE(); // Comment out by TTTT 20181125
			return 0;
		// messages transfered from the dll
		case WM_CONTEXTMENU:	//右クリックされた場合->メニューを開く関数(OnContextMenu())をコール
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_CONTEXTMENU received");
			// menu.c
			OnContextMenu(hwnd, (HWND)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			//DO_WS_AGGRESSIVE(); // Comment out by TTTT 20181125
			return 0;
		case WM_DROPFILES:	//ファイルがドロップされた場合の処理、要確認
			OnDropFiles(hwnd, (HDROP)wParam); // mouse.c
			//DO_WS_AGGRESSIVE(); // Comment out by TTTT 20181125
			return 0;
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
			//if (FindVistaCalenderWindow())
			//{
			//	DWORD_PTR dw = 0;
			//	SendMessageTimeout(g_hwndClock, CLOCKM_VISTACALENDAR, 1, 0, SMTO_BLOCK | SMTO_ABORTIFHUNG, 5000, &dw);
			//}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
		case WM_MOUSEWHEEL:
			OnMouseMsg(hwnd, message, wParam, lParam); // mouse.c
			return 0;
		case WM_ENTERMENULOOP:	//右クリックメニューの入力待ちループに入った時に出る。
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_ENTERMENULOOP received");
			bMenuOpened = TRUE;
			break;
		case WM_EXITMENULOOP:	//右クリックメニューの入力待ちループから出た時に出る。
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_EXITMENULOOP received");
			bMenuOpened = FALSE;
			break;
		case WM_HOTKEY:	//Hotkey機能。要否要検討
			OnHotkey(hwnd, (int)wParam);
			break;
		case WM_MEASUREITEM:	//ファイルリスト関係
			OnMeasureItem(hwnd, wParam, lParam); // filelist.c
			break;
		case WM_DRAWITEM:	//ファイルリスト関係
			OnDrawItem(hwnd, wParam, lParam); // filelist.c
			break;
		case WM_INITMENUPOPUP:	//ファイルリスト関係
			OnInitMenuPopup(hwnd, wParam, lParam); // filelist.c
			break;
		case WM_MENURBUTTONUP:	//ファイルリスト関係
			OnMenuRButtonUp(hwnd, wParam, lParam); //filelist.c
			break;
		case WM_DISPLAYCHANGE:	//added by TTTT
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_DISPLAYCHANGE received");
			PostMessage(g_hwndClock, WM_COMMAND, (WPARAM)CLOCKM_DISPLAYSTATUS_CHECK, 0);
			b_DisplayChanged = TRUE;
			break;
	}

	if(message == s_uTaskbarRestart) // When Explorer is hung up,
	{								 // and the taskbar is recreated.
		if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] message: s_uTaskbarRestart received");

		if (b_NormalLog)
		{
			WriteNormalLog("[Warning] Windows Taskbar restarted. (notified from OS)");
		}

		if (b_DisplayChanged)
		{
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] No action is taken because b_DisplayChanged = TRUE.");
			b_DisplayChanged = FALSE;
		}
		else if (b_AutoRestart)
		{
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] Windows Taskbar restarted. b_AutoRestart = TRUE, Restart TClock-WIn10.");
			if (b_NormalLog) WriteNormalLog("b_AutoRestart = TRUE, Restart TClock-WIn10");

			char fname[MAX_PATH];
			strcpy(fname, g_mydir);
			add_title(fname, "TClock-Win10.exe");
			ShellExecute(NULL, "open", fname, "/restart", NULL, SW_HIDE);

		}
		else
		{
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] Windows Taskbar restarted. b_AutoRestart = FALSE. Quit TClock.");
			if (b_NormalLog) WriteNormalLog("b_AutoRestart = FALSE, Quit TClock-Win10");

			PostMessage(hwnd, WM_CLOSE, 0, 0);		//メインウィンドウにWM_CLOSE(102)を送出する。(この終了動作はdll先行ではなくてexemainからなので)

		}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

void TerminateTClock(HWND hwnd)
{
	//TClockが動作中から終了する場合には、こちらで終了する。

	if (b_DebugLog) WriteDebug_New2("[exemain.c] TerminateTClock called.");

	if (g_hwndClock)
	{
		SendMessage(g_hwndClock, WM_COMMAND, IDC_EXIT, 0);
		g_hwndClock = NULL;		//EndClockから呼ばれたこと(EndClock実行済みの確認のために消す)
	}
	if (g_hwndPropDlg && IsWindow(g_hwndPropDlg))
		PostMessage(g_hwndPropDlg, WM_CLOSE, 0, 0);
	g_hwndPropDlg = NULL;

	HookEnd();  // uninstall a hook, Ver 4.0.5.3以降ではすでにフック外れているはずだが念のため。

	if (bDestroy == TRUE) return 0;	//２重終了しないように

	bDestroy = TRUE;
	EndMouseFunction(hwnd);
	KillTimer(hwnd, IDTIMER_MAIN);
	KillTimer(hwnd, IDTIMER_CREATE);
	KillTimer(hwnd, IDTIMER_ZOMBIECHECK);

	if (bcontractTimer)
	{
		KillTimer(hwnd, IDTIMER_START);
		bcontractTimer = FALSE;
	}

	PostQuitMessage(0);
	g_hwndMain = NULL;

	PostMessage(hwnd, WM_DESTROY, 0, 0);
	if (b_DebugLog) WriteDebug_New2("[exemain.c] TerminateTClock completed.");

}


void TerminateTClockFromDLL(HWND hwnd)
{
	if (b_DebugLog) WriteDebug_New2("[exemain.c] TerminateTClockFromDLL called.");

	//FromDLLは、時計のほうで終了処理(EndClock)してから呼ばれる(WM_USER+2経由)状況で使う。

	if (g_hwndClock)
	{
//		SendMessage(g_hwndClock, WM_COMMAND, IDC_EXIT, 0);
		g_hwndClock = NULL;		//EndClockから呼ばれたこと(EndClock実行済みの確認のために消す)
	}
	if (g_hwndPropDlg && IsWindow(g_hwndPropDlg))
		PostMessage(g_hwndPropDlg, WM_CLOSE, 0, 0);
	g_hwndPropDlg = NULL;

	HookEnd();  // uninstall a hook, Ver 4.0.5.3以降ではすでにフック外れているはずだが念のため。

	if (bDestroy == TRUE) return 0;	//２重終了しないように

	bDestroy = TRUE;
	EndMouseFunction(hwnd);
	KillTimer(hwnd, IDTIMER_MAIN);
	KillTimer(hwnd, IDTIMER_CREATE);
	KillTimer(hwnd, IDTIMER_ZOMBIECHECK);

	if (bcontractTimer)
	{
		KillTimer(hwnd, IDTIMER_START);
		bcontractTimer = FALSE;
	}

	PostQuitMessage(0);
	g_hwndMain = NULL;

	PostMessage(hwnd, WM_DESTROY, 0, 0);
	if (b_DebugLog) WriteDebug_New2("[exemain.c] TerminateTClockFromDLL completed.");

}



/*-------------------------------------------------------
  show a message when TClock failed to customize the clock
---------------------------------------------------------*/
void InitError(int n)
{
	char s[160];

	wsprintf(s, "%s: %d", MyString(IDS_NOTFOUNDCLOCK), n);
	MyMessageBox(NULL, s, NULL, MB_OK, MB_ICONEXCLAMATION);
}

/*-------------------------------------------------------
   Timer
   synchronize, alarm, timer, execute Desktop Calendar...
---------------------------------------------------------*/
void OnTimerMain(HWND hwnd)		//メインループタイマー(デフォルト1秒)のタイムアウトの処理
{
	//時計ウィンドウのタイマー動作が停止して2秒したらフラグが立つ。
	//負荷が小さいので残してあるが、現在のところ停止動作等は実装されていない。
	//Ver4.0.4時点で不測の自体に対する対処はOnTimerZombieCheck2で行っている。
	//スリープ等でフラグが経つのですぐに終了してはいけない。

	LONG currentFileTime = 0;

	GetSystemTimeAsFileTime(&currentFileTime);

	if (((LONGLONG)currentFileTime - (LONGLONG)lastFileTimeDLLAlive) > 20000000)
	{
		b_FlagDLLAlive = FALSE;	//Aliveメッセージが2秒開くとDLL音信不通フラグを立てる(復活可能)
	}
	else 
	{
		b_FlagDLLAlive = TRUE;
	}
}





void OnTimerZombieCheck2(HWND hwnd)
{
	if (g_hwndClock)
	{
		int ret = 0;
		ret = (int)SendMessage(g_hwndClock, WM_COMMAND, (WPARAM)CLOCKM_ZOMBIECHECK_CALL, 0);
		if (ret == 255)
		{
			if (b_DebugLog) WriteDebug_New2("[exemain.c][OnTimerZombieCheck2] TClock is alive.");
		}
		else
		{
			if (b_DebugLog) WriteDebug_New2("[exemain.c][OnTimerZombieCheck2] No responce from DLL. TClock may be unexpectedly dead. Quit TClock, regardless b_AutoRestart.");
			if (b_NormalLog) WriteNormalLog("No responce from DLL. TClock may be unexpectedly dead. Quit TClock, regardless b_AutoRestart.");
			TerminateTClockFromDLL(hwnd);		//すでにTClockの改造部は終了/消失していると判断されるため、FromDLLでの終了動作を行う。

			//if (b_AutoRestart)
			//{
			//	if (b_DebugLog) WriteDebug_New2("[exemain.c][OnTimerZombieCheck2] TClock is dead. b_AutoRestart = TRUE, Restart TClock.");
			//	if (b_NormalLog) WriteNormalLog("TClock is unexpectedly dead. b_AutoRestart = TRUE, Restart TClock-Win10");
			//	char fname[MAX_PATH];
			//	strcpy(fname, g_mydir);
			//	add_title(fname, "TClock-Win10.exe");
			//	ShellExecute(NULL, "open", fname, "/restart", NULL, SW_HIDE);
			//}
			//else
			//{
			//	if (b_DebugLog) WriteDebug_New2("[exemain.c][OnTimerZombieCheck2] TClock is dead. b_AutoRestart = FALSE. Quit TClock.");
			//	if (b_NormalLog) WriteNormalLog("TClock is unexpectedly dead. b_AutoRestart = FALSE, Quit TClock-Win10");
			//	TerminateTClockFromDLL(hwnd);		//すでにTClockの改造部は終了/消失していると判断されるため、FromDLLでの終了動作を行う。
			//}
		}
	}
}



//void OnTimerZombieCheck(HWND hwnd)	//ゾンビチェック by TTTT, Ver 4.0.4以降は使われていない。
//{
//	HWND hwndTaskBar;
//
//	b_DisplayChanged = FALSE;	 // Clear DisplayChange Flag
//
//	if (b_DebugLog) WriteDebug_New2("[exemain.c] OnTimerZombieCheck called.");
//
//	// find the taskbar
//	hwndTaskBar = FindWindow("Shell_TrayWnd", "");
//	if (hwndTaskBar)
//	{
//		if (hwndTaskBar_Prev == NULL)
//		{
//			hwndTaskBar_Prev = hwndTaskBar;
//		}
//		else if (hwndTaskBar != hwndTaskBar_Prev)
//		{
//			if (b_NormalLog)
//			{
//				WriteNormalLog("[Warning] Windows Taskbar restarted (detected by TClockMain). Cannot identify whether TClock-Win10 caused it or not.");
//			}
//
//
//			if (b_AutoRestart)
//			{
//				if (b_DebugLog) WriteDebug_New2("[OnTimerZombieCheck] Restart TClock with /restart option.");
//
//				if (b_NormalLog) WriteNormalLog("b_AutoRestart = TRUE, Restart TClock-Win10");
//
//				char fname[MAX_PATH];
//				strcpy(fname, g_mydir);
//				add_title(fname, "TClock-Win10.exe");
//				ShellExecute(NULL, "open", fname, "/restart", NULL, SW_HIDE);
//			}
//			else
//			{
//				if (b_DebugLog) WriteDebug_New2("[OnTimerZombieCheck] b_AutoRestart = FALSE. Quit TClock.");
//
//				if (b_NormalLog) WriteNormalLog("b_AutoRestart = FALSE, Quit TClock-Win10");
//
//
//				PostMessage(hwnd, WM_CLOSE, 0, 0);		//メインウィンドウにWM_CLOSE(102)を送出する。
//			}
//		}
//	}
//
//}


/*-------------------------------------------
  load a language dll
---------------------------------------------*/
HINSTANCE LoadLanguageDLL(char *langdllname)
{
	if (b_DebugLog) WriteDebug_New2("[exemain.c][LoadLanguageDLL] LoadLanguageDLL() called");
	HINSTANCE hInst = NULL;
	char fname[MAX_PATH];
	WIN32_FIND_DATA fd;
	HANDLE hfind = INVALID_HANDLE_VALUE;

	if(hfind == INVALID_HANDLE_VALUE)
	{
		GetModuleFileName(g_hInst, fname, MAX_PATH);
		del_title(fname);

		add_title(fname, "tclang-win10.dll");
		hfind = FindFirstFile(fname, &fd);
		if(hfind != INVALID_HANDLE_VALUE)
		{
			FindClose(hfind);
			del_title(fname); add_title(fname, fd.cFileName);
		}
	}

	if(hfind != INVALID_HANDLE_VALUE)
	{
		if(!CheckDLL(fname)) return NULL;
		hInst = LoadLibrary(fname);
	}

	if(hInst == NULL)
		MyMessageBox(NULL, "Can't load a language module.",
			NULL, MB_OK, MB_ICONEXCLAMATION);
	else strcpy(langdllname, fname);
	return hInst;
}

HINSTANCE GetLangModule(void)
{
	return g_hInstResource;
}

/*-------------------------------------------
  Check version of dll
---------------------------------------------*/
BOOL CheckTCDLL(void)
{
	if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckTCDLL] CheckTCDLL() called");
	char fname[MAX_PATH];
	strcpy(fname, g_mydir);
	add_title(fname, "tcdll-win10.dll");
	return CheckDLL(fname);
}

/*-------------------------------------------
  Check version of dll
---------------------------------------------*/
BOOL CheckDLL(char *fname)
{
	if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckDLL] CheckDLL() called");
	DWORD size;
	char *pBlock;
	VS_FIXEDFILEINFO *pffi;
	BOOL br = FALSE;

	size = GetFileVersionInfoSize(fname, 0);
	if(size > 0)
	{
		pBlock = malloc(size);
		if(GetFileVersionInfo(fname, 0, size, pBlock))
		{
			UINT tmp;
			if(VerQueryValue(pBlock, "\\", &pffi, &tmp))
			{
				if(pffi->dwFileVersionMS == exeVersionM &&
					HIWORD(pffi->dwFileVersionLS) == HIWORD(exeVersionL))
				{
					br = TRUE;
				}
			}
		}
		free(pBlock);
	}
	if(!br)
	{
		char msg[MAX_PATH+30];

		strcpy(msg, "Invalid file version: ");
		get_title(msg + strlen(msg), fname);
		MyMessageBox(NULL, msg,
			NULL, MB_OK, MB_ICONEXCLAMATION);
	}
	return br;
}


void My2chHelp(HWND hwnd)
{
	char helpurl[1024];
	char s[20];

	GetMyRegStr("ETC", "2chHelpURL", helpurl, 1024, "");
	if (helpurl[0] == 0)
	{
		strcpy(helpurl, MyString(IDS_HELP2CH));
		SetMyRegStr("ETC", "2chHelpURL", helpurl);
	}

	ShellExecute(hwnd, NULL, helpurl, NULL, "", SW_SHOW);
}






/*------------------------------------------------
Create Default Setting File		//Added by TTTT
--------------------------------------------------*/
void CreateDefaultIniFile_Win10(char *fname)
{
	HANDLE hCreate;
	char fname2[MAX_PATH];


	hCreate = CreateFile(fname, GENERIC_WRITE, 0, NULL,
		CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hCreate != INVALID_HANDLE_VALUE) {

		CloseHandle(hCreate);

		g_bIniSetting = TRUE;
		strcpy(g_inifile, fname);

		SetMyRegLong(NULL, "DebugLog", 0);
		SetMyRegLong(NULL, "NormalLog", 1);
		SetMyRegLong(NULL, "AutoClearLogFile", 1);
		SetMyRegLong(NULL, "AutoClearLogLines", 1000);
		//////SetMyRegLong(NULL, "WarnDelayedUsageRetrieval", TRUE);
		SetMyRegLong(NULL, "AutoRestart", 1);
		SetMyRegLong(NULL, "CompactMode", 0);

		SetMyRegLong(NULL, "EnglishMenu", 0);
		SetMyRegLong(NULL, "AdjustThreshold", 200);
		SetMyRegLong(NULL, "EnableOnSubDisplay", 1);
		SetMyRegLong(NULL, "OffsetClockMS", 0);

		SetMyRegStr("Status_DoNotEdit", "Version", "");
		SetMyRegLong("Status_DoNotEdit", "SafeMode", 0);
		SetMyRegLong("Status_DoNotEdit", "LastLaunchTimeStamp", 0);
		SetMyRegLong("Status_DoNotEdit", "ExcessNetProfiles", FALSE);
		SetMyRegLong("Status_DoNotEdit", "ExistLTEProfile", FALSE);
		SetMyRegLong("Status_DoNotEdit", "ExistMeteredProfile", FALSE);
		SetMyRegLong("Status_DoNotEdit", "PreviousLTEProfNumber", 0);
		SetMyRegStr("Status_DoNotEdit", "PreviousLTEProfName", "");
		SetMyRegLong("Status_DoNotEdit", "BatteryLifeAvailable", 1);
		SetMyRegLong("Status_DoNotEdit", "CurrentInternetProfileNumber", 0);
		SetMyRegLong("Status_DoNotEdit", "NumberOfProfiles", 0);
		SetMyRegLong("Status_DoNotEdit", "TimerCountForSec", 1000);
		SetMyRegLong("Status_DoNotEdit", "ModernStandbySupported", 0);

		SetMyRegLong("Win11", "UseTClockNotify", 1);
		SetMyRegLong("Win11", "AdjustCutTray", 0);
		SetMyRegLong("Win11", "AdjustWin11ClockWidth", 0);
		SetMyRegLong("Win11", "AdjustDetectNotify", 0);
		SetMyRegLong("Win11", "AdjustWin11IconPosition", 1);
//		SetMyRegLong("Win11", "ColorNotificationIcon", (long)RGB(255, 255, 255));
		SetMyRegLong("Win11", "ShowWin11NotifyNumber", 1);
		
		SetMyRegLong("Color_Font", "UseBackColor", 0);
		SetMyRegLong("Color_Font", "BackColor", 2147483633);
		SetMyRegLong("Color_Font", "UseBackColor2", 0);
		SetMyRegLong("Color_Font", "BackColor2", 2147483633);
		SetMyRegLong("Color_Font", "GradDir", 0);
		SetMyRegLong("Color_Font", "ForeColor", 16777215);
		SetMyRegLong("Color_Font", "ForeColorShadow", 0);
		SetMyRegLong("Color_Font", "ForeColorBORDER", 0);
		SetMyRegLong("Color_Font", "ShadowColor", 0);
		SetMyRegLong("Color_Font", "ClockShadowRange", 1);
		SetMyRegStr("Color_Font", "Font", "lucida console");
		SetMyRegLong("Color_Font", "FontSize", 10);
		SetMyRegLong("Color_Font", "TextPos", 2);
		SetMyRegLong("Color_Font", "Bold", 0);
		SetMyRegLong("Color_Font", "Italic", 0);
		//SetMyRegLong("Color_Font", "ClockHeight", 0);
		SetMyRegLong("Color_Font", "ClockWidth", 0);
		SetMyRegLong("Color_Font", "VertPos", 0);
		SetMyRegLong("Color_Font", "LineHeight", 0);

		SetMyRegLong("Color_Font", "UseAllColor", 0);
		SetMyRegLong("Color_Font", "UseDateColor", 0);
		SetMyRegLong("Color_Font", "UseDowColor", 0);
		SetMyRegLong("Color_Font", "UseTimeColor", 0);
		SetMyRegLong("Color_Font", "UseVPNColor", 0);
		SetMyRegLong("Color_Font", "Saturday_TextColor", 0x00C8FFC8);
		SetMyRegLong("Color_Font", "Sunday_TextColor", 0x00C8C8FF);
		SetMyRegLong("Color_Font", "Holiday_TextColor", 0x00C8C8FF);
		SetMyRegLong("Color_Font", "VPN_TextColor", 0x00FFFF00);

		SetMyRegLong("Format", "Locale", 1033);
		SetMyRegLong("Format", "Year4", 0);
		SetMyRegLong("Format", "Year", 1);
		SetMyRegLong("Format", "Month", 1);
		SetMyRegLong("Format", "MonthS", 0);
		SetMyRegLong("Format", "Day", 1);
		SetMyRegLong("Format", "Weekday", 1);
		SetMyRegLong("Format", "Hour", 1);
		SetMyRegLong("Format", "Minute", 1);
		SetMyRegLong("Format", "Second", 1);
		SetMyRegLong("Format", "Kaigyo", 0);
		SetMyRegLong("Format", "InternetTime", 0);
		SetMyRegLong("Format", "AMPM", 0);
		SetMyRegLong("Format", "Hour12", 0);
		SetMyRegLong("Format", "HourZero", 0);
		SetMyRegStr("Format", "AMsymbol", "AM");
		SetMyRegStr("Format", "PMsymbol", "PM");
		SetMyRegLong("Format", "Custom", 1);

		SetMyRegStr("Format", "Format", "[ICP] WiFi LTE \"U\"NSSK___x\"KB/s\" Vol \"CPU\"CU__x\"%\" mm/dd ddd\\nVPNS NMX1 \"D\"NRSK___x\"KB/s\" VL__xVMCC___x\"MHz\" hh:nn:ss\" \"");
		SetMyRegStr("Format", "CustomFormat", "[ICP] WiFi LTE \"U\"NSSK___x\"KB/s\" Vol \"CPU\"CU__x\"%\" mm/dd ddd\\nVPNS NMX1 \"D\"NRSK___x\"KB/s\" VL__xVMCC___x\"MHz\" hh:nn:ss\" \"");


		SetMyRegLong("Mouse", "DropFiles", 0);
		SetMyRegStr("Mouse", "DropFilesApp", "");
		SetMyRegLong("Mouse", "RightClickMenu", 1);

		SetMyRegLong("Mouse", "01", MOUSEFUNC_VISTACALENDAR);
		//SetMyRegLong("Mouse", "02", MOUSEFUNC_TASKMGR);
		//SetMyRegLong("Mouse", "03", 0);
		//SetMyRegLong("Mouse", "04", 0);


		SetMyRegLong("Graph", "BackNetColSend", 255);
		SetMyRegLong("Graph", "BackNetColSR", 16769279);
		SetMyRegLong("Graph", "BackNetColRecv", 65280);
		SetMyRegLong("Graph", "BackNet", 1);
		SetMyRegLong("Graph", "LogGraph", 1);

		SetMyRegLong("Graph", "GraphTate", 0);
		SetMyRegLong("Graph", "ReverseGraph", 0);

		SetMyRegLong("Graph", "CpuHigh", 70);
		SetMyRegLong("Graph", "NetGraphScaleRecv", 1000);
		SetMyRegLong("Graph", "NetGraphScaleSend", 1000);
		SetMyRegLong("Graph", "GraphMode", 1);
		SetMyRegLong("Graph", "GraphType", 2);
		SetMyRegLong("Graph", "GraphLeft", 0);
		SetMyRegLong("Graph", "GraphTop", 0);
		SetMyRegLong("Graph", "GraphRight", 230);
		SetMyRegLong("Graph", "GraphBottom", 1);




		SetMyRegLong("AnalogClock", "UseAnalogClock", 0);
		strcpy(fname2, "tclock.bmp");
		SetMyRegStr("AnalogClock", "AnalogClockBmp", fname2);
		SetMyRegLong("AnalogClock", "AClockHourHandColor", 255);
		SetMyRegLong("AnalogClock", "AClockMinHandColor", 16711680);
		SetMyRegLong("AnalogClock", "AnalogClockHourHandBold", 0);
		SetMyRegLong("AnalogClock", "AnalogClockMinHandBold", 0);
		SetMyRegLong("AnalogClock", "AnalogClockPos", 0);
		SetMyRegLong("AnalogClock", "AnalogClockAtStartBtn", 0);
		SetMyRegLong("AnalogClock", "AnalogClockHPos", 10);
		SetMyRegLong("AnalogClock", "AnalogClockVPos", 2);
		SetMyRegLong("AnalogClock", "AnalogClockSize", 25);


		strcpy(fname2, "file:");
		strcat(fname2, "tclock_tooltip.txt");
		SetMyRegLong("Tooltip", "EnableTooltip", 1);
		SetMyRegStr("Tooltip", "Tooltip", fname2);
		SetMyRegStr("Tooltip", "Tooltip2", "TClock <%LDATE%>");
		SetMyRegStr("Tooltip", "Tooltip3", "TClock <%LDATE%>");
		SetMyRegLong("Tooltip", "Tip2Use", 0);
		SetMyRegLong("Tooltip", "Tip3Use", 0);
		SetMyRegLong("Tooltip", "TipTateFlg", 0);
		SetMyRegLong("Tooltip", "Tip1Update", 1);
		SetMyRegLong("Tooltip", "Tip2Update", 0);
		SetMyRegLong("Tooltip", "Tip3Update", 0);
		SetMyRegLong("Tooltip", "Tip2Use", 0);
		SetMyRegLong("Tooltip", "Tip2Use", 0);
		SetMyRegStr("ToolTip", "TipFont", "Lucida Console");
		SetMyRegStr("Tooltip", "TipTitle", "TClock-Win10 <%VerTC%>");
		SetMyRegLong("Tooltip", "TipFontSize", 9);
		SetMyRegLong("Tooltip", "TipBold", 0);
		SetMyRegLong("Tooltip", "TipItalic", 0);
		SetMyRegLong("Tooltip", "BalloonFlg", 1);
		SetMyRegLong("Tooltip", "TipDispTime", 1);
		SetMyRegLong("Tooltip", "TipFontColor", (long)RGB(0, 0, 0));
		SetMyRegLong("Tooltip", "TipTitleColor", (long)RGB(0, 0, 255));
		SetMyRegLong("Tooltip", "TipBakColor", (long)RGB(255, 255, 255));
		//SetMyRegLong("Tooltip", "TipIcon", 0);
		//SetMyRegLong("Tooltip", "AlphaTip", 0);
		//SetMyRegLong("Tooltip", "TipDisableCustomDraw", 0);
		//SetMyRegLong("Tooltip", "TipEnableDoubleBuffering", 0);



		SetMyRegLong("BarMeter", "UseBarMeterVL", 0);
		SetMyRegLong("BarMeter", "BarMeterVL_Horizontal", 0);
		SetMyRegLong("BarMeter", "ColorBarMeterVL", (long)RGB(0, 255, 0));
		SetMyRegLong("BarMeter", "ColorBarMeterVL_Mute", (long)RGB(255, 0, 0));
		SetMyRegLong("BarMeter", "BarMeterVL_Right", 150);
		SetMyRegLong("BarMeter", "BarMeterVL_Width", 5);
		SetMyRegLong("BarMeter", "BarMeterVL_Bottom", 0);
		SetMyRegLong("BarMeter", "BarMeterVL_Height", -1);

		SetMyRegLong("BarMeter", "UseBarMeterBL",0);
		SetMyRegLong("BarMeter", "BarMeterBL_Horizontal", 0);
		SetMyRegLong("BarMeter", "ColorBarMeterBL_Charge", (long)RGB(255, 165, 0));
		SetMyRegLong("BarMeter", "ColorBarMeterBL_High", (long)RGB(0, 255, 0));
		SetMyRegLong("BarMeter", "ColorBarMeterBL_Mid", (long)RGB(255, 255, 0));
		SetMyRegLong("BarMeter", "ColorBarMeterBL_Low", (long)RGB(255, 0, 0));
		SetMyRegLong("BarMeter", "BarMeterBL_Right", 130);
		SetMyRegLong("BarMeter", "BarMeterBL_Width", 5);
		SetMyRegLong("BarMeter", "BarMeterBL_Bottom", 0);
		SetMyRegLong("BarMeter", "BarMeterBL_Height", -1);

		SetMyRegLong("BarMeter", "UseBarMeterCU", 0);
		SetMyRegLong("BarMeter", "BarMeterCU_Horizontal", 0);
		SetMyRegLong("BarMeter", "ColorBarMeterCU_High", (long)RGB(255, 0, 0));
		SetMyRegLong("BarMeter", "ColorBarMeterCU_Mid", (long)RGB(255, 255, 0));
		SetMyRegLong("BarMeter", "ColorBarMeterCU_Low", (long)RGB(0, 255, 0));
		SetMyRegLong("BarMeter", "BarMeterCU_Right", 110);
		SetMyRegLong("BarMeter", "BarMeterCU_Width", 5);
		SetMyRegLong("BarMeter", "BarMeterCU_Bottom", 0);
		SetMyRegLong("BarMeter", "BarMeterCU_Height", -1);

		SetMyRegLong("BarMeter", "UseBarMeterCore", 0);
		SetMyRegLong("BarMeter", "NumberBarMeterCore", 8);
		SetMyRegLong("BarMeter", "ColorBarMeterCore_High", (long)RGB(255, 0, 0));
		SetMyRegLong("BarMeter", "ColorBarMeterCore_Mid", (long)RGB(255, 255, 0));
		SetMyRegLong("BarMeter", "ColorBarMeterCore_Low", (long)RGB(0, 255, 0));
		SetMyRegLong("BarMeter", "BarMeterCore_Left", 0);
		SetMyRegLong("BarMeter", "BarMeterCore_Width", 0);
		SetMyRegLong("BarMeter", "BarMeterCore_Spacing", 2);

		SetMyRegLong("BarMeter", "UseBarMeterNet", 0);
		SetMyRegLong("BarMeter", "BarMeterNet_LogGraph", 0);
		//SetMyRegLong("BarMeter", "BarMeterNet_Float", 0);
		SetMyRegLong("BarMeter", "ColorBarMeterNet_Recv", (long)RGB(255, 0, 0));
		SetMyRegLong("BarMeter", "ColorBarMeterNet_Send", (long)RGB(0, 255, 0));
		SetMyRegLong("BarMeter", "BarMeterNet_Width", 5);
		SetMyRegLong("BarMeter", "BarMeterNet_Height", -1);

		SetMyRegLong("BarMeter", "BarMeterNetRecv_Right", 160);
		SetMyRegLong("BarMeter", "BarMeterNetRecv_Bottom", 0);
		SetMyRegLong("BarMeter", "BarMeterNetSend_Right", 170);
		SetMyRegLong("BarMeter", "BarMeterNetSend_Bottom", 0);

		

		SetMyRegStr("VPN", "SoftEtherKeyword", "VPN Client Adapter");
		SetMyRegStr("VPN", "VPN_Keyword1", "");
		SetMyRegStr("VPN", "VPN_Keyword2", "");
		SetMyRegStr("VPN", "VPN_Keyword3", "");
		SetMyRegStr("VPN", "VPN_Keyword4", "");
		SetMyRegStr("VPN", "VPN_Keyword5", "");

		SetMyRegStr("VPN", "VPN_Exclude1", "");
		SetMyRegStr("VPN", "VPN_Exclude2", "");
		SetMyRegStr("VPN", "VPN_Exclude3", "");
		SetMyRegStr("VPN", "VPN_Exclude4", "");
		SetMyRegStr("VPN", "VPN_Exclude5", "");



		SetMyRegLong("ETC", "ZombieCheckInterval", 10);
		SetMyRegStr("ETC", "LTEString", "LTE");
		SetMyRegStr("ETC", "LTEChar", "L");
		SetMyRegStr("ETC", "MuteString", "*");
		SetMyRegStr("ETC", "2chHelpURL", "http://tclock2ch.no.land.to/");
		SetMyRegLong("ETC", "NetMIX_Length", 10);
		SetMyRegLong("ETC", "SSID_AP_Length", 10);

		SetMyRegStr("ETC", "Ethernet_Keyword1", "");
		SetMyRegStr("ETC", "Ethernet_Keyword2", "");
		SetMyRegStr("ETC", "Ethernet_Keyword3", "");
		SetMyRegStr("ETC", "Ethernet_Keyword4", "");
		SetMyRegStr("ETC", "Ethernet_Keyword5", "");

//		SetMyRegStr("ETC", "HolidayList", "2020, 0101, 0113, 0211, 0223, 0224, 0320, 0429, 0503, 0504, 0505, 0506, 0723, 0724, 0810, 0921, 0922, 1103, 1123");

		SetMyRegStr("ETC", "AdditionalMountPath0", "");
		SetMyRegStr("ETC", "AdditionalMountPath1", "");
		SetMyRegStr("ETC", "AdditionalMountPath2", "");
		SetMyRegStr("ETC", "AdditionalMountPath3", "");
		SetMyRegStr("ETC", "AdditionalMountPath4", "");
		SetMyRegStr("ETC", "AdditionalMountPath5", "");
		SetMyRegStr("ETC", "AdditionalMountPath6", "");
		SetMyRegStr("ETC", "AdditionalMountPath7", "");
		SetMyRegStr("ETC", "AdditionalMountPath8", "");
		SetMyRegStr("ETC", "AdditionalMountPath9", "");


	}
	else
	{
		MessageBox(NULL, "tclock-win10.iniの作成に失敗しました。書き込み可能なフォルダで実行してください",
			"TClock-Win10", MB_ICONERROR | MB_OK);
	}
}


/*------------------------------------------------
initialize the registy	//Added by TTTT
--------------------------------------------------*/
BOOL CheckRegistry_Win10(void)
{
	WIN32_FIND_DATA fd;
	HANDLE hfind;
	char fname[MAX_PATH];
	char s[80];
	BOOL br = FALSE;

	strcpy(fname, g_mydir);
	add_title(fname, "tclock-win10.ini");
	hfind = FindFirstFile(fname, &fd);

	if (hfind == INVALID_HANDLE_VALUE)
	{
		CreateDefaultIniFile_Win10(fname);
		hfind = FindFirstFile(fname, &fd);
	}

	if (hfind != INVALID_HANDLE_VALUE)
	{
		FindClose(hfind);
		g_bIniSetting = TRUE;
		strcpy(g_inifile, fname);

		br = TRUE;

		GetMyRegStr("Color_Font", "Font", s, 80, "");
		if (s[0] == 0)
		{
			HFONT hfont;
			LOGFONT lf;
			hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			if (hfont)
			{
				GetObject(hfont, sizeof(lf), (LPVOID)&lf);
				SetMyRegStr("Color_Font", "Font", lf.lfFaceName);
			}
		}


		GetMyRegStr("Status_DoNotEdit", "Version", s, 16, "");
		//if ((strcmp(s, "3.3.4.1") == 0)
		//	| (strcmp(s, "3.3.3.1") == 0)
		//	| (strcmp(s, "3.3.2.1") == 0)
		//	| (strcmp(s, "3.3.1.1") == 0))
		//{
		//	SetMyRegStr("VPN", "VPN_Keyword1", "");
		//	SetMyRegStr("VPN", "VPN_Keyword2", "");
		//	SetMyRegStr("VPN", "VPN_Keyword3", "");
		//	SetMyRegStr("VPN", "VPN_Keyword4", "");
		//	SetMyRegStr("VPN", "VPN_Keyword5", "");
		//}

		//if ((strcmp(s, "") != 0) && (strcmp(s, exeVersionString) != 0)) {
		//	//Version Upの時のダイアログ表示
		//	char tempString[1024];
		//	wsprintf(tempString, "%s%s%s%s%s%s%s%s",
		//		"アップデート情報(",
		//		exeVersionString,
		//		")\n\nサブモニタへの時計表示機能追加 (\"書式\"設定パネル)\n※起動時点でメインと同じ方向のタスクバーのみ。サブモニタクロックではクリック等の機能は働きません。",
		//		"\n\nツールチップの無効化設定追加 (\"ツールチップ\"設定パネル)",
		//		"\n\n\nUpdate Information (",
		//		exeVersionString,
		//		")\n\nTClocks on sub monitors (On \"Format\" settings)\n※Only same-direction taskbars. No TClock functions available on sub monitors",
		//		"\n\nTooltip can be disabled (On \"Tooltip\" settings)"
		//		);

		//	MessageBox(NULL, tempString, "TClock-Win10", MB_OK | MB_SETFOREGROUND | MB_ICONINFORMATION);
		//}

		SetMyRegStr("Status_DoNotEdit", "Version", exeVersionString);

	}


	return br;

}



// IsUserAnAdmin shell32.dll@680
// http://msdn2.microsoft.com/en-us/library/aa376389.aspx
static BOOL IsUserAdmin(void)
{
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	BOOL b = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
																		DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
																		&AdministratorsGroup);
	if (b)
	{
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &b))
		{
			b = FALSE;
		}
		FreeSid(AdministratorsGroup);
	}
	return b;
}





#define MSGFLT_ADD 1
#define MSGFLT_REMOVE 2
static BOOL AddMessageFilters(void)
{
	typedef BOOL (WINAPI *pfnChangeWindowMessageFilter)(UINT, DWORD);
	int i;
	UINT messages[] = {
		WM_CLOSE,
		WM_DESTROY,
		WM_COMMAND,
		WM_CONTEXTMENU,
		WM_EXITMENULOOP,
		WM_DROPFILES,
		WM_MOUSEWHEEL,
		WM_LBUTTONDOWN,
		WM_RBUTTONDOWN,
		WM_MBUTTONDOWN,
		WM_XBUTTONDOWN,
		WM_LBUTTONUP,
		WM_RBUTTONUP,
		WM_MBUTTONUP,
		WM_XBUTTONUP,
		WM_USER,
		WM_USER+1,
		WM_USER+2,
	};

	HMODULE hUser32 = GetModuleHandle("user32.dll");
	pfnChangeWindowMessageFilter ChangeWindowMessageFilter = (pfnChangeWindowMessageFilter)
		GetProcAddress(hUser32, "ChangeWindowMessageFilter");
	if (!ChangeWindowMessageFilter)
		return FALSE;

	for (i = 0; i < RTL_NUMBER_OF(messages); i++) {
		ChangeWindowMessageFilter(messages[i], MSGFLT_ADD);
	}
	return TRUE;
}






/*-------------------------------------------
getExeVersion added by TTTT
---------------------------------------------*/
void getExeVersion(char *fname)
{
	DWORD size;
	char *pBlock;
	VS_FIXEDFILEINFO *pffi;


	size = GetFileVersionInfoSize(fname, 0);
	if (size > 0)
	{
		pBlock = malloc(size);
		if (GetFileVersionInfo(fname, 0, size, pBlock))
		{
			UINT tmp;
			if (VerQueryValue(pBlock, "\\", &pffi, &tmp))
			{
				exeVersionM = pffi->dwFileVersionMS;
				exeVersionL = pffi->dwFileVersionLS;

				wsprintf(exeVersionString, "%d.%d.%d.%d", (int)HIWORD(exeVersionM), (int)LOWORD(exeVersionM)
					, (int)HIWORD(exeVersionL), (int)LOWORD(exeVersionL));
			}
		}
		free(pBlock);
	}

}




/*-------------------------------------------
  entry point of program
  not use "WinMain" for compacting the file size
---------------------------------------------*/
#ifndef _DEBUG
#pragma comment(linker, "/subsystem:windows")
#pragma message("entry WinMainCRTStartup")
void __cdecl WinMainCRTStartup(void)
{
#else
#pragma message("entry WinMain")
int WINAPI WinMain(HINSTANCE hinst,HINSTANCE hinstPrev,LPSTR lpszCmdLine, int nShow)
{
	UNREFERENCED_PARAMETER(hinst);
	UNREFERENCED_PARAMETER(hinstPrev);
	UNREFERENCED_PARAMETER(lpszCmdLine);
	UNREFERENCED_PARAMETER(nShow);
#endif

	g_hInst = GetModuleHandle(NULL);

	SetProcessShutdownParameters(0x1FF, 0); // 最後の方でシャットダウンするアプリケーション

	ExitProcess(TclockExeMain());
}


/*------------------------------------------------
Open a file copied from alarm.c
--------------------------------------------------*/
BOOL ExecFile(HWND hwnd, char* command)
{
	char fname[MAX_PATH], fpath[MAX_PATH], *opt;
	SHELLEXECUTEINFO sei;

	UNREFERENCED_PARAMETER(hwnd);
	if (*command == 0) return FALSE;

	opt = malloc(strlen(command));
	if (opt == NULL) return FALSE;
	GetFileAndOption(command, fname, opt);
	strcpy(fpath, fname);
	del_title(fpath);
	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.lpFile = fname;
	sei.lpDirectory = fpath;
	sei.lpParameters = opt[0] ? opt : NULL;
	sei.nShow = SW_SHOW;
	ShellExecuteEx(&sei);
	free(opt);

	return (sei.hInstApp > (HINSTANCE)32);
}


/*--------------------------------------------------------
Retrieve a file name and option from a command string
copied from alarm.c
----------------------------------------------------------*/
void GetFileAndOption(const char* command, char* fname, char* opt)
{
	const char *p, *pe;
	char *pd;
	WIN32_FIND_DATA fd;
	HANDLE hfind;

	p = command; pd = fname;
	pe = NULL;
	for (; ;)
	{
		if (*p == ' ' || *p == 0)
		{
			*pd = 0;
			hfind = FindFirstFile(fname, &fd);
			if (hfind != INVALID_HANDLE_VALUE)
			{
				FindClose(hfind);
				pe = p;
				break;
			}
			if (*p == 0) break;
		}
		*pd++ = *p++;
	}
	if (pe == NULL) pe = p;

	p = command; pd = fname;
	for (; p != pe; )
	{
		*pd++ = *p++;
	}
	*pd = 0;
	if (*p == ' ') p++;

	pd = opt;
	for (; *p; ) *pd++ = *p++;
	*pd = 0;
}


static BOOL IsWow64(void)
{
	BOOL bIsWow64 = FALSE;

	typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS)(HANDLE hProcess, PBOOL Wow64Process);
	LPFN_ISWOW64PROCESS IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
		GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if (IsWow64Process)
	{
		if (!IsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			bIsWow64 = FALSE;
		}
	}
	return bIsWow64;
}