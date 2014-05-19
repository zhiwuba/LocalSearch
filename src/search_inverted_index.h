#ifndef  __SEARCH_INVERTED_INDEX_H__
#define  __SEARCH_INVERTED_INDEX_H__

#include <map>
#include <set>

#include "search_util.h"
#include "search_doc_word.h"

#define g_Inverted_Index  Search_Inverted_Index::instance()

class Search_Inverted_Index
{
public:
	Search_Inverted_Index(void);
	~Search_Inverted_Index(void);

	static Search_Inverted_Index &instance()
	{
		static Search_Inverted_Index index_;
		return index_;
	}

	int add_doc(DocumentIndex* doc);
	
	int save_index();

	int query_word(int word_id, std::set<uint>& doc_id_set);

	float get_word_IDF(int word_id);  //逆文档频率

	int get_doc_word_count(uint doc_id, uint word_id);

	int get_doc_total_word_count(uint doc_id);

private:

	int build_index(DocumentIndex* doc);

	std::map<uint, WordIndex*>  m_words;  //倒排表

	std::list<DocumentIndex*>  m_documents;  //正排表

	uint64_t m_docs_count;
};


#endif

