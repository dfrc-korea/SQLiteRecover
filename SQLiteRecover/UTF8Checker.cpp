#include "stdafx.h"
#include "UTF8Checker.h"



INT UTF8Checker::GetByteSize(BYTE str){

	INT Size = 0;
	BYTE Bitmap = 0x80;
	BYTE Checker = str & Bitmap;
	while (Checker)
	{
		Bitmap = Bitmap >> 1;
		Checker = str & Bitmap;
		Size += 1;
	}

	return Size;
}


BOOL UTF8Checker::CheckString(BYTE* str, INT Length){


	INT idx = 0;
	while (idx < Length){
		
		INT Bitmap = str[idx] & 0x80;
		if (Bitmap){

			INT Size = GetByteSize(str[idx]);

			if (Size == 1){
				return FALSE;
			}

			for (int i = 1; i < Size; i++){

				if (str[idx + i] & 0x80)
				{
					//
				}
				else{
					return FALSE;
				}

			}
			//for (int)
			idx += Size;


		}
		else{
			idx++;
		}

	}

	return TRUE;
}

