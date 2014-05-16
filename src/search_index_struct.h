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
	std::string doc_file_path;
	std::map<uint,Word*> words;  //word_id<---->Word
};


struct   WordIndex
{
	uint word_id;
	std::string word;

	std::map<uint, Word*> documents;  //doc_id <--->Word
};


#endif
