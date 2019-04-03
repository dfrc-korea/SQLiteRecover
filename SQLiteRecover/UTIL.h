#include "stdafx.h"
#ifndef __UTIL_H__
#define __UTIL_H__


class UTIL
{
public:
	unsigned __int64 IntToByte(BYTE src[], int length);
	unsigned __int64 LongIntToByte(BYTE src[], int length);
	int LittleEndianToInteger(BYTE src[]);
	int LittleEndianToInteger(BYTE src[], INT length);
	CString convertUTF8toANSI(CString str);
	CString unixTimeToStrTime(INT32 unixtime);
	char* UTF8toANSI(char *pszCode);

	CString UTFtoANSI(char * pszCode);
};

#endif