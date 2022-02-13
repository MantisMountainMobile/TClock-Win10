/*-------------------------------------------
  font.c
  create a font of the clock
         KAZUBON 1999
---------------------------------------------*/
#include "tcdll.h"

struct {
	int cp;
	BYTE charset;
} codepage_charset[] = {
	{ 932,  SHIFTJIS_CHARSET },
	{ 936,  GB2312_CHARSET },
	{ 949,  HANGEUL_CHARSET },
	{ 950,  CHINESEBIG5_CHARSET },
	{ 1250, EASTEUROPE_CHARSET },
	{ 1251, RUSSIAN_CHARSET },
	{ 1252, ANSI_CHARSET },
	{ 1253, GREEK_CHARSET },
	{ 1254, TURKISH_CHARSET },
	{ 1257, BALTIC_CHARSET },
	{ 0, 0}
};

int GetLocaleInfoWA(WORD wLanguageID, LCTYPE LCType, char* dst, int n);

/*------------------------------------------------
   callback function for EnumFontFamiliesEx,
   to find a designated font
--------------------------------------------------*/
BOOL CALLBACK EnumFontFamExProc(ENUMLOGFONTEX* pelf,
	NEWTEXTMETRICEX* lpntm, int FontType, LPARAM fontname)
{
	UNREFERENCED_PARAMETER(lpntm);
	UNREFERENCED_PARAMETER(FontType);
	if(strcmp((LPSTR)fontname, pelf->elfLogFont.lfFaceName) == 0)
		return FALSE;
	return TRUE;
}

/*------------------------------------------------
   create a font of the clock
--------------------------------------------------*/
HFONT CreateMyFont(char* fontname, int fontsize,
	LONG weight, LONG italic)
{
	LOGFONT lf;
	POINT pt;
	HDC hdc;
	WORD langid;
	char s[11];
	int cp;
	BYTE charset;
	int i;

	memset(&lf, 0, sizeof(LOGFONT));

	langid = (WORD)GetMyRegLong("Format", "Locale", (int)GetUserDefaultLangID());
	cp = CP_ACP;
	if(GetLocaleInfoWA(langid, LOCALE_IDEFAULTANSICODEPAGE, s, 10) > 0)
	{
		char *p;
		p = s; cp = 0;
		while('0' <= *p && *p <= '9') cp = cp * 10 + *p++ - '0';
		if(!IsValidCodePage(cp)) cp = CP_ACP;
	}

	charset = 0;
	for(i = 0; codepage_charset[i].cp; i++)
	{
		if(cp == codepage_charset[i].cp)
		{
			charset = codepage_charset[i].charset; break;
		}
	}

	hdc = GetDC(NULL);

	// find a font named "fontname"
	if(charset == 0) charset = (BYTE)GetTextCharset(hdc);
	lf.lfCharSet = charset;
	if(EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc,
		(LPARAM)fontname, 0))
	{
		lf.lfCharSet = OEM_CHARSET;
		if(EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc,
			(LPARAM)fontname, 0))
		{
			lf.lfCharSet = ANSI_CHARSET;
			EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC)EnumFontFamExProc,
				(LPARAM)fontname, 0);
		}
	}

	pt.x = 0;
	pt.y = GetDeviceCaps(hdc, LOGPIXELSY) * fontsize / 72;
	DPtoLP(hdc, &pt, 1);
	lf.lfHeight = -pt.y;

	ReleaseDC(NULL, hdc);

	lf.lfWidth = lf.lfEscapement = lf.lfOrientation = 0;
	lf.lfWeight = weight;
	lf.lfItalic = (BYTE)italic;
	lf.lfUnderline = 0;
	lf.lfStrikeOut = 0;
	//lf.lfCharSet = ;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	strcpy(lf.lfFaceName, fontname);

	return CreateFontIndirect(&lf);
}


