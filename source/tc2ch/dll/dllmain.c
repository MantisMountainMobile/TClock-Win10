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
	// explorer.exe�̃^�X�N�o�[�̃X���b�hID���擾����
	ThreadID = GetWindowThreadProcessId(hwndTaskBarMain, NULL);
	if(!ThreadID)
	{
		SendMessage(hwnd, WM_USER+1, 0, 2);
		return;
	}

	// install an hook to thread of taskbar
	//�擾����explorer.exe�̃X���b�h��(CallWndProc�Ŏ󂯂���悤��)�t�b�N����
	hHookMain = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, hmod, ThreadID);
	if(!hHookMain)
	{
		SendMessage(hwnd, WM_USER+1, 0, 3);
		return;
	}

	//Explorer.exe�̃t�b�N�ɐ���������A���̏����TaskBar�ւ̃��b�Z�[�W��InitClock���g���K�����B
	//�ȉ��̃R�[�h�͂Ȃ��Ă��������A���܂��Ȃ��Ƃ��āB
	//�����ȍ~�ɏ������R�[�h�́AInitClock���O�Ɏ��s�����ۏ�͂Ȃ��B

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
	//	PostMessage(hwndTaskBarMain, WM_SIZE, SIZE_RESTORED, 0);	//���C���N���b�N�͂���𑗂�ƃ^�X�N�g���C�Ĕz�u�A�ĕ`�悵�Ă����B
	//	//InvalidateRect(hwndTaskBarMain, NULL, TRUE);		//3.5.0.1���O�͂��̍s�����������A�����Ă��I����̍ĕ`��͏��PostMessage�����ł���Ă����悤���B
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
		//�C�ӂ̃^�X�N�o�[�����b�Z�[�W���͂�����1�񂾂�InitClock���Ă�
		if (!bStarted && pcwps->hwnd == hwndTaskBarMain)
		{
			bStarted = TRUE;
			InitClock();	//�t�b�N�����v���V�[�W������InitClock���N�����邱�Ƃ�explorer�̃X���b�h�ɑg�ݍ��܂�ă^�X�N�o�[�ɃA�N�Z�X�ł���B
		}
	}
	return CallNextHookEx(hHookMain, nCode, wParam, lParam);
}

