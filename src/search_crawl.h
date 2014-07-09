#ifndef  __SEARCH_CRAWL_H__
#define __SEARCH_CRAWL_H__

#define g_Crawl Search_Crawl::instance()

class Search_Crawl
{
public:
	static Search_Crawl& instance()
	{
		static Search_Crawl _instance;
		return _instance;
	}
	~Search_Crawl();

	int traverse_directory( const char* directory );
	int traverse_sqllite(const char* db_path);

private:
	Search_Crawl();
};


#endif

