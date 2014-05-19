#ifndef  __SEARCH_INDEX_STRUCT_H__
#define __SEARCH_INDEX_STRUCT_H__

#include <string>
#include <vector>
#include <map>
#include <hash_map>
#include <list>

#include "search_define.h"

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
	std::string doc_file_path;
	std::map<uint,Word*> words;  //word_id<---->Word
};


struct   WordIndex
{
	uint word_id;
	std::string word;

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
	bool  add_word(uint word_id, std::string word);
	std::string get_word(uint word_id);

private:
	Search_WordId();
	std::map<uint, std::string>  m_words;
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

	bool  add_document(uint doc_id, std::string document, uint word_count);
	std::string get_doc_path(uint doc_id);
	uint          get_doc_word_count(uint doc_id);
private:
	Search_DocID();

	struct DocInfo
	{
		std::string file_path;
		uint          word_count;
	};

	std::map<uint, DocInfo*> m_documents;
};

#endif
