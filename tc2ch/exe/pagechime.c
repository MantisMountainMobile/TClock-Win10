/*-------------------------------------------
pagechime.c
"Chime" page
add by Mantis Mountain Mobile (=MMM, TTTT)
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnBrowseChimeWavFile(HWND hDlg, BOOL bSecondary);
static void OnPlayWav(HWND hDlg, BOOL bSecondary);
static void OnStopWav(HWND hDlg);
static void OnChangeHours(HWND hDlg);
static void OnChangeEnable(HWND hDlg);


//extern int confNo;

extern BOOL b_EnglishMenu;
extern int Language_Offset;

char fname[MAX_PATH + 1];
char fname2[MAX_PATH + 1];
int hStart, hEnd;
BOOL b_suppress_response = FALSE;

__inline void SendPSChanged(HWND hDlg)
{
	g_bApplyClock = TRUE;
	SendMessage(GetParent(hDlg), PSM_CHANGED, (WPARAM)(hDlg), 0);
}

/*------------------------------------------------
  Dialog procedure
--------------------------------------------------*/
BOOL CALLBACK PageChimeProc(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			switch(id)
			{
			case IDC_EDIT_CHIME_START:
			case IDC_EDIT_CHIME_END:
			case IDC_SPIN_CHIME_START:
			case IDC_SPIN_CHIME_END:
				if (code == EN_CHANGE)
				{
					SendPSChanged(hDlg);
					if (!b_suppress_response)OnChangeHours(hDlg);
					break;
				}
			case IDC_CHECK_ENABLE_CHIME:
				if (code == BN_CLICKED)
				{
					SendPSChanged(hDlg);
					if (!b_suppress_response)OnChangeEnable(hDlg);
					break;
				}
			case IDC_CHECK_ENABLE_CHIME_BLINK:
			case IDC_CHECK_ENABLE_CUCKOOCLOCK:
				if (code == BN_CLICKED)
				{
					SendPSChanged(hDlg);
					break;
				}
			case IDC_CHIME_WAV:
			case IDC_EDIT_CHIME_OFFSET_S:
			case IDC_SPIN_CHIME_OFFSET_S:
			case IDC_EDIT_CHIME_BLINK:
			case IDC_SPIN_CHIME_BLINK:
			case IDC_CHIME_WAV2:
			case IDC_EDIT_CHIME_OFFSET_S2:
			case IDC_SPIN_CHIME_OFFSET_S2:
				if(code == EN_CHANGE)
					SendPSChanged(hDlg);
				break;
			case IDC_CHIME_WAV_BROWSE:
				OnBrowseChimeWavFile(hDlg, FALSE);
				break;
			case IDC_CHIME_WAV_BROWSE2:
				OnBrowseChimeWavFile(hDlg, TRUE);
				break;
			case IDC_PLAY_WAV:
				OnPlayWav(hDlg, FALSE);
				break;
			case IDC_PLAY_WAV2:
				OnPlayWav(hDlg, TRUE);
				break;
			case IDC_PLAY_WAV_STOP:
			case IDC_PLAY_WAV_STOP2:
				OnStopWav(hDlg);
				break;
			}
			return TRUE;
		}
		case WM_NOTIFY:
			switch(((NMHDR *)lParam)->code)
			{
				case PSN_APPLY: OnApply(hDlg); break;
				case PSN_HELP: My2chHelp(GetParent(hDlg)); break;
			}
			return TRUE;
		case WM_DESTROY:
			break;
	}
	return FALSE;
}

/*------------------------------------------------
  Initialize
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	HDC hdc;
	int index;

	b_suppress_response = TRUE;


	CheckDlgButton(hDlg, IDC_CHECK_ENABLE_CHIME,
		GetMyRegLong("Chime", "EnableChime", FALSE));
	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_OFFSET_S, UDM_SETRANGE, 0,
		MAKELONG(1800, -1800));
	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_OFFSET_S, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Chime", "OffsetChimeSec", 0));

	//à»â∫ÇÃçsÇÕOnChangeEnable()Ç≈ê›íËÇ≥ÇÍÇÈÅB
	//CheckDlgButton(hDlg, IDC_CHECK_ENABLE_CHIME2,
	//	GetMyRegLong("Chime", "EnableSecondaryChime", FALSE));
	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_OFFSET_S2, UDM_SETRANGE, 0,
		MAKELONG(1800, -1800));
	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_OFFSET_S2, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Chime", "OffsetSecondaryChimeSec", 0));

	CheckDlgButton(hDlg, IDC_CHECK_ENABLE_CHIME_BLINK,
		GetMyRegLong("Chime", "EnableBlinkOnChime", FALSE));

	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_BLINK, UDM_SETRANGE, 0,
		MAKELONG(10, 0));
	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_BLINK, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Chime", "BlinksOnChime", 3));

	GetMyRegStr("Chime", "ChimeWav", fname, MAX_PATH, "C:\\Windows\\Media\\notify.wav");
	SetDlgItemText(hDlg, IDC_CHIME_WAV, fname);

	GetMyRegStr("Chime", "SecondaryChimeWav", fname2, MAX_PATH, "C:\\Windows\\Media\\chimes.wav");
	SetDlgItemText(hDlg, IDC_CHIME_WAV2, fname2);

	hStart = (int)(short)GetMyRegLong("Chime", "ChimeHourStart", 0);
	hEnd = (int)(short)GetMyRegLong("Chime", "ChimeHourEnd", 24);

	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_START, UDM_SETRANGE, 0,
		MAKELONG(hEnd-1, 0));
	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_START, UDM_SETPOS, 0, hStart);


	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_END, UDM_SETRANGE, 0,
		MAKELONG(24, hStart + 1));
	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_END, UDM_SETPOS, 0, hEnd);

	CheckDlgButton(hDlg, IDC_CHECK_ENABLE_CUCKOOCLOCK,
		GetMyRegLong("Chime", "CuckooClock", FALSE));

	OnChangeEnable(hDlg);

	b_suppress_response = FALSE;

}

void OnChangeHours(HWND hDlg)
{
	hStart = SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_START, UDM_GETPOS, 0, 0);
	hEnd = SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_END, UDM_GETPOS, 0, 0);

	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_START, UDM_SETRANGE, 0,
		MAKELONG(hEnd - 1, 0));

	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_END, UDM_SETRANGE, 0,
		MAKELONG(24, hStart + 1));

}

void OnChangeEnable(HWND hDlg)
{
	if (IsDlgButtonChecked(hDlg, IDC_CHECK_ENABLE_CHIME)) {
		EnableDlgItem(hDlg, IDC_CHECK_ENABLE_CUCKOOCLOCK, TRUE);
		CheckDlgButton(hDlg, IDC_CHECK_ENABLE_CHIME2,
			GetMyRegLong("Chime", "EnableSecondaryChime", FALSE));
		EnableDlgItem(hDlg, IDC_CHECK_ENABLE_CHIME2, TRUE);
		EnableDlgItem(hDlg, IDC_CHIME_WAV2, TRUE);
		EnableDlgItem(hDlg, IDC_EDIT_CHIME_OFFSET_S2, TRUE);
		EnableDlgItem(hDlg, IDC_SPIN_CHIME_OFFSET_S2, TRUE);
		EnableDlgItem(hDlg, IDC_PLAY_WAV2, TRUE);
		EnableDlgItem(hDlg, IDC_PLAY_WAV_STOP2, TRUE);

	}else{
		EnableDlgItem(hDlg, IDC_CHECK_ENABLE_CUCKOOCLOCK, FALSE);
		CheckDlgButton(hDlg, IDC_CHECK_ENABLE_CHIME2, FALSE);
		EnableDlgItem(hDlg, IDC_CHECK_ENABLE_CHIME2, FALSE);
		EnableDlgItem(hDlg, IDC_CHIME_WAV2, FALSE);
		EnableDlgItem(hDlg, IDC_EDIT_CHIME_OFFSET_S2, FALSE);
		EnableDlgItem(hDlg, IDC_SPIN_CHIME_OFFSET_S2, FALSE);
		EnableDlgItem(hDlg, IDC_PLAY_WAV2, FALSE);
		EnableDlgItem(hDlg, IDC_PLAY_WAV_STOP2, FALSE);

	}
}

/*------------------------------------------------
  Apply
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	DWORD dw;


	SetMyRegLong("Chime", "EnableChime",
		IsDlgButtonChecked(hDlg, IDC_CHECK_ENABLE_CHIME));

	SetMyRegLong("Chime", "EnableSecondaryChime",
		IsDlgButtonChecked(hDlg, IDC_CHECK_ENABLE_CHIME2));

	SetMyRegLong("Chime", "EnableBlinkOnChime",
		IsDlgButtonChecked(hDlg, IDC_CHECK_ENABLE_CHIME_BLINK));

	//SetMyRegLong("Chime", "VolChime", intVolChime);

	SetMyRegLong("Chime", "OffsetChimeSec", 
		(short)SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_OFFSET_S, UDM_GETPOS, 0, 0));

	SetMyRegLong("Chime", "OffsetSecondaryChimeSec",
		(short)SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_OFFSET_S2, UDM_GETPOS, 0, 0));

	SetMyRegLong("Chime", "BlinksOnChime",
		(short)SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_BLINK, UDM_GETPOS, 0, 0));

	SetMyRegLong("Chime", "ChimeHourStart", hStart);
	SetMyRegLong("Chime", "ChimeHourEnd", hEnd);

	if (GetFileAttributes(fname) != 0xFFFFFFFF)
	{
		SetMyRegStr("Chime", "ChimeWav", fname);
	}

	if (GetFileAttributes(fname2) != 0xFFFFFFFF)
	{
		SetMyRegStr("Chime", "SecondaryChimeWav", fname2);
	}

	SetMyRegLong("Chime", "CuckooClock",
		IsDlgButtonChecked(hDlg, IDC_CHECK_ENABLE_CUCKOOCLOCK));

}

void OnBrowseChimeWavFile(HWND hDlg, BOOL bSecondary)
{
	char filter[80];
	char deffile[MAX_PATH];

	filter[0] = filter[1] = 0;
	str0cat(filter, MyString(IDS_WAVFILE));
	str0cat(filter, "*.wav");
	
	str0cat(filter, MyString(IDS_ALLFILE));
	str0cat(filter, "*.*");

	if (!bSecondary) {
		GetDlgItemText(hDlg, IDC_CHIME_WAV, deffile, MAX_PATH);
		if (!SelectMyFile(hDlg, filter, 0, deffile, fname)) {
			return;
		}

		SetDlgItemText(hDlg, IDC_CHIME_WAV, fname);
	}
	else {
		GetDlgItemText(hDlg, IDC_CHIME_WAV2, deffile, MAX_PATH);
		if (!SelectMyFile(hDlg, filter, 0, deffile, fname2)) {
			return;
		}

		SetDlgItemText(hDlg, IDC_CHIME_WAV2, fname2);
	}

	SendPSChanged(hDlg);
}



void OnPlayWav(HWND hDlg, BOOL bSecondary) 
{
	if (!bSecondary) {
		if (GetFileAttributes(fname) != 0xFFFFFFFF)
		{
			PlaySound(fname, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		}
	}
	else {
		if (GetFileAttributes(fname2) != 0xFFFFFFFF)
		{
			PlaySound(fname2, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
		}
	}
}


void OnStopWav(HWND hDlg)
{
	PlaySound(NULL, 0, 0);
}
