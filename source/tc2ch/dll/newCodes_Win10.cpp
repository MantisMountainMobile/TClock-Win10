#include "tcdll.h"
#include "string.h"
#include <Windows.h>
#include <iostream>
#include <PowrProf.h>
#include <ntstatus.h>
#pragma comment(lib, "PowrProf.lib")

//focus assist状態取得
//https://stackoverflow.com/questions/53407374/is-there-a-way-to-detect-changes-in-focus-assist-formerly-quiet-hours-in-windo
#include <map>

// concurrency名前空間
#include <ppltasks.h>
#include <endpointvolume.h>	//added by TTTT for volume
#include <mmdeviceapi.h>	//added by TTTT for volume

// GPU取得
#include <pdh.h>
#include <pdhmsg.h>
#pragma comment(lib, "pdh.lib")


#define TIMEDIF_Win10 (ULONGLONG)324000000000
#define ONEDAY_Win10 (ULONGLONG)864000000000
#define ONESEC_Win10 (ULONGLONG)10000000
#define MAX_PROFILE_NUMBER 128
#define USAGE_TIMEOUT 5

using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections; //for IVectorView<>
using namespace Platform;
using namespace Windows::Networking::Connectivity;
using namespace concurrency; //for async functionality
using namespace Windows::UI::Notifications;	// for Win10(UWP) notirfation
using namespace Windows::Data::Xml::Dom;	// for Win10(UWP) notirfation
using namespace Windows::UI::Notifications::Management;

ConnectionProfile^ internetConnectProf;
Collections::IVectorView<ConnectionProfile^>^ connectProfs;

DataUsageGranularity granularityForInquiry;
NetworkUsageStates networkUsageStatesForInquiry;

FILETIME filetime_Win10;
SYSTEMTIME systemtime_Win10;



String^ connectProfName[MAX_PROFILE_NUMBER];
String^ ssidOrApName[MAX_PROFILE_NUMBER];

BOOL b_LTEProf[MAX_PROFILE_NUMBER];
BOOL b_WiFiProf[MAX_PROFILE_NUMBER];
GUID adapterIDforProf[MAX_PROFILE_NUMBER];


int connectedWiFiProfNum = -1;
int connectedLTEProfNum = -1;

int connectProfConnectLevel[MAX_PROFILE_NUMBER];



BOOL b_LTEProfNum_Confirmed = FALSE;

BOOL b_StartupCompleted = FALSE;

DWORD tickCount_LastCalled = 0;
DWORD tickCount_LastLog = 0;

int numOfConnectProfs = 0;

IAudioEndpointVolume *endpointVolume;
IMMDevice *defaultDevice_Win10;
IMMDeviceEnumerator *deviceEnumerator_Win10;

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);


BOOL b_ToDo_updateConnectProfsInfo_Win10 = FALSE;


int prevNum_SoundDev = 0;

//WORD prev_wPid_WaveMapper = 0;
//char prevSoundDevName[128];


int soundDevChcekCountDown = 10;

UserNotificationListenerAccessStatus accessStatus;


//focus assist状態取得コード
//https://stackoverflow.com/questions/53407374/is-there-a-way-to-detect-changes-in-focus-assist-formerly-quiet-hours-in-windo







// from ntdef.h
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

// from ntdef.h
typedef struct _WNF_STATE_NAME
{
	ULONG Data[2];
} WNF_STATE_NAME;

typedef struct _WNF_STATE_NAME* PWNF_STATE_NAME;
typedef const struct _WNF_STATE_NAME* PCWNF_STATE_NAME;

typedef struct _WNF_TYPE_ID
{
	GUID TypeId;
} WNF_TYPE_ID, *PWNF_TYPE_ID;

typedef const WNF_TYPE_ID* PCWNF_TYPE_ID;

typedef ULONG WNF_CHANGE_STAMP, *PWNF_CHANGE_STAMP;


//enum FocusAssistResult
//{
//	not_supported = -2,
//	failed = -1,
//	off = 0,
//	priority_only = 1,
//	alarms_only = 2
//};



typedef NTSTATUS(NTAPI *PNTQUERYWNFSTATEDATA)(
	_In_ PWNF_STATE_NAME StateName,
	_In_opt_ PWNF_TYPE_ID TypeId,
	_In_opt_ const VOID* ExplicitScope,
	_Out_ PWNF_CHANGE_STAMP ChangeStamp,
	_Out_writes_bytes_to_opt_(*BufferSize, *BufferSize) PVOID Buffer,
	_Inout_ PULONG BufferSize);

extern "C" int GetFocusAssistState(void)
{
	extern BOOL b_DebugLog;

	// note: ntdll is guaranteed to be in the process address space.
	const auto h_ntdll = GetModuleHandle(_T("ntdll"));

	// get pointer to function
	const auto pNtQueryWnfStateData = PNTQUERYWNFSTATEDATA(GetProcAddress(h_ntdll, "NtQueryWnfStateData"));
	if (!pNtQueryWnfStateData)
	{
		if (b_DebugLog)writeDebugLog_Win10("[newCodes_Win10.cpp][GetFocusAssistState] Error to get API pointer.", 999);
		return -1;
	}

	// state name for active hours (Focus Assist)
	WNF_STATE_NAME WNF_SHEL_QUIETHOURS_ACTIVE_PROFILE_CHANGED{ 0xA3BF1C75, 0xD83063E };

	// note: we won't use it but it's required
	WNF_CHANGE_STAMP change_stamp = { 0 };

	// on output buffer will tell us the status of Focus Assist
	DWORD buffer = 0;
	ULONG buffer_size = sizeof(buffer);

	if (NT_SUCCESS(pNtQueryWnfStateData(&WNF_SHEL_QUIETHOURS_ACTIVE_PROFILE_CHANGED, nullptr, nullptr, &change_stamp,
		&buffer, &buffer_size)))
	{
		if (b_DebugLog)writeDebugLog_Win10("[newCodes_Win10.cpp][GetFocusAssistState] Focus assist state =", buffer);
		return buffer;
	}
	else
	{
		if (b_DebugLog)writeDebugLog_Win10("[newCodes_Win10.cpp][GetFocusAssistState] Error while calling NtQueryWnfStateData.", 999);
		return -1;
	}
	return -1;
}



extern "C" int GetNotificationNumber(void)
{

	extern BOOL b_DebugLog;
	if (b_DebugLog)writeDebugLog_Win10("[newCodes_Win10.cpp] GetNotificationNumber called.", 999);

	// note: ntdll is guaranteed to be in the process address space.
	const auto h_ntdll = GetModuleHandle(_T("ntdll"));

	// get pointer to function
	const auto pNtQueryWnfStateData = PNTQUERYWNFSTATEDATA(GetProcAddress(h_ntdll, "NtQueryWnfStateData"));
	if (!pNtQueryWnfStateData)
	{
		if (b_DebugLog)writeDebugLog_Win10("[newCodes_Win10.cpp][GetNotificationNumber] Error to get API pointer.", 999);
		return -1;
	}

	// state name for active hours (Focus Assist)
	WNF_STATE_NAME WNF_NOTIFICATION_NUMBER{ 0xA3BC1035, 0xD83063E };

	// note: we won't use it but it's required
	WNF_CHANGE_STAMP change_stamp = { 0 };

	// on output buffer will tell us the status of Focus Assist
	DWORD buffer = 0;
	ULONG buffer_size = sizeof(buffer);

	if (NT_SUCCESS(pNtQueryWnfStateData(&WNF_NOTIFICATION_NUMBER, nullptr, nullptr, &change_stamp,
		&buffer, &buffer_size)))
	{
		if (b_DebugLog)writeDebugLog_Win10("[newCodes_Win10.cpp][GetNotificationNumber] Number of Notifications =", buffer);
		if (buffer > 99)buffer = 99;
		return buffer;
	}
	else
	{
		if (b_DebugLog)writeDebugLog_Win10("[newCodes_Win10.cpp][GetNotificationNumber] Error while calling NtQueryWnfStateData.", 999);
		return -1;
	}
	return -1;
}




extern "C" BOOL WINAPI CheckModernStandbyCapability_Win10(void) //20190725
{
	extern BOOL b_DebugLog;
	//extern BOOL b_ModernStandbySupported;

	//http://answers.flyppdevportal.com/MVC/Post/Thread/70e7fde7-aa4b-4b77-becc-5ff79793a1e2?category=csharpgeneralja
	SYSTEM_POWER_CAPABILITIES spc;
	auto r1 = CallNtPowerInformation(SystemPowerCapabilities, nullptr, 0, &spc, sizeof spc);
	if (r1 != STATUS_SUCCESS) 
	{
		//この関数内のログは通常動作では記録されない。確認したければCheckWinVersion_Win10()の最後のほうのコメントアウトからこの関数を呼ぶ必要あり。
		if (b_DebugLog) writeDebugLog_Win10("[newCodes_Win10][CheckModernStandbyCapability_Win10] CallNtPowerInformation failed: NTSTATUS %d", r1);
	}
	else
	{
		if (!spc.AoAc)
		{
			if (b_DebugLog) writeDebugLog_Win10("[newCodes_Win10] ModernStandby not supported", 999);
			//SetMyRegLong("Status_DoNotEdit", "ModernStandbySupported", 0);
			return FALSE;
		}
		else
		{
			if (b_DebugLog) writeDebugLog_Win10("[newCodes_Win10][CheckModernStandbyCapability_Win10] ModernStandby supported", 999);
			//SetMyRegLong("Status_DoNotEdit", "ModernStandbySupported", 1);
			return TRUE;
		}
	}
}



extern "C" int WINAPI CheckWinVersion_Win10(void)
{
	extern BOOL b_DebugLog;

	int ret;
	int majorVersion;
	int minorVersion;
	int buildNumber;
	auto versionInfo = RTL_OSVERSIONINFOW{ sizeof(RTL_OSVERSIONINFOW) };
	HMODULE hModule = GetModuleHandle("ntdll.dll");

	if (hModule)
	{
		auto RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hModule, "RtlGetVersion");
		if (RtlGetVersion != nullptr)
		{
			RtlGetVersion(&versionInfo);
			majorVersion = (int)versionInfo.dwMajorVersion;
			minorVersion = (int)versionInfo.dwMinorVersion;
			buildNumber = (int)versionInfo.dwBuildNumber;

			if (b_DebugLog)
			{
				writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] Version was checked with RtlGetVersion in ntdll.dll", 999);
				writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] MajorVersion = ", majorVersion);
				writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] MinorVersion = ", minorVersion);
				writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] BuildNumber = ", buildNumber);
			}
		}
		else
		{
			DWORD dw;
			dw = 0;
			dw = GetVersion();
			majorVersion = (int)(LOBYTE(LOWORD(dw)));
			minorVersion = (int)(HIBYTE(LOWORD(dw)));
			buildNumber = (int)HIWORD(dw) & 0x7fff;

			if (b_DebugLog)
			{
				writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] RtlGetVersion in ntdll.dll is not available. Version was checked with GetVersion()", 999);
				writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] MajorVersion = ", majorVersion);
				writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] MinorVersion = ", minorVersion);
				writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] BuildNumber = ", buildNumber);
			}
		}
	}
	else
	{
		DWORD dw;
		dw = 0;
		dw = GetVersion();
		majorVersion = (int)(LOBYTE(LOWORD(dw)));
		minorVersion = (int)(HIBYTE(LOWORD(dw)));
		buildNumber = (int)HIWORD(dw) & 0x7fff;

		if (b_DebugLog)
		{
			writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] ntdll.dll is not available. Version was checked with GetVersion()", 999);
			writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] MajorVersion = ", majorVersion);
			writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] MinorVersion = ", minorVersion);
			writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] BuildNumber = ", buildNumber);
		}
	}

	{
		ret = 0;
		ret |= WINNT;
		if (majorVersion >= 5)	// 5.0
		{
			ret |= WIN2000;
		}
		if (majorVersion >= 5 && minorVersion >= 1)	// 5.1
		{
			ret |= WINXP;
		}
		if (majorVersion >= 6 && minorVersion >= 0)	// 6.0
		{
			ret |= WINVISTA;
		}
		if (majorVersion >= 6 && minorVersion >= 1)	// 6.1
		{
			ret |= WIN7;
		}
			if (majorVersion >= 6 && minorVersion >= 2)	// 6.2
		{
			ret |= WIN8;
		}
		if (majorVersion >= 6 && minorVersion >= 3)	// 6.3
		{
			ret |= WIN8_1;
		}
		if (majorVersion >= 10)	// 10.0
		{
			if (buildNumber >= 10240)		// Ver. 1507
			{
				ret |= WIN10;		// Threshold 1
			}
			if (buildNumber >= 10586)		// Ver. 1511
			{
				ret |= WIN10TH2;	// Threshold 2, November Update
			}
			if (buildNumber >= 14393)		// Ver. 1607
			{
				ret |= WIN10RS1;	// Redstone 1, Anniversary Update
			}
		}
	}

	if (b_DebugLog) writeDebugLog_Win10("[newCodes_Win10][CheckWinVersion_Win10] result =", ret);



	//以下の行はコメントアウトのままで動作する。関数内のログを得る必要があればここで余分に一回呼ぶ。
	//CheckModernStandbyCapability_Win10();	


	return ret;
}








extern "C" void initializeVolume_Win10()
{


	HRESULT hr;
	extern BOOL b_DebugLog;

	float currentVolume = 0;
	extern int iVolume;
	extern BOOL muteStatus;
	extern BOOL b_SoundCapability;

	int dev_num = 0;

	if (b_DebugLog) writeDebugLog_Win10("[newCodes_Win10] initializeVolume_Win10() called", 999);

	soundDevChcekCountDown = 10;

	dev_num = waveOutGetNumDevs();
	prevNum_SoundDev = dev_num;
	if (b_DebugLog) writeDebugLog_Win10("[newCodes_Win10] [initializeVolume_Win10()] Number of Sound Dev. is Updated as:", dev_num);

	if (dev_num == 0) {
		b_SoundCapability = FALSE;
		muteStatus = TRUE;
		iVolume = 0;
		//prev_wPid_WaveMapper = 0;
	}
	else
	{
	//	WAVEOUTCAPS tempWaveOutCaps;

	//	waveOutGetDevCaps(WAVE_MAPPER, &tempWaveOutCaps, sizeof(WAVEOUTCAPS));
	//	prev_wPid_WaveMapper = tempWaveOutCaps.wPid;

	//	strncpy_s(prevSoundDevName, tempWaveOutCaps.szPname, 32);

	//	if (b_DebugLog)
	//	{
	//		writeDebugLog_Win10("[newCodes_Win10] [initializeVolume_Win10()] Index of wavout device (prev_wPid_WaveMapper) is updated as:", (int)(prev_wPid_WaveMapper));
	//		writeDebugLog_Win10("[newCodes_Win10] [initializeVolume_Win10()] Name of Sound Device (prevSoundDevName) is:", 999);
	//		writeDebugLog_Win10(prevSoundDevName, 999);
	//	}



		CoInitialize(NULL);
		CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IMMDeviceEnumerator), (void**)&deviceEnumerator_Win10);

		hr = deviceEnumerator_Win10->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice_Win10);
		deviceEnumerator_Win10->Release();
		deviceEnumerator_Win10 = NULL;

		hr = defaultDevice_Win10->Activate(__uuidof(IAudioEndpointVolume),
			CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);

		b_SoundCapability = TRUE;
	}


	if (b_DebugLog) writeDebugLog_Win10("[newCodes_Win10] [initializeVolume_Win10)] b_SoundCapability = ", (int)b_SoundCapability);



	if (b_SoundCapability && (endpointVolume != NULL))
	{
		endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
		endpointVolume->GetMute(&muteStatus);
		iVolume = (int)(currentVolume * 100);
	}
	
	//if (b_DebugLog) writeDebugLog_Win10("[newCodes_Win10] [Startup] Current Volume Level in % is:", (int)(currentVolume * 100));

}


extern "C" void getMasterVolume_Win10()
{

	float currentVolume = 0;
	
	extern int iVolume;
	extern BOOL muteStatus;
	extern int dataPlanChkPeriod_Win10;
	extern BOOL b_SoundCapability;
	extern BOOL b_DebugLog;

	int dev_num = 0;

	dev_num = waveOutGetNumDevs();
	if (dev_num != prevNum_SoundDev) {
		if (b_DebugLog) writeDebugLog_Win10("[newCodes_Win10] [getMasterVolume_Win10()] prevNum_SoundDev is :", prevNum_SoundDev);
		if (b_DebugLog) writeDebugLog_Win10("[newCodes_Win10] [getMasterVolume_Win10()] Number of Sound Dev. was changed to :", dev_num);
		initializeVolume_Win10();
	}


	soundDevChcekCountDown--;

	if (soundDevChcekCountDown <= 0)
	{
		initializeVolume_Win10();
	}
	else if (b_SoundCapability && endpointVolume != NULL)
	{
		endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
		endpointVolume->GetMute(&muteStatus);
		iVolume = (int)(currentVolume * 100);
	}

}



extern "C" void toggleBarMeterFunc_Win10(int f)
{
	extern BOOL b_UseBarMeterVL;
	extern BOOL b_UseBarMeterCU;
	extern BOOL b_UseBarMeterBL;

	if (f == 0)
	{
		b_UseBarMeterVL = !b_UseBarMeterVL;
		SetMyRegLong("BarMeter", "UseBarMeterVL", b_UseBarMeterVL);
	}
	else if (f == 1)
	{
		b_UseBarMeterCU = !b_UseBarMeterCU;
		SetMyRegLong("BarMeter", "UseBarMeterCU", b_UseBarMeterCU);
	}
	else if (f == 2)
	{
		b_UseBarMeterBL = !b_UseBarMeterBL;
		SetMyRegLong("BarMeter", "UseBarMeterBL", b_UseBarMeterBL);
	}

}



extern "C" void writeDebugLog_Win10(LPSTR s, int num)
{
		SYSTEMTIME systemtime;
		char tempstr_Win10[256];
		GetLocalTime(&systemtime);
		DWORD tickCount_LastLog_temp = 0;
		tickCount_LastLog_temp = GetTickCount();
		int msPast = tickCount_LastLog_temp - tickCount_LastLog;
		char marker[32];
		if (msPast > 1000) strcpy(marker, "*");
		else strcpy(marker, " ");

		if (num != 999)
		{
			snprintf(tempstr_Win10, 256, "%s %d/%02d/%02d %02d:%02d:%02d.%03d [%5dms] %s %d", marker,
				systemtime.wYear, systemtime.wMonth, systemtime.wDay,
				systemtime.wHour, systemtime.wMinute, systemtime.wSecond,
				systemtime.wMilliseconds, msPast, s, num);
		}
		else
		{
			snprintf(tempstr_Win10, 256, "%s %d/%02d/%02d %02d:%02d:%02d.%03d [%5dms] %s", marker,
				systemtime.wYear, systemtime.wMonth, systemtime.wDay,
				systemtime.wHour, systemtime.wMinute, systemtime.wSecond,
				systemtime.wMilliseconds, msPast, s);
		}
		WriteDebugDLL_New(tempstr_Win10);
		tickCount_LastLog = tickCount_LastLog_temp;
}


extern "C" void newCodes_startup_Win10()
{
	extern BOOL b_DebugLog;
	if (b_DebugLog) writeDebugLog_Win10("newCodes_startup_Win10 Called", 999);

	//////extern BOOL b_ImmediateUsageUpdate;

	auto th1 = std::thread([] {internetConnectProf = NetworkInformation::GetInternetConnectionProfile(); });
	auto th2 = std::thread([] {connectProfs = NetworkInformation::GetConnectionProfiles(); });

	granularityForInquiry = DataUsageGranularity::Total;
	networkUsageStatesForInquiry.Shared = TriStates::No;
	networkUsageStatesForInquiry.Roaming = TriStates::No;

	th1.join();
	th2.join();


	
	initializeVolume_Win10();

	int i = CheckWinVersion_Win10();


	b_ToDo_updateConnectProfsInfo_Win10 = FALSE;

	b_StartupCompleted = TRUE;

	if (b_DebugLog) writeDebugLog_Win10("b_StartupCompleted = TRUE", 999);

}



extern "C" void identifyInternetConnectProfNum_Win10()
{
	extern int internetConnectProfNum;
	extern int currentLTEProfNum;
	extern int previousLTEProfNum;


	extern BOOL b_DebugLog;
	if (b_DebugLog) writeDebugLog_Win10("identifyInternetConnectionProfileNumber_Win10 Called", 999);


	internetConnectProfNum = -1;

	if (internetConnectProf != nullptr)
	{
		auto tempInternetConnecProfName = internetConnectProf->ProfileName;

		for (int i = 0; i < numOfConnectProfs; i++)
		{
			if (connectProfName[i] == tempInternetConnecProfName)
			{
				internetConnectProfNum = i;
				//if (b_DebugLog) writeDebugLog_Win10("internetConnectProfNum =", i);
				if (internetConnectProf->IsWwanConnectionProfile)
				{
					{
						currentLTEProfNum = i;
						previousLTEProfNum = i;
						SetMyRegLong("Status_DoNotEdit", "PreviousLTEProfNumber", i);

						char strTemp[256];
						WideCharToMultiByte(CP_THREAD_ACP, 0, connectProfName[i]->Data(), -1, strTemp, 250, NULL, NULL);
						SetMyRegStr("Status_DoNotEdit", "PreviousLTEProfName", strTemp);

						b_LTEProfNum_Confirmed = TRUE;
						if (b_DebugLog) writeDebugLog_Win10("[identifyInternetConnectProfNum_Win10] LTEProfileNumber confirmed :", i);
					}
				}
			}
		}


	}
}


extern "C" BOOL updateConnectProfsInfo_Win10(BOOL b_Detail)	// return value: 1 = error, 0 = success;
{
	int intTemp;

	extern int currentLTEProfNum;
	extern int previousLTEProfNum;
	extern char previousLTEProfName[];

	extern BOOL b_ExistLTEProfile;
	BOOL b_ExistLTEProfile_old;	

	extern BOOL b_ExistMeteredProfile;
	BOOL b_ExistMeteredProfile_old;

	extern char activeSSID[];
	extern char activeAPName[];

	BOOL b_LTEProfNameMatched;


	extern BOOL b_DebugLog;
	if (b_DebugLog) writeDebugLog_Win10("updateConnectProfsInfo_Win10 called, with Detail option = ", b_Detail);

	extern BOOL b_NormalLog;



	auto th1 = std::thread([] {connectProfs = NetworkInformation::GetConnectionProfiles(); });
	th1.join();



	connectedWiFiProfNum = -1;
	connectedLTEProfNum = -1;

	if (connectProfs != nullptr)
	{
		intTemp = (int)connectProfs->Size;
		if (numOfConnectProfs != intTemp)
		{
			numOfConnectProfs = intTemp;
			SetMyRegLong("Status_DoNotEdit", "NumberOfProfiles", numOfConnectProfs);
			if (b_DebugLog) writeDebugLog_Win10("New numOfConnectProfs = ", numOfConnectProfs);
		}
		else
		{
			numOfConnectProfs = intTemp;
		}

		if (numOfConnectProfs > MAX_PROFILE_NUMBER)
		{
			if (b_DebugLog) writeDebugLog_Win10("Too Many ConnectionProfiles, Profiles ignored over ", MAX_PROFILE_NUMBER);
			SetMyRegLong("Status_DoNotEdit", "ExcessNetProfiles", TRUE);
			numOfConnectProfs = MAX_PROFILE_NUMBER;
		}


		b_ExistLTEProfile_old = b_ExistLTEProfile;
		b_ExistMeteredProfile_old = b_ExistMeteredProfile;

		b_ExistLTEProfile = FALSE;
		b_ExistMeteredProfile = FALSE;

		b_LTEProfNameMatched = FALSE;

		for (int i = 0; i < numOfConnectProfs; i++)
		{
			b_LTEProf[i] = FALSE;
			b_WiFiProf[i] = FALSE;


			auto connectProf_work = connectProfs->GetAt(i);
			if (connectProf_work != nullptr)
			{
				connectProfConnectLevel[i] = -1;

				connectProfName[i] = connectProf_work->ProfileName;


				NetworkConnectivityLevel tempConnectLevel;
				std::thread th2;

				if (b_Detail)
				{
					th2 = std::thread([&] {tempConnectLevel = connectProf_work->GetNetworkConnectivityLevel(); });

				}


				if (connectProf_work->IsWwanConnectionProfile)
				{
					b_ExistLTEProfile = TRUE;
					b_LTEProf[i] = TRUE;
					ssidOrApName[i] = connectProf_work->WwanConnectionProfileDetails->AccessPointName;

					if (!b_LTEProfNameMatched)
					{
						currentLTEProfNum = i;

						char strTemp[256];
						WideCharToMultiByte(CP_THREAD_ACP, 0, connectProfName[i]->Data(), -1, strTemp, 250, NULL, NULL);
						
						if (!strcmp(strTemp, previousLTEProfName)) b_LTEProfNameMatched = TRUE;
					}

				}

				if (connectProf_work->IsWlanConnectionProfile)
				{
					b_WiFiProf[i] = TRUE;
					ssidOrApName[i] = connectProf_work->WlanConnectionProfileDetails->GetConnectedSsid();

				}

				auto connectionCost = connectProf_work->GetConnectionCost();
				if (connectionCost != nullptr)
				{
					if ((connectionCost->NetworkCostType == NetworkCostType::Fixed)
						&& connectProf_work->IsWlanConnectionProfile)
					{
						b_LTEProf[i] = TRUE;
						b_ExistMeteredProfile = TRUE;
						if (b_DebugLog) writeDebugLog_Win10("MeteredProfielNumber was found, with #: ", i);
					}
				}


				if (b_Detail)
				{
					th2.join();

					if (tempConnectLevel == NetworkConnectivityLevel::None)
						connectProfConnectLevel[i] = 0;
					else if (tempConnectLevel == NetworkConnectivityLevel::LocalAccess)
						connectProfConnectLevel[i] = 1;
					else if (tempConnectLevel == NetworkConnectivityLevel::ConstrainedInternetAccess)
						connectProfConnectLevel[i] = 2;
					else if (tempConnectLevel == NetworkConnectivityLevel::InternetAccess)
					{
						adapterIDforProf[i] = connectProf_work->NetworkAdapter->NetworkAdapterId;
						connectProfConnectLevel[i] = 3;
						if (b_WiFiProf[i]) connectedWiFiProfNum = i;
						if (b_LTEProf[i] && !b_WiFiProf[i])
						{
							connectedLTEProfNum = i;
							currentLTEProfNum = connectedLTEProfNum;
							b_LTEProfNum_Confirmed = TRUE;

							if (previousLTEProfNum != connectedLTEProfNum)
							{
								previousLTEProfNum = connectedLTEProfNum;
								SetMyRegLong("Status_DoNotEdit", "PreviousLTEProfNumber", connectedLTEProfNum);

								char strTemp[256];
								WideCharToMultiByte(CP_THREAD_ACP, 0, connectProfName[connectedLTEProfNum]->Data(), -1, strTemp, 250, NULL, NULL);
								SetMyRegStr("Status_DoNotEdit", "PreviousLTEProfName", strTemp);
							}
						}
					}
					else
						connectProfConnectLevel[i] = 999;

				}

			}
		}

		if (b_DebugLog)
		{
			if (b_LTEProfNum_Confirmed)
			{
				writeDebugLog_Win10("[updateConnectProfsInfo_Win10] LTEProfileNumber confirmed :", currentLTEProfNum);
			}
			//else
			//{
			//	writeDebugLog_Win10("[updateConnectProfsInfo_Win10] LTEProfileNumber may be", currentLTEProfNum);
			//	writeDebugLog_Win10("[updateConnectProfsInfo_Win10] b_LTEProfNameMatched = ", b_LTEProfNameMatched);
			//}
		}

		if (connectedWiFiProfNum != -1)
			WideCharToMultiByte(CP_THREAD_ACP, 0, ssidOrApName[connectedWiFiProfNum]->Data(), -1, activeSSID, 64, NULL, NULL);
		else
			strcpy(activeSSID, "SSID:N/A");

		if (connectedLTEProfNum != -1)
			WideCharToMultiByte(CP_THREAD_ACP, 0, ssidOrApName[connectedLTEProfNum]->Data(), -1, activeAPName, 64, NULL, NULL);
		else
			strcpy(activeAPName, "APN: N/A");

		if (b_ExistLTEProfile_old != b_ExistLTEProfile)
		{
			SetMyRegLong("Status_DoNotEdit", "ExistLTEProfile", b_ExistLTEProfile);
		}


		if (b_ExistMeteredProfile_old != b_ExistMeteredProfile)
		{
			SetMyRegLong("Status_DoNotEdit", "ExistMeteredProfile", b_ExistMeteredProfile);
		}


		if (b_DebugLog) writeDebugLog_Win10("Refreshed currentLTEProfNum =", currentLTEProfNum);

	}
	else
	{
		if (b_DebugLog) writeDebugLog_Win10("connectProfs is not available", 999);
	}
	return 0;
}




extern "C" BOOL chkExistProf_Win10()
{
	BOOL ret = FALSE;
	extern BOOL b_DebugLog;

	if (b_DebugLog) writeDebugLog_Win10("CheckExistProfie_Win10 Called", 999);

	if (connectProfs != nullptr)
	{
		int temp_int = (int)connectProfs->Size;
	}
	return ret;
}








extern "C" void chkInternetConnectionProfile_Win10()
{
	int newIntenetConnectionStatus_Win10 = -1;

	extern int g_InternetConnectStat_Win10;
	extern char icp_SSID_APName[];
	extern BOOL b_DebugLog;
	DWORD tickCount_LastCalled_temp;
	extern int active_physical_adapter_Win10;
	extern BOOL b_ExistMeteredProfile;
	extern BOOL b_ExistLTEProfile;

	extern int internetConnectProfNum;
	extern BOOL b_MeteredNetNow;



	extern BOOL flag_VPN;
	extern BOOL flag_Ether;
	extern BOOL flag_WiFi;
	extern BOOL flag_LTE;



	if (!b_StartupCompleted) newCodes_startup_Win10();

	if (internetConnectProf != nullptr)
	{
		if (active_physical_adapter_Win10 != -1) newIntenetConnectionStatus_Win10 = 0;

		if (internetConnectProf->IsWwanConnectionProfile) newIntenetConnectionStatus_Win10 = 2;
		{

			if ((internetConnectProf->WwanConnectionProfileDetails != nullptr) && !flag_VPN)
			{
				auto ssid_temp = internetConnectProf->WwanConnectionProfileDetails->AccessPointName;
				const wchar_t* wide_chars = ssid_temp->Data();

				WideCharToMultiByte(CP_THREAD_ACP, 0, wide_chars, -1, icp_SSID_APName, 32, NULL, NULL);

				if (b_ExistLTEProfile == FALSE)
				{
					SetMyRegLong("Status_DoNotEdit", "ExistLTEProfile", TRUE);
					b_ExistLTEProfile = TRUE;
					if (b_DebugLog) writeDebugLog_Win10("WAN Profile was newly found", 999);
				}

			}

		}
		if (internetConnectProf->IsWlanConnectionProfile)
		{
			newIntenetConnectionStatus_Win10 = 1;
			auto connectionCost = internetConnectProf->GetConnectionCost();
			if (connectionCost != nullptr)
			{
				switch (connectionCost->NetworkCostType)
				{
				case NetworkCostType::Unrestricted:
					//					newIntenetConnectionStatus_Win10 = 3;
					break;
				case NetworkCostType::Fixed:
					{
						newIntenetConnectionStatus_Win10 = 4;
					}
					if (b_ExistMeteredProfile == FALSE)
					{
						SetMyRegLong("Status_DoNotEdit", "ExistMeteredProfile", TRUE);
						b_ExistMeteredProfile = TRUE;
						if (b_DebugLog) writeDebugLog_Win10("Metered Profile was newly found", 999);
					}
					break;
				case NetworkCostType::Variable:
					//					newIntenetConnectionStatus_Win10 = 6;
					break;
				case NetworkCostType::Unknown:
					//					newIntenetConnectionStatus_Win10 = 7;
					break;
				}

				if ((internetConnectProf->WlanConnectionProfileDetails != nullptr)&& !flag_VPN)
				{
					auto ssid_temp = internetConnectProf->WlanConnectionProfileDetails->GetConnectedSsid();
					const wchar_t* wide_chars = ssid_temp->Data();

					WideCharToMultiByte(CP_THREAD_ACP, 0, wide_chars, -1, icp_SSID_APName, 32, NULL, NULL);
				}

			}

		}


		//VPN接続状態(flag_VPN=TRUE)ではInternetConnectionProfileを正しく検出できないので、Overrideする。


		if (flag_VPN)
		{
//			wsprintf(icp_SSID_APName, "Connected");
			if (flag_Ether) newIntenetConnectionStatus_Win10 = 0;
			else if (flag_WiFi) 
			{
				newIntenetConnectionStatus_Win10 = 1;
			}
			else if (flag_LTE)
			{
				newIntenetConnectionStatus_Win10 = 2;
			}
		}





	}






	//WAN接続時に、以下の行がたまーに失敗してクラッシュしている可能性
	auto th1 = std::thread([] {internetConnectProf = NetworkInformation::GetInternetConnectionProfile(); });
	th1.join();



	BOOL b_ToDo_identifyInternetConnectProfNum_Win10 = FALSE;


	if (newIntenetConnectionStatus_Win10 != -1)
	{
		if (g_InternetConnectStat_Win10 != newIntenetConnectionStatus_Win10)
		{
			extern BOOL b_DebugLog;
			if (b_DebugLog) writeDebugLog_Win10("[chkInternetConnectionProfile_Win10]New g_InternetConnectStat_Win10 =", newIntenetConnectionStatus_Win10);
			//b_ToDo_updateConnectProfsInfo_Win10 = TRUE;
			b_ToDo_identifyInternetConnectProfNum_Win10 = TRUE;
		}
		else
		{
			if (b_DebugLog) writeDebugLog_Win10("[chkInternetConnectionProfile_Win10] g_InternetConnectStat_Win10 =", newIntenetConnectionStatus_Win10);
		}
	}
	else
	{
		if (b_DebugLog) writeDebugLog_Win10("No Internet Connection (Status = -1)", 999);
	}


	tickCount_LastCalled_temp = GetTickCount();
	tickCount_LastCalled = tickCount_LastCalled_temp;


	if (b_ToDo_identifyInternetConnectProfNum_Win10)
	{
		int prevValue = internetConnectProfNum;
		identifyInternetConnectProfNum_Win10();
		if (b_DebugLog) writeDebugLog_Win10("[chkInternetConnectionProfile_Win10] CurrentInternetProfileNumber is identified as: ", internetConnectProfNum);
		if (internetConnectProfNum != prevValue) SetMyRegLong("Status_DoNotEdit", "CurrentInternetProfileNumber", internetConnectProfNum);
	}
	

	if (newIntenetConnectionStatus_Win10 == 2 || newIntenetConnectionStatus_Win10 == 4)
	{
		b_MeteredNetNow = TRUE;
	}
	
	if ((newIntenetConnectionStatus_Win10 == 0 && active_physical_adapter_Win10 == 0) || newIntenetConnectionStatus_Win10 == 1)
	{
		b_MeteredNetNow = FALSE;
	}



	g_InternetConnectStat_Win10 = newIntenetConnectionStatus_Win10;

}

extern "C" void newCodes_close_Win10()
{
	extern BOOL b_DebugLog;
	extern BOOL b_SoundCapability;

	if (b_DebugLog) writeDebugLog_Win10("newCodes_close_Win10 Called", 999);


	if (b_SoundCapability) {
		defaultDevice_Win10->Release();
		defaultDevice_Win10 = NULL;
	}

	delete internetConnectProf;
	delete connectProfs;

}



extern "C" void saveAndOpenProfTable(BOOL b_Open)
{
	HANDLE hFile;
	DWORD dwWriteSize;
	char fname[MAX_PATH];
	char strTemp[256];
	char strTemp2[256];
	char strTemp3[256];
	int i = 0;
	SYSTEMTIME systemtime;
	GetLocalTime(&systemtime);

	extern BOOL b_DebugLog;
	extern char g_mydir_dll[];

	extern int g_InternetConnectStat_Win10;
	extern int internetConnectProfNum;


	extern BOOL b_ExistLTEProfile;
	extern BOOL b_ExistMeteredProfile;

	extern char strNetworkAdapterDescriptor[];


	if (b_DebugLog) writeDebugLog_Win10("saveAndOpenProfTable, with OpenFile option = ", b_Open);

	i = 0;
		

	
	if (updateConnectProfsInfo_Win10(TRUE)) return;







	strcpy(fname, g_mydir_dll);
	add_title(fname, "NetworkProfileList.txt");

	hFile = CreateFile(
		fname, FILE_APPEND_DATA, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{



		if (numOfConnectProfs == MAX_PROFILE_NUMBER)
		{
			wsprintf(strTemp, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n");
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);
			wsprintf(strTemp, "!!!!! Too many profiles. Delete unused profiles !!!!!\r\n");
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);
			wsprintf(strTemp, "!!!!!             to less than %d              !!!!!\r\n", MAX_PROFILE_NUMBER);
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);
			wsprintf(strTemp, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n\r\n\r\n");
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);
		}


		wsprintf(strTemp, "Network ConnectionProflie List Generated by TClock-Win10\r\n");
		WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);
		snprintf(strTemp, 256, "\t at %02d:%02d:%02d, on %d/%02d/%02d\r\n",
			systemtime.wHour, systemtime.wMinute, systemtime.wSecond, 
			systemtime.wYear, systemtime.wMonth, systemtime.wDay
			);
		WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);


		{
			wsprintf(strTemp, "\r\n", connectedLTEProfNum);
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);

			wsprintf(strTemp, "Number of Network Connection Profiles:\t%d\r\n", numOfConnectProfs);
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);

			if (g_InternetConnectStat_Win10 == -1)
				wsprintf(strTemp, "Current Internet Access Profile #:\tUnavailable\r\n");
			else
				wsprintf(strTemp, "Current Internet Access Profile #:\t%d\r\n", internetConnectProfNum);

			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);
			if (g_InternetConnectStat_Win10 == -1)
				wsprintf(strTemp, "Current Internet Access Mode:\t\tUnavailable\r\n");
			else if (g_InternetConnectStat_Win10 == 0)
				wsprintf(strTemp, "Current Internet Access Mode:\t\tEthernet\r\n");
			else if (g_InternetConnectStat_Win10 == 1)
				wsprintf(strTemp, "Current Internet Access Mode:\t\tWiFi\r\n");
			else if (g_InternetConnectStat_Win10 == 2)
				wsprintf(strTemp, "Current Internet Access Mode:\t\tLTE(WAN)\r\n");
			else if (g_InternetConnectStat_Win10 == 4)
				wsprintf(strTemp, "Current Internet Access Mode:\t\tMetered WiFi\r\n");

			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);



			wsprintf(strTemp, "\r\n", connectedLTEProfNum);
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);


			if (connectedWiFiProfNum == -1)
				wsprintf(strTemp, "Connected WiFi Profile #:\t\tNot available\r\n");
			else
				wsprintf(strTemp, "Connected WiFi Profile #:\t\t%d\r\n", connectedWiFiProfNum);
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);

			if (connectedLTEProfNum == -1)
				wsprintf(strTemp, "Connected LTE(WAN) Profile #:\tNot available\r\n");
			else
				wsprintf(strTemp, "Connected LTE(WAN) Profile #:\t%3d\r\n", connectedLTEProfNum);
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);

			
			wsprintf(strTemp, "\r\n", connectedLTEProfNum);
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);


			if (b_ExistLTEProfile)
				wsprintf(strTemp, "Exist LTE(WAN) Profile:\tYES\r\n");
			else
				wsprintf(strTemp, "Exist LTE(WAN) Profile:\tNO\r\n");
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);

			if (b_ExistMeteredProfile)
				wsprintf(strTemp, "Exist Metered WiFi Profile:\tYES\r\n");
			else
				wsprintf(strTemp, "Exist Metered WiFi Profile:\tNO\r\n");
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);

			wsprintf(strTemp, "===========================\r\n");
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);
		}


		for (i = 0; i < numOfConnectProfs; i++)
		{

			wsprintf(strTemp, "\r\nProfile #: %3d\r\n",i);
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);




			WideCharToMultiByte(CP_THREAD_ACP, 0, connectProfName[i]->Data(), -1, strTemp2, 250, NULL, NULL);
			wsprintf(strTemp, "Profile Name: %s\r\n", strTemp2);
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);

			







			if (connectProfConnectLevel[i] == 0)
				strcpy(strTemp2, "None");
			else if (connectProfConnectLevel[i] == 1)
				strcpy(strTemp2, "LocalAccess");
			else if (connectProfConnectLevel[i] == 2)
				strcpy(strTemp2, "ConstrainedInternetAccess");
			else if (connectProfConnectLevel[i] == 3)
				strcpy(strTemp2, "InternetAccess");

			wsprintf(strTemp, "NetworkConnectivityLevel: %d (%s)\r\n",
				connectProfConnectLevel[i], strTemp2);
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);

			if (connectProfConnectLevel[i] == 3)
			{

				wsprintf(strTemp, "Adapter GUID: %08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\r\n"
					, adapterIDforProf[i].Data1, adapterIDforProf[i].Data2, adapterIDforProf[i].Data3
					, adapterIDforProf[i].Data4[0], adapterIDforProf[i].Data4[1], adapterIDforProf[i].Data4[2], adapterIDforProf[i].Data4[3]
					, adapterIDforProf[i].Data4[4], adapterIDforProf[i].Data4[5], adapterIDforProf[i].Data4[6], adapterIDforProf[i].Data4[7]);
				WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);

				if (Net_getAdapterDescriptor(adapterIDforProf[i]))
				{
					wsprintf(strTemp, "Adapter Descriptor: %s\r\n",
						strNetworkAdapterDescriptor);
					WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);
				}
			}

			strcpy(strTemp2, "");
			if (b_WiFiProf[i] && b_LTEProf[i]) strcpy(strTemp2, "Metered WiFi");
			else if (b_WiFiProf[i]) strcpy(strTemp2, "WiFi");
			else if (b_LTEProf[i]) strcpy(strTemp2, "LTE(WAN)");
			else strcpy(strTemp2, "Non-WiFi, Non-LTE");
			wsprintf(strTemp, "Type: %s\r\n", strTemp2);
			WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);

			if (b_WiFiProf[i] || b_LTEProf[i])
			{
				WideCharToMultiByte(CP_THREAD_ACP, 0, ssidOrApName[i]->Data(), -1, strTemp2, 250, NULL, NULL);
				if (!strlen(strTemp2)) strcpy(strTemp2, "N/A");

				if (b_WiFiProf[i])
				{
					wsprintf(strTemp, "Current SSID: %s\r\n", strTemp2);
				}
				else
				{
					wsprintf(strTemp, "AccessPoint Name: %s\r\n", strTemp2);
				}

				WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);

			}


		}

		CloseHandle(hFile);
	}

	if (b_Open) ShellExecute(NULL, "open", "notepad.exe", fname, NULL, SW_SHOWNORMAL);

}