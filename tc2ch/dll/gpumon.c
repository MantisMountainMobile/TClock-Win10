/*-------------------------------------------------------------------------
  gpumon.c
  get performance monitor counter
  Kazubon 2001 (permon.c) -> MantisMountainMobile 2022
---------------------------------------------------------------------------*/

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
//#include <sysinfoapi.h>
#include "tcdll.h"

//#pragma comment(lib, "pdh.lib")
#define MAX_GPU_COUNTER               4096

static PDH_HCOUNTER hGPUCounter[MAX_GPU_COUNTER] = { NULL };
int numberGPUCounter = 0;
int numPDHGPUInstance = 0;

extern BOOL b_DebugLog;
extern int totalGPUUsage;
static PDH_HQUERY hQueryGPU = NULL;
static HMODULE hmodPDH = NULL;


typedef PDH_STATUS(WINAPI *pfnPdhOpenQueryW)(LPCWSTR, DWORD_PTR, PDH_HQUERY*);
typedef PDH_STATUS(WINAPI *pfnPdhAddCounterW)(PDH_HQUERY, LPCWSTR, DWORD_PTR, PDH_HCOUNTER*);
typedef PDH_STATUS(WINAPI *pfnPdhCollectQueryData)(PDH_HQUERY);
typedef PDH_STATUS(WINAPI *pfnPdhGetFormattedCounterValue)(PDH_HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE);
typedef PDH_STATUS(WINAPI *pfnPdhCloseQuery)(PDH_HQUERY);
typedef PDH_STATUS(WINAPI *pfnPdhRemoveCounter)(PDH_HCOUNTER);
typedef PDH_STATUS(WINAPI *pfnPdhAddCounterA)(PDH_HQUERY, LPCSTR, DWORD_PTR, PDH_HCOUNTER*);

static pfnPdhOpenQueryW pPdhOpenQueryW;
static pfnPdhAddCounterW pPdhAddCounterW;
static pfnPdhCollectQueryData pPdhCollectQueryData;
static pfnPdhGetFormattedCounterValue pPdhGetFormattedCounterValue;
static pfnPdhCloseQuery pPdhCloseQuery;
static pfnPdhRemoveCounter pPdhRemoveCounter;
static pfnPdhAddCounterA pPdhAddCounterA;

void GPUMoni_start(void)
{
	if (b_DebugLog) writeDebugLog_Win10("[gpumon.c] GPUMoni_start called.", 999);


	int i;

	if (hQueryGPU)GPUMoni_end();

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




	if (pPdhOpenQueryW(NULL, 0, &hQueryGPU) != ERROR_SUCCESS)
	{
		goto FAILURE_PDH_COUNTER_INITIALIZATION;
	}


	SetGPUUsageCounter();
	//// create gpu counter

	//AddGPUUsageCounter();


//	if (b_DebugLog) writeDebugLog_Win10("[gpumon.c][GPUMoni_start] PDH_CounterGPU successfully initialized.", 999);
	return; /* SUCCESS */


FAILURE_PDH_COUNTER_INITIALIZATION:
	hQueryGPU = NULL;
	FreeLibrary(hmodPDH); hmodPDH = NULL;
	if (b_DebugLog) writeDebugLog_Win10("[gpumon.c][GPUMoni_start] PDH_CounterGPU initialization Failed.", 999);
	return; /* FAILURE */
}



int SetGPUUsageCounter(void)
{
	//https://gist.github.com/fecf/2103a82afc76b5c88829c4383944a5aa
	//https://docs.microsoft.com/en-us/windows/win32/perfctrs/enumerating-process-objects

	extern BOOL b_DebugLog;
	if (b_DebugLog)writeDebugLog_Win10("[newCodes_Win10.cpp] SetGPUUsageCounter called.", 999);
	int count = 0, numPDHGPUInstanceNew = 0;
	CONST PSTR COUNTER_OBJECT = "GPU Engine";


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
				//wprintf(L"Counters that the Process objects defines:\n\n");

				// Walk the counters list. The list can contain one
				// or more null-terminated strings. The list is terminated
				// using two null-terminator characters.

				//for (pTemp = pwsCounterListBuffer; *pTemp != 0; pTemp += strlen(pTemp) + 1)
				//{
				//	WriteDebugDLL_New(pTemp);
				//	//wprintf(L"%s\n", pTemp);
				//}

				//wprintf(L"\nInstances of the Process object:\n\n");

				// Walk the instance list. The list can contain one
				// or more null-terminated strings. The list is terminated
				// using two null-terminator characters.

				//if (hQueryGPU) {
				//	pPdhCloseQuery(hQueryGPU);
				//	hQueryGPU = NULL;
				//}

				if (hQueryGPU) {
					for (pTemp = pwsInstanceListBuffer; *pTemp != 0; pTemp += strlen(pTemp) + 1)
					{
						numPDHGPUInstanceNew++;
					}

					//if (b_DebugLog) {
					//	writeDebugLog_Win10("[newCodes_Win10.cpp][SetGPUUsageCounter] Number of instances (prev) = ", numPDHGPUInstance);
					//	writeDebugLog_Win10("[newCodes_Win10.cpp][SetGPUUsageCounter] Number of instances = ", numPDHGPUInstanceNew);
					//}

					if (numPDHGPUInstance == numPDHGPUInstanceNew) {
						//if (b_DebugLog) {
						//	writeDebugLog_Win10("[newCodes_Win10.cpp][SetGPUUsageCounter] Number of instances was not changed. GPU counter is not updated ", 999);
						//}
					}
					else {
						numPDHGPUInstance = numPDHGPUInstanceNew;

						pPdhCloseQuery(hQueryGPU);
						if (pPdhOpenQueryW(NULL, 0, &hQueryGPU) != ERROR_SUCCESS)
						{
							hQueryGPU = NULL;
							numberGPUCounter = 0;
							return 0;
						}

						char counterName[PDH_MAX_COUNTER_PATH];
						for (pTemp = pwsInstanceListBuffer; *pTemp != 0; pTemp += strlen(pTemp) + 1)
						{
//							if (strstr(pTemp, "_0_eng_0_engtype_3D") || strstr(pTemp, "_0_eng_0_engtype_VideoDecode"))	//Currently use limited instances
							{
								snprintf(counterName, PDH_MAX_COUNTER_PATH, "\\GPU Engine(%s)\\Utilization Percentage", pTemp);	//shoud be "%%" because of using wsprintfW
								if (pPdhAddCounterA(hQueryGPU, counterName, 0, &hGPUCounter[count]) == ERROR_SUCCESS)
								{
									count++;
									if (count >= MAX_GPU_COUNTER)
									{
										break;
									}
								}
							}
						}

						numberGPUCounter = count;

						if (b_DebugLog) {
							writeDebugLog_Win10("[newCodes_Win10.cpp][SetGPUUsageCounter] Number of instances was changed. GPU counter is updated ", 999);
							writeDebugLog_Win10("[newCodes_Win10.cpp][SetGPUUsageCounter] Number of registered counter items = ", count);
						}
					}
				}
				else {
					if (b_DebugLog) {
						writeDebugLog_Win10("[newCodes_Win10.cpp][SetGPUUsageCounter] hQueryGPU is not available. Try to recureate. ", 999);
					}
					numberGPUCounter = 0;
					numPDHGPUInstance = 0;
					if (pPdhOpenQueryW(NULL, 0, &hQueryGPU) != ERROR_SUCCESS)
					{
						hQueryGPU = NULL;
					}
				}
			}
		}
		else
		{
			//wprintf(L"Unable to allocate buffers.\n");
		}
	}
	else
	{
		//wprintf(L"\nPdhEnumObjectItems failed with 0x%x.\n", status);
	}

	if (pwsCounterListBuffer != NULL)
		free(pwsCounterListBuffer);

	if (pwsInstanceListBuffer != NULL)
		free(pwsInstanceListBuffer);

	return numberGPUCounter;
}

int GPUMoni_get(void)
{
	double tempDouble = 0.0;
	long tempLong = 0;
	int i = 0;
	PDH_FMT_COUNTERVALUE FmtValue;

	//if (b_DebugLog) writeDebugLog_Win10("[gpumon.c] GPUMoni_get called", 999);

	totalGPUUsage = 0;

	if (hQueryGPU) {
		pPdhCollectQueryData(hQueryGPU);
		for (int i = 0; i < numberGPUCounter; i++)
		{
			if (pPdhGetFormattedCounterValue(hGPUCounter[i], PDH_FMT_LONG | PDH_FMT_1000, NULL, &FmtValue) == ERROR_SUCCESS)
			{
				tempLong += FmtValue.longValue;
			}
		}
		totalGPUUsage = (int)((tempLong + 500) / 1000);
		if (totalGPUUsage > 100)totalGPUUsage = 100;
	}
	else {
//		if (b_DebugLog) writeDebugLog_Win10("[gpumon.c][GPUMoni_get] hQueryGPU does not exist!", 999);
	}


//	if (b_DebugLog) writeDebugLog_Win10("[gpumon.c][GPUMoni_get] GPU usage = ", totalGPUUsage);

	if (hmodPDH)SetGPUUsageCounter();

	return totalGPUUsage;
}



void GPUMoni_end(void)
{
	if (b_DebugLog) writeDebugLog_Win10("[gpumon.c] GPUMoni_end called.", 999);

	if (hQueryGPU)
	{
		pPdhCloseQuery(hQueryGPU);
		hQueryGPU = NULL;
	}

	if (hmodPDH)
	{
		FreeLibrary(hmodPDH);
		hmodPDH = NULL;
	}
}
