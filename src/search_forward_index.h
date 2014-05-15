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

class ForwardIndex
{
public:
	ForwardIndex(void);
	~ForwardIndex(void);
	
	int load_index();

	int add_doc(Document* doc);

	int save_index();

private:
	std::list<Document*> m_documents;

};

