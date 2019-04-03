#include "stdafx.h"
#include "SQLiteJournalRecover.h"


SQLiteJournal::SQLiteJournal(INT _PageSize){
	PageSize = _PageSize;
}


BOOL SQLiteJournal::SetJournalFrameList(TCHAR* pszfilename){
	CFile f;
	HANDLE hFile = CreateFile(pszfilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	int file_size = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	if (!f.Open(pszfilename, CFile::modeRead))
	{
		TRACE(_T("File could not be opened %d\n"));
		exit(1);

	}
	BYTE* JOURNAL_BUFFER = (BYTE*)malloc(file_size);
	f.Read(JOURNAL_BUFFER, file_size);
	f.Close();



	INT FileOffset = START_OF_JOURNAL_PAGE;

	while (FileOffset < file_size){

		JournalFrame frame;
		memcpy(frame.PageNumber, &JOURNAL_BUFFER[FileOffset], 4);

		FileOffset += JOURNAL_PAGE_NUMBER_SIZE;

		frame.Page = new BYTE[PageSize + 1];
		memcpy(frame.Page, &JOURNAL_BUFFER[FileOffset], PageSize);
		
		FileOffset += PageSize;

		memcpy(frame.CheckSum, &JOURNAL_BUFFER[FileOffset], JOURNAL_PAGE_CHECKSUM_SIZE);
		FileOffset += JOURNAL_PAGE_CHECKSUM_SIZE;
		INT PageNumber = ut.IntToByte(frame.PageNumber, 4);

		if (journal.find(PageNumber) != journal.end()){
			// 같은 페이지가 있다면 next에 넣어줘야함.
			journal[PageNumber].push_back(frame);
		}
		else{
			std::vector<JournalFrame> flist;
			flist.push_back(frame);
			journal.insert(std::map<INT, std::vector<JournalFrame>>::value_type(PageNumber, flist));
		}

	}

	free(JOURNAL_BUFFER);

}


std::map<INT, std::vector<JournalFrame>> SQLiteJournal::GetJournalFrameList(){
	return journal;
}