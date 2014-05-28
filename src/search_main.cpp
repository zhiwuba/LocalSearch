#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <iostream>

#include "search_md5.h"
#include "search_parser.h"
#include "search_util.h"
#include "search_index.h"
#include "search_query.h"
#include "search_crawl.h"
#include "search_segment.h"

int test()
{
	int len=0;
	uchar buffer[10];
	std::vector<uint> vec1;
	vec1.push_back(2563);
	variable_byte_encode(vec1,buffer,&len);

	std::vector<uint> vec2;
	variable_byte_decode(buffer,len, vec2);

	return 0;
}

int main()
{

#if 1
	long startTime=GetTickCount();
	g_Crawl.traverse_directory("D:\\Workspace\\LocalSearch\\msvc\\Data");
	long costTime=GetTickCount()-startTime;
	printf("build index cost time : %d \n", costTime);

	startTime=GetTickCount();
	g_Index_Manager.save_index();
	costTime=GetTickCount()-startTime;
	printf("save index cost time : %d \n", costTime);
#endif

	g_Index_Query.initialize();
	bool exit_flag=false;
	while ( !exit_flag )
	{
		std::string query;
		std::cout<<"====ÇëÊäÈë²éÑ¯´Ê£º"<<std::endl;
		std::cin>>query;
		//startTime=GetTickCount();
		g_Query.query(query);
		//costTime=GetTickCount()-startTime;
		//printf("query cost time : %d \n", costTime);
	}


	return 0;
}

