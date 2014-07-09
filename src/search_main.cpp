#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "search_md5.h"
#include "search_parser.h"
#include "search_util.h"
#include "search_index.h"
#include "search_query.h"
#include "search_crawl.h"
#include "search_segment.h"
#include "search_db.h"
#include "search_http_server.h"


int main()
{
#ifdef WIN32
	WSADATA data;
	int ret=WSAStartup(MAKEWORD(2,2) , &data);
	if ( ret!=0 )
	{
		printf("WSAStartup Error. \n");
		return -1;
	}
#endif

#if 1
	long startTime=GetTickCount();
	g_Crawl.traverse_sqllite("D:\\Git\\LocalSearch\\msvc\\apks.sqlite");
	long costTime=GetTickCount()-startTime;
	printf("build index cost time : %d \n", costTime);

	startTime=GetTickCount();
	g_Index_Manager.save_index();
	costTime=GetTickCount()-startTime;
	printf("save index cost time : %d \n", costTime);
#endif

	g_Index_Query.initialize();

	g_HttpServer.start_server(7799);

	while(true)
	{
		Sleep(1000);
	}

#ifdef WIN32
	WSACleanup();
#endif
	return 0;
}

