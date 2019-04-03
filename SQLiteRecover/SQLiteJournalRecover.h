#include "stdafx.h"
#include "UTIL.h"
#include <map>
#include <vector>
#ifndef __SQLITEJOURNALRECOVER_H__
#define __SQLITEJOURNALRECOVER_H__

#define START_OF_JOURNAL_PAGE		0x200
#define JOURNAL_PAGE_NUMBER_SIZE	4
#define JOURNAL_PAGE_CHECKSUM_SIZE	4

struct JournalFrame
{
	BYTE PageNumber[4];
	BYTE *Page;
	BYTE CheckSum[4];
};

class SQLiteJournal{

private:
	UTIL ut;
	INT PageSize;
	std::map<INT, std::vector<JournalFrame>> journal;

public:
	
	SQLiteJournal(INT _PageSize);
	std::map<INT, std::vector<JournalFrame>> GetJournalFrameList();

	BOOL SetJournalFrameList(TCHAR* pszfilename);

};

#endif