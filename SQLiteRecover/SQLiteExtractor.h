
#include "stdafx.h"
#include "CppSQLite3U.h"
#include "SQLiteDTO.h"
#include <iostream>
#include <vector>
#include <string>

// enum SQLITIECOPYERR { SOURCE_DB_ERROR, DESTINATION_DB_ERROR, QUERY_ERROR, ANALYZE_SUCCESS, FORENSIC_DB_ERROR, SELECTQUERY_ERROR, PLIST_ERROR };

class SQLiteExtractor
{
private :
	
	/*
		File information
	*/
	CppSQLite3DB		m_srcDB;			// Specify the SQLite database File
	CString				m_strFilePath;		// SQLite database file path	
	unsigned __int64	m_nFileSize;		// SQLite database file size


	///*
	//	Page information
	//*/
	//unsigned __int64	m_nPageStartOffset;	// Current page start offset
	//unsigned __int64	m_nPageEndOffset;	// Current page end offset
	//
	///*
	//	Page Header information
	//*/
	//

	unsigned __int64	m_nCurrentOffset;	// Current offset
	
public :
	SQLiteExtractor();
	SQLiteExtractor( CString strFilePath ); 

	~SQLiteExtractor();

	int		Init();					// initialization 
	BOOL	Open();					// SQLite database file open	: Compare signature, Check page size
	std::vector<SQLITE_TABLE> SchemaTblParsing();		// Schema table parsing			: Send query ("SELECT * FROM sqlite_master;") -> Get the information of table count, root page, page field ...
	
	BOOL Close();
};