#include "stdafx.h"
#include "SQLiteExtractor.h"


SQLiteExtractor::SQLiteExtractor()
{
}

SQLiteExtractor::SQLiteExtractor( CString strFilePath ) 
{
	m_strFilePath = strFilePath;
}

SQLiteExtractor::~SQLiteExtractor()
{
}

int SQLiteExtractor::Init() // 초기화 함수
{
	return false;
}

BOOL SQLiteExtractor::Open() 
{
	if( !m_srcDB.Open_Optimization( m_strFilePath ) )
	{
		return false;
	}

	return true;
}

std::vector<SQLITE_TABLE> SQLiteExtractor::SchemaTblParsing()
{
	std::vector<SQLITE_TABLE> tables;
	CppSQLite3Table ResultTable;
	CString Query = _T("");

	UINT64 rowCount = 0;

	Query.Format(_T("SELECT tbl_name, rootpage FROM sqlite_master WHERE type='table'"));
	ResultTable = m_srcDB.getTable(Query.GetBuffer());

	rowCount = ResultTable.GetRowCount();

	if (rowCount != 0){

		for (int i = 0; i < rowCount; i++){
			ResultTable.setRow(i);
			SQLITE_TABLE table;
			table.TableName = ResultTable.GetWCharFieldValue(0);
			table.RootPageNum = _wtoi64(ResultTable.GetWCharFieldValue(1));

			UINT64 FieldrowCount = 0;
			CppSQLite3Table FieldResultTable;
			Query.Format(_T("PRAGMA table_info(`%s`)"), ResultTable.GetWCharFieldValue(0));
			FieldResultTable = m_srcDB.getTable((CStringA)Query);

			if (FieldResultTable.mnRows == 0)
			{
				table.NumberOfField = 0;
				tables.push_back(table);
				continue;							//No Data
			}
			else{
				FieldrowCount = FieldResultTable.GetRowCount();
				table.NumberOfField = FieldrowCount;
				table.Column = new SQLITE_COLUMN[FieldrowCount];
				for (int k = 0; k < FieldrowCount; k++){
					FieldResultTable.setRow(k);
					table.Column[k].c_id = FieldResultTable.GetWCharFieldValue(0);
					table.Column[k].ColumnName = FieldResultTable.GetWCharFieldValue(1);
					table.Column[k].Type = FieldResultTable.GetWCharFieldValue(2);
					table.Column[k].Type.MakeUpper();
					if (table.Column[k].Type.Find(_T("INT")) != -1 || table.Column[k].Type.Find(_T("DECIMAL")) != -1
						|| table.Column[k].Type.Find(_T("REAL")) != -1 || table.Column[k].Type.Find(_T("DOUBLE")) != -1 ||
						table.Column[k].Type.Find(_T("NUMERIC")) != -1 || (table.Column[k].Type.Find(_T("DATE")) != -1 && table.Column[k].Type.Compare(_T("DATETIME")) != 0) ||
						table.Column[k].Type.Find(_T("BOOL")) != -1 || table.Column[k].Type.Find(_T("LONG")) != -1)
					{
						table.Column[k].Type = _T("INTEGER");
					}
					else if (table.Column[k].Type.Find(_T("TEXT")) != -1 || table.Column[k].Type.Find(_T("CHAR")) != -1 ||
						table.Column[k].Type.Find(_T("STRING")) != -1 || table.Column[k].Type.Find(_T("DATETIME")) != -1 || table.Column[k].Type.Find(_T("NVARCHAR")) != -1)
					{
						table.Column[k].Type = _T("TEXT");
					}
					else if (table.Column[k].Type.Find(_T("BLOB")) != -1 || table.Column[k].Type.Find(_T("CLOB")) != -1)
					{
						table.Column[k].Type = _T("BLOB");
					}

				}		
			}
			tables.push_back(table);
		}
	}

	return tables;
}

BOOL SQLiteExtractor::Close(){
	m_srcDB.close();
	return TRUE;
}