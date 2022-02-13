/*-----------------------------------------------------
　BMP.C
　　ビットマップの読み込み
　　KAZUBON 1997
-------------------------------------------------------*/

#include "tcdll.h"

// 参考：Charls Petzold 「プログラミング Windows3.1」

/*--------------------------------------------------
　ビットマップのパレットの数を得る
----------------------------------------------------*/
int GetDibNumColors(LPBITMAPINFOHEADER pbmih)
{
	int numColors;
	int BitCount;

	BitCount = (int)pbmih->biBitCount;
	numColors = (int)pbmih->biClrUsed;
	if(numColors == 0)
	{
		if(BitCount <= 8) numColors = 1 << BitCount;
		else numColors = 0;
	}
	return numColors;
}

/*--------------------------------------------------
　ビットマップビットのアドレスを得る
----------------------------------------------------*/
BYTE* GetDibBitsAddr(BYTE* pDib)
{
	int numColors;

	numColors = GetDibNumColors((LPBITMAPINFOHEADER)pDib);
	return pDib + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * numColors;
}

/*--------------------------------------------------
　ビットマップの読み込み
　LoadImageと同じことを自力で行う
----------------------------------------------------*/
HBITMAP ReadBitmap(HWND hwnd, char* fname, BOOL b)
{
	BITMAPFILEHEADER bmfh;
	BYTE* pDib;
	DWORD size;
	HFILE hf;
	BITMAPINFOHEADER* pbmih;
	BYTE* pDIBits;
	HDC hdc;
	BYTE index = 0;
	HBITMAP hBmp;

	hf = _lopen(fname, OF_READ);
	if(hf == HFILE_ERROR) return NULL;

	size = _llseek(hf, 0, 2) - sizeof(BITMAPFILEHEADER);
	_llseek(hf, 0, 0);

	if(_lread(hf, (LPSTR)&bmfh, sizeof(BITMAPFILEHEADER)) !=
		sizeof(BITMAPFILEHEADER))
	{
		_lclose(hf); return NULL;
	}

	if(bmfh.bfType != *(WORD *)"BM")
	{
		_lclose(hf); return NULL;
	}

	pDib = GlobalAllocPtr(GMEM_MOVEABLE,size);
	if(pDib == NULL)
	{
		_lclose (hf); return NULL;
	}

	if(_hread(hf, pDib, size) != (long)size)
	{
		_lclose(hf); GlobalFree(pDib);
		return NULL;
	}
	_lclose(hf);

	pbmih = (BITMAPINFOHEADER*)pDib;
	// OS/2形式には対応しない
	if(pbmih->biSize != sizeof(BITMAPINFOHEADER))
	{
		GlobalFree(pDib); return NULL;
	}
	// RLE圧縮には対応しない
	if(pbmih->biCompression != BI_RGB &&
		pbmih->biCompression != BI_BITFIELDS)
	{
		GlobalFree(pDib); return NULL;
	}

	if(pbmih->biCompression == BI_RGB)
		pDIBits = GetDibBitsAddr(pDib);
	else
		pDIBits = pDib + sizeof(BITMAPINFOHEADER) + 3 * sizeof(DWORD);

	if(b)
	{
		// 最初のドットのパレットを、ボタンの表面の色と同じにする
		if(pbmih->biBitCount == 1)
			index = (*pDIBits & 0x80) >> 7;
		else if(pbmih->biBitCount == 4)
			index = (*pDIBits & 0xF0) >> 4;
		else if(pbmih->biBitCount == 8)
			index = *pDIBits;
		if(pbmih->biBitCount <= 8)
		{
			COLORREF col = GetSysColor(COLOR_3DFACE);
			((BITMAPINFO*)pDib)->bmiColors[index].rgbBlue = GetBValue(col);
			((BITMAPINFO*)pDib)->bmiColors[index].rgbGreen = GetGValue(col);
			((BITMAPINFO*)pDib)->bmiColors[index].rgbRed = GetRValue(col);
		}
	}

	hdc = GetDC(hwnd);
	hBmp = CreateDIBitmap(hdc,
		(LPBITMAPINFOHEADER)pDib, CBM_INIT,
		(LPSTR)pDIBits, (LPBITMAPINFO)pDib, DIB_RGB_COLORS);
	ReleaseDC(hwnd, hdc);
	GlobalFree(pDib);
	return hBmp;
}
