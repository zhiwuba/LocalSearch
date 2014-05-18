#ifndef  __SEARCH_QUERY_H__
#define __SEARCH_QUERY_H__

#include "search_util.h"

#define g_Query Search_Query::instance()
class Search_Query
{
public:
	static Search_Query& instance()
	{
		static Search_Query instance_;
		return instance_;
	}

	~Search_Query();

	int query(std::string question);

private:
	Search_Query();

	//TFIDF+CosineLaw 
	float  get_similarity();

};


#endif
