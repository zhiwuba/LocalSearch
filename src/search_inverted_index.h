#ifndef  __SEARCH_INVERTED_INDEX_H__
#define  __SEARCH_INVERTED_INDEX_H__

#include <map>

#include "search_util.h"
#include "search_index_struct.h"



class Search_Inverted_Index
{
public:
	Search_Inverted_Index(void);
	~Search_Inverted_Index(void);

	int add_doc(DocumentIndex* doc);
	int save_index();
private:
	int build_index(DocumentIndex* doc);
	
	std::map<uint, WordIndex*>  m_words;  //到排表

	std::list<DocumentIndex*>  m_documents;  //正排表
};


#endif

