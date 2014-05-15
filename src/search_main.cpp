#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "search_md5.h"
#include "search_parser.h"

int main()
{
	//int code=Search_MD5::get_buffer_md5_code("then", 3, 1000*1000);
	//printf("code is %d \n", code);

	long startTime=GetTickCount();
	ParseDocument parser;
	parser.Parse("H:\\Workspace\\sqlite\\sqlite-autoconf-3080200\\sqlite3.c");
	parser.Parse("H:\\Workspace\\LocalSearch\\msvc\\A_Game_of_Thrones.txt");
	parser.Parse("H:\\Workspace\\LocalSearch\\msvc\\The_English_Patient.txt");


	long costTime=GetTickCount()-startTime;

	printf("cost time : %d \n", costTime);
	return 0;
}




