#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "search_md5.h"
#include "search_parser.h"
#include "search_util.h"
#include "search_inverted_index.h"
#include "search_query.h"
#include "search_crawl.h"
#include "search_segment.h"

int main()
{
	Search_Segment segment;
	segment.test();


#if 0
	long startTime=GetTickCount();
	g_Crawl.traverse_directory("D:\\Workspace\\LocalSearch\\msvc\\Data");
	long costTime=GetTickCount()-startTime;
	printf("build index cost time : %d \n", costTime);

	startTime=GetTickCount();
	g_Inverted_Index.save_index();
	costTime=GetTickCount()-startTime;
	printf("save index cost time : %d \n", costTime);

	startTime=GetTickCount();
	g_Query.query("english patient");
	costTime=GetTickCount()-startTime;
	printf("query cost time : %d \n", costTime);
#endif

	return 0;
}

