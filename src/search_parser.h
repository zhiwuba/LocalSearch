#pragma once

#include <string>
#include <vector>
#include <map>
#include <hash_map>
#include <list>

#include "search_define.h"
#include "search_doc_word.h"



class Search_Parser
{
public:
	Search_Parser(){ m_document=NULL; };
	virtual ~Search_Parser(){};

	virtual int Parse(const char* filepath)=0;
	virtual DocumentIndex* get_document(){ return m_document;};

protected:
	DocumentIndex* m_document;
};


class Search_English_Parser: public Search_Parser
{
public:
	Search_English_Parser(void){};
	virtual ~Search_English_Parser(void){};

	virtual int Parse(const char* filepath);

private:
	int  get_next_word( const char* line, char* word,  int* length );
	
};

class Search_UTF8_Parser: public Search_Parser
{
public:
	Search_UTF8_Parser(void){};
	virtual ~Search_UTF8_Parser(void){};

	virtual int Parse(const char* filepath);

private:
	int  get_next_word( const char* line, char* word,  int* length );

};

class Search_GBK_Parser: public Search_Parser
{
public:
	Search_GBK_Parser(void){};
	virtual ~Search_GBK_Parser(void){};

	virtual int Parse(const char* filepath);

private:
	int  get_next_word( const char* line, char* word,  int* length );

};