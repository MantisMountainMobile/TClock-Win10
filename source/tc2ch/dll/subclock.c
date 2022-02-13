#include "tcdll.h"
#include "resource.h"



extern BOOL b_DebugLog;
extern HWND hwndTClockExeMain;
extern HWND	hwndTaskBarMain;
extern HWND hwndTrayMain;
extern HWND hwndClockMain;
extern HANDLE hmod;
extern int widthMainClockFrame;
extern int heightMainClockFrame;
extern BOOL bWin11Main;
extern BOOL bWin11Sub;
extern BOOL g_bVertTaskbar;



extern WNDPROC oldWndProcSub[];
extern HWND hwndTaskBarSubClk[];
extern HWND hwndClockSubClk[];
extern BOOL bEnableSpecificSubClk[];
extern BOOL bClearSubClk[];
extern BOOL bEnableSubClks;
extern BOOL bTimerSubClks;
extern int	heightSubClock[];
extern int	widthSubClock[];
extern int	origSubClockWidth[];
extern int	origSubClockHeight[];
extern int	widthSubTaskbar[];
extern int	heightSubTaskbar[];

extern BOOL bEnableTooltip;






/*------------------------------------------------
subclass procedure of the Sub Display clocks , 20211107 TTTT
--------------------------------------------------*/
LRESULT CALLBACK WndProcSubClk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//���̊֐��̓T�u�E�B���h�E�̎��v(hwndSubClk[i])�̃v���V�[�W���Ƃ��ēo�^���Ă���̂ŁAhwnd��hwndSubClk[i]�̂����ꂩ�ɂȂ�B
	//GetSubClkIndexFromHWND���g����i�𒲂ׂ�K�v������Bhwnd��hwndClockMain�ł͂Ȃ��̂Œ��ӂ���B

//	if (b_DebugLog) writeDebugLog_Win10("[subclock.c][WndProcSubClk] Window Message was recevied, message = ", message);

	switch (message) {
	case (WM_USER + 100):
		// �e�E�B���h�E���瑗���A�T�C�Y��Ԃ����b�Z�[�W�B
		// (Win10RS1(=AU)�ȍ~��)�ł̓��C���N���b�N�ɂ��T�u�N���b�N�ɂ��A�T�C�Y�������K�v�ȏꍇ�ɂ́A���̃��b�Z�[�W�͗��Ȃ��B
		break;
		//case (WM_NOTIFY):
	case (WM_NCCALCSIZE):
		//�T�u�^�X�N�o�[�ɕύX������Ƃ��ꂪ�͂��ASetAllSubClocks�̌�ɂ��͂��̂ŁA���ꂪ������^�X�N�o�[�������I��
		//�߂��ꂽ�A�Ɣ��f���邱�Ƃ͂ł��Ȃ��B�f�o�b�O�R�[�h�ɂ���悤�ȕ��G��lparam�̒��Ƀf�[�^�������Ă���B
		//rgrc[0]���ύX����悤�Ƃ���T�C�Y�Ȃ̂ŁA���ꂪ�T�u�N���b�N�̃T�C�Y�ƈႦ�΁AWindows�W�����v�ɖ߂���悤�Ƃ��Ă��邱�Ƃ��킩��B
		//���̏ꍇ��rgrc[0]�̒l��origSubClock�̃T�C�Y�Ƃ��čX�V����΁A�V�����^�X�N�o�[��Windows�W�����v�T�C�Y���킩��B
		//���̂����ŁA�߂�l��0(�e�F�Ƃ����Ӗ��H)�ł͂Ȃ��AWVR _VALIDRECTS(=0x0400)��Ԃ�����X�V����Ă��܂킸�ɍς�?
		//http://blog.livedoor.jp/oans/archives/50628113.html

		//������StoreSpecificSubClockDimensions���Ă�ł͍s���Ȃ��B�T�C�Y���܂��ύX����Ă��Ȃ����ߐ������l���擾���邱�Ƃ͂ł��Ȃ��B

	{
		int i, newWidth, newHeight;
		LRESULT ret;
		i = GetSubClkIndexFromHWND(hwnd);
		if ((i != 999) && (bEnableSpecificSubClk[i] == TRUE))
		{

			NCCALCSIZE_PARAMS* pncsp = (NCCALCSIZE_PARAMS*)lParam;

			if (b_DebugLog) {
				writeDebugLog_Win10("[subclock.c][WndProcSubClk] SubClock index = ", i);
				writeDebugLog_Win10("[subclock.c][WndProcSubClk] wParam = ", wParam);
			}

			newWidth = (int)(*pncsp).rgrc[0].right - (int)(*pncsp).rgrc[0].left;
			newHeight = (int)(*pncsp).rgrc[0].bottom - (int)(*pncsp).rgrc[0].top;
			if ((widthSubClock[i] != newWidth) || (heightSubClock[i] != newHeight)) //Clocksize will be changed to original by windows
			{


				origSubClockWidth[i] = newWidth;
				origSubClockHeight[i] = newHeight;

				if (b_DebugLog) {
					writeDebugLog_Win10("[subclock.c][WndProcSubClk] origSubClockWidth updated: ", origSubClockWidth[i]);
					writeDebugLog_Win10("[subclock.c][WndProcSubClk] origSubClockHeight updated: ", origSubClockHeight[i]);
				}

				//�V�����^�X�N�o�[�T�C�Y�Ȃǂ͕ۑ��ł��Ȃ����A���܂������̂ł܂��������A�Ƃ����Ƃ���B
			}
		}
		break;
	}
	case WM_SIZE:
	case WM_WININICHANGE:
	{
		//NG�c�[�I���g�̃T�C�Y��ύX����R�[�h������Ɩ������[�v����I�I
		//int i = 0;
		//i = GetSubClkIndexFromHWND(hwnd);
		//if ((i != 999) && bEnableSpecificSubClk[i]) {
		//	SetSpecificSubClock(i);
		//}
		break;
	}
	case WM_CONTEXTMENU:
	{	// �E�N���b�N���j���[�B�Ȃ�Win11�ł͍�����܂������Ȃ����A�����OS�̂������ƍl������B
		PostMessage(hwndTClockExeMain, message, wParam, lParam);
		return 0;
	}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void ActivateSubClocks(void)
{
	if (b_DebugLog) writeDebugLog_Win10("[subclock.c]ActivateSubClocks called. ", 999);

	//�T�u�f�B�X�v���C��^�X�N�o�[���v�̃t�b�N
	FindAllSubClocks();

	//���C���^�X�N�o�[�̕������`�F�b�N
	g_bVertTaskbar = IsVertTaskbar(hwndTaskBarMain);

	//�T�u�^�X�N�o�[�̓��C���ƃT�u���������̏ꍇ�̂ݕ\������
	for (int i = 0; i < MAX_SUBSCREEN; i++) {
		if (hwndClockSubClk[i])
		{
			bEnableSpecificSubClk[i] = TRUE;
		}
		if (b_DebugLog) {
			writeDebugLog_Win10("[subclock.c][ActivateSubClocks] Sub Screen ID = ", i);
			writeDebugLog_Win10("[subclock.c][ActivateSubClocks] bEnableSpecificSubClk[i] = ", bEnableSpecificSubClk[i]);
		}
	}

	//�T�u�f�B�X�v���C�㎞�v�N�����_�̃T�C�Y��ۑ�
	GetOrigSubClkDimensions();

	//�T�u�f�B�X�v���C�㎞�v�̃T�C�Y�ƈʒu�ݒ�
	SetAllSubClocks();

	//�T�u�f�B�X�v���C�㎞�v�̃T�u�N���X��
	for (int i = 0; i < MAX_SUBSCREEN; i++) {
		if (bEnableSpecificSubClk[i])
		{
			oldWndProcSub[i] = (WNDPROC)GetWindowLongPtr(hwndClockSubClk[i], GWLP_WNDPROC);
			SubclassWindow(hwndClockSubClk[i], WndProcSubClk);
		}
	}


}

void StoreSpecificSubClockDimensions(int i)
{

	POINT tempPos = { 0, 0 };
	RECT tempRect;
	char tempClassName[32];
	HWND tempHwnd;

	GetWindowRect(hwndClockSubClk[i], &tempRect);
	origSubClockWidth[i] = tempRect.right - tempRect.left;
	origSubClockHeight[i] = tempRect.bottom - tempRect.top;

	GetWindowRect(hwndTaskBarSubClk[i], &tempRect);
	widthSubTaskbar[i] = tempRect.right - tempRect.left;
	heightSubTaskbar[i] = tempRect.bottom - tempRect.top;


	if (b_DebugLog) {
		writeDebugLog_Win10("[subclock.c][Screen Number = ", i);

		writeDebugLog_Win10("[subclock.c][StoreSpecificSubClockDimensions] origSubClockWidth[i] = ", origSubClockWidth[i]);
		writeDebugLog_Win10("[subclock.c][StoreSpecificSubClockDimensions] origSubClockHeight[i] = ", origSubClockHeight[i]);

		writeDebugLog_Win10("[subclock.c][StoreSpecificSubClockDimensions] widthSubTaskbar[i] = ", widthSubTaskbar[i]);
		writeDebugLog_Win10("[subclock.c][StoreSpecificSubClockDimensions] heightSubTaskbar[i] = ", heightSubTaskbar[i]);
	}
}

void GetOrigSubClkDimensions(void)
{

	for (int i = 0; i < MAX_SUBSCREEN; i++) {
		if (bEnableSpecificSubClk[i])
		{
			StoreSpecificSubClockDimensions(i);
		}
	}


}

int GetSubClkIndexFromHWND(HWND tempHwndSubClk)
{
	//	if (b_DebugLog) writeDebugLog_Win10("[subclock.c] GetSubClkIndexFromHWND called. ", 999);

	HWND tempHwnd;
	int ret = 999;

	for (int i = 0; i < MAX_SUBSCREEN; i++) {
		if (tempHwndSubClk == hwndClockSubClk[i])
		{
			ret = i;
			break;
		}
	}

	if (b_DebugLog) writeDebugLog_Win10("[subclock.c][SetSubClkIndexFromHWND] Index of SubClk = ", ret);

	return ret;
}

void CalcSpecificSubClockSize(int i)
{
	if (b_DebugLog) writeDebugLog_Win10("[subclock.c] CalcSpecificSubClockSize called. ", 999);

	RECT tempRect;

	GetWindowRect(hwndTaskBarSubClk[i], &tempRect);	//���̎��_��tempRect�ɂ͑ΏۃT�u�^�X�N�o�[�̏�񂪓����Ă���B
	widthSubTaskbar[i] = tempRect.right - tempRect.left;
	heightSubTaskbar[i] = tempRect.bottom - tempRect.top;

	if (IsVertTaskbar(hwndTaskBarSubClk[i])) {
		widthSubClock[i] = tempRect.right - tempRect.left;
		heightSubClock[i] = heightMainClockFrame * widthSubClock[i] / widthMainClockFrame;
		if ((heightSubClock[i] * 2) > heightSubTaskbar[i]) {
			heightSubClock[i] = heightSubTaskbar[i] / 2;
			widthSubClock[i] = heightSubClock[i] * widthMainClockFrame / heightMainClockFrame;
		}
	}
	else {
		heightSubClock[i] = tempRect.bottom - tempRect.top;
		widthSubClock[i] = widthMainClockFrame * heightSubClock[i] / heightMainClockFrame;
		if ((widthSubClock[i] * 2) > widthSubTaskbar[i]) {
			widthSubClock[i] = widthSubTaskbar[i] / 2;
			heightSubClock[i] = widthSubClock[i] * heightMainClockFrame / widthMainClockFrame;
		}
	}

	if (b_DebugLog) {
		writeDebugLog_Win10("[subclock.c][CalcSpecificSubClockSize] widthSubClock[i] = ", widthSubClock[i]);
		writeDebugLog_Win10("[subclock.c][CalcSpecificSubClockSize] heightSubClock[i] = ", heightSubClock[i]);
	}
}

//�T�u�N���b�N�o�[�ł́A��������o����Windows Ink���[�N�X�y�[�X�A�C�R���͖��������Ă��c���Ă��āAWorkerW�̌��ɉB��Ă��邾���̏ꍇ������B
//���C���o�[�ɂ��c�邪�A�A�C�R������0�ɂȂ�̂ŁA����Ŋm�F���邱�Ƃ��ł���B
BOOL GetInkWorkspaceSetting(void)
{
	BOOL ret = FALSE;
	HWND tempHwnd;
	RECT tempRect;

	tempHwnd = FindWindowEx(hwndTrayMain, NULL, "PenWorkspaceButton", NULL);
	if (tempHwnd)
	{
		GetWindowRect(tempHwnd, &tempRect);
		if ((tempRect.right != tempRect.left) && (tempRect.bottom != tempRect.top))
		{
			ret = TRUE;
		}
	}

	if (b_DebugLog)writeDebugLog_Win10("[subclock.c][GetInkWorkspaceSetting] Ink Workspace ON? = ", ret);

	return ret;
}

void SetSpecificSubClock(int i)
{
	if (b_DebugLog) writeDebugLog_Win10("[subclock.c] SetSpecificSubClock called. ", 999);

	if (!bEnableSpecificSubClk[i]) return;


	POINT pos, pos2, nextcorner;
	HWND tempHwnd;
	char tempClassName[32];
	RECT tempRect;
	BOOL tempIsVert;

	CalcSpecificSubClockSize(i);

	tempIsVert = IsVertTaskbar(hwndTaskBarSubClk[i]);

	if (tempIsVert) {
		nextcorner.x = widthSubTaskbar[i];
		nextcorner.y = heightSubTaskbar[i] - heightSubClock[i];
		SetWindowPos(hwndClockSubClk[i], NULL, 0, nextcorner.y, widthSubClock[i], heightSubClock[i],
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);
	}
	else {
		nextcorner.x = widthSubTaskbar[i] - widthSubClock[i];
		nextcorner.y = heightSubTaskbar[i];
		SetWindowPos(hwndClockSubClk[i], NULL, nextcorner.x, 0, widthSubClock[i], heightSubClock[i],
			SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSENDCHANGING);
	}

	//�T�u�N���b�N�`��}������������
	bClearSubClk[i] = FALSE;

	//Win11�ł͂����܂łŏ�������(Inkspace�{�^���͂Ȃ��AWorkerW�̃E�B���h�E�͍L�����Ă��Ă��Ȃ�)

	tempHwnd = hwndClockSubClk[i];
	pos.x = 0;
	pos.y = 0;
	MapWindowPoints(tempHwnd, hwndTaskBarSubClk[i], &pos, 1);

	if (bWin11Sub)
	{
		tempHwnd = NULL;
		for (int j = 0; j < 2; j++) {
			tempHwnd = FindWindowEx(hwndTaskBarSubClk[i], tempHwnd, "Windows.UI.Composition.DesktopWindowContentBridge", NULL);
			if (tempHwnd)
			{
				pos2.x = 0;
				pos2.y = 0;
				MapWindowPoints(tempHwnd, hwndTaskBarSubClk[i], &pos2, 1);
				if (pos2.x == 0) {	//�S�̑��̃N���X�̏ꍇ
					SetWindowPos(tempHwnd, NULL, 0, 0, pos.x, heightSubClock[i],
						SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING);
				}
				else {	//Win11�̃T�u�N���b�N���v�N���X�̏ꍇ
					ShowWindow(tempHwnd, SW_HIDE);
				}
			}
		}
	}
	else
	{
		tempHwnd = FindWindowEx(hwndTaskBarSubClk[i], NULL, "PenWorkspaceButton", NULL);
		if (tempHwnd)
		{
			//�T�u�N���b�N�o�[�ł́A��������o����Windows Ink���[�N�X�y�[�X�A�C�R���͖��������Ă��c���Ă��āAWorkerW�̌��ɉB��Ă��邾���̏ꍇ������B
			//���C���o�[�ɂ��A�C�R���͎c�邪�A��0�ɂȂ�̂ŁA����Ŋm�F���邱�Ƃ��ł���(GetInkWorkspaceSetting)�B
			GetWindowRect(tempHwnd, &tempRect);
			if (tempIsVert) {
				SetWindowPos(tempHwnd, NULL, 0, nextcorner.y - (tempRect.bottom - tempRect.top), 0, 0,
					SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING);
				if (GetInkWorkspaceSetting()) {
					nextcorner.y -= (tempRect.bottom - tempRect.top);					//��������Ȃ����WorkerW���d�Ȃ��Ă����B
				}
			}
			else {
				SetWindowPos(tempHwnd, NULL, nextcorner.x - (tempRect.right - tempRect.left), 0, 0, 0,
					SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING);
				if (GetInkWorkspaceSetting()) {
					nextcorner.x -= (tempRect.right - tempRect.left);					//����������Ȃ����WorkerW���d�Ȃ��Ă����B
				}
			}
		}

		tempHwnd = FindWindowEx(hwndTaskBarSubClk[i], NULL, "WorkerW", NULL);
		if (tempHwnd)
		{
			pos.x = 0;
			pos.y = 0;
			MapWindowPoints(tempHwnd, hwndTaskBarSubClk[i], &pos, 1);
			SetWindowPos(tempHwnd, NULL, pos.x, pos.y, (nextcorner.x - pos.x), (nextcorner.y - pos.y),
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING);	//SWP_NOMOVE�t���O���Ȃ̂Ŏ��ۂɂ͈ʒu���͔��f����Ă��Ȃ�
		}
	}
}

void SetAllSubClocks(void) {
	if (b_DebugLog) writeDebugLog_Win10("[subclock.c] SetAllSubClocks called. ", 999);

	for (int i = 0; i < MAX_SUBSCREEN; i++) {
		if (bEnableSpecificSubClk[i])SetSpecificSubClock(i);
	}
}

void DelayedUpdateSubClks(void)
{
	if (b_DebugLog)writeDebugLog_Win10("[subclock.c] DelayedUpdateSubClks called.", 999);
	SetTimer(hwndClockMain, IDTIMERDLL_UPDATESUBCLKS, 500, NULL);
	bTimerSubClks = TRUE;
}

void CheckSubClocks(void)
{

	//���I�ȃT�u�N���b�N���o

	if (b_DebugLog) {
		writeDebugLog_Win10("[subclock.c] CheckSubClocks called.", 999);
	}

	if (!bEnableSubClks) return;	//�_�u���`�F�b�N

	int i, tempIndex;
	HWND tempHwndSubTaskbar = NULL;
	HWND tempHwndSubClk = NULL;

	//�����T�u���v�̃`�F�b�N
	for (i = 0; i < MAX_SUBSCREEN; i++)
	{
		if (!hwndClockSubClk[i] && bEnableSpecificSubClk[i])
		{
			DisableSpecificSubClock(i);
		}
	}


	while ((tempHwndSubTaskbar = FindWindowEx(NULL, tempHwndSubTaskbar, "Shell_SecondaryTrayWnd", NULL)) != NULL)
	{
		tempHwndSubClk = NULL;
		if (bWin11Sub) {
			tempHwndSubClk = FindWindowEx(tempHwndSubTaskbar, NULL, "TClockSub", NULL);
			if (!tempHwndSubClk) {
				tempHwndSubClk = CreateWin11SubClock(tempHwndSubTaskbar);
			}
		}
		else
		{
			tempHwndSubClk = FindWindowEx(tempHwndSubTaskbar, NULL, "ClockButton", NULL);
		}

		if (tempHwndSubClk)
		{
			tempIndex = GetSubClkIndexFromHWND(tempHwndSubClk);
			if (tempIndex == 999)
			{
				i = 0;
				while (bEnableSpecificSubClk[i]) {
					i++;
					if (i == MAX_SUBSCREEN) return;
				}						//��index��������

				hwndTaskBarSubClk[i] = tempHwndSubTaskbar;
				hwndClockSubClk[i] = tempHwndSubClk;

				oldWndProcSub[i] = (WNDPROC)GetWindowLongPtr(hwndClockSubClk[i], GWLP_WNDPROC);
				SubclassWindow(hwndClockSubClk[i], WndProcSubClk);
				bEnableSpecificSubClk[i] = TRUE;

				if (b_DebugLog)
				{
					writeDebugLog_Win10("[subclock.c][CheckSubClocks] New Subclock was found and the provided Index = ", i);
					writeDebugLog_Win10("[subclock.c][CheckSubClocks] bEnableSpecificSubClk[i] = ", bEnableSpecificSubClk[i]);
				}
			}
			else {
				if (b_DebugLog)
				{
					writeDebugLog_Win10("[subclock.c][CheckSubClocks] Subclock reconfirmed, Index = ", tempIndex);
					writeDebugLog_Win10("[subclock.c][CheckSubClocks] bEnableSpecificSubClk[i] = ", bEnableSpecificSubClk[tempIndex]);
				}
			}
		}
	}
}

void FindAllSubClocks(void)
{
	//�ŏ��̃T�u�N���b�N���o

	for (int i = 0; i < MAX_SUBSCREEN; i++) {
		hwndTaskBarSubClk[i] = NULL;
		hwndClockSubClk[i] = NULL;
		bEnableSpecificSubClk[i] = FALSE;
		bClearSubClk[i] = FALSE;
	}

	if (!bEnableSubClks) return;	//�_�u���`�F�b�N

	for (int i = 0; i < MAX_SUBSCREEN; i++) {
		if (i == 0)
		{
			hwndTaskBarSubClk[0] = FindWindowEx(NULL, NULL, "Shell_SecondaryTrayWnd", NULL);
		}
		else
		{
			hwndTaskBarSubClk[i] = FindWindowEx(NULL, hwndTaskBarSubClk[i - 1], "Shell_SecondaryTrayWnd", NULL);
		}

		if (hwndTaskBarSubClk[i])
		{
			//// find the secondary clock window
			//hwndClockSubClk[i] = FindWindowEx(hwndTaskBarSubClk[i], NULL, "ClockButton", NULL);
			//if (!hwndClockSubClk[i])hwndClockSubClk[i] = CreateWin11SubClock(hwndTaskBarSubClk[i]);	//Win11���ƍl������̂ō쐬����B

			// Ver 4.2.1�ȍ~ find or create the secondary clock window
			if (!bWin11Main) {
				hwndClockSubClk[i] = FindWindowEx(hwndTaskBarSubClk[i], NULL, "ClockButton", NULL);
			}
			else {
				hwndClockSubClk[i] = CreateWin11SubClock(hwndTaskBarSubClk[i]);
			}

			if (hwndClockSubClk[i])
			{
				//bSubClockFound[i] = TRUE;
				if (b_DebugLog) writeDebugLog_Win10("[subclock.c][FindAllSubClocks] Clock Found on Sub Screen ID; ", i);
			}
		}
		else {
			if (b_DebugLog) {
				writeDebugLog_Win10("[subclock.c][FindAllSubClocks] Number of SubClks = ", i);
			}
			break;
		}
	}
}

void DisableSpecificSubClock(int i) {
	char tempClassName[32];
	HWND tempHwnd;

	POINT pos, pos2, nextcorner;
	RECT tempRect;

	if (b_DebugLog) {
		writeDebugLog_Win10("[subclock.c]DisableSpecificSubClock called for screen:", i);
	}

	//�T�u�N���X������:������ŏ��ɂ���Ă����Ȃ��ƁA�T�u���v�̃T�C�Y��߂����̂ɔ�������SetSpecificSubClock���Ă΂�ăT�C�Y���傫���Ȃ��Ă��܂��I
	if (bEnableSpecificSubClk[i] && hwndClockSubClk[i] && oldWndProcSub[i])
	{
		SubclassWindow(hwndClockSubClk[i], oldWndProcSub[i]);
	}
	oldWndProcSub[i] = NULL;

	CalcSpecificSubClockSize(i);	//widthSubTaskbar[i], heightSubTaskbar[i]�������~����


	if (bWin11Sub) {
		tempHwnd = NULL;
		for (int j = 0; j < 2; j++) {
			tempHwnd = FindWindowEx(hwndTaskBarSubClk[i], tempHwnd, "Windows.UI.Composition.DesktopWindowContentBridge", NULL);
			if (tempHwnd)
			{
				pos2.x = 0;
				pos2.y = 0;
				MapWindowPoints(tempHwnd, hwndTaskBarSubClk[i], &pos2, 1);
				if (pos2.x == 0) {	//�S�̑��̃N���X�̏ꍇ
					SetWindowPos(tempHwnd, NULL, 0, 0, widthSubTaskbar[i], heightSubTaskbar[i],
						SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING);
				}
				else {	//Win11�̃T�u�N���b�N���v�N���X�̏ꍇ
					ShowWindow(tempHwnd, SW_SHOW);
				}
			}
		}
	}
	else {
		if (g_bVertTaskbar) {
			nextcorner.x = widthSubTaskbar[i];
			nextcorner.y = heightSubTaskbar[i] - origSubClockHeight[i];

			SetWindowPos(hwndClockSubClk[i], NULL, 0, nextcorner.y, origSubClockWidth[i], origSubClockHeight[i],
				SWP_NOACTIVATE | SWP_NOZORDER);
		}
		else {
			nextcorner.x = widthSubTaskbar[i] - origSubClockWidth[i];
			nextcorner.y = heightSubTaskbar[i];

			SetWindowPos(hwndClockSubClk[i], NULL, nextcorner.x, 0, origSubClockWidth[i], origSubClockHeight[i],
				SWP_NOACTIVATE | SWP_NOZORDER);
		}

		//InkSpace�{�^���̈ʒu��߂�(Win10�̂�)
		tempHwnd = FindWindowEx(hwndTaskBarSubClk[i], NULL, "PenWorkspaceButton", NULL);
		if (tempHwnd)
		{
			if (b_DebugLog)writeDebugLog_Win10("[subclock.c][DisableSpecificSubClock] PenWorkspaceButton is recovered", 999);
			GetWindowRect(tempHwnd, &tempRect);
			if (g_bVertTaskbar) {
				SetWindowPos(tempHwnd, NULL, 0, nextcorner.y - (tempRect.bottom + tempRect.top), 0, 0,
					SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
				if (GetInkWorkspaceSetting()) {
					nextcorner.y -= (tempRect.bottom + tempRect.top);			//�������Ȃ���Ύ��̏�����WorkerW���d�Ȃ��Ă����B
				}
			}
			else {
				SetWindowPos(tempHwnd, NULL, nextcorner.x - (tempRect.right - tempRect.left), 0, 0, 0,
					SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
				if (GetInkWorkspaceSetting()) {
					nextcorner.x -= (tempRect.right - tempRect.left);			//�������Ȃ���Ύ��̏�����WorkerW���d�Ȃ��Ă����B
				}
			}
		}

		//�A�v���A�C�R���̈�̗̈敝��߂�
		tempHwnd = FindWindowEx(hwndTaskBarSubClk[i], NULL, "WorkerW", NULL);
		if (tempHwnd)
		{
			pos.x = 0;
			pos.y = 0;
			MapWindowPoints(tempHwnd, hwndTaskBarSubClk[i], &pos, 1);
			SetWindowPos(tempHwnd, NULL, pos.x, pos.y, (nextcorner.x - pos.x), (nextcorner.y - pos.y),
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);	//SWP_NOMOVE�t���O���Ȃ̂Ŏ��ۂɂ͈ʒu���͔��f����Ă��Ȃ�
		}
	}

	if (hwndClockSubClk[i]) {
		if (bWin11Sub)
		{
			//Win11�̏ꍇ�B�T�u�N���b�N�E�B���h�E���폜����B
			ClearSpecificSubClock(i);
//			ShowWindow(hwndClockSubClk[i], SW_HIDE);		//�ǂ������킯���A���ꂪ�����Ȃ��̂ŏ�̃N���A�֐����g���B
			PostMessage(hwndClockSubClk[i], WM_CLOSE, 0, 0);
		}
		else
		{
			//Win10�̏ꍇ�B�T�u�N���b�N�͌��ɖ߂����E�B���h�E�v���V�[�W���ɂ���𑗂��Ă����Ȃ��ƍĕ`�悳��Ȃ��B
			PostMessage(hwndClockSubClk[i], WM_SIZE, SIZE_RESTORED, 0);
		}
	}
	hwndClockSubClk[i] = NULL;

	// refresh the taskbar
	if (hwndTaskBarSubClk[i])
	{
		PostMessage(hwndTaskBarSubClk[i], WM_SIZE, SIZE_RESTORED, 0);
		InvalidateRect(hwndTaskBarSubClk[i], NULL, TRUE);
	}
	hwndTaskBarSubClk[i] = NULL;

	bEnableSpecificSubClk[i] = FALSE;
}

void DisableAllSubClocks(void)
{
	for (int i = 0; i < MAX_SUBSCREEN; i++) {
		if (bEnableSpecificSubClk[i])
		{
			DisableSpecificSubClock(i);
		}
	}

	if (bWin11Sub)		//�S���������I�������N���X�o�^���폜����B
	{
		UnregisterClass("TClockSub", hmod);
		bWin11Sub = FALSE;
	}

	bEnableSubClks = FALSE;
}


void ClearSpecificSubClock(int i)
{
	HDC hdcSub = NULL;
	hdcSub = GetDC(hwndClockSubClk[i]);		//�T�u�f�B�X�v���C�̎��v�����݂����hdcSub�����݂��邱�ƂɂȂ�B
	if (hdcSub != NULL)
	{
		PatBlt(hdcSub, 0, 0, widthSubClock[i], heightSubClock[i], BLACKNESS);
	}
}


