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
extern HWND hwndOriginalWin11SubClk[];
extern BOOL bEnableSpecificSubClk[];
extern BOOL bSuppressUpdateSubClk[];
extern BOOL bEnableSubClks;
extern int	heightSubClock[];
extern int	widthSubClock[];
extern int	origSubClockWidth[];
extern int	origSubClockHeight[];
extern int	widthSubTaskbar[];
extern int	heightSubTaskbar[];

extern BOOL bEnableTooltip;

BOOL bWmPaintRecevied = FALSE;
BOOL bWmWinPosChangingRecevied = FALSE;
BOOL bSubClkRecovering = FALSE;


/*------------------------------------------------
subclass procedure of the Sub Display clocks , 20211107 TTTT
--------------------------------------------------*/
LRESULT CALLBACK WndProcSubClk(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//この関数はサブウィンドウの時計(hwndSubClk[i])のプロシージャとして登録しているので、hwndはhwndSubClk[i]のいずれかになる。
	//GetSubClkIndexFromHWNDを使ってiを調べる必要がある。hwndはhwndClockMainではないので注意する。

//	if (b_DebugLog) writeDebugLog_Win10("[subclock.c][WndProcSubClk] Window Message was recevied, message = ", message);

	switch (message) {
	case (WM_USER + 100):
		// 親ウィンドウから送られ、サイズを返すメッセージ。
		// (Win10RS1(=AU)以降は)ではメインクロックにもサブクロックにも、サイズ調整が必要な場合には、このメッセージは来ない。
		break;
		//case (WM_NOTIFY):
	case (WM_NCCALCSIZE):
		//サブタスクバーに変更があるとこれが届く、SetAllSubClocksの後にも届くので、これが来たらタスクバーが強制的に
		//戻された、と判断することはできない。デバッグコードにあるような複雑なlparamの中にデータが入っている。
		//rgrc[0]が変更されようとするサイズなので、これがサブクロックのサイズと違えば、Windows標準時計に戻されようとしていることがわかる。
		//その場合はrgrc[0]の値をorigSubClockのサイズとして更新すれば、新しいタスクバーでWindows標準時計サイズがわかる。
		//そのうえで、戻り値に0(容認という意味？)ではなく、WVR _VALIDRECTS(=0x0400)を返したら更新されてしまわずに済む?
		//http://blog.livedoor.jp/oans/archives/50628113.html

		//ここでStoreSpecificSubClockDimensionsを呼んでは行けない。サイズがまだ変更されていないため正しい値を取得することはできない。

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

				//新しいタスクバーサイズなどは保存できないが、うまく動くのでまあいいか、というところ。
			}
		}

		break;
	}
	case WM_SIZE:
	case WM_WININICHANGE:
	{
		//NG残骸！自身のサイズを変更するコードを入れると無限ループする！！
		//int i = 0;
		//i = GetSubClkIndexFromHWND(hwnd);
		//if ((i != 999) && bEnableSpecificSubClk[i]) {
		//	SetSpecificSubClock(i);
		//}
		break;
	}
	case WM_PAINT:	// =15, タスクバーを隠す、から表示が起こると、このほか70, 20, 15, が1秒以内に連続して入ってくる(Win10のみ、Win11では来ない？)
						//隠していたタスクバーが出てくるときは最後にWM_PAINT(15)が2回連続届く。隠れるときは最後にWM_NCPAINT(70)->WM_PAINT(15)になる。
						//Win10ではサブ時計のサイズが変更されてしまう。Win11ではサイズ変更は行われないが元の時計を隠していたのが表示されてしまう。
						//このメッセージが100ms以内に2回来たらサブタスクバー再表示と判定して、時間差でサイズ調整を実施する。
						//このサブ時計プロシージャは共通なので、Win10のみで実行されるように実装する。
	{
		if (bWmPaintRecevied)
		{
			bSubClkRecovering = FALSE;
			if (b_DebugLog) writeDebugLog_Win10("[subclock.c][WndProcSubClk] Hidden SubTaskBar recovered. Index = ", GetSubClkIndexFromHWND(hwnd));
			//出現完了したらすみやかに処理を実施する。(Delayは不要)
			if (!bWin11Sub) {
				int i = GetSubClkIndexFromHWND(hwnd);
				if ((i != 999) && bEnableSpecificSubClk[i])
				{
//					bSuppressUpdateSubClk[i] = FALSE;		//SetSpecificSubClock()で描画抑制解除されるので不要
					SetSpecificSubClock(i);
					RedrawTClock();
				}
			}		
		}
		if (bWmWinPosChangingRecevied)
		{
			bSubClkRecovering = FALSE;
			if (b_DebugLog) writeDebugLog_Win10("[subclock.c][WndProcSubClk] SubTaskBar was hidden. Index = ", GetSubClkIndexFromHWND(hwnd));
		}
		bWmPaintRecevied = TRUE;
		bWmWinPosChangingRecevied = FALSE;
		break;
	}
	case WM_CONTEXTMENU:
	{	// 右クリックメニュー。なおWin11では今一つうまく消えないが、それはOSのせいだと考えられる。
		PostMessage(hwndTClockExeMain, message, wParam, lParam);
		return 0;
	}
	//ツールチップ処理導入準備(MainClockのWndProcのコード)
	case WM_MOUSEMOVE:
		//if (b_DebugLog) {
		//	writeDebugLog_Win10("[subclock.c][WndProcSubClk] WM_MOUSEMOVE Received", 999);
		//}
		if (bEnableTooltip) {
			TooltipOnMouseEvent(hwnd, message, wParam, lParam, BASE_UID_SUBSCREEN + GetSubClkIndexFromHWND(hwnd));
		}
		return 0;
	case WM_NOTIFY:
		//if (b_DebugLog) {
		//	writeDebugLog_Win10("[subclock.c][WndProcSubClk] WM_NOTIFY Received", 999);
		//}
		{
			LRESULT lres;
			if (TooltipOnNotify(&lres, lParam)) return lres;
			break;
		}
	case WM_WINDOWPOSCHANGING:		// =70, これが2回連続する。
	{
		if (bWmWinPosChangingRecevied)
		{
			if (!bSubClkRecovering)
			{
				if (b_DebugLog) writeDebugLog_Win10("[subclock.c][WndProcSubClk] Hidden SubTaskBar recovering /hiding started. Index = ", GetSubClkIndexFromHWND(hwnd));
				bSubClkRecovering = TRUE;
				//隠されたタスクバーの表示/隠すプロセスの開始時
				//ここにサブクロック配置修正の処理を入れても、残念ながら出現中のプロセスで上書きされてしまう。WM_PAINT(15)の連続で出現完了を判定して処理するしかない。
				//表示をクリア(全部は消えない)して更新抑制することで、再表示時に半端な表示が見えないようにする。
				//なお、ここでWindowsのVisibilityを帰るとウィンドウメッセージが受け取れなくなるので処理ができなくなる。
				if (!bWin11Sub)
				{
					int i = GetSubClkIndexFromHWND(hwnd);
					if ((i != 999) && bEnableSpecificSubClk[i])
					{
						ClearSpecificSubClock(i);
						bSuppressUpdateSubClk[i] = TRUE;
					}
				}
			}
		}
		bWmPaintRecevied = FALSE;	//隠れたタスクバーが現れる場合の最後をWM_PAINT(15)の連続で判定するためのフラグ下げ
		bWmWinPosChangingRecevied = TRUE;	//タスクバーが隠れた場合の最後をWM_PAINT(70)への連続で判定するためのフラグ上げ
		break;
	}
	case WM_NCPAINT:
	case WM_ERASEBKGND:
	{
		bWmPaintRecevied = FALSE;
		bWmWinPosChangingRecevied = FALSE;
		break;
	}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}




void ActivateSubClocks(void)
{
	if (b_DebugLog) writeDebugLog_Win10("[subclock.c]ActivateSubClocks called. ", 999);

	//サブディスプレイ上タスクバー時計のフック
	FindAllSubClocks();

	//メインタスクバーの方向をチェック
	g_bVertTaskbar = IsVertTaskbar(hwndTaskBarMain);

	//サブタスクバーはメインとサブが両方横の場合のみ表示する
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

	//サブディスプレイ上時計起動時点のサイズを保存
	GetOrigSubClkDimensions();

	//サブディスプレイ上時計のサイズと位置設定
	SetAllSubClocks();

	//サブディスプレイ上時計のサブクラス化
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

	GetWindowRect(hwndTaskBarSubClk[i], &tempRect);	//この時点でtempRectには対象サブタスクバーの情報が入っている。
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

//サブクロックバーでは、いったん出したWindows Inkワークスペースアイコンは無効化しても残っていて、WorkerWの後ろに隠れているだけの場合がある。
//メインバーにも残るが、アイコン幅が0になるので、それで確認することができる。
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

	//ツールチップ連携開始(width, heightが必要なのでここで実行する)
	TooltipAddSubClock(i);

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

	//サブクロック描画抑制を解除する
	bSuppressUpdateSubClk[i] = FALSE;

	tempHwnd = hwndClockSubClk[i];
	pos.x = 0;
	pos.y = 0;
	MapWindowPoints(tempHwnd, hwndTaskBarSubClk[i], &pos, 1);

	if (bWin11Sub)
	{
		tempHwnd = NULL;
		hwndOriginalWin11SubClk[i] = NULL;
		for (int j = 0; j < 2; j++) {
			tempHwnd = FindWindowEx(hwndTaskBarSubClk[i], tempHwnd, "Windows.UI.Composition.DesktopWindowContentBridge", NULL);
			if (tempHwnd)
			{
				pos2.x = 0;
				pos2.y = 0;
				MapWindowPoints(tempHwnd, hwndTaskBarSubClk[i], &pos2, 1);
				if (pos2.x == 0) {	//全体側のクラスの場合
					SetWindowPos(tempHwnd, NULL, 0, 0, pos.x, heightSubClock[i],
						SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING);
				}
				else {	//Win11のサブクロック時計クラスの場合
					hwndOriginalWin11SubClk[i] = tempHwnd;	//オリジナル時計のHWNDを更新
					//DWORD dwStyle = (DWORD)GetWindowLong(hwndOriginalWin11SubClk[i], GWL_STYLE);
					//if ((dwStyle & WS_VISIBLE) != 0)
					//{
					//	SetWindowLongPtr(hwndOriginalWin11SubClk[i], GWL_STYLE, dwStyle & ~WS_VISIBLE);
					//}
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
			//サブクロックバーでは、いったん出したWindows Inkワークスペースアイコンは無効化しても残っていて、WorkerWの後ろに隠れているだけの場合がある。
			//メインバーにもアイコンは残るが、幅0になるので、それで確認することができる(GetInkWorkspaceSetting)。
			GetWindowRect(tempHwnd, &tempRect);
			if (tempIsVert) {
				SetWindowPos(tempHwnd, NULL, 0, nextcorner.y - (tempRect.bottom - tempRect.top), 0, 0,
					SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING);
				if (GetInkWorkspaceSetting()) {
					nextcorner.y -= (tempRect.bottom - tempRect.top);					//これをしなければWorkerWが重なってくれる。
				}
			}
			else {
				SetWindowPos(tempHwnd, NULL, nextcorner.x - (tempRect.right - tempRect.left), 0, 0, 0,
					SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOSENDCHANGING);
				if (GetInkWorkspaceSetting()) {
					nextcorner.x -= (tempRect.right - tempRect.left);					//ここれをしなければWorkerWが重なってくれる。
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
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING);	//SWP_NOMOVEフラグつきなので実際には位置情報は反映されていない
		}
	}
}

void SetAllSubClocks(void) {
	if (b_DebugLog) writeDebugLog_Win10("[subclock.c] SetAllSubClocks called. ", 999);

	for (int i = 0; i < MAX_SUBSCREEN; i++) {
		if (bEnableSpecificSubClk[i])SetSpecificSubClock(i);
	}
}



void CheckSubClocks(void)
{

	//動的なサブクロック検出

	if (b_DebugLog) {
		writeDebugLog_Win10("[subclock.c] CheckSubClocks called.", 999);
	}

	if (!bEnableSubClks) return;	//ダブルチェック

	int i, tempIndex;
	HWND tempHwndSubTaskbar = NULL;
	HWND tempHwndSubClk = NULL;



	//既存サブ時計のチェック
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
				}						//空きindexを見つける

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
	//最初のサブクロック検出

	for (int i = 0; i < MAX_SUBSCREEN; i++) {
		hwndTaskBarSubClk[i] = NULL;
		hwndClockSubClk[i] = NULL;
		bEnableSpecificSubClk[i] = FALSE;
		bSuppressUpdateSubClk[i] = FALSE;
	}

	if (!bEnableSubClks) return;	//ダブルチェック

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
			//if (!hwndClockSubClk[i])hwndClockSubClk[i] = CreateWin11SubClock(hwndTaskBarSubClk[i]);	//Win11だと考えられるので作成する。

			// Ver 4.2.1以降 find or create the secondary clock window
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

	//ツールチップ連携解除
	TooltipRemoveSubClock(i);

	//サブクラス化解除:これを最初にやっておかないと、サブ時計のサイズを戻したのに反応してSetSpecificSubClockが呼ばれてサイズが大きくなってしまう！
	if (bEnableSpecificSubClk[i] && hwndClockSubClk[i] && oldWndProcSub[i])
	{
		SubclassWindow(hwndClockSubClk[i], oldWndProcSub[i]);
	}
	oldWndProcSub[i] = NULL;

	CalcSpecificSubClockSize(i);	//widthSubTaskbar[i], heightSubTaskbar[i]だけが欲しい


	if (bWin11Sub) {
		tempHwnd = NULL;
		for (int j = 0; j < 2; j++) {
			tempHwnd = FindWindowEx(hwndTaskBarSubClk[i], tempHwnd, "Windows.UI.Composition.DesktopWindowContentBridge", NULL);
			if (tempHwnd)
			{
				pos2.x = 0;
				pos2.y = 0;
				MapWindowPoints(tempHwnd, hwndTaskBarSubClk[i], &pos2, 1);
				if (pos2.x == 0) {	//全体側のクラスの場合
					SetWindowPos(tempHwnd, NULL, 0, 0, widthSubTaskbar[i], heightSubTaskbar[i],
						SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING);
				}
				else {	//Win11のサブクロック時計クラスの場合
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

		//InkSpaceボタンの位置を戻す(Win10のみ)
		tempHwnd = FindWindowEx(hwndTaskBarSubClk[i], NULL, "PenWorkspaceButton", NULL);
		if (tempHwnd)
		{
			if (b_DebugLog)writeDebugLog_Win10("[subclock.c][DisableSpecificSubClock] PenWorkspaceButton is recovered", 999);
			GetWindowRect(tempHwnd, &tempRect);
			if (g_bVertTaskbar) {
				SetWindowPos(tempHwnd, NULL, 0, nextcorner.y - (tempRect.bottom + tempRect.top), 0, 0,
					SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
				if (GetInkWorkspaceSetting()) {
					nextcorner.y -= (tempRect.bottom + tempRect.top);			//こうしなければ次の処理でWorkerWが重なってくれる。
				}
			}
			else {
				SetWindowPos(tempHwnd, NULL, nextcorner.x - (tempRect.right - tempRect.left), 0, 0, 0,
					SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
				if (GetInkWorkspaceSetting()) {
					nextcorner.x -= (tempRect.right - tempRect.left);			//こうしなければ次の処理でWorkerWが重なってくれる。
				}
			}
		}

		//アプリアイコン領域の領域幅を戻す
		tempHwnd = FindWindowEx(hwndTaskBarSubClk[i], NULL, "WorkerW", NULL);
		if (tempHwnd)
		{
			pos.x = 0;
			pos.y = 0;
			MapWindowPoints(tempHwnd, hwndTaskBarSubClk[i], &pos, 1);
			SetWindowPos(tempHwnd, NULL, pos.x, pos.y, (nextcorner.x - pos.x), (nextcorner.y - pos.y),
				SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);	//SWP_NOMOVEフラグつきなので実際には位置情報は反映されていない
		}
	}

	if (hwndClockSubClk[i]) {
		if (bWin11Sub)
		{
			//Win11の場合。サブクロックウィンドウを削除する。
			ClearSpecificSubClock(i);
			PostMessage(hwndClockSubClk[i], WM_CLOSE, 0, 0);
		}
		else
		{
			//Win10の場合。サブクロックは元に戻したウィンドウプロシージャにこれを送っておかないと再描画されない。
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

	if (bWin11Sub)		//全部処理が終わったらクラス登録を削除する。
	{
		UnregisterClass("TClockSub", hmod);
		bWin11Sub = FALSE;
	}

	bEnableSubClks = FALSE;
}


void ClearSpecificSubClock(int i)
{
	HDC hdcSub = NULL;
	hdcSub = GetDC(hwndClockSubClk[i]);		//サブディスプレイの時計が存在するとhdcSubが存在することになる。
	if (hdcSub != NULL)
	{
		PatBlt(hdcSub, 0, 0, widthSubClock[i], heightSubClock[i], BLACKNESS);
	}
}



