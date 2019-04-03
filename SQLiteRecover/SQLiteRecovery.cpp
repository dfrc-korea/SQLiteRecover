#include "stdafx.h"
#include "SQLiteRecovery.h"


int debug = 0;

/*SQLiteRecovery::SQLiteRecovery(TCHAR* pszfilename){
	CFile f;
	HANDLE hFile = CreateFile(pszfilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	FileSize = GetFileSize(hFile, NULL);
	//temp = FileSize;
	CloseHandle(hFile);

	CString copyName(pszfilename);
	CString outputName(pszfilename);
	outputName = outputName.Left(outputName.Find(L".db"));
	copyName = copyName.Left(copyName.Find(L".db"));
	outputName += "_recovered.db";
	outputName = outputName.Right(outputName.GetLength() - outputName.ReverseFind(('\\')) - 1);
	copyName += "_copy.db";

	
	CString databasePath = OutputPath + outputName;

	BOOL b = CopyFile(pszfilename, copyName, FALSE);

	if (!f.Open(pszfilename, CFile::modeRead))
	{
		TRACE(_T("File could not be opened %d\n"));
		exit(1);

	}
	sqlite = SQLiteExtractor(copyName);


//	CSV.CreateDatabase(databasePath);

	SQLITE_BUFFER = (BYTE*)malloc(FileSize);
	f.Read(SQLITE_BUFFER, FileSize);
	f.Close();

	GetTableInformation();
//	journal = SQLiteJournal(PageSize);
	::DeleteFile(copyName);

}*/

SQLiteRecovery::SQLiteRecovery(TCHAR* pszfilename,TCHAR* _outputName){
	CFile f;
	HANDLE hFile = CreateFile(pszfilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);

	FileSize = GetFileSize(hFile, NULL);

	CloseHandle(hFile);

	CString copyName(pszfilename);
	CString outputName(pszfilename);
	outputName = outputName.Left(outputName.Find(L".db"));
	copyName = copyName.Left(copyName.Find(L".db"));
	outputName += "_recovered.db";
	outputName = outputName.Right(outputName.GetLength() - outputName.ReverseFind(('\\')) - 1);
	copyName += "_copy.db";

	OutputPath = _outputName;
	if (OutputPath.GetAt(OutputPath.GetLength() - 1) != '\\'){
		OutputPath += L"\\";
	}
	CString databasePath = OutputPath + outputName;
	//
	OutputPath = databasePath;
	BOOL b = CopyFile(pszfilename, copyName, FALSE);

	if (!f.Open(pszfilename, CFile::modeRead))
	{
		TRACE(_T("File could not be opened %d\n"));
		exit(1);

	}
	sqlite = SQLiteExtractor(copyName);


	CSV.CreateDatabase(databasePath);

	//버퍼 다올리지않을거임.
	SQLITE_BUFFER = (BYTE*)malloc(FileSize + 1);
	f.Read(SQLITE_BUFFER, FileSize);
	f.Close();

	BYTE SizeofPage[2];
	memcpy(SizeofPage, &SQLITE_BUFFER[0x10], 2);
	PageSize = ut.IntToByte(SizeofPage, 2);
	GetTableSchema();

//	GetTableInformation(); //sqlite_master 정상일때
//	journal = SQLiteJournal(PageSize);
	::DeleteFile(copyName);

}

BOOL SQLiteRecovery::GetTableInformation(){

	//헤더 세팅
	BYTE SizeofPage[2];
	memcpy(SizeofPage, &SQLITE_BUFFER[0x10], 2);

	PageSize = ut.IntToByte(SizeofPage, 2);

	sqlite.Init();
	sqlite.Open();
	tables = sqlite.SchemaTblParsing();
	sqlite.Close();
	return true;

}

// 비트 패턴으로 사이즈구하는거고
INT SQLiteRecovery::GetAvailableByteByBitPattern(BYTE* Page, INT OffsetOfRecord, INT Length){

	BYTE CheckingNumber = Page[OffsetOfRecord];


	if (!(CheckingNumber & 0x80)){
		return Length;
	}
	else{
		return GetAvailableByteByBitPattern(Page, OffsetOfRecord + 1, Length + 1);
	}


}
// 비트 패턴으로 사이즈구한 뒤 실제 최상위비트제거한 길이 값 구하는 거고
INT SQLiteRecovery::GetLengthOfBitPattern(BYTE Bit[], INT BitLength){

	INT Result = 0;
	for (int i = 0; i < BitLength; i++){

		INT Temp = Bit[i] & 0x7F;
		Result = Result | Temp;
		if ((Bit[i] & 0x80))
			Result = (Result << 7);
	}

	return Result;

}

VOID SQLiteRecovery::CollectLeafPage(){

	/*
	Leaf Page 수집
	*/
	for (int idx = 0; idx < tables.size(); idx++){

		if (tables[idx].RootPageNum != 0){
			INT PageNum = GetLeafPage(tables[idx].RootPageNum, &tables[idx]);
			if (PageNum == tables[idx].RootPageNum)
				tables[idx].LeafPageNum.push_back(PageNum);
		}


	}

}
VOID SQLiteRecovery::RecoverSQLite(){



	for (int idx = 0; idx < tables.size(); idx++){

		if (tables[idx].TableName.Find(L"word") != -1 || tables[idx].TableName.Find(L"sqlite_") != -1)
			continue;

			CSV.OpenFile(tables[idx]);
			CSV.CreateTableCSV(tables[idx]);
// 			CSV.CreateTable(tables[idx]);
// 			CSV.CreateTableDamaged(tables[idx]);
			wprintf(_T("# Table %s \n"), tables[idx].TableName);
			for (int i = 0; i < tables[idx].LeafPageNum.size(); i++){
				//각 테이블의 leaf page 개수만큼 순회 - hj
				//wprintf(_T("# Table %s : Page Number 0x%x\n"), tables[idx].TableName, tables[idx].LeafPageNum[i]);
				INT PageOffset = (tables[idx].LeafPageNum[i] - 1) * PageSize; // 각 leaf page의 page offset - hj
				if (PageOffset > FileSize){
					/*wprintf(_T("%d, %d : %d vs %d\n"), i, tables[idx].LeafPageNum[i] - 1, temp, PageOffset);*/
					continue;
				}

				SQLitePage Page;
				Page.PageNumber = tables[idx].LeafPageNum[i]; 
				Page.Page = new BYTE[PageSize + 1];

				memcpy(Page.Page, &SQLITE_BUFFER[PageOffset], PageSize);
				// leaf Page의 내용을 Page 변수에 저장 - hj
//				GetDeletedRecord(Page, &tables[idx]);
				//171024
				GetNormalCell(Page, &tables[idx]);
				// 각 leaf page에대해 Get Deleted Record ( FreeBlockAreaCarving / FreeSpaceAreaCarving ) - hj
				delete[] Page.Page;

			}

			wprintf(L"# Recover Table %s\n", tables[idx].TableName);

			CSV.InsertRecoveredRecordCSV(tables[idx]);
			CSV.InsertNormalRecordCSV(tables[idx]);

//			CSV.InsertRecoveredRecord(tables[idx]);
//			CSV.InsertRecoveredRecordDamaged(tables[idx]);
			CSV.CloseFile(tables[idx]);
		
	}

}

VOID SQLiteRecovery::NormalSQLite(){


	CSV.CreateTableDB(sqlite_master);

	for (int idx = 0; idx < tables.size(); idx++){

		if (tables[idx].TableName.Find(L"word") != -1 || tables[idx].TableName.Find(L"sqlite_") != -1)
			continue;

//			CSV.OpenFile(tables[idx]); 원래코드
//			CSV.CreateTableCSV(tables[idx]); //171026원래있어야함
			
// 			CSV.CreateTable(tables[idx]);
// 			CSV.CreateTableDamaged(tables[idx]);
			wprintf(_T("# Table %s \n"), tables[idx].TableName);
			for (int i = 0; i < tables[idx].LeafPageNum.size(); i++){
				//각 테이블의 leaf page 개수만큼 순회 - hj
				//wprintf(_T("# Table %s : Page Number 0x%x\n"), tables[idx].TableName, tables[idx].LeafPageNum[i]);
				INT PageOffset = (tables[idx].LeafPageNum[i] - 1) * PageSize; // 각 leaf page의 page offset - hj
				if (PageOffset > FileSize){
					/*wprintf(_T("%d, %d : %d vs %d\n"), i, tables[idx].LeafPageNum[i] - 1, temp, PageOffset);*/
					continue;
				}

				SQLitePage Page;
				Page.PageNumber = tables[idx].LeafPageNum[i]; 
				Page.Page = new BYTE[PageSize + 1];

				memcpy(Page.Page, &SQLITE_BUFFER[PageOffset], PageSize);
				// leaf Page의 내용을 Page 변수에 저장 - hj
				//GetDeletedRecord(Page, &tables[idx]);
				//171024

				//if (Page.PageNumber == 126)
//					printf("here\n");

				GetNormalCell(Page, &tables[idx]);
				//printf("%d\n", Page.PageNumber);
				// 각 leaf page에대해 Get Deleted Record ( FreeBlockAreaCarving / FreeSpaceAreaCarving ) - hj
				delete[] Page.Page;

			}

			wprintf(L"# Normal Parsing Table %s\n", tables[idx].TableName);

//			CSV.InsertRecoveredRecordCSV(tables[idx]); 원래코드!

			CSV.InsertRecoveredRecordDB(tables[idx]);
//			CSV.InsertRecoveredRecord(tables[idx]);
//			CSV.InsertRecoveredRecordDamaged(tables[idx]);
//			CSV.CloseFile(tables[idx]); 원래코드
		
	}

}




INT SQLiteRecovery::GetLeafPage(INT PAGE_START, SQLITE_TABLE* table){

	__int64 RootPageOffset = PageSize * (PAGE_START - 1);
	BYTE PageFlag = SQLITE_BUFFER[RootPageOffset];

	if (PageFlag == INTERNAL_PAGE){

		BYTE NumberOfRecord[2];
		memcpy(NumberOfRecord, &SQLITE_BUFFER[RootPageOffset + 3], 2);
		INT RecordNum = ut.IntToByte(NumberOfRecord, 2);
		int offsetToRecord = RootPageOffset + INTERNAL_PAGE_HEADER_SIZE;
		table->InternalNum.push_back(PAGE_START);
		
		for (int i = 0; i < RecordNum; i++){
			BYTE offset[2];
			memcpy(offset, &SQLITE_BUFFER[offsetToRecord], 2);
			INT pointer = ut.IntToByte(offset, 2);

			BYTE nextPage[4];
			memcpy(nextPage, &SQLITE_BUFFER[RootPageOffset + pointer], 4);
			INT nextOffset = ut.IntToByte(nextPage, 4);

			table->LeafPageNum.push_back(GetLeafPage(nextOffset, table));
			offsetToRecord += 2;
		}

	}
	else if (PageFlag == LEAF_PAGE){

		//table->ChilePageNum.push_back(PAGE_START);
		return PAGE_START;
	}

}
INT SQLiteRecovery::GetLeafPage(SQLitePage* Page, INT PAGE_START, SQLITE_TABLE* table){



	__int64 RootPageOffset = 0;


	if (PAGE_START == 1){
		RootPageOffset += SCHEMA_PAGE_START;
	}
	BYTE PageFlag = Page->Page[RootPageOffset];




	if (PageFlag == INTERNAL_PAGE){


		BYTE Cell[2];
		memcpy(Cell, &Page->Page[RootPageOffset + NUMBER_OF_CELL_OFFSET], NUMBER_OF_CELL_OFFSET_SIZE);
		INT CellCNT = ut.LongIntToByte(Cell, NUMBER_OF_CELL_OFFSET_SIZE);

		std::vector<INT>	celloffset;
		INT StartOffset = RootPageOffset + INTERNAL_PAGE_HEADER_SIZE;

		BYTE rightchildPage[4];
		memcpy(rightchildPage, &Page->Page[RootPageOffset + 8], 4);
		INT rightnextOffset = ut.IntToByte(rightchildPage, 4);

		table->LeafPageNum.push_back(rightnextOffset);
		//잘못돼잇잔아?celloffset.push_back(RootPageOffset + 8);

		table->InternalNum.push_back(PAGE_START);
		for (int i = 0; i < CellCNT; i++){

			memcpy(Cell, &Page->Page[StartOffset], NUMBER_OF_CELL_OFFSET_SIZE);
			celloffset.push_back(ut.LongIntToByte(Cell, 2));
			StartOffset += 2;

		}




		for (int i = 0; i < celloffset.size(); i++){
			BYTE nextPage[4];
			memcpy(nextPage, &Page->Page[celloffset[i]], 4);
			INT nextOffset = ut.IntToByte(nextPage, 4);
			SQLitePage NextPage;
			/*			NextPage->PageNumber; */
			NextPage.Page = new BYTE[PageSize + 1];
			memcpy(NextPage.Page, &SQLITE_BUFFER[(nextOffset - 1) * PageSize], PageSize);
			table->LeafPageNum.push_back(GetLeafPage(&NextPage, nextOffset, table));
			delete[] NextPage.Page;

		}

		/*

		BYTE NumberOfRecord[2];
		memcpy(NumberOfRecord, &Page.Page[RootPageOffset + 3], 2);
		INT RecordNum = ut.IntToByte(NumberOfRecord, 2);
		int offsetToRecord = RootPageOffset + INTERNAL_PAGE_HEADER_SIZE;
		table->InternalNum.push_back(PAGE_START);

		for (int i = 0; i < RecordNum; i++){
		BYTE offset[2];
		memcpy(offset, &Page.Page[offsetToRecord], 2);
		INT pointer = ut.IntToByte(offset, 2);

		BYTE nextPage[4];
		memcpy(nextPage, &Page.Page[RootPageOffset + pointer], 4);
		INT nextOffset = ut.IntToByte(nextPage, 4);
		SQLitePage NextPage;
		NextPage.PageNumber = nextOffset;
		NextPage.Page = new BYTE[PageSize + 1];
		memcpy(NextPage.Page, &SQLITE_BUFFER[(nextOffset - 1) * PageSize], PageSize);
		table->LeafPageNum.push_back(GetLeafPage(Page, nextOffset, table));
		delete[] NextPage.Page;
		offsetToRecord += 2;
		}*/

	}
	else if (PageFlag == LEAF_PAGE){

		//table->ChilePageNum.push_back(PAGE_START);
		return PAGE_START;
	}

}


VOID SQLiteRecovery::GetDeletedRecord(SQLitePage Page, SQLITE_TABLE* table){

//	wprintf(L"# Free space carving Table %s\n", table->TableName);
// 	FreeSpaceAreaCarving(Page, table);
// 
// 	FreeBlockAreaCarving(Page, table);




	/*
	
		1. 정상 레코드 오프셋 가져오기
		2. Freeblock 과 free space 오프셋 가져오기
		3. 소팅으로 레코드 to 레코드 구하기
	
	*/


	BYTE Cell[2];
	memcpy(Cell, &Page.Page[NUMBER_OF_CELL_OFFSET], NUMBER_OF_CELL_OFFSET_SIZE);
	INT CellCNT = ut.LongIntToByte(Cell, NUMBER_OF_CELL_OFFSET_SIZE);


	std::map<INT, BOOL>		celloffset;			//TRUE : 삭제, FALSE : 정상.


	INT StartOffset = LEAF_PAGE_HEADER_SIZE;

	for (int i = 0; i < CellCNT; i++){

		memcpy(Cell, &Page.Page[StartOffset], NUMBER_OF_CELL_OFFSET_SIZE);
		celloffset.insert(std::map<INT, BOOL>::value_type(ut.IntToByte(Cell, 2), FALSE));
		StartOffset += 2;

	}

	FreeBlockAreaCarving(Page, table);
	FreeSpaceAreaCarving(Page, table);


//	FreeBlockAreaCarving(Page, table, &celloffset);
//	FreeSpaceAreaCarving(Page, table, &celloffset);

}


VOID SQLiteRecovery::FreeBlockAreaCarving(SQLitePage Page, SQLITE_TABLE* table, std::map<INT, BOOL> *celloffset){
	INT CurrentOffset;
	BYTE FreeBlockOffset[2];
	memcpy(FreeBlockOffset, &Page.Page[0x01], 2);
	CurrentOffset = ut.IntToByte(FreeBlockOffset, 2);


	INT StartOffset = CurrentOffset;
	//wprintf(_T("# Free Block Offset Carving in %s\n"), table->TableName);
	while (StartOffset != 0){

		BYTE NextFreeBlock[2];
		BYTE FreeBlockSize[2];

		memcpy(NextFreeBlock, &Page.Page[StartOffset], 2);
		memcpy(FreeBlockSize, &Page.Page[StartOffset + 2], 2);

		INT EndOffset = StartOffset + ut.IntToByte(FreeBlockSize, 2);
		StartOffset += 4;


		while (StartOffset <= EndOffset){
			
			
			StartOffset = GetColumnStartOffset(Page.Page, StartOffset, EndOffset, *table);

			if (StartOffset != -1){
				//printf("columnnnnnnn\n");
				(*celloffset).insert(std::map<INT, BOOL>::value_type(StartOffset, TRUE));
				StartOffset += table->NumberOfField;
				CarvingRecord(Page, StartOffset, EndOffset, table); //임의 추가
			}
			else{
				// 프리블록에 삭제된거 파괴됨.
				break;
			}

		}

		StartOffset = ut.IntToByte(NextFreeBlock, 2);
	}

}


VOID SQLiteRecovery::FreeSpaceAreaCarving(SQLitePage Page, SQLITE_TABLE* table, std::map<INT, BOOL> *celloffset){


	BYTE NumberNormalCell[2];
	memset(NumberNormalCell, 0, 2);
	memcpy(NumberNormalCell, &Page.Page[0x03], 2);
	INT CellNumber = ut.IntToByte(NumberNormalCell, 2);

	memcpy(NumberNormalCell, &Page.Page[0x05], 2);
	INT SmallArea = ut.IntToByte(NumberNormalCell, 2);
	//StartOffset = LEAF_PAGE_HEADER_SIZE + (CellNumber * 2);

	/*for (int i = 0; i < CellNumber; i++){

	memcpy(NumberNormalCell, &Page[StartOffset], 2);
	SmallArea = SmallArea > ut.IntToUInt8(NumberNormalCell, 2) ? ut.IntToUInt8(NumberNormalCell, 2) : SmallArea;
	StartOffset += 2;

	}*/
	INT StartOffset = LEAF_PAGE_HEADER_SIZE + (CellNumber * 2);
	memcpy(NumberNormalCell, &Page.Page[StartOffset], 2);
	INT Offset = ut.IntToByte(NumberNormalCell, 2);

	while (Offset != 0){
		StartOffset += 2;
		memcpy(NumberNormalCell, &Page.Page[StartOffset], 2);
		Offset = ut.IntToByte(NumberNormalCell, 2);

	}

	//StartOffset = Offset;
	BYTE Scanner[4];
	memset(Scanner, 0, 4);
	memcpy(Scanner, &Page.Page[StartOffset], 4);
	Offset = ut.IntToByte(Scanner, 4);
	while (Offset == 0){
		StartOffset++;

		memcpy(Scanner, &Page.Page[StartOffset], 4);
		Offset = ut.IntToByte(Scanner, 4);

	}

	INT EndOffset = SmallArea;
	while (StartOffset <= EndOffset){
		StartOffset = GetColumnStartOffset(Page.Page, StartOffset, EndOffset, *table);


		if (StartOffset != -1){
			(*celloffset).insert(std::map<INT, BOOL>::value_type(StartOffset, TRUE));
			StartOffset += table->NumberOfField;
		}
		else{
			// 프리블록에 삭제된거 파괴됨.
			break;
		}
	}


}


VOID SQLiteRecovery::FreeBlockAreaCarving(SQLitePage Page, SQLITE_TABLE* table){
	// Free Block Offset 탐색
	INT CurrentOffset;
	BYTE FreeBlockOffset[2];
	memcpy(FreeBlockOffset, &Page.Page[0x01], 2);
	CurrentOffset = ut.IntToByte(FreeBlockOffset, 2);

	INT StartOffset = CurrentOffset;
	//wprintf(_T("# Free Block Offset Carving in %s\n"), table->TableName);
	while (StartOffset != 0){

		BYTE NextFreeBlock[3];
		BYTE FreeBlockSize[3];

		memcpy(NextFreeBlock, &Page.Page[StartOffset], 2);
		memcpy(FreeBlockSize, &Page.Page[StartOffset + 2], 2);

		INT EndOffset = StartOffset + ut.IntToByte(FreeBlockSize, 2);
		StartOffset += 4;

		StartOffset = GetColumnStartOffset(Page.Page, StartOffset, EndOffset, *table);

		if (StartOffset != -1){

			CarvingRecord(Page, StartOffset, EndOffset, table);
		}
		else{
			// 프리블록에 삭제된거 파괴됨.
		}

		StartOffset = ut.IntToByte(NextFreeBlock, 2);
	}


}
VOID SQLiteRecovery::FreeBlockAreaCarvingReverse(SQLitePage Page, SQLITE_TABLE* table){
	INT CurrentOffset;
	BYTE FreeBlockOffset[2];
	memcpy(FreeBlockOffset, &Page.Page[0x01], 2);
	CurrentOffset = ut.IntToByte(FreeBlockOffset, 2);

	INT StartOffset = CurrentOffset;
	//wprintf(_T("# Free Block Offset Carving in %s\n"), table->TableName);
	while (StartOffset != 0){		// While : Next Free Block Offset 

		BYTE NextFreeBlock[2];
		BYTE FreeBlockSize[2];

		memcpy(NextFreeBlock, &Page.Page[StartOffset], 2);
		memcpy(FreeBlockSize, &Page.Page[StartOffset + 2], 2);

		INT EndOffset = StartOffset + ut.IntToByte(FreeBlockSize, 2);
		StartOffset += 4;

		/*
		Free Block Size 만큼 이동해서 계속 찾을것. FreeBlock 저장저장저장

		*/
		std::vector<FreeBlock> freeblockdata;
		while (StartOffset < EndOffset){
			INT ColumnStartOffset = GetColumnStartOffsetReverse(Page.Page, StartOffset, EndOffset, *table);

			if (ColumnStartOffset != -1 && ColumnStartOffset + table->NumberOfField < EndOffset)
			{
				FreeBlock temp;
				temp.ColumnInfoStartOffset = ColumnStartOffset;
				temp.columnInfo = new COLUMN[table->NumberOfField + 1];
				for (int i = 1; i < table->NumberOfField; i++){
					INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, ColumnStartOffset, 1);
					BYTE* length = new BYTE[SizeOfLengthField + 1];
					memcpy(length, &Page.Page[ColumnStartOffset], SizeOfLengthField);
					INT ColumnByte = GetLengthOfBitPattern(length, SizeOfLengthField);

					temp.columnInfo[i].Length = ColumnByte;
					delete[] length;
					ColumnStartOffset += SizeOfLengthField;
				}
				if (!CheckingFreeBlockSizeArea(temp, table, ColumnStartOffset, EndOffset)){
					break;
				}
				temp.DataAreaStartOffset/* 데이터 시작이 FreeBlock 끝부분부터 파싱해오면서 마지막 오프셋이 되는지점임.*/ = ColumnStartOffset;
				freeblockdata.push_back(temp);
				StartOffset = ColumnStartOffset;

			}
			else{
				StartOffset++;
				continue;
			}


		}

		for (int i = freeblockdata.size() - 1; i >= 0; i--){
			CarvingRecordReverse(Page, EndOffset, freeblockdata[i].DataAreaStartOffset, &freeblockdata[i], table);
			EndOffset = (freeblockdata[i].ColumnInfoStartOffset - 4);
		}



		// 		if (StartOffset != -1){
		// 
		// 			INT RecordDataSize = CarvingRecord(Page, StartOffset, EndOffset, table);
		// 		}
		// 		else{
		// 			// 프리블록에 삭제된거 파괴됨.
		// 		}

		StartOffset = ut.IntToByte(NextFreeBlock, 2);
	}

}
BOOL SQLiteRecovery::CheckingFreeBlockSizeArea(FreeBlock data, SQLITE_TABLE* table,INT StartOffset, INT EndOffset){
	INT SizeOfDataField = EndOffset - StartOffset;
	INT ColumnSize = 0;
	for (int i = 1; i < table->NumberOfField; i++){

		switch (data.columnInfo[i].Length){
		case 0: case 8: case 9:
			break;
		case 1:case 2:case 3: case 4:
			ColumnSize += data.columnInfo[i].Length;
			break;
		case 5:
			ColumnSize += 6;
			break;
		case 6: case 7:
			ColumnSize += 8;
			break;
		default:
			INT BitOfLength = data.columnInfo[i].Length;
			if (BitOfLength > 12 || BitOfLength > 13){
				ColumnSize += ((BitOfLength % 2) == 0 ? ((BitOfLength - 12) / 2) : ((BitOfLength - 13) / 2));
			}
			break;

		}


	}


	if (SizeOfDataField < ColumnSize)
		return FALSE;
	return TRUE;
}
VOID SQLiteRecovery::FreeSpaceAreaCarving(SQLitePage Page, SQLITE_TABLE* table){

	BYTE NumberNormalCell[2];
	memset(NumberNormalCell, 0 , 2);
	memcpy(NumberNormalCell, &Page.Page[0x03], 2);
	INT CellNumber = ut.IntToByte(NumberNormalCell, 2);

	memcpy(NumberNormalCell, &Page.Page[0x05], 2);
	INT SmallArea = ut.IntToByte(NumberNormalCell, 2);
	//StartOffset = LEAF_PAGE_HEADER_SIZE + (CellNumber * 2);

	/*for (int i = 0; i < CellNumber; i++){

	memcpy(NumberNormalCell, &Page[StartOffset], 2);
	SmallArea = SmallArea > ut.IntToUInt8(NumberNormalCell, 2) ? ut.IntToUInt8(NumberNormalCell, 2) : SmallArea;
	StartOffset += 2;

	}*/
	INT StartOffset = LEAF_PAGE_HEADER_SIZE + (CellNumber * 2);
	memcpy(NumberNormalCell, &Page.Page[StartOffset], 2);
	INT Offset = ut.IntToByte(NumberNormalCell, 2);

	while (Offset != 0){
		StartOffset += 2;
		memcpy(NumberNormalCell, &Page.Page[StartOffset], 2);
		Offset = ut.IntToByte(NumberNormalCell, 2);
		
	}

	//StartOffset = Offset;
	BYTE Scanner[4];
	memset(Scanner, 0, 4);
	memcpy(Scanner, &Page.Page[StartOffset], 4);
	Offset = ut.IntToByte(Scanner, 4);
	while (Offset == 0){
		StartOffset++;

		memcpy(Scanner, &Page.Page[StartOffset], 4);
		Offset = ut.IntToByte(Scanner, 4);

	}

	INT EndOffset = SmallArea;
	while (StartOffset < EndOffset){
		StartOffset = GetColumnStartOffset(Page.Page, StartOffset, EndOffset, *table);

		if (StartOffset != -1){

			INT RecordDataSize = CarvingRecord(Page, StartOffset, EndOffset, table);
			StartOffset += RecordDataSize;
		}
		else{
			// 프리블록에 삭제된거 파괴됨.
			break;
		}
	}
//	wprintf(L"3333333333333333\n");
}




BOOL SQLiteRecovery::isOverwriteCellHeader(BYTE* Page){
	BYTE NormalCell[2];
	INT StartOffset = LEAF_PAGE_HEADER_SIZE ;
	memcpy(NormalCell, &Page[StartOffset], 2);
	INT Offset = ut.IntToByte(NormalCell, 2);
	INT HeaderSize = 0;

	// 1. RecordSize 2. RecordID 3. CellHeader Size
	
	for (int i = 0; i < 3; i++){
		INT SizeOfLengthField = GetAvailableByteByBitPattern(Page, Offset, 1);
		HeaderSize += SizeOfLengthField;
		Offset += SizeOfLengthField;
	}


	if (HeaderSize == 3)
		return TRUE;		// 컬럼 앞에 덮어씌워짐
	else
		return FALSE;		//컬럼 안덮어써짐

}


INT SQLiteRecovery::GetColumnStartOffset(BYTE* Page, INT StartOffset, INT EndOffset, SQLITE_TABLE table){

	INT Start = 0;
	INT i = 0;
	while (StartOffset < EndOffset && StartOffset < PageSize){


		Start = StartOffset;

		for (i = 0; i < table.NumberOfField; i++){

			INT SizeOfLengthField = GetAvailableByteByBitPattern(Page, StartOffset, 1);
			BYTE* length = new BYTE[SizeOfLengthField + 1];
			memcpy(length, &Page[StartOffset], SizeOfLengthField);
			INT ColumnByte = GetLengthOfBitPattern(length, SizeOfLengthField);


			if (table.Column[i].Type.Find(L"INTEGER") != -1){

				if (0x00 <= ColumnByte && ColumnByte <= 9){
					StartOffset++;
				}
				else{
					StartOffset = Start + 1;
					break;
				}

			}
			else{
				if (12 <= ColumnByte || ColumnByte == 0){
/*					if (SizeOfLengthField > 1)*/
						StartOffset += SizeOfLengthField;
// 					else
// 						StartOffset++;
				}
				else{
					StartOffset = Start + 1;
					break;
				}
			}



			delete[] length;
		}

		if (i == table.NumberOfField)
			return Start;

		if (StartOffset > PageSize)
			return -1;
	}


	return -1;


}

INT SQLiteRecovery::GetColumnStartOffsetReverse(BYTE* Page, INT StartOffset, INT EndOffset, SQLITE_TABLE table){
	INT Start = 0;
	INT i = 1;
	while (StartOffset < EndOffset && StartOffset < PageSize){

		Start = StartOffset;
		for (i = 1; i < table.NumberOfField; i++){

			INT SizeOfLengthField = GetAvailableByteByBitPattern(Page, StartOffset, 1);
			BYTE* length = new BYTE[SizeOfLengthField + 1];
			memcpy(length, &Page[StartOffset], SizeOfLengthField);
			INT ColumnByte = GetLengthOfBitPattern(length, SizeOfLengthField);


			if (table.Column[i].Type.Find(L"INTEGER") != -1 ){

				if (0x00 <= ColumnByte && ColumnByte <= 0x09){
					StartOffset++;
				}
				else{
					StartOffset = Start + 1;
					break;
				}

			}
			else{
				if (12 <= ColumnByte || ColumnByte == 0){
//					if (SizeOfLengthField > 1)
						StartOffset += SizeOfLengthField;
// 					else
// 						StartOffset++;
				}
				else{
					StartOffset = Start + 1;
					break;
				}
			}



			delete[] length;
		}

		if (i == table.NumberOfField )
			return Start;

		if (StartOffset > PageSize)
			return -1;
	}


	return -1;
}



BOOL SQLiteRecovery::CheckingFreeBlockHeaderInFreeSpace(BYTE* CheckingBYTE){

	BYTE NextFreeBlock[2];
	memcpy(NextFreeBlock, &CheckingBYTE[0], 2);

	BYTE SizeOfBlock[2];
	memcpy(SizeOfBlock, &CheckingBYTE[2], 2);


	INT NFB = ut.IntToByte(NextFreeBlock, 2);
	INT SOB = ut.LongIntToByte(SizeOfBlock, 2);


	if (NFB == 0 && SOB != 0)
		return TRUE;
	return FALSE;


}
VOID SQLiteRecovery::FreeSpaceAreaCarvingReverse(SQLitePage Page, SQLITE_TABLE* table){

	BYTE NumberNormalCell[2];
	memset(NumberNormalCell, 0, 2);
	memcpy(NumberNormalCell, &Page.Page[0x03], 2);
	INT CellNumber = ut.IntToByte(NumberNormalCell, 2);

	memcpy(NumberNormalCell, &Page.Page[0x05], 2);
	INT SmallArea = ut.IntToByte(NumberNormalCell, 2);
	//StartOffset = LEAF_PAGE_HEADER_SIZE + (CellNumber * 2);

	/*for (int i = 0; i < CellNumber; i++){

	memcpy(NumberNormalCell, &Page[StartOffset], 2);
	SmallArea = SmallArea > ut.IntToUInt8(NumberNormalCell, 2) ? ut.IntToUInt8(NumberNormalCell, 2) : SmallArea;
	StartOffset += 2;

	}*/
	INT StartOffset = LEAF_PAGE_HEADER_SIZE + (CellNumber * 2);
	memcpy(NumberNormalCell, &Page.Page[StartOffset], 2);
	INT Offset = ut.IntToByte(NumberNormalCell, 2);

	while (Offset != 0){
		StartOffset += 2;
		memcpy(NumberNormalCell, &Page.Page[StartOffset], 2);
		Offset = ut.IntToByte(NumberNormalCell, 2);

	}

	//StartOffset = Offset;
	BYTE Scanner[4];
	memset(Scanner, 0, 4);
	memcpy(Scanner, &Page.Page[StartOffset], 4);
	Offset = ut.IntToByte(Scanner, 4);
	while (Offset == 0){
		StartOffset++;

		memcpy(Scanner, &Page.Page[StartOffset], 4);
		Offset = ut.IntToByte(Scanner, 4);

	}

	BOOL Checking = CheckingFreeBlockHeaderInFreeSpace(Scanner);

	if (Checking)
		StartOffset += 4;

	//	wprintf(L"22222222\n");
	INT EndOffset = SmallArea;
	//	wprintf(L"Page Number : %d \n", Page.PageNumber);

	std::vector<FreeBlock> freeblockdata;
	while (StartOffset < EndOffset){
		INT ColumnStartOffset = GetColumnStartOffsetReverse(Page.Page, StartOffset, EndOffset, *table);

		if (ColumnStartOffset != -1 && ColumnStartOffset + table->NumberOfField < EndOffset)
		{
			FreeBlock temp;
			temp.ColumnInfoStartOffset = ColumnStartOffset;
			temp.columnInfo = new COLUMN[table->NumberOfField + 1];
			for (int i = 1; i < table->NumberOfField; i++){
				INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, ColumnStartOffset, 1);
				BYTE* length = new BYTE[SizeOfLengthField + 1];
				memcpy(length, &Page.Page[ColumnStartOffset], SizeOfLengthField);
				INT ColumnByte = GetLengthOfBitPattern(length, SizeOfLengthField);

				temp.columnInfo[i].Length = ColumnByte;
				delete[] length;
				ColumnStartOffset += SizeOfLengthField;
			}
			if (!CheckingFreeBlockSizeArea(temp, table, ColumnStartOffset, EndOffset)){
				break;
			}
			temp.DataAreaStartOffset/* 데이터 시작이 FreeBlock 끝부분부터 파싱해오면서 마지막 오프셋이 되는지점임.*/ = ColumnStartOffset;
			freeblockdata.push_back(temp);
			StartOffset = ColumnStartOffset;

		}
		else{
			StartOffset++;
			continue;
		}


	}

	for (int i = freeblockdata.size() - 1; i >= 0; i--){
		CarvingRecordReverse(Page, EndOffset, freeblockdata[i].DataAreaStartOffset, &freeblockdata[i], table);
		EndOffset = (freeblockdata[i].ColumnInfoStartOffset - 4);
	}

}



INT SQLiteRecovery::InitTableInfo(){


	for (int i = 0; i < tables.size(); i++){

		tables[i].DeletedRecord.clear();

	}
	return 1;
}

INT SQLiteRecovery::GetIndexOfTableListByWALPageNumber(INT PageNumber){


	for (int idx = 0; idx < tables.size(); idx++){
		
		if (tables[idx].RootPageNum == PageNumber)
			return idx;
		for (int i = 0; i < tables[idx].LeafPageNum.size(); i++){

			if (tables[idx].LeafPageNum[i] == PageNumber)
				return idx;
		}

	}
	return -1;

}
VOID SQLiteRecovery::RecoverWAL(TCHAR* pszPath){

	WAL.SetWALFrameList(pszPath);
	std::map<INT, std::vector<WALFrame>> wal_list = WAL.GetWALFrameList();


	std::map<INT, std::vector<WALFrame>>::iterator iter;
	wprintf(L"[*] WAL Recover start\n");
	for (iter = wal_list.begin(); iter != wal_list.end(); iter++){

		INT PageNumber = iter->first;
		INT idx = GetIndexOfTableListByWALPageNumber(PageNumber);


		if (idx == -1) // 테이블 드롭
			continue;

		if (tables[idx].TableName.Find(L"word") != -1 || tables[idx].TableName.Find(L"sqlite_") != -1)
			continue;

		CSV.OpenFile(tables[idx]);
		for (int i = 0; i < iter->second.size(); i++){

			SQLitePage Page;
			Page.PageNumber = ut.LongIntToByte(iter->second[i].header.SQLitePageNumber,4);
			Page.Page = new BYTE[PageSize + 1];
			memcpy(Page.Page, &iter->second[i].Page[0], PageSize);
			GetDeletedRecord(Page, &tables[idx]);
			delete[] Page.Page;

		}
		CSV.InsertRecoveredRecordCSV(tables[idx]);
		CSV.CloseFile(tables[idx]);
	}

}

VOID SQLiteRecovery::SetCSVPath(CString Path){
	OutputPath = Path;
	CSV.SetOutputPath(Path);
}

VOID SQLiteRecovery::GetCellFromJournal(SQLitePage Page, SQLITE_TABLE* table){


	BYTE Cell[2];
	memcpy(Cell, &Page.Page[NUMBER_OF_CELL_OFFSET], NUMBER_OF_CELL_OFFSET_SIZE);
	INT CellCNT = ut.LongIntToByte(Cell, NUMBER_OF_CELL_OFFSET_SIZE);

	std::vector<INT>	celloffset;
	INT StartOffset = LEAF_PAGE_HEADER_SIZE;

	for (int i = 0; i < CellCNT; i++){

		memcpy(Cell, &Page.Page[StartOffset], NUMBER_OF_CELL_OFFSET_SIZE);
		celloffset.push_back(ut.LongIntToByte(Cell, 2));
		StartOffset += 2;

	}


	for (int i = 0; i < celloffset.size(); i++){

	
		StartOffset = celloffset[i];
		INT EndOffset = StartOffset;
		for (int k = 0; k < 3; k++){

			INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, StartOffset, 1);
	//		HeaderSize += SizeOfLengthField;

			if (k == 0){
				BYTE* SizeOfRecord = new BYTE[SizeOfLengthField + 1];
				memcpy(SizeOfRecord, &Page.Page[StartOffset], SizeOfLengthField);
				EndOffset += GetLengthOfBitPattern(SizeOfRecord, SizeOfLengthField);
				EndOffset += SizeOfLengthField;
				delete[] SizeOfRecord;
			}

			StartOffset += SizeOfLengthField;
		}

		CarvingRecord(Page, StartOffset, EndOffset, table);

	}


}

INT SQLiteRecovery::GetPageSize(){
	return PageSize;
}
// 미확인 테이블 -> 정상레코드로부터 스키마정보 획득 후 복구 진행
SQLITE_TABLE SQLiteRecovery::GetCellFromJournal(SQLitePage Page){
	
	SQLITE_TABLE UnknownTable;
	UnknownTable.NumberOfField = 0;

	

	BYTE NormalCell[2];
	memcpy(NormalCell, &Page.Page[8], 2);
	INT celloffset = ut.LongIntToByte(NormalCell, 2);




	if (celloffset == 0)
		return UnknownTable;


	INT StartOffset = celloffset;
	INT EndOffset = StartOffset;


	for (int k = 0; k < 2; k++){

		INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, StartOffset, 1);
		//		HeaderSize += SizeOfLengthField;

		if (k == 0){
			BYTE* SizeOfRecord = new BYTE[SizeOfLengthField + 1];
			memcpy(SizeOfRecord, &Page.Page[StartOffset], SizeOfLengthField);
			EndOffset += GetLengthOfBitPattern(SizeOfRecord, SizeOfLengthField);
			EndOffset += SizeOfLengthField;
			delete[] SizeOfRecord;
		}

		StartOffset += SizeOfLengthField;
	}

	// HeaderSize
	INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, StartOffset, 1);
	BYTE* SizeOfRecord = new BYTE[SizeOfLengthField + 1];
	memcpy(SizeOfRecord, &Page.Page[StartOffset], SizeOfLengthField);

	INT HeaderSize = GetLengthOfBitPattern(SizeOfRecord, SizeOfLengthField);
	StartOffset += SizeOfLengthField;
	delete[] SizeOfRecord;

	INT NumberOfColumn = 0;
	std::vector<SQLITE_COLUMN>	TargetColumn;
	INT EndOfHeader = StartOffset + HeaderSize - 1;


	while (StartOffset < EndOfHeader){

		INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, StartOffset, 1);
		BYTE* length = new BYTE[SizeOfLengthField + 1];
		memcpy(length, &Page.Page[StartOffset], SizeOfLengthField);
		INT ColumnByte = GetLengthOfBitPattern(length, SizeOfLengthField);

		SQLITE_COLUMN column;
		if (1 <= ColumnByte && ColumnByte <= 9){
			column.ColumnName = L"INTEGER";
			column.Type = L"INTEGER";
		}
		else if (ColumnByte == 0){
			column.ColumnName = L"Unknown";
			column.Type = L"NULL";
		}
		else if (12 <= ColumnByte && ColumnByte % 2 == 0){
			column.ColumnName = L"BLOB";
			column.Type = L"BLOB";
		}
		else{
			column.ColumnName = L"TEXT";
			column.Type = L"TEXT";
		}


		TargetColumn.push_back(column);
		StartOffset += SizeOfLengthField;
		delete[] length;

	}
	UnknownTable.NumberOfField = TargetColumn.size();
	UnknownTable.Column = new SQLITE_COLUMN[TargetColumn.size()];
	for (int i = 0; i < TargetColumn.size(); i++){

		UnknownTable.Column[i] = TargetColumn[i];


	}


	return UnknownTable;
	
}

VOID SQLiteRecovery::RecoverJournal(TCHAR* pszPath){
	
	SQLiteJournal journal(PageSize);
	journal.SetJournalFrameList(pszPath);
	std::map<INT, std::vector<JournalFrame>> journal_list = journal.GetJournalFrameList();
	

	std::map<INT, std::vector<JournalFrame>>::iterator iter;
	wprintf(L"[*] -journal Recover start\n");

	int UnknownTableNumber = 0;

	int num = 0; 
	for (iter = journal_list.begin(); iter != journal_list.end(); iter++){

		INT PageNumber = iter->first;
// 		INT idx = GetIndexOfTableListByWALPageNumber(PageNumber);
// 
// 
// 		if (idx == -1 ) // 테이블 드롭
// 			continue;
// 
// 		if (tables[idx].TableName.Find(L"word") != -1 || tables[idx].TableName.Find(L"sqlite_") != -1)
// 			continue;

		

		for (int i = 0; i < iter->second.size(); i++){

			SQLitePage Page;
			Page.PageNumber = ut.LongIntToByte(iter->second[i].PageNumber, 4);
			Page.Page = new BYTE[PageSize + 1];
			memcpy(Page.Page, &iter->second[i].Page[0], PageSize);
			if (Page.Page[0] == LEAF_PAGE){

				num++;

				int TableIndex = GetSchemaFromNormalRecord(Page);
				//GetCellFromJournal(Page, &tables[TableIndex]);
				if (TableIndex != -1){
					

					if (!CSV.isFileCreated(tables[TableIndex])){
						CSV.OpenFile(tables[TableIndex]);
						CSV.CreateTableCSV(tables[TableIndex]);
					}
					else{
						CSV.OpenFile(tables[TableIndex]);
					}
						


					GetCellFromJournal(Page, &tables[TableIndex]);

					if (OutputPath.Find(L"Kakao") == -1)
						GetDeletedRecord(Page, &tables[TableIndex]);



	

					CSV.InsertRecoveredRecordCSV(tables[TableIndex]);
					CSV.CloseFile(tables[TableIndex]);

				}
				else{
					// Unknown Table
					SQLITE_TABLE UnKnownTable = GetCellFromJournal(Page);
					UnknownTableNumber++;
					UnKnownTable.TableName.Format(L"UnKnownTableinPage_%d", UnknownTableNumber);
					if (UnKnownTable.NumberOfField != 0){


					


						GetCellFromJournal(Page, &UnKnownTable);



						if (OutputPath.Find(L"Kakao") == -1)
							GetDeletedRecord(Page, &UnKnownTable);



						if (!CSV.isFileCreated(UnKnownTable)){
							CSV.OpenFile(UnKnownTable);
							CSV.CreateTableCSV(UnKnownTable);
						}
						else{
							CSV.OpenFile(UnKnownTable);
						}

						CSV.InsertRecoveredRecordCSV(UnKnownTable);
						CSV.CloseFile(UnKnownTable);


						
					}

				}
					
			}
			
			delete[] Page.Page;

		}

	}
	wprintf(L"[*] Number of Leaf page : %d\n", num);
}