#include "stdafx.h"
#ifndef __UTF8CHECKER_H__
#define __UTF8CHECKER_H__

#define			UTF8ByteHeader			0x10


class UTF8Checker{

private:
	INT GetByteSize(BYTE str);
public:
	BOOL CheckString(BYTE* str, INT Length);
};

#endif