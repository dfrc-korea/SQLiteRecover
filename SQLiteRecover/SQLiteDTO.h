#include "stdafx.h"
#include <vector>
#ifndef __SQLITEDTO_H__
#define __SQLITEDTO_H__


#define USES_CONVERSION int _convert = 0; (_convert); UINT _acp = ATL::_AtlGetConversionACP() /*CP_THREAD_ACP*/; (_acp); LPCWSTR _lpw = NULL; (_lpw); LPCSTR _lpa = NULL; (_lpa)

#define		INTERNAL_PAGE						0x05
#define		LEAF_PAGE							0x0D
#define		INTERNAL_PAGE_HEADER_SIZE			0x0C
#define		LEAF_PAGE_HEADER_SIZE				0x08

#define		NUMBER_OF_CELL_OFFSET				0x03
#define		NUMBER_OF_CELL_OFFSET_SIZE			2


#define		SCHEMA_PAGE_START					0x64
#define		NUMBER_OF_SQLITE_MASTER_SCHEMA		5
#define		NUMBER_OF_CELL_OFFSET				0x03
#define		NUMBER_OF_CELL_OFFSET_SIZE			2


struct SQLITE_COLUMN{
	CString Type;
	CString c_id;
	CString	ColumnName;
	INT ColumnLength;
};

struct COLUMN
{
	INT Length;
	INT CLength;
	BYTE* Data;

	BOOL isDamaged;
};
struct FreeBlock{
	INT ColumnInfoStartOffset;
	INT DataAreaStartOffset;
	COLUMN* columnInfo;
};
struct Rows{
	INT PageNumber;
	__int64 RecordOffset;
	std::vector<COLUMN> columns;
	
};


struct SQLitePage{
	INT PageNumber;
	BYTE* Page;
};

struct SQLITE_TABLE{

	CString TableName;
	INT RootPageNum;
	
	std::vector<INT> LeafPageNum;			//leaf page
	std::vector<INT> InternalNum;

	INT NumberOfField;
	SQLITE_COLUMN* Column;

	std::vector<Rows> DeletedRecord;
	//171024
	std::vector<Rows> NormalRecord;
};


#endif