// SQLiteRecover.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "SQLiteRecover.h"
#include "SQLiteRecovery.h"
#include "SQLiteReader.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 유일한 응용 프로그램 개체입니다.

CWinApp theApp;

using namespace std;
int Usage()
{
// 	int length = 16;
// 	char* version = (char*)malloc(strlen(VERSION2) + 1);
// 	memset(version, NULL, strlen(VERSION2) + 1);
// 	sprintf(version, " v%s ", VERSION2);

	string strVersion = "\tSQLiteRecover Ver. 1.0 for recover deleted records in the SQLite db & WAL \n";

	printf("\n");
	printf("　-------------------------------------------------------------------------------------------------------	\n\n");
	printf(strVersion.c_str());
	printf("　-------------------------------------------------------------------------------------------------------	\n\n");
	printf("　usage : SQLiteRecover <command>	\n\n");
	printf("　commands : \n");
	printf("\t<target file path> <output path> \n\n");
	printf("\t If there is No WAL/Journal file, please input '.' \n\n");
	//	printf( "　options : \n");
	//	printf( "\t-e \t\t\t extracting normal records\n");
	//	printf( "\t-l <3~27> \t\t guessing the variable-length record, default : all\n");
	//	printf( "\t-o <csv|db> \t\t output file format, default : csv\n\n"); 
	printf("　example : \n");
	printf("\t#1 > SQLiteRecover c:\\KakaoTalk.db c:\\KakaoTalk.db-wal d:\\output	\n\n");
	// 	printf( "\t#1 > SQLiteRecover -e c:\\KakaoTalk.db d:\\output \n");
	// 	printf( "\t#2 > SQLiteRecover -o db c:\\KakaoTalk.db d:\\output \n\n");

	return 0;
}
int UsageAccount()
{
	// 	int length = 16;
	// 	char* version = (char*)malloc(strlen(VERSION2) + 1);
	// 	memset(version, NULL, strlen(VERSION2) + 1);
	// 	sprintf(version, " v%s ", VERSION2);

	string strVersion = "\tSQLiteRecover Ver. 1.0 for recover deleted records in the Fuzzy2.db \n";

	printf("\n");
	printf("　-------------------------------------------------------------------------------------------------------	\n\n");
	printf(strVersion.c_str());
	printf("\n\n　　　　　　　　　　　　			　Ver. 1.0　by Digital Forensic Center, Korea University \n");
	printf("　-------------------------------------------------------------------------------------------------------	\n\n");
	printf("　usage : SQLiteRecover <command>	\n\n");
	printf("　commands : \n");
	printf("\t<target file path> <output path> \n\n");
	//	printf( "　options : \n");
	//	printf( "\t-e \t\t\t extracting normal records\n");
	//	printf( "\t-l <3~27> \t\t guessing the variable-length record, default : all\n");
	//	printf( "\t-o <csv|db> \t\t output file format, default : csv\n\n"); 
	printf("　example : \n");
	printf("\t#1 > SQLiteRecover c:\\KakaoTalk.db d:\\output	\n\n");
	// 	printf( "\t#1 > SQLiteRecover -e c:\\KakaoTalk.db d:\\output \n");
	// 	printf( "\t#2 > SQLiteRecover -o db c:\\KakaoTalk.db d:\\output \n\n");

	return 0;
}
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// MFC를 초기화합니다. 초기화하지 못한 경우 오류를 인쇄합니다.
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: 오류 코드를 필요에 따라 수정합니다.
			_tprintf(_T("심각한 오류: MFC를 초기화하지 못했습니다.\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: 응용 프로그램의 동작은 여기에서 코딩합니다.
			setlocale(LC_ALL, "korean");
// 			SQLiteRecovery Recover(L"FUZZY2-2.db");
// 			//SQLiteRecovery Recover(L"mmssms.db");
// 			Recover.SetCSVPath(L".");
// 			
// 			Recover.RecoverSQLite();
// 			Recover.InitTableInfo();


// 			SQLiteRecovery Recover(L"mmssms.db");
// 			Recover.PageTravelRecovery();

//			Recover.RecoverWAL(L"mmssms.db-wal");
			if (argc == 4){
				CString strSrcPath;
				CString strDstPath;
				CString strWALPath;		// -journal path
				strSrcPath.Format(_T("%s"), argv[1]);
				strWALPath.Format(_T("%s"), argv[2]);
				strDstPath.Format(_T("%s"), argv[3]);


				setlocale(LC_ALL, "korean");
				SQLiteRecovery Recover(argv[1], argv[3]);

				Recover.SetCSVPath(argv[3]);

//				Recover.CollectLeafPage();
				
				if (strSrcPath.Find(L"Kakao") == -1){
					Recover.RecoverSQLite();
					
//					//Recover.PageTravelRecovery();
				}
//				Recover.RecoverSQLite(); //171024 //kakao는 삭제되면 NULL로 덮어씌우는듯

				
//				Recover.NormalSQLite();
				printf("Run\n");

				SQLReader sql(argv[1], TRUE);
				sql.SetPageSize(Recover.GetPageSize());
				INT NumberOfPage = sql.GetSplitSQLiteByPageSize();

				sql.Run();
// 
/*				if (strWALPath != ".")
				{
					//printf("WAL or Journal\n");
					Recover.InitTableInfo();
					//Recover.RecoverWAL(argv[2]);
					Recover.RecoverJournal(argv[2]);
				}*/
				if (strWALPath.Find(L"wal") != -1 || strWALPath.Find(L"WAL") != -1)
				{
					Recover.InitTableInfo();
					Recover.RecoverWAL(argv[2]);
//					Recover.RecoverJournal(argv[2]);
				}
				else if(strWALPath.Find(L"JOURNAL") != -1 || strWALPath.Find(L"journal") != -1)
				{
					Recover.InitTableInfo();
					Recover.RecoverJournal(argv[2]);
				}
//				*/
			}

			else{
				Usage();
			}


		}
	}
	else
	{
		// TODO: 오류 코드를 필요에 따라 수정합니다.
		_tprintf(_T("심각한 오류: GetModuleHandle 실패\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
