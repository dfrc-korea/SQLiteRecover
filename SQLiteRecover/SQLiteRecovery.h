#include "stdafx.h"
#include "SQLiteExtractor.h"
#include "SQLiteDTO.h"
#include "UTIL.h"
#include "SQLiteCreator.h"
#include "UTF8Checker.h"
#include "SQLiteJournalRecover.h"
#include <algorithm>
#include "SQLiteWALRecover.h"
#ifndef __SQLITERECOVERY_H__
#define __SQLITERECOVERY_H__


class SQLiteRecovery{




private:
	UTIL ut;
	BYTE* SQLITE_BUFFER;
	UTF8Checker utfchecker;
	INT PageSize;
	INT FileSize;
	SQLiteExtractor sqlite;
	std::vector<SQLITE_TABLE> tables;

	std::vector<SQLITE_TABLE> deleted_table;  /* TEXT	*/	

	INT GetColumnStartOffset(BYTE* Page, INT StartOffset, INT EndOffset, SQLITE_TABLE table);
	INT GetColumnStartOffsetReverse(BYTE* Page, INT StartOffset, INT EndOffset, SQLITE_TABLE table);

	INT GetAvailableByteByBitPattern(BYTE* Page, INT OffsetOfRecord, INT Length);
	INT GetLengthOfBitPattern(BYTE Bit[], INT BitLength);

	
	INT GetLeafPage(INT PAGE_START, SQLITE_TABLE* table);
	INT GetLeafPage(SQLitePage* Page, INT PAGE_START, SQLITE_TABLE* table);
	VOID GetDeletedRecord(SQLitePage Page, SQLITE_TABLE* table);
	
	INT CarvingTableInfo(BYTE* Page, INT StartOffset, SQLITE_TABLE* table);
	INT CarvingRecord(SQLitePage Page, INT StartOffset, INT EndOffset, SQLITE_TABLE* table);
	INT CarvingRecordReverse(SQLitePage Page, INT StartOffset, INT EndOffset, FreeBlock* FreeBlockData, SQLITE_TABLE* table);
	SQLiteCreator CSV;

	//171024
	INT ParsingRecord(SQLitePage Page, INT StartOffset, INT EndOffset, SQLITE_TABLE* table);

private:

	VOID FreeBlockAreaCarving(SQLitePage Page, SQLITE_TABLE* table);
	VOID FreeBlockAreaCarvingReverse(SQLitePage Page, SQLITE_TABLE* table);
	VOID FreeSpaceAreaCarving(SQLitePage Page, SQLITE_TABLE* table);

	VOID FreeSpaceAreaCarvingReverse(SQLitePage Page, SQLITE_TABLE* table);
	BOOL CheckingFreeBlockHeaderInFreeSpace(BYTE* CheckingBYTE);
	BOOL isOverwriteCellHeader(BYTE* Page);
	BOOL CheckingFreeBlockSizeArea(FreeBlock data, SQLITE_TABLE* table,INT StartOffset, INT EndOffset);
public:

	VOID SetCSVPath(CString Path);
	CString OutputPath;

	//SQLiteRecovery(TCHAR* pszfilename);
	SQLiteRecovery(TCHAR* pszfilename, TCHAR* outputName);
	BOOL GetTableInformation();
	
	VOID CollectLeafPage();
	VOID RecoverSQLite();
	
	INT InitTableInfo();
	SQLiteWALRecover WAL;
	VOID RecoverWAL(TCHAR* pszPath);


	VOID RecoverJournal(TCHAR* pszPath);
	VOID GetCellFromJournal(SQLitePage Page, SQLITE_TABLE* table);
	SQLITE_TABLE GetCellFromJournal(SQLitePage Page);

	VOID NormalSQLite(); //정상 레코드 출력 171024
private:
	// 스키마정보 찾는곳.
	INT GetIndexOfTableListByWALPageNumber(INT PageNumber);


private:
	// 페이지단위 복구
	std::vector<SQLitePage>	pages;
	VOID GetPageListFromSQLite();
	INT GetSchemaFromNormalRecord(SQLitePage Page);
	INT GetSchemaFromPageNumber(INT PageNumber);
	INT GetSchemaFromByteScan(SQLitePage Page);
public:
	VOID PageTravelRecovery();
	INT GetPageSize();


private:
	/*
		
		스키마 가져오기
		
	*/
	SQLITE_TABLE	sqlite_master;
	VOID InitSQLiteMasterTable();
	VOID GetNormalCell(SQLitePage Page, SQLITE_TABLE* table);
	VOID GetSchemaCell(SQLitePage Page, SQLITE_TABLE* table);
	VOID GetSchemaCell_First(SQLitePage Page, SQLITE_TABLE* table); //171024
	VOID SetSchemaInformationFromSQLITEMaster();
	VOID GetTableSchema();
	std::vector<SQLITE_COLUMN> GetSchemaInformation(CString input);



private:
	/*
		오버플로우
	*/
	INT ParsingRecordContainOverflow(SQLitePage Page, INT NextPage, INT StartOffset, INT EndOffset, INT RecordSize/* 전체 셀 사이즈*/, SQLITE_TABLE* table, BOOL overflow_e/*대처*/);
	INT CarvingRecordContainOverflow(SQLitePage Page, INT NextPage, INT StartOffset, INT EndOffset, INT RecordSize/* 전체 셀 사이즈*/, SQLITE_TABLE* table, BOOL overflow_e/*대처*/);
	INT FindOverflowPage(SQLitePage Page, INT ColumnNumber, INT CurrentLength, INT RecordSize, Rows* row, SQLITE_TABLE* table);
	INT CheckingRecordSize(INT Length);


	VOID FreeSpaceAreaCarving(SQLitePage Page, SQLITE_TABLE* table, std::map<INT,BOOL> *celloffset);
	VOID FreeBlockAreaCarving(SQLitePage Page, SQLITE_TABLE* table, std::map<INT, BOOL> *celloffset);
};



#endif