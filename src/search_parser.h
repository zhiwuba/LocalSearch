#pragma once

#include <string>
#include <vector>
#include <map>
#include <hash_map>
#include <list>

struct Word
{
	int  word_id;  
	std::string word;
	std::vector<int> positions;
};

struct Document
{
	int  doc_id;
	std::string doc_file_path;
	std::map<int,Word*> words;
};

class ParseDocument
{
public:
	ParseDocument(void);
	~ParseDocument(void);

	int Parse(const char* filepath);

private:
	int  get_next_word( const char* line, char* word,  int* length );
	
};

