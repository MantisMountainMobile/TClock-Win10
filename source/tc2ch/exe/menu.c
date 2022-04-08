/*-------------------------------------------------------------
  menu.c
  pop-up menu on right button click
---------------------------------------------------------------*/

#include "tclock.h"

#define	REMOVE_DRIVE_MENUPOSITION 15

// main.c
HMENU g_hMenu = NULL;
static HMENU hPopupMenu = NULL;




//Added by TTTT
BOOL b_CompactMode_menu;
BOOL b_SafeMode_menu;
//BOOL b_UseDataPlanFunc;
extern BOOL b_DebugLog;
//BOOL b_AutomaticFindWLTEProfile;
BOOL b_UseBarMeterVL;
BOOL b_UseBarMeterBL;
BOOL b_UseBarMeterCU;
BOOL b_BatteryLifeAvailable;




char	driveLetter_Win10[10][20];

extern BOOL b_UnplugDriveAvailable;

void InitializeMenuItems(void);
char stringMenuItem_RemoveDriveHeader[32];
char stringMenuItem_RemoveDriveNoDrive[32];

BOOL b_MenuItems_Initialized = FALSE;


//extern BOOL b_AcceptRisk;

extern BOOL b_NormalLog;

/*------------------------------------------------
   when the clock is right-clicked
   show pop-up menu
--------------------------------------------------*/
void OnContextMenu(HWND hwnd, HWND hwndClicked, int xPos, int yPos)
{
	char s[80];


	int i;

	UNREFERENCED_PARAMETER(hwndClicked);
	if (!g_hMenu)
	{
		g_hMenu = LoadMenu(GetLangModule(), MAKEINTRESOURCE(IDR_MENU));
		hPopupMenu = GetSubMenu(g_hMenu, 0);
		SetMenuDefaultItem(hPopupMenu, 408, FALSE);

	}

	if (!b_MenuItems_Initialized)
	{
		InitializeMenuItems();
		b_MenuItems_Initialized = TRUE;
	}

	b_CompactMode_menu = GetMyRegLong(NULL, "CompactMode", FALSE);
	b_SafeMode_menu = GetMyRegLong("Status_DoNotEdit", "SafeMode", FALSE);
	//b_UseDataPlanFunc = GetMyRegLong("DataPlan", "UseDataPlanFunction", FALSE);
	MENUITEMINFO menuiteminfo_temp;
	menuiteminfo_temp.cbSize = sizeof(MENUITEMINFO);
	menuiteminfo_temp.fMask = MIIM_STATE;

	BOOL b_ExistLTEProfile;
	BOOL b_ExistMeteredProfile;
	b_ExistLTEProfile = GetMyRegLong("Status_DoNotEdit", "ExistLTEProfile", FALSE);
	b_ExistMeteredProfile = GetMyRegLong("Status_DoNotEdit", "ExistMeteredProfile", FALSE);
	//b_AutomaticFindWLTEProfile = GetMyRegLong("DataPlan", "AutomaticFindLTEProfile", FALSE);


	b_UseBarMeterVL = GetMyRegLong("BarMeter", "UseBarMeterVL", 0);
	b_UseBarMeterBL = GetMyRegLong("BarMeter", "UseBarMeterBL", 0);
	b_UseBarMeterCU = GetMyRegLong("BarMeter", "UseBarMeterCU", 0);
	b_BatteryLifeAvailable = GetMyRegLong("Status_DoNotEdit", "BatteryLifeAvailable", 0);

	extern BOOL b_EnglishMenu;



	if (b_EnglishMenu)
	{
		menuiteminfo_temp.fState = MFS_CHECKED;
		SetMenuItemInfo(hPopupMenu, IDC_LANGUAGE_ENGLISH, FALSE, &menuiteminfo_temp);
		menuiteminfo_temp.fState = MFS_UNCHECKED;
		SetMenuItemInfo(hPopupMenu, IDC_LANGUAGE_JAPANESE, FALSE, &menuiteminfo_temp);
	}
	else
	{
		menuiteminfo_temp.fState = MFS_UNCHECKED;
		SetMenuItemInfo(hPopupMenu, IDC_LANGUAGE_ENGLISH, FALSE, &menuiteminfo_temp);
		menuiteminfo_temp.fState = MFS_CHECKED;
		SetMenuItemInfo(hPopupMenu, IDC_LANGUAGE_JAPANESE, FALSE, &menuiteminfo_temp);
	}



	if (b_UnplugDriveAvailable)
	{
	char   volume_name[256], volume_system[256];
	DWORD	serial, length, flags;
	DWORD   dwDrive;
	INT     nDrive;
	char	driveLetter[20];
	char	strTemp_Win10[256];

	char	driveList_Win10[10][265];
	//char	driveLetter_Win10[10][20];
	int		driveIndex_Win10;

	if (b_DebugLog) {
		WriteDebug_New2("[menu.c][OnContextMenu] Drive list will be made based on GetLogicalDrives()");
		char buf[1000];
		DWORD buf_size, cnt;
		buf_size = GetLogicalDriveStrings(1000, buf);
		cnt = 0;
		i = 0;
		while (cnt < buf_size) {
			wsprintf(strTemp_Win10, "[menu.c][OnContextMenu] DriveName(%d): %s", i, &buf[cnt]);
			WriteDebug_New2(strTemp_Win10);
			i++;
			cnt += lstrlen(&buf[cnt]) + 1;
		}
	}


	//HDD(DRIVE_FIXED) 情報取得用構造体等
	DWORD dwResult = 0;
	STORAGE_PROPERTY_QUERY tStragePropertyQuery;
	tStragePropertyQuery.PropertyId = StorageDeviceProperty;
	tStragePropertyQuery.QueryType = PropertyStandardQuery;
	// メモリの確保
	DWORD dwInfoSize = 4096;
	STORAGE_DEVICE_DESCRIPTOR* tpStorageDeviceDescripter = (STORAGE_DEVICE_DESCRIPTOR*)malloc(dwInfoSize);


		driveIndex_Win10 = 0;
		dwDrive = GetLogicalDrives();
		for (nDrive = 0; nDrive < 26; nDrive++) 
		{

			if (dwDrive & (1 << nDrive)) {

				wsprintf(driveLetter, "%c:\\", 'A' + nDrive);

				auto tempDriveType = GetDriveTypeA(driveLetter);
				
				if (b_DebugLog && tempDriveType != DRIVE_UNKNOWN)
				{
					wsprintf(strTemp_Win10, "[menu.c][OnContextMenu] DriveType for Drive %c =%d", ('A' + nDrive), (int)tempDriveType);
					WriteDebug_New2(strTemp_Win10);
				}

				if (tempDriveType == DRIVE_REMOVABLE)
				{
					if (GetVolumeInformationA(driveLetter, volume_name, sizeof(volume_name), &serial, &length, &flags, volume_system, sizeof(volume_system)) != 0)
					{
						//wsprintf(driveList_Win10[driveIndex_Win10], "%s%s (%c:)", stringMenuItem_RemoveDriveHeader, volume_name, ('A' + nDrive));
						wsprintf(driveList_Win10[driveIndex_Win10], "%s(%c) %s", stringMenuItem_RemoveDriveHeader, ('A' + nDrive), volume_name);
						wsprintf(driveLetter_Win10[driveIndex_Win10], "%c", 'A' + nDrive);
						driveIndex_Win10++;

						if (b_DebugLog)
						{
							wsprintf(strTemp_Win10, "[menu.c][OnContextMenu] %s (%c:)", volume_name, ('A' + nDrive));
							WriteDebug_New2(strTemp_Win10);
						}

					}

				}

				if (tempDriveType == DRIVE_FIXED)
				{
					char fname[16];
					wsprintf(fname, "\\\\.\\%c:", 'A' + nDrive);
					HANDLE hDevice = CreateFile(fname, 0,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL, OPEN_EXISTING, 0, NULL);

					if (hDevice != INVALID_HANDLE_VALUE) {



						////HDD(DRIVE_FIXED) 情報取得用構造体等
						//DWORD dwResult = 0;
						//STORAGE_PROPERTY_QUERY tStragePropertyQuery;
						//tStragePropertyQuery.PropertyId = StorageDeviceProperty;
						//tStragePropertyQuery.QueryType = PropertyStandardQuery;
						//// メモリの確保
						//DWORD dwInfoSize = 4096;
						//STORAGE_DEVICE_DESCRIPTOR* tpStorageDeviceDescripter = (STORAGE_DEVICE_DESCRIPTOR*)malloc(dwInfoSize);


						/*
						IOCTL_STORAGE_QUERY_PROPERTY
						*/
						BOOL bResult = DeviceIoControl(
							(HANDLE)hDevice                            // handle to device
							, IOCTL_STORAGE_QUERY_PROPERTY                // dwIoControlCode
							, &tStragePropertyQuery                       // lpInBuffer
							, (DWORD)sizeof(tStragePropertyQuery)       // nInBufferSize
							, (LPVOID)tpStorageDeviceDescripter           // output buffer
							, (DWORD)dwInfoSize                           // size of output buffer
							, (LPDWORD)&dwResult                          // number of bytes returned
							, (LPOVERLAPPED)NULL                          // OVERLAPPED structure
						);

						if (0 == bResult) {
							if (b_DebugLog) WriteDebug_New2("[menu.c][OnContextMenu] Failed in IOCTL_STORAGE_QUERY_PROPERTY for HDD (Fixed_Drive)");
						}
						else
						{
							if (tpStorageDeviceDescripter->BusType == BusTypeUsb)	// BusTypeUsb = 0x7
							{
								if (GetVolumeInformationA(driveLetter, volume_name, sizeof(volume_name), &serial, &length, &flags, volume_system, sizeof(volume_system)) != 0)
								{
									wsprintf(driveList_Win10[driveIndex_Win10], "%s(%c) %s (HDD)", stringMenuItem_RemoveDriveHeader, ('A' + nDrive), volume_name);
									wsprintf(driveLetter_Win10[driveIndex_Win10], "%c", 'A' + nDrive);
									driveIndex_Win10++;

									if (b_DebugLog)
									{
										wsprintf(strTemp_Win10, "[menu.c][OnContextMenu] %s (%c:)", volume_name, ('A' + nDrive));
										WriteDebug_New2(strTemp_Win10);
									}
								}
							}

							if (b_DebugLog)
							{
								wsprintf(strTemp_Win10, "[menu.c][OnContextMenu] %s (%c:) is FixedDrive with STORAGE_BUS_TYPE =%02x", volume_name, ('A' + nDrive), (int)tpStorageDeviceDescripter->BusType);
								WriteDebug_New2(strTemp_Win10);
							}
						}
						CloseHandle(hDevice);
					}
					else
					{
						if (b_DebugLog) WriteDebug_New2("[menu.c][OnContextMenu] Failed to get HANDLE(CreateFile Func) for HDD (Fixed_Drive)");
					}
				}

			}
		}

		for (i = 1; i < 10; i++) DeleteMenu(hPopupMenu, IDC_REMOVE_DRIVE0 + i, MF_BYCOMMAND);


		if (driveIndex_Win10 > 10) driveIndex_Win10 = 10;
		if (driveIndex_Win10 > 1)
		{
			driveIndex_Win10--;
			ModifyMenu(hPopupMenu, IDC_REMOVE_DRIVE0, MF_BYCOMMAND, IDC_REMOVE_DRIVE0 + driveIndex_Win10, driveList_Win10[driveIndex_Win10]);
			driveIndex_Win10--;
			for (i = driveIndex_Win10; i >= 0; i--)
			{
				InsertMenu(hPopupMenu, IDC_REMOVE_DRIVE0 + i + 1, MF_BYCOMMAND, IDC_REMOVE_DRIVE0 + i, driveList_Win10[i]);
			}
		}
		else if (driveIndex_Win10 == 1)
		{
			ModifyMenu(hPopupMenu, IDC_REMOVE_DRIVE0, MF_BYCOMMAND, IDC_REMOVE_DRIVE0, driveList_Win10[0]);
			EnableMenuItem(hPopupMenu, IDC_REMOVE_DRIVE0, MF_BYCOMMAND | MF_ENABLED);
		}
		else
		{
			ModifyMenu(hPopupMenu, IDC_REMOVE_DRIVE0, MF_BYCOMMAND, IDC_REMOVE_DRIVE0, stringMenuItem_RemoveDriveNoDrive);
			EnableMenuItem(hPopupMenu, IDC_REMOVE_DRIVE0, MF_BYCOMMAND | MF_GRAYED);
		}

	}


	SetForegroundWindow98(hwnd);
	TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN|TPM_RIGHTBUTTON,
		xPos, yPos, 0, hwnd, NULL);

}

/*------------------------------------------------
    command message
--------------------------------------------------*/
void OnTClockCommand(HWND hwnd, WORD wID, WORD wCode)
{
	extern BOOL b_EnglishMenu;
	extern int Language_Offset;

	switch(wID)
	{
		case IDC_SHOWDIR: // Show Directory
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_SHOWDIR received");
			ShellExecute(g_hwndMain, NULL, g_mydir, NULL, NULL, SW_SHOWNORMAL);
			break;
		case IDC_SHOWPROP: // Show property
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_SHOWPROP received");
			MyPropertyDialog();
			return;
		case IDC_EXIT: // Exit
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_EXIT received");
			if (b_NormalLog) WriteNormalLog("Exit TClock-Win10 from right-click menu.");
			PostMessage(g_hwndMain, WM_CLOSE, 0, 0);
//			PostMessage(g_hwndClock, WM_COMMAND, IDC_EXIT, 0);
			return;

		case IDC_RESTART: // Restart
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_RESTART received");
			if (b_NormalLog) WriteNormalLog("Restart TClock-Win10 from right-click menu.");
			PostMessage(g_hwndClock, WM_COMMAND, IDC_RESTART, 0);
			return;
		case IDC_POWERPNL:	//Added by TTTT
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_POWERPNL received");
			ShellExecute(NULL, "open", "control", "/name Microsoft.PowerOptions /page pageGlobalSettings", NULL, SW_SHOWNORMAL);
			return;

		case IDC_NETWORKSTG:	//Added by TTTT
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_NETWORKSTG received");
			ShellExecute(NULL, "open", "ms-settings:network-status", NULL, NULL, SW_SHOWNORMAL);
			return;

		case IDC_NETWORKPNL:	//Added by TTTT
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_NETWORKPNL received");
			ShellExecute(NULL, "open", "control", "ncpa.cpl", NULL, SW_SHOWNORMAL);
			return;

		case IDC_DATAUSAGE:	//Added by TTTT
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_DATAUSAGE received");
			ShellExecute(NULL, "open", "ms-settings:datausage", NULL, NULL, SW_SHOWNORMAL);
			return;

		case IDC_CONTROLPNL:	//Added by TTTT
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_CONTROLPNL received");
			ShellExecute(NULL, "open", "control", NULL, NULL, SW_SHOWNORMAL);
			return;

		case IDC_SETTING:	//Added by TTTT
			if (b_DebugLog) WriteDebug_New2("[menu.c][CLOCKM_TOGGLE_DATAPLANFUNCOnTClockCommand] IDC_SETTING received");
			ShellExecute(NULL, "open", "ms-settings:", NULL, NULL, SW_SHOWNORMAL);
			return;

		case IDC_CMD:	//Added by TTTT
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_CMD received");
			ShellExecute(NULL, "open", "cmd.exe", "/k cd \\", NULL, SW_SHOWNORMAL);
			return;

		case IDC_LANGUAGE_JAPANESE:	//Added by TTTT
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_LANGUAGE_JAPANESE received");
			if (b_EnglishMenu)
			{
				b_EnglishMenu = !b_EnglishMenu;
				SetMyRegLong(NULL, "EnglishMenu", b_EnglishMenu);
				b_MenuItems_Initialized = FALSE;
				Language_Offset = LANGUAGE_OFFSET_JAPANESE;
			}
			return;

		case IDC_LANGUAGE_ENGLISH:	//Added by TTTT
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_LANGUAGEC_ENGLISH received");
			if (!b_EnglishMenu)
			{
				b_EnglishMenu = !b_EnglishMenu;
				SetMyRegLong(NULL, "EnglishMenu", b_EnglishMenu);
				b_MenuItems_Initialized = FALSE;
				Language_Offset = LANGUAGE_OFFSET_ENGLISH;
			}
			return;

		//case IDC_TOGGLE_CLOUD_APP: //Added by TTTT
		//{
		//	if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_TOGGLE_CLOUD_APP received");
		//	PostMessage(g_hwndClock, WM_COMMAND, (WPARAM)CLOCKM_TOGGLE_AUTOPAUSE_CLOUDAPP, 0);
		//	return;
		//}

		//case IDC_TOGGLE_DATAPLANFUNC: //Added by TTTT
		//{
		//	if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_TOGGLE_DATAPLANFUNC received");
		//	PostMessage(g_hwndClock, WM_COMMAND, (WPARAM)CLOCKM_TOGGLE_DATAPLANFUNC, 0);
		//	return;
		//}

		case IDC_VISTACALENDAR:
		{
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_VISTACALENDAR received");
			PostMessage(g_hwndClock, CLOCKM_VISTACALENDAR, 0, 0);
			return;
		}

		case IDC_SHOWAVAILABLENETWORKS:
		{
			if (b_DebugLog) WriteDebug_New2("[menu.c][OnTClockCommand] IDC_SHOWAVAILABLENETWORKS received");
			PostMessage(g_hwndClock, CLOCKM_SHOWAVAILABLENETWORKS, 0, 0);
			return;
		}


		case IDC_REMOVE_DRIVE0:
		case IDC_REMOVE_DRIVE1:
		case IDC_REMOVE_DRIVE2:
		case IDC_REMOVE_DRIVE3:
		case IDC_REMOVE_DRIVE4:
		case IDC_REMOVE_DRIVE5:
		case IDC_REMOVE_DRIVE6:
		case IDC_REMOVE_DRIVE7:
		case IDC_REMOVE_DRIVE8:
		case IDC_REMOVE_DRIVE9:
		{
			if (wID == IDC_REMOVE_DRIVE0 && !b_UnplugDriveAvailable)
			{
				MessageBox(NULL, "TClockフォルダにフリーソフトのUnplugDrive Portable (UnplugDrive.exe)を置くと、リムーバブルドライブ取り外し機能を利用することができます。\n\nHaving UnplugDrive.exe (Japanese freeware) in TClock folder enables \"Remove Drive\" function.",
					"TClock-Win10", MB_OK | MB_SETFOREGROUND | MB_ICONINFORMATION);
				return;
			}

			char strAppTemp_Win10[MAX_PATH];
			strcpy(strAppTemp_Win10, g_mydir);
			add_title(strAppTemp_Win10, "UnplugDrive.exe");
			if (b_DebugLog) WriteDebug_New2("Remove Drive Application will be executed:");
			if (b_DebugLog) WriteDebug_New2(strAppTemp_Win10);
			if (b_DebugLog) WriteDebug_New2(driveLetter_Win10[wID - IDC_REMOVE_DRIVE0]);
			ShellExecute(NULL, "open", strAppTemp_Win10, driveLetter_Win10[wID - IDC_REMOVE_DRIVE0], NULL, SW_SHOWNORMAL);
			return;
		}

		case IDC_TASKMAN:
		{
			ShellExecute(NULL, "open", "taskmgr", NULL, NULL, SW_SHOWNORMAL);
			SetWindowPos(GetActiveWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			return;
		}

		case IDC_ALARM_CLOCK:
		{
			ShellExecute(NULL, "open", "ms-clock:", NULL, NULL, SW_SHOWNORMAL);
			SetWindowPos(GetActiveWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			return;
		}

		case IDC_PUSHBACK:
		{
			extern int PusuBackIndex;
			PusuBackIndex = 0;
			EnumWindows(PusuBackOBWindow, NULL);
			return;
		}

		case IDC_DATETIME_Win10:
		{
			HWND hwndTray;
			hwndTray = FindWindow("Shell_TrayWnd", NULL);
			if(hwndTray)
			{
				if (wID == IDC_DATETIME_Win10)
					PostMessage(hwndTray, WM_COMMAND, (WPARAM)IDC_DATETIME, 0);
				else
					PostMessage(hwndTray, WM_COMMAND, (WPARAM)wID, 0);
			}
			return;
		}
		case IDC_HOTKEY0:
			ExecuteMouseFunction(hwnd, wCode, IDS_HOTKEY - IDS_LEFTBUTTON, 0);
			return;
		case IDC_HOTKEY1:
		case IDC_HOTKEY2:
		case IDC_HOTKEY3:
		case IDC_HOTKEY4:
			ExecuteMouseFunction(hwnd, -1, IDS_HOTKEY - IDS_LEFTBUTTON, wID - IDC_HOTKEY1 + 1);
			return;
	}

	return;
}



/*-------------------------------------------------------
Initialize Menu Items Japanese or English, Added by TTTT
---------------------------------------------------------*/
void InitializeMenuItems(void)
{

	extern BOOL b_EnglishMenu;
	char s[64];
	char stringLTE[32];


	GetMyRegStr("ETC", "LTEstring", stringLTE, 32, "LTE");





	strcpy(stringMenuItem_RemoveDriveHeader, MyString(IDS_RMVDRVHEAD));
	strcpy(stringMenuItem_RemoveDriveNoDrive, MyString(IDS_NORMVDRV));


	//if (b_AcceptRisk)
	//{
	//	if (b_EnglishMenu)	
	//		wsprintf(s, "Kill Apps on %s / Metered WiFi", stringLTE);
	//	else
	//		wsprintf(s, "%s・従量課金WiFiでアプリ強制終了", stringLTE);
	//	ModifyMenu(hPopupMenu, IDC_TOGGLE_CLOUD_APP, MF_BYCOMMAND, IDC_TOGGLE_CLOUD_APP, s);
	//}
	//else
	//{
	//	DeleteMenu(hPopupMenu, IDC_TOGGLE_CLOUD_APP, MF_BYCOMMAND);
	//}

	wsprintf(s, MyString(IDS_MENURETRIEVE));
//	ModifyMenu(hPopupMenu, IDC_TOGGLE_DATAPLANFUNC, MF_BYCOMMAND, IDC_TOGGLE_DATAPLANFUNC, s);

	ModifyMenu(hPopupMenu, IDC_TASKMAN, MF_BYCOMMAND, IDC_TASKMAN, MyString(IDS_TASKMGR));
	ModifyMenu(hPopupMenu, IDC_CMD, MF_BYCOMMAND, IDC_CMD,MyString(IDS_CMD));
	ModifyMenu(hPopupMenu, IDC_ALARM_CLOCK, MF_BYCOMMAND, IDC_ALARM_CLOCK, MyString(IDS_ALARM_CLOCK));
	ModifyMenu(hPopupMenu, IDC_PUSHBACK, MF_BYCOMMAND, IDC_PUSHBACK, MyString(IDS_PUSHBACK));
	ModifyMenu(hPopupMenu, IDC_VISTACALENDAR, MF_BYCOMMAND, IDC_VISTACALENDAR, MyString(IDS_VISTACALENDAR));
	ModifyMenu(hPopupMenu, IDC_SHOWAVAILABLENETWORKS, MF_BYCOMMAND, IDC_SHOWAVAILABLENETWORKS, MyString(IDS_SHOWAVAILABLENETWORKS));
	ModifyMenu(hPopupMenu, IDC_CONTROLPNL, MF_BYCOMMAND, IDC_CONTROLPNL, MyString(IDS_CONTROLPNL));
	ModifyMenu(hPopupMenu, IDC_POWERPNL, MF_BYCOMMAND, IDC_POWERPNL, MyString(IDS_POWERPNL));
	ModifyMenu(hPopupMenu, IDC_NETWORKPNL, MF_BYCOMMAND, IDC_NETWORKPNL, MyString(IDS_NETWORKPNL));
	ModifyMenu(hPopupMenu, IDC_SETTING, MF_BYCOMMAND, IDC_SETTING, MyString(IDS_SETTING));
	ModifyMenu(hPopupMenu, IDC_NETWORKSTG, MF_BYCOMMAND, IDC_NETWORKSTG, MyString(IDS_NETWORKSTG));
	ModifyMenu(hPopupMenu, IDC_DATETIME_Win10, MF_BYCOMMAND, IDC_DATETIME_Win10, MyString(IDS_PROPDATE));
	ModifyMenu(hPopupMenu, IDC_REMOVE_DRIVE0, MF_BYCOMMAND, IDC_REMOVE_DRIVE0, MyString(IDS_ABOUTRMVDRV));
	ModifyMenu(hPopupMenu, IDC_SHOWDIR, MF_BYCOMMAND, IDC_SHOWDIR, MyString(IDS_OPENTCFOLDER));
	ModifyMenu(hPopupMenu, IDC_SHOWPROP, MF_BYCOMMAND, IDC_SHOWPROP, MyString(IDS_PROPERTY));
	ModifyMenu(hPopupMenu, IDC_EXIT, MF_BYCOMMAND, IDC_EXIT, MyString(IDS_EXITTCLOCK));
	ModifyMenu(hPopupMenu, IDC_RESTART, MF_BYCOMMAND, IDC_RESTART, MyString(IDS_RESTART));






}
