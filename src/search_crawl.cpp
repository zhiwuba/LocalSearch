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
#include "pugixml/pugixml.hpp"
#include "mysql/mysql.h"

#include "search_crawl.h"
#include "search_parser.h"
#include "search_index.h"

using namespace pugi;

Search_Crawl::Search_Crawl()
{

}

Search_Crawl::~Search_Crawl()
{

}


int Search_Crawl::initialize()
{
	int ret=load_config();
	if( ret!=0 )
	{
		printf("Search_Crawl::initialize load_config error.\n");
		return -1;
	}



	return 0;
}

int Search_Crawl::load_config()
{
	std::string config_file_path=get_core_path()+kConfigFile;
	pugi::xml_document doc;
	xml_parse_result result=doc.load_file(config_file_path.c_str());
	if ( result.status!=status_ok )
	{
		printf("load xml file error! \n");
		return -1;
	}

	const char* query_path="/dbs";
	xpath_node_set seed_set=doc.select_nodes(query_path);
	for ( pugi::xpath_node_set::const_iterator iter=seed_set.begin(); iter!=seed_set.end(); ++iter )
	{
		pugi::xml_node dnode=iter->node();
		if ( dnode )
		{
			m_db_config.db_name=dnode.attribute("name").as_string();
			m_db_config.db_type=dnode.attribute("type").as_string();
			m_db_config.db_host=dnode.attribute("host").as_string();
			m_db_config.db_port=dnode.attribute("host").as_int();
			m_db_config.db_user=dnode.attribute("host").as_string();
			m_db_config.db_password=dnode.attribute("host").as_string();

			xpath_node_set table_set=dnode.select_nodes("table");
			for ( pugi::xpath_node_set::const_iterator iter2=table_set.begin(); iter2!=table_set.end(); ++iter2 )
			{
				DB_Config::Table table;
				pugi::xml_node tnode=iter->node();
				if( tnode )
				{
					table.tb_name=tnode.attribute("name").as_string();
					table.tb_id= tnode.attribute("id").as_string();
					xpath_node_set field_set=tnode.select_nodes("field");
					for ( pugi::xpath_node_set::const_iterator iter3=field_set.begin(); iter3!=field_set.end(); ++iter3 )
					{
						pugi::xml_node fnode=iter->node();
						table.tb_fields.push_back(fnode.attribute("name").as_string());
					}
				}
				m_db_config.db_tables.push_back(table);
			}
		}
	}
	return 0;
}

int Search_Crawl::traverse_mysql()
{
	MYSQL* res=mysql_real_connect(&m_mysql, 
		m_db_config.db_host.c_str(),
		m_db_config.db_user.c_str(),
		m_db_config.db_password.c_str(),
		m_db_config.db_name.c_str(),
		m_db_config.db_port,NULL,0);
	if ( res==NULL )
	{
		printf("mysql: connect server error!! \n");
		return -1;
	}

	for (int i = 0; i < m_db_config.db_tables.size() ; i++)
	{   //遍历每个表
		std::string sql;
		for ( int j=0; j<m_db_config.db_tables[i].tb_fields.size(); j++ )
		{
			sql+=",";
			sql+=m_db_config.db_tables[i].tb_fields[i];
		}
		sql="select "+ m_db_config.db_tables[i].tb_id + sql +" from " + m_db_config.db_tables[i].tb_name;

		int ret=mysql_real_query(&m_mysql, sql.c_str(), sql.length());
		if( ret!=0 )
		{
			printf("mysql query error! \n");
			return -1;
		}

		MYSQL_ROW row;
		MYSQL_RES* result=mysql_store_result(&m_mysql);
		int field_count=mysql_num_fields(result);
		while (row=mysql_fetch_row(result))
		{
			for ( int i=0; i<field_count-1; i++ )
			{
				row[i];
			}

		}
	}


	return 0;
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






