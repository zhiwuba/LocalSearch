#include "search_doc_word.h"



Search_WordId::Search_WordId()
{
#if USE_DB 
	std::string  db_path=get_core_path()+"word_info.db";
	m_word_db=new Search_DB( db_path.c_str() ,sizeof(Table) ,false);
#endif
}

Search_WordId::~Search_WordId()
{
#if USE_DB
	delete m_word_db;
#endif
}

std::string Search_WordId::get_word( uint word_id )
{
#if USE_DB
	Table table;
	strset(table.word, 0);
	m_word_db->search(word_id, (void*)&table);
	return table.word;
#else
	std::map<uint, std::string>::iterator iter=word_db_.find(word_id);
	if( iter!=word_db_.end() )
	{
		return iter->second;
	}
	return  "";
#endif
}

bool Search_WordId::add_word( uint word_id, std::string word )
{
#if USE_DB
	Table table;
	strcpy(table.word, word.c_str());
	m_word_db->insert(word_id, (void*)&table);
#else
	word_db_[word_id]=word;
#endif

	return true;
}

//////////////////////////////////////////

Search_DocID::Search_DocID()
{
	m_docs_count=0;
#if USE_DB
	std::string db_path=get_core_path()+"docs_info.db";
	m_docs_db=new Search_DB(db_path.c_str(), sizeof(Table) ,false);
#endif
}

Search_DocID::~Search_DocID()
{
#if USE_DB
	delete m_docs_db;
#endif
}

std::string Search_DocID::get_doc_path( uint doc_id )
{
#if USE_DB
	Table table;
	m_docs_db->search(doc_id, (void*)&table);
	return table.path;
#else
	std::map<uint, std::pair<std::string, uint>>::iterator iter=docs_db_.find(doc_id);
	if( iter!=docs_db_.end() )
	{
		return iter->second.first;
	}
	return "";
#endif
}

bool Search_DocID::add_document( uint doc_id, std::string file_path, uint word_count )
{
#if USE_DB
	Table table;
	table.word_count=word_count;
	strcpy(table.path, file_path.c_str());
	m_docs_db->insert(doc_id, (void*)&table );
#else
	docs_db_[doc_id]=std::make_pair(file_path, word_count);
#endif
	m_docs_count++;
	return true;
}

uint Search_DocID::get_doc_total_word_count( uint doc_id )
{
#if USE_DB
	Table table;
	m_docs_db->search(doc_id, (void*)&table);
	return table.word_count;
#else
	std::map<uint, std::pair<std::string, uint>>::iterator iter=docs_db_.find(doc_id);
	if( iter!=docs_db_.end() )
	{
		return iter->second.second;
	}
	return 0;
#endif
}
