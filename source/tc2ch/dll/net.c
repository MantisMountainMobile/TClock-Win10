/*-------------------------------------------------------------------------
  net.c
  get network interface info
---------------------------------------------------------------------------*/

#include <windows.h>
#include <iprtrmib.h>
#include <string.h>

#define TRAFFIC_THRESHOLD	256

void Net_start(void);
void Net_get(void);
void Net_end(void);
void Net_clear_Win10(void);
void Net_getIP_Win10(void);
void Net_UpdateInterfaceTable(void);

static HMODULE hmodIPHLP = NULL;
static HGLOBAL *buffer = NULL;
static MIB_IFROW *ifr;
static MIB_IFTABLE *ift;
static int count;
static int sec = 5;


//net[21] was changed from double to unsigned int on 20191024, it may cause any unintentional reaction!!
double net[21] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; //Extended for Ether/WiFi/LTE indicator by TTTT
int ifIndex_Ether, ifIndex_WiFi, ifIndex_LTE, ifIndex_VPN;

typedef DWORD (WINAPI *pfnGetIfTable)(PMIB_IFTABLE,PULONG,BOOL);
typedef DWORD (WINAPI *pfnGetIfEntry)(PMIB_IFROW);
static pfnGetIfTable pGetIfTable;
static pfnGetIfEntry pGetIfEntry;

extern LONG GetMyRegLong(char* section, char* entry, LONG defval);

//extern BOOL b_UseDataPlanFunction;

BOOL flag_Ether, flag_WiFi, flag_LTE, flag_VPNCheck;
BOOL flag_SoftEther = FALSE;
BOOL flag_VPN = FALSE;

int active_physical_adapter_Win10 = -1; //-1: No active adapter, 0: Ether, 1: WiFi, 2: LTE

extern char strSoftEtherKeyword[];
extern char strVPN_Keyword1[];
extern char strVPN_Keyword2[];
extern char strVPN_Keyword3[];
extern char strVPN_Keyword4[];
extern char strVPN_Keyword5[];

extern char strVPN_Exclude1[];
extern char strVPN_Exclude2[];
extern char strVPN_Exclude3[];
extern char strVPN_Exclude4[];
extern char strVPN_Exclude5[];


extern char strEthernet_Keyword1[];
extern char strEthernet_Keyword2[];
extern char strEthernet_Keyword3[];
extern char strEthernet_Keyword4[];
extern char strEthernet_Keyword5[];

// IP addresses added by TTTT
extern char ipLTE[];
extern char ipEther[];
extern char ipWiFi[];
extern char ipVPN[];

extern BOOL b_DebugLog;


#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))


/* Variables used by GetIpAddrTable */
PMIB_IPADDRTABLE pIPAddrTable;
DWORD dwSize = 32 * sizeof(MIB_IPADDRTABLE);
DWORD dwRetVal = 0;

double recv_old, send_old;

int number_ActiveNet;
int number_ActiveNet_old;


typedef DWORD(WINAPI *pfnGetIpAddrTable)(PMIB_IPADDRTABLE, PULONG, BOOL);

static pfnGetIpAddrTable pGetIpAddrTable;

char  strNetworkAdapterDescriptor[MAXLEN_IFDESCR];

extern int megabytesInGigaByte;
int bytesInGigaByte;



void Net_getIP_Win10(void)
{



	if (ift && hmodIPHLP)
	{

		//pGetIpAddrTable = (pfnGetIpAddrTable)GetProcAddress(hmodIPHLP, "GetIpAddrTable");

		if (pIPAddrTable) {
			if (pGetIpAddrTable(pIPAddrTable, &dwSize, 0) ==
				ERROR_INSUFFICIENT_BUFFER) {
				FREE(pIPAddrTable);
				pIPAddrTable = (MIB_IPADDRTABLE *)MALLOC(dwSize);
			}
			if (pIPAddrTable == NULL) {
				if (b_DebugLog) writeDebugLog_Win10("[net.c][Net_getIP_Win10] Memory allocation failed for GetIpAddrTable", 999);
			}

		}


		if ((dwRetVal = pGetIpAddrTable(pIPAddrTable, &dwSize, 0)) == NO_ERROR)
		{
			int i;
			char buf_Str[32];
			DWORD buf_IPAddr = 0;


			for (i = 0; i < pIPAddrTable->dwNumEntries; i++)
			{
				buf_IPAddr = pIPAddrTable->table[i].dwAddr;
				if (pIPAddrTable->table[i].dwIndex == ifIndex_Ether)
				{
					sprintf(buf_Str, "%3d.%3d.%3d.%3d"
						, (int)((LOWORD(buf_IPAddr) & 0x00FF))
						, (int)((LOWORD(buf_IPAddr) & 0xFF00) >> 8)
						, (int)((HIWORD(buf_IPAddr) & 0x00FF))
						, (int)((HIWORD(buf_IPAddr) & 0xFF00)>> 8)	
					);
					strcpy(ipEther, buf_Str);
				}
				else if (pIPAddrTable->table[i].dwIndex == ifIndex_WiFi)
				{
					sprintf(buf_Str, "%3d.%3d.%3d.%3d"
						, (int)((LOWORD(buf_IPAddr) & 0x00FF))
						, (int)((LOWORD(buf_IPAddr) & 0xFF00) >> 8)
						, (int)((HIWORD(buf_IPAddr) & 0x00FF))
						, (int)((HIWORD(buf_IPAddr) & 0xFF00) >> 8)
					);
					strcpy(ipWiFi, buf_Str);
				}
				else if (pIPAddrTable->table[i].dwIndex == ifIndex_LTE)
				{
					sprintf(buf_Str, "%3d.%3d.%3d.%3d"
						, (int)((LOWORD(buf_IPAddr) & 0x00FF))
						, (int)((LOWORD(buf_IPAddr) & 0xFF00) >> 8)
						, (int)((HIWORD(buf_IPAddr) & 0x00FF))
						, (int)((HIWORD(buf_IPAddr) & 0xFF00) >> 8)
					);
					strcpy(ipLTE, buf_Str);
				}
				else if (pIPAddrTable->table[i].dwIndex == ifIndex_VPN)
				{
					sprintf(buf_Str, "%3d.%3d.%3d.%3d"
						, (int)((LOWORD(buf_IPAddr) & 0x00FF))
						, (int)((LOWORD(buf_IPAddr) & 0xFF00) >> 8)
						, (int)((HIWORD(buf_IPAddr) & 0x00FF))
						, (int)((HIWORD(buf_IPAddr) & 0xFF00) >> 8)
					);
					strcpy(ipVPN, buf_Str);
				}
			}

		}
		else
		{
			if (b_DebugLog) writeDebugLog_Win10("[net.c][Net_getIP_Win10] GetIpAddrTable failed with error: ", (int)dwRetVal);
			return;
		}
	}

}



void Net_clear_Win10(void)
{
	net[2] = 0;
	net[3] = 0;

	//Added for LTE indicator by TTTT (3 lines)
	net[12] = 0;
	//Added for Ether / WiFi indicator by TTTT (6 lines)
	net[15] = 0;
	net[18] = 0;

	flag_Ether = flag_WiFi = flag_LTE = flag_VPNCheck = FALSE;
	flag_SoftEther = flag_VPN = FALSE;

	active_physical_adapter_Win10 = -1;

	ifIndex_Ether = ifIndex_WiFi = ifIndex_LTE = ifIndex_VPN = -1;

	strcpy(ipLTE, "--- --- --- ---");
	strcpy(ipWiFi, "--- --- --- ---");
	strcpy(ipEther, "--- --- --- ---");
	strcpy(ipVPN, "--- --- --- ---");
}

void Net_start(void)
{
	DWORD bufsize;
	DWORD iftable;



	recv_old = 0, 	
	send_old = 0;

	net[0] = 0;
	net[1] = 0;




	bytesInGigaByte = 1024 * 1024 * megabytesInGigaByte;



	number_ActiveNet = 0;
	number_ActiveNet_old = 0;

	if (hmodIPHLP) Net_end();
	ifr = NULL;
	ift = NULL;

	hmodIPHLP = LoadLibrary("iphlpapi.dll");
	if (hmodIPHLP == NULL) return;

	pGetIfTable = (pfnGetIfTable)GetProcAddress(hmodIPHLP, "GetIfTable");
	pGetIfEntry = (pfnGetIfEntry)GetProcAddress(hmodIPHLP, "GetIfEntry");

	pGetIpAddrTable = (pfnGetIpAddrTable)GetProcAddress(hmodIPHLP, "GetIpAddrTable");


	if (pGetIfTable == NULL || pGetIfEntry == NULL)
	{
		FreeLibrary(hmodIPHLP); hmodIPHLP = NULL;
		return;
	}

	bufsize = 0;
	iftable = pGetIfTable((PMIB_IFTABLE)buffer, &bufsize, TRUE);
	if (iftable != ERROR_INSUFFICIENT_BUFFER)
	{
		return;
	}



	buffer = GlobalAlloc(GPTR, bufsize);

	if (buffer == NULL)
	{
		return;
	}
	iftable = pGetIfTable((PMIB_IFTABLE)buffer, &bufsize, TRUE);
	if (iftable != NO_ERROR)
	{
		GlobalFree(buffer); buffer = NULL;
		return;
	}

	ift = (MIB_IFTABLE *)buffer;
	count = ift->dwNumEntries;

	pIPAddrTable = (MIB_IPADDRTABLE *)MALLOC(dwSize);

	sec = 1;

	Net_get();



}




BOOL Net_getAdapterDescriptor(GUID targetGUID)
{
	if (b_DebugLog) writeDebugLog_Win10("[net.c] Net_getAdapterDescriptor called", 999);

	strcpy(strNetworkAdapterDescriptor, "");
	if (ift && hmodIPHLP)
	{
		int i, j;
		WCHAR tempwszName[MAX_INTERFACE_NAME_LEN];
		count = ift->dwNumEntries;
		char char_tempwszName[64];
		char strTempGUID[64];
		wsprintf(strTempGUID, "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"
			, targetGUID.Data1, targetGUID.Data2, targetGUID.Data3
			, targetGUID.Data4[0], targetGUID.Data4[1], targetGUID.Data4[2], targetGUID.Data4[3]
			, targetGUID.Data4[4], targetGUID.Data4[5], targetGUID.Data4[6], targetGUID.Data4[7]);

		if (b_DebugLog) writeDebugLog_Win10(strTempGUID, 999);

		for (i = 0; i < count; i++)
		{
			ifr = (ift->table) + i;
			pGetIfEntry(ifr);

			wcscpy(tempwszName, ifr->wszName);

			for (j = 0; j < 36; j++)
			{
				char_tempwszName[j] = (char)tempwszName[j + 15];
				//if (strNetworkAdapterDescriptor[j] == '}')
				//{
				//	strNetworkAdapterDescriptor[j] = '\0';
				//	break;
				//}
			}
			char_tempwszName[36] = '\0';

			//if (b_DebugLog)
			//{
			//	writeDebugLog_Win10(char_tempwszName, i);
			//}

			if (!strncmp(char_tempwszName, strTempGUID, 36))
			{
				strcpy(strNetworkAdapterDescriptor, &ifr->bDescr);
				if (b_DebugLog)
				{
					char tempStr[MAXLEN_IFDESCR + 64];
					wsprintf(tempStr, "Network Adapter: %s", strNetworkAdapterDescriptor);
					writeDebugLog_Win10(tempStr, 999);
				}
				return 1;
			}
		}
	}
	return 0;
}


void Net_getRecvSend_Win10(double* recv, double* send, double* recvWAN, double* sendWAN, double* recvWiFi, double* sendWiFi, double* recvEther, double* sendEther)
{

	flag_Ether = flag_WiFi = flag_LTE = flag_VPNCheck = FALSE;
	flag_SoftEther = flag_VPN = FALSE;
	number_ActiveNet = 0;



	if (ift && hmodIPHLP)
	{
		int i;

		*recv = *send = 0;
		*recvWAN = *sendWAN = 0;	//Added for LTE indicator by TTTT
		*recvWiFi = *sendWiFi = 0;	//Added for WiFi indicator by TTTT
		*recvEther = *sendEther = 0;	//Added for Ethernet indicator by TTTT

		count = ift->dwNumEntries;


		for (i = 0; i < count; i++)
		{
			ifr = (ift->table) + i;
			pGetIfEntry(ifr);
			//Added for LTE indicator by TTTT
			if (!flag_LTE)
			{
				if (((ifr->dwType == IF_TYPE_WWANPP ||
					ifr->dwType == IF_TYPE_WWANPP2)) &&
					(ifr->dwOperStatus == IF_OPER_STATUS_OPERATIONAL))
				{
					flag_LTE = TRUE;
					number_ActiveNet++;

//					pGetIfEntry(ifr);
					*recv += ifr->dwInOctets;
					*send += ifr->dwOutOctets;

					net[12] = 1;
					*recvWAN += ifr->dwInOctets;
					*sendWAN += ifr->dwOutOctets;

					ifIndex_LTE = i+1;	//âΩåÃÇ©1Çë´Ç∑Ç∆çáÇ§

				}
			}

			//Added for WiFi indicator by TTTT
			if (!flag_WiFi)
			{
				if ((ifr->dwType == IF_TYPE_IEEE80211) &&
					(ifr->dwOperStatus == IF_OPER_STATUS_OPERATIONAL))
				{
					flag_WiFi = TRUE;
					number_ActiveNet++;


//					pGetIfEntry(ifr);
					*recv += ifr->dwInOctets;
					*send += ifr->dwOutOctets;

					net[15] = 1;
					*recvWiFi += ifr->dwInOctets;
					*sendWiFi += ifr->dwOutOctets;

					ifIndex_WiFi = i+1;	//âΩåÃÇ©1Çë´Ç∑Ç∆çáÇ§


				}
			}


			//Added for Ethenet indicator by TTTT
			if (!flag_Ether)
			{
				if ((ifr->dwType == MIB_IF_TYPE_ETHERNET) &&
					(	(strstr(ifr->bDescr, "Ethernet") != NULL)
						|| (strstr(ifr->bDescr, "Realtek") != NULL)
						|| (strstr(ifr->bDescr, "BUFFALO") != NULL)
						|| (strstr(ifr->bDescr, "Buffalo") != NULL)
						|| (strstr(ifr->bDescr, "I-O DATA") != NULL)
						|| (strstr(ifr->bDescr, "Qualcomm") != NULL)
						|| (strstr(ifr->bDescr, strEthernet_Keyword1) != NULL)
						|| (strstr(ifr->bDescr, strEthernet_Keyword2) != NULL)
						|| (strstr(ifr->bDescr, strEthernet_Keyword3) != NULL)
						|| (strstr(ifr->bDescr, strEthernet_Keyword4) != NULL)
						|| (strstr(ifr->bDescr, strEthernet_Keyword5) != NULL)
						) &&
					(strstr(ifr->bDescr, "Virtual") == NULL) &&
					(strstr(ifr->bDescr, "Scheduler") == NULL) &&
					(ifr->dwOperStatus == IF_OPER_STATUS_OPERATIONAL))
				{
					flag_Ether = TRUE;
					number_ActiveNet++;


//					pGetIfEntry(ifr);
					*recv += ifr->dwInOctets;
					*send += ifr->dwOutOctets;

					net[18] = 1;
					*recvEther += ifr->dwInOctets;
					*sendEther += ifr->dwOutOctets;

					ifIndex_Ether = i+1;	//âΩåÃÇ©1Çë´Ç∑Ç∆çáÇ§


				}
			}

			//Added for VPNê⁄ë± indicator by TTTT
			if (!flag_VPNCheck)
			{
//				if ((ifr->dwType == MIB_IF_TYPE_ETHERNET) &&
				//if (((ifr->dwType == MIB_IF_TYPE_ETHERNET) 
				//		|| (ifr->dwType == MIB_IF_TYPE_PPP))
				//	&&
				if (
					((strstr(ifr->bDescr, strSoftEtherKeyword) != NULL) 
						|| (strstr(ifr->bDescr, strVPN_Keyword1) != NULL)
						|| (strstr(ifr->bDescr, strVPN_Keyword2) != NULL)
						|| (strstr(ifr->bDescr, strVPN_Keyword3) != NULL)
						|| (strstr(ifr->bDescr, strVPN_Keyword4) != NULL)
						|| (strstr(ifr->bDescr, strVPN_Keyword5) != NULL)) &&
					((strstr(ifr->bDescr, strVPN_Exclude1) == NULL)
						&& (strstr(ifr->bDescr, strVPN_Exclude2) == NULL)
						&& (strstr(ifr->bDescr, strVPN_Exclude3) == NULL)
						&& (strstr(ifr->bDescr, strVPN_Exclude4) == NULL)
						&& (strstr(ifr->bDescr, strVPN_Exclude5) == NULL)) &&
						(ifr->dwOperStatus == IF_OPER_STATUS_OPERATIONAL))
				{
					flag_VPNCheck = TRUE;
					number_ActiveNet++;


					flag_VPN = TRUE;
					if (strstr(ifr->bDescr, strSoftEtherKeyword) != NULL) flag_SoftEther = TRUE;

					ifIndex_VPN = i+1;	//âΩåÃÇ©1Çë´Ç∑Ç∆çáÇ§


				}
			}

		}
	}
	else
	{
		*recv = *send = 0;
		*recvWAN = *sendWAN = 0;	//Added for LTE indicator by TTTT
		*recvWiFi = *sendWiFi = 0;	//Added for WiFi indicator by TTTT
		*recvEther = *sendEther = 0;	//Added for Ethernet indicator by TTTT
	}

}


void Net_get(void)
{

	extern BOOL b_DebugLog;
	//if (b_DebugLog) writeDebugLog_Win10("[net.c] Net_get called", 999);


	if (ift && hmodIPHLP)
	{
		if (sec <= 0) sec = 5;

		Net_clear_Win10();


		double recv, send;
		double recvWAN, sendWAN; //Added for LTE indicator by TTTT
		double recvWiFi, sendWiFi; //Added for WiFi indicator by TTTT
		double recvEther, sendEther; //Added for Ether indicator by TTTT

		Net_UpdateInterfaceTable(); //Added to conform dyanamic change of interface table by TTTT 20190723

		Net_getRecvSend_Win10(&recv, &send, &recvWAN, &sendWAN, &recvWiFi, &sendWiFi, &recvEther, &sendEther); //Added for Ether/WiFi/LTE indicator by TTTT

		net[2] = (recv - recv_old) / sec;

		net[3] = (send - send_old) / sec;

		if (number_ActiveNet != number_ActiveNet_old)
		{
			net[2] = 0;
			net[3] = 0;
		}

		if ((net[2] < 0) || (net[2] > bytesInGigaByte)) net[2] = 0;
		if ((net[3] < 0) || (net[3] > bytesInGigaByte)) net[3] = 0;

		number_ActiveNet_old = number_ActiveNet;

		recv_old = recv;
		send_old = send;


		net[6] = net[2] / 1024;
		net[7] = net[3] / 1024;

		net[10] = net[2] / 1048576;
		net[11] = net[3] / 1048576;

		net[0] += (net[2] * sec);
		net[1] += (net[3] * sec);

		net[4] = net[0] / 1024;
		net[5] = net[1] / 1024;

		net[8] = net[0] / 1048576;
		net[9] = net[1] / 1048576;






		//Added for LTE indicator by TTTT (3 lines)
		if ((recvWAN - net[13]) > TRAFFIC_THRESHOLD) net[12] = 2;
		if ((sendWAN - net[14]) > TRAFFIC_THRESHOLD) net[12] = 2;
		net[13] = recvWAN;
		net[14] = sendWAN;
		//Added for WiFi indicator by TTTT (3 lines)
		if ((recvWiFi - net[16]) > TRAFFIC_THRESHOLD) net[15] = 2;
		if ((sendWiFi - net[17]) > TRAFFIC_THRESHOLD) net[15] = 2;
		net[16] = recvWiFi;
		net[17] = sendWiFi;

		if (((recvEther - net[19]) > TRAFFIC_THRESHOLD) || ((sendEther - net[20]) > TRAFFIC_THRESHOLD))
		{
			if (net[18] == 1) net[18] = 2;
		}

		net[19] = recvEther;
		net[20] = sendEther;

		active_physical_adapter_Win10 = -1;
		if (net[18] > 0) active_physical_adapter_Win10 = 0;
		else if (net[15] > 0)active_physical_adapter_Win10 = 1;
		else if (net[12] > 0)active_physical_adapter_Win10 = 2;

		Net_getIP_Win10();

	}
	
}

void Net_end(void)
{
	if(hmodIPHLP)
	{
		FreeLibrary(hmodIPHLP);
	}
	if(buffer)
	{
		GlobalFree(buffer);
	}

	hmodIPHLP = NULL; buffer = NULL;

	if (pIPAddrTable == NULL) FREE(pIPAddrTable);
}

//void Net_restart(void)
//{
//	DWORD bufsize;
//	DWORD iftable;
//	int i;
//
//
//	GlobalFree(buffer); buffer=NULL;
//	if(pGetIfTable == NULL || pGetIfEntry == NULL)return;
//	bufsize = 0;
//	iftable = pGetIfTable((PMIB_IFTABLE)buffer, &bufsize, TRUE);
//	if(iftable != ERROR_INSUFFICIENT_BUFFER)
//	{
//		FreeLibrary(hmodIPHLP); hmodIPHLP = NULL;
//		return;
//	}
//	buffer = GlobalAlloc(GPTR, bufsize);
//	if(buffer == NULL)return;
//	iftable = pGetIfTable((PMIB_IFTABLE)buffer, &bufsize, TRUE);
//	if(iftable != NO_ERROR)
//	{
//		GlobalFree(buffer); buffer = NULL;
//		return;
//	}
//	ift = (MIB_IFTABLE *)buffer;
//	count = ift->dwNumEntries;
//
//	Net_get();
//
//	sec = 1;
//}


void Net_UpdateInterfaceTable(void)
{
	DWORD bufsize;
	DWORD iftable;


	if (hmodIPHLP)
	{
		ifr = NULL;
		ift = NULL;

		bufsize = 0;
		iftable = pGetIfTable((PMIB_IFTABLE)buffer, &bufsize, TRUE);
		if (iftable != ERROR_INSUFFICIENT_BUFFER)
		{
			return;
		}

		if (buffer)
		{
			GlobalFree(buffer);
		}

		buffer = GlobalAlloc(GPTR, bufsize);

		if (buffer == NULL)
		{
			return;
		}

		iftable = pGetIfTable((PMIB_IFTABLE)buffer, &bufsize, TRUE);
		if (iftable != NO_ERROR)
		{
			GlobalFree(buffer); buffer = NULL;
			return;
		}

		ift = (MIB_IFTABLE *)buffer;
		count = ift->dwNumEntries;

		pIPAddrTable = (MIB_IPADDRTABLE *)MALLOC(dwSize);
	}

}
