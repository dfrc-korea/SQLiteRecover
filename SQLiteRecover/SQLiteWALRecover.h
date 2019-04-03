#include "stdafx.h"
#include "SQLiteWALDTO.h"
#include "SQLiteDTO.h"
#include <list>
#include <vector>
#include <map>
#include "UTIL.h"
#ifndef __SQLITEWALRECOVER_H__
#define __SQLITEWALRECOVER_H__

class SQLiteWALRecover
{

public:

	std::map<INT, std::vector<WALFrame>> GetWALFrameList();

	BOOL SetWALFrameList(TCHAR* pszfilename);

private:
	

	SQLiteWALHeader GetWALHeader(BYTE* buffer);
	mWALFrameHeader GetmWALFrameHeader(BYTE* buffer);
	WALFrame GetWALFrame(BYTE* buffer, INT PageSize);

	UTIL ut;

	std::map<INT, std::vector<WALFrame>> walmap;


};

#endif