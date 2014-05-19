#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#ifdef WIN32
#include <windows.h>
#else
#endif

#include "search_crawl.h"
#include "search_parser.h"
#include "search_inverted_index.h"

Search_Crawl::Search_Crawl()
{

}

Search_Crawl::~Search_Crawl()
{

}

int Search_Crawl::traverse_directory( const char* directory )
{
	Search_English_Parser parser;

#ifdef  WIN32
	assert(directory!=NULL);
	char file_path[1024];
	sprintf(file_path,"%s\\*.txt", directory);

	WIN32_FIND_DATA find_data;  //记录文件信息
	HANDLE handle=::FindFirstFile(file_path,&find_data);
	if ( handle==INVALID_HANDLE_VALUE )
	{
		return -1;
	}

	parser.Parse("D:\\Workspace\\LocalSearch\\msvc\\Data\\A_Game_of_Thrones.txt");
	g_Inverted_Index.add_doc(parser.get_document());

	std::cout<<directory<<"\\"<<find_data.cFileName<<std::endl;
	while (::FindNextFile(handle, &find_data))
	{
		if ( strcmp(find_data.cFileName,".")==0||strcmp(find_data.cFileName,"..")==0 )
		{
			continue;
		}

		if ( find_data.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY )
		{
			char child_path[1024];
			sprintf(child_path,"%s\\%s", file_path, find_data.cFileName);
			traverse_directory(child_path);
		}
		else
		{
			std::cout<<directory<<"\\"<<find_data.cFileName<<std::endl;
		}
	}
	FindClose(handle);
	return 0;
#endif //  WIN32

}


