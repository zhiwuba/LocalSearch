#ifndef  __SEARCH_INVERTED_INDEX_H__
#define  __SEARCH_INVERTED_INDEX_H__

#include <map>
#include <set>

#include "search_util.h"
#include "search_doc_word.h"

#define kWordIndexFile  "word.dat"
#define kDocIndexFile    "doc.dat"
#define kPosIndexFile    "pos.dat"


struct TermIndexItem
{
	uint       doc_count;
	uint64_t start_offset;
	uint64_t end_offset;
};

/*
**   临时倒排文件
**/
class Search_Index_File
{
public:
	Search_Index_File(){};
	virtual ~Search_Index_File(){};

	/* 索引路径设置 */
	void set_index_file_path(std::string path, std::string prefix);
	std::string get_word_index_file_path(){return m_word_index_file_path;};
	std::string get_doc_index_file_path(){return m_doc_index_file_path;};
	std::string get_position_file_path(){return m_position_file_path;};

	int load_word_index();
	int save_word_index();

	//添加 和 保存
	int add_doc(DocumentIndex* doc);
	int save_index();

	int write_item_to_doc_index( FILE* file, std::vector<uint>& doc_id_vec , std::vector<uint>& doc_hits_vec, std::vector<uint>& pos_offset_vec);
	int read_item_from_doc_index( FILE* file, uint& doc_id, std::vector<uint>& positions );
	int read_item_from_doc_index( FILE* file, uint& doc_id, uint& hits );

	int zipper_merge(Search_Index_File* other_index);

	int clean(); //清空全部

public:
	std::map<uint, WordIndex*>      m_words;  //倒排表
	std::map<uint, TermIndexItem> m_word_pos;

	std::string   m_word_index_file_path;
	std::string   m_doc_index_file_path;
	std::string   m_position_file_path;
private:
	int file_write_buffer_with_head(FILE* file, char* buffer, int length);

	int file_read_buffer_by_head(FILE* file, char** buffer);

	int write_item_to_pos_index(FILE* file, std::vector<uint> poss);
	int read_item_from_pos_index(FILE* file, std::vector<uint>& poss);
};



#define g_Index_Manager  Search_Index_File_Manager::instance()
class Search_Index_File_Manager
{
public:
	static Search_Index_File_Manager& instance()
	{
		static Search_Index_File_Manager _instance;
		return _instance;
	}

	Search_Index_File_Manager();
	~Search_Index_File_Manager();

	int add_doc(DocumentIndex* doc);
	int save_index();

private:
	Search_Index_File*  m_main_index_file; //主索引
	Search_Index_File*  m_temp_index_file; //临时索引
private:
	uint  m_doc_count;
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
	~Search_Index();

	int initialize();
	/**
	***  查询
	**/
	int query_word(int word_id, std::set<uint>& doc_id_set);

	float get_word_IDF(int word_id);  //逆文档频率

	int get_doc_word_count(uint doc_id, uint word_id);

	int get_doc_total_word_count(uint doc_id);

private:
	Search_Index();

	FILE* m_doc_index_file;
};


#endif

