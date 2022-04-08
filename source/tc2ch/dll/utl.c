/*-------------------------------------------
  utl.c
  misc functions
  KAZUBON 1997-1999
---------------------------------------------*/

#include "tcdll.h"

extern HANDLE hmod;

BOOL g_bIniSetting = TRUE;
char g_inifile[MAX_PATH];

BOOL flag_LogClear = FALSE;


BOOL b_AutoClearLogFile_back;
extern BOOL b_AutoClearLogFile;

/*-------------------------------------------
���O�t�@�C���̃N���A��}�~���� by TTTT
 ---------------------------------------------*/
void SuspendClearLog_Win10(void)
{
	b_AutoClearLogFile_back = b_AutoClearLogFile;
	b_AutoClearLogFile = FALSE;
}

/*-------------------------------------------
���O�t�@�C���̃N���A�w��𕜋A���� by TTTT
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
�@�p�X���Ƀt�@�C����������
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
�@�p�X������t�@�C�������Ƃ�̂���
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
�@�p�X������t�@�C�����𓾂�	copied from utl.c in tclock by TTTT
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
	�J���}�ŋ�؂�ꂽ����������o��
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
�@�����̃��W�X�g�����當����𓾂�
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


	r = GetPrivateProfileString(key, entry, defval, val, cbData, g_inifile);


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
�@�����̃��W�X�g������LONG�l�𓾂�
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
		writeDebugLog_Win10(strLog, 999);
	}

	return r;
}



/*-------------------------------------------
�@���W�X�g���ɕ��������������
---------------------------------------------*/
BOOL SetMyRegStr(char* section, char* entry, char* val)
{
	HKEY hkey;
	BOOL r;
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

		r = FALSE;
		chk_val = val;
		while(*chk_val)
		{
			if (*chk_val == '\"' || *chk_val == '\'' || *chk_val == ' ') {
				b_chkflg = TRUE;
			}
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

		if (WritePrivateProfileString(key, entry, saveval, g_inifile)) {
			r = TRUE;
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
�@���W�X�g����DWORD�l����������
---------------------------------------------*/
BOOL SetMyRegLong(char* section, char* entry, DWORD val)
{
	HKEY hkey;
	BOOL r;
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
		r = FALSE;
		if (WritePrivateProfileString(key, entry, s, g_inifile)) {
			r = TRUE;
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
�@�f�o�b�O�p with New API	Adde by TTTT
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





// �^����ꂽ�t�@�C���������΃p�X�Ȃ��
// TClock�̃t�H���_����̐�΃p�X�ɕϊ�
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
	// �ȏ�̐�΃p�X�ȊO�𑊑΃p�X�Ɣ��f����
	// ���̑O��TClock�̃p�X����f�B���N�g���Ƃ��ĕt��
	len = strlen(fname);
	if (len >= 2) {
		if ((*fname == '\\') && (*(fname + 1) == '\\')) {
			//UNC name
			return NULL;
		} else if (*(fname + 1) == ':') {
			return NULL;
		}
	}

	// TClock�̈ʒu����p�X�Ƃ��Ďw�蕶����𑊑΃p�X�Ƃ��Ēǉ�
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


/*-------------------------------------------
�@���W�X�g���̒l���폜
 ---------------------------------------------*/
BOOL DelMyReg_DLL(char* section, char* entry)
{
	BOOL r = FALSE;
	char key[80];
	HKEY hkey;

	if (strlen(g_inifile) == 0)return;

	key[0] = 0;

	if (section && *section)
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
�@���W�X�g���̃L�[���폜
 ---------------------------------------------*/
BOOL DelMyRegKey_DLL(char* section)
{
	BOOL r = FALSE;
	char key[80];

	if (strlen(g_inifile) == 0)return;

	key[0] = 0;

	if (section && *section)
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


void UpdateSettingFile(void)
{

}


void CleanSettingFile(void)
{
	//�p�~�����Z�N�V����
	DelMyRegKey_DLL("AppControl");
	DelMyRegKey_DLL("DataPlan");


	//�p�~�����G���g��
	DelMyReg_DLL("ETC", "DisplayString_Single");
	DelMyReg_DLL("ETC", "DisplayString_Clone");
	DelMyReg_DLL("ETC", "DisplayString_Extend");

	DelMyReg_DLL("Graph", "CpuHigh");
	
	DelMyReg_DLL("Main", "WarnDelayedUsageRetrieval");

	DelMyReg_DLL("Tooltip", "TipDispTime");
	DelMyReg_DLL("Tooltip", "TipDisableCustomDraw");
	DelMyReg_DLL("Tooltip", "TipEnableDoubleBuffering");
	DelMyReg_DLL("Tooltip", "TipDispInterval");
		
	DelMyReg_DLL("Win11", "UseTClockNotify");
	DelMyReg_DLL("Win11", "ShowWin11NotifyNumber");

	DelMyReg_DLL(NULL, "EnhanceSubClkOnDarkTray");
}

