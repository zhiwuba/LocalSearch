#include "search_query.h"
#include "search_inverted_index.h"

#include <algorithm>
#include <iostream>
#include <set>

Search_Query::Search_Query()
{

}

Search_Query::~Search_Query()
{

}

int Search_Query::query( std::string question )
{
	std::vector<int>  words;
	words.push_back(11275516);  //2
	words.push_back(12295678);  //1
	
	/*  È¡½»¼¯  */
	bool  init=false;
	std::set<uint>  result;
	for ( int i=0; i< words.size() ;i++ )
	{
		if ( !init )
		{
			g_Inverted_Index.query_word( words[i], result );
			init=true;
		}
		else
		{
			std::set<uint>  temp;
			std::set<uint>  temp2=result;
			g_Inverted_Index.query_word(words[i], temp);
			result.clear();
			std::set_intersection(temp.begin(), temp.end(), temp2.begin(), temp2.end(), std::inserter(result, result.begin()) );
		}
	}
	
	std::set<uint>::iterator iter=result.begin();
	for ( ; iter!=result.end() ; ++iter )
	{
		std::string doc=g_DocId.get_document( *iter );
		std::cout<<doc<<std::endl;
	}
	
	return 0;
}

float Search_Query::get_similarity()
{

	return 0.0f;
}

