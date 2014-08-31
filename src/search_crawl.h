#ifndef  __SEARCH_CRAWL_H__
#define __SEARCH_CRAWL_H__

#include <vector>
#include <string>
#include "search_porting.h"
#include "mysql/mysql.h"

#define g_Crawl Search_Crawl::instance()

struct DB_Config 
{
	std::string db_type;   //mysql sqlite
	std::string db_name;
	std::string db_host;
	std::string db_user;
	std::string db_password;
	int            db_port;

	struct Table
	{
		std::string tb_name;
		std::string tb_id;
		std::vector<std::string> tb_fields;
	};

	std::vector<Table> db_tables;
};

class Search_Crawl
{
public:
	static Search_Crawl& instance()
	{
		static Search_Crawl _instance;
		return _instance;
	}
	~Search_Crawl();

	int initialize();
		


private:
	Search_Crawl();

	int load_config();
	int traverse_mysql();

	MYSQL  m_mysql;
	DB_Config  m_db_config;  //暂时 只读取一个数据库
};


#endif

