#include "stdafx.h"
#include <vector>
#include "UTIL.h"
#ifndef __JANG_H__
#define __JANG_H__


#define LEAF		0x0D
#define INTERNAL	0x05
#define OVERFLOW	0x00


struct SplitSQLiteContainer{

	INT		PAGE_NUMBER;
	BYTE	PageFlag;
	BYTE	NextFreeSpace[2];
	BYTE	NumberOfRecord[2];
	BYTE	OffsetOfTheFirstBytesOfRecord[2];
	BYTE	NumberOfFragmentedFreeByte;
	BYTE	ChildInternalPage[4];

	BYTE*	PAGE;

};


struct SQLiteOverFlowPage
{
	BYTE	PageFlag;
	BYTE	NextFreeSpace[2];
	BYTE	NumberOfRecord[2];
	BYTE	OffsetOfTheFirstBytesOfRecord[2];
	BYTE	NumberOfFragmentedFreeByte;
	BYTE	ChildInternalPage[4];

//	BYTE	PAGE[PageSize * 2];			//일단 2개인데('주식' 키워드가 있는것이) 만약을 대비해서 나중에 바꿔야함.
};



/*
	지금 해야될거는 : 
			1. Spit된애들중에 overflow난애들 찾아야하고,
			2. Split된애들 복구안되는애들 찾아서 모두 복구 모듈만들어줘야하고
			3. 일단 오버플로우부터 복구
*/

class SQLReader
{
private:

	BYTE* SQLBUFFER;
	int file_size;


	std::vector<SplitSQLiteContainer>	splitpage;


	//임시
	SQLiteOverFlowPage overflow;
	INT CurrentOffset;
	UTIL ut;


	INT PageSize;

public:
	SQLReader(TCHAR* pszfilename);
	SQLReader(TCHAR* pszfilename, BOOL OverFlow);
	INT GetSplitSQLiteByPageSize();


	VOID SetPageSize(INT _PageSize);

	/*여기서댐*/
	VOID Run();

	

	/*이거*/
	BOOL ExtractStringData(SplitSQLiteContainer Page);
	BOOL SaveStringToCSV(SplitSQLiteContainer Page, INT END);


	
	
	BOOL Run(BOOL);

	/*안슴*/
	INT GetRecordSize(INT OFFSET_TO_SIZE_AREA); // 이건 헤더에있잖니.. 그냥 헤더써는아닌가? 일단내둬야지
	/*안슴*/
	BYTE GetPageFlag(INT START_PAGE);//임시
	BOOL SetOverFlowSQLite();
	

	/*			if Deleted Cell Exists			*/		

};

#endif