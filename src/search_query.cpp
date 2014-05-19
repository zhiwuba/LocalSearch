#include "search_query.h"
#include "search_inverted_index.h"

#include <math.h>
#include <algorithm>
#include <numeric>
#include <iostream>




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
	std::vector<int>  words;
	words.push_back(11275516);  //2
	words.push_back(12295678);  //1
	
	/*  取交集  */
	bool  init=false;
	std::set<uint>  doc_list;
	for ( uint i=0; i< words.size() ;i++ )
	{
		if ( !init )
		{
			g_Inverted_Index.query_word( words[i], doc_list );
			init=true;
		}
		else
		{
			std::set<uint>  temp;
			std::set<uint>  temp2=doc_list;
			g_Inverted_Index.query_word(words[i], temp);
			doc_list.clear();
			std::set_intersection(temp.begin(), temp.end(), temp2.begin(), temp2.end(), std::inserter(doc_list, doc_list.begin()) );
		}
	}

	PairVec query_vec;
	query_vec.push_back(std::make_pair(11275516,1));
	query_vec.push_back(std::make_pair(12295678,1));

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
		float IDF_value=g_Inverted_Index.get_word_IDF( query_vec[i].first );
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
			doc_vec.push_back(std::make_pair(query_vec[i].first, g_Inverted_Index.get_doc_word_count(doc_id, query_vec[i].first) ));
		}

		float doc_word_sum=g_Inverted_Index.get_doc_total_word_count(doc_id); //std::accumulate(doc_vec.begin(), doc_vec.end(), doc_word_sum ,PairVecPlusFunc());
		for ( int i=0; i<doc_vec.size(); ++i )
		{
			float TF_value=doc_vec[i].second/doc_word_sum;
			float IDF_value=g_Inverted_Index.get_word_IDF(doc_vec[i].first);
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

