/*-------------------------------------------
  newapi.c
  GradientFill and Layerd Window
  Kazubon 1999
---------------------------------------------*/

#include "tcdll.h"

#if (defined(_MSC_VER) && (_MSC_VER < 1200)) || (defined(__BORLANDC__) && (__BORLANDC__ < 0x550))

typedef USHORT COLOR16;

typedef struct _TRIVERTEX
{
    LONG    x;
    LONG    y;
    COLOR16 Red;
    COLOR16 Green;
    COLOR16 Blue;
    COLOR16 Alpha;
}TRIVERTEX,*PTRIVERTEX,*LPTRIVERTEX;

typedef struct _GRADIENT_RECT
{
    ULONG UpperLeft;
    ULONG LowerRight;
}GRADIENT_RECT,*PGRADIENT_RECT,*LPGRADIENT_RECT;

typedef struct _BLENDFUNCTION
{
    BYTE   BlendOp;
    BYTE   BlendFlags;
    BYTE   SourceConstantAlpha;
    BYTE   AlphaFormat;
} BLENDFUNCTION,*PBLENDFUNCTION;

#define AC_SRC_OVER                 0x00

#define GRADIENT_FILL_RECT_H    0x00000000
#define GRADIENT_FILL_RECT_V    0x00000001
#define GRADIENT_FILL_TRIANGLE  0x00000002
#define GRADIENT_FILL_OP_FLAG   0x000000ff

#endif

#if !defined(WS_EX_LAYERED)
#define WS_EX_LAYERED 0x80000
#endif

#if !defined(LWA_COLORKEY)
#define LWA_COLORKEY  1
#endif

#if !defined(LWA_ALPHA)
#define LWA_ALPHA     2
#endif





//Imported from TClock Redux by TTTT 20200918

static struct {
	DWORD flags;
	COLORREF color;
	BYTE alpha;
	char had_style;
} m_layered = { 0, 0, 0, -1 };
//static uint16_t m_theme_version = 0;
//static char m_theme_clock_part = CLP_TIME;
//#define TV_2000 0x0500
//#define TV_XP 0x0501
//#define TV_XP_64 0x0502
//#define TV_Vista 0x0600
//#define TV_7 0x0601
//#define TV_8 0x0602
//#define TV_8_1 0x0603
#define TV_10_beta 0x0604
#define TV_10 0x0a00


// DrawTheme
#define VSCLASS_CLOCK L"Clock"
#define VSCLASS_TASKBAND2 L"TaskBand2" /**< Win10+ */

#ifndef CLP_TIME
#	define CLP_TIME 1
#	define CLS_NORMAL 1
#	define CLS_HOT 2
#	define CLS_PRESSED 3
//#	define TMT_COLOR 204
#	define TMT_BACKGROUND 1602
#	define TMT_WINDOWTEXT 1609
#	define TMT_CAPTIONTEXT 1610
#	define TMT_BTNTEXT 1619
#	define TMT_INFOTEXT 1624
#	define TMT_TEXTCOLOR 3803

#	define TMT_TRANSPARENTCOLOR 3809
#	define TMT_WINDOW 1606
#	define TMT_WINDOWFRAME 1607
#	define TMT_FILLCOLOR 3802
#endif





//End of code imported from TClock Redux by TTTT 20200918






HMODULE hmodMSIMG32 = NULL;
typedef BOOL (WINAPI *pfnGradientFill)(HDC,PTRIVERTEX,ULONG,PVOID,ULONG,ULONG);
typedef BOOL (WINAPI *pfnAlphaBlend)(HDC,int,int,int,int,HDC,int,int,int,int,BLENDFUNCTION);
typedef BOOL (WINAPI *pfnTransparentBlt)(HDC,int,int,int,int,HDC,int,int,int,int,UINT);
pfnGradientFill pGradientFill = NULL;
pfnAlphaBlend pAlphaBlend = NULL;
pfnTransparentBlt pTransparentBlt = NULL;




HMODULE hmodUSER32 = NULL;
typedef BOOL (WINAPI *pfnSetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);
pfnSetLayeredWindowAttributes pSetLayeredWindowAttributes = NULL;

HMODULE hmodUXTHEME = NULL;
typedef HRESULT (WINAPI *pfnDrawThemeParentBackground)(HWND,HDC,RECT*);
pfnDrawThemeParentBackground pDrawThemeParentBackground = NULL;

typedef HRESULT(WINAPI *pfnOpenThemeData)(HWND, LPCWSTR);	//Adde by TTTT 20200918
pfnOpenThemeData pOpenThemeData = NULL;

typedef HRESULT(WINAPI *pfnCloseThemeData)(HTHEME);	//Adde by TTTT 20200918
pfnCloseThemeData pCloseThemeData = NULL;

typedef HRESULT(WINAPI *pfnDrawThemeBackground)(HTHEME, HDC, int, int, LPCRECT, LPCRECT);	//Adde by TTTT 20200918
pfnDrawThemeBackground pDrawThemeBackground = NULL;


typedef HRESULT(WINAPI *pfnGetThemeColor)(HTHEME, int, int, int, COLORREF*);
pfnGetThemeColor pGetThemeColor = NULL;


static BOOL bInitGradientFill = FALSE;
static BOOL bInitAlphaBlend = FALSE;
static BOOL bInitTransparentBlt = FALSE;
static BOOL bInitLayeredWindow = FALSE;
static BOOL bInitDrawTheme = FALSE;

static void RefreshRebar(HWND hwndBar);

int nAlphaStartMenu = 255;

static HTHEME hClockTheme = NULL;






void InitGradientFill(void)
{
	if(bInitGradientFill) return;

	hmodMSIMG32 = LoadLibrary("msimg32.dll");
	if(hmodMSIMG32 != NULL)
	{
		pGradientFill = (pfnGradientFill)GetProcAddress(hmodMSIMG32, "GradientFill");
		if(pGradientFill == NULL)
		{
			FreeLibrary(hmodMSIMG32); hmodMSIMG32 = NULL;
		}
	}
	bInitGradientFill = TRUE;
}

void InitAlphaBlend(void)
{
	if(bInitAlphaBlend) return;

	hmodMSIMG32 = LoadLibrary("msimg32.dll");
	if(hmodMSIMG32 != NULL)
	{
		pAlphaBlend = (pfnAlphaBlend)GetProcAddress(hmodMSIMG32, "AlphaBlend");
		if(pAlphaBlend == NULL)
		{
			FreeLibrary(hmodMSIMG32); hmodMSIMG32 = NULL;
		}
	}
	bInitAlphaBlend = TRUE;
}

void InitTransparentBlt(void)
{
	if(bInitTransparentBlt) return;

	hmodMSIMG32 = LoadLibrary("msimg32.dll");
	if(hmodMSIMG32 != NULL)
	{
		pTransparentBlt = (pfnTransparentBlt)GetProcAddress(hmodMSIMG32, "TransparentBlt");
		if(pTransparentBlt == NULL)
		{
			FreeLibrary(hmodMSIMG32); hmodMSIMG32 = NULL;
		}
	}
	bInitTransparentBlt = TRUE;
}

void InitLayeredWindow(void)
{
	if(bInitLayeredWindow) return;

	hmodUSER32 = LoadLibrary("user32.dll");
	if(hmodUSER32 != NULL)
	{
		pSetLayeredWindowAttributes = (pfnSetLayeredWindowAttributes)
			GetProcAddress(hmodUSER32, "SetLayeredWindowAttributes");
		if(pSetLayeredWindowAttributes == NULL)
		{
			FreeLibrary(hmodUSER32); hmodUSER32 = NULL;
		}
	}
	bInitLayeredWindow = TRUE;
}


void InitDrawTheme(void)
{
	if (bInitDrawTheme) return;

	hmodUXTHEME = LoadLibrary("uxtheme.dll");
	if (hmodUXTHEME != NULL)
	{
		pDrawThemeParentBackground = (pfnDrawThemeParentBackground)
			GetProcAddress(hmodUXTHEME, "DrawThemeParentBackground");
		if (pDrawThemeParentBackground == NULL)
		{
			FreeLibrary(hmodUXTHEME); hmodUXTHEME = NULL;
		}

		pOpenThemeData = (pfnOpenThemeData)
			GetProcAddress(hmodUXTHEME, "OpenThemeData");
		if (pOpenThemeData == NULL)
		{
			FreeLibrary(hmodUXTHEME); hmodUXTHEME = NULL;
		}

		pCloseThemeData = (pfnCloseThemeData)
			GetProcAddress(hmodUXTHEME, "CloseThemeData");
		if (pCloseThemeData == NULL)
		{
			FreeLibrary(hmodUXTHEME); hmodUXTHEME = NULL;
		}

		pDrawThemeBackground = (pfnDrawThemeBackground)
			GetProcAddress(hmodUXTHEME, "DrawThemeBackground");
		if (pDrawThemeBackground == NULL)
		{
			FreeLibrary(hmodUXTHEME); hmodUXTHEME = NULL;
		}

		pGetThemeColor = (pfnGetThemeColor)
			GetProcAddress(hmodUXTHEME, "GetThemeColor");
		if (pGetThemeColor == NULL)
		{
			FreeLibrary(hmodUXTHEME); hmodUXTHEME = NULL;
		}

	}
	bInitDrawTheme = TRUE;
}

//backuped 20200918
//void InitDrawTheme(void)
//{
//	if(bInitDrawTheme) return;
//
//	hmodUXTHEME = LoadLibrary("uxtheme.dll");
//	if(hmodUXTHEME != NULL)
//	{
//		pDrawThemeParentBackground = (pfnDrawThemeParentBackground)
//			GetProcAddress(hmodUXTHEME, "DrawThemeParentBackground");
//		if(pDrawThemeParentBackground == NULL)
//		{
//			FreeLibrary(hmodUXTHEME); hmodUXTHEME = NULL;
//		}
//	}
//	bInitDrawTheme = TRUE;
//}

void EndNewAPI(HWND hwndClock)
{
	if(hmodUXTHEME != NULL) FreeLibrary(hmodUXTHEME);
	hmodUXTHEME = NULL; pDrawThemeParentBackground = NULL;

	if(hmodMSIMG32 != NULL) FreeLibrary(hmodMSIMG32);
	hmodMSIMG32 = NULL; pGradientFill = NULL; pAlphaBlend = NULL; pTransparentBlt = NULL;

	if(pSetLayeredWindowAttributes)
	{
		HWND hwnd;
		LONG_PTR exstyle;

		hwnd = GetParent(GetParent(hwndClock));
		exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
		if(exstyle & WS_EX_LAYERED)
		{
			exstyle &= ~WS_EX_LAYERED;
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle);
			RefreshRebar(hwnd);
		}

	
	}

	if(hmodUSER32 != NULL) FreeLibrary(hmodUSER32);
	hmodUSER32 = NULL;
	pSetLayeredWindowAttributes = NULL;
}




void GradientFillBack(HDC hdcTarget, int width, int height, COLORREF col1, COLORREF col2, DWORD grad)
{
	TRIVERTEX vert[2];
	GRADIENT_RECT gRect;
	RECT tempRect;

	tempRect.left = 0;
	tempRect.right = width;
	tempRect.top = 0;
	tempRect.bottom = height;

	if (!pGradientFill) InitGradientFill();
	if (!pGradientFill) return;

	vert[0].x = tempRect.left;
	vert[0].y = tempRect.top;
	vert[0].Red = (COLOR16)GetRValue(col1) * 256;
	vert[0].Green = (COLOR16)GetGValue(col1) * 256;
	vert[0].Blue = (COLOR16)GetBValue(col1) * 256;
	vert[0].Alpha = 0x0000;
	vert[1].x = tempRect.right;
	vert[1].y = tempRect.bottom;
	vert[1].Red = (COLOR16)GetRValue(col2) * 256;
	vert[1].Green = (COLOR16)GetGValue(col2) * 256;
	vert[1].Blue = (COLOR16)GetBValue(col2) * 256;
	vert[1].Alpha = 0x0000;
	gRect.UpperLeft = 0;
	gRect.LowerRight = 1;
	pGradientFill(hdcTarget, vert, 2, &gRect, 1, grad);
}



void SetLayeredTaskbar(HWND hwndClock)
{
	LONG_PTR exstyle;
	HWND hwnd;
	int alpha;

	alpha = 0;
	alpha = 255 - (alpha * 255 / 100);
	if(alpha != 0 && alpha < 8) alpha = 8; else if(alpha > 255) alpha = 255;



	if(!pSetLayeredWindowAttributes && alpha < 255) InitLayeredWindow();
	if(!pSetLayeredWindowAttributes) return;

	hwnd = GetParent(GetParent(hwndClock));

	exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	if(alpha < 255) exstyle |= WS_EX_LAYERED;
	else exstyle &= ~WS_EX_LAYERED;
	SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle);

	RefreshRebar(hwnd);

	if(alpha == 0)
		pSetLayeredWindowAttributes(hwnd, GetSysColor(COLOR_3DFACE), 0, LWA_COLORKEY);
	else if(alpha < 255)
		pSetLayeredWindowAttributes(hwnd, 0, (BYTE)alpha, LWA_ALPHA);

}


void SetLayeredWindow(HWND hwnd, INT alphaTip, COLORREF colBack)
{
	LONG_PTR exstyle;

	if(!pSetLayeredWindowAttributes) InitLayeredWindow();
	if(!pSetLayeredWindowAttributes) return;

	exstyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
	if(!(exstyle & WS_EX_LAYERED))
	{
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, exstyle|WS_EX_LAYERED);
	}
	if(alphaTip == 0)
		pSetLayeredWindowAttributes(hwnd, colBack, 0, LWA_COLORKEY);
	else
		pSetLayeredWindowAttributes(hwnd, 0, (BYTE)alphaTip, LWA_ALPHA);
}

//void SetLayeredStartMenu(HWND hwnd)
//{
//	SetLayeredWindow(hwnd, nAlphaStartMenu, GetSysColor(COLOR_3DFACE));
//}

HRESULT MyAlphaBlend(
  HDC hdcDest,                 // handle to destination DC
  int nXOriginDest,            // x-coord of upper-left corner
  int nYOriginDest,            // y-coord of upper-left corner
  int nWidthDest,              // destination width
  int nHeightDest,             // destination height
  HDC hdcSrc,                  // handle to source DC
  int nXOriginSrc,             // x-coord of upper-left corner
  int nYOriginSrc,             // y-coord of upper-left corner
  int nWidthSrc,               // source width
  int nHeightSrc,              // source height
  BLENDFUNCTION blendFunction  // alpha-blending function
)
{
	if(!pAlphaBlend) InitAlphaBlend();
	if(!pAlphaBlend) return S_FALSE;

	return pAlphaBlend(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
                       hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, blendFunction);
}

HRESULT MyTransparentBlt(
  HDC hdcDest,        // handle to destination DC
  int nXOriginDest,   // x-coord of destination upper-left corner
  int nYOriginDest,   // y-coord of destination upper-left corner
  int nWidthDest,     // width of destination rectangle
  int nHeightDest,    // height of destination rectangle
  HDC hdcSrc,         // handle to source DC
  int nXOriginSrc,    // x-coord of source upper-left corner
  int nYOriginSrc,    // y-coord of source upper-left corner
  int nWidthSrc,      // width of source rectangle
  int nHeightSrc,     // height of source rectangle
  UINT crTransparent  // color to make transparent
)
{
	if(!pTransparentBlt) InitTransparentBlt();
	if(!pTransparentBlt) return S_FALSE;

	return pTransparentBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
                       hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, crTransparent);
}

HRESULT MyDrawThemeParentBackground(HWND hwnd,HDC hdc, RECT* prc)
{
	if(!pDrawThemeParentBackground) InitDrawTheme();
	if(!pDrawThemeParentBackground) return S_FALSE;


	if (!hClockTheme) {
			hClockTheme = pOpenThemeData(hwnd, VSCLASS_TASKBAND2);
	}


//	return pDrawThemeParentBackground(hwnd, hdc, prc);
	pDrawThemeParentBackground(hwnd, hdc, prc);
	return pDrawThemeBackground(hClockTheme, hdc, 5, CLS_NORMAL, prc, NULL);

}



HRESULT GetThemeColor_BG_Win10(HWND hwnd)
{
	if (!pDrawThemeParentBackground) InitDrawTheme();
	if (!pDrawThemeParentBackground) return S_FALSE;

	if (!hClockTheme) {
		hClockTheme = pOpenThemeData(hwnd, VSCLASS_TASKBAND2);
	}

	COLORREF ret;
	pGetThemeColor(hClockTheme, 5, CLS_NORMAL, TMT_BACKGROUND, &ret);
	return ret;

}


/*--------------------------------------------------
    redraw ReBarWindow32 forcely
----------------------------------------------------*/
void RefreshRebar(HWND hwndBar)
{
	HWND hwnd;
	char classname[80];

	hwnd = GetWindow(hwndBar, GW_CHILD);
	while(hwnd)
	{
		GetClassName(hwnd, classname, 80);
		if(lstrcmpi(classname, "ReBarWindow32") == 0)
		{
			InvalidateRect(hwnd, NULL, TRUE);
			hwnd = GetWindow(hwnd, GW_CHILD);
			while(hwnd)
			{
				InvalidateRect(hwnd, NULL, TRUE);
				hwnd = GetWindow(hwnd, GW_HWNDNEXT);
			}
			break;
		}
		hwnd = GetWindow(hwnd, GW_HWNDNEXT);
	}
}

