#include "stdafx.h"
#include "SQLiteCreator.h"



SQLiteCreator::SQLiteCreator(){

}

SQLiteCreator::~SQLiteCreator(){

}


VOID SQLiteCreator::SetOutputPath(CString output){
	outputfilepath = output;
}
BOOL SQLiteCreator::CreateDatabase(CString database){

	CFile file;
	CFile dFile;

	CString Ddatabase = database;
	Ddatabase = Ddatabase.Left(Ddatabase.Find(L".db"));
	Ddatabase += "_Damaged.db";
	if (!file.Open(database, CFile::modeCreate)){
		return FALSE;
	}
// 	if (!dFile.Open(Ddatabase, CFile::modeCreate)){
// 		return FALSE;
// 	}

	
	file.Close();
	//dFile.Close();
	BOOL isOpen1 = Open(database);
	//BOOL isOpen2 = Open(Ddatabase);
	return isOpen1;
}

BOOL SQLiteCreator::Open(CString database){


	if (!m_srcDB.Open_Optimization(database)){
		return FALSE;
	}
	return TRUE;

}

BOOL SQLiteCreator::GetSchemaInformation_DB(CString input)
{

	CString Field = input;

	CString temp;
	std::vector<SQLITE_COLUMN> parse;
	SQLITE_COLUMN write;

	Field.Replace(L"\r\n", L"");
	Field.Replace(L"\n", L"");
	Field.Replace(L"\r", L"");
	Field.Replace(L"  ", L"");
	Field.Replace(L"\t", L"");
	Field = Field.Right(Field.GetLength() - Field.Find(L"(") - 1);
	while (Field.GetLength() != 0/*temp.Find(ex, i) < 0 && cnt < TotalCnt*/)
	{

		if (Field.Find(L",") == -1){

			temp = Field;
			Field = L"";

		}
		else{

			temp = Field.Left(Field.Find(L","));
			Field = Field.Right(Field.GetLength() - Field.Find(L",") - 1);
		}

		int pos = 0;
		//////////////////////////////
		if (temp.Find(_T("INTEGER"))!=-1)
		{
			write.Type = L"INTEGER";
			pos = temp.Find(_T("INTEGER"));
		}
		else if (temp.Find(_T("TEXT"))!=-1)
		{
			write.Type = L"TEXT";
			pos = temp.Find(_T("TEXT"));
		}
		else if (temp.Find(_T("BLOB"))!=-1)
		{
			write.Type = L"BLOB";
			pos = temp.Find(_T("BLOB"));
		}
		write.ColumnName = temp.Left(pos);	
/*			
			((pos = temp.Find(_T("INTEGER"))) || (pos = temp.Find(_T("TEXT"))) || (pos = temp.Find(_T("BLOB"))))
		{
			write.ColumnName = temp.Left(pos);
			CString type_key = temp.Mid(pos);
			if (type_key.Find(L" "))
				write.Type = type_key.Left(type_key.Find(L" "));
			else
				write.Type = type_key;
		}

//		while (temp.GetAt(0) == ' '){
//			temp = temp.Right(temp.GetLength() - 1);
//		}
*/

		write.ColumnName.Replace(L" ", L"");
//		write.Type.Replace(L" ", L"");


//		AfxExtractSubString(write.ColumnName, temp, 0, ' ');
//		AfxExtractSubString(write.Type, temp, 1, ' ');

		parse.push_back(write);

	}
	create_columns = parse;

	return TRUE;
}


BOOL SQLiteCreator::CreateTableDB(SQLITE_TABLE sqlite_master){

/*
	for (int i = 0; i < sqlite_master.DeletedRecord.size(); i++){

		CString data(sqlite_master.DeletedRecord[i].columns[0].Data);
		CString tblName(sqlite_master.DeletedRecord[i].columns[1].Data);
		if (data.Find(L"table") != -1 && tblName.Find(L"sqlite_sequence") == -1)
		{
			CString Query(sqlite_master.DeletedRecord[i].columns[4].Data);
			m_srcDB.execQuery(Query);
		}

	}
	*/
	CString Query;

	for (int i=0; i<sqlite_master.DeletedRecord.size(); i++)
	{
		CString data(sqlite_master.DeletedRecord[i].columns[0].Data);
		CString tblName(sqlite_master.DeletedRecord[i].columns[1].Data);
		if (data.Find(L"table") != -1 && tblName.Find(L"sqlite_sequence") == -1 && tblName.Find(L"words") == -1)
		{

			Query.Format(_T("Create table %s ("), tblName);

			CString createQuery(sqlite_master.DeletedRecord[i].columns[4].Data);
			GetSchemaInformation_DB(createQuery);

			int j;
			for (j=0; j<create_columns.size(); j++){
				Query.AppendFormat(_T("%s %s"), create_columns[j].ColumnName, create_columns[j].Type);
				if (j != (create_columns.size() -1))
					Query.AppendFormat(_T(", "));
			}
			Query.AppendFormat(_T(")"));
			m_srcDB.execQuery(Query);

		}
		create_columns.clear();
	}
//		for (int j=0; j<sqlite_master.DeletedRecord
	return TRUE;
}

BOOL SQLiteCreator::CreateTable(SQLITE_TABLE table){
	CString Query;

	Query.Format(_T("Create table %s ( RecordOffset INT,"), table.TableName);

	for (int i = 0; i < table.NumberOfField; i++){
		
		Query.AppendFormat(_T("%s %s"), table.Column[i].ColumnName, table.Column[i].Type);

		if (i != table.NumberOfField - 1)
			Query.AppendFormat(_T(","));
	}

	Query.AppendFormat(_T(")"));

	m_srcDB.execQuery(Query);

	return TRUE;
}
BOOL SQLiteCreator::CreateTableDamaged(SQLITE_TABLE table){
	CString Query;

	Query.Format(_T("Create table %s_damaged ( RecordOffset INT,"), table.TableName);

	for (int i = 0; i < table.NumberOfField; i++){

		Query.AppendFormat(_T("%s %s"), table.Column[i].ColumnName, table.Column[i].Type);

		if (i != table.NumberOfField - 1)
			Query.AppendFormat(_T(","));
	}

	Query.AppendFormat(_T(")"));

	m_srcDB.execQuery(Query);

	return TRUE;
}

BOOL SQLiteCreator::InsertRecoveredRecordDamaged(SQLITE_TABLE table){


	CString Query;


	wprintf(L"# Insert deleted record in %s_damaged table.\n", table.TableName);
	for (int i = 0; i < table.DeletedRecord.size(); i++){
		BOOL isDamaged = FALSE;
		Query.Format(_T("Insert into %s_damaged values ("), table.TableName);
		for (int k = 0; k < table.DeletedRecord[i].columns.size(); k++){
			if (k == 0){
				int number = table.DeletedRecord[i].RecordOffset;
				Query.AppendFormat(L"%I64d,", table.DeletedRecord[i].RecordOffset);
				//printf("# Offset %d.\n", table.DeletedRecord[i].RecordOffset);
			}

			if (table.DeletedRecord[i].columns[k].isDamaged)
			{
				isDamaged = TRUE;
			//	; break;
			}
			if (table.DeletedRecord[i].columns[k].Length == 0){
				Query.AppendFormat(L"NULL");
			}
			else{
				if (table.Column[k].Type == "INTEGER"){
					__int64 Value = ut.LongIntToByte(table.DeletedRecord[i].columns[k].Data, table.DeletedRecord[i].columns[k].Length);
					Query.AppendFormat(_T("%I64d"), Value);
				}
				else
				{
					CString data;

					CString parm_src_string/* =szMultiByte*/;
					int temp_length = MultiByteToWideChar(CP_UTF8, 0, (char*)table.DeletedRecord[i].columns[k].Data/*parm_src_string*/, -1, NULL, 0);

					if (temp_length > 0)
					{
						// 변환된 유니코드를 저장할 공간을 할당한다.
						BSTR unicode_str = SysAllocStringLen(NULL, temp_length);

						// 유니 코드로 변환한다.
						MultiByteToWideChar(CP_UTF8, 0, (char*)table.DeletedRecord[i].columns[k].Data/*parm_src_string*/, -1, unicode_str, temp_length);
						parm_src_string = unicode_str;

						// 유니코드 형식의 문자열을 저장하기 위해 생성했던 메모리를 삭제한다.
						data = parm_src_string;
						SysFreeString(unicode_str);
					}
					else
					{
						parm_src_string.Empty();
					}



					//	data.Replace(_T());
					data.Replace(_T(","), _T(""));
					data.Replace(_T("'"), _T(""));
					data.Replace(_T("\n"), _T(""));
					data.Replace(_T("\""), _T(""));
					data.Replace(_T("\r\n"), _T(""));
					Query.AppendFormat(_T("'%s'"), data);
				}
			}

			if (k != table.NumberOfField - 1){
				Query.AppendFormat(_T(","));
			}

		}

		Query.AppendFormat(_T(")"));

		if (isDamaged){
			try
			{
				m_srcDB.execQuery(Query);
			}
			catch (CMemoryException* e)
			{

			}
			catch (CFileException* e)
			{
			}
			catch (CException* e)
			{
			}
		}


	}



	return TRUE;
}

BOOL SQLiteCreator::InsertRecoveredRecord(SQLITE_TABLE table){


	CString Query;
	
	
	wprintf(L"# Insert deleted record in %s table.\n", table.TableName);
	for (int i = 0; i < table.DeletedRecord.size(); i++){
		BOOL isDamaged = FALSE;
		Query.Format(_T("Insert into %s values ("), table.TableName);
		for (int k = 0; k < table.DeletedRecord[i].columns.size(); k++){
			if (k == 0){
				int number = table.DeletedRecord[i].RecordOffset;
				Query.AppendFormat(L"%I64d,", table.DeletedRecord[i].RecordOffset);
				//printf("# Offset %d.\n", table.DeletedRecord[i].RecordOffset);
			}

			if (table.DeletedRecord[i].columns[k].isDamaged)
			{
				isDamaged = TRUE;
				break;
			}
			if (table.DeletedRecord[i].columns[k].Length == 0){
				Query.AppendFormat(L"NULL");
			}
			else{
				if (table.Column[k].Type == "INTEGER"){
					__int64 Value = ut.LongIntToByte(table.DeletedRecord[i].columns[k].Data, table.DeletedRecord[i].columns[k].Length);
					Query.AppendFormat(_T("%I64d"), Value);
				}
				else
				{
					CString data;

					CString parm_src_string/* =szMultiByte*/;
					int temp_length = MultiByteToWideChar(CP_UTF8, 0, (char*)table.DeletedRecord[i].columns[k].Data/*parm_src_string*/, -1, NULL, 0);

					if (temp_length > 0)
					{
						// 변환된 유니코드를 저장할 공간을 할당한다.
						BSTR unicode_str = SysAllocStringLen(NULL, temp_length);

						// 유니 코드로 변환한다.
						MultiByteToWideChar(CP_UTF8, 0, (char*)table.DeletedRecord[i].columns[k].Data/*parm_src_string*/, -1, unicode_str, temp_length);
						parm_src_string = unicode_str;

						// 유니코드 형식의 문자열을 저장하기 위해 생성했던 메모리를 삭제한다.
						data = parm_src_string;
						SysFreeString(unicode_str);
					}
					else
					{
						parm_src_string.Empty();
					}



					//	data.Replace(_T());
					data.Replace(_T(","), _T(""));
					data.Replace(_T("'"), _T(""));
					data.Replace(_T("\n"), _T(""));
					data.Replace(_T("\""), _T(""));
					data.Replace(_T("\r\n"), _T(""));
					Query.AppendFormat(_T("'%s'"), data);
				}
			}

			if (k != table.NumberOfField - 1){
				Query.AppendFormat(_T(","));
			}

		}
		
		Query.AppendFormat(_T(")"));
		
		if (!isDamaged){
			try
			{
				m_srcDB.execQuery(Query);
			}
			catch (CMemoryException* e)
			{

			}
			catch (CFileException* e)
			{
			}
			catch (CException* e)
			{
			}
		}
		
		
	}
	

	
	return TRUE;
}

BOOL SQLiteCreator::CreateTableCSV(SQLITE_TABLE table){
	USHORT nUniSig = 0xfeff;	// 유니코드 문자를 저장할때 hj
	csv.Write(&nUniSig, 2);
	csv.SeekToEnd();


	CString Query;
	Query.Format(_T("\"PageOffset\""));
	Query.AppendFormat(_T("\t\"rowid\""));
	//0131 
	Query.AppendFormat(_T("\t\"isDeleted\""));

	//Query.Format(_T("Create table %s ("), table.TableName);
	//csv.WriteString(Query.GetBuffer());
//	csv.Write(Query.GetBuffer(), Query.GetLength() * 2);;
	for (int i = 0; i < table.NumberOfField; i++){

		CString data = table.Column[i].ColumnName;
		Query += _T("\t\"") + data + _T("\"");

	}

	Query.AppendFormat(_T("\n"));
	csv.Write(Query.GetBuffer(), Query.GetLength() * 2);;

	return TRUE;
}




BOOL SQLiteCreator::InsertRecoveredRecordCSV(SQLITE_TABLE table){

	csv.SeekToEnd();
	CString Query;
	

	for (int k = 0; k < table.DeletedRecord.size(); k++){
		//Query.Format(_T("\"PageOffset\""));
		for (int a = 0; a < table.DeletedRecord[k].columns.size(); a++){

//			if(k==821 && a==5)
//				printf("k = %d, a = %d\n", k, a);
			//대처용 테스트
			//if(k==0x51a && a==0xa)
				//printf("error here\n");
			//
			if (a == 0){
			//	Query.Format(_T("\"%I64d\""), table.DeletedRecord[k].RecordOffset);
				Query.Format(_T("\"PageOffset\""));
				Query.AppendFormat(_T("\t\"NULL\""));

				// isDeleted
				Query.AppendFormat(_T("\t\"TRUE\""));
			}
			if (table.DeletedRecord[k].columns[a].Length == 0){
				Query.AppendFormat(_T("\t\"NULL\""));
			}
			else{
				if (table.Column[a].Type == "INTEGER"){
	
// 					WCHAR* temp = new WCHAR[table.DeletedRecord[k].columns[a].Length + 1];
// 					wsprintf(temp, L"%S", (CHAR*)table.DeletedRecord[k].columns[a].Data);
					

					//int nStrLength = table.DeletedRecord[k].columns[a].Length + 1;
					CHAR temp[256];
					memcpy(temp, table.DeletedRecord[k].columns[a].Data, table.DeletedRecord[k].columns[a].Length + 1);
					temp[table.DeletedRecord[k].columns[a].Length] = NULL;
					
					CString integer(temp);
					int nStrLength = integer.GetLength() + 1;
					LPTSTR lpsz = new TCHAR[nStrLength];
					_tcscpy_s(lpsz, nStrLength, integer);
					int col = nStrLength;
					int len = table.DeletedRecord[k].columns[a].Length + 1;
// 					USES_CONVERSION;
// 					__int64 nValue;
// 					try{
// 						
// 						nValue = _atoi64(temp);
// 					}
// 					catch (std::bad_exception &e) {
// 						nValue = 0;
// 					}					//delete[] temp;

					__int64 Value = ut.LongIntToByte(table.DeletedRecord[k].columns[a].Data, table.DeletedRecord[k].columns[a].Length);
					Query.AppendFormat(_T("\t\"%I64d\""), Value);
				//	delete[] lpsz;
				}
				else if (table.Column[a].Type == "TEXT")
				{
					//CString data(ut.UTFtoANSI((char*)table.DeletedRecord[k].columns[a].Data));
					CString data;

					CString parm_src_string/* =szMultiByte*/;
					int temp_length = MultiByteToWideChar(CP_UTF8, 0, (char*)table.DeletedRecord[k].columns[a].Data/*parm_src_string*/, -1, NULL, 0);

					if (temp_length > 0)
					{
						// 변환된 유니코드를 저장할 공간을 할당한다.
						BSTR unicode_str = SysAllocStringLen(NULL, temp_length);

						// 유니 코드로 변환한다.
						MultiByteToWideChar(CP_UTF8, 0, (char*)table.DeletedRecord[k].columns[a].Data/*parm_src_string*/, -1, unicode_str, temp_length);
						parm_src_string = unicode_str;

						// 유니코드 형식의 문자열을 저장하기 위해 생성했던 메모리를 삭제한다.
						data = parm_src_string;
						SysFreeString(unicode_str);
					}
					else
					{
						parm_src_string.Empty();
					}



				//	data.Replace(_T());
				//	data.Replace(_T(","), _T(""));
					data.Replace(_T("\n"), _T(""));
					data.Replace(_T("\""), _T(""));
					data.Replace(_T("\r\n"), _T(""));
					Query.AppendFormat(_T("\t\"%s\""), data);

				}
				
			}
		}
		Query.AppendFormat(_T("\n"));
		csv.Write(Query.GetBuffer(), Query.GetLength() * 2);

	}

	return TRUE;

}

BOOL SQLiteCreator::InsertNormalRecordCSV(SQLITE_TABLE table){

	csv.SeekToEnd();
	CString Query;
	

	for (int k = 0; k < table.NormalRecord.size(); k++){
		//Query.Format(_T("\"PageOffset\""));
		for (int a = 0; a < table.NormalRecord[k].columns.size(); a++){

//			if(k==821 && a==5)
//				printf("k = %d, a = %d\n", k, a);
			//대처용 테스트
			//if(k==0x51a && a==0xa)
				//printf("error here\n");
			//
			if (a == 0){
			//	Query.Format(_T("\"%I64d\""), table.DeletedRecord[k].RecordOffset);
				Query.Format(_T("\"PageOffset\""));
				Query.AppendFormat(_T("\t\"NULL\""));

				// isDeleted
				Query.AppendFormat(_T("\t\"FALSE\""));
			}
			if (table.NormalRecord[k].columns[a].Length == 0){
				Query.AppendFormat(_T("\t\"NULL\""));
			}
			else{
				if (table.Column[a].Type == "INTEGER"){
	
// 					WCHAR* temp = new WCHAR[table.DeletedRecord[k].columns[a].Length + 1];
// 					wsprintf(temp, L"%S", (CHAR*)table.DeletedRecord[k].columns[a].Data);
					

					//int nStrLength = table.DeletedRecord[k].columns[a].Length + 1;
					CHAR temp[256];
					memcpy(temp, table.NormalRecord[k].columns[a].Data, table.NormalRecord[k].columns[a].Length + 1);
					temp[table.NormalRecord[k].columns[a].Length] = NULL;
					
					CString integer(temp);
					int nStrLength = integer.GetLength() + 1;
					LPTSTR lpsz = new TCHAR[nStrLength];
					_tcscpy_s(lpsz, nStrLength, integer);
					int col = nStrLength;
					int len = table.NormalRecord[k].columns[a].Length + 1;
// 					USES_CONVERSION;
// 					__int64 nValue;
// 					try{
// 						
// 						nValue = _atoi64(temp);
// 					}
// 					catch (std::bad_exception &e) {
// 						nValue = 0;
// 					}					//delete[] temp;

					__int64 Value = ut.LongIntToByte(table.NormalRecord[k].columns[a].Data, table.NormalRecord[k].columns[a].Length);
					Query.AppendFormat(_T("\t\"%I64d\""), Value);
				//	delete[] lpsz;
				}
				else if (table.Column[a].Type == "TEXT")
				{
					//CString data(ut.UTFtoANSI((char*)table.DeletedRecord[k].columns[a].Data));
					CString data;

					CString parm_src_string/* =szMultiByte*/;
					int temp_length = MultiByteToWideChar(CP_UTF8, 0, (char*)table.NormalRecord[k].columns[a].Data/*parm_src_string*/, -1, NULL, 0);

					if (temp_length > 0)
					{
						// 변환된 유니코드를 저장할 공간을 할당한다.
						BSTR unicode_str = SysAllocStringLen(NULL, temp_length);

						// 유니 코드로 변환한다.
						MultiByteToWideChar(CP_UTF8, 0, (char*)table.NormalRecord[k].columns[a].Data/*parm_src_string*/, -1, unicode_str, temp_length);
						parm_src_string = unicode_str;

						// 유니코드 형식의 문자열을 저장하기 위해 생성했던 메모리를 삭제한다.
						data = parm_src_string;
						SysFreeString(unicode_str);
					}
					else
					{
						parm_src_string.Empty();
					}



				//	data.Replace(_T());
				//	data.Replace(_T(","), _T(""));
					data.Replace(_T("\n"), _T(""));
					data.Replace(_T("\""), _T(""));
					data.Replace(_T("\r\n"), _T(""));
					Query.AppendFormat(_T("\t\"%s\""), data);

				}
				
			}
		}
		Query.AppendFormat(_T("\n"));
		csv.Write(Query.GetBuffer(), Query.GetLength() * 2);

	}

	return TRUE;

}


//171026 
BOOL SQLiteCreator::InsertRecoveredRecordDB(SQLITE_TABLE table){

	CString Query;
	int zero = 0;
//	Query.Format(_T("Create table %s ( RecordOffset INT,"), table.TableName);


	for (int k = 0; k < table.DeletedRecord.size(); k++){
		//Query.Format(_T("\"PageOffset\""));

		//insert 시작
		for (int a = 0; a < table.DeletedRecord[k].columns.size(); a++){

//			if(k==821 && a==5)
//				printf("k = %d, a = %d\n", k, a);
			//대처용 테스트
			//if(k==0x51a && a==0xa)
				//printf("error here\n");
			//
			if (a == 0){
				Query.Format(_T("INSERT INTO %s values ("), table.TableName);
			//	Query.Format(_T("\"%I64d\""), table.DeletedRecord[k].RecordOffset);
			//	Query.Format(_T("\"PageOffset\""));
			//	Query.AppendFormat(_T("\t\"NULL\""));
			}
			if (table.Column[a].Type == "INTEGER"){
	
				if (table.DeletedRecord[k].columns[a].Length == 0){
					if (table.TableName != "schema_migrations")
						Query.AppendFormat(_T("NULL"));
					else
						
						Query.AppendFormat(_T("\"%d\""), zero);
				}

// 					WCHAR* temp = new WCHAR[table.DeletedRecord[k].columns[a].Length + 1];
// 					wsprintf(temp, L"%S", (CHAR*)table.DeletedRecord[k].columns[a].Data);
					

				//int nStrLength = table.DeletedRecord[k].columns[a].Length + 1;
				else
				{
					CHAR temp[256];
					memcpy(temp, table.DeletedRecord[k].columns[a].Data, table.DeletedRecord[k].columns[a].Length + 1);
					temp[table.DeletedRecord[k].columns[a].Length] = NULL;
					
					CString integer(temp);
					int nStrLength = integer.GetLength() + 1;
					LPTSTR lpsz = new TCHAR[nStrLength];
					_tcscpy_s(lpsz, nStrLength, integer);
					int col = nStrLength;
					int len = table.DeletedRecord[k].columns[a].Length + 1;
	// 					USES_CONVERSION;
	// 					__int64 nValue;
	// 					try{
	// 						
	// 						nValue = _atoi64(temp);
	// 					}
	// 					catch (std::bad_exception &e) {
	// 						nValue = 0;
	// 					}					//delete[] temp;

					__int64 Value = ut.LongIntToByte(table.DeletedRecord[k].columns[a].Data, table.DeletedRecord[k].columns[a].Length);
					Query.AppendFormat(_T("\"%I64d\""), Value);
				}
			//	delete[] lpsz;
			}
			else if (table.Column[a].Type == "TEXT")
			{
				if (table.DeletedRecord[k].columns[a].Length == 0){
					Query.AppendFormat(_T("\"NULL\""));
				}
				else
				{

					//CString data(ut.UTFtoANSI((char*)table.DeletedRecord[k].columns[a].Data));
					CString data;

					CString parm_src_string/* =szMultiByte*/;
					int temp_length = MultiByteToWideChar(CP_UTF8, 0, (char*)table.DeletedRecord[k].columns[a].Data/*parm_src_string*/, -1, NULL, 0);

					if (temp_length > 0)
					{
						// 변환된 유니코드를 저장할 공간을 할당한다.
						BSTR unicode_str = SysAllocStringLen(NULL, temp_length);

						// 유니 코드로 변환한다.
						MultiByteToWideChar(CP_UTF8, 0, (char*)table.DeletedRecord[k].columns[a].Data/*parm_src_string*/, -1, unicode_str, temp_length);
						parm_src_string = unicode_str;

						// 유니코드 형식의 문자열을 저장하기 위해 생성했던 메모리를 삭제한다.
						data = parm_src_string;
						SysFreeString(unicode_str);
					}
					else
					{
						parm_src_string.Empty();
					}



				//	data.Replace(_T());
				//	data.Replace(_T(","), _T(""));
					data.Replace(_T("\n"), _T(""));
					data.Replace(_T("\""), _T(""));
					data.Replace(_T("\r\n"), _T(""));
					Query.AppendFormat(_T("\"%s\""), data);
				}
			}
				
			if (a != table.DeletedRecord[k].columns.size() -1)
			{
				Query.AppendFormat(_T(", "));
			}
		}
		Query.AppendFormat(_T(")"));
		m_srcDB.execQuery(Query);
		//csv.Write(Query.GetBuffer(), Query.GetLength() * 2);

	}

	return TRUE;

}


BOOL SQLiteCreator::CloseFile(SQLITE_TABLE table){
	csv.Close();
	return TRUE;
}

BOOL SQLiteCreator::OpenFile(SQLITE_TABLE table){

	
	CString filename = outputfilepath + L"\\" + table.TableName;
	filename += ".csv";
	if (!csv.Open(filename, CFile::typeBinary | CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate))
	{
	
		return TRUE;
	}
	
	return FALSE;
}

BOOL SQLiteCreator::isFileCreated(SQLITE_TABLE table){

	CString filename = outputfilepath + L"\\" + table.TableName;
	filename += ".csv";
/*
	CFileFind pFind;

	BOOL bRet = pFind.FindFile(filename);*/
	CFileStatus FileOn;

	if (CFile::GetStatus(filename, FileOn))
	{
		return TRUE;
	}
	return FALSE;
}