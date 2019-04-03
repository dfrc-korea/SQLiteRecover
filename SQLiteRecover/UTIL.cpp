#include "stdafx.h"
#include "UTIL.h"


unsigned __int64 UTIL::IntToByte(BYTE src[], int length){
	unsigned __int64 result = 0;
	int shift = (length - 1) * 8;
	for (int i = 0; i < length; i++){
		result += ((src[i] & 0xFF) << shift);
		shift -= 8;
	}

	return result;
}

unsigned __int64 UTIL::LongIntToByte(BYTE src[], int length){
	__int64 result = 0x00;
	__int64 temp = 0x00;
	__int64 shift = (length - 1) * 8;
	for (int i = 0; i < length; i++){
		temp = src[i];
		temp = temp & 0xFF;
		temp = temp << shift;
		//result += ((src[i] & 0xFF) << shift);
		//result = result << shift;
		result += temp;
		shift -= 8;
	}

	return result;
}


int UTIL::LittleEndianToInteger(BYTE src[]){

	int COUNT1 = src[0] & 0xFF;
	int COUNT2 = src[1] & 0xFF;

	return (COUNT2 << 8) + COUNT1;
}
int UTIL::LittleEndianToInteger(BYTE src[], INT length){

	int result = 0;
	int shift = (length - 1) * 8;
	for (int i = length - 1; i >=  0; i--){
		result += ((src[i] & 0xFF) << shift);
		shift -= 8;
	}
	return result;
}

CString UTIL::convertUTF8toANSI(CString str)
{
	BSTR    bstrWide;
	CHAR*   pszAnsi;
	INT     nLength, i;
	CString result;
	INT		size = str.GetLength();

	CHAR* pszCode = new CHAR[size + 1];

	for (i = 0; i < size; i++)
		pszCode[i] = str[i];
	pszCode[i] = 0x00;

	nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, size, NULL, NULL);
	bstrWide = SysAllocStringLen(NULL, nLength);

	MultiByteToWideChar(CP_UTF8, 0, pszCode, size, bstrWide, nLength);

	nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);
	pszAnsi = new CHAR[nLength];

	WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);
	SysFreeString(bstrWide);

	result = pszAnsi;

	delete[] pszCode;
	delete[] pszAnsi;

	return result;
}

CString UTIL::unixTimeToStrTime(INT32 unixTime)
{
	INT32 fftime;
	CTime ct;
	CString YYMM;

	//시스템의 GMT시간 설정 불러오기
	TIME_ZONE_INFORMATION timezone;
	GetTimeZoneInformation(&timezone);
	int gmt = timezone.Bias / 60;
	gmt = (0xffffffff ^ gmt) + 1;

	//유니코드이므로 _tstoi()를 사용한다.
	fftime = unixTime;

	//CTime은 시스템의 GMT 시간을 적용하기 때문에 그 시간 만끔 빼줘야 한다. -gmt*60*60;
	//fftime = fftime - gmt*60*60;
	ct = CTime(fftime);

	YYMM.Format(TEXT("%0.4d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d"), ct.GetYear(), ct.GetMonth(), ct.GetDay(), ct.GetHour(), ct.GetMinute(), ct.GetSecond());
	return YYMM;
} // unixTimeToStrTime()

char* UTIL::UTF8toANSI(char *pszCode)
{
	BSTR    bstrWide;
	char*   pszAnsi;
	int     nLength;
	// Get nLength of the Wide Char buffer
	nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, NULL, NULL);
	bstrWide = SysAllocStringLen(NULL, nLength);
	// Change UTF-8 to Unicode (UTF-16)
	MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, bstrWide, nLength);
	// Get nLength of the multi byte buffer<
	nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);
	pszAnsi = new char[nLength];
	// Change from unicode to mult byte>
	WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);
	SysFreeString(bstrWide);
	return pszAnsi;
}

CString UTIL::UTFtoANSI(char *pszCode)
{
	BSTR    bstrWide;
	char*   pszAnsi;
	int     nLength;
	// Get nLength of the Wide Char buffer
	nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, NULL, NULL);
	bstrWide = SysAllocStringLen(NULL, nLength);
	// Change UTF-8 to Unicode (UTF-16)
	MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, bstrWide, nLength);
	// Get nLength of the multi byte buffer<
	
	nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);
	pszAnsi = new char[nLength];
	// Change from unicode to mult byte>
	WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);
	SysFreeString(bstrWide);
	CString result(pszAnsi);
	delete[] pszAnsi;
	return result;
}