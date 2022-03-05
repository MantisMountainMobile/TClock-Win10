/*-------------------------------------------------------------------------
  permon.c
  get performance monitor counter
  Kazubon 2001
---------------------------------------------------------------------------*/

#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
//#include <sysinfoapi.h>
#define MAX_PROCESSOR               64
#include "tcdll.h"


void PerMoni_start(void);
int PerMoni_get(void);
void PerMoni_end(void);

//int permon[4] = { 0, 0, 0, 0 };

// cpu usage
int CPUUsage[MAX_PROCESSOR] = { 0 };
static PDH_HCOUNTER hCPUCounter[MAX_PROCESSOR] = { NULL };



int CPUClock2[MAX_PROCESSOR] = { 0 };						//20201230, TTTT
static PDH_HCOUNTER hCPUClock2[MAX_PROCESSOR] = { NULL };	//20201230, TTTT
int CPUClock2Ave = 0;										//20201230, TTTT

int MaxCPUClock[MAX_PROCESSOR] = { 0 };						//20201230, TTTT
BOOL b_EnableClock2 = FALSE;

// cpu total usage
extern int totalCPUUsage;
static PDH_HCOUNTER hTotalCPUCounter = NULL;

int nLogicalProcessors = 0;
int nCores = 0;


static HMODULE hmodPDH = NULL;
static PDH_HQUERY hQuery = NULL;

typedef PDH_STATUS (WINAPI *pfnPdhOpenQueryW)(LPCWSTR, DWORD_PTR, PDH_HQUERY*);
typedef PDH_STATUS (WINAPI *pfnPdhAddCounterW)(PDH_HQUERY, LPCWSTR, DWORD_PTR, PDH_HCOUNTER*);
typedef PDH_STATUS (WINAPI *pfnPdhCollectQueryData)(PDH_HQUERY);
typedef PDH_STATUS (WINAPI *pfnPdhGetFormattedCounterValue)(PDH_HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE);
typedef PDH_STATUS (WINAPI *pfnPdhCloseQuery)(PDH_HQUERY);
typedef PDH_STATUS (WINAPI *pfnPdhRemoveCounter)(PDH_HCOUNTER);


static pfnPdhOpenQueryW pPdhOpenQueryW;
static pfnPdhAddCounterW pPdhAddCounterW;
static pfnPdhCollectQueryData pPdhCollectQueryData;
static pfnPdhGetFormattedCounterValue pPdhGetFormattedCounterValue;
static pfnPdhCloseQuery pPdhCloseQuery;
static pfnPdhRemoveCounter pPdhRemoveCounter;

extern BOOL b_DebugLog;

void PerMoni_start(void)
{
	int i;

	//nLogicalProcessors = atoi(getenv("NUMBER_OF_PROCESSORS"));

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	nLogicalProcessors = (int)si.dwNumberOfProcessors;
	if (nLogicalProcessors > MAX_PROCESSOR) nLogicalProcessors = MAX_PROCESSOR;

	//éQçl: https://wlog.flatlib.jp/item/878
	DWORD retlen = 0;
	GetLogicalProcessorInformation(NULL, &retlen);
	if (retlen <= sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)*MAX_PROCESSOR) {
		SYSTEM_LOGICAL_PROCESSOR_INFORMATION infobuffer[MAX_PROCESSOR];
		if (GetLogicalProcessorInformation(infobuffer, &retlen))
		{
			SYSTEM_LOGICAL_PROCESSOR_INFORMATION *infop = infobuffer;
			nCores = 0;
			for (DWORD size = 0; size < retlen; size += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION), infop++)
			{
				if (infop->Relationship == RelationProcessorCore) {
					nCores++;
				}
			}
		}
	}
	if (b_DebugLog) writeDebugLog_Win10("[permon.c][PerMoni_start] Number of Physical Cores = ", nCores);
	if (b_DebugLog) writeDebugLog_Win10("[permon.c][PerMoni_start] Number of Logical Processors = ", nLogicalProcessors);

	

	//if (hmodPDH) PerMoni_end();
	if(hQuery)PerMoni_end();

	if (!hmodPDH) {
		hmodPDH = LoadLibrary("pdh.dll");
		if (hmodPDH == NULL) return;

		pPdhOpenQueryW = (pfnPdhOpenQueryW)GetProcAddress(hmodPDH, "PdhOpenQueryW");
		pPdhAddCounterW = (pfnPdhAddCounterW)GetProcAddress(hmodPDH, "PdhAddCounterW");
		pPdhRemoveCounter = (pfnPdhRemoveCounter)GetProcAddress(hmodPDH, "PdhRemoveCounter");
		pPdhCollectQueryData = (pfnPdhCollectQueryData)GetProcAddress(hmodPDH, "PdhCollectQueryData");
		pPdhGetFormattedCounterValue = (pfnPdhGetFormattedCounterValue)GetProcAddress(hmodPDH, "PdhGetFormattedCounterValue");
		pPdhCloseQuery = (pfnPdhCloseQuery)GetProcAddress(hmodPDH, "PdhCloseQuery");

		if (pPdhOpenQueryW == NULL ||
			pPdhAddCounterW == NULL ||
			pPdhCollectQueryData == NULL ||
			pPdhRemoveCounter == NULL ||
			pPdhGetFormattedCounterValue == NULL ||
			pPdhCloseQuery == NULL)
		{
			goto FAILURE_PDH_COUNTER_INITIALIZATION;
		}
	}

	if(pPdhOpenQueryW(NULL, 0, &hQuery) != ERROR_SUCCESS)
	{
		goto FAILURE_PDH_COUNTER_INITIALIZATION;
	}
	

	// create cpu counter
	for(i=0; i< nLogicalProcessors; i++)
	{
		wchar_t counterName[64];
		wsprintfW(counterName, L"\\Processor Information(0,%d)\\%% Processor Utility", i);	//shoud be "%%" because of using wsprintfW
		if(pPdhAddCounterW(hQuery, counterName, 0, &hCPUCounter[i]) != ERROR_SUCCESS)
		{
			if (b_DebugLog && i == 0) writeDebugLog_Win10("[permon.c][PerMoni_start] Processor Information(N) Performance Counter unavailable. Try for Processor(*).", 999);
			wsprintfW(counterName, L"\\Processor(%d)\\%% Processor Time", i);	//shoud be "%%" because of using wsprintfW
			if (pPdhAddCounterW(hQuery, counterName, 0, &hCPUCounter[i]) != ERROR_SUCCESS)
			{
				goto FAILURE_PDH_COUNTER_INITIALIZATION;
			}
		}

		wsprintfW(counterName, L"\\Processor Information(0,%d)\\%% of Maximum Frequency", i);	//shoud be "%%" because of using wsprintfW
		if (pPdhAddCounterW(hQuery, counterName, 0, &hCPUClock2[i]) != ERROR_SUCCESS)
		{
			b_EnableClock2 = FALSE;
			if (b_DebugLog && i == 0) writeDebugLog_Win10("[permon.c][PerMoni_start] Performance Counter for Core Clocks registoration failed. iClock[](sysres.c) is used.", 999);
		}
		else
		{
			b_EnableClock2 = TRUE;
			if (b_DebugLog && i == 0) writeDebugLog_Win10("[permon.c][PerMoni_start] Performance Counter for Core Clocks successfully registered. Clock2[](permon.c) is used.", 999);
		}
	}


	// create total cpu usage counter
	if (pPdhAddCounterW(hQuery, L"\\Processor Information(_Total)\\% Processor Utility",0, &hTotalCPUCounter) != ERROR_SUCCESS)	//should be "%" because of not using wsprintfW
	{
		if (b_DebugLog) writeDebugLog_Win10("[permon.c][PerMoni_start] Processor Information(_Total) Performance Counter unavailable. Try for Processor(*).", 999);
		if (pPdhAddCounterW(hQuery, L"\\Processor(_Total)\\% Processor Time", 0, &hTotalCPUCounter) != ERROR_SUCCESS) {//should be "%" because of not using wsprintfW
			goto FAILURE_PDH_COUNTER_INITIALIZATION;
		}
	}

	if(pPdhCollectQueryData(hQuery) != ERROR_SUCCESS)
	{
		goto FAILURE_PDH_COUNTER_INITIALIZATION;
	}

	if (b_DebugLog) writeDebugLog_Win10("[permon.c][PerMoni_start] PDH_Counter successfully initialized.", 999);
	return; /* SUCCESS */


FAILURE_PDH_COUNTER_INITIALIZATION:
	hQuery = NULL;
	FreeLibrary(hmodPDH); hmodPDH = NULL;
	if (b_DebugLog) writeDebugLog_Win10("[permon.c][PerMoni_start] PDH_Counter initialization Failed.", 999);
	return; /* FAILURE */
}

int PerMoni_get(void)
{
	int i;


	if(hmodPDH)
	{
		PDH_FMT_COUNTERVALUE FmtValue;

		pPdhCollectQueryData(hQuery);

		//for(i=0; i<4; i++)
		//{
		//	if(pPdhGetFormattedCounterValue(hCounter[i], PDH_FMT_DOUBLE, NULL, &FmtValue) == ERROR_SUCCESS)
		//	{
		//		permon[i] = (int)(FmtValue.doubleValue/1024);
		//	}
		//	else
		//	{
		//		permon[i] = 0;
		//	}
		//}


		CPUClock2Ave = 0;

		// get cpu counter
		for(i=0; i<nLogicalProcessors; i++)
		{
			if(pPdhGetFormattedCounterValue(hCPUCounter[i], PDH_FMT_DOUBLE, NULL, &FmtValue) == ERROR_SUCCESS)
			{
				CPUUsage[i] = (int)(FmtValue.doubleValue +0.5);
			}
			else
			{
				CPUUsage[i] = 0;
			}

			if (b_EnableClock2) {
				if (pPdhGetFormattedCounterValue(hCPUClock2[i], PDH_FMT_DOUBLE, NULL, &FmtValue) == ERROR_SUCCESS)
				{
					CPUClock2[i] = (int)(FmtValue.doubleValue * MaxCPUClock[i] / 100);
					CPUClock2Ave += CPUClock2[i];
				}
				else
				{
					CPUClock2[i] = 0;
				}
			}
		}

		CPUClock2Ave /= nLogicalProcessors;

		// get total cpu usage
		if(pPdhGetFormattedCounterValue(hTotalCPUCounter, PDH_FMT_DOUBLE, NULL, &FmtValue) == ERROR_SUCCESS)
		{
			totalCPUUsage = (int)(FmtValue.doubleValue + 0.5);
			if (totalCPUUsage > 100)totalCPUUsage = 100;
		}
		else
		{
			totalCPUUsage = 0;
		}
	}
	return -1;
}

void PerMoni_end(void)
{

	if (hQuery)
	{
		pPdhCloseQuery(hQuery);
		hQuery = NULL;
	}

	if(hmodPDH)
	{
		FreeLibrary(hmodPDH);
		hmodPDH = NULL;
	}
}
