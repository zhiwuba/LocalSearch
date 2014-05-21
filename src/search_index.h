#ifndef  __SEARCH_INVERTED_INDEX_H__
#define  __SEARCH_INVERTED_INDEX_H__

#include <map>
#include <set>

#include "search_util.h"
#include "search_doc_word.h"

#define kWordIndexFileName "word.dat"
#define kDocFileName "doc.dat"

/*
**   临时倒排文件
**/
class Search_Index_File
{
public:
	Search_Index_File();
	virtual ~Search_Index_File();

	void set_word_index_file_path(std::string& path){ m_word_index_file_path=path;};
	void set_doc_index_file_path(std::string& path){ m_doc_index_file_path=path;};
	std::string get_word_index_file_path(){return m_word_index_file_path;};
	std::string get_doc_index_file_path(){return m_doc_index_file_path;};

	virtual int load_word_index();
	virtual int save_word_index();


	int zipper_merge(Search_Index_File* dest_index, Search_Index_File* source_index);

public:	
	struct WordData
	{
		uint       doc_count;
		uint64_t start_offset;
		uint64_t end_offset;
	};
	std::map<uint, WordData> m_word_pos;

	std::string   m_word_index_file_path;
	std::string   m_doc_index_file_path;
};



#define g_Inverted_Index  Search_Inverted_Index::instance()
class Search_Inverted_Index:public Search_Index_File
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

private:

	int build_index(DocumentIndex* doc);

	std::map<uint, WordIndex*>  m_words;  //倒排表

	std::list<DocumentIndex*>  m_documents;  //正排表
	
};

class Search_Index_File_Manager
{
public:
	Search_Index_File_Manager(){};
	~Search_Index_File_Manager(){};

	int zipper_merge(Search_Index_File* dest_index, Search_Index_File* source_index);

private:

};


#define  g_Index_Query  Search_Index::instance()
class Search_Index:public Search_Index_File
{
public:
	static Search_Index& instance()
	{
		static Search_Index _instance;
		return _instance;
	}
	~Search_Index(){};

	int initialize();

	/**
	***  查询
	**/
	int query_word(int word_id, std::set<uint>& doc_id_set);

	float get_word_IDF(int word_id);  //逆文档频率

	int get_doc_word_count(uint doc_id, uint word_id);

	int get_doc_total_word_count(uint doc_id);

private:
	Search_Index(){};

	FILE* m_doc_index_file;
};


#endif

