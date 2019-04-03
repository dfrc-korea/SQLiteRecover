#include "stdafx.h"
#include "SQLiteWALRecover.h"

BOOL SQLiteWALRecover::SetWALFrameList(TCHAR* pszfilename){


	CFile f;
	HANDLE hFile = CreateFile(pszfilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	int file_size = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	if (!f.Open(pszfilename, CFile::modeRead))
	{
		TRACE(_T("File could not be opened %d\n"));
		exit(1);

	}
	BYTE* WAL_BUFFER = (BYTE*)malloc(file_size);
	f.Read(WAL_BUFFER, file_size);
	f.Close();


	SQLiteWALHeader walheader = GetWALHeader(WAL_BUFFER);
	INT PageSize = ut.IntToByte(walheader.PageSize, 4);

	INT FileOffset = WAL_HEADER_SIZE;
	
	while (FileOffset < file_size){

		WALFrame frame = GetWALFrame(&WAL_BUFFER[FileOffset], PageSize);
		INT PageNumber = ut.IntToByte(frame.header.SQLitePageNumber, 4);

		if (walmap.find(PageNumber) != walmap.end()){
			// 같은 페이지가 있다면 next에 넣어줘야함.
			walmap[PageNumber].push_back(frame);
		}
		else{
			std::vector<WALFrame> flist;
			flist.push_back(frame);
			walmap.insert(std::map<INT, std::vector<WALFrame>>::value_type(PageNumber, flist));
		}


		FileOffset += WAL_FRAME_HEADER_SIZE;
		FileOffset += PageSize;
	}

	free(WAL_BUFFER);

}

WALFrame SQLiteWALRecover::GetWALFrame(BYTE* buffer, INT PageSize){

	WALFrame frame;

	frame.header = GetmWALFrameHeader(buffer);
	frame.Page = new BYTE[PageSize + 1];
	memcpy(frame.Page, &buffer[WAL_FRAME_HEADER_SIZE], PageSize);

	return frame;

}

mWALFrameHeader SQLiteWALRecover::GetmWALFrameHeader(BYTE* buffer){
	mWALFrameHeader header;

	memcpy(header.SQLitePageNumber, &buffer[0], 4);
	memcpy(header.CommitRecord, &buffer[4], 4);
	memcpy(header.SALT1_COPY, &buffer[8], 4);
	memcpy(header.SALT2_COPY, &buffer[12], 4);
	memcpy(header.Checksum1, &buffer[16], 4);
	memcpy(header.Checksum2, &buffer[20], 4);


	return header;
}

SQLiteWALHeader SQLiteWALRecover::GetWALHeader(BYTE* buffer){

	SQLiteWALHeader header;

	memcpy(header.FileSignature, &buffer[0], 4);
	memcpy(header.FileFormatVersion, &buffer[4], 4);
	memcpy(header.PageSize, &buffer[8], 4);
	memcpy(header.CheckPointSequenceNumber, &buffer[12], 4);
	memcpy(header.SALT1, &buffer[16], 4);
	memcpy(header.SALT2, &buffer[20], 4);
	memcpy(header.ChecksumPart1, &buffer[24], 4);
	memcpy(header.ChecksumPart2, &buffer[28], 4);

	return header;
}

std::map<INT, std::vector<WALFrame>> SQLiteWALRecover::GetWALFrameList(){
	return walmap;
}