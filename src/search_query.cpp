#include <math.h>
#include <algorithm>
#include <numeric>
#include <iostream>


#include "search_query.h"
#include "search_index.h"
#include "search_segment.h"
#include "search_md5.h"



class PairVecPlusFunc
{
public:
	int operator() (int v1, std::pair<uint,int>& v2 )
	{
		return v1+v2.second;
	}
};

class QuerySort
{
public:
	bool operator()( QueryPair& v1, QueryPair& v2 )
	{
		return v1.second>v2.second;
	}
};


Search_Query::Search_Query()
{

}

Search_Query::~Search_Query()
{

}

int Search_Query::query( std::string question )
{
	PairVec query_vec;
	build_query_vec(question, query_vec);
	
	/*  取交集  */
	bool  init=false;
	std::set<uint>  doc_list;
	for ( uint i=0; i< query_vec.size() ;i++ )
	{
		if ( !init )
		{
			g_Index_Query.query_word( query_vec[i].first, doc_list );
			init=true;
		}
		else
		{
			std::set<uint>  temp;
			std::set<uint>  temp2=doc_list;
			g_Index_Query.query_word(query_vec[i].first, temp);
			doc_list.clear();
			std::set_intersection(temp.begin(), temp.end(), temp2.begin(), temp2.end(), std::inserter(doc_list, doc_list.begin()) );
		}
	}

	std::vector<QueryPair> result_vec;
	get_similarity( query_vec ,doc_list, result_vec);
	
	for ( uint i=0; i< result_vec.size(); ++i )
	{
		std::cout<<i<<" : "<<g_DocId.get_doc_path(result_vec[i].first)<<std::endl;
	}

	return 0;
}

float Search_Query::get_similarity(PairVec& query_vec, std::set<uint>& doc_list , std::vector<QueryPair>& result_vec )
{
	//TF-IDF
	std::vector<float>  doc_TFIDF_vec;  //文档向量
	std::vector<float>  query_TFIDF_vec; //查询向量

	// 计算查询词的参数
	float query_TFIDF_vec_module=0;
	float query_word_sum=0.0f;
	query_word_sum=std::accumulate(query_vec.begin() , query_vec.end(), query_word_sum, PairVecPlusFunc());

	for ( uint i=0; i<query_vec.size(); ++i )
	{
		float TF_value=query_vec[i].second/query_word_sum;
		float IDF_value=g_Index_Query.get_word_IDF( query_vec[i].first );
		query_TFIDF_vec.push_back(TF_value*IDF_value);
		query_TFIDF_vec_module+=(query_TFIDF_vec[i]*query_TFIDF_vec[i]);
	}

	//计算各个匹配文档的参数 并计算余弦值
	std::set<uint>::iterator iter=doc_list.begin();
	for ( ; iter!=doc_list.end() ; ++iter )
	{
		uint doc_id=*iter;

		PairVec doc_vec;
		for ( uint i=0; i<query_vec.size(); ++i )
		{
			int  count=g_Index_Query.get_doc_word_count(doc_id, query_vec[i].first);
			if ( count<0 )
			{
				printf("get_similarity get_doc_word_count warnning! count is %d \n", count);
			}
			doc_vec.push_back(std::make_pair(query_vec[i].first, count ));
		}

		int doc_word_sum=g_Index_Query.get_doc_total_word_count(doc_id); //std::accumulate(doc_vec.begin(), doc_vec.end(), doc_word_sum ,PairVecPlusFunc());
		for ( uint i=0; i<doc_vec.size(); ++i )
		{
			float TF_value=doc_vec[i].second/(float)doc_word_sum;
			float IDF_value=g_Index_Query.get_word_IDF(doc_vec[i].first);
			doc_TFIDF_vec.push_back(TF_value*IDF_value);
		}

		// 余弦定理
		float  dot_product=0;
		float  doc_TFIDF_vec_module=0;
		for ( uint  i=0; i< doc_TFIDF_vec.size(); ++i )
		{
			dot_product+= (doc_TFIDF_vec[i]*query_TFIDF_vec[i]);
			doc_TFIDF_vec_module+=(doc_TFIDF_vec[i]*doc_TFIDF_vec[i]);
		}

		double cosine_power= dot_product/((double)(sqrt(doc_TFIDF_vec_module)*sqrt(query_TFIDF_vec_module)));
		QueryPair vv=std::make_pair(doc_id, cosine_power);
		result_vec.push_back(vv);
	}
	std::sort(result_vec.begin(), result_vec.end(),QuerySort());

	return 0.0f;
}


/*
**    查询词加权重  构建查询参数
**/
int  Search_Query::build_query_vec(std::string& question, PairVec& query_vec)
{ 
	std::vector<uint>  word_id_vec;
	std::vector<SegValue> word_str_vec;
	g_Segment.segment(question.c_str(), question.length(), word_str_vec);

	int len=word_str_vec.size();
	for ( int i=0; i<len; i++ )
	{
		std::string cur_word=word_str_vec[i].first;
		uint word_id=Search_MD5::get_buffer_md5_code(cur_word.c_str(), cur_word.size(), kMaxWordID);
		uint  j=0;
		bool bfind=false;
		for ( ; j<query_vec.size(); j++ )
		{
			if ( query_vec[j].first==word_id )
			{
				bfind=true;
				break;
			}
		}
		if ( bfind )
		{
			query_vec[j].second+=(len-i);
		}
		else
		{
			query_vec.push_back(std::make_pair(word_id, len-i));
		}
	}

	return 0;
}
