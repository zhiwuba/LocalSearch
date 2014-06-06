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
#include "search_db.h"

struct Value 
{
	char name[30];
	int    count;
	char value[30];
};

int test()
{
	int value_size=sizeof(Value);

	Search_DB db("D:\\Workspace\\LocalSearch\\bin\\test.db", value_size,  true);
	
	for ( int i=0; i<10000000; i++ )
	{
		Value* vv=new Value;
		strcpy(vv->name, "namename");
		vv->count=i ; 
		strcpy( vv->value, "valuevalue");
		db.insert(i, vv);
		printf(" %d \n", i);
	}

	Value* vv=(Value*)new char[value_size];
	db.search(300, vv);
	printf("v %s %d  %s\n",vv->name, vv->count, vv->value);

	db.remove(100);
	db.remove(101);

	return 0;
}


int main()
{
#if 1
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

#endif

	return 0;
}

