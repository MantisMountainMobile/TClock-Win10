/*-------------------------------------------
  pagebarmeter_color.c
  "Barmeter Color" dialog
                MantisMountainMobile 2022
---------------------------------------------*/

#include "tclock.h"

static void OnInit(HWND hDlg);
static void OnOK(HWND hDlg);

extern BOOL b_EnglishMenu;
extern int Language_Offset;

extern int BarMeterColorDlgIndex;


static COMBOCOLOR combocolor[4] = {
	{ IDC_BARMETERCOL_0, "ColorBarMeterBL_High", RGB(255, 255, 255) },
	{ IDC_BARMETERCOL_1, "ColorBarMeterBL_Mid", RGB(255, 255, 255) },
	{ IDC_BARMETERCOL_2, "ColorBarMeterBL_Low", RGB(255, 255, 255) },
	{ IDC_BARMETERCOL_3, "ColorBarMeterBL_Charge", RGB(255, 255, 255) }
};

/*------------------------------------------------
   dialog procedure of "Detail of format"
--------------------------------------------------*/
INT_PTR CALLBACK DlgProcBarmeterColor(HWND hDlg, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			OnInit(hDlg);
			return TRUE;
		case WM_MEASUREITEM:
			OnMeasureItemColorCombo(lParam);
			return TRUE;
		case WM_DRAWITEM:
			OnDrawItemColorCombo(lParam, 16);
			return TRUE;
		case WM_COMMAND:
		{
			WORD id, code;
			id = LOWORD(wParam); code = HIWORD(wParam);
			switch (id)
			{
				// コンボボックス
				case IDC_BARMETERCOL_0:
				case IDC_BARMETERCOL_1:
				case IDC_BARMETERCOL_2:
				case IDC_BARMETERCOL_3:
					if (code == CBN_SELCHANGE)
						SendPSChanged(hDlg);
					break;
					//「...」色の選択
				case IDC_CHOOSEBARMETERCOL_0:
				case IDC_CHOOSEBARMETERCOL_1:
				case IDC_CHOOSEBARMETERCOL_2:
				case IDC_CHOOSEBARMETERCOL_3:
					OnChooseColor(hDlg, id, 16);
					SendPSChanged(hDlg);
					break;

				case IDOK:
				{
					OnOK(hDlg); // fall through
					EndDialog(hDlg, id);
					return TRUE;
				}
				case IDCANCEL: 
				{
					EndDialog(hDlg, id);
					return TRUE;
				}
					break;
			}
			return TRUE;
		}
	}
	return FALSE;
}

/*------------------------------------------------
  initialize the dialog
--------------------------------------------------*/
void OnInit(HWND hDlg)
{
	//char tempStr[64];
	BOOL b_ShowThresholds = TRUE;

	SendDlgItemMessage(hDlg, IDC_LABEL_BARMETER_THRESHOLD, WM_SETTEXT, NULL, MyString(IDS_BM_THRESHOLD));

	switch(BarMeterColorDlgIndex)
	{
		case IDC_BARMETERCOL_VOL:
			SetWindowText(hDlg, MyString(IDS_BMCOLOR_TITLE_VOL));

			b_ShowThresholds = FALSE;

			combocolor[0].colname = "ColorBarMeterVL";
			combocolor[1].colname = "ColorBarMeterVL_Mute";

			InitComboColor(hDlg, 2, combocolor, 16, FALSE);

			SendDlgItemMessage(hDlg, IDC_TITLE_BARMETERCOL_0, WM_SETTEXT, NULL, MyString(IDS_BMCOLOR_VOLUME));
			SendDlgItemMessage(hDlg, IDC_TITLE_BARMETERCOL_1, WM_SETTEXT, NULL, MyString(IDS_BMCOLOR_MUTE));

			ShowDlgItem(hDlg, IDC_TITLE_BARMETERCOL_2, FALSE);
			ShowDlgItem(hDlg, IDC_BARMETERCOL_2, FALSE);
			ShowDlgItem(hDlg, IDC_CHOOSEBARMETERCOL_2, FALSE);

			ShowDlgItem(hDlg, IDC_TITLE_BARMETERCOL_3, FALSE);
			ShowDlgItem(hDlg, IDC_BARMETERCOL_3, FALSE);
			ShowDlgItem(hDlg, IDC_CHOOSEBARMETERCOL_3, FALSE);

			break;
		case IDC_BARMETERCOL_CPU:
			SetWindowText(hDlg, MyString(IDS_BMCOLOR_TITLE_CPU));

			b_ShowThresholds = TRUE;

			SendDlgItemMessage(hDlg, IDC_SPGBARMETER_THRESHOLD_H, UDM_SETPOS, 0,
				(int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Threshold_High", 70));

			SendDlgItemMessage(hDlg, IDC_SPGBARMETER_THRESHOLD_M, UDM_SETPOS, 0,
				(int)(short)GetMyRegLong("BarMeter", "BarMeterCU_Threshold_Mid", 50));

			combocolor[0].colname = "ColorBarMeterCU_High";
			combocolor[1].colname = "ColorBarMeterCU_Mid";
			combocolor[2].colname = "ColorBarMeterCU_Low";
			combocolor[3].colname = "ColorBarMeterGPU";

			InitComboColor(hDlg, 4, combocolor, 16, FALSE);

			SendDlgItemMessage(hDlg, IDC_TITLE_BARMETERCOL_0, WM_SETTEXT, NULL, MyString(IDS_BMCOLOR_CU_HIGH));
			SendDlgItemMessage(hDlg, IDC_TITLE_BARMETERCOL_1, WM_SETTEXT, NULL, MyString(IDS_BMCOLOR_CU_MID));
			SendDlgItemMessage(hDlg, IDC_TITLE_BARMETERCOL_2, WM_SETTEXT, NULL, MyString(IDS_BMCOLOR_CU_LOW));
			SendDlgItemMessage(hDlg, IDC_TITLE_BARMETERCOL_3, WM_SETTEXT, NULL, MyString(IDS_BMCOLOR_GPU));

			break;
		case IDC_BARMETERCOL_BAT:
			SetWindowText(hDlg, MyString(IDS_BMCOLOR_TITLE_BAT));

			b_ShowThresholds = TRUE;

			SendDlgItemMessage(hDlg, IDC_SPGBARMETER_THRESHOLD_H, UDM_SETPOS, 0,
				(int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Threshold_High", 70));

			SendDlgItemMessage(hDlg, IDC_SPGBARMETER_THRESHOLD_M, UDM_SETPOS, 0,
				(int)(short)GetMyRegLong("BarMeter", "BarMeterBL_Threshold_Mid", 50));

			combocolor[0].colname = "ColorBarMeterBL_High";
			combocolor[1].colname = "ColorBarMeterBL_Mid";
			combocolor[2].colname = "ColorBarMeterBL_Low";
			combocolor[3].colname = "ColorBarMeterBL_Charge";

			InitComboColor(hDlg, 4, combocolor, 16, FALSE);

			SendDlgItemMessage(hDlg, IDC_TITLE_BARMETERCOL_0, WM_SETTEXT, NULL, MyString(IDS_BMCOLOR_BL_HIGH));
			SendDlgItemMessage(hDlg, IDC_TITLE_BARMETERCOL_1, WM_SETTEXT, NULL, MyString(IDS_BMCOLOR_BL_MID));
			SendDlgItemMessage(hDlg, IDC_TITLE_BARMETERCOL_2, WM_SETTEXT, NULL, MyString(IDS_BMCOLOR_BL_LOW));
			SendDlgItemMessage(hDlg, IDC_TITLE_BARMETERCOL_3, WM_SETTEXT, NULL, MyString(IDS_BMCOLOR_BL_CHARGE));

			break;
		case IDC_BARMETERCOL_NET:
			SetWindowText(hDlg, MyString(IDS_BMCOLOR_TITLE_NET));

			b_ShowThresholds = FALSE;

			combocolor[0].colname = "ColorBarMeterNet_Recv";
			combocolor[1].colname = "ColorBarMeterNet_Send";
			
			InitComboColor(hDlg, 2, combocolor, 16, FALSE);

			SendDlgItemMessage(hDlg, IDC_TITLE_BARMETERCOL_0, WM_SETTEXT, NULL, MyString(IDS_BMCOLOR_NET_RECV));
			SendDlgItemMessage(hDlg, IDC_TITLE_BARMETERCOL_1, WM_SETTEXT, NULL, MyString(IDS_BMCOLOR_NET_SEND));

			ShowDlgItem(hDlg, IDC_TITLE_BARMETERCOL_2, FALSE);
			ShowDlgItem(hDlg, IDC_BARMETERCOL_2, FALSE);
			ShowDlgItem(hDlg, IDC_CHOOSEBARMETERCOL_2, FALSE);

			ShowDlgItem(hDlg, IDC_TITLE_BARMETERCOL_3, FALSE);
			ShowDlgItem(hDlg, IDC_BARMETERCOL_3, FALSE);
			ShowDlgItem(hDlg, IDC_CHOOSEBARMETERCOL_3, FALSE);

			break;
	}

	if (b_ShowThresholds) {

		ShowDlgItem(hDlg, IDC_LABEL_BARMETER_THRESHOLD, TRUE);
		
		ShowDlgItem(hDlg, IDC_BARMETER_THRESHOLD_H, TRUE);
		ShowDlgItem(hDlg, IDC_SPGBARMETER_THRESHOLD_H, TRUE);
		ShowDlgItem(hDlg, IDC_BARMETER_THRESHOLD_M, TRUE);
		ShowDlgItem(hDlg, IDC_SPGBARMETER_THRESHOLD_M, TRUE);

		SendDlgItemMessage(hDlg, IDC_SPGBARMETER_THRESHOLD_H, UDM_SETRANGE, 0,
			(LPARAM)MAKELONG((short)101, (short)0));
		SendDlgItemMessage(hDlg, IDC_SPGBARMETER_THRESHOLD_M, UDM_SETRANGE, 0,
			(LPARAM)MAKELONG((short)101, (short)0));
	}
	else 
	{
		ShowDlgItem(hDlg, IDC_LABEL_BARMETER_THRESHOLD, FALSE);

		ShowDlgItem(hDlg, IDC_BARMETER_THRESHOLD_H, FALSE);
		ShowDlgItem(hDlg, IDC_SPGBARMETER_THRESHOLD_H, FALSE);
		ShowDlgItem(hDlg, IDC_BARMETER_THRESHOLD_M, FALSE);
		ShowDlgItem(hDlg, IDC_SPGBARMETER_THRESHOLD_M, FALSE);
	}
	
}

void OnOK(HWND hDlg)
{

	DWORD tempCol_0, tempCol_1, tempCol_2, tempCol_3;
	int tempThreshold_H, tempThreshold_M;

	tempCol_0 = CBGetItemData(hDlg, IDC_BARMETERCOL_0, CBGetCurSel(hDlg, IDC_BARMETERCOL_0));
	tempCol_1 = CBGetItemData(hDlg, IDC_BARMETERCOL_1, CBGetCurSel(hDlg, IDC_BARMETERCOL_1));
	tempCol_2 = CBGetItemData(hDlg, IDC_BARMETERCOL_2, CBGetCurSel(hDlg, IDC_BARMETERCOL_2));
	tempCol_3 = CBGetItemData(hDlg, IDC_BARMETERCOL_3, CBGetCurSel(hDlg, IDC_BARMETERCOL_3));

	tempThreshold_H = GetDlgItemInt(hDlg, IDC_BARMETER_THRESHOLD_H, NULL, FALSE);
	tempThreshold_M = GetDlgItemInt(hDlg, IDC_BARMETER_THRESHOLD_M, NULL, FALSE);


	switch (BarMeterColorDlgIndex)
	{
	case IDC_BARMETERCOL_VOL:

		SetMyRegLong("BarMeter", "ColorBarMeterVL", tempCol_0);
		SetMyRegLong("BarMeter", "ColorBarMeterVL_Mute", tempCol_1);

		break;
	case IDC_BARMETERCOL_CPU:

		SetMyRegLong("BarMeter", "BarMeterCU_Threshold_High", tempThreshold_H);
		SetMyRegLong("BarMeter", "BarMeterCU_Threshold_Mid", tempThreshold_M);

		SetMyRegLong("BarMeter", "ColorBarMeterCU_High", tempCol_0);
		SetMyRegLong("BarMeter", "ColorBarMeterCU_Mid", tempCol_1);
		SetMyRegLong("BarMeter", "ColorBarMeterCU_Low", tempCol_2);
		SetMyRegLong("BarMeter", "ColorBarMeterGPU", tempCol_3);

		break;
	case IDC_BARMETERCOL_BAT:

		SetMyRegLong("BarMeter", "BarMeterBL_Threshold_High", tempThreshold_H);
		SetMyRegLong("BarMeter", "BarMeterBL_Threshold_Mid", tempThreshold_M);

		SetMyRegLong("BarMeter", "ColorBarMeterBL_High", tempCol_0);
		SetMyRegLong("BarMeter", "ColorBarMeterBL_Mid", tempCol_1);
		SetMyRegLong("BarMeter", "ColorBarMeterBL_Low", tempCol_2);
		SetMyRegLong("BarMeter", "ColorBarMeterBL_Charge", tempCol_3);

		break;
	case IDC_BARMETERCOL_NET:

		SetMyRegLong("BarMeter", "ColorBarMeterNet_Recv", tempCol_0);
		SetMyRegLong("BarMeter", "ColorBarMeterNet_Send", tempCol_1);

		break;
	}
}
