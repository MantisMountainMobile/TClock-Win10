//---[s]--- For InternetTime 99/03/16@211 M.Takemura -----

/*-----------------------------------------------------
    format.c
    to make a string to display in the clock
    KAZUBON 1997-1998
-------------------------------------------------------*/

#include "tcdll.h"
#include "string.h"
#define MAX_PROCESSOR               64

int codepage = CP_ACP;
int actdvl[36] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static char DayOfWeekShort[11], DayOfWeekLong[31];
static char DayOfWeekShortPrev[11], DayOfWeekLongPrev[31];
static char DayOfWeekShortNext[11], DayOfWeekLongNext[31];
static char MonthShort[11], MonthLong[31];
static char MonthShortPrev[11], MonthLongPrev[31];
static char MonthShortNext[11], MonthLongNext[31];
static char *DayOfWeekEng[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char *MonthEng[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static char AM[11], PM[11], SDate[5], STime[5];
static char EraStr[11];
static int AltYear;

static int ilang;

extern BOOL bHour12, bHourZero;



extern int iFreeRes[3], totalCPUUsage, iBatteryLife, iVolume, totalGPUUsage;
extern int iCPUClock[];
extern int CPUClock2[];
extern int CPUClock2Ave;

extern MEMORYSTATUSEX msMemory;
extern double temperatures[];
extern double voltages[];
extern double fans[];

extern int CPUUsage[];
extern double net[];
extern double diskFree[];
extern double diskAll[];
extern int blt_h, blt_m, blt_s, pw_mode;
extern BYTE bat_flag;	//added for charge status by TTTT
extern BOOL b_Charging;

extern int g_InternetConnectStat_Win10;	//added for Internet Connectionstatus by TTTT
extern char icp_SSID_APName[];


extern int currentLTEProfNum;

extern int internetConnectProfNum;


extern int megabytesInGigaByte;


extern BOOL flag_SoftEther;
extern BOOL flag_VPN;
extern int active_physical_adapter_Win10;



extern char Ver_TClockWin10[];

extern BOOL b_SafeMode;
extern BOOL b_ExcessNetProfiles;

extern BOOL muteStatus;

extern char strLTE[];
extern char charLTE[];
extern char strMute[];

// IP addresses added by TTTT
extern char ipLTE[];
extern char ipEther[];
extern char ipWiFi[];
extern char ipVPN[];
extern char ipActive[];

extern BOOL b_MeteredNetNow;

extern COLORREF colfore;

extern char strDispStatus[];



extern BOOL b_DataPlanRetreveOK;

extern char activeSSID[];
extern char activeAPName[];

extern int NetMIX_Length;
extern int SSID_AP_Length;


extern BOOL b_FlagTimerAdjust;

BOOL b_FlagNextDay;
BOOL b_FlagPrevDay;
BOOL b_FlagNextMonth;
BOOL b_FlagPrevMonth;

BOOL b_SummerTime_US = FALSE;
BOOL b_SummerTime_Europe = FALSE;

BOOL b_exist_DOWzone = FALSE;

extern BOOL b_DebugLog;
extern int nLogicalProcessors;
extern BOOL b_EnableClock2;

extern int numPDHGPUInstance;
extern int pdhTemperature;
extern double pdhTemperatureDouble;
extern BOOL b_TempAvailable;

/*------------------------------------------------
  GetLocaleInfo() for 95/NT
--------------------------------------------------*/
int GetLocaleInfoWA(WORD wLanguageID, LCTYPE LCType, char* dst, int n)
{
	int r;
	LCID Locale;

	*dst = 0;
	Locale = MAKELCID(wLanguageID, SORT_DEFAULT);
	{
		WCHAR* pw;
		pw = (WCHAR*)GlobalAllocPtr(GHND, sizeof(WCHAR)*(n+1));
		*pw = 0;
		r = GetLocaleInfoW(Locale, LCType, pw, n);
		if(r)
			WideCharToMultiByte(codepage, 0, pw, -1, dst, n,
				NULL, NULL);
		GlobalFreePtr(pw);
	}
	return r;
}

/*------------------------------------------------
  GetDateFormat() for 95/NT
--------------------------------------------------*/
int GetDateFormatWA(WORD wLanguageID, DWORD dwFlags, CONST SYSTEMTIME *t,
	char* fmt, char* dst, int n)
{
	int r;
	LCID Locale;

	*dst = 0;
	Locale = MAKELCID(wLanguageID, SORT_DEFAULT);
	{
		WCHAR* pw1, *pw2;
		pw1 = NULL;
		if(fmt)
		{
			pw1 = (WCHAR*)GlobalAllocPtr(GHND,
				sizeof(WCHAR)*(strlen(fmt)+1));
			MultiByteToWideChar(CP_ACP, 0, fmt, -1,
				pw1, strlen(fmt));
		}
		pw2 = (WCHAR*)GlobalAllocPtr(GHND, sizeof(WCHAR)*(n+1));
		r = GetDateFormatW(Locale, dwFlags, t, pw1, pw2, n);
		if(r)
			WideCharToMultiByte(CP_ACP, 0, pw2, -1, dst, n,
				NULL, NULL);
		if(pw1) GlobalFreePtr(pw1);
		GlobalFreePtr(pw2);
	}
	return r;
}

/*------------------------------------------------
  GetTimeFormat() for 95/NT
--------------------------------------------------*/
int GetTimeFormatWA(WORD wLanguageID, DWORD dwFlags, CONST SYSTEMTIME *t,
	char* fmt, char* dst, int n)
{
	int r;
	LCID Locale;

	*dst = 0;
	Locale = MAKELCID(wLanguageID, SORT_DEFAULT);
	{
		WCHAR* pw1, *pw2;
		pw1 = NULL;
		if(fmt)
		{
			pw1 = (WCHAR*)GlobalAllocPtr(GHND,
				sizeof(WCHAR)*(strlen(fmt)+1));
			MultiByteToWideChar(CP_ACP, 0, fmt, -1,
				pw1, strlen(fmt));
		}
		pw2 = (WCHAR*)GlobalAllocPtr(GHND, sizeof(WCHAR)*(n+1));
		r = GetTimeFormatW(Locale, dwFlags, t, pw1, pw2, n);
		if(r)
			WideCharToMultiByte(CP_ACP, 0, pw2, -1, dst, n,
				NULL, NULL);
		if(pw1) GlobalFreePtr(pw1);
		GlobalFreePtr(pw2);
	}
	return r;
}


/*------------------------------------------------
  load strings of day, month
--------------------------------------------------*/
void InitFormat(SYSTEMTIME* lt)
{
	char s[80], *p;
//	int i, ilang, ioptcal;
	int i, ioptcal;

	ilang = GetMyRegLong("Format", "Locale", (int)GetUserDefaultLangID());

	codepage = CP_ACP;
	if(GetLocaleInfoWA((WORD)ilang, LOCALE_IDEFAULTANSICODEPAGE,
		s, 10) > 0)
	{
		p = s; codepage = 0;
		while('0' <= *p && *p <= '9')
			codepage = codepage * 10 + *p++ - '0';
		if(!IsValidCodePage(codepage)) codepage = CP_ACP;
	}

	i = lt->wDayOfWeek;
	if (i > 6) i = 0;
	GetLocaleInfoWA((WORD)ilang, LOCALE_SABBREVDAYNAME1 + i,
		DayOfWeekShortNext, 10);
	GetLocaleInfoWA((WORD)ilang, LOCALE_SDAYNAME1 + i,
		DayOfWeekLongNext, 30);
	i--;
	if(i < 0) i = 6;
	GetLocaleInfoWA((WORD)ilang, LOCALE_SABBREVDAYNAME1 + i,
		DayOfWeekShort, 10);
	GetLocaleInfoWA((WORD)ilang, LOCALE_SDAYNAME1 + i,
		DayOfWeekLong, 30);
	i--;
	if (i < 0) i = 6;
	GetLocaleInfoWA((WORD)ilang, LOCALE_SABBREVDAYNAME1 + i,
		DayOfWeekShortPrev, 10);
	GetLocaleInfoWA((WORD)ilang, LOCALE_SDAYNAME1 + i,
		DayOfWeekLongPrev, 30);

	


	i = lt->wMonth; 
	if (i > 11) i = 0;
	GetLocaleInfoWA((WORD)ilang, LOCALE_SABBREVMONTHNAME1 + i,
		MonthShortNext, 10);
	GetLocaleInfoWA((WORD)ilang, LOCALE_SMONTHNAME1 + i,
		MonthLongNext, 30);
	i--;
	if (i < 0) i = 11;
	GetLocaleInfoWA((WORD)ilang, LOCALE_SABBREVMONTHNAME1 + i,
		MonthShort, 10);
	GetLocaleInfoWA((WORD)ilang, LOCALE_SMONTHNAME1 + i,
		MonthLong, 30);
	i--;
	if (i < 0) i = 11;
	GetLocaleInfoWA((WORD)ilang, LOCALE_SABBREVMONTHNAME1 + i,
		MonthShortPrev, 10);
	GetLocaleInfoWA((WORD)ilang, LOCALE_SMONTHNAME1 + i,
		MonthLongPrev, 30);


	GetLocaleInfoWA((WORD)ilang, LOCALE_S1159, AM, 10);
	GetMyRegStr("Format", "AMsymbol", s, 80, AM);
	if(s[0] == 0) strcpy(s, "AM");
	strcpy(AM, s);
	GetLocaleInfoWA((WORD)ilang, LOCALE_S2359, PM, 10);
	GetMyRegStr("Format", "PMsymbol", s, 80, PM);
	if(s[0] == 0) strcpy(s, "PM");
	strcpy(PM, s);

	GetLocaleInfoWA((WORD)ilang, LOCALE_SDATE, SDate, 4);
	GetLocaleInfoWA((WORD)ilang, LOCALE_STIME, STime, 4);

	EraStr[0] = 0;
	AltYear = -1;

	ioptcal = 0;
	if(GetLocaleInfoWA((WORD)ilang, LOCALE_IOPTIONALCALENDAR,
		s, 10))
	{
		ioptcal = 0;
		p = s;
		while('0' <= *p && *p <= '9')
			ioptcal = ioptcal * 10 + *p++ - '0';
	}
	if(ioptcal < 3) ilang = LANG_USER_DEFAULT;

	if(GetDateFormatWA((WORD)ilang,
		DATE_USE_ALT_CALENDAR, lt, "gg", s, 12) != 0);
		strcpy(EraStr, s);

	if(GetDateFormatWA((WORD)ilang,
		DATE_USE_ALT_CALENDAR, lt, "yyyy", s, 6) != 0)
	{
		if(s[0])
		{
			p = s;
			AltYear = 0;
			while('0' <= *p && *p <= '9')
				AltYear = AltYear * 10 + *p++ - '0';
		}
	}
}

BOOL GetNumFormat(char **sp, char x, char c, int *len, int *slen, BOOL *bComma)
{
	char *p;
	int n, ns;

	p = *sp;
	n = 0;
	ns = 0;

	while (*p == '_')
	{
		ns++;
		p++;
	}
	if (*p != x && *p != c) return FALSE;
	while (*p == x)
	{
		n++;
		p++;
	}
	while (*p == c)
	{
		n++;
		p++;
		*bComma = TRUE;
	}

	*len = n+ns;
	*slen = ns;
	*sp = p;
	return TRUE;
}

int SetNumFormat(char **dp, int n, int len, int slen, BOOL bComma)	//返される文字列の長さは、lenと実際の必要文字数のうち長いほうになる。
{
	char *p;
	int minlen,i,ii;
	int int_max_value = 1000000000; // 10^nしたときに桁あふれを起こさずに処理できる最大値
	int ret;

	p = *dp;

	for (i=10,minlen=1; i<int_max_value +1; i*=10,minlen++)
		if (n < i) break;
	if (bComma)
	{
		if (minlen%3 == 0)
			minlen += minlen/3 - 1;
		else
			minlen += minlen/3;
	}

	if (minlen < len)
	{
		ret = len;
	}
	else
	{
		ret = minlen;
	}

	while (minlen < len)
	{
		if (slen > 0) { *p++ = ' '; slen--; }
		else { *p++ = '0'; }
		len--;
	}
	for (i=minlen-1,ii=1; i>=0; i--,ii++)
	{
		*(p+i) = (char)((n%10)+'0');
		if (ii%3 == 0 && i != 0 && bComma)
			*(p+--i) = ',';
		n/=10;
	}
	p += minlen;

	*dp = p;

	return ret;		//文字数を返答
}


/*------------------------------------------------
   make a string from date and time format
--------------------------------------------------*/
//void MakeFormat(char* s, SYSTEMTIME* pt, int beat100, char* fmt)
void MakeFormat(char* s, char* s_info, SYSTEMTIME* pt, int beat100, char* fmt)
{
	char *sp, *dp, *p, *infop;
	DWORD TickCount = 0;
	SYSTEMTIME disptime;
	BOOL b_WCS_Token = TRUE;
	BOOL b_WCE_Token = TRUE;
	int len_ret;

	sp = fmt; 
	dp = s;
	infop = s_info;

	b_SummerTime_US = FALSE;
	b_SummerTime_Europe = FALSE;

	b_FlagNextDay = FALSE;
	b_FlagPrevDay = FALSE;

	disptime = *pt;



	// added by TTTT for SafeMode notification
	if (b_SafeMode)
	{
		*dp++ = '['; *infop++ = 0x01;
		*dp++ = 'S'; *infop++ = 0x01;
		*dp++ = 'a'; *infop++ = 0x01;
		*dp++ = 'f'; *infop++ = 0x01;
		*dp++ = 'e'; *infop++ = 0x01;
		*dp++ = 'M'; *infop++ = 0x01;
		*dp++ = 'o'; *infop++ = 0x01;
		*dp++ = 'd'; *infop++ = 0x01;
		*dp++ = 'e'; *infop++ = 0x01;
		*dp++ = ']'; *infop++ = 0x01;
	}

	while(*sp)
	{
		if(*sp == '<' && *(sp + 1) == '%')
		{
			sp += 2;
			while(*sp)
			{
				if(*sp == '%' && *(sp + 1) == '>')
				{
					sp += 2;
					break;
				}
				if(*sp == '\"')
				{
					sp++;
					while(*sp != '\"' && *sp)
					{
						p = CharNext(sp);
						while (sp != p) 
						{
							*dp++ = *sp++; *infop++ = 0x01;
						}
					}
					if(*sp == '\"') sp++;
				}
				else if(*sp == '/')
				{
					p = SDate;
					while (*p) {
						*dp++ = *p++; *infop++ = 0x02;
					}
					sp++;
				}
				else if(*sp == ':')
				{
					p = STime;
					while (*p) 
					{
						*dp++ = *p++; *infop++ = 0x08;
					}
					sp++;
				}

				//else if(*sp == 'S' && *(sp + 1) == 'S' && *(sp + 2) == 'S')
				//{
				//	len_ret = SetNumFormat(&dp, (int)disptime.wMilliseconds, 3, 0, FALSE);
				//	sp += 3;
				//}

				else if(*sp == 'y' && *(sp + 1) == 'y')
				{
					int len;
					len = 2;
					if (*(sp + 2) == 'y' && *(sp + 3) == 'y') len = 4;

					len_ret = SetNumFormat(&dp, (len==2)?(int)disptime.wYear%100:(int)disptime.wYear, len, 0, FALSE);
					for (int i = 0; i < len_ret; i++)*infop++ = 0x02;
					sp += len;
				}
				else if(*sp == 'm')
				{
					if(*(sp + 1) == 'm' && *(sp + 2) == 'e')
					{
						*dp++ = MonthEng[disptime.wMonth-1][0]; *infop++ = 0x02;
						*dp++ = MonthEng[disptime.wMonth-1][1]; *infop++ = 0x02;
						*dp++ = MonthEng[disptime.wMonth-1][2]; *infop++ = 0x02;
						sp += 3;
					}
					else if(*(sp + 1) == 'm' && *(sp + 2) == 'm')
					{
						if(*(sp + 3) == 'm')
						{
							if (b_FlagNextMonth) {
								p = MonthLongNext;
							}
							else if (b_FlagPrevMonth) {
								p = MonthLongPrev;
							}
							else {
								p = MonthLong;
							}
							while (*p) {
								*dp++ = *p++; *infop++ = 0x02;
							}
							sp += 4;
						}
						else
						{
							if (b_FlagNextMonth) {
								p = MonthShortNext;
							}
							else if (b_FlagPrevMonth) {
								p = MonthShortPrev;
							}
							else {
								p = MonthShort;
							}

							while (*p) {
								*dp++ = *p++; *infop++ = 0x02;
							}
							sp += 3;
						}
					}
					else
					{
						if(*(sp + 1) == 'm')
						{
							*dp++ = (char)((int)disptime.wMonth / 10) + '0'; *infop++ = 0x02;
							sp += 2;
						}
						else
						{
							if (disptime.wMonth > 9)
							{
								*dp++ = (char)((int)disptime.wMonth / 10) + '0'; *infop++ = 0x02;
							}
							sp++;
						}
						*dp++ = (char)((int)disptime.wMonth % 10) + '0'; *infop++ = 0x02;
					}
				}
				else if(*sp == 'a' && *(sp + 1) == 'a' && *(sp + 2) == 'a')
				{
					if(*(sp + 3) == 'a')
					{
						if (b_FlagNextDay) 
						{
							p = DayOfWeekLongNext;
						}
						else if (b_FlagPrevDay) {
							p = DayOfWeekLongPrev;
						}
						else {
							p = DayOfWeekLong;
						}

						while (*p) 
						{
							*dp++ = *p++; *infop++ = 0x04;
						}
						sp += 4;
					}
					else
					{
						if (b_FlagNextDay) {
							p = DayOfWeekShortNext;
						}else if (b_FlagPrevDay){
							p = DayOfWeekShortPrev;
						}else{
							p = DayOfWeekShort;
						}

						while (*p)
						{
							*dp++ = *p++; *infop++ = 0x04;
						}
						sp += 3;
					}
				}
				else if(*sp == 'd')
				{
					if(*(sp + 1) == 'd' && *(sp + 2) == 'e')
					{
						p = DayOfWeekEng[disptime.wDayOfWeek];
						while (*p)
						{
							*dp++ = *p++; *infop++ = 0x04;
						}
						sp += 3;
					}
					else if(*(sp + 1) == 'd' && *(sp + 2) == 'd')
					{
						if(*(sp + 3) == 'd')
						{
							if (b_FlagNextDay) {
								p = DayOfWeekLongNext;
							}
							else if (b_FlagPrevDay) {
								p = DayOfWeekLongPrev;
							}
							else {
								p = DayOfWeekLong;
							}
							while (*p)
							{
								*dp++ = *p++; *infop++ = 0x04;
							}
							sp += 4;
						}
						else
						{
							if (b_FlagNextDay) {
								p = DayOfWeekShortNext;
							}
							else if (b_FlagPrevDay) {
								p = DayOfWeekShortPrev;
							}
							else {
								p = DayOfWeekShort;
							}

							while (*p)
							{
								*dp++ = *p++; *infop++ = 0x04;
							}
							sp += 3;
						}
					}
					else
					{
						if(*(sp + 1) == 'd')
						{
							*dp++ = (char)((int)disptime.wDay / 10) + '0'; *infop++ = 0x02;
							sp += 2;
						}
						else
						{
							if (disptime.wDay > 9)
							{
								*dp++ = (char)((int)disptime.wDay / 10) + '0'; *infop++ = 0x02;
							}
							sp++;
						}
						*dp++ = (char)((int)disptime.wDay % 10) + '0'; *infop++ = 0x02;
					}
				}
				else if(*sp == 'h')
				{
					int hour;
					hour = disptime.wHour;
					if(bHour12)
					{
						if(hour > 12) hour -= 12;
						else if(hour == 0) hour = 12;
						if(hour == 12 && bHourZero) hour = 0;
					}
					if(*(sp + 1) == 'h')
					{
						*dp++ = (char)(hour / 10) + '0'; *infop++ = 0x08;
						sp += 2;
					}
					else
					{
						if(hour > 9) 
						{
							*dp++ = (char)(hour / 10) + '0'; *infop++ = 0x08;
						}
						sp++;
					}
					*dp++ = (char)(hour % 10) + '0'; *infop++ = 0x08;
				}
				else if (*sp == 'w' )
				{
					char xs_diff[3];
					int xdiff;
					int hour;

					xs_diff[0] = (char)(*(sp+2));
					xs_diff[1] = (char)(*(sp+3));
					xs_diff[2] = (char)'\x0';
					xdiff = atoi( xs_diff );
					if ( *(sp+1) == '-' ) xdiff = -xdiff;
					hour = ( disptime.wHour + xdiff )%24;
					if ( hour < 0 ) hour += 24;
					if ( bHour12 ) 
					{
						if(hour > 12) hour -= 12;
						else if(hour == 0) hour = 12;
						if(hour == 12 && bHourZero) hour = 0;
					}
					*dp++ = (char)(hour / 10) + '0'; *infop++ = 0x08;
					*dp++ = (char)(hour % 10) + '0'; *infop++ = 0x08;
					sp += 4;
				}
				else if(*sp == 'n')
				{
					if(*(sp + 1) == 'n')
					{
						*dp++ = (char)((int)disptime.wMinute / 10) + '0'; *infop++ = 0x08;
						sp += 2;
					}
					else
					{
						if (disptime.wMinute > 9) {
							*dp++ = (char)((int)disptime.wMinute / 10) + '0'; *infop++ = 0x08;
						}
						sp++;
					}
					*dp++ = (char)((int)disptime.wMinute % 10) + '0'; *infop++ = 0x08;
				}
				else if(*sp == 's')
				{
					if(*(sp + 1) == 's')
					{
						*dp++ = (char)((int)disptime.wSecond / 10) + '0'; *infop++ = 0x08;
						sp += 2;
					}
					else
					{
						if (disptime.wSecond > 9)
						{
							*dp++ = (char)((int)disptime.wSecond / 10) + '0'; *infop++ = 0x08;
						}
						sp++;
					}
					*dp++ = (char)((int)disptime.wSecond % 10) + '0'; *infop++ = 0x08;
				}
				else if(*sp == 't' && *(sp + 1) == 't')
				{
					if (disptime.wHour < 12) 
					{
						p = AM;
					}
					else 
					{
						p = PM;
					}
					while (*p) 
					{
						*dp++ = *p++; *infop++ = 0x08;
					}
					sp += 2;
				}
				else if(*sp == 'A' && *(sp + 1) == 'M')
				{
					if(*(sp + 2) == '/' &&
						*(sp + 3) == 'P' && *(sp + 4) == 'M')
					{
						if (disptime.wHour < 12) {
							*dp++ = 'A'; *infop++ = 0x08;
						}
						else 
						{
							*dp++ = 'P'; *infop++ = 0x08;
						}
						*dp++ = 'M'; *infop++ = 0x08;
						sp += 5;
					}
					else if(*(sp + 2) == 'P' && *(sp + 3) == 'M')
					{
						if (disptime.wHour < 12) {
							p = AM;
						}
						else {
							p = PM;
						}
						while (*p) 
						{
							*dp++ = *p++; *infop++ = 0x08;
						}
						sp += 4;
					}
					else {
						*dp++ = *sp++; *infop++ = 0x08;
					}
				}
				else if(*sp == 'a' && *(sp + 1) == 'm' && *(sp + 2) == '/' &&
					*(sp + 3) == 'p' && *(sp + 4) == 'm')
				{
					if (disptime.wHour < 12) {
						*dp++ = 'a'; *infop++ = 0x08;
					}
					else
					{
						*dp++ = 'p'; *infop++ = 0x08;
					}
					*dp++ = 'm'; *infop++ = 0x08;
					sp += 5;
				}
				else if(*sp == '\\' && *(sp + 1) == 'n')
				{
					*dp++ = 0x0d; *infop++ = 0x08;
					*dp++ = 0x0a; *infop++ = 0x08;
					sp += 2;
					b_WCS_Token = TRUE;
					b_WCE_Token = TRUE;
				}
				// internet time
				else if (*sp == '@' && *(sp + 1) == '@' && *(sp + 2) == '@')
				{
					*dp++ = '@'; *infop++ = 0x08;
					*dp++ = (char)( beat100 / 10000 + '0' ); *infop++ = 0x08;
					*dp++ = (char)( (beat100 % 10000) / 1000 + '0' ); *infop++ = 0x08;
					*dp++ = (char)( (beat100 % 1000) / 100 + '0' ); *infop++ = 0x08;
					sp += 3;
					if(*sp == '.' && *(sp + 1) == '@')
					{
						*dp++ = '.'; *infop++ = 0x08;
						*dp++ = (char)(beat100 % 100) / 10 + '0'; *infop++ = 0x08;
						sp += 2;
					}
				}
				// alternate calendar
				else if(*sp == 'Y' && AltYear > -1)
				{
					int n = 1;
					while(*sp == 'Y') { n *= 10; sp++; }
					if(n < AltYear)
					{
						n = 1; while(n < AltYear) n *= 10;
					}
					for (;;)
					{
						*dp++ = (char)( (AltYear % n) / (n/10) + '0' ); *infop++ = 0x02;
						if(n == 10) break;
						n /= 10;
					}
				}
				else if(*sp == 'g')
				{
					char *p2;
					p = EraStr;
					while(*p && *sp == 'g')
					{
						p2 = CharNextExA((WORD)codepage, p, 0);
						while (p != p2) {
							*dp++ = *p++; *infop++ = 0x08;
						}
						sp++;
					}
					while (*sp == 'g') 
					{
						sp++;
					}
				}

				// CPU Usage
				else if(*sp == 'C') 
				{
					if(totalCPUUsage >= 0 && *(sp + 1) == 'U')
					{
						int len, slen;
						BOOL bComma = FALSE;
						int CPU = 0;

						if (isdigit(*(sp + 2)))
						{	// from perfmon
							int processorNum = *(sp + 2) - '0';
							if (processorNum >= nLogicalProcessors)
							{
								CPU = 0;
							}
							else
							{
								CPU = CPUUsage[processorNum];
							}
							sp += 3;
						}
						else if (*(sp + 2) == 'e' && isdigit(*(sp + 3)) && isdigit(*(sp + 4)))
						{
							int processorNum = (*(sp + 3) - '0') * 10 + (*(sp + 4) - '0');
							if (processorNum >= nLogicalProcessors)
							{
								CPU = 0;
							}
							else 
							{
								CPU = CPUUsage[processorNum];
							}
							sp += 5;
						}
						else
						{	// legacy(Total)
							CPU = totalCPUUsage;
							sp += 2;
						}

						if(GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
						{
							len_ret = SetNumFormat(&dp, CPU, len, slen, bComma);
							for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
						}
						else
						{
							if (CPU > 99) {
								*dp++ = (char)((CPU % 1000) / 100 + '0'); *infop++ = 0x01;
							}
							*dp++ = (char)((CPU % 100) / 10 + '0'); *infop++ = 0x01;
							*dp++ = (char)((CPU % 10) + '0'); *infop++ = 0x01;
						}
					}

					// CPU Clock
					else if(*(sp + 1) == 'C')
					{
						int len, slen;
						BOOL bComma = FALSE;
						int clock = 0;

						if (isdigit(*(sp + 2)))
						{	// from perfmon
							int processorNum = *(sp + 2) - '0';
							if (processorNum >= nLogicalProcessors)
							{
								clock = 0;
							}
							else 
							{
								if (b_EnableClock2) 
								{
									clock = CPUClock2[processorNum];
								}
								else 
								{
									clock = iCPUClock[processorNum];
								}
							}
							sp += 3;
						}
						else if (*(sp + 2) == 'e' && isdigit(*(sp + 3)) && isdigit(*(sp + 4)))
						{	// from perfmon
							int processorNum = (*(sp + 3) - '0') *10 + (*(sp + 4) - '0');
							if (processorNum >= nLogicalProcessors)
							{
								clock = 0;
							}
							else {
								if (b_EnableClock2) 
								{
									clock = CPUClock2[processorNum];
								}
								else 
								{
									clock = iCPUClock[processorNum];
								}
							}
							sp += 5;
						}
						else
						{	// legacy(#0)
							if (b_EnableClock2) {
								clock = CPUClock2Ave;
							}
							else {
								clock = iCPUClock[0];
							}
							sp += 2;
						}

							if (GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
							{
								len_ret = SetNumFormat(&dp, clock, len, slen, bComma);
								for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
							}
							else
							{
								len_ret = SetNumFormat(&dp, clock, 3, 0, FALSE);
								for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
							}
					}

					else {
						*dp++ = *sp++;
						*infop++ = 0x01;
					}
				}

				//TEMPERATURE
				else if (*sp == 'T' && (*(sp + 1) == 'E') && (*(sp + 2) == 'M') && (*(sp + 3) == 'P'))
				{
					int len, slen;
					BOOL bComma = FALSE;
					sp += 4;

					if (!b_TempAvailable)
					{
						*dp++ = 'N'; *infop++ = 0x01;
						*dp++ = 'A'; *infop++ = 0x01;
					}
					else
					{
						if (GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
						{
							len_ret = SetNumFormat(&dp, pdhTemperature, len, slen, bComma);
							for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
						}
						else
						{
							if (pdhTemperature > 99) {
								*dp++ = (char)((pdhTemperature % 1000) / 100 + '0'); *infop++ = 0x01;
							}
							*dp++ = (char)((pdhTemperature % 100) / 10 + '0'); *infop++ = 0x01;
							*dp++ = (char)((pdhTemperature % 10) + '0'); *infop++ = 0x01;
						}
					}
				}



				// GPU Usage
				else if (*sp == 'G')
				{
					if (*(sp + 1) == 'U')
					{
						int len, slen;
						BOOL bComma = FALSE;
						sp += 2;

						if (GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
						{
							len_ret = SetNumFormat(&dp, totalGPUUsage, len, slen, bComma);
							for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
						}
						else
						{
							if (totalGPUUsage > 99) {
								*dp++ = (char)((totalGPUUsage % 1000) / 100 + '0'); *infop++ = 0x01;
							}
							*dp++ = (char)((totalGPUUsage % 100) / 10 + '0'); *infop++ = 0x01;
							*dp++ = (char)((totalGPUUsage % 10) + '0'); *infop++ = 0x01;
						}
					}
					else if (*(sp + 1) == 'I')
					{
						int len, slen;
						BOOL bComma = FALSE;
						sp += 2;

						if (GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
						{
							len_ret = SetNumFormat(&dp, numPDHGPUInstance, len, slen, bComma);
							for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
						}
						else
						{
							if (numPDHGPUInstance > 999) {
								*dp++ = (char)((numPDHGPUInstance % 10000) / 1000 + '0'); *infop++ = 0x01;
							}
							else if (numPDHGPUInstance > 99) 
							{
								*dp++ = (char)((numPDHGPUInstance % 1000) / 100 + '0'); *infop++ = 0x01;
							}
							*dp++ = (char)((numPDHGPUInstance % 100) / 10 + '0'); *infop++ = 0x01;
							*dp++ = (char)((numPDHGPUInstance % 10) + '0'); *infop++ = 0x01;
						}			
					}
					else {
						*dp++ = *sp++;
						*infop++ = 0x01;
					}
				}

				// Battery mode
				else if(*sp == 'A' && *(sp + 1) == 'D' ) 
				{
					sp += 2;
					{
						if(pw_mode == 0)
						{
							*dp++ = 'D'; *infop++ = 0x01;
							*dp++ = 'C'; *infop++ = 0x01;
						}
						else if(pw_mode == 1)
						{
							*dp++ = 'A'; *infop++ = 0x01;
							*dp++ = 'C'; *infop++ = 0x01;
						}
						else
						{
							*dp++ = 'U'; *infop++ = 0x01;
							*dp++ = 'N'; *infop++ = 0x01;
						}
					}
				}
				else if(*sp == 'a' && *(sp + 1) == 'd' ) // Battery mode
				{
					sp += 2;
					{
						if(pw_mode == 0)
						{
							*dp++ = 'D'; *infop++ = 0x01;
						}
						else if(pw_mode == 1)
						{
							*dp++ = 'A'; *infop++ = 0x01;
						}
						else
						{
							*dp++ = 'U'; *infop++ = 0x01;
						}
					}
				}

				//added for charge status by TTTT
				else if (*sp == 'B' && *(sp + 1) == 'C' && *(sp + 2) == 'S')
				{
					sp += 3;
					{
						if (b_Charging)
						{
							*dp++ = '*'; *infop++ = 0x01;
						}
						else
						{
							*dp++ = ' '; *infop++ = 0x01;
						}
					}
				}

				//added for time difference by TTTT
				else if (*sp == 't' && ((*(sp + 1) == 'd') || (*(sp + 1) == 'u') || (*(sp + 1) == 'e')) && ((*(sp + 2) == '+') || (*(sp + 2) == '-'))
					&& ((*(sp + 3) >= '0') && (*(sp + 3) <= '2'))
					&& ((*(sp + 4) >= '0') && (*(sp + 4) <= '9'))
					&& (*(sp + 5) == ':')
					&& ((*(sp + 6) >= '0') && (*(sp + 6) <= '5'))
					&& ((*(sp + 7) >= '0') && (*(sp + 7) <= '9'))
					)
				{
					int td_hour = 0;
					int td_min = 0;
					BOOL td_neg = FALSE;
					if (*(sp + 2) == '-') td_neg = TRUE;
					td_hour = (*(sp + 3) - '0') * 10 + (*(sp + 4) - '0');
					td_min = (*(sp + 6) - '0') * 10 + (*(sp + 7) - '0');

					if (td_hour > 23) {
						td_hour = 0;
						td_min = 0;
					}

					if (*(sp + 1) == 'd') {
						disptime = CalcTimeDifference_Win10(pt, td_hour, td_min, td_neg);
					}else if (*(sp + 1) == 'u') {
						if (disptime.wYear > 2023) 
						{
							disptime = CalcTimeDifference_Win10(pt, td_hour-1, td_min, td_neg);
						}
						else 
						{
							disptime = CalcTimeDifference_US_Win10(pt, td_hour, td_min, td_neg);
						}
					}
					else {		//Only applicable for UK in 2022 and future.
						disptime = CalcTimeDifference_Europe_Win10(pt, td_hour, td_min, td_neg);
					}

					sp += 8;
				}



				//added for time difference by TTTT
				else if (*sp == 'S' && (*(sp + 1) == 't') && ((*(sp + 2) == 'U') || (*(sp + 2) == 'E')))
				{
					if ((*(sp + 2) == 'U'))
					{
						if (b_SummerTime_US) {
							*dp++ = (char)'*'; *infop++ = 0x08;
						}
						else {
							*dp++ = (char)' '; *infop++ = 0x01;
						}
					}
					else if ((*(sp + 2) == 'E'))
					{
						if (b_SummerTime_Europe) {		//This flag will not be enabled in 2022-.
							*dp++ = (char)'*'; *infop++ = 0x08;
						}
						else {
							*dp++ = (char)' '; *infop++ = 0x01;
						}
					}
					sp += 3;
				}
					

				else if (*sp == 'M') // Available Physical Memory
				{
					int len, slen;
					DWORDLONG ms, mst;
					BOOL bComma = FALSE;
					BOOL bFlagGB = FALSE;
					ms = mst = (DWORDLONG)-1;
					double d_ms;


					if (*(sp + 1) == 'K')
					{
						sp += 2;
						ms = msMemory.ullAvailPhys / 1024;
					}
					else if (*(sp + 1) == 'M')
					{
						sp += 2;
						ms = msMemory.ullAvailPhys / (1024 * 1024);
					}
					else if (*(sp + 1) == 'G')
					{
						sp += 2;
						d_ms = (double)msMemory.ullAvailPhys / (1024 * 1024 * 1024);
						bFlagGB = TRUE;
					}
					else if (*(sp + 1) == 'S')
					{
						sp += 2;
						ms = (double)msMemory.ullTotalPhys / (1024 * 1024 * megabytesInGigaByte);
					}
					else if (*(sp + 1) == 'T')
					{
						if (*(sp + 2) == 'P')      ms = msMemory.ullTotalPhys;
						else if (*(sp + 2) == 'F') ms = msMemory.ullTotalPageFile;
						else if (*(sp + 2) == 'V') ms = msMemory.ullTotalVirtual;
						if (ms != -1)
						{
							if (*(sp + 3) == 'K') { ms /= 1024; sp += 4; }
							else if (*(sp + 3) == 'M') { ms /= 1024 * 1024; sp += 4; }
							else if (*(sp + 3) == 'G') {
								d_ms = (double)ms;
								d_ms /= 1024 * 1024 * 1024;
								bFlagGB = TRUE;
								sp += 4;
							}
							else ms = (DWORDLONG)-1;
						}
					}
					else if (*(sp + 1) == 'A')
					{
						if (*(sp + 2) == 'P')
						{
							ms = msMemory.ullAvailPhys;
							mst = msMemory.ullTotalPhys;
						}
						else if (*(sp + 2) == 'F')
						{
							ms = msMemory.ullAvailPageFile;
							mst = msMemory.ullTotalPageFile;
						}
						else if (*(sp + 2) == 'V')
						{
							ms = msMemory.ullAvailVirtual;
							mst = msMemory.ullTotalVirtual;
						}
						if (ms != -1)
						{
							if (*(sp + 3) == 'K') { ms /= 1024; sp += 4; }
							else if (*(sp + 3) == 'M') { ms /= 1024 * 1024; sp += 4; }
							else if (*(sp + 3) == 'P') { mst /= 100; if (!mst) ms = 0; else ms = ms / mst; sp += 4; }
							else if (*(sp + 3) == 'G') {
								d_ms = (double)ms;
								d_ms /= 1024 * 1024 * 1024;
								bFlagGB = TRUE;
								sp += 4;
							}
							else ms = (DWORDLONG)-1;
						}
					}
					else if (*(sp + 1) == 'U')
					{
						if (*(sp + 2) == 'P')
						{
							ms = msMemory.ullTotalPhys - msMemory.ullAvailPhys;
							mst = msMemory.ullTotalPhys;
						}
						else if (*(sp + 2) == 'F')
						{
							ms = msMemory.ullTotalPageFile - msMemory.ullAvailPageFile;
							mst = msMemory.ullTotalPageFile;
						}
						else if (*(sp + 2) == 'V')
						{
							ms = msMemory.ullTotalVirtual - msMemory.ullAvailVirtual;
							mst = msMemory.ullTotalVirtual;
						}
						if (ms != -1)
						{
							if (*(sp + 3) == 'K') { ms /= 1024; sp += 4; }
							else if (*(sp + 3) == 'M') { ms /= 1024 * 1024; sp += 4; }
							else if (*(sp + 3) == 'P') { mst /= 100; if (!mst) ms = 0; else ms = ms / mst; sp += 4; }
							else if (*(sp + 3) == 'G') {
								d_ms = (double)ms;
								d_ms /= 1024 * 1024 * 1024;
								bFlagGB = TRUE;
								sp += 4;
							}
							else ms = (DWORDLONG)-1;
						}
					}


					if(bFlagGB)
					{
						int i;
						ms = (int)d_ms;
						if(GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
						{
							len_ret = SetNumFormat(&dp, ms, len, slen, bComma);
							for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
						}
						else
						{
							len_ret = SetNumFormat(&dp, ms, 1, 0, FALSE);
							for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
						}
						d_ms = (d_ms -(int)d_ms);
						if(*sp == '.')
						{
							sp++;
							if(GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
							{
								*dp++ = (char)'.'; *infop++ = 0x01;
								if(len > 3) len = 3;
								for(i=0; i<len; i++) d_ms *= 10;
								ms = (int)d_ms;
								len_ret = SetNumFormat(&dp, ms, len, slen, FALSE);
								for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
							}
						}
					}
					else if (ms != -1)
					{
						if (GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == FALSE) { len = 1; slen = 0; }
						_ASSERTE(ms <= INT_MAX);
						len_ret = SetNumFormat(&dp, (int)ms, len, slen, bComma);
						for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
					else {
						*dp++ = *sp++; *infop++ = 0x01;
					}
				}


				else if(*sp == 'B' && *(sp + 1) == 'L') // Battery Life Percentage
				{
					sp += 2;
					{
						if(iBatteryLife <= 100)
						{
							int len, slen;
							BOOL bComma = FALSE;
							if(GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
							{
								len_ret = SetNumFormat(&dp, iBatteryLife, len, slen, bComma);
								for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
							}
							else
							{
								if (iBatteryLife > 99) {
									*dp++ = (char)((iBatteryLife % 1000) / 100 + '0'); *infop++ = 0x01;
								}
								*dp++ = (char)((iBatteryLife % 100) / 10 + '0'); *infop++ = 0x01;
								*dp++ = (char)((iBatteryLife % 10) + '0'); *infop++ = 0x01;
							}
						}
					}
				}
				else if(*sp == 'B' && (*(sp + 1) == 'h' || *(sp + 1) == 'n' || *(sp + 1) == 's' || *(sp + 1) == '_'))  // Battery Life Time
				{
					int len, slen;
					BOOL bComma = FALSE;
					sp++;
					if(GetNumFormat(&sp, 'h', ',', &len, &slen, &bComma) == TRUE)
					{
						len_ret = SetNumFormat(&dp, blt_h, len, slen, bComma);
						for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
					if(GetNumFormat(&sp, 'n', ',', &len, &slen, &bComma) == TRUE)
					{
						len_ret = SetNumFormat(&dp, blt_m, len, slen, bComma);
						for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
					if(GetNumFormat(&sp, 's', ',', &len, &slen, &bComma) == TRUE)
					{
						len_ret = SetNumFormat(&dp, blt_s, len, slen, bComma);
						for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
				}
				else if ((*sp == 'L') && (*(sp + 1) == 'T') && (*(sp + 2) == 'E')) // LTE status by TTTT
				{
					sp += 3;
					int len = strlen(strLTE);
						if (net[12] == 2)
						{
							if (len != 0)
							{
								for (int i = 0; i < len; i++)
								{
									*dp++ = strLTE[i]; *infop++ = 0x01;
								}
							}
							*dp++ = (char)'*'; *infop++ = 0x01;
						}
						else if (net[12] == 1)
						{
							if (len != 0)
							{
								for (int i = 0; i < len; i++)
								{
									*dp++ = strLTE[i]; *infop++ = 0x01;
								}
							}
							*dp++ = (char)' '; *infop++ = 0x01;
						}
						else
						{
							if (len != 0)
							{
								for (int i = 0; i < len; i++)
								{
									*dp++ = ' '; *infop++ = 0x01;
								}
							}
							*dp++ = (char)' '; *infop++ = 0x01;
						}
				}
				else if ((*sp == 'W') && (*(sp + 1) == 'i') && (*(sp + 2) == 'F') && (*(sp + 3) == 'i')) // WiFi status by TTTT
				{
					sp += 4;
					if (net[15] == 2)
					{
						*dp++ = (char)'W'; *infop++ = 0x01;
						*dp++ = (char)'i'; *infop++ = 0x01;
						*dp++ = (char)'F'; *infop++ = 0x01;
						*dp++ = (char)'i'; *infop++ = 0x01;
						*dp++ = (char)'*'; *infop++ = 0x01;
					}
					else if (net[15] == 1)
					{
						*dp++ = (char)'W'; *infop++ = 0x01;
						*dp++ = (char)'i'; *infop++ = 0x01;
						*dp++ = (char)'F'; *infop++ = 0x01;
						*dp++ = (char)'i'; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					else
					{
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
				}

				else if ((*sp == 'E') && (*(sp + 1) == 't') && (*(sp + 2) == 'h') && (*(sp + 3) == 'S')) // Ether status by TTTT
				{
					sp += 4;
					if (net[18] == 2)
					{
						*dp++ = (char)'E'; *infop++ = 0x01;
						*dp++ = (char)'t'; *infop++ = 0x01;
						*dp++ = (char)'h'; *infop++ = 0x01;
						*dp++ = (char)'*'; *infop++ = 0x01;
					}
					else if (net[18] == 1)
					{
						*dp++ = (char)'E'; *infop++ = 0x01;
						*dp++ = (char)'t'; *infop++ = 0x01;
						*dp++ = (char)'h'; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					else
					{
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
				}

				else if ((*sp == 'E') && (*(sp + 1) == 't') && (*(sp + 2) == 'h') && (*(sp + 3) == 'L')) // Ether status by TTTT
				{
					sp += 4;
					if (net[18] == 2)
					{
						*dp++ = (char)'E'; *infop++ = 0x01;
						*dp++ = (char)'t'; *infop++ = 0x01;
						*dp++ = (char)'h'; *infop++ = 0x01;
						*dp++ = (char)'e'; *infop++ = 0x01;
						*dp++ = (char)'r'; *infop++ = 0x01;
						*dp++ = (char)'n'; *infop++ = 0x01;
						*dp++ = (char)'e'; *infop++ = 0x01;
						*dp++ = (char)'t'; *infop++ = 0x01;
						*dp++ = (char)'*'; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					else if (net[18] == 1)
					{
						*dp++ = (char)'E'; *infop++ = 0x01;
						*dp++ = (char)'t'; *infop++ = 0x01;
						*dp++ = (char)'h'; *infop++ = 0x01;
						*dp++ = (char)'e'; *infop++ = 0x01;
						*dp++ = (char)'r'; *infop++ = 0x01;
						*dp++ = (char)'n'; *infop++ = 0x01;
						*dp++ = (char)'e'; *infop++ = 0x01;
						*dp++ = (char)'t'; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					else
					{
						for (int i = 0; i < 10; i++) {
							*dp++ = (char)' '; *infop++ = 0x01;
						}
					}
				}
				// SSID and APN by TTTT
				else if ((*sp == 'S') && (*(sp + 1) == 'S') && (*(sp + 2) == 'I') && (*(sp + 3) == 'D')) 
				{
					sp += 4;
					char strTemp[64];
					snprintf(strTemp, 60, "%s", activeSSID);
					int len = strlen(strTemp);
					if (len > SSID_AP_Length) len = SSID_AP_Length;
					if (len != 0)
					{
						for (int i = 0; i < len; i++)
						{
							*dp++ = strTemp[i]; *infop++ = 0x01;
						}
					}
					if (len < SSID_AP_Length)
						for (int i = 0; i < (SSID_AP_Length - len); i++) 
						{
							*dp++ = (char)' '; *infop++ = 0x01;
						}
				}
				else if ((*sp == 'A') && (*(sp + 1) == 'P') && (*(sp + 2) == 'N'))
				{
					sp += 3;
					char strTemp[64];
					snprintf(strTemp, 60, "%s", activeAPName);
					int len = strlen(strTemp);
					if (len > SSID_AP_Length) len = SSID_AP_Length;
					if (len != 0)
					{
						for (int i = 0; i < len; i++)
						{
							*dp++ = strTemp[i]; *infop++ = 0x01;
						}
					}
					if (len < SSID_AP_Length)
						for (int i = 0; i < (SSID_AP_Length - len); i++) {
							*dp++ = (char)' ';*infop++ = 0x01;
						}
				}


				//"NMX1" or "NMX2", added for DataPlan Usage by TTTT
				else if (*sp == 'N' && *(sp + 1) == 'M' && *(sp + 2) == 'X' && ((*(sp + 3) == '2') || (*(sp + 3) == '1')))
				{
					sp += 4;
					char strTemp[32];
					if (g_InternetConnectStat_Win10 == 0 && net[18] == 2)
					{
						strcpy(strTemp, "Ethernet*");
					}
					else if (g_InternetConnectStat_Win10 == 0 && net[18] == 1)
					{
						strcpy(strTemp, "Ethernet");
					}
					else if (g_InternetConnectStat_Win10 == 1 || g_InternetConnectStat_Win10 == 4 || (flag_SoftEther && active_physical_adapter_Win10 == 1))
					{
						strcpy(strTemp, icp_SSID_APName);
						if (strlen(strTemp) == 0) strcpy(strTemp, "SSID:N/A");
					}
					else if (((g_InternetConnectStat_Win10 == 2) || (flag_SoftEther && (active_physical_adapter_Win10 == 2))))
					{
						strcpy(strTemp, icp_SSID_APName);
						if (strlen(strTemp) == 0) strcpy(strTemp, "APN: N/A");
					}
					else
					{
						strcpy(strTemp, "");
					}
					int len = strlen(strTemp);
					if (len > NetMIX_Length) len = NetMIX_Length;
					if (len != 0)
					{
						for (int i = 0; i < len; i++)
						{
							*dp++ = strTemp[i]; *infop++ = 0x01;
						}
					}
					if (len < NetMIX_Length) {
						for (int i = 0; i < (NetMIX_Length - len); i++) 
						{
							*dp++ = (char)' '; *infop++ = 0x01;
						}
					}
				}



				else if ((*sp == 'I') && (*(sp + 1) == 'C') && (*(sp + 2) == 'P')) // "CIP" Internet Connection Profile by TTTT
				{
					sp += 3;
					if (g_InternetConnectStat_Win10 == 0 && active_physical_adapter_Win10 == 0)
					{
						*dp++ = (char)'E'; *infop++ = 0x01;
					}
					else if ((g_InternetConnectStat_Win10 == 1) || (flag_SoftEther && (active_physical_adapter_Win10 == 1) && !b_MeteredNetNow))
					{
						*dp++ = (char)'W'; *infop++ = 0x01;
					}
					else if ((g_InternetConnectStat_Win10 == 2) || (flag_SoftEther && (active_physical_adapter_Win10 == 2)))
					{
						*dp++ = charLTE[0]; *infop++ = 0x01;
					}
					else if ((g_InternetConnectStat_Win10 == 4) || (flag_SoftEther && (active_physical_adapter_Win10 == 1) && b_MeteredNetNow))
					{
						*dp++ = (char)'M'; *infop++ = 0x01;
					}
					else
					{
						*dp++ = (char)'-'; *infop++ = 0x01;
					}
				}

				else if ((*sp == 'E') && (*(sp + 1) == 'W') && (*(sp + 2) == 'L') && (*(sp + 3) == 'L')) // Ether/WiFi/LTE status shortformat by TTTT
				{
					sp += 4;
					if (net[18] == 2)
					{
						*dp++ = (char)'E'; *infop++ = 0x01;
						*dp++ = (char)'*'; *infop++ = 0x01;
					}
					else if (net[18] == 1)
					{
						*dp++ = (char)'E'; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					else
					{
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					*dp++ = (char)' '; *infop++ = 0x01;
					if (net[15] == 2)
					{
						*dp++ = (char)'W'; *infop++ = 0x01;
						*dp++ = (char)'*'; *infop++ = 0x01;
					}
					else if (net[15] == 1)
					{
						*dp++ = (char)'W'; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					else
					{
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					*dp++ = (char)' '; *infop++ = 0x01;
					if (net[12] == 2)
					{
						*dp++ = charLTE[0]; *infop++ = 0x01;
						*dp++ = (char)'*'; *infop++ = 0x01;
					}
					else if (net[12] == 1)
					{
						*dp++ = charLTE[0]; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					else
					{
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
				}

				else if ((*sp == 'E') && (*(sp + 1) == 'W') && (*(sp + 2) == 'L') && (*(sp + 3) == 'S')) // Ether/WiFi/LTE status shortformat by TTTT
				{
					sp += 4;
					if (net[18] == 2)
					{
						*dp++ = (char)'E'; *infop++ = 0x01;
						*dp++ = (char)'*'; *infop++ = 0x01;
					}
					else if (net[18] == 1)
					{
						*dp++ = (char)'E'; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					else
					{
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					if (net[15] == 2)
					{
						*dp++ = (char)'W'; *infop++ = 0x01;
						*dp++ = (char)'*'; *infop++ = 0x01;
					}
					else if (net[15] == 1)
					{
						*dp++ = (char)'W'; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					else
					{
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					if (net[12] == 2)
					{
						*dp++ = charLTE[0]; *infop++ = 0x01;
						*dp++ = (char)'*'; *infop++ = 0x01;
					}
					else if (net[12] == 1)
					{
						*dp++ = charLTE[0]; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
					else
					{
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
				}


				else if ((*sp == 'V') && (*(sp + 1) == 'P') && (*(sp + 2) == 'N') && (*(sp + 3) == 'S')) // VPNS = VPN status by TTTT
				{
					sp += 4;
					if (flag_VPN)
					{
						*dp++ = (char)'V'; *infop++ = 0x01;
						*dp++ = (char)'P'; *infop++ = 0x01;
						*dp++ = (char)'N'; *infop++ = 0x01;
					}
					else
					{
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
						*dp++ = (char)' '; *infop++ = 0x01;
					}
				}

				else if ((*sp == 'I') && (*(sp + 1) == 'P'))	 // IP addresses by TTTT
				{
					sp += 2;
					if (*sp == 'E')
					{
						*sp++;
						int len = strlen(ipEther);
						if (len > 15) len = 15;
						for (int i = 0; i < len; i++)
						{
							*dp++ = ipEther[i]; *infop++ = 0x01;
						}
						if (len < 15) for (int i = 0; i < (15-len); i++)
						{
							*dp++ = (char)' '; *infop++ = 0x01;
						}
					}
					else if (*sp == 'W')
					{
						*sp++;
						int len = strlen(ipWiFi);
						if (len > 15) len = 15;
						for (int i = 0; i < len; i++)
						{
							*dp++ = ipWiFi[i]; *infop++ = 0x01;
						}
						if (len < 15) for (int i = 0; i < (15 - len); i++)
						{
							*dp++ = (char)' '; *infop++ = 0x01;
						}
					}
					else if (*sp == 'L')
					{
						*sp++;
						int len = strlen(ipLTE);
						if (len > 15) len = 15;
						for (int i = 0; i < len; i++)
						{
							*dp++ = ipLTE[i]; *infop++ = 0x01;
						}
						if (len < 15) for (int i = 0; i < (15 - len); i++)
						{
							*dp++ = (char)' '; *infop++ = 0x01;
						}
					}
					else if (*sp == 'V')
					{
						*sp++;
						int len = strlen(ipVPN);
						if (len > 15) len = 15;
						for (int i = 0; i < len; i++)
						{
							*dp++ = ipVPN[i]; *infop++ = 0x01;
						}
						if (len < 15) for (int i = 0; i < (15 - len); i++)
						{
							*dp++ = (char)' '; *infop++ = 0x01;
						}
					}
					else if (*sp == 'A')
					{
						*sp++;
						char buf_Win10[32];
						strcpy(buf_Win10, "IP[Active] -NA-");
						if (flag_VPN) strcpy(buf_Win10, ipVPN);
						else if (g_InternetConnectStat_Win10 == 0 && (net[18] == 1 || net[18] == 2)) strcpy(buf_Win10, ipEther);
						else if (g_InternetConnectStat_Win10 == 1 || g_InternetConnectStat_Win10 == 4) strcpy(buf_Win10, ipWiFi);
						else if (g_InternetConnectStat_Win10 == 2) strcpy(buf_Win10, ipLTE);

						int len = strlen(buf_Win10);
						if (len > 15) len = 15;
						for (int i = 0; i < len; i++)
						{
							*dp++ = buf_Win10[i]; *infop++ = 0x01;
						}
						if (len < 15) for (int i = 0; i < (15 - len); i++)
						{
							*dp++ = (char)' '; *infop++ = 0x01;
						}
					}
				}




				else if ((*sp == 'W') && (*(sp + 1) == 'A') && (*(sp + 2) == 'N') && (*(sp + 3) == 'P')) // LTE Profile Number by TTTT
				{
					sp += 4;
					char buf_Win10[20];


					if (currentLTEProfNum != -1)
					{
						snprintf(buf_Win10, 20, "%2d", currentLTEProfNum);
					}
					else
					{
						snprintf(buf_Win10, 20, "N/A");
					}

					int len = strlen(buf_Win10);
					if (len != 0)
					{
						for (int i = 0; i < len; i++)
						{
							*dp++ = buf_Win10[i]; *infop++ = 0x01;
						}
					}
				}



				else if ((*sp == 'A') && (*(sp + 1) == 'I') && (*(sp + 2) == 'P') && (*(sp + 3) == 'F')) // Active Internet Connection Profile Number by TTTT
				{
					sp += 4;
					char buf_Win10[20];


					if (internetConnectProfNum != -1)
					{
						snprintf(buf_Win10, 20, "%2d", internetConnectProfNum);
					}
					else
					{
						snprintf(buf_Win10, 20, "N/A");
					}

					int len = strlen(buf_Win10);
					if (len != 0)
					{
						for (int i = 0; i < len; i++)
						{
							*dp++ = buf_Win10[i]; *infop++ = 0x01;
						}
					}
				}



				//FTA Flag Timer Adjust : "*" / " "
				else if ((*sp == 'F') && (*(sp + 1) == 'T') && (*(sp + 2) == 'A'))
				{
					sp += 3;
						if (b_FlagTimerAdjust)
						{
							*dp++ = (char)'*'; *infop++ = 0x08;
						}
						else
						{
							*dp++ = (char)' '; *infop++ = 0x08;
						}
				}


				else if ((*sp == 'V') && (*(sp + 1) == 'e') && (*(sp + 2) == 'r') && (*(sp + 3) == 'T') && (*(sp + 4) == 'C')) // Auto Pause Cloud Applications by TTTT
				{
					sp += 5;
					for (int i = 0; i < strlen(Ver_TClockWin10); i++)
					{
						*dp++ = Ver_TClockWin10[i]; *infop++ = 0x01;
					}
				}



				else if ((*sp == 'N') && (*(sp + 1) == 'R') && (*(sp + 2) == 'A') && (*(sp + 3) == 'A')) // "NRAA" Toral Receive Data in Autoformat by TTTT
				{
					sp += 4;
					char buf_Win10[20];
					{
						char tempstr_Win10[10];
						if ((net[4] < 1024) && (net[4] >= 0))
						{
							snprintf(tempstr_Win10, 10, "%4.0fKB", net[4]);
						}
						else if (net[8] < 10)
						{
							snprintf(tempstr_Win10, 10, "%1.2fMB", net[8]);
						}
						else if (net[8] < 100)
						{
							snprintf(tempstr_Win10, 10, "%2.1fMB", net[8]);
						}
						else if (net[8] < megabytesInGigaByte)
						{
							snprintf(tempstr_Win10, 10, "%4.0fMB", net[8]);
						}
						else if (net[8] < 10 * megabytesInGigaByte)
						{
							snprintf(tempstr_Win10, 10, "%1.2fGB", (net[8] / megabytesInGigaByte));
						}
						else if (net[8] < 100 * megabytesInGigaByte)
						{
							snprintf(tempstr_Win10, 10, "%2.1fGB", (net[8] / megabytesInGigaByte));
						}
						else if(net[8] < 10000 * megabytesInGigaByte)
						{
							snprintf(tempstr_Win10, 10, "%4.0fGB", (net[8] / megabytesInGigaByte));
						}
						else
						{
							snprintf(tempstr_Win10, 10, "%dGB", (int)(net[8] / megabytesInGigaByte));
						}
						snprintf(buf_Win10, 20, "%s", (tempstr_Win10));

					}
					int len = strlen(buf_Win10);
					if (len != 0)
					{
						for (int i = 0; i < len; i++)
						{
							*dp++ = buf_Win10[i]; *infop++ = 0x01;
						}
					}
				}


				else if ((*sp == 'N') && (*(sp + 1) == 'S') && (*(sp + 2) == 'A') && (*(sp + 3) == 'A')) // "NSAA" Toral sent Data in Autoformat by TTTT
				{
					sp += 4;
					char buf_Win10[20];
					{
						char tempstr_Win10[10];
						if ((net[5] < 1024) && (net[5] >= 0))
						{
							snprintf(tempstr_Win10, 10, "%4.0fKB", net[5]);
						}
						else if (net[9] < 10)
						{
							snprintf(tempstr_Win10, 10, "%1.2fMB", net[9]);
						}
						else if (net[9] < 100)
						{
							snprintf(tempstr_Win10, 10, "%2.1fMB", net[9]);
						}
						else if (net[9] < megabytesInGigaByte)
						{
							snprintf(tempstr_Win10, 10, "%4.0fMB", net[9]);
						}
						else if (net[9] < 10 * megabytesInGigaByte)
						{
							snprintf(tempstr_Win10, 10, "%1.2fGB", (net[9] / megabytesInGigaByte));
						}
						else if (net[9] < 100 * megabytesInGigaByte)
						{
							snprintf(tempstr_Win10, 10, "%2.1fGB", (net[9] / megabytesInGigaByte));
						}
						else if (net[9] < 10000 * megabytesInGigaByte)
						{
							snprintf(tempstr_Win10, 10, "%4.fGB", (net[9] / megabytesInGigaByte));
						}
						else
						{
							snprintf(tempstr_Win10, 10, "%dGB", (int)(net[9] / megabytesInGigaByte));
						}
						snprintf(buf_Win10, 20, "%s", (tempstr_Win10));

					}
					int len = strlen(buf_Win10);
					if (len != 0)
					{
						for (int i = 0; i < len; i++)
						{
							*dp++ = buf_Win10[i]; *infop++ = 0x01;
						}
					}
				}



				else if ((*sp == 'P') && (*(sp + 1) == 'C') && (*(sp + 2) == 'O') && (*(sp + 3) == 'R') && (*(sp + 4) == 'E')) // "CORES" "LPROC" for number of physical cores and logical processorsby TTTT 20201230
				{
					sp += 5;
					//extern int nLogicalProcessors;
					extern int nCores;

					if (nCores > 9)
					{
						*dp++ = (char)((int)nCores / 10) + '0'; *infop++ = 0x01;
					}
					*dp++ = (char)((int)nCores % 10) + '0'; *infop++ = 0x01;

//					len_ret = SetNumFormat(&dp, nCores, 1, 0, FALSE);
				}

				else if ((*sp == 'L') && (*(sp + 1) == 'P') && (*(sp + 2) == 'R') && (*(sp + 3) == 'O') && (*(sp + 4) == 'C')) // "CORES" "LPROC" for number of physical cores and logical processorsby TTTT 20201230
				{
					sp += 5;
					extern int nLogicalProcessors;
					//extern int nCores;

					if (nLogicalProcessors > 9)
					{
						*dp++ = (char)((int)nLogicalProcessors / 10) + '0'; *infop++ = 0x01;
					}
					*dp++ = (char)((int)nLogicalProcessors % 10) + '0'; *infop++ = 0x01;

//					len_ret = SetNumFormat(&dp, nLogicalProcessors, 1, 0, FALSE);
				}

				else if (*sp == 'N') // Network Interface
				{
					int len, slen, i, unit, nt;
					double ntd;
					BOOL bComma = FALSE;
					BOOL bFlag = TRUE;
					ntd = -1;
					//unit = 0;
					if (*(sp + 1) == 'R')
					{
						if (*(sp + 2) == 'A')
						{
							if (*(sp + 3) == 'B') ntd = net[0];
							else if (*(sp + 3) == 'K') ntd = net[4];
							else if (*(sp + 3) == 'M') ntd = net[8];
							else if (*(sp + 3) == 'G') ntd = (int)(net[8] / megabytesInGigaByte);
							else bFlag = FALSE;
						}
						else if (*(sp + 2) == 'S')
						{
							if (*(sp + 3) == 'B') ntd = net[2];
							else if (*(sp + 3) == 'K') ntd = net[6];
							else if (*(sp + 3) == 'M') ntd = net[10];
							else bFlag = FALSE;
						}
					}
					if (*(sp + 1) == 'S')
					{
						if (*(sp + 2) == 'A')
						{
							if (*(sp + 3) == 'B') ntd = net[1];
							else if (*(sp + 3) == 'K') ntd = net[5];
							else if (*(sp + 3) == 'M') ntd = net[9];
							else if (*(sp + 3) == 'G') ntd = (int)(net[9] / megabytesInGigaByte);
							else bFlag = FALSE;
						}
						else if (*(sp + 2) == 'S')
						{
							if (*(sp + 3) == 'B') ntd = net[3];
							else if (*(sp + 3) == 'K') ntd = net[7];
							else if (*(sp + 3) == 'M') ntd = net[11];
							else bFlag = FALSE;
						}
					}
					if (bFlag)
					{
						sp += 4;
						nt = (int)ntd;
						if (GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
						{
							len_ret = SetNumFormat(&dp, nt, len, slen, bComma);
							for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
						}
						else
						{
							len_ret = SetNumFormat(&dp, nt, 1, 0, FALSE);
							for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
						}
						ntd = (ntd - (int)ntd);
						if (*sp == '.')
						{
							sp++;
							if (GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
							{
								*dp++ = (char)'.'; *infop++ = 0x01;
								if (len > 3) len = 3;
								for (i = 0; i<len; i++) ntd *= 10;
								nt = (int)ntd;
								len_ret = SetNumFormat(&dp, nt, len, 0, FALSE);
								for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
							}
						}
					}
					else {
						*dp++ = *sp++; *infop++ = 0x01;
					}
				}



				else if(*sp == 'L' && _strncmp(sp, "LDATE", 5) == 0)
				{
					char s[80], *p;
					GetDateFormatWA(MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
						DATE_LONGDATE, pt, NULL, s, 80);
					p = s;
					while (*p) {
						*dp++ = *p++; *infop++ = 0x02;
					}
					sp += 5;
				}
				else if(*sp == 'D' && _strncmp(sp, "DATE", 4) == 0)
				{
					char s[80], *p;
					GetDateFormatWA(MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
						DATE_SHORTDATE, pt, NULL, s, 80);
					p = s;
					while (*p) {
						*dp++ = *p++; *infop++ = 0x02;
					}
					sp += 4;
				}
				else if(*sp == 'T' && _strncmp(sp, "TIME", 4) == 0)
				{
					char s[80], *p;
					GetTimeFormatWA(MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
						TIME_FORCE24HOURFORMAT, pt, NULL, s, 80);
					p = s;
					while (*p) {
						*dp++ = *p++; *infop++ = 0x08;
					}
					sp += 4;
				}
				else if(*sp == 'S')
				{
					int len, slen, st;
					BOOL bComma = FALSE;
					sp++;
					if(GetNumFormat(&sp, 'd', ',', &len, &slen, &bComma) == TRUE)
					{
						if (!TickCount) TickCount = GetTickCount();
						st = TickCount/86400000;		//day
						len_ret = SetNumFormat(&dp, st, len, slen, bComma);
						for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
					else if(GetNumFormat(&sp, 'a', ',', &len, &slen, &bComma) == TRUE)
					{
						if (!TickCount) TickCount = GetTickCount();
						st = TickCount/3600000;		//hour
						len_ret = SetNumFormat(&dp, st, len, slen, bComma);
						for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
					else if(GetNumFormat(&sp, 'h', ',', &len, &slen, &bComma) == TRUE)
					{
						if (!TickCount) TickCount = GetTickCount();
						st = (TickCount/3600000)%24;
						len_ret = SetNumFormat(&dp, st, len, slen, FALSE);
						for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
					else if(GetNumFormat(&sp, 'n', ',', &len, &slen, &bComma) == TRUE)
					{
						if (!TickCount) TickCount = GetTickCount();
						st = (TickCount/60000)%60;
						len_ret = SetNumFormat(&dp, st, len, slen, FALSE);
						for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
					else if(GetNumFormat(&sp, 's', ',', &len, &slen, &bComma) == TRUE)
					{
						if (!TickCount) TickCount = GetTickCount();
						st = (TickCount/1000)%60;
						len_ret = SetNumFormat(&dp, st, len, slen, FALSE);
						for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
					else if(*sp == 'T')
					{
						DWORD dw;
						int sth, stm, sts;
						if (!TickCount) TickCount = GetTickCount();
						dw = TickCount;
						dw /= 1000;
						sts = dw%60; dw /= 60;
						stm = dw%60; dw /= 60;
						sth = dw;

						len_ret = SetNumFormat(&dp, sth, 2, 0, FALSE); for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
						*dp++ = ':'; *infop++ = 0x01;
						len_ret = SetNumFormat(&dp, stm, 2, 0, FALSE); for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
						*dp++ = ':'; *infop++ = 0x01;
						len_ret = SetNumFormat(&dp, sts, 2, 0, FALSE); for (int i = 0; i < len_ret; i++)*infop++ = 0x01;

						sp++;
					}
					else
					{
						*dp++ = 'S'; *infop++ = 0x01;
					}
				}
				else if(*sp == 'H')
				{
					int dv, dski, len, slen, i;
					BOOL bComma = FALSE;
					double dsk = 0;

					dv = *(sp + 2) - 'A';
					if (*(sp + 2) <= '9') dv = *(sp + 2) - '0' + 26;

					if(*(sp + 1) == 'T')
					{
						if (*(sp + 3) == 'M')
							dsk = diskAll[dv];
						else if (*(sp + 3) == 'G')
							dsk = diskAll[dv+36];
					}
					if(*(sp + 1) == 'A')
					{
						if (*(sp + 3) == 'M')
							dsk = diskFree[dv];
						else if (*(sp + 3) == 'G')
							dsk = diskFree[dv+36];
						else if (*(sp + 3) == 'P')
							dsk = (diskFree[dv]/diskAll[dv])*100;
					}
					if(*(sp + 1) == 'U')
					{
						if (*(sp + 3) == 'M')
							dsk = diskAll[dv] - diskFree[dv];
						else if (*(sp + 3) == 'G')
							dsk = diskAll[dv+36] - diskFree[dv+36];
						else if (*(sp + 3) == 'P')
							dsk = ((diskAll[dv] - diskFree[dv])/diskAll[dv])*100;
					}
					sp += 4;
					dski = (int)dsk;
					if(GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
					{
						len_ret = SetNumFormat(&dp, dski, len, slen, bComma);
						for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
					else
					{
						len_ret = SetNumFormat(&dp, dski, 1, 0, FALSE);
						for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
					dsk = (dsk-(int)dsk);
					if(*sp == '.')
					{
						sp++;
						if(GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
						{
							*dp++ = (char)'.'; *infop++ = 0x01;
							if(len > 6) len = 6;
							for(i=0; i<len; i++) dsk *= 10;
							dski = (int)dsk;
							len_ret = SetNumFormat(&dp, dski, len, 0, FALSE);
							for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
						}
					}
				}
				else if(*sp == 'V' && *(sp + 1) == 'L') // Volume
				{
					int len, slen;
					BOOL bComma = FALSE;
					sp += 2;

					if(GetNumFormat(&sp, 'x', ',', &len, &slen, &bComma) == TRUE)
					{
						len_ret = SetNumFormat(&dp, iVolume, len, slen, FALSE); for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
					else
					{
						len_ret = SetNumFormat(&dp, iVolume, 3, 2, FALSE); for (int i = 0; i < len_ret; i++)*infop++ = 0x01;
					}
				}
				else if (*sp == 'V' && *(sp + 1) == 'M')	//Mute Status
				{
					int len;
					sp += 2;
					if (muteStatus)
					{
						int len = strlen(strMute);
						if (len != 0)
						{
							for (int i = 0; i < len; i++)
							{
								*dp++ = strMute[i]; *infop++ = 0x01;
							}
						}
					}
					else
					{
						int len = strlen(strMute);
						if (len != 0)
						{
							for (int i = 0; i < len; i++)
							{
								*dp++ = ' '; *infop++ = 0x01;
							}
						}
					}
				}
				else
				{
					p = CharNext(sp);
					while (sp != p) {
						*dp++ = *sp++; *infop++ = 0x01;
					}
				}
			}
		}

		else
		{
			p = CharNext(sp);
			while (sp != p) {
				*dp++ = *sp++; *infop++ = 0x01;
			}
		}
	}
	*dp = 0; *infop++ = 0;	//文字列終端

	//if (b_DebugLog) {
	//	writeDebugLog_Win10("[format.c][MakeFormat] Length of string =", strlen(s));
	//	writeDebugLog_Win10("[format.c][MakeFormat] Length of string_info =", strlen(s_info));
	//}

	b_FlagTimerAdjust = FALSE;		//Clear Flag Timer Adjust, Added by TTTT
}





/*------------------------------------------------
  check format
--------------------------------------------------*/
DWORD FindFormat(char* fmt)
{
	char *sp;
	DWORD ret = 0;

	sp = fmt;
	while(*sp)
	{
		if(*sp == '<' && *(sp + 1) == '%')
		{
			sp += 2;
			while(*sp)
			{
				if(*sp == '%' && *(sp + 1) == '>')
				{
					sp += 2;
					break;
				}
				if(*sp == '\"')
				{
					sp++;
					while(*sp != '\"' && *sp) sp++;
					if(*sp == '\"') sp++;
				}
				else if(*sp == 's')
				{
					sp++;
					ret |= FORMAT_SECOND;
				}
				else if (*sp == '@' && *(sp + 1) == '@' && *(sp + 2) == '@')
				{
					sp += 3;
					if(*sp == '.' && *(sp + 1) == '@')
					{
						ret |= FORMAT_BEAT2;
						sp += 2;
					}
					else
						ret |= FORMAT_BEAT1;
				}
				//else if(*sp == 'R' &&
				//	(*(sp + 1) == 'S' || *(sp + 1) == 'G' || *(sp + 1) == 'U') )
				//{
				//	sp += 2;
				//	ret |= FORMAT_SYSINFO;
				//}
				else if(*sp == 'C' && *(sp + 1) == 'U' &&
					(isdigit(*(sp + 2)) && *(sp + 2) != '8' && *(sp + 2) != '9') )
				{
					sp += 3;
					//ret |= FORMAT_PERMON;
					ret |= FORMAT_CPU;
				}
				else if(*sp == 'C' && *(sp + 1) == 'U')
				{
					sp += 2;
					//ret |= FORMAT_SYSINFO;
					//ret |= FORMAT_PERMON;
					ret |= FORMAT_CPU;
				}
				else if(*sp == 'C' && *(sp + 1) == 'C')
				{
					sp += 2;
					//ret |= FORMAT_PERMON;
					ret |= FORMAT_CPU;
				}
				else if(*sp == 'B' && *(sp + 1) == 'L')
				{
					sp += 2;
					ret |= FORMAT_BATTERY;
				}
				else if(*sp == 'B' && (*(sp + 1) == 'h' || *(sp + 1) == 'n' || *(sp + 1) == 's' || *(sp + 1) == '_'))
				{
					sp += 2;
					ret |= FORMAT_BATTERY;
				}
				else if(*sp == 'A' && *(sp + 1) == 'D' )
				{
					sp += 2;
					ret |= FORMAT_BATTERY;
				}
				else if(*sp == 'a' && *(sp + 1) == 'd' )
				{
					sp += 2;
					ret |= FORMAT_BATTERY;
				}
				else if(*sp == 'M' && (*(sp + 1) == 'K' || *(sp + 1) == 'M'))
				{
					sp += 2;
					ret |= FORMAT_MEMORY;
				}
				else if(*sp == 'M' &&
					(*(sp + 1) == 'T' || *(sp + 1) == 'A' || *(sp + 1) == 'U') &&
					(*(sp + 2) == 'P' || *(sp + 2) == 'F' || *(sp + 2) == 'V') &&
					(*(sp + 3) == 'K' || *(sp + 3) == 'M' || *(sp + 3) == 'P' || *(sp + 3) == 'G'))
				{
					sp += 4;
					ret |= FORMAT_MEMORY;
				}
				//else if(*sp == 'B' && *(sp + 1) == 'T')
				//{
				//	sp += 2;
				//	ret |= FORMAT_MOTHERBRD;
				//}
				//else if(*sp == 'B' && *(sp + 1) == 'V')
				//{
				//	sp += 2;
				//	ret |= FORMAT_MOTHERBRD;
				//}
				//else if(*sp == 'B' && *(sp + 1) == 'F')
				//{
				//	sp += 2;
				//	ret |= FORMAT_MOTHERBRD;
				//}
				else if(*sp == 'N' &&
					(*(sp + 1) == 'R' || *(sp + 1) == 'S') &&
					(*(sp + 2) == 'S' || *(sp + 2) == 'A') &&
					(*(sp + 3) == 'M' || *(sp + 3) == 'K' || *(sp + 3) == 'B' || *(sp + 3) == 'G' || *(sp + 3) == 'A'))
				{
					sp += 4;
					ret |= FORMAT_NET;
				}
				else if(*sp == 'H' && (*(sp + 1) == 'A' || *(sp + 1) == 'U' || *(sp + 1) == 'T') && (*(sp + 2) >= 'A' && *(sp + 2) <= 'Z') && (*(sp + 3) == 'M' || *(sp + 3) == 'G' || *(sp + 3) == 'P'))
				{
					int dv;
					dv = *(sp + 2) - 'A';
					actdvl[dv] = 1;
					sp += 4;
					ret |= FORMAT_HDD;
				}
				else if (*sp == 'H' && (*(sp + 1) == 'A' || *(sp + 1) == 'U' || *(sp + 1) == 'T') && (*(sp + 2) >= '0' && *(sp + 2) <= '9') && (*(sp + 3) == 'M' || *(sp + 3) == 'G' || *(sp + 3) == 'P'))
				{
					int dv;
					extern char strAdditionalMountPath;
					dv = *(sp + 2) - '0';
					if (strlen(&strAdditionalMountPath + 64 * dv) > 0) {
						actdvl[dv + 26] = 1;
					}
					sp += 4;
					ret |= FORMAT_HDD;
				}
				else if(*sp == 'V' && *(sp + 1) == 'L')
				{
					sp += 2;
					ret |= FORMAT_VOL;
				}
				else if (*sp == 'V' && *(sp + 1) == 'M')
				{
					sp += 2;
					ret |= FORMAT_VOL;
				}
				else if (*sp == 'G' && *(sp + 1) == 'U')
				{
					sp += 2;
					ret |= FORMAT_GPU;
				}
				else if (*sp == 'T' && (*(sp + 1) == 'E') && (*(sp + 2) == 'M') && (*(sp + 3) == 'P')) {
					sp += 4;
					ret |= FORMAT_TEMP;
				}
				else sp = CharNext(sp);
			}
		}
		else sp = CharNext(sp);
	}
	return ret;
}


SYSTEMTIME CalcTimeDifference_Win10(SYSTEMTIME* pt, int td_h, int td_m, BOOL pol_neg)
{
	SYSTEMTIME systemtime_temp;
	FILETIME filetime_temp;
	ULONGLONG ulonglong_temp;
	systemtime_temp = *pt;
	ULARGE_INTEGER ularge_integer_temp;

	SystemTimeToFileTime(&systemtime_temp, &filetime_temp);

	ularge_integer_temp.HighPart = filetime_temp.dwHighDateTime;
	ularge_integer_temp.LowPart = filetime_temp.dwLowDateTime;

	if (pol_neg)
	{
		ularge_integer_temp.QuadPart -= (ULONGLONG)(td_h * 60 + td_m) * 600000000;
	}
	else
	{
		ularge_integer_temp.QuadPart += (ULONGLONG)(td_h * 60 + td_m) * 600000000;
	}



	filetime_temp.dwHighDateTime = ularge_integer_temp.HighPart;
	filetime_temp.dwLowDateTime = ularge_integer_temp.LowPart;

	FileTimeToSystemTime(&filetime_temp, &systemtime_temp);

	b_FlagPrevDay = FALSE;
	b_FlagNextDay = FALSE;
	b_FlagPrevMonth = FALSE;
	b_FlagNextMonth = FALSE;

	if ((systemtime_temp.wDay == pt->wDay + 1) || ((systemtime_temp.wDay + 1) < pt->wDay))
	{
		b_FlagNextDay = TRUE;
		if ((systemtime_temp.wDay + 1) < pt->wDay) b_FlagNextMonth = TRUE;
	}
	else if ((systemtime_temp.wDay + 1 == pt->wDay) || (systemtime_temp.wDay > (pt->wDay + 1))) 
	{
		b_FlagPrevDay = TRUE;
		if (systemtime_temp.wDay > (pt->wDay + 1)) b_FlagPrevMonth = TRUE;
	}

	return(systemtime_temp);
}














SYSTEMTIME CalcTimeDifference_US_Win10(SYSTEMTIME* pt, int td_h, int td_m, BOOL pol_neg)
{
	SYSTEMTIME systemtime_temp;
	FILETIME filetime_temp;
	ULONGLONG ulonglong_temp;
	systemtime_temp = *pt;
	ULARGE_INTEGER ularge_integer_temp;
	int i = 0;

	SystemTimeToFileTime(&systemtime_temp, &filetime_temp);

	ularge_integer_temp.HighPart = filetime_temp.dwHighDateTime;
	ularge_integer_temp.LowPart = filetime_temp.dwLowDateTime;

	if (pol_neg)
	{
		ularge_integer_temp.QuadPart -= (ULONGLONG)(td_h * 60 + td_m) * 600000000;
	}
	else
	{
		ularge_integer_temp.QuadPart += (ULONGLONG)(td_h * 60 + td_m) * 600000000;
	}


	filetime_temp.dwHighDateTime = ularge_integer_temp.HighPart;
	filetime_temp.dwLowDateTime = ularge_integer_temp.LowPart;

	FileTimeToSystemTime(&filetime_temp, &systemtime_temp);

	i = systemtime_temp.wDayOfWeek;
	if (i == 0) i = 7;


	if (((systemtime_temp.wMonth > 3) && (systemtime_temp.wMonth < 11))
		|| ((systemtime_temp.wMonth == 3) && ((systemtime_temp.wDay - i) > 7))
		|| ((systemtime_temp.wMonth == 3) && (systemtime_temp.wDay > 7) && (systemtime_temp.wDay <= 14) && (systemtime_temp.wDayOfWeek == 0) && (systemtime_temp.wHour >= 2))
		|| ((systemtime_temp.wMonth == 11) && (systemtime_temp.wDay <= systemtime_temp.wDayOfWeek))
		|| ((systemtime_temp.wMonth == 11) && (systemtime_temp.wDay <= 7) && (systemtime_temp.wDayOfWeek == 0) && (systemtime_temp.wHour == 0))
		) {
		b_SummerTime_US = TRUE;
	}
	else {
		b_SummerTime_US = FALSE;
	}

	if (b_SummerTime_US) {
		ularge_integer_temp.QuadPart += 36000000000;
		filetime_temp.dwHighDateTime = ularge_integer_temp.HighPart;
		filetime_temp.dwLowDateTime = ularge_integer_temp.LowPart;
		FileTimeToSystemTime(&filetime_temp, &systemtime_temp);
	}

	b_FlagPrevDay = FALSE;
	b_FlagNextDay = FALSE;
	b_FlagPrevMonth = FALSE;
	b_FlagNextMonth = FALSE;

	if ((systemtime_temp.wDay == pt->wDay + 1) || ((systemtime_temp.wDay + 1) < pt->wDay))
	{
		b_FlagNextDay = TRUE;
		if ((systemtime_temp.wDay + 1) < pt->wDay) b_FlagNextMonth = TRUE;
	}
	else if ((systemtime_temp.wDay + 1 == pt->wDay) || (systemtime_temp.wDay > (pt->wDay + 1)))
	{
		b_FlagPrevDay = TRUE;
		if (systemtime_temp.wDay > (pt->wDay + 1)) b_FlagPrevMonth = TRUE;
	}

	return(systemtime_temp);
}



//This function is not called in 2022 or later.
SYSTEMTIME CalcTimeDifference_Europe_Win10(SYSTEMTIME* pt, int td_h, int td_m, BOOL pol_neg)
{
	SYSTEMTIME systemtime_temp, systemtime_utc;
	FILETIME filetime_temp, filetime_utc;
	ULONGLONG ulonglong_temp;
	ULARGE_INTEGER ularge_integer_temp;
	int i = 0;

	extern int currentTimeZoneBiasMin;

	systemtime_temp = *pt;

	SystemTimeToFileTime(&systemtime_temp, &filetime_temp);

	ularge_integer_temp.HighPart = filetime_temp.dwHighDateTime;
	ularge_integer_temp.LowPart = filetime_temp.dwLowDateTime;

	ularge_integer_temp.QuadPart += (LONGLONG)currentTimeZoneBiasMin * 600000000;		//UTC

	filetime_utc.dwHighDateTime = ularge_integer_temp.HighPart;
	filetime_utc.dwLowDateTime = ularge_integer_temp.LowPart;

	FileTimeToSystemTime(&filetime_utc, &systemtime_utc);

	i = systemtime_utc.wDayOfWeek;
	if (i == 0) i = 7;



	if (((systemtime_utc.wMonth > 3) && (systemtime_utc.wMonth < 10))
		|| ((systemtime_utc.wMonth == 3) && ((systemtime_utc.wDay + 7 - i) > 31))
		|| ((systemtime_utc.wMonth == 3) && (systemtime_utc.wDay >= 24) && (i == 7) && (systemtime_utc.wHour >= 1))
		|| ((systemtime_utc.wMonth == 10) && ((systemtime_utc.wDay + 7 - systemtime_utc.wDayOfWeek) <= 31))
		|| ((systemtime_utc.wMonth == 10) && (systemtime_utc.wDay >= 24) && (i == 7) && (systemtime_utc.wHour == 0))
		) {
		b_SummerTime_Europe = TRUE;
	}
	else {
		b_SummerTime_Europe = FALSE;
	}


	ularge_integer_temp.HighPart = filetime_temp.dwHighDateTime;
	ularge_integer_temp.LowPart = filetime_temp.dwLowDateTime;

	if (pol_neg)
	{
		ularge_integer_temp.QuadPart -= (ULONGLONG)(td_h * 60 + td_m) * 600000000;
	}
	else
	{
		ularge_integer_temp.QuadPart += (ULONGLONG)(td_h * 60 + td_m) * 600000000;
	}

	if (b_SummerTime_Europe) {
		ularge_integer_temp.QuadPart += 36000000000;
	}

	filetime_temp.dwHighDateTime = ularge_integer_temp.HighPart;
	filetime_temp.dwLowDateTime = ularge_integer_temp.LowPart;

	FileTimeToSystemTime(&filetime_temp, &systemtime_temp);


	b_FlagPrevDay = FALSE;
	b_FlagNextDay = FALSE;
	b_FlagPrevMonth = FALSE;
	b_FlagNextMonth = FALSE;

	if ((systemtime_temp.wDay == pt->wDay + 1) || ((systemtime_temp.wDay + 1) < pt->wDay))
	{
		b_FlagNextDay = TRUE;
		if ((systemtime_temp.wDay + 1) < pt->wDay) b_FlagNextMonth = TRUE;
	}
	else if ((systemtime_temp.wDay + 1 == pt->wDay) || (systemtime_temp.wDay > (pt->wDay + 1)))
	{
		b_FlagPrevDay = TRUE;
		if (systemtime_temp.wDay > (pt->wDay + 1)) b_FlagPrevMonth = TRUE;
	}

	return(systemtime_temp);
}