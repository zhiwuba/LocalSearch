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
#include "search_index.h"

Search_Crawl::Search_Crawl()
{

}

Search_Crawl::~Search_Crawl()
{

}

int Search_Crawl::traverse_directory( const char* directory )
{
#ifdef  WIN32
	assert(directory!=NULL);
	char file_regex[1024]; //正则
	char file_dir[1024];     //结果
	sprintf(file_regex,"%s\\*.txt", directory);

	WIN32_FIND_DATA find_data;
	HANDLE handle=::FindFirstFile(file_regex,&find_data);
	if ( handle==INVALID_HANDLE_VALUE )
	{
		return -1;
	}

	sprintf(file_dir, "%s\\%s", directory, find_data.cFileName );
	g_Parser.Parse(file_dir);
	g_Inverted_Index.add_doc(g_Parser.get_document());

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
			sprintf(child_path,"%s\\%s\\*.txt", directory, find_data.cFileName);
			traverse_directory(child_path);
		}
		else
		{
			sprintf(file_dir, "%s\\%s", directory, find_data.cFileName );
			g_Parser.Parse(file_dir);
			g_Inverted_Index.add_doc(g_Parser.get_document());
		}
	}
	FindClose(handle);
#endif //  WIN32

	return 0;
}


