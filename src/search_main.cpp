#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "search_md5.h"
#include "search_parser.h"
#include "search_util.h"
#include "search_inverted_index.h"
#include "search_query.h"

int main()
{
	long startTime=GetTickCount();
	Search_English_Parser parser;

	parser.Parse("H:\\Workspace\\LocalSearch\\msvc\\Data\\A_Game_of_Thrones.txt");
	g_Inverted_Index.add_doc(parser.get_document());
	parser.Parse("H:\\Workspace\\LocalSearch\\msvc\\Data\\The_English_Patient.txt");
	g_Inverted_Index.add_doc(parser.get_document());

	g_Inverted_Index.save_index();

	g_Query.query("english patient");


	long costTime=GetTickCount()-startTime;
	printf("cost time : %d \n", costTime);
	return 0;
}

