/*
**    Manage the information of words and documents. 
**    Saved in DB which using B+Tree DB .
*/

#ifndef  __SEARCH_INDEX_STRUCT_H__
#define __SEARCH_INDEX_STRUCT_H__

#include <string>
#include <vector>
#include <map>
#include <hash_map>
#include <list>

#include "search_define.h"
#include "search_db.h"

#define USE_DB 0

struct Word
{
	uint  word_id;
	std::string word;
	std::vector<uint> positions;
};

struct DocumentIndex
{
	uint  doc_id;
	uint  word_count;
	std::string doc_path;
	std::map<uint,Word*> words;  //word_id<---->Word
};


struct WordIndex
{
	uint word_id;
	std::map<uint, Word*> documents;  //doc_id <--->Word
};

#define g_WordId Search_WordId::instance()
class Search_WordId
{
public:
	static Search_WordId& instance()
	{
		static Search_WordId _instance;
		return _instance;
	}
	~Search_WordId();
	bool        add_word(uint word_id, std::string word);
	std::string get_word(uint word_id);

protected:
	struct Table 
	{
		char word[30];
	};

private:
	Search_WordId();

#if USE_DB
	Search_DB*   m_word_db;
#else
	std::map<uint, std::string> word_db_;
#endif
};

#define g_DocId Search_DocID::instance()
class Search_DocID
{
public:
	static Search_DocID& instance()
	{
		static Search_DocID instance_;
		return instance_;
	}

	~Search_DocID();

	bool  add_document(uint doc_id, std::string doc_path, uint word_count);
	std::string get_doc_path(uint doc_id);
	uint          get_doc_total_word_count(uint doc_id);
	uint64_t    get_doc_count(){return m_docs_count;};

protected:
	struct Table 
	{
		uint word_count;
		char path[256];
	};

private:
	Search_DocID();

	uint64_t       m_docs_count;

#if USE_DB
	Search_DB*  m_docs_db;
#else
	std::map<uint, std::pair<std::string, uint>> docs_db_;
#endif
};

#endif
