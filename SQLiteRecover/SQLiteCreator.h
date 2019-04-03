#include "stdafx.h"
#include "CppSQLite3U.h"
#include "SQLiteDTO.h"
#include "UTIL.h"

#ifndef __SQLITECREATOR_H__
#define __SQLITECREATOR_H__


class SQLiteCreator
{
private:
	UTIL ut;
private:
	/*
	File information
	*/
	CppSQLite3DB		m_srcDB;			// Specify the SQLite database File
	CppSQLite3DB		m_srcDB_Damaged;


	CString				m_strFilePath;		// SQLite database file path	
	unsigned __int64	m_nFileSize;		// SQLite database file size

	unsigned __int64	m_nCurrentOffset;	// Current offset
	CString outputfilepath;

	std::vector<SQLITE_COLUMN> create_columns;

public:

	SQLiteCreator();


	~SQLiteCreator();

	BOOL isFileCreated(SQLITE_TABLE table);

	BOOL CreateDatabase(CString database);
	BOOL Open(CString database);
	BOOL CreateTable(SQLITE_TABLE table);
	BOOL CreateTableDamaged(SQLITE_TABLE table);
	BOOL InsertRecoveredRecord(SQLITE_TABLE table);
	BOOL InsertRecoveredRecordDamaged(SQLITE_TABLE table);

	VOID SetOutputPath(CString output);

	BOOL InsertNormalRecordCSV(SQLITE_TABLE table);

private:
	CStdioFile csv;
public:

	BOOL OpenFile(SQLITE_TABLE table);
	BOOL CreateTableCSV(SQLITE_TABLE table);
	BOOL InsertRecoveredRecordCSV(SQLITE_TABLE table);
	BOOL CloseFile(SQLITE_TABLE table);

	BOOL CreateTableDB(SQLITE_TABLE sqlite_master); //171026수정
	BOOL InsertRecoveredRecordDB(SQLITE_TABLE table); //171026
	BOOL GetSchemaInformation_DB(CString input); //171026 수정


private:
	
// public:
// 	BOOL CreateDatabase(CString databaseName);
// 	BOOL CreateTable(SQLITE_TABLE table);
// 	BOOL InsertData(SQLITE_TABLE table);
// 	BOOL CloseDatabase(CString databaseName);
};

#endif