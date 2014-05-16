#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "search_md5.h"
#include "search_parser.h"
#include "search_util.h"
#include "search_forward_index.h"
#include "search_inverted_index.h"

int main()
{
	long startTime=GetTickCount();
	Search_English_Parser parser;
	Search_Inverted_Index index;

	parser.Parse("D:\\Workspace\\LocalSearch\\msvc\\Data\\A_Game_of_Thrones.txt");
	index.add_doc(parser.get_document());
	parser.Parse("D:\\Workspace\\LocalSearch\\msvc\\Data\\The_English_Patient.txt");
	index.add_doc(parser.get_document());

	index.save_index();

	long costTime=GetTickCount()-startTime;
	printf("cost time : %d \n", costTime);
	return 0;
}

