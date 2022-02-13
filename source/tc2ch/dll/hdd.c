/*-------------------------------------------------------------------------
  hdd.c
  HDD ‚Ì‹ó‚«—e—Ê‚ðŽæ“¾‚·‚é
---------------------------------------------------------------------------*/

#include <windows.h>
#include <iprtrmib.h>

void Hdd_get(void);
void GetDiskSpace(int nDrive, ULONGLONG *nFree, ULONGLONG *nAll);
void GetDiskSpaceFromPath(char *szDrive, ULONGLONG *nFree, ULONGLONG *nAll);
double diskFree[72];
double diskAll[72];

extern int actdvl[];

void Hdd_get(void)
{
	int i;
	extern char strAdditionalMountPath;

	for(i=0;i<26;i++)
	{
		diskFree[i] = 0;
		diskFree[i + 36] = 0;
		diskAll[i] = 0;
		diskAll[i + 36] = 0;

		if(actdvl[i] == 1)
		{
			ULONGLONG freeDisk = 0, allDisk = 0;
			GetDiskSpace(i,&freeDisk, &allDisk);
			diskFree[i]    = (double)(LONGLONG)(freeDisk/1048576);
			diskFree[i+36] = diskFree[i]/1024;
			diskAll[i]     = (double)(LONGLONG)(allDisk/1048576);
			diskAll[i+36]  = diskAll[i]/1024;
		}
	}

	for (i = 0; i<10; i++)
	{
		diskFree[i + 26] = 0;
		diskFree[i + 62] = 0;
		diskAll[i + 26] = 0;
		diskAll[i + 62] = 0;
		if (actdvl[26+i] == 1)
		{
			ULONGLONG freeDisk = 0, allDisk = 0;
			GetDiskSpaceFromPath(&strAdditionalMountPath + 64 * i, &freeDisk, &allDisk);
			diskFree[i + 26] = (double)(LONGLONG)(freeDisk / 1048576);
			diskFree[i + 62] = diskFree[i + 26] / 1024;
			diskAll[i + 26] = (double)(LONGLONG)(allDisk / 1048576);
			diskAll[i + 62] = diskAll[i + 26] / 1024;
		}
	}


}

void GetDiskSpace(int nDrive, ULONGLONG *nFree, ULONGLONG *nAll)
{
	char    szDrive[32];
	ULARGE_INTEGER useByte, allByte, freeByte;

	wsprintf(szDrive, "%c:\\", nDrive + 'A');
	if (GetDiskFreeSpaceEx(szDrive, &useByte, &allByte, &freeByte) != 0)
	{
		*nFree = freeByte.QuadPart;
		*nAll = allByte.QuadPart;
	}
		//	return(allByte.QuadPart);

	return;
}


void GetDiskSpaceFromPath(char *szDrive, ULONGLONG *nFree, ULONGLONG *nAll)
{
	ULARGE_INTEGER useByte, allByte, freeByte;


	if (GetDiskFreeSpaceEx(szDrive, &useByte, &allByte, &freeByte) != 0)
	{
		*nFree = freeByte.QuadPart;
		*nAll = allByte.QuadPart;
	};

//	return(allByte.QuadPart);
	return;
}
