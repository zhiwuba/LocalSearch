#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#ifdef WIN32
#include <windows.h>
#else
#endif
#include "sqlite/CppSQLite3U.h"

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
	g_Parser.parse_file(file_dir);
	g_Index_Manager.add_doc(g_Parser.get_document());

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
			g_Parser.parse_file(file_dir);
			g_Index_Manager.add_doc(g_Parser.get_document());
		}
	}
	FindClose(handle);
#endif //  WIN32

	return 0;
}

int Search_Crawl::traverse_sqllite(const char* db_path)
{
	CppSQLite3DB db;
	db.open(db_path);

	const int tables_size=5;

	const char* apks_items[]={"soft_name","soft_brief","pname", NULL};
	const char* cartoon_items[]={"name", "director", "starring", "info","other_name", NULL };
	const char* movies_items[]={"name", "director", "starring", "info", "other_name", NULL};
	const char* tv_items[]={"name", "director", "starring", "info", "other_name", NULL};
	const char* variety_items[]={ "name", "director", "starring", "info", "other_name", NULL};
	const char* music_items[]={"singer_name", "song_name", "artist", NULL};

	const char* tables[]={"apks","cartoon","movies","tv","variety", "music"};
	const char** tables_items[]={apks_items, cartoon_items, movies_items, tv_items, variety_items, music_items, NULL};

	for ( int i=1; i<tables_size; i++ )
	{
		std::string sql;
		const char** temp=tables_items[i];
		int j=0;
		while (temp[j]!=NULL)
		{
			sql+=temp[j];
			sql+=",";
			j++;
		}
		sql.pop_back();
		sql="select id,"+ sql +" from " + tables[i];
		
		CppSQLite3Query q=db.execQuery(sql.c_str());
		while(!q.eof())
		{
			std::string content;
			for ( int m=0; temp[m]!=NULL; m++)
			{
				const char* field=q.getStringField(temp[m]);
				content+=field;
				content+="   ";
			}
			char doc_id_str[100];
			sprintf(doc_id_str, "%s_%d", tables[i], q.getIntField("id"));

			printf("%s.\n", doc_id_str);
			g_Parser.parse_text(doc_id_str, content.c_str());
			g_Index_Manager.add_doc(g_Parser.get_document());
			q.nextRow();
		}
		q.finalize();
	}

	db.close();
	return 0;
}


