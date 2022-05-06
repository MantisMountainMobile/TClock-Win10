/*-----------------------------------------------------
  main.c
   API, hook procedure
   KAZUBON 1997-2001
-------------------------------------------------------*/

#include "tcdll.h"

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);


void InitClock();

/*------------------------------------------------
  shared data among processes
--------------------------------------------------*/
HHOOK hHookMain = 0;
extern HWND hwndTClockExeMain;
extern HWND hwndTaskBarMain;




/*------------------------------------------------
  globals
--------------------------------------------------*/
extern HANDLE hmod;

BOOL bStarted = FALSE;

/*------------------------------------------------
  entry point of this DLL
--------------------------------------------------*/

int WINAPI DllMain(HANDLE hModule, DWORD dwFunction, LPVOID lpNot)
{
	UNREFERENCED_PARAMETER(lpNot);
	UNREFERENCED_PARAMETER(dwFunction);

	hmod = hModule;
	DisableThreadLibraryCalls(hModule);
	return TRUE;
}


/*------------------------------------------------
   API: install my hook
--------------------------------------------------*/
void WINAPI HookStart(HWND hwnd)
{
	DWORD ThreadID;


	hwndTClockExeMain = hwnd;

	// find the taskbar
	hwndTaskBarMain = FindWindow("Shell_TrayWnd", "");
	if(!hwndTaskBarMain)
	{
		SendMessage(hwnd, WM_USER+1, 0, 1);
		return;
	}

	// get thread ID of taskbar (explorer)
	// Specal thanks to T.Iwata.
	// explorer.exeのタスクバーのスレッドIDを取得する
	ThreadID = GetWindowThreadProcessId(hwndTaskBarMain, NULL);
	if(!ThreadID)
	{
		SendMessage(hwnd, WM_USER+1, 0, 2);
		return;
	}

	// install an hook to thread of taskbar
	//取得したexplorer.exeのスレッドを(CallWndProcで受けられるように)フックする
	hHookMain = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, hmod, ThreadID);
	if(!hHookMain)
	{
		SendMessage(hwnd, WM_USER+1, 0, 3);
		return;
	}

	//Explorer.exeのフックに成功したら、次の初回のTaskBarへのメッセージでInitClockがトリガされる。
	//以下のコードはなくても動くが、おまじないとして。
	//ここ以降に書いたコードは、InitClockより前に実行される保障はない。

	if (hwndTaskBarMain) SendMessage(hwndTaskBarMain, WM_NULL, 0, 0);

}



/*------------------------------------------------
  API: uninstall my hook
--------------------------------------------------*/
void WINAPI HookEnd(void)
{
	if (hHookMain != NULL) {
		UnhookWindowsHookEx(hHookMain);
		hHookMain = NULL;
	}

	//// refresh the taskbar
	//if(hwndTaskBarMain)
	//{	
	//	PostMessage(hwndTaskBarMain, WM_SIZE, SIZE_RESTORED, 0);	//メインクロックはこれを送るとタスクトレイ再配置、再描画してくれる。
	//	//InvalidateRect(hwndTaskBarMain, NULL, TRUE);		//3.5.0.1より前はこの行があったが、無くても終了後の再描画は上のPostMessageだけでやってくれるようだ。
	//}
}

/*------------------------------------------------
  hook procedure
--------------------------------------------------*/
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	LPCWPSTRUCT pcwps = (LPCWPSTRUCT)lParam;

	if(nCode == HC_ACTION && pcwps && pcwps->hwnd)
	{
		//任意のタスクバー宛メッセージが届いたら1回だけInitClockを呼ぶ
		if (!bStarted && pcwps->hwnd == hwndTaskBarMain)
		{
			bStarted = TRUE;
			InitClock();	//フックしたプロシージャからInitClockを起動することでexplorerのスレッドに組み込まれてタスクバーにアクセスできる。
		}
	}
	return CallNextHookEx(hHookMain, nCode, wParam, lParam);
}

