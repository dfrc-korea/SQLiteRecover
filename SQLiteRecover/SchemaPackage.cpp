#include "stdafx.h"
#include "SQLiteRecovery.h"


VOID SQLiteRecovery::InitSQLiteMasterTable(){

	sqlite_master.Column = new SQLITE_COLUMN[NUMBER_OF_SQLITE_MASTER_SCHEMA];

	sqlite_master.Column[0].Type = _T("TEXT");
	sqlite_master.Column[0].ColumnName = L"type";
	sqlite_master.Column[1].Type = _T("TEXT");
	sqlite_master.Column[1].ColumnName = L"name";
	sqlite_master.Column[2].Type = _T("TEXT");
	sqlite_master.Column[2].ColumnName = L"tbl_name";
	sqlite_master.Column[3].Type = _T("INTEGER");
	sqlite_master.Column[3].ColumnName = L"rootpage";
	sqlite_master.Column[4].Type = _T("TEXT");
	sqlite_master.Column[4].ColumnName = L"sql";
	sqlite_master.RootPageNum = 1;




	sqlite_master.TableName = L"sqlite_master";
	sqlite_master.NumberOfField = 5;

}

VOID SQLiteRecovery::GetNormalCell(SQLitePage Page, SQLITE_TABLE* table){

	BYTE Cell[2];
	memcpy(Cell, &Page.Page[NUMBER_OF_CELL_OFFSET], NUMBER_OF_CELL_OFFSET_SIZE);
	INT CellCNT = ut.LongIntToByte(Cell, NUMBER_OF_CELL_OFFSET_SIZE);

	std::vector<INT>	celloffset;
	std::vector<INT>	deleted_cell;

	INT StartOffset = LEAF_PAGE_HEADER_SIZE;

	for (int i = 0; i < CellCNT; i++){

		memcpy(Cell, &Page.Page[StartOffset], NUMBER_OF_CELL_OFFSET_SIZE);
		celloffset.push_back(ut.IntToByte(Cell, 2));
		StartOffset += 2;

	}

//	if(Page.PageNumber==0x9b)
//		printf("error page here\n");

	memcpy(Cell, &Page.Page[1], 2);
	CellCNT = ut.LongIntToByte(Cell, NUMBER_OF_CELL_OFFSET_SIZE);

	while (CellCNT != 0){

		celloffset.push_back(ut.IntToByte(Cell, 2));
		deleted_cell.push_back(ut.IntToByte(Cell, 2));

		memcpy(Cell, &Page.Page[CellCNT], NUMBER_OF_CELL_OFFSET_SIZE);
		CellCNT = ut.LongIntToByte(Cell, NUMBER_OF_CELL_OFFSET_SIZE);

	}


//	printf("%d , error page number\n", Page.PageNumber);

	sort(celloffset.begin(), celloffset.end());

	/*
	1. 정상레코드는 셀과 셀 사이 간격을 구해
	2. 레코드 총 길이보다 셀과 셀 사이 간격이 더 작을경우 오버플로우
	3. 뒤에 셀오프셋에서 4바이트땅겨서 그거 오버플로우 페이지 판별.
	*/

	for (int i = 0; i < celloffset.size(); i++){


		StartOffset = celloffset[i];
		INT EndOffset = StartOffset;
		INT RecordToRecord = 0;
		INT CellSize = 0;
		INT HeaderSize = 0;
		BOOL isDeleted = FALSE;

		if (i == celloffset.size() - 1)
			RecordToRecord = PageSize - celloffset[i] + 2;
		else
			RecordToRecord = celloffset[i + 1] - celloffset[i];


		for (int k = 0; k < deleted_cell.size(); k++){

			if (celloffset[i] == deleted_cell[k])
			{
				isDeleted = TRUE;
			}

		}

		if (isDeleted)  //deleterecord
		{
			BYTE FreeBlockSize[3];
			memcpy(FreeBlockSize, &Page.Page[StartOffset + 2], 2);

//			INT EndOffset = StartOffset + ut.IntToByte(FreeBlockSize,2);
			CellSize = ut.IntToByte(FreeBlockSize,2);
			INT EndOffset = StartOffset + CellSize;
			StartOffset += 4;

			StartOffset = GetColumnStartOffset(Page.Page, StartOffset, EndOffset, *table);

			if (StartOffset == -1)
				continue;

			BOOL overflow_e = FALSE; 

			if (CellSize > RecordToRecord)
			{
				BYTE PageNumber[4];
				memcpy(PageNumber, &Page.Page[StartOffset + RecordToRecord - 4], 4);
				INT OverflowPageNumberStart = ut.LongIntToByte(PageNumber, 4);

				if(OverflowPageNumberStart > FileSize / PageSize)
				{
					OverflowPageNumberStart = OverflowPageNumberStart >> 16;
					overflow_e = TRUE;

				}
				CarvingRecordContainOverflow(Page,
					OverflowPageNumberStart,
					StartOffset,
					RecordToRecord - 4, // 레코드 크기.
					CellSize, // 전체 셀 사이즈로 하자.
					table,
					overflow_e); //overflow 페이지 읽어올때 2byte 초과함

			}
			else
				CarvingRecord(Page, StartOffset, EndOffset, table);
		}
		//0131	continue;

		else // 0131 normal record
		{
			for (int k = 0; k < 3; k++){

				INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, StartOffset, 1);
				//		HeaderSize += SizeOfLengthField;

				if (k == 0){
					BYTE* SizeOfRecord = new BYTE[SizeOfLengthField + 1];
					memcpy(SizeOfRecord, &Page.Page[StartOffset], SizeOfLengthField);

					CellSize = GetLengthOfBitPattern(SizeOfRecord, SizeOfLengthField);
					EndOffset += CellSize;
					EndOffset += SizeOfLengthField;

					delete[] SizeOfRecord;
				}
				HeaderSize += SizeOfLengthField;
				StartOffset += SizeOfLengthField;
			}

			BOOL overflow_e = FALSE; //대처

			if (CellSize > RecordToRecord)		// overflow
			{
				RecordToRecord -= HeaderSize;
			
				//대처용
	
				BYTE PageNumber[4];
				memcpy(PageNumber, &Page.Page[StartOffset + RecordToRecord - 4], 4);
				INT OverflowPageNumberStart = ut.LongIntToByte(PageNumber, 4);

				if(OverflowPageNumberStart > FileSize / PageSize)
				{
					OverflowPageNumberStart = OverflowPageNumberStart >> 16;
					overflow_e = TRUE;

				}
				ParsingRecordContainOverflow(Page,
					OverflowPageNumberStart,
					StartOffset,
					RecordToRecord - 4, // 레코드 크기.
					CellSize, // 전체 셀 사이즈로 하자.
					table,
					overflow_e); //overflow 페이지 읽어올때 2byte 초과함
			}

			else
				ParsingRecord(Page, StartOffset, EndOffset, table);
		}
	}
	FreeSpaceAreaCarving(Page, table);

}

VOID SQLiteRecovery::GetSchemaCell(SQLitePage Page, SQLITE_TABLE* table){

	BYTE Cell[2];
	memcpy(Cell, &Page.Page[NUMBER_OF_CELL_OFFSET], NUMBER_OF_CELL_OFFSET_SIZE);
	INT CellCNT = ut.LongIntToByte(Cell, NUMBER_OF_CELL_OFFSET_SIZE);

	std::vector<INT>	celloffset;
	std::vector<INT>	deleted_cell;

	INT StartOffset = LEAF_PAGE_HEADER_SIZE;

	for (int i = 0; i < CellCNT; i++){

		memcpy(Cell, &Page.Page[StartOffset], NUMBER_OF_CELL_OFFSET_SIZE);
		celloffset.push_back(ut.IntToByte(Cell, 2));
		StartOffset += 2;

	}

//	if(Page.PageNumber==0x9b)
//		printf("error page here\n");

	memcpy(Cell, &Page.Page[1], 2);
	CellCNT = ut.LongIntToByte(Cell, NUMBER_OF_CELL_OFFSET_SIZE);

	while (CellCNT != 0){

		celloffset.push_back(ut.IntToByte(Cell, 2));
		deleted_cell.push_back(ut.IntToByte(Cell, 2));

		memcpy(Cell, &Page.Page[CellCNT], NUMBER_OF_CELL_OFFSET_SIZE);
		CellCNT = ut.LongIntToByte(Cell, NUMBER_OF_CELL_OFFSET_SIZE);

	}


//	printf("%d , error page number\n", Page.PageNumber);

	sort(celloffset.begin(), celloffset.end());

	/*
	1. 정상레코드는 셀과 셀 사이 간격을 구해
	2. 레코드 총 길이보다 셀과 셀 사이 간격이 더 작을경우 오버플로우
	3. 뒤에 셀오프셋에서 4바이트땅겨서 그거 오버플로우 페이지 판별.
	*/

	for (int i = 0; i < celloffset.size(); i++){


		StartOffset = celloffset[i];
		INT EndOffset = StartOffset;
		INT RecordToRecord = 0;
		INT CellSize = 0;
		INT HeaderSize = 0;
		BOOL isDeleted = FALSE;

		if (i == celloffset.size() - 1)
			RecordToRecord = PageSize - celloffset[i] + 2;
		else
			RecordToRecord = celloffset[i + 1] - celloffset[i];


		for (int k = 0; k < deleted_cell.size(); k++){

			if (celloffset[i] == deleted_cell[k])
			{
				isDeleted = TRUE;
			}

		}

		if (isDeleted)  //delete 된 스키마는 고려 x ? 
			continue;

		for (int k = 0; k < 3; k++){

			INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, StartOffset, 1);
			//		HeaderSize += SizeOfLengthField;

			if (k == 0){
				BYTE* SizeOfRecord = new BYTE[SizeOfLengthField + 1];
				memcpy(SizeOfRecord, &Page.Page[StartOffset], SizeOfLengthField);

				CellSize = GetLengthOfBitPattern(SizeOfRecord, SizeOfLengthField);
				EndOffset += CellSize;
				EndOffset += SizeOfLengthField;

				delete[] SizeOfRecord;
			}
			HeaderSize += SizeOfLengthField;
			StartOffset += SizeOfLengthField;
		}

		BOOL overflow_e = FALSE; //대처

		if (CellSize > RecordToRecord)		// overflow
		{
			RecordToRecord -= HeaderSize;
			
			//대처용
	
			BYTE PageNumber[4];
			memcpy(PageNumber, &Page.Page[StartOffset + RecordToRecord - 4], 4);
			INT OverflowPageNumberStart = ut.LongIntToByte(PageNumber, 4);

			if(OverflowPageNumberStart > FileSize / PageSize)
			{
				OverflowPageNumberStart = OverflowPageNumberStart >> 16;
				overflow_e = TRUE;

			}
			ParsingRecordContainOverflow(Page,
				OverflowPageNumberStart,
				StartOffset,
				RecordToRecord - 4, // 레코드 크기.
				CellSize, // 전체 셀 사이즈로 하자.
				table,
				overflow_e); //overflow 페이지 읽어올때 2byte 초과함
		}

		else
			ParsingRecord(Page, StartOffset, EndOffset, table);

	}
	FreeSpaceAreaCarving(Page, table);

}


//171024
VOID SQLiteRecovery::GetSchemaCell_First(SQLitePage Page, SQLITE_TABLE* table){


	BYTE Cell[2];
	memcpy(Cell, &Page.Page[NUMBER_OF_CELL_OFFSET+0x64], NUMBER_OF_CELL_OFFSET_SIZE);
	INT CellCNT = ut.LongIntToByte(Cell, NUMBER_OF_CELL_OFFSET_SIZE);

	std::vector<INT>	celloffset;
	std::vector<INT>	deleted_cell;

	INT StartOffset = LEAF_PAGE_HEADER_SIZE + 0x64;

	for (int i = 0; i < CellCNT; i++){

		memcpy(Cell, &Page.Page[StartOffset], NUMBER_OF_CELL_OFFSET_SIZE);
		celloffset.push_back(ut.IntToByte(Cell, 2));
		StartOffset += 2;

	}

	memcpy(Cell, &Page.Page[0x65], 2);
	CellCNT = ut.LongIntToByte(Cell, NUMBER_OF_CELL_OFFSET_SIZE);

	while (CellCNT != 0){

		celloffset.push_back(ut.IntToByte(Cell, 2));
		deleted_cell.push_back(ut.IntToByte(Cell, 2));

		memcpy(Cell, &Page.Page[CellCNT], NUMBER_OF_CELL_OFFSET_SIZE);
		CellCNT = ut.LongIntToByte(Cell, NUMBER_OF_CELL_OFFSET_SIZE);

	}


	sort(celloffset.begin(), celloffset.end());

	/*
	1. 정상레코드는 셀과 셀 사이 간격을 구해
	2. 레코드 총 길이보다 셀과 셀 사이 간격이 더 작을경우 오버플로우
	3. 뒤에 셀오프셋에서 4바이트땅겨서 그거 오버플로우 페이지 판별.
	*/

	for (int i = 0; i < celloffset.size(); i++){


		StartOffset = celloffset[i];
		INT EndOffset = StartOffset;
		INT RecordToRecord = 0;
		INT CellSize = 0;
		INT HeaderSize = 0;
		BOOL isDeleted = FALSE;

		if (i == celloffset.size() - 1)
			RecordToRecord = PageSize - celloffset[i] + 2;
		else
			RecordToRecord = celloffset[i + 1] - celloffset[i];


		for (int k = 0; k < deleted_cell.size(); k++){

			if (celloffset[i] == deleted_cell[k])
			{
				isDeleted = TRUE;
			}

		}


		if (isDeleted)
			continue;


		for (int k = 0; k < 3; k++){

			INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, StartOffset, 1);
			//		HeaderSize += SizeOfLengthField;

			if (k == 0){
				BYTE* SizeOfRecord = new BYTE[SizeOfLengthField + 1];
				memcpy(SizeOfRecord, &Page.Page[StartOffset], SizeOfLengthField);

				CellSize = GetLengthOfBitPattern(SizeOfRecord, SizeOfLengthField);
				EndOffset += CellSize;
				EndOffset += SizeOfLengthField;

				delete[] SizeOfRecord;
			}
			HeaderSize += SizeOfLengthField;
			StartOffset += SizeOfLengthField;
		}

		BOOL overflow_e = FALSE;
		if (CellSize > RecordToRecord)		// overflow
		{
			RecordToRecord -= HeaderSize;
			
			BYTE PageNumber[4];
			memcpy(PageNumber, &Page.Page[StartOffset + RecordToRecord - 4], 4);
			INT OverflowPageNumberStart = ut.LongIntToByte(PageNumber, 4);
			ParsingRecordContainOverflow(Page,
				OverflowPageNumberStart,
				StartOffset,
				RecordToRecord - 4, // 레코드 크기.
				CellSize, // 전체 셀 사이즈로 하자.
				table,
				overflow_e);
		}

		else
			ParsingRecord(Page, StartOffset, EndOffset, table);
	}


}
VOID SQLiteRecovery::GetTableSchema(){

	InitSQLiteMasterTable();

	SQLitePage FirstPage;
	FirstPage.Page = new BYTE[PageSize + 1];
	memcpy(FirstPage.Page, &SQLITE_BUFFER[0], PageSize);

	INT PageNum = GetLeafPage(&FirstPage, sqlite_master.RootPageNum, &sqlite_master);
	delete[] FirstPage.Page;

	if (PageNum == sqlite_master.RootPageNum)
		sqlite_master.LeafPageNum.push_back(PageNum);
	// 첫 페이지에서 leaf일때

	for (int i = 0; i < sqlite_master.LeafPageNum.size(); i++){

		INT PageOffset = (sqlite_master.LeafPageNum[i] - 1) * PageSize;

		if (PageOffset > FileSize || PageOffset < 0)
			continue;

		SQLitePage Page;
		INT TEST;

		Page.PageNumber = sqlite_master.LeafPageNum[i];
		Page.Page = new BYTE[PageSize + 1];

		TEST = sqlite_master.DeletedRecord.size();
		memcpy(Page.Page, &SQLITE_BUFFER[PageOffset], PageSize);
		if(sqlite_master.LeafPageNum.size() == 1)
			GetSchemaCell_First(Page, &sqlite_master);
		else
			GetSchemaCell(Page, &sqlite_master);

		delete[] Page.Page;

	}
	SetSchemaInformationFromSQLITEMaster();
}
std::vector<SQLITE_COLUMN> SQLiteRecovery::GetSchemaInformation(CString input)
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
	return parse;
}
VOID SQLiteRecovery::SetSchemaInformationFromSQLITEMaster(){

//0131 DeletedRecord -> NormalRecord

	for (int i = 0; i < sqlite_master.NormalRecord.size(); i++){

		CString data(sqlite_master.NormalRecord[i].columns[0].Data);
		if (data.Find(L"table") != -1){


			SQLITE_TABLE table;


			table.RootPageNum = ut.LongIntToByte(sqlite_master.NormalRecord[i].columns[3].Data, sqlite_master.NormalRecord[i].columns[3].Length);
			table.TableName = sqlite_master.NormalRecord[i].columns[2].Data;

			CString Query(sqlite_master.NormalRecord[i].columns[4].Data);


			std::vector<SQLITE_COLUMN>	columns = GetSchemaInformation(Query);

			table.Column = new SQLITE_COLUMN[columns.size()];

			for (int idx = 0; idx < columns.size(); idx++){

//				CString ColumnType = columns[idx].Type.MakeUpper();

/*				if (ColumnType.Find(_T("INT")) != -1 || ColumnType.Find(_T("DECIMAL")) != -1
					|| ColumnType.Find(_T("REAL")) != -1 || ColumnType.Find(_T("DOUBLE")) != -1 ||
					ColumnType.Find(_T("NUMERIC")) != -1 || (ColumnType.Find(_T("DATE")) != -1 && ColumnType.Compare(_T("DATETIME")) != 0) ||
					ColumnType.Find(_T("BOOL")) != -1 || ColumnType.Find(_T("LONG")) != -1)
				{
					table.Column[idx].Type = _T("INTEGER");
				}
				else if (ColumnType.Find(_T("TEXT")) != -1 || ColumnType.Find(_T("CHAR")) != -1 ||
					ColumnType.Find(_T("STRING")) != -1 || ColumnType.Find(_T("DATETIME")) != -1 || ColumnType.Find(_T("NVARCHAR")) != -1)
				{
					table.Column[idx].Type = _T("TEXT");
				}
				else if (ColumnType.Find(_T("BLOB")) != -1 || ColumnType.Find(_T("CLOB")) != -1)
				{
					table.Column[idx].Type = _T("BLOB");
				}
				*/
				table.Column[idx].Type = columns[idx].Type;
				table.Column[idx].ColumnName = columns[idx].ColumnName;

			}



			//deleted_table.push_back(table);
			table.NumberOfField = columns.size();
			tables.push_back(table);

		}



	}


	/*
		
		리프노드 수집절차~!		ㄴ

		나중에 deleted_table 을 tables 로 옮겨야함 
	
	*/
	for (int i = 0; i < tables.size(); i++){


		/*if (deleted_table[i].RootPageNum != 0){
			INT PageNum = GetLeafPage(deleted_table[i].RootPageNum, &deleted_table[i]);
			if (PageNum == deleted_table[i].RootPageNum)
				deleted_table[i].LeafPageNum.push_back(PageNum);
		}*/

		if (tables[i].RootPageNum != 0){
			INT PageNum = GetLeafPage(tables[i].RootPageNum, &tables[i]);
			if (PageNum == tables[i].RootPageNum)
				tables[i].LeafPageNum.push_back(PageNum);
		}

	}



}
