/*-------------------------------------------
  utl.c
    その他の関数
---------------------------------------------*/

#include "tclock.h"


//#if defined(_MSC_VER) && (_MSC_VER >= 1200)
typedef LARGE_INTEGER TC_SINT64;
typedef ULARGE_INTEGER TC_UINT64;
//#else
//typedef union _TC_SINT64 {
//    struct {
//        DWORD LowPart;
//        LONG HighPart;
//    } u;
//    LONGLONG QuadPart;
//} TC_SINT64;
//typedef union _TC_UINT64 {
//    struct {
//        DWORD LowPart;
//        DWORD HighPart;
//    } u;
//    ULONGLONG QuadPart;
//} TC_UINT64;
//#endif

/*-------------------------------------------
  ランタイム関数の代用
---------------------------------------------*/
void r_memcpy(void *d, const void *s, size_t l)
{
	size_t i;
	for (i = 0; i < l; i++) ((char *)d)[i] = ((const char *)s)[i];
}

void r_memset(void *d, int c, size_t l)
{
	size_t i;
	for (i = 0; i < l; i++) ((char *)d)[i] = (char)c;
}

int r_atoi(const char *p)
{
	int r = 0;
	while(*p)
	{
		if('0' <= *p && *p <= '9')
			r = r * 10 + *p - '0';
		p++;

	}
	return r;
}

int atox(const char *p)
{
	int r = 0;
	while(*p)
	{
		if('0' <= *p && *p <= '9')
			r = r * 16 + *p - '0';
		else if('A' <= *p && *p <= 'F')
			r = r * 16 + *p - 'A' + 10;
		else if('a' <= *p && *p <= 'f')
			r = r * 16 + *p - 'a' + 10;
		p++;

	}
	return r;
}

__inline int r_toupper(int c)
{
	if('a' <= c && c <= 'z')
		c -= 'a' - 'A';
	return c;
}

int r_strnicmp(const char* d, const char* s, size_t n)
{
	int c1, c2;
	unsigned int i;
	for(i = 0; i < n; i++)
	{
		if(*s == 0 && *d == 0) break;
		c1 = r_toupper(*d); c2 = r_toupper(*s);
		if(c1 != c2) return (c1 - c2);
		d++; s++;
	}
	return 0;
}

int r_stricmp(const char* d, const char* s)
{
	int c1, c2;
	for (;;)
	{
		if(*s == 0 && *d == 0) break;
		c1 = r_toupper(*d); c2 = r_toupper(*s);
		if(c1 != c2) return (c1 - c2);
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

	if(*p == 0) ;
	else if(*title && *(title + 1) == ':') ;
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
　パス名からファイル名を得る
---------------------------------------------*/
void get_title(char* dst, const char *path)
{
	const char *p, *ep;

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

	if(*ep == '\\' || *ep == '/') ep++;

	while(*ep) *dst++ = *ep++;
	*dst = 0;
}




/*------------------------------------------------
　ファイルの拡張子の比較
--------------------------------------------------*/
int ext_cmp(const char *fname, const char *ext)
{
	const char* p, *sp;

	sp = NULL; p = fname;
	while(*p)
	{
		if(*p == '.') sp = p;
		else if(*p == '\\' || *p == '/') sp = NULL;
		p = CharNext(p);
	}

	if(sp == NULL) sp = p;
	if(*sp == '.') sp++;

	for (;;)
	{
		if(*sp == 0 && *ext == 0) return 0;
		if(r_toupper(*sp) != r_toupper(*ext))
			return (r_toupper(*sp) - r_toupper(*ext));
		sp++; ext++;
	}
	//return 0;
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

/*------------------------------------------------
	文字で区切られた文字列を取り出す
--------------------------------------------------*/
void parsechar(char *dst, char *src, char ch, int n)
{
	char *dp;
	int i;

	for(i = 0; i < n; i++)
	{
		while(*src && *src != ch) src++;
		if(*src == ch) src++;
	}
	if(*src == 0)
	{
		*dst = 0; return;
	}

	while(*src == ' ') src++;

	dp = dst;
	while(*src && *src != ch) *dst++ = *src++;
	*dst = 0;

	while(dst != dp)
	{
		dst--;
		if(*dst == ' ') *dst = 0;
		else break;
	}
}

/*------------------------------------------------
　'\0'で終了する文字列を追加する
　最後は"\0\0"で終了
--------------------------------------------------*/
void str0cat(char* dst, const char* src)
{
	char* p;
	p = dst;
	while(*p) { while(*p) p++; p++; }
	strcpy(p, src);
	while(*p) p++; p++; *p = 0;
}

/*-------------------------------------------
  returns a resource string
---------------------------------------------*/
char* MyString(UINT id)
{
	static char buf[MAX_PATH];
	HINSTANCE hInst;

	extern int Language_Offset;

	buf[0] = 0;
	hInst = GetLangModule();
	if(hInst) LoadString(hInst, id + Language_Offset, buf, MAX_PATH);

	if (strlen(buf) == 0) strcpy(buf, "NG_String");

	return buf;
}

/*-------------------------------------------
  アイコンつきメッセージボックス
---------------------------------------------*/
int MyMessageBox(HWND hwnd, char* msg, char* title, UINT uType, UINT uBeep)
{
	MSGBOXPARAMS mbp;

	mbp.cbSize = sizeof(MSGBOXPARAMS);
	mbp.hwndOwner = hwnd;
	mbp.hInstance = g_hInst;
	mbp.lpszText = msg;
	mbp.lpszCaption = title;
	mbp.dwStyle = MB_USERICON | uType;
	mbp.lpszIcon = MAKEINTRESOURCE(IDI_ICON1);
	mbp.dwContextHelpId = 0;
	mbp.lpfnMsgBoxCallback = NULL;
	mbp.dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
	if(uBeep != 0xFFFFFFFF)
		MessageBeep(uBeep);
	return MessageBoxIndirect(&mbp);
}

/*------------------------------------------------
  get locale info for 95/NT
--------------------------------------------------*/
int GetLocaleInfoWA(int ilang, LCTYPE LCType, char* dst, int n)
{
	int r;
	LCID Locale;

	*dst = 0;
	Locale = MAKELCID((WORD)ilang, SORT_DEFAULT);
	//if(GetVersion() & 0x80000000) // 95
	//	r = GetLocaleInfoA(Locale, LCType, dst, n);
	//else  // NT
	{
		WCHAR* pw;
		pw = (WCHAR*)GlobalAllocPtr(GHND, sizeof(WCHAR)*(n+1));
		r = GetLocaleInfoW(Locale, LCType, pw, n);
		if(r)
			WideCharToMultiByte(CP_ACP, 0, pw, -1, dst, n,
				NULL, NULL);
		GlobalFreePtr(pw);
	}
	return r;
}

/*-------------------------------------------
  32bit x 32bit = 64bit
---------------------------------------------*/
DWORDLONG M32x32to64(DWORD a, DWORD b)
{
	TC_UINT64 r;
	DWORD *p1, *p2, *p3;
	r.QuadPart = 0;
	p1 = &r.u.LowPart;
	p2 = (DWORD*)((BYTE*)p1 + 2);
	p3 = (DWORD*)((BYTE*)p2 + 2);
	*p1 = LOWORD(a) * LOWORD(b);
	*p2 += LOWORD(a) * HIWORD(b) + HIWORD(a) * LOWORD(b);
	*p3 += HIWORD(a) * HIWORD(b);
	return *(DWORDLONG*)(&r);
}

/*-------------------------------------------
  SetForegroundWindow for Windows98
---------------------------------------------*/
void SetForegroundWindow98(HWND hwnd)
{
	DWORD dwVer;

	{
		DWORD thread1, thread2;
		DWORD pid;
		thread1 = GetWindowThreadProcessId(
			GetForegroundWindow(), &pid);
		thread2 = GetCurrentThreadId();
		AttachThreadInput(thread2, thread1, TRUE);
		SetForegroundWindow(hwnd);
		AttachThreadInput(thread2, thread1, FALSE);
		BringWindowToTop(hwnd);
	}
	//else  // Win95/NT
	//	SetForegroundWindow(hwnd);
}


/*-------------------------------------------
for debugging, with new API by TTTT
---------------------------------------------*/
//void WriteDebug_New(const char* s)
//{
//	HANDLE hFile;
//	DWORD dwWriteSize;
//	char fname[MAX_PATH];
//
//	strcpy(fname, g_mydir);
//	add_title(fname, "TCLOCK-WIN10-DEBUG.LOG");
//
//
//
//	hFile = CreateFile(
//		fname, FILE_APPEND_DATA, FILE_SHARE_READ, NULL,
//		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
//	if (hFile != INVALID_HANDLE_VALUE) {
//		WriteFile(hFile, "[TClock main] ", lstrlen("[TClock main] "), &dwWriteSize, NULL);
//		WriteFile(hFile, s, lstrlen(s), &dwWriteSize, NULL);
//		WriteFile(hFile, "\x0d\x0a", lstrlen("\x0d\x0a"), &dwWriteSize, NULL);
//		CloseHandle(hFile);
//	}
//}


//DWORD tickCount_LastLog = 0;

/*-------------------------------------------
for debugging, with new API by TTTT
---------------------------------------------*/
void WriteDebug_New2(const char* s)
{
	HANDLE hFile;
	DWORD dwWriteSize;
	char fname[MAX_PATH];

	
	SYSTEMTIME systemtime;
	char strTemp[1024];
	GetLocalTime(&systemtime);
	//DWORD tickCount_LastLog_temp = 0;
	//tickCount_LastLog_temp = GetTickCount();

	{
		wsprintf(strTemp, "[TCock Main]   %d/%02d/%02d %02d:%02d:%02d.%03d %s",
			systemtime.wYear, systemtime.wMonth, systemtime.wDay,
			systemtime.wHour, systemtime.wMinute, systemtime.wSecond,
			systemtime.wMilliseconds, s);
	}
	

	//tickCount_LastLog = tickCount_LastLog_temp;

	strcpy(fname, g_mydir);
	add_title(fname, "TCLOCK-WIN10-DEBUG.LOG");

	hFile = CreateFile(
		fname, FILE_APPEND_DATA, FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		//WriteFile(hFile, "[TClock main] ", lstrlen("[TClock main] "), &dwWriteSize, NULL);
		//WriteFile(hFile, s, lstrlen(s), &dwWriteSize, NULL);

		WriteFile(hFile, strTemp, lstrlen(strTemp), &dwWriteSize, NULL);
		WriteFile(hFile, "\x0d\x0a", lstrlen("\x0d\x0a"), &dwWriteSize, NULL);

		CloseHandle(hFile);
	}
}





/*-------------------------------------------
Normal Operation Log by TTTT
---------------------------------------------*/
void WriteNormalLog(const char* s)
{
	HANDLE hFile;
	DWORD dwWriteSize;
	char fname[MAX_PATH];


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

	strcpy(fname, g_mydir);
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



/*-------------------------------------------
Clear Normal Operation Log by size with new API by TTTT
---------------------------------------------*/
void CheckNormalLog(void)
{
	HANDLE hFile;
	DWORD tempFileSize;
	char fname[MAX_PATH];



	strcpy(fname, g_mydir);
	add_title(fname, "TClock-Win10.log");

	hFile = CreateFile(
		fname, FILE_APPEND_DATA, FILE_SHARE_READ, NULL,
		OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		tempFileSize = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
		if (tempFileSize > 10240) DeleteFile(fname);
	}
}



//TClock用のレジストリのキー
#define mykey "Software\\Kazubon\\TClock"
#define mykey2 "Software\\Kazubon"
/*----------------------------------------------
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

	if (strlen(g_inifile) == 0)return;

	key[0] = 0;

	if(section && *section)
	{
		strcat(key, section);
	}
	else
	{
		strcpy(key, "Main");
	}


	r = GetPrivateProfileString(key, entry, defval, val,
			cbData, g_inifile);


	extern BOOL b_DebugLog_RegAccess;
	if (b_DebugLog_RegAccess)
	{
		char strLog[256];
		wsprintf(strLog, "***** GetMyRegStr, %s, %s called and returned string is %s", section, entry, val);
		WriteDebug_New2(strLog);
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

	if (strlen(g_inifile) == 0)return;


	key[0] = 0;

	if(section && *section)
	{
		strcat(key, section);
	}
	else
	{
		strcpy(key, "Main");
	}


	r = GetPrivateProfileInt(key, entry, defval, g_inifile);


	extern BOOL b_DebugLog_RegAccess;
	if (b_DebugLog_RegAccess)
	{
		char strLog[256];
		wsprintf(strLog, "***** GetMyRegLong, %s, %s called and returned value is %d", section, entry, (int)r);
		WriteDebug_New2(strLog);
	}

	return r;
}



/*-------------------------------------------
　レジストリに文字列を書き込む
---------------------------------------------*/
BOOL SetMyRegStr(char* section, char* entry, char* val)
{
	HKEY hkey;
	BOOL r = FALSE;
	char key[80];

	if (strlen(g_inifile) == 0)return;


	key[0] = 0;

	if(section && *section)
	{
		strcat(key, section);
	}
	else
	{
		strcpy(key, "Main");
	}


	char *chk_val;
	BOOL b_chkflg = FALSE;
	char saveval[1024];


	chk_val = val;
	while (*chk_val)
	{
		if (*chk_val == '\"' || *chk_val == '\'' || *chk_val == ' ')
			b_chkflg = TRUE;
		chk_val++;
	}

	if (b_chkflg)
	{
		strcpy(saveval, "\"");
		strcat(saveval, val);
		strcat(saveval, "\"");
	}
	else
		strcpy(saveval, val);

	if (WritePrivateProfileString(key, entry, saveval, g_inifile)) {
		r = TRUE;
	}


	extern BOOL b_DebugLog_RegAccess;
	if (b_DebugLog_RegAccess)
	{
		char strLog[256];
		wsprintf(strLog, "***** SetMyRegStr, %s, %s called to save string %s", section, entry, val);
		WriteDebug_New2(strLog);
	}


	return r;
}


/*-------------------------------------------
　レジストリにDWORD値を書き込む
---------------------------------------------*/
BOOL SetMyRegLong(char* section, char* entry, DWORD val)
{
	HKEY hkey;
	BOOL r = FALSE;
	char key[80];


	if (strlen(g_inifile) == 0)return;

	key[0] = 0;

	if(section && *section)
	{
		strcat(key, section);
	}
	else
	{
		strcpy(key, "Main");
	}


	char s[20];
	wsprintf(s, "%d", val);
	if (WritePrivateProfileString(key, entry, s, g_inifile)) {
		r = TRUE;
	}

	extern BOOL b_DebugLog_RegAccess;
	if (b_DebugLog_RegAccess)
	{
		char strLog[256];
		wsprintf(strLog, "***** SetMyRegLong, %s, %s called to save value %d", section, entry, (int)val);
		WriteDebug_New2(strLog);
	}

	return r;
}



/*-------------------------------------------
　レジストリの値を削除
---------------------------------------------*/
BOOL DelMyReg(char* section, char* entry)
{
	BOOL r = FALSE;
	char key[80];
	HKEY hkey;

	if (strlen(g_inifile) == 0)return;

	key[0] = 0;

	if(section && *section)
	{
		strcat(key, section);
	}
	else
	{
		strcpy(key, "Main");
	}


	if (WritePrivateProfileString(key, entry, NULL, g_inifile)) {
		r = TRUE;
	}

	return r;
}

/*-------------------------------------------
　レジストリのキーを削除
---------------------------------------------*/
BOOL DelMyRegKey(char* section)
{
	BOOL r = FALSE;
	char key[80];

	if (strlen(g_inifile) == 0)return;

	key[0] = 0;

	if(section && *section)
	{
		strcat(key, section);
	}
	else
	{
		strcpy(key, "Main");
	}

	if (WritePrivateProfileSection(key, NULL, g_inifile)) {
		r = TRUE;
	}

	return r;
}


int DelRegAll(void)
{
	char key[80], name[80], class[80];
	DWORD cbName, cbClass;
	BOOL r;
	HKEY hkey;
	FILETIME ft;
	int i;

	strcpy(key, mykey);

	if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
	{
		for(i = 0; ; i++)
		{
			cbName = 80; cbClass = 80;
			if(RegEnumKeyEx(hkey, i, name, &cbName, NULL, class, &cbClass, &ft) != ERROR_SUCCESS)
				break;
			strcpy(key, mykey);
			strcat(key, "\\");
			strcat(key, name);
			RegDeleteKey(HKEY_CURRENT_USER, key);
		}
		RegCloseKey(hkey);
	}

	r = FALSE;
	RegDeleteKey(HKEY_CURRENT_USER, mykey);
	if(RegDeleteKey(HKEY_CURRENT_USER, mykey2) == 0)
		r = TRUE;
	if (r)
		return 1;
	else
		return 2;
}

/*-------------------------------------------
　コンボボックスのリスト項目の表示数を指定する
---------------------------------------------*/

void AdjustConboBoxDropDown(HWND hComboBox, int nDropDownCount)
{
	int nCount, nItemHeight;
	RECT rect;

	nCount = SendMessage(hComboBox, CB_GETCOUNT, 0, 0);
	if (nCount > nDropDownCount) nCount = nDropDownCount;
	if (nCount < 1) nCount = 1;

	GetWindowRect(hComboBox, &rect);
	nItemHeight = SendMessage(hComboBox, CB_GETITEMHEIGHT, 0, 0);
	SetWindowPos(hComboBox, 0, 0, 0, rect.right - rect.left,
		nItemHeight * nCount + rect.bottom - rect.top + 2,
		SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_SHOWWINDOW);
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



// Vistaのカレンダーが表示されていればそのハンドルを返す
HWND FindVistaCalenderWindow(void)
{
	return FindWindowEx(FindWindow("ClockFlyoutWindow", NULL), NULL, "DirectUIHWND", "");
}
