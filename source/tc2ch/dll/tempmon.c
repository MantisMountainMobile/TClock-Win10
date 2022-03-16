/*-------------------------------------------------------------------------
  tempmon.c
  get performance monitor counter
  Kazubon 2001 (permon.c) -> MantisMountainMobile 2022
---------------------------------------------------------------------------*/

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
//#include <sysinfoapi.h>
#include "tcdll.h"

#define MAX_THERMALZONE_COUNTER               32

//moved to tcdll.h
//void TempMoni_start(void);
//int TempMoni_get(void);
//void TempMoni_end(void);

//int permon[4] = { 0, 0, 0, 0 };

// temp counter
int pdhTemperature = 0;
BOOL b_TempAvailable = FALSE;

int numThermalZone = 0;
int indexSelectedThermalZone = 0;

static PDH_HCOUNTER hTempCounter = NULL;
static PDH_HCOUNTER hThermalZoneCounter[MAX_THERMALZONE_COUNTER] = { NULL };

static HMODULE hmodPDH = NULL;
static PDH_HQUERY hQueryTemp = NULL;

typedef PDH_STATUS (WINAPI *pfnPdhOpenQueryW)(LPCWSTR, DWORD_PTR, PDH_HQUERY*);
typedef PDH_STATUS (WINAPI *pfnPdhAddCounterW)(PDH_HQUERY, LPCWSTR, DWORD_PTR, PDH_HCOUNTER*);
typedef PDH_STATUS (WINAPI *pfnPdhCollectQueryData)(PDH_HQUERY);
typedef PDH_STATUS (WINAPI *pfnPdhGetFormattedCounterValue)(PDH_HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE);
typedef PDH_STATUS (WINAPI *pfnPdhCloseQuery)(PDH_HQUERY);
typedef PDH_STATUS (WINAPI *pfnPdhRemoveCounter)(PDH_HCOUNTER);
typedef PDH_STATUS(WINAPI *pfnPdhAddCounterA)(PDH_HQUERY, LPCSTR, DWORD_PTR, PDH_HCOUNTER*);

static pfnPdhOpenQueryW pPdhOpenQueryW;
static pfnPdhAddCounterW pPdhAddCounterW;
static pfnPdhCollectQueryData pPdhCollectQueryData;
static pfnPdhGetFormattedCounterValue pPdhGetFormattedCounterValue;
static pfnPdhCloseQuery pPdhCloseQuery;
static pfnPdhRemoveCounter pPdhRemoveCounter;
static pfnPdhAddCounterA pPdhAddCounterA;

extern BOOL b_DebugLog;

void TempMoni_start(void)
{
	int i;

	if (b_DebugLog) writeDebugLog_Win10("[permon.c]TemoMoni_start called.", 999);


	//if (hmodPDH) PerMoni_end();
	if(hQueryTemp)PerMoni_end();

	if (!hmodPDH) {
		hmodPDH = LoadLibrary("pdh.dll");
		if (hmodPDH == NULL) return;

		pPdhOpenQueryW = (pfnPdhOpenQueryW)GetProcAddress(hmodPDH, "PdhOpenQueryW");
		pPdhAddCounterW = (pfnPdhAddCounterW)GetProcAddress(hmodPDH, "PdhAddCounterW");
		pPdhAddCounterA = (pfnPdhAddCounterA)GetProcAddress(hmodPDH, "PdhAddCounterA");
		pPdhRemoveCounter = (pfnPdhRemoveCounter)GetProcAddress(hmodPDH, "PdhRemoveCounter");
		pPdhCollectQueryData = (pfnPdhCollectQueryData)GetProcAddress(hmodPDH, "PdhCollectQueryData");
		pPdhGetFormattedCounterValue = (pfnPdhGetFormattedCounterValue)GetProcAddress(hmodPDH, "PdhGetFormattedCounterValue");
		pPdhCloseQuery = (pfnPdhCloseQuery)GetProcAddress(hmodPDH, "PdhCloseQuery");

		if (pPdhOpenQueryW == NULL ||
			pPdhAddCounterW == NULL ||
			pPdhAddCounterA == NULL ||
			pPdhCollectQueryData == NULL ||
			pPdhRemoveCounter == NULL ||
			pPdhGetFormattedCounterValue == NULL ||
			pPdhCloseQuery == NULL)
		{
			goto FAILURE_PDH_COUNTER_INITIALIZATION;
		}
	}

	if(pPdhOpenQueryW(NULL, 0, &hQueryTemp) != ERROR_SUCCESS)
	{
		goto FAILURE_PDH_COUNTER_INITIALIZATION;
	}
	
	// create temperature counter
	//if (pPdhAddCounterA(hQueryTemp, "\\Thermal Zone Information(\\_TZ.THM0)\\Temperature", 0, &hTempCounter) != ERROR_SUCCESS)	//should be "%" because of not using wsprintfW
	//{
	//	if (b_DebugLog) writeDebugLog_Win10("[permon.c][TempMoni_start] Performance Counter for Temperature registoration failed.", 999);
	//}
	
	SetThermalZoneCounter();



	if(pPdhCollectQueryData(hQueryTemp) != ERROR_SUCCESS)
	{
		goto FAILURE_PDH_COUNTER_INITIALIZATION;
	}

	if (b_DebugLog) writeDebugLog_Win10("[permon.c][TempMoni_start] PDH_Counter successfully initialized.", 999);
	return; /* SUCCESS */


FAILURE_PDH_COUNTER_INITIALIZATION:
	hQueryTemp = NULL;
	FreeLibrary(hmodPDH); hmodPDH = NULL;
	if (b_DebugLog) writeDebugLog_Win10("[permon.c][TempMoni_start] PDH_Counter initialization Failed.", 999);
	return; /* FAILURE */
}

int TempMoni_get(int zone)
{
	double tempDouble = 0.0;
	long tempLong = 0;
	int i = 0, ret;
	PDH_FMT_COUNTERVALUE FmtValue;

	if (zone >= numThermalZone) {
		return 0;
	}

	//if (b_DebugLog) writeDebugLog_Win10("[gpumon.c] TempMoni_get called", 999);

	if (hQueryTemp) {
		pPdhCollectQueryData(hQueryTemp);
//		if (pPdhGetFormattedCounterValue(hTempCounter, PDH_FMT_LONG | PDH_FMT_1000, NULL, &FmtValue) == ERROR_SUCCESS)
		if (pPdhGetFormattedCounterValue(hThermalZoneCounter[zone], PDH_FMT_LONG | PDH_FMT_1000, NULL, &FmtValue) == ERROR_SUCCESS)
		{
			ret = (int)((FmtValue.longValue - 272650)/1000);	//272650 = (+0.5 -273.15)*1000		for case of "Temperature performance counter

			if (ret < 0) ret = 0;

			b_TempAvailable = TRUE;
		}
	}
	else {
		b_TempAvailable = FALSE;
	}


	return ret;
}

void TempMoni_end(void)
{

	if (hQueryTemp)
	{
		pPdhCloseQuery(hQueryTemp);
		hQueryTemp = NULL;
	}

	if(hmodPDH)
	{
		FreeLibrary(hmodPDH);
		hmodPDH = NULL;
	}
}



int SetThermalZoneCounter(void)
{
	//https://gist.github.com/fecf/2103a82afc76b5c88829c4383944a5aa
	//https://docs.microsoft.com/en-us/windows/win32/perfctrs/enumerating-process-objects

	extern BOOL b_DebugLog;
	if (b_DebugLog)writeDebugLog_Win10("[newCodes_Win10.cpp] SetThermalZoneCounter called.", 999);
	int count = 0;
	CONST PSTR COUNTER_OBJECT = "Thermal Zone Information";


	PDH_STATUS status = ERROR_SUCCESS;
	LPSTR pwsCounterListBuffer = NULL;
	DWORD dwCounterListSize = 0;
	LPSTR pwsInstanceListBuffer = NULL;
	DWORD dwInstanceListSize = 0;
	LPSTR pTemp = NULL;

	// Determine the required buffer size for the data. 
	status = PdhEnumObjectItemsA(
		NULL,                   // real-time source
		NULL,                   // local machine
		COUNTER_OBJECT,         // object to enumerate
		pwsCounterListBuffer,   // pass NULL and 0
		&dwCounterListSize,     // to get required buffer size
		pwsInstanceListBuffer,
		&dwInstanceListSize,
		PERF_DETAIL_WIZARD,     // counter detail level
		0);

	if (status == PDH_MORE_DATA)
	{
		// Allocate the buffers and try the call again.
		pwsCounterListBuffer = (LPSTR)malloc(dwCounterListSize * sizeof(CHAR));
		pwsInstanceListBuffer = (LPSTR)malloc(dwInstanceListSize * sizeof(CHAR));

		if (NULL != pwsCounterListBuffer && NULL != pwsInstanceListBuffer)
		{
			status = PdhEnumObjectItemsA(
				NULL,                   // real-time source
				NULL,                   // local machine
				COUNTER_OBJECT,         // object to enumerate
				pwsCounterListBuffer,
				&dwCounterListSize,
				pwsInstanceListBuffer,
				&dwInstanceListSize,
				PERF_DETAIL_WIZARD,     // counter detail level
				0);

			if (status == ERROR_SUCCESS)
			{
				if (hQueryTemp) {
					char counterName[PDH_MAX_COUNTER_PATH];
					numThermalZone = 0;

					for (pTemp = pwsInstanceListBuffer; *pTemp != 0; pTemp += strlen(pTemp) + 1)
					{
						//							if (strstr(pTemp, "_0_eng_0_engtype_3D") || strstr(pTemp, "_0_eng_0_engtype_VideoDecode"))	//Currently use limited instances
						snprintf(counterName, PDH_MAX_COUNTER_PATH, "\\Thermal Zone Information(%s)\\Temperature", pTemp);	//shoud be "%%" because of using wsprintfW
//						if (b_DebugLog) writeDebugLog_Win10(counterName, numThermalZone);
						if (pPdhAddCounterA(hQueryTemp, counterName, 0, &hThermalZoneCounter[numThermalZone]) == ERROR_SUCCESS)
						{
							numThermalZone++;
							if (numThermalZone >= MAX_THERMALZONE_COUNTER)
							{
								break;
							}
						}
					}

					if (indexSelectedThermalZone >= numThermalZone) {
						indexSelectedThermalZone = 0;
						SetMyRegLong("ETC", "SelectedThermalZone", indexSelectedThermalZone);
					}

					if (b_DebugLog) {
						writeDebugLog_Win10("[newCodes_Win10.cpp][SetThermalZoneCounter] Number of registered ThermalZone counters = ", numThermalZone);
					}
				}
			}
		}
	}


	if (pwsCounterListBuffer != NULL)
		free(pwsCounterListBuffer);

	if (pwsInstanceListBuffer != NULL)
		free(pwsInstanceListBuffer);

	return numThermalZone;
}