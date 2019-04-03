#include "stdafx.h"


#include "SQLiteRecovery.h"



VOID SQLiteRecovery::PageTravelRecovery(){


	GetPageListFromSQLite();

	for (int i = 0; i < pages.size(); i++){


		// 1. 페이지돌면서 플래그확인
		// 2. 0d 이면 GetSchemaFromNormalRecord
		// 3. 다른거면 모두찾고
		INT data;

		
		switch (pages[i].Page[0])
		{
		case INTERNAL_PAGE:


			break;

		case LEAF_PAGE:
		{
			 data = GetSchemaFromPageNumber(i);
			  if (data == -1){

				  /*
				  
					1. 스키마 탐색
					2. 찾아야함



					1. 정상 셀 오프셋 수집
					2. 스키마 탐색 또는 스키마 수집된거로 삭제레코드 일치하는 영역 탐색(셀 오프셋 저장)
					3. 오프셋들 소팅
					4. GetNormalRecord 처럼 정상 셀 오프셋을 제외하고 복구.
				  
				  */

				  


			  }
			  else{
				  GetDeletedRecord(pages[i], &tables[data]);
			  }
			  break;
		}
			

		default:
			break;
		}

	}



}

VOID SQLiteRecovery::GetPageListFromSQLite(){

	INT SplitStart = 0;
	INT PageNumber = 1;
	while (SplitStart < FileSize){

		SQLitePage page;
		page.PageNumber = PageNumber;
		page.Page = new BYTE[PageSize + 1];
		memcpy(page.Page, &SQLITE_BUFFER[SplitStart], PageSize);

		pages.push_back(page);

		SplitStart += PageSize;
		PageNumber++;

	}

}


INT SQLiteRecovery::GetSchemaFromByteScan(SQLitePage Page){

	INT Data = -1;
	for (int i = 0; i < tables.size(); i++){

		/*

			1. 처음 0번째부터 00만있는곳까지 찾음
			2. 00만있는곳에서 데이터있는 영역까지 찾음 
			3. 거기서부터 바이트스캔하면됨.

		*/

		//Data 

	}

	return Data;
}

INT SQLiteRecovery::GetSchemaFromPageNumber(INT PageNumber){

	
	for (int i = 0; i < tables.size(); i++){

		//if (tables[i].RootPageNum == )

		for (int k = 0; k < tables[i].LeafPageNum.size(); k++){


			if (PageNumber == tables[i].LeafPageNum[k])
				return i; // 수집된 스키마~!

		}


	}

	return -1;		// 스키마 바이트 스캔으로 찾아야함
}

INT SQLiteRecovery::GetSchemaFromNormalRecord(SQLitePage Page) // Get Table index
{

	BYTE NormalCell[2];
	memcpy(NormalCell, &Page.Page[8], 2);
	INT celloffset = ut.LongIntToByte(NormalCell, 2);
	



	if (celloffset == 0)
		return -1;



	/*
	1. 노멀 헤더 다음에
	2. row id 다음에
	3. 데이터헤더영역! (길이 구하고)
	4. 길이만큼 계속 구해(헤더 떼버려)
	5. 타입 길이가 일치한다면 ?
	6. 타입끼리 비교 해서 테이블 겟겟

	*/
	INT StartOffset = celloffset;
	INT EndOffset = StartOffset;

	/*
		레코드사이즈, ROW ID, 
	*/

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

			column.Type = L"INTEGER";
		}
		else if( ColumnByte == 0){
			column.Type = L"NULL";
		}
		else if (12 <= ColumnByte && ColumnByte % 2 == 0){
			column.Type = L"BLOB";
		}
		else{
			column.Type = L"TEXT";
		}


		TargetColumn.push_back(column);
		StartOffset += SizeOfLengthField;
		delete[] length;

	}


	for (int i = 0; i < tables.size(); i++){

		//스키마 비교구간
		int k = 0;
		if (tables[i].NumberOfField == TargetColumn.size()){


			/*for ( k = 0; k < TargetColumn.size(); k++){

				if (TargetColumn[k].Type == L"NULL"){
					continue;
				}
				else if (TargetColumn[k].Type != tables[i].Column[k].Type){
					break;
				}

			}*/

			return i;

		}
		// 맨뒤에 카톡 컬럼 하나추가됨
// 		else if (tables[i].NumberOfField == TargetColumn.size() + 1){
// 
// 			return i;
// 		}

/*
		if (k == tables[i].NumberOfField)
			return i;*/
		

	}
	return -1;
}