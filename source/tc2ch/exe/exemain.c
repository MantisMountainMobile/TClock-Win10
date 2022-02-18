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

char      g_inifile[MAX_PATH];		//�t���p�X��*.ini�t�@�C����

BOOL	b_DisplayChanged = FALSE;






// �X���b�v�A�E�g������ /WS:AGGRESSIVE			
// 20181125 Ver3.3.2.1�ŃR�[�h����̓R�����g�A�E�g���ėl�q������B�X���b�v�A�E�g���Ă�0.3-2MB���x���A�X���b�v���Ȃ���΍ő�3.5MB���x�B
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

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	//tclock.exe�{�̂̃E�B���h�E�v���V�[�W���R�[���o�b�N

char szClassName[] = "TClockMainClass"; // window class name
char szWindowText[] = "TClock";         // caption of the window	(TClock-Win10�ɂ���H)

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
		hwnd = FindWindow(szClassName, szWindowText);		//�v���Z�X�����łɋN�����Ă�����hwnd != NULL�ɂȂ�
		if (hwnd == NULL) return FALSE;
		Sleep(100);
	}

	MessageBox(NULL, "�ċN�������݂܂������A������TClock-Win10�̃v���Z�X�I���Ɏ��s���܂����B�����_�Ő���Ɏ��v����������Ă��Ȃ��ꍇ�́A�^�X�N�}�l�[�W���[����TClock-Win10�̃v���Z�X�������I�����Ă��������B\n\nFailed to terminate the previous TClock-Win10 proccess. If you don't see the modified Clock on Taskbar, please kill the previous TClock-Win10 in the Taskmanager.",
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
	//	hwnd = FindWindow(szClassName, szWindowText);		//�v���Z�X�����łɋN�����Ă�����hwnd != NULL�ɂȂ�
	//	for (int i = 0; i < 10; i++)
	//	{
	//		if (hwnd != NULL) Sleep(500);
	//		hwnd = FindWindow(szClassName, szWindowText);		//�v���Z�X�����łɋN�����Ă�����hwnd != NULL�ɂȂ�
	//	}
	//	if (hwnd != NULL)
	//	{
	//		MessageBox(NULL, "������TClock-Win10�̃v���Z�X�I���Ɏ��Ԃ��������Ă��܂��B�wOK�x�������Ă��ċN�����Ȃ��ꍇ�ɂ̓^�X�N�}�l�[�W���[����TClock-Win10�̃v���Z�X�������I�����Ă��������B\n\nTerminating Previous TClock-Win10 is taking a long time. If you do not have the restarted TClock-Win10 even after clicking \"OK\", please kill the previous TClock-Win10 in the Taskmanager.",
	//			"TClock-Win10", MB_ICONEXCLAMATION | MB_SETFOREGROUND);
	//		hwnd = FindWindow(szClassName, szWindowText);
	//		if (hwnd != NULL) return 1;
	//	}
	//}




	// check wow64
	if (IsWow64()) {
		MessageBox(NULL, "�{���s�t�@�C����32bit (x86)�o�C�i���ł��B\n64bit���ł�x64�o�C�i�����g�p����K�v������܂��B\n\nThis is 32bit (x86) binary.\nx64 binary is required for 64bit Windows.",
			"TClock-Win10", MB_ICONERROR | MB_SETFOREGROUND);
		return 1;
	}



	// for Vista 
	if (IsUserAdmin()) {		//�Ǘ��Ҍ����������
		AddMessageFilters();	//���b�Z�[�W�t�B���^(�E�B���h�E�v���V�[�W����)��ݒ肷��(AddMessageFilters)
	}	



	// Call WINAPI CheckWinVersion_Win10() in tcdll.dll
	if (CheckWinVersion_Win10() < 0x0400) // = WIN10, 1024
	{
		MessageBox(NULL, "�{�A�v���P�[�V������Windows10�ȍ~�p�ł��B\n\nThis application works on Windows 10(Anniversary Update) or later.",
			"TClock-Win10", MB_ICONERROR | MB_SETFOREGROUND);
		return 1;
	}




	// get the path where .exe is positioned
	GetModuleFileName(g_hInst, g_mydir, MAX_PATH);	//���̎��_�ł̓t���p�X�t����tclock���s�t�@�C�������擾
	char fname[MAX_PATH];
	strcpy(fname, g_mydir);		//exe�̃t�@�C�����������܂܂�g_mydir��fname�ɓ����
	getExeVersion(fname);		//file�o�[�W�������擾����exeVersionM, exeVersionL���擾->DLL�o�[�W�����`�F�b�N�ɗ��p
	del_title(g_mydir);		//g_mydir�͂���ȍ~�ATClock�̃t�H���_�ւ̃p�X�Ƃ��ė��p�����



	//CheckRegistry();
	if (!CheckRegistry_Win10()) {		//���O�ɂ�Registry�Ƃ��邪�Aini�t�@�C����T���A�Ȃ���΍쐬����֐�
		MessageBox(NULL, "tclock-win10.ini���������炸�A�܂��쐬�Ɏ��s���܂����B�A�v���P�[�V�������I�����܂��B\n\nCould not access / create tclock-win10.ini.",
			"TClock-Win10", MB_ICONERROR | MB_SETFOREGROUND);
		return 1;
	}




	// not to execute the program twice
	hwnd = FindWindow(szClassName, szWindowText);
	if(hwnd != NULL)				//���łɃv���Z�X���N�����Ă���΁A	
	{
		CheckCommandLine(hwnd);		//�R�}���h���C���I�v�V�������`�F�b�N("/exit"�̏ꍇ�̂���)����	

		if (b_Exit) return 1;
		else if (b_RestartNOW)
		{
			//SetMyRegLong("Status_DoNotEdit", "LastLaunchTimeStamp", 0);
			PostMessage(hwnd, WM_CLOSE, 0, 0);		//���C���E�B���h�E��WM_CLOSE(102)�𑗏o����B���X���s���邪�A�����������܂������B
			if (WaitQuitPrevTClock(50))	return 1;
		}
		else
		{
			int reply = MessageBox(NULL, "TClock�̃v���Z�X���ғ����ł��B�ċN�����܂����H\n�wOK�x��I�Ԃƌ��݂̃v���Z�X���I�����ĐV�v���Z�X�ōċN�����܂��B\n�w�L�����Z���x��I�Ԃƌ��݂̃v���Z�X���ێ����܂��B\n\nPrevious TClock process is still running. Will you restart TClock?\nChoosing:\n\"OK\" initiates restarting from existing TClock Process.\n\"Cancel\" simply aborts this new process",
				"TClock-Win10", MB_ICONEXCLAMATION | MB_OKCANCEL | MB_DEFBUTTON1 | MB_SETFOREGROUND);
			if (reply == IDOK)
			{
				SetMyRegLong("Status_DoNotEdit", "LastLaunchTimeStamp", 0);
				PostMessage(hwnd, WM_CLOSE, 0, 0);		//���C���E�B���h�E��WM_CLOSE(102)�𑗏o����B���X���s���邪�A�����������܂������B			
				if (WaitQuitPrevTClock(50))	return 1;
				b_RestartNOW = TRUE;	//NormalLog�ւ̘A���̂��߂ɍė��p�B
			}
			else
			{
				return 1;					//�I������
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

	//�N�����ɑO��I�����̘A�����X�^�[�g�񐔂��擾����
	countRestart = GetMyRegLong("Status_DoNotEdit", "CountAutoRestart", 0);
	if (countRestart >= MAX_AUTORESTART) {
		MessageBox(NULL, "�N���b�V�����[�v�����o���܂����B�A�v���P�[�V�������I�����܂��B\n\nTClock is terminated because of repeting crash.",
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

	if(!CheckTCDLL()) { return 1; }	//tclock.dll�̃o�[�W�����`�F�b�N

	g_hInstResource = LoadLanguageDLL(g_langdllname);		//langja.dll�̃��[�h�����݂�
	if(g_hInstResource == NULL) { return 1; }				//langja.dll�����[�h�ł��Ȃ���Β�~

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
	hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, szClassName, szWindowText,		//������xzClassName, szWindowText����o�^���āAhwnd���擾
		0, CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
		NULL, NULL, g_hInst, NULL);
	//ShowWindow(hwnd, SW_SHOW);	//�����Ȃ��E�B���h�E�����݂��Ă���B���̍s�̃R�����g�A�E�g���O���ƌ�����B���ꂪ"hwnd"
	//UpdateWindow(hwnd);


	if (b_DebugLog) WriteDebug_New2("[exemain.c][TclockExeMain] Hidden Main Window Created");


	if(OleInitialize(NULL) != S_OK){	//STA�i�V���O���X���b�h�A�p�[�g�����g�j�X���b�h�Ƃ��ď��������AOLE�p�̒ǉ��������s���c�炵���B
		MessageBox(NULL, "OLE�̏������Ɏ��s���܂����B\n\nFailed to initialize OLE.", "TClock-Win10", MB_ICONERROR);
	}

	g_hwndMain = hwnd;	//���C���B���E�B���h�E�̃n���h�����O���[�o���ϐ���g_hwndMain�ɃR�s�[

	SetIdlePriority();	//�f�t�H���g�ł�IDLE_PRIORITY_CLASS�Ƃ���	added by TTTT

	CheckCommandLine(hwnd);		//�R�}���h���C���`�F�b�N�B���̎��_�ł̓^�X�N�g���C�̉����͍s���Ă��Ȃ��B���̒��ŊJ�n�E�F�C�g���ݒ肳��Ă���(?)

	//b_RestartDLL = FALSE;		//�ǂ������̂��Ǝg��Ȃ����A�C���������̂ŃN���A
	
	HPOWERNOTIFY handle_PowerNotify;
	b_ModernStandbySupported = CheckModernStandbyCapability_Win10();
	if (b_ModernStandbySupported)
	{
		handle_PowerNotify = RegisterPowerSettingNotification(hwnd, &GUID_CONSOLE_DISPLAY_STATE, DEVICE_NOTIFY_WINDOW_HANDLE);
	}

	while(GetMessage(&msg, NULL, 0, 0))		//�L���[���烁�b�Z�[�W���󂯎��GetMessage�̖߂�l��0�ɂȂ�(��WM_QUIT���|�X�g�����) �܂ŁA
											//�擾�������b�Z�[�W���E�B���h�E�v���V�[�W���ɑ���Â���
	{
		if(g_hwndPropDlg && IsWindow(g_hwndPropDlg)
			&& IsDialogMessage(g_hwndPropDlg, &msg))	//g_hwndPropDlg�̃��b�Z�[�W�͖���
			;
		//else if(g_hDlgTimer && IsWindow(g_hDlgTimer)
		//	&& IsDialogMessage(g_hDlgTimer, &msg))		//g_hDlgTimer�̃��b�Z�[�W�͖���
		//	;
		else		//����ȊO�͈ȉ���2�֐��ŏ���
		{
			TranslateMessage(&msg);	//TranslateMessage: ���z�L�[���b�Z�[�W�𕶎����b�Z�[�W�֕ϊ�(?)
			DispatchMessage(&msg);	// DispatchMessage�Ŏ󂯎�������b�Z�[�W���E�B���h�E�v���V�[�W��(?)�ɑ��o.
		}
	}

	if (b_DebugLog) WriteDebug_New2("[exemain.c][TclockExeMain] Got out from the main message loop");

	if(g_hMenu) DestroyMenu(g_hMenu);				//�E�N���b�N���j���\�����Ȃ�A���j���[������

	if(g_hInstResource) FreeLibrary(g_hInstResource);		//langja.dll���A�����[�h

	UnregisterClass(szClassName, g_hInst);	// for TTBASE �c�Ə����Ă��������A���Ԃ�K�v 

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
			if(_strnicmp(p, "prop", 4) == 0)	//prop�I�v�V�����F�v���p�e�B���J��
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with  prop option");
				PostMessage(hwnd, WM_COMMAND, IDC_SHOWPROP, 0);
				p += 4;
			}
			//else if (_strnicmp(p, "restartdll", 10) == 0)	//exit�I�v�V�����F�I���������s��
			//{
			//	//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with  restartdll option");
			//	b_RestartDLL = TRUE;
			//	p += 10;
			//}
			else if (_strnicmp(p, "restart", 7) == 0)	//exit�I�v�V�����F�I���������s��
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with  restart option");
				b_RestartNOW = TRUE;
				p += 10;
			}
			else if(_strnicmp(p, "exit", 4) == 0)	//exit�I�v�V�����F�I���������s��
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with  exit option");
				b_Exit = TRUE;
				PostMessage(hwnd, WM_CLOSE, 0, 0);		//���C���E�B���h�E��WM_CLOSE(102)�𑗏o����
				p += 4;
			}
			else if(_strnicmp(p, "nowait", 6) == 0)	//nowait�I�v�V�����F�x���X�^�[�g�𖳎�
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with nowait option");
				KillTimer(hwnd, IDTIMER_START);				//���ݓ����Ă���IDTIMER_START���~
				SetTimer(hwnd, IDTIMER_START, 100, NULL);	//100ms�E�F�C�g��IDTIMER_START���J�n, �^�C���A�E�g���ɂ̓��b�Z�[�W���o
				p += 6;
			}
			else if(_strnicmp(p, "idle", 4) == 0)	//idle�I�v�V�����F�D��x��IDLE�ɂ��ċN��
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][CheckCommandLine] Launched with  idle option");
				HANDLE op = OpenProcess(PROCESS_ALL_ACCESS, TRUE, GetCurrentProcessId());
				SetPriorityClass(op, IDLE_PRIORITY_CLASS);
				Sleep(10);
				p += 4;
			}
			else if (_strnicmp(p, "normal", 4) == 0)	//normal�I�v�V�����F�D��x��NORMAL�ɂ��ċN��
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
LRESULT CALLBACK WndProc(HWND hwnd,	UINT message, WPARAM wParam, LPARAM lParam)	//message���[�v�̒���DispatchMessage()����̃��b�Z�[�W���󂯂Ă�H
{
	switch (message)
	{
		case WM_CREATE:
		{
			//if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_CREATE received");
			int nDelay;
			bDestroy = FALSE;
			InitFormat(); // initialize a Date/Time format
			nDelay = GetMyRegLong(NULL, "DelayStart", 0);		//�ݒ�t�@�C������x���X�^�[�g�b����ǂݍ���
			if(nDelay > 0)
			{
				SetTimer(hwnd, IDTIMER_START, nDelay * 1000, NULL);		//�^�C�}�[���N��(�^�C���A�E�g���Ƀ��C���E�B���h�E����WM_TIMER, IDTIMER_START)���b�Z�[�W
				bcontractTimer = TRUE;									//�N���^�C�}�[���쒆�t���OTRUE
			}
			else SendMessage(hwnd, WM_TIMER, IDTIMER_START, 0);			//�^�C�}�[�̑���Ƀ��C���E�B���h�E�ɑ���IDTIMER_START���b�Z�[�W
			InitMouseFunction(hwnd);
			SetTimer(hwnd, IDTIMER_MAIN, 1000, NULL);		//���C�����[�v�^�C�}�[(IDTIMER_MAIN)������1�b�ɐݒ�, �����_�ł͉��̏������s���Ă��Ȃ��B
			SetTimer(hwnd, IDTIMER_CREATE, 5000, NULL);		//�N���G�C�g�^�C�}�[(IDTIMER_CREATE)������5�b�ɐݒ�
			//DO_WS_AGGRESSIVE(); // Comment out by TTTT 20181125
			return 0;
		}
		case WM_TIMER:		//WM_TIMER�ɑ΂��鏈��
			if(wParam == IDTIMER_START)		//�N���p�^�C�}�[�̃^�C���A�E�g�̏���
			{
				if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_TIMER(IDTIMER_START) received");
				if(bcontractTimer) KillTimer(hwnd, wParam);		//�^�C�}�[��~
				bcontractTimer = FALSE;							//�N���^�C�}�[���쒆�t���OFALSE
				HookStart(hwnd);				// install a hook	dllmain.c�̒��ɂ���B�d�v�B�^�X�N�g���C�̃��b�Z�[�W���t�b�N�B�R�A�@�\�̋N��

				SetTimer(hwnd, IDTIMER_ZOMBIECHECK, zombieCheckInterval * 1000, NULL);	//

				//nCountFindingClock = 0;			// ���v�J�E���^(�G���[���o�p�H)
				//DO_WS_AGGRESSIVE(); // Comment out by TTTT 20181125
			}
			else if (wParam == IDTIMER_MAIN)	//���C�����[�v�^�C�}�[(�f�t�H���g1�b)�̃^�C���A�E�g�̏���
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_TIMER(IDTIMER_MAIN) received");
				OnTimerMain(hwnd);
			}
			else if (wParam == IDTIMER_MOUSE)
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_TIMER(IDTIMER_MOUSE) received");
				OnTimerMouse(hwnd);
			}
			else if(wParam == IDTIMER_CREATE)	//�N���G�C�g�^�C�}�[(�f�t�H���g5�b)�̃^�C���A�E�g�̏���
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_TIMER(IDTIMER_CREATE) received");
				SetTimer(hwnd, IDTIMER_CREATE, 60000, NULL);	//�ȍ~�̓^�C���A�E�g��60�b�ɏꍇ
				//DO_WS_AGGRESSIVE(); // Comment out by TTTT 20181125
			}
			else if (wParam == IDTIMER_ZOMBIECHECK)	//�]���r�`�F�b�N�̃^�C���A�E�g�̏��� by TTTT 20181125
			{
				//if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_TIMER(IDTIMER_ZOMBIECHECK) received");
//				OnTimerZombieCheck(hwnd);
				OnTimerZombieCheck2(hwnd);
			}
			return 0;
		case WM_CLOSE:		//�蓮�ł̏C�������͂����炩��s���B
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_CLOSE received");
			TerminateTClock(hwnd);
			return 0;
		case WM_DESTROY:	//�I���������B���ۂɂ͂��ׂďI����Ă���͂��悤�ɂȂ��Ă���B
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_DESTROY received");
			return 0;
		case WM_ENDSESSION:	//�Z�b�V�����I������TClock�I���͂����炩��
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_ENDSESSION received");
			if(wParam) TerminateTClock(hwnd);
			break;
		case WM_QUERYENDSESSION:	//�Z�b�V�����I�����̎��O�m�F�̂悤�Ȃ��́B�C��������WM_ENDSESSION�Ɏ�������B
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_QUERYENDSESSION received");
			if (b_NormalLog) WriteNormalLog("Exit TClock-Win10 by EndSession.");
			break;		//DefWindowProc��1��Ԃ��Ă����̂ŔC����B
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
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_USER received");	//DLL Window�������Ƀn���h����ʒm
			//nCountFindingClock = -1;
			g_hwndClock = (HWND)lParam;
			return 0;
		case (WM_USER+1):   // error
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_USER+1 received");	//DLL Window�������̃G���[
			//nCountFindingClock = -1;
			InitError((int)lParam);
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		case (WM_USER+2):   // exit (from tclock.c EndClock())	���̃R�[�h��Ver4.0.3�ȍ~�ł͏C�������Ƃ��ČĂ΂�Ȃ��Ȃ��Ă���(�͂�)�B�K���Ȏ����ɍ폜���邱�ƁB
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_USER+2 received");

			TerminateTClockFromDLL(hwnd);	//tcdlll(tclock.c����Ă΂��Ƃ���FromDLL�����s����)
			////�^�C�}�@�\�̂Ȃ���
			////if(g_hDlgTimer && IsWindow(g_hDlgTimer))
			////	PostMessage(g_hDlgTimer, WM_CLOSE, 0, 0);
			////g_hDlgTimer = NULL;
			return 0;
		case CLOCKM_DLLALIVE:
			OnDLLAliveMessage(wParam);	//wParam�ɏ�������z��(���g�p)
			return 0;
		case WM_WININICHANGE:		//��ʃe�[�}���ς�������̑Ή�
		{
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_WININICHANGE received");
			PostMessage(g_hwndClock, WM_COMMAND, (WPARAM)CLOCKM_BGCOLOR_UPDATE, 0);
			return 0;
		}
		case WM_SYSCOLORCHANGE:		//SYSCOLOR���ς�����ꍇ�́A�A�C�R�����w�i�������̃^�C�}�[(IDTIMER_DESKTOPICON)���쓮�����邾���B�s�v���B
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_SYSCOLORCHANGE received");
			PostMessage(hwnd, WM_USER+10, 1,0);
			return 0;
		case (WM_USER+10):		//WM_USER+10��DESKCAL�֘A�̂悤�Ȃ̂ŁA�폜���Ă��悢���B���������t�B���^����ē͂��Ȃ������B
		{
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_USER+10 received");
			return 0;
		}

		// return from power saving
		case WM_POWERBROADCAST:		//������J�����_�[�֌W�݂̂̃R�[�h�B�s�v���B
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
		case WM_COMMAND:	//�E�N���b�N���j���[����R�}���h���s���ꂽ�ꍇ�͂�����ʂ���menu.c�̃R�[�h�ɓ͂�����͗l
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_COMMAND received");
			OnTClockCommand(hwnd, LOWORD(wParam), HIWORD(wParam)); // menu.c
			//DO_WS_AGGRESSIVE(); // Comment out by TTTT 20181125
			return 0;
		// messages transfered from the dll
		case WM_CONTEXTMENU:	//�E�N���b�N���ꂽ�ꍇ->���j���[���J���֐�(OnContextMenu())���R�[��
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_CONTEXTMENU received");
			// menu.c
			OnContextMenu(hwnd, (HWND)wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			//DO_WS_AGGRESSIVE(); // Comment out by TTTT 20181125
			return 0;
		case WM_DROPFILES:	//�t�@�C�����h���b�v���ꂽ�ꍇ�̏����A�v�m�F
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
		case WM_ENTERMENULOOP:	//�E�N���b�N���j���[�̓��͑҂����[�v�ɓ��������ɏo��B
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_ENTERMENULOOP received");
			bMenuOpened = TRUE;
			break;
		case WM_EXITMENULOOP:	//�E�N���b�N���j���[�̓��͑҂����[�v����o�����ɏo��B
			if (b_DebugLog) WriteDebug_New2("[exemain.c][WndProc] WM_EXITMENULOOP received");
			bMenuOpened = FALSE;
			break;
		case WM_HOTKEY:	//Hotkey�@�\�B�v�ۗv����
			OnHotkey(hwnd, (int)wParam);
			break;
		case WM_MEASUREITEM:	//�t�@�C�����X�g�֌W
			OnMeasureItem(hwnd, wParam, lParam); // filelist.c
			break;
		case WM_DRAWITEM:	//�t�@�C�����X�g�֌W
			OnDrawItem(hwnd, wParam, lParam); // filelist.c
			break;
		case WM_INITMENUPOPUP:	//�t�@�C�����X�g�֌W
			OnInitMenuPopup(hwnd, wParam, lParam); // filelist.c
			break;
		case WM_MENURBUTTONUP:	//�t�@�C�����X�g�֌W
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

			PostMessage(hwnd, WM_CLOSE, 0, 0);		//���C���E�B���h�E��WM_CLOSE(102)�𑗏o����B(���̏I�������dll��s�ł͂Ȃ���exemain����Ȃ̂�)

		}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

void TerminateTClock(HWND hwnd)
{
	//TClock�����쒆����I������ꍇ�ɂ́A������ŏI������B

	if (b_DebugLog) WriteDebug_New2("[exemain.c] TerminateTClock called.");

	if (g_hwndClock)
	{
		SendMessage(g_hwndClock, WM_COMMAND, IDC_EXIT, 0);
		g_hwndClock = NULL;		//EndClock����Ă΂ꂽ����(EndClock���s�ς݂̊m�F�̂��߂ɏ���)
	}
	if (g_hwndPropDlg && IsWindow(g_hwndPropDlg))
		PostMessage(g_hwndPropDlg, WM_CLOSE, 0, 0);
	g_hwndPropDlg = NULL;

	HookEnd();  // uninstall a hook, Ver 4.0.5.3�ȍ~�ł͂��łɃt�b�N�O��Ă���͂������O�̂��߁B

	if (bDestroy == TRUE) return 0;	//�Q�d�I�����Ȃ��悤��

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

	//FromDLL�́A���v�̂ق��ŏI������(EndClock)���Ă���Ă΂��(WM_USER+2�o�R)�󋵂Ŏg���B

	if (g_hwndClock)
	{
//		SendMessage(g_hwndClock, WM_COMMAND, IDC_EXIT, 0);
		g_hwndClock = NULL;		//EndClock����Ă΂ꂽ����(EndClock���s�ς݂̊m�F�̂��߂ɏ���)
	}
	if (g_hwndPropDlg && IsWindow(g_hwndPropDlg))
		PostMessage(g_hwndPropDlg, WM_CLOSE, 0, 0);
	g_hwndPropDlg = NULL;

	HookEnd();  // uninstall a hook, Ver 4.0.5.3�ȍ~�ł͂��łɃt�b�N�O��Ă���͂������O�̂��߁B

	if (bDestroy == TRUE) return 0;	//�Q�d�I�����Ȃ��悤��

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
void OnTimerMain(HWND hwnd)		//���C�����[�v�^�C�}�[(�f�t�H���g1�b)�̃^�C���A�E�g�̏���
{
	//���v�E�B���h�E�̃^�C�}�[���삪��~����2�b������t���O�����B
	//���ׂ��������̂Ŏc���Ă��邪�A���݂̂Ƃ����~���쓙�͎�������Ă��Ȃ��B
	//Ver4.0.4���_�ŕs���̎��̂ɑ΂���Ώ���OnTimerZombieCheck2�ōs���Ă���B
	//�X���[�v���Ńt���O���o�̂ł����ɏI�����Ă͂����Ȃ��B

	LONG currentFileTime = 0;

	GetSystemTimeAsFileTime(&currentFileTime);

	if (((LONGLONG)currentFileTime - (LONGLONG)lastFileTimeDLLAlive) > 20000000)
	{
		b_FlagDLLAlive = FALSE;	//Alive���b�Z�[�W��2�b�J����DLL���M�s�ʃt���O�𗧂Ă�(�����\)
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
			TerminateTClockFromDLL(hwnd);		//���ł�TClock�̉������͏I��/�������Ă���Ɣ��f����邽�߁AFromDLL�ł̏I��������s���B

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
			//	TerminateTClockFromDLL(hwnd);		//���ł�TClock�̉������͏I��/�������Ă���Ɣ��f����邽�߁AFromDLL�ł̏I��������s���B
			//}
		}
	}
}



//void OnTimerZombieCheck(HWND hwnd)	//�]���r�`�F�b�N by TTTT, Ver 4.0.4�ȍ~�͎g���Ă��Ȃ��B
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
//				PostMessage(hwnd, WM_CLOSE, 0, 0);		//���C���E�B���h�E��WM_CLOSE(102)�𑗏o����B
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
		MessageBox(NULL, "tclock-win10.ini�̍쐬�Ɏ��s���܂����B�������݉\�ȃt�H���_�Ŏ��s���Ă�������",
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
		//	//Version Up�̎��̃_�C�A���O�\��
		//	char tempString[1024];
		//	wsprintf(tempString, "%s%s%s%s%s%s%s%s",
		//		"�A�b�v�f�[�g���(",
		//		exeVersionString,
		//		")\n\n�T�u���j�^�ւ̎��v�\���@�\�ǉ� (\"����\"�ݒ�p�l��)\n���N�����_�Ń��C���Ɠ��������̃^�X�N�o�[�̂݁B�T�u���j�^�N���b�N�ł̓N���b�N���̋@�\�͓����܂���B",
		//		"\n\n�c�[���`�b�v�̖������ݒ�ǉ� (\"�c�[���`�b�v\"�ݒ�p�l��)",
		//		"\n\n\nUpdate Information (",
		//		exeVersionString,
		//		")\n\nTClocks on sub monitors (On \"Format\" settings)\n��Only same-direction taskbars. No TClock functions available on sub monitors",
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

	SetProcessShutdownParameters(0x1FF, 0); // �Ō�̕��ŃV���b�g�_�E������A�v���P�[�V����

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