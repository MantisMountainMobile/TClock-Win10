/*-------------------------------------------
pagechime.c
"Chime" page
add by Mantis Mountain Mobile (=MMM, TTTT)
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnApply(HWND hDlg);
static void OnBrowseChimeWavFile(HWND hDlg);
static void OnPlayWav(HWND hDlg);
static void OnStopWav(HWND hDlg);
static void OnChangeHours(HWND hDlg);


//extern int confNo;

extern BOOL b_EnglishMenu;
extern int Language_Offset;

char fname[MAX_PATH + 1];
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
			case IDC_CHIME_WAV:
			//case IDC_EDIT_CHIME_VOL:
			//case IDC_SPIN_CHIME_VOL:
			case IDC_EDIT_CHIME_OFFSET_S:
			case IDC_SPIN_CHIME_OFFSET_S:
				if(code == EN_CHANGE)
					SendPSChanged(hDlg);
				break;
			case IDC_CHIME_WAV_BROWSE:
				OnBrowseChimeWavFile(hDlg);
				break;
			case IDC_PLAY_WAV:
				OnPlayWav(hDlg);
				break;
			case IDC_PLAY_WAV_STOP:
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

	//char fname[MAX_PATH + 1];

	//SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_VOL, UDM_SETRANGE, 0,
	//	MAKELONG(100, 1));
	//SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_VOL, UDM_SETPOS, 0,
	//	(int)(short)GetMyRegLong("Chime", "VolChime", 100));

	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_OFFSET_S, UDM_SETRANGE, 0,
		MAKELONG(1800, -1799));
	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_OFFSET_S, UDM_SETPOS, 0,
		(int)(short)GetMyRegLong("Chime", "OffsetChimeSec", 0));

	CheckDlgButton(hDlg, IDC_CHECK_ENABLE_CHIME,
		GetMyRegLong("Chime", "EnableChime", FALSE));

	GetMyRegStr("Chime", "ChimeWav", fname, MAX_PATH, "C:\\Windows\\Media\\notify.wav");
	SetDlgItemText(hDlg, IDC_CHIME_WAV, fname);

	hStart = (int)(short)GetMyRegLong("Chime", "ChimeHourStart", 0);
	hEnd = (int)(short)GetMyRegLong("Chime", "ChimeHourEnd", 24);

	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_START, UDM_SETRANGE, 0,
		MAKELONG(hEnd-1, 0));
	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_START, UDM_SETPOS, 0, hStart);


	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_END, UDM_SETRANGE, 0,
		MAKELONG(24, hStart + 1));
	SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_END, UDM_SETPOS, 0, hEnd);

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

/*------------------------------------------------
  Apply
--------------------------------------------------*/
void OnApply(HWND hDlg)
{
	DWORD dw;


	SetMyRegLong("Chime", "EnableChime",
		IsDlgButtonChecked(hDlg, IDC_CHECK_ENABLE_CHIME));

	//SetMyRegLong("Chime", "VolChime", intVolChime);

	SetMyRegLong("Chime", "OffsetChimeSec", 
		(short)SendDlgItemMessage(hDlg, IDC_SPIN_CHIME_OFFSET_S, UDM_GETPOS, 0, 0));

	SetMyRegLong("Chime", "ChimeHourStart", hStart);
	SetMyRegLong("Chime", "ChimeHourEnd", hEnd);

	if (GetFileAttributes(fname) != 0xFFFFFFFF)
	{
		SetMyRegStr("Chime", "ChimeWav", fname);
	}

}

void OnBrowseChimeWavFile(HWND hDlg)
{
	char filter[80];
	char deffile[MAX_PATH];

	filter[0] = filter[1] = 0;
	str0cat(filter, MyString(IDS_WAVFILE));
	str0cat(filter, "*.wav");
	
	str0cat(filter, MyString(IDS_ALLFILE));
	str0cat(filter, "*.*");

	GetDlgItemText(hDlg, IDC_CHIME_WAV, deffile, MAX_PATH);

	if (!SelectMyFile(hDlg, filter, 0, deffile, fname)) {
		return;
	}

	SetDlgItemText(hDlg, IDC_CHIME_WAV, fname);

	//PostMessage(hDlg, WM_NEXTDLGCTL, 1, FALSE);

	SendPSChanged(hDlg);
}



void OnPlayWav(HWND hDlg) 
{
	if (GetFileAttributes(fname) != 0xFFFFFFFF) 
	{
		PlaySound(fname, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
	}
}


void OnStopWav(HWND hDlg)
{
	PlaySound(NULL, 0, 0);
}
