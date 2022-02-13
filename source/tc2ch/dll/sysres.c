/*-------------------------------------------------------------------------
  sysres.c
  GetFreeSystemResources
  Kazubon 1999
  cf. http://www2.justnet.ne.jp/~tyche/samples/sysres.html
      http://ftp.digital.com/pub/published/oreilly/windows/win95.update/k32exp.html
---------------------------------------------------------------------------*/

#include <windows.h>

#if defined(_DEBUG)
# include <crtdbg.h>
#else
# define _ASSERTE(expr) ((void)0)
#endif


#define MAX_PROCESSOR               64

typedef HINSTANCE (WINAPI *pfnLoadLibrary16)(LPCSTR);
typedef void (WINAPI *pfnFreeLibrary16)(HINSTANCE);
typedef DWORD (WINAPI *pfnGetProcAddress16)(HINSTANCE, LPCSTR);
typedef void (WINAPI *pfnQT_Thunk)(DWORD);

pfnLoadLibrary16 LoadLibrary16 = NULL;
pfnFreeLibrary16 FreeLibrary16 = NULL;
pfnGetProcAddress16 GetProcAddress16 = NULL;
pfnQT_Thunk QT_Thunk = NULL;

typedef BOOL (WINAPI *pfnGetSystemPowerStatus)(LPSYSTEM_POWER_STATUS);
typedef ULONG (WINAPI *pfnCallNtPowerInformation)(POWER_INFORMATION_LEVEL, PVOID, ULONG, PVOID, ULONG);
pfnGetSystemPowerStatus pGetSystemPowerStatus = NULL;
pfnCallNtPowerInformation pCallNtPowerInformation = NULL;

static BOOL bInitSysres = FALSE;
static HMODULE hmodKERNEL32 = NULL;
static HMODULE hmodUSER16 = NULL;
static HMODULE hmodPowrprof = NULL;
static DWORD dwNumberOfProcessors = 0;
DWORD dwGetFreeSystemResources = 0;

int blt_h, blt_m, blt_s, pw_mode;
BYTE bat_flag;	//added for charge status by TTTT
BOOL b_Charging = FALSE;

extern int iCPUClock[];
extern int MaxCPUClock[];
extern BOOL b_DebugLog;
extern BOOL b_EnableClock2;

typedef struct _PROCESSOR_POWER_INFORMATION {
	ULONG Number;
	ULONG MaxMhz;
	ULONG CurrentMhz;
	ULONG MhzLimit;
	ULONG MaxIdleState;
	ULONG CurrentIdleState;
} PROCESSOR_POWER_INFORMATION, *PPROCESSOR_POWER_INFORMATION;

#define ENEWHDR  0x003CL    /* offset of new EXE header */
#define EMAGIC   0x5A4D     /* old EXE magic id:  'MZ'  */
#define PEMAGIC  0x4550     /* NT portable executable */

#define GET_DIR(x)  (hdr->OptionalHeader.DataDirectory[x].VirtualAddress)



/*----------------------------------------------------
  load 16bit "USER.EXE" and get procedure address of
  "GetFreeSystemResources"
------------------------------------------------------*/
void InitSysres(void)
{
	if(bInitSysres) return;

	bInitSysres = TRUE;

}

/*----------------------------------------------------
  free libraries
------------------------------------------------------*/
void EndSysres(void)
{
	if(hmodKERNEL32) FreeLibrary(hmodKERNEL32); hmodKERNEL32 = NULL;
	if(hmodUSER16) FreeLibrary16(hmodUSER16); hmodUSER16 = NULL;
}



void InitBatteryLife(void)
{
	DWORD ver;

	//ver = GetVersion();
	//if((!(ver & 0x80000000) && LOBYTE(LOWORD(ver)) < 5)) // NT 4
	//	return;

	if(hmodKERNEL32 == NULL)
		hmodKERNEL32 = LoadLibrary("KERNEL32.dll");
	if(hmodKERNEL32 == NULL) return;

	pGetSystemPowerStatus = (pfnGetSystemPowerStatus)
		GetProcAddress(hmodKERNEL32, "GetSystemPowerStatus");
}



void CheckBatteryAvailability(void)
{
	SYSTEM_POWER_STATUS sps;
	extern BOOL b_DebugLog;
	extern BOOL b_BatteryLifeAvailable;


	if (pGetSystemPowerStatus == NULL) InitBatteryLife();

	if (pGetSystemPowerStatus)
	{
		if (pGetSystemPowerStatus(&sps))
		{
			if ((sps.BatteryFlag & 0x80) || (sps.BatteryFlag & 0xF0))
			{
				b_BatteryLifeAvailable = FALSE;
			}
			else
			{
				b_BatteryLifeAvailable = TRUE;
			}
		}
	}
	if (b_DebugLog) writeDebugLog_Win10("[tclock.c][CheckBatteryAvailability] b_BatteryLifeAvailable =", b_BatteryLifeAvailable);
}

int GetBatteryLifePercent(void)
{
	SYSTEM_POWER_STATUS sps;
	extern BOOL b_DebugLog;
	//if (b_DebugLog) writeDebugLog_Win10("GetBatteryLifePercent called.", 999);


	if(pGetSystemPowerStatus == NULL) InitBatteryLife();

	if(pGetSystemPowerStatus)
	{
		if(pGetSystemPowerStatus(&sps))
		{
			BYTE b;
			DWORD blt;

			b = sps.BatteryLifePercent;
			blt = sps.BatteryLifeTime;
			pw_mode = (int)sps.ACLineStatus;
			bat_flag = sps.BatteryFlag;	//added for charge status by TTTT

			if((int)blt == -1)
			{
				blt_h = 99;
				blt_m = 99;
				blt_s = 99;
			}
			else
			{
				blt_h = (int)blt/3600;
				blt_m = ((int)blt - blt_h*3600)/60;
				blt_s = ((int)blt - blt_h*3600 - blt_m*60);
			}

			b_Charging = bat_flag & 0x08;

			return (int)b;
		}
		else return 255;
	}
	return 255;
}

void FreeBatteryLife(void)
{
	if(hmodKERNEL32) FreeLibrary(hmodKERNEL32);
	hmodKERNEL32 = NULL;
	pGetSystemPowerStatus = NULL;
}

void InitCpuClock(void)
{
	DWORD ver;
	SYSTEM_INFO si;


	if(hmodPowrprof == NULL)
		hmodPowrprof = LoadLibrary("Powrprof.dll");
	if (hmodPowrprof == NULL) {
		if (b_DebugLog) writeDebugLog_Win10("[sysres.c][InitCpuClock()] Loading Powrprof.dll Failed.", 999);
		return;
	}
	else {
		if (b_DebugLog) writeDebugLog_Win10("[sysres.c][InitCpuClock()] Loading Powrprof.dll succeeded.", 999);
	}

	pCallNtPowerInformation = (pfnCallNtPowerInformation)
		GetProcAddress(hmodPowrprof, "CallNtPowerInformation");

	GetSystemInfo(&si);
	dwNumberOfProcessors = si.dwNumberOfProcessors;
	if (dwNumberOfProcessors > MAX_PROCESSOR) dwNumberOfProcessors = MAX_PROCESSOR;


	if (pCallNtPowerInformation)
	{
		PROCESSOR_POWER_INFORMATION ppi[MAX_PROCESSOR];
		ULONG status;	// NTSTATUS
		DWORD i;

		status = pCallNtPowerInformation(ProcessorInformation, NULL, 0, &ppi[0], sizeof(ppi));
		if (status != 0) {
			if (b_DebugLog) writeDebugLog_Win10("[sysres.c][UpdateCpuClock()] Getting Processor Information Failed.", 999);
			return; // != STATUS_SUCCESS
		}


		for (i = 0; i<dwNumberOfProcessors; ++i)
		{
			MaxCPUClock[i] = ppi[i].MaxMhz;
		}
	}


}


void UpdateCpuClock(void)
{
	if(pCallNtPowerInformation == NULL) InitCpuClock();


	if(pCallNtPowerInformation && !b_EnableClock2)
	{
		PROCESSOR_POWER_INFORMATION ppi[MAX_PROCESSOR];
		ULONG status;	// NTSTATUS
		DWORD i;

		status = pCallNtPowerInformation(ProcessorInformation, NULL, 0, &ppi[0], sizeof(ppi));
		if (status != 0){
			if (b_DebugLog) writeDebugLog_Win10("[sysres.c][UpdateCpuClock()] Getting Processor Information Failed.", 999);
			return; // != STATUS_SUCCESS
		}


		for (i=0; i<dwNumberOfProcessors; ++i)
		{
			iCPUClock[i] = ppi[i].CurrentMhz;
		}
	}
}


void FreeCpuClock(void)
{
	if(hmodPowrprof) FreeLibrary(hmodPowrprof);
	hmodPowrprof = NULL;
	pCallNtPowerInformation = NULL;
}

