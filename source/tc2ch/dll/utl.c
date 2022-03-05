/*-------------------------------------------
  utl.c
  misc functions
  KAZUBON 1997-1999
---------------------------------------------*/

#include "tcdll.h"

extern HANDLE hmod;

BOOL g_bIniSetting = FALSE;
char g_inifile[MAX_PATH];

BOOL flag_LogClear = FALSE;


BOOL b_AutoClearLogFile_back;
extern BOOL b_AutoClearLogFile;

/*-------------------------------------------
ログファイルのクリアを抑止する by TTTT
 ---------------------------------------------*/
void SuspendClearLog_Win10(void)
{
	b_AutoClearLogFile_back = b_AutoClearLogFile;
	b_AutoClearLogFile = FALSE;
}

/*-------------------------------------------
ログファイルのクリア指定を復帰する by TTTT
---------------------------------------------*/
void RecoverClearLog_Win10(void)
{
	b_AutoClearLogFile = b_AutoClearLogFile_back;
}



int _strncmp(const char* d, const char* s, size_t n)
{
	unsigned int i;
	for(i = 0; i < n; i++)
	{
		if(*s == 0 && *d == 0) break;
		if(*d != *s) return (*d - *s);
		d++; s++;
	}
	return 0;
}

/*-------------------------------------------
　パス名にファイル名をつける
---------------------------------------------*/
void add_title(char *path, char *title)
{
	char *p;

	p = path;

	if(*title && *(title + 1) == ':') ;
	else if(*title == '\\')
	{
		if(*p && *(p + 1) == ':') p += 2;
	}
	else
	{
		while(*p)
		{
			if((*p == '\\' || *p == '/') && *(p + 1) == 0)
			{
				break;
			}
			p = CharNext(p);
		}
		*p++ = '\\';
	}
	while(*title) *p++ = *title++;
	*p = 0;
}

/*-------------------------------------------
　パス名からファイル名をとりのぞく
---------------------------------------------*/
void del_title(char *path)
{
	char *p, *ep;

	p = ep = path;
	while(*p)
	{
		if(*p == '\\' || *p == '/')
		{
			if(p > path && *(p - 1) == ':') ep = p + 1;
			else ep = p;
		}
		p = CharNext(p);
	}
	*ep = 0;
}


/*-------------------------------------------
　パス名からファイル名を得る	copied from utl.c in tclock by TTTT
 ---------------------------------------------*/
void get_title(char* dst, const char *path)
{
	const char *p, *ep;

	p = ep = path;
	while (*p)
	{
		if (*p == '\\' || *p == '/')
		{
			if (p > path && *(p - 1) == ':') ep = p + 1;
			else ep = p;
		}
		p = CharNext(p);
	}

	if (*ep == '\\' || *ep == '/') ep++;

	while (*ep) *dst++ = *ep++;
	*dst = 0;
}





/*------------------------------------------------
	カンマで区切られた文字列を取り出す
--------------------------------------------------*/
void parse(char *dst, char *src, int n)
{
	char *dp;
	int i;

	for(i = 0; i < n; i++)
	{
		while(*src && *src != ',') src++;
		if(*src == ',') src++;
	}
	if(*src == 0)
	{
		*dst = 0; return;
	}

	while(*src == ' ') src++;

	dp = dst;
	while(*src && *src != ',') *dst++ = *src++;
	*dst = 0;

	while(dst != dp)
	{
		dst--;
		if(*dst == ' ') *dst = 0;
		else break;
	}
}

/*-------------------------------------------
  returns a resource string
---------------------------------------------*/
char* MyString(UINT id)
{
	static char buf[80];

	if(LoadString(hmod, id, buf, 80) == 0)
		buf[0] = 0;

	return buf;
}

char mykey[] = "Software\\Kazubon\\TClock";

/*------------------------------------------------
　自分のレジストリから文字列を得る
--------------------------------------------------*/
int GetMyRegStr(char* section, char* entry, char* val, int cbData,
	char* defval)
{
	char key[80];
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	int r = 0;

	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);

	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}

	if(g_bIniSetting)
	{
		r = GetPrivateProfileString(key, entry, defval, val,
			cbData, g_inifile);
	}
	else
	{
		b = FALSE;
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			size = cbData;
			if(RegQueryValueEx(hkey, entry, 0, &regtype,
				(LPBYTE)val, &size) == 0)
			{
				if(size == 0) *val = 0;
				r = size;
				b = TRUE;
			}
			RegCloseKey(hkey);
		}
		if(b == FALSE)
		{
			strcpy(val, defval);
			r = strlen(defval);
		}
	}

	extern BOOL b_DebugLog_RegAccess;
	if (b_DebugLog_RegAccess)
	{
		char strLog[256];
		wsprintf(strLog, "***** GetMyRegStr, %s, %s called and returned string is %s", section, entry, val);
		writeDebugLog_Win10(strLog, 999);
	}

	return r;
}



/*------------------------------------------------
　自分のレジストリからLONG値を得る
--------------------------------------------------*/
LONG GetMyRegLong(char* section, char* entry, LONG defval)
{
	char key[80];
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	LONG r = 0;




	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);

	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}

	if(g_bIniSetting)
	{
		r = GetPrivateProfileInt(key, entry, defval, g_inifile);
	}
	else
	{
		b = FALSE;
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			size = 4;
			if(RegQueryValueEx(hkey, entry, 0, &regtype,
				(LPBYTE)&r, &size) == 0)
			{
				if(size == 4) b = TRUE;
			}
			RegCloseKey(hkey);
		}
		if(b == FALSE) r = defval;
	}



	extern BOOL b_DebugLog_RegAccess;
	if (b_DebugLog_RegAccess)
	{
		char strLog[256];
		wsprintf(strLog, "***** GetMyRegLong, %s, %s called and returned value is %d", section, entry, (int)r);
		writeDebugLog_Win10(strLog, 999);
	}

	return r;
}



/*-------------------------------------------
　レジストリに文字列を書き込む
---------------------------------------------*/
BOOL SetMyRegStr(char* section, char* entry, char* val)
{
	HKEY hkey;
	BOOL r;
	char key[80];

	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);

	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}

	if(g_bIniSetting)
	{
		char *chk_val;
		BOOL b_chkflg = FALSE;
		char saveval[1024];

		r = FALSE;
		chk_val = val;
		while(*chk_val)
		{
			if (*chk_val == '\"' || *chk_val == '\'' || *chk_val == ' '  )
				b_chkflg = TRUE;
			chk_val++;
		}

		if (b_chkflg)
		{
			strcpy(saveval,"\"");
			strcat(saveval,val);
			strcat(saveval,"\"");
		}
		else
			strcpy(saveval,val);

		if(WritePrivateProfileString(key, entry, saveval, g_inifile))
			r = TRUE;
	}
	else
	{
		r = FALSE;
		if(RegCreateKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			if(RegSetValueEx(hkey, entry, 0, REG_SZ,
				(CONST BYTE*)val, strlen(val)) == 0)
			{
				r = TRUE;
			}
			RegCloseKey(hkey);
		}
	}

	extern BOOL b_DebugLog_RegAccess;
	if (b_DebugLog_RegAccess)
	{
		char strLog[256];
		wsprintf(strLog, "***** SetMyRegStr, %s, %s called to save string %s", section, entry, val);
		writeDebugLog_Win10(strLog, 999);
	}

	return r;
}

/*-------------------------------------------
　レジストリにDWORD値を書き込む
---------------------------------------------*/
BOOL SetMyRegLong(char* section, char* entry, DWORD val)
{
	HKEY hkey;
	BOOL r;
	char key[80];

	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, mykey);

	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}

	if(g_bIniSetting)
	{
		char s[20];
		wsprintf(s, "%d", val);
		r = FALSE;
		if(WritePrivateProfileString(key, entry, s, g_inifile))
			r = TRUE;
	}
	else
	{
		r = FALSE;
		if(RegCreateKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			if(RegSetValueEx(hkey, entry, 0, REG_DWORD,
				(CONST BYTE*)&val, 4) == 0)
			{
				r = TRUE;
			}
			RegCloseKey(hkey);
		}
	}

	extern BOOL b_DebugLog_RegAccess;
	if (b_DebugLog_RegAccess)
	{
		char strLog[256];
		wsprintf(strLog, "***** SetMyRegLong, %s, %s called to save value %d", section, entry, (int)val);
		writeDebugLog_Win10(strLog, 999);
	}

	return r;
}


/*------------------------------------------------
　デバッグ用 with New API	Adde by TTTT
 --------------------------------------------------*/
void WriteDebugDLL_New(LPSTR s)
{

	HANDLE hFile;
	DWORD dwWriteSize;
	char fname[MAX_PATH];
	extern BOOL b_AutoClearLogFile;
	extern int LogLineCount;
	extern int AutoClearLogLines;
	extern char g_mydir_dll[];
	char strTemp[1024];

	strcpy(fname, g_mydir_dll);
	add_title(fname, "TCLOCK-WIN10-DEBUG.LOG");

	LogLineCount++;

	if (b_AutoClearLogFile && (LogLineCount > AutoClearLogLines))
	{
		hFile = CreateFile(
			fname, FILE_APPEND_DATA, FILE_SHARE_READ, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			LogLineCount = 0;
		}
	}
	else
	{
		hFile = CreateFile(
			fname, FILE_APPEND_DATA, FILE_SHARE_READ, NULL,
			OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if (hFile != INVALID_HANDLE_VALUE)
	{
		wsprintf(strTemp, "[tcdll:%4d] ", LogLineCount);
		WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);
		WriteFile(hFile, s, lstrlen(s), &dwWriteSize, NULL);
		WriteFile(hFile, "\x0d\x0a", lstrlen("\x0d\x0a"), &dwWriteSize, NULL);
		CloseHandle(hFile);
	}
}





// 与えられたファイル名が相対パスならば
// TClockのフォルダからの絶対パスに変換
PSTR CreateFullPathName(HINSTANCE hmod, PSTR fname)
{
	int len;
	int tlen;
	char szTClockPath[MAX_PATH];
	PSTR pstr;

	if (hmod == NULL) {
		return NULL;
	}
	if (fname == NULL) {
		return NULL;
	}
	if (*fname == '\0') {
		return NULL;
	}

	// \\NAME\C\path\path\filename.txt
	// C:\path\path\filename.txt
	// 以上の絶対パス以外を相対パスと判断して
	// その前にTClockのパスを基準ディレクトリとして付加
	len = strlen(fname);
	if (len >= 2) {
		if ((*fname == '\\') && (*(fname + 1) == '\\')) {
			//UNC name
			return NULL;
		} else if (*(fname + 1) == ':') {
			return NULL;
		}
	}

	// TClockの位置を基準パスとして指定文字列を相対パスとして追加
	if (GetModuleFileName(hmod, szTClockPath, MAX_PATH) == 0) {
		return NULL;
	}
	del_title(szTClockPath);
	tlen = strlen(szTClockPath);

	pstr = malloc(tlen + len + 2);
	if (pstr == NULL) {
		return NULL;
	}
	strcpy(pstr, szTClockPath);
	add_title(pstr, fname);

	return pstr;
}


/*-------------------------------------------
Normal Operation Log by TTTT
---------------------------------------------*/
void WriteNormalLog_DLL(const char* s)
{
	HANDLE hFile;
	DWORD dwWriteSize;
	char fname[MAX_PATH];
	extern char g_mydir_dll[];


	SYSTEMTIME systemtime;
	char strTemp[1024];
	GetLocalTime(&systemtime);

	{
		wsprintf(strTemp, "%d/%02d/%02d %02d:%02d:%02d.%03d %s",
			systemtime.wYear, systemtime.wMonth, systemtime.wDay,
			systemtime.wHour, systemtime.wMinute, systemtime.wSecond,
			systemtime.wMilliseconds, s);
	}


	//tickCount_LastLog = tickCount_LastLog_temp;

	strcpy(fname, g_mydir_dll);
	add_title(fname, "TClock-Win10.log");

	hFile = CreateFile(
		fname, FILE_APPEND_DATA, FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE) {

		WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);
		WriteFile(hFile, "\x0d\x0a", lstrlen("\x0d\x0a"), &dwWriteSize, NULL);

		CloseHandle(hFile);
	}
}
