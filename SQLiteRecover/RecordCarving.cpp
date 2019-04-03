#include "stdafx.h"
#include "SQLiteRecovery.h"

//171024 정상 레코드 파싱

INT SQLiteRecovery::ParsingRecord(SQLitePage Page, INT StartOffset, INT EndOffset, SQLITE_TABLE* table){
	// 페이지 사이즈 고려해서 해야됨.
	INT RecordSize = 0;
	Rows row;
	BOOL isDamaged = FALSE;


	row.PageNumber = Page.PageNumber;
	row.RecordOffset = ((Page.PageNumber-1) * PageSize) + StartOffset;
	//wprintf(L"#1 Page Offet : %d \n", row.RecordOffset);
	// 테이블 컬럼 정보 가져오고 
	for (int i = 0; i < table->NumberOfField; i++){
		INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, StartOffset, 1);
		BYTE* length = new BYTE[SizeOfLengthField + 1];
		memcpy(length, &Page.Page[StartOffset], SizeOfLengthField);
		INT ColumnByte = GetLengthOfBitPattern(length, SizeOfLengthField);

		COLUMN column;
		column.Length = ColumnByte;
		row.columns.push_back(column);

		StartOffset += SizeOfLengthField;
		RecordSize += SizeOfLengthField;
		delete[] length;
	}
	//wprintf(L"#2 Page Offet : %d \n", row.RecordOffset);
	// 레코드 파싱
	for (int i = 0; i < table->NumberOfField; i++){

		row.columns[i].isDamaged = FALSE;
		int Length = row.columns[i].Length;

		if (StartOffset < EndOffset && Length == 8){
			row.columns[i].Data = new BYTE[2];
			row.columns[i].Length = 1;

			row.columns[i].Data[0] = 0x00;
			row.columns[i].Data[1] = NULL;

			if (table->Column[i].ColumnName == L"Unknown"){
				table->Column[i].Type = L"INTEGER";
			}


		}
		else if (StartOffset < EndOffset && Length == 9){
			row.columns[i].Data = new BYTE[2];
			row.columns[i].Length = 1;
			row.columns[i].Data[0] = 0x01;
			row.columns[i].Data[1] = NULL;

			if (table->Column[i].ColumnName == L"Unknown"){
				table->Column[i].Type = L"INTEGER";
			}



		}
		else if (StartOffset < EndOffset && 1 <= Length && Length <= 4){
			row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
			memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);

			StartOffset += row.columns[i].Length;
			RecordSize += row.columns[i].Length;

			if (table->Column[i].ColumnName == L"Unknown"){
				table->Column[i].Type = L"INTEGER";
			}


		}
		else if (StartOffset < EndOffset && Length == 5){
			row.columns[i].Length = 6;
			row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
			memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
			row.columns[i].Data[row.columns[i].Length] = NULL;

			StartOffset += row.columns[i].Length;
			RecordSize += row.columns[i].Length;

			if (table->Column[i].ColumnName == L"Unknown"){
				table->Column[i].Type = L"INTEGER";
			}


		}
		else if (StartOffset < EndOffset && (Length == 6 || Length == 7)){
			row.columns[i].Length = 8;
			row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
			memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
			row.columns[i].Data[row.columns[i].Length] = NULL;

			StartOffset += row.columns[i].Length;
			RecordSize += row.columns[i].Length;

			if (table->Column[i].ColumnName == L"Unknown"){
				table->Column[i].Type = L"INTEGER";
			}


		}
		else if (StartOffset < EndOffset && (Length > 12 || Length > 13)){
			INT BitOfLength = row.columns[i].Length;
			if (BitOfLength > 12 || BitOfLength > 13){
				row.columns[i].Length = ((BitOfLength % 2) == 0 ? ((BitOfLength - 12) / 2) : ((BitOfLength - 13) / 2));
			}
			row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
			memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
			row.columns[i].Data[row.columns[i].Length] = NULL;

			//if (row.RecordOffset == 8471649)
		//	row.columns[i].isDamaged = !utfchecker.CheckString(row.columns[i].Data, row.columns[i].Length);

			StartOffset += row.columns[i].Length;
			RecordSize += row.columns[i].Length;

			if (table->Column[i].ColumnName == L"Unknown"){
				table->Column[i].Type = L"TEXT";
			}


		}
		else if (Length == 0){
			row.columns[i].Length = 0;
			row.columns[i].isDamaged = FALSE;
		}
		else if (StartOffset >= EndOffset){
			row.columns[i].Length = 0;
			row.columns[i].isDamaged = TRUE;
		}

	}
	//wprintf(L"#3 Page Offet : %d \n", row.RecordOffset);
	table->NormalRecord.push_back(row);
	//wprintf(L"#4 Page Offet : %d \n", row.RecordOffset);
	return RecordSize;
}



INT SQLiteRecovery::CarvingRecord(SQLitePage Page, INT StartOffset, INT EndOffset, SQLITE_TABLE* table){
	// 페이지 사이즈 고려해서 해야됨.
	INT RecordSize = 0;
	Rows row;
	BOOL isDamaged = FALSE;


	row.PageNumber = Page.PageNumber;
	row.RecordOffset = ((Page.PageNumber-1) * PageSize) + StartOffset;
	//wprintf(L"#1 Page Offet : %d \n", row.RecordOffset);
	// 테이블 컬럼 정보 가져오고 
	for (int i = 0; i < table->NumberOfField; i++){
		INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, StartOffset, 1);
		BYTE* length = new BYTE[SizeOfLengthField + 1];
		memcpy(length, &Page.Page[StartOffset], SizeOfLengthField);
		INT ColumnByte = GetLengthOfBitPattern(length, SizeOfLengthField);

		COLUMN column;
		column.Length = ColumnByte;
		row.columns.push_back(column);

		StartOffset += SizeOfLengthField;
		RecordSize += SizeOfLengthField;
		delete[] length;
	}
	//wprintf(L"#2 Page Offet : %d \n", row.RecordOffset);
	// 레코드 파싱
	for (int i = 0; i < table->NumberOfField; i++){

		row.columns[i].isDamaged = FALSE;
		int Length = row.columns[i].Length;

		if (StartOffset < EndOffset && Length == 8){
			row.columns[i].Data = new BYTE[2];
			row.columns[i].Length = 1;

			row.columns[i].Data[0] = 0x00;
			row.columns[i].Data[1] = NULL;

			if (table->Column[i].ColumnName == L"Unknown"){
				table->Column[i].Type = L"INTEGER";
			}


		}
		else if (StartOffset < EndOffset && Length == 9){
			row.columns[i].Data = new BYTE[2];
			row.columns[i].Length = 1;
			row.columns[i].Data[0] = 0x01;
			row.columns[i].Data[1] = NULL;

			if (table->Column[i].ColumnName == L"Unknown"){
				table->Column[i].Type = L"INTEGER";
			}



		}
		else if (StartOffset < EndOffset && 1 <= Length && Length <= 4){
			row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
			memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);

			StartOffset += row.columns[i].Length;
			RecordSize += row.columns[i].Length;

			if (table->Column[i].ColumnName == L"Unknown"){
				table->Column[i].Type = L"INTEGER";
			}


		}
		else if (StartOffset < EndOffset && Length == 5){
			row.columns[i].Length = 6;
			row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
			memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
			row.columns[i].Data[row.columns[i].Length] = NULL;

			StartOffset += row.columns[i].Length;
			RecordSize += row.columns[i].Length;

			if (table->Column[i].ColumnName == L"Unknown"){
				table->Column[i].Type = L"INTEGER";
			}


		}
		else if (StartOffset < EndOffset && (Length == 6 || Length == 7)){
			row.columns[i].Length = 8;
			row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
			memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
			row.columns[i].Data[row.columns[i].Length] = NULL;

			StartOffset += row.columns[i].Length;
			RecordSize += row.columns[i].Length;

			if (table->Column[i].ColumnName == L"Unknown"){
				table->Column[i].Type = L"INTEGER";
			}


		}
		else if (StartOffset < EndOffset && (Length >= 12 || Length >= 13)){

//		else if (StartOffset < EndOffset && (Length > 12 || Length > 13)){ //수정전
			INT BitOfLength = row.columns[i].Length;
			if (BitOfLength >= 12 || BitOfLength >= 13){
//			if (BitOfLength > 12 || BitOfLength > 13){ //수정전
				row.columns[i].Length = ((BitOfLength % 2) == 0 ? ((BitOfLength - 12) / 2) : ((BitOfLength - 13) / 2));
			}
			row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
			memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
			row.columns[i].Data[row.columns[i].Length] = NULL;

			//if (row.RecordOffset == 8471649)
		//	row.columns[i].isDamaged = !utfchecker.CheckString(row.columns[i].Data, row.columns[i].Length);

			StartOffset += row.columns[i].Length;
			RecordSize += row.columns[i].Length;

			if (table->Column[i].ColumnName == L"Unknown"){
				table->Column[i].Type = L"TEXT";
			}


		}
		else if (Length == 0){
			row.columns[i].Length = 0;
			row.columns[i].isDamaged = FALSE;
		}
		else if (StartOffset >= EndOffset){
			row.columns[i].Length = 0;
			row.columns[i].isDamaged = TRUE;
		}

	}
	//wprintf(L"#3 Page Offet : %d \n", row.RecordOffset);
	table->DeletedRecord.push_back(row);
	//wprintf(L"#4 Page Offet : %d \n", row.RecordOffset);
	return RecordSize;
}

INT SQLiteRecovery::CarvingRecordReverse(SQLitePage Page, INT StartOffset, INT EndOffset, FreeBlock* FreeBlockData, SQLITE_TABLE* table){
	// 페이지 사이즈 고려해서 해야됨.
	INT RecordSize = 0;
	Rows row;

	row.PageNumber = Page.PageNumber;
	row.RecordOffset = ((Page.PageNumber - 1) * PageSize) + EndOffset;

	// 테이블 컬럼 정보 가져오고 
	for (int i = 1; i < table->NumberOfField; i++){

		row.columns.push_back(FreeBlockData->columnInfo[i]);
		// 
		// 		StartOffset += SizeOfLengthField;
		// 		RecordSize += SizeOfLengthField;
	}
	//EndOffset = FreeBlockData->DataAreaStartOffset;		// 컬럼 정보끝나고 데이터 시작부분!!
	// 레코드 파싱


	for (int i = row.columns.size() - 1; i >= 0; i--){

		int Length = row.columns[i].Length;
		row.columns[i].isDamaged = FALSE;

		if (Length == 8){
			row.columns[i].Data = new BYTE[2];
			row.columns[i].Length = 1;

			row.columns[i].Data[0] = 0x00;
			row.columns[i].Data[1] = NULL;

		}
		else if (Length == 9){
			row.columns[i].Data = new BYTE[2];
			row.columns[i].Length = 1;
			row.columns[i].Data[0] = 0x01;
			row.columns[i].Data[1] = NULL;

		}
		else if (StartOffset >= EndOffset && 1 <= Length && Length <= 4){

			row.columns[i].Data = new BYTE[row.columns[i].Length + 1];

			StartOffset -= row.columns[i].Length;

			memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);

			RecordSize += row.columns[i].Length;


		}
		else if (StartOffset >= EndOffset && Length == 5){

			row.columns[i].Length = 6;
			row.columns[i].Data = new BYTE[row.columns[i].Length + 1];

			StartOffset -= row.columns[i].Length;

			memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
			row.columns[i].Data[row.columns[i].Length] = NULL;


			RecordSize += row.columns[i].Length;

		}
		else if (StartOffset >= EndOffset && (Length == 6 || Length == 7)){
			row.columns[i].Length = 8;
			row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
			StartOffset -= row.columns[i].Length;

			memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
			row.columns[i].Data[row.columns[i].Length] = NULL;

			RecordSize += row.columns[i].Length;
		}
		else if (StartOffset >= EndOffset && (Length >= 12 || Length >= 13)){
			INT BitOfLength = row.columns[i].Length;
			if (BitOfLength > 12 || BitOfLength > 13){
				row.columns[i].Length = ((BitOfLength % 2) == 0 ? ((BitOfLength - 12) / 2) : ((BitOfLength - 13) / 2));
			}
			row.columns[i].Data = new BYTE[row.columns[i].Length + 1];

			StartOffset -= row.columns[i].Length;

			memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
			row.columns[i].Data[row.columns[i].Length] = NULL;

// 			if (CharSetType != 2)
// 				row.columns[i].isDamaged = !utfchecker.CheckString(row.columns[i].Data, row.columns[i].Length);
// 			else
// 				row.columns[i].isDamaged = FALSE;

			RecordSize += row.columns[i].Length;
		}
		else if (Length == 0){
			row.columns[i].Length = 0;
			row.columns[i].isDamaged = FALSE;
		}
		/*else if (StartOffset < EndOffset){
		row.columns[i].Length = 0;
		row.columns[i].isDamaged = TRUE;
		}*/


	}

	INT SizeOfFirstColumn = StartOffset - EndOffset;

	if (SizeOfFirstColumn <= 0){	// 복구 불가

		// 첫 번째 컬럼이 덮어써졌는지 다시한번 확인하고 만약 int형에 맞는 컬럼 타입이라면 거기서 구해와. 보통 8/9 아니면 없는거.

		COLUMN unknownColumn;
		unknownColumn.Length = 0;
		unknownColumn.isDamaged = FALSE;
		row.columns.insert(row.columns.begin(), unknownColumn);


	}
	else{
		COLUMN unknownColumn;
		unknownColumn.Length = SizeOfFirstColumn;
		unknownColumn.Data = new BYTE[SizeOfFirstColumn + 1];
		unknownColumn.isDamaged = FALSE;
		memcpy(unknownColumn.Data, &Page.Page[EndOffset], SizeOfFirstColumn);
		row.columns.insert(row.columns.begin(), unknownColumn);
		// 		row.columns[0].Length = SizeOfFirstColumn;
		// 		row.columns[0].Data = new BYTE[SizeOfFirstColumn + 1];
		// 		memcpy(row.columns[0].Data, &Page[EndOffset], SizeOfFirstColumn);
	}

	table->DeletedRecord.push_back(row);

	return RecordSize;
}


INT SQLiteRecovery::FindOverflowPage(SQLitePage Page, INT ColumnNumber, INT CurrentLength, INT RecordSize, Rows* row, SQLITE_TABLE* table){

	BYTE NextP[4];
	INT StartOffset = 0;
	memcpy(NextP, &Page.Page[StartOffset], 4);
	StartOffset += 4;
	INT NextPageNumber = ut.LongIntToByte(NextP, 4);

	BOOL SplitData = TRUE;
	for (int idx = ColumnNumber; idx < table->NumberOfField; idx++){
		int Length = row->columns[idx].Length;


		while (1){


			if (Length == 8){
				row->columns[idx].Data = new BYTE[2];
				row->columns[idx].Length = 1;

				row->columns[idx].Data[0] = 0x00;
				row->columns[idx].Data[1] = NULL;

			}
			else if (Length == 9){
				row->columns[idx].Data = new BYTE[2];
				row->columns[idx].Length = 1;
				row->columns[idx].Data[0] = 0x01;
				row->columns[idx].Data[1] = NULL;

			}
			else if (StartOffset < PageSize && 1 <= Length && Length <= 4){
				row->columns[idx].Data = new BYTE[row->columns[idx].Length + 1];
				memcpy(row->columns[idx].Data, &Page.Page[StartOffset], row->columns[idx].Length);
				row->columns[idx].Data[row->columns[idx].Length] = NULL;
				StartOffset += row->columns[idx].Length;
				CurrentLength += row->columns[idx].Length;
			}
			else if (StartOffset < PageSize && Length == 5){
				row->columns[idx].Length = 6;
				row->columns[idx].Data = new BYTE[row->columns[idx].Length + 1];
				memcpy(row->columns[idx].Data, &Page.Page[StartOffset], row->columns[idx].Length);
				row->columns[idx].Data[row->columns[idx].Length] = NULL;

				StartOffset += row->columns[idx].Length;
				CurrentLength += row->columns[idx].Length;
			}
			else if (StartOffset < PageSize && (Length == 6 || Length == 7)){
				row->columns[idx].Length = 8;
				row->columns[idx].Data = new BYTE[row->columns[idx].Length + 1];
				memcpy(row->columns[idx].Data, &Page.Page[StartOffset], row->columns[idx].Length);
				row->columns[idx].Data[row->columns[idx].Length] = NULL;

				StartOffset += row->columns[idx].Length;
				CurrentLength += row->columns[idx].Length;
			}
			else if (StartOffset < PageSize && (Length >= 12 || Length >= 13)){
				INT BitOfLength = row->columns[idx].Length;
				if (BitOfLength > 12 || BitOfLength > 13){
					row->columns[idx].Length = ((BitOfLength % 2) == 0 ? ((BitOfLength - 12) / 2) : ((BitOfLength - 13) / 2));
				}

//대처				row->columns[idx].Length = RecordSize;

				//	printf("%d\n", row->columns[idx].Length);

				INT Temp= 0;
				if (SplitData)
				{
					BYTE *temp = new BYTE[CurrentLength + 1];
					memcpy(temp, row->columns[idx].Data, CurrentLength);
					delete[] row->columns[idx].Data;

					row->columns[idx].Data = new BYTE[row->columns[idx].Length + 1];

					memcpy(row->columns[idx].Data, temp, CurrentLength);
					INT Temp = CurrentLength + PageSize - 0x20;

					if (Temp <= row->columns[idx].Length){
						memcpy(&row->columns[idx].Data[CurrentLength], &Page.Page[StartOffset], PageSize - 0x20 - 4);
						StartOffset += PageSize - 0x20 -4;
						CurrentLength += (PageSize - 0x20 - 4);
					}
					else{
						memcpy(&row->columns[idx].Data[CurrentLength], &Page.Page[StartOffset], row->columns[idx].Length - CurrentLength);
						StartOffset += row->columns[idx].Length - CurrentLength;
						CurrentLength += (row->columns[idx].Length - CurrentLength);
					}
					row->columns[idx].Data[row->columns[idx].Length] = NULL;
					SplitData = FALSE;
					delete[] temp;
				}
				else
				{
					if ((NextPageNumber == 0) && (row->columns[idx].Length > (PageSize - StartOffset)))
					{
						row->columns[idx].Length = 0;
						row->columns[idx].isDamaged = TRUE;
					}
					else{
						row->columns[idx].Data = new BYTE[row->columns[idx].Length + 1];
						memcpy(row->columns[idx].Data, &Page.Page[StartOffset], row->columns[idx].Length);
						row->columns[idx].Data[row->columns[idx].Length] = NULL;
					}
					StartOffset += row->columns[idx].Length;

				}



				

				//	wprintf(L"%S", row->columns[idx].Data);

				//StartOffset += row->columns[idx].Length; //대처용




			}
			else if (Length == 0){
				row->columns[idx].Length = 0;
				row->columns[idx].isDamaged = FALSE;
			}
			else if (StartOffset >= PageSize){
				row->columns[idx].Length = 0;
				row->columns[idx].isDamaged = TRUE;
			}


			if (NextPageNumber != 0){

				SQLitePage NextPage;
				NextPage.Page = new BYTE[PageSize + 1];
				memcpy(NextPage.Page, &SQLITE_BUFFER[(NextPageNumber - 1) * PageSize], PageSize);
				FindOverflowPage(NextPage, idx, CurrentLength, RecordSize, row, table);
				delete[] NextPage.Page;
				return 0;

			}
			if (CurrentLength >= row->columns[idx].Length)
				break;
				//return 0;

		}

	}
	return 0;

}


INT SQLiteRecovery::ParsingRecordContainOverflow(SQLitePage Page, INT NextPage, INT StartOffset, INT EndOffset, INT RecordSize/* 전체 셀 사이즈*/, SQLITE_TABLE* table, BOOL overflow_e/*대처*/){

	// 페이지 사이즈 고려해서 해야됨.
	INT nRecordSize = EndOffset;
	Rows row;
	BOOL isDamaged = FALSE;
	INT EndofSize = nRecordSize + StartOffset;

	row.PageNumber = Page.PageNumber;
	row.RecordOffset = ((Page.PageNumber - 1) * PageSize) + StartOffset;
	//wprintf(L"#1 Page Offet : %d \n", row.RecordOffset);
	// 테이블 컬럼 정보 가져오고 
	for (int i = 0; i < table->NumberOfField; i++){
		INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, StartOffset, 1);
		BYTE* length = new BYTE[SizeOfLengthField + 1];
		memcpy(length, &Page.Page[StartOffset], SizeOfLengthField);
		INT ColumnByte = GetLengthOfBitPattern(length, SizeOfLengthField);

		COLUMN column;
		column.Length = ColumnByte;
		column.isDamaged = FALSE;
		row.columns.push_back(column);

		StartOffset += SizeOfLengthField;
		nRecordSize -= SizeOfLengthField;/* 헤더영역 사이즈 */
		delete[] length;
	}
	//wprintf(L"#2 Page Offet : %d \n", row.RecordOffset);

	/*
	1. Overflow 확인 => 전체길이보다 긴 컬럼의 수 있을경우 오버플로우가 난 컬럼 번호 획득.
	2. 위에 컬럼 번호부터 오버플로우


	*/


	for (int i = 0; i < table->NumberOfField; i++){

		int Length = row.columns[i].Length;

		if (Length > nRecordSize){

			if (overflow_e)
			{
				nRecordSize -= 2;
			}
			row.columns[i].Data = new BYTE[nRecordSize + 1];
			memcpy(row.columns[i].Data, &Page.Page[StartOffset], nRecordSize);
			row.columns[i].CLength = nRecordSize;
			// 			BYTE PageNumber[4];
			// 			memcpy(PageNumber, &Page.Page[StartOffset + nRecordSize], 4);
			INT OPOffset = (NextPage - 1) * PageSize;
			SQLitePage OverflowPage;
			OverflowPage.Page = new BYTE[PageSize + 1];
			memcpy(OverflowPage.Page, &SQLITE_BUFFER[OPOffset], PageSize);			// 나중에 Seek 로 바꾸기
			FindOverflowPage(OverflowPage, i, nRecordSize, CheckingRecordSize(row.columns[i].Length), &row, table);
			delete[]OverflowPage.Page;

			break;

		}
		else{
			if (Length == 8){
				row.columns[i].Data = new BYTE[2];
				row.columns[i].Length = 1;

				row.columns[i].Data[0] = 0x00;
				row.columns[i].Data[1] = NULL;

			}
			else if (Length == 9){
				row.columns[i].Data = new BYTE[2];
				row.columns[i].Length = 1;
				row.columns[i].Data[0] = 0x01;
				row.columns[i].Data[1] = NULL;

			}
			else if (StartOffset < EndofSize && 1 <= Length && Length <= 4){
				row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
				memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
				row.columns[i].Data[row.columns[i].Length] = NULL;
				StartOffset += row.columns[i].Length;
				nRecordSize -= row.columns[i].Length;
			}
			else if (StartOffset < EndofSize && Length == 5){
				row.columns[i].Length = 6;
				row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
				memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
				row.columns[i].Data[row.columns[i].Length] = NULL;

				StartOffset += row.columns[i].Length;
				nRecordSize -= row.columns[i].Length;
			}
			else if (StartOffset < EndofSize && (Length == 6 || Length == 7)){
				row.columns[i].Length = 8;
				row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
				memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
				row.columns[i].Data[row.columns[i].Length] = NULL;

				StartOffset += row.columns[i].Length;
				nRecordSize -= row.columns[i].Length;
			}
			else if (StartOffset < EndofSize && (Length >= 12 || Length >= 13)){
				INT BitOfLength = row.columns[i].Length;
				if (BitOfLength > 12 || BitOfLength > 13){
					row.columns[i].Length = ((BitOfLength % 2) == 0 ? ((BitOfLength - 12) / 2) : ((BitOfLength - 13) / 2));
				}
				row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
				memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
				row.columns[i].Data[row.columns[i].Length] = NULL;

				

				StartOffset += row.columns[i].Length;
				nRecordSize -= row.columns[i].Length;
			}
			else if (Length == 0){
				row.columns[i].Length = 0;
				row.columns[i].isDamaged = FALSE;
			}
			else if (StartOffset >= EndofSize){
				row.columns[i].Length = 0;
				row.columns[i].isDamaged = TRUE;
			}
		}

	}
	//wprintf(L"#3 Page Offet : %d \n", row.RecordOffset);
	table->NormalRecord.push_back(row);
	//wprintf(L"#4 Page Offet : %d \n", row.RecordOffset);
	return RecordSize;


}


INT SQLiteRecovery::CarvingRecordContainOverflow(SQLitePage Page, INT NextPage, INT StartOffset, INT EndOffset, INT RecordSize/* 전체 셀 사이즈*/, SQLITE_TABLE* table, BOOL overflow_e/*대처*/){

	// 페이지 사이즈 고려해서 해야됨.
	INT nRecordSize = EndOffset;
	Rows row;
	BOOL isDamaged = FALSE;
	INT EndofSize = nRecordSize + StartOffset;

	row.PageNumber = Page.PageNumber;
	row.RecordOffset = ((Page.PageNumber - 1) * PageSize) + StartOffset;
	//wprintf(L"#1 Page Offet : %d \n", row.RecordOffset);
	// 테이블 컬럼 정보 가져오고 
	for (int i = 0; i < table->NumberOfField; i++){
		INT SizeOfLengthField = GetAvailableByteByBitPattern(Page.Page, StartOffset, 1);
		BYTE* length = new BYTE[SizeOfLengthField + 1];
		memcpy(length, &Page.Page[StartOffset], SizeOfLengthField);
		INT ColumnByte = GetLengthOfBitPattern(length, SizeOfLengthField);

		COLUMN column;
		column.Length = ColumnByte;
		column.isDamaged = FALSE;
		row.columns.push_back(column);

		StartOffset += SizeOfLengthField;
		nRecordSize -= SizeOfLengthField;/* 헤더영역 사이즈 */
		delete[] length;
	}
	//wprintf(L"#2 Page Offet : %d \n", row.RecordOffset);

	/*
	1. Overflow 확인 => 전체길이보다 긴 컬럼의 수 있을경우 오버플로우가 난 컬럼 번호 획득.
	2. 위에 컬럼 번호부터 오버플로우


	*/


	for (int i = 0; i < table->NumberOfField; i++){

		int Length = row.columns[i].Length;

		if (Length > nRecordSize){

			if (overflow_e)
			{
				nRecordSize -= 2;
			}
			row.columns[i].Data = new BYTE[nRecordSize + 1];
			memcpy(row.columns[i].Data, &Page.Page[StartOffset], nRecordSize);
			row.columns[i].CLength = nRecordSize;
			// 			BYTE PageNumber[4];
			// 			memcpy(PageNumber, &Page.Page[StartOffset + nRecordSize], 4);
			INT OPOffset = (NextPage - 1) * PageSize;
			SQLitePage OverflowPage;
			OverflowPage.Page = new BYTE[PageSize + 1];
			memcpy(OverflowPage.Page, &SQLITE_BUFFER[OPOffset], PageSize);			// 나중에 Seek 로 바꾸기
			FindOverflowPage(OverflowPage, i, nRecordSize, CheckingRecordSize(row.columns[i].Length), &row, table);
			delete[]OverflowPage.Page;

			break;

		}
		else{
			if (Length == 8){
				row.columns[i].Data = new BYTE[2];
				row.columns[i].Length = 1;

				row.columns[i].Data[0] = 0x00;
				row.columns[i].Data[1] = NULL;

			}
			else if (Length == 9){
				row.columns[i].Data = new BYTE[2];
				row.columns[i].Length = 1;
				row.columns[i].Data[0] = 0x01;
				row.columns[i].Data[1] = NULL;

			}
			else if (StartOffset < EndofSize && 1 <= Length && Length <= 4){
				row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
				memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
				row.columns[i].Data[row.columns[i].Length] = NULL;
				StartOffset += row.columns[i].Length;
				nRecordSize -= row.columns[i].Length;
			}
			else if (StartOffset < EndofSize && Length == 5){
				row.columns[i].Length = 6;
				row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
				memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
				row.columns[i].Data[row.columns[i].Length] = NULL;

				StartOffset += row.columns[i].Length;
				nRecordSize -= row.columns[i].Length;
			}
			else if (StartOffset < EndofSize && (Length == 6 || Length == 7)){
				row.columns[i].Length = 8;
				row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
				memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
				row.columns[i].Data[row.columns[i].Length] = NULL;

				StartOffset += row.columns[i].Length;
				nRecordSize -= row.columns[i].Length;
			}
			else if (StartOffset < EndofSize && (Length >= 12 || Length >= 13)){
				INT BitOfLength = row.columns[i].Length;
				if (BitOfLength > 12 || BitOfLength > 13){
					row.columns[i].Length = ((BitOfLength % 2) == 0 ? ((BitOfLength - 12) / 2) : ((BitOfLength - 13) / 2));
				}
				row.columns[i].Data = new BYTE[row.columns[i].Length + 1];
				memcpy(row.columns[i].Data, &Page.Page[StartOffset], row.columns[i].Length);
				row.columns[i].Data[row.columns[i].Length] = NULL;

				

				StartOffset += row.columns[i].Length;
				nRecordSize -= row.columns[i].Length;
			}
			else if (Length == 0){
				row.columns[i].Length = 0;
				row.columns[i].isDamaged = FALSE;
			}
			else if (StartOffset >= EndofSize){
				row.columns[i].Length = 0;
				row.columns[i].isDamaged = TRUE;
			}
		}




	}
	//wprintf(L"#3 Page Offet : %d \n", row.RecordOffset);
	table->DeletedRecord.push_back(row);
	//wprintf(L"#4 Page Offet : %d \n", row.RecordOffset);
	return RecordSize;


}


INT SQLiteRecovery::CheckingRecordSize(INT Length){
	if (Length == 8){
		Length = 0;

	}
	else if (Length == 9){
		Length = 0;
	}
	else if (1 <= Length && Length <= 4){

	}
	else if (Length == 5){
		Length = 6;

	}
	else if ((Length == 6 || Length == 7)){
		Length = 8;

	}
	else if ((Length >= 12 || Length >= 13)){
		INT BitOfLength = Length;
		if (BitOfLength > 12 || BitOfLength > 13){
			Length = ((BitOfLength % 2) == 0 ? ((BitOfLength - 12) / 2) : ((BitOfLength - 13) / 2));
		}
	}
	else if (Length == 0){
		Length = 0;
	}

	return Length;
}
