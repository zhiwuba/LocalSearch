#pragma once

#include <string>
#include <vector>
#include <map>
#include <hash_map>
#include <list>

#include "search_define.h"
#include "search_doc_word.h"


#define g_Parser Search_Parser::instance()
class Search_Parser
{
public:
	static Search_Parser& instance()
	{
		static Search_Parser _instance;
		return _instance;
	}
	~Search_Parser(){};

	int parse_file(const char* filepath);
	int parse_text(const char* identity , const char* body);
	DocumentIndex* get_document(){ return m_document;};

protected:
	DocumentIndex* m_document;

private:
	Search_Parser(){ m_document=NULL; };
	int  get_next_word( const char* line, char* word,  int* length );

};

