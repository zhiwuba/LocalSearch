#include "search_doc_word.h"


Search_WordId::Search_WordId()
{
	std::string  db_path=get_core_path()+"word_info.db";
	m_word_db=new Search_DB( db_path.c_str() ,sizeof(Table) ,false);
}

Search_WordId::~Search_WordId()
{
	delete m_word_db;
}

std::string Search_WordId::get_word( uint word_id )
{
	Table table;
	strset(table.word, 0);
	m_word_db->search(word_id, (void*)&table);

	return table.word;
}

bool Search_WordId::add_word( uint word_id, std::string word )
{
	Table table;
	strcpy(table.word, word.c_str());
	m_word_db->insert(word_id, (void*)&table);
	
	return true;
}

//////////////////////////////////////////

Search_DocID::Search_DocID()
{
	m_docs_count=0;
	std::string db_path=get_core_path()+"docs_info.db";
	m_docs_db=new Search_DB(db_path.c_str(), sizeof(Table) ,false);
}

Search_DocID::~Search_DocID()
{
	delete m_docs_db;
}

std::string Search_DocID::get_doc_path( uint doc_id )
{
	Table table;
	m_docs_db->search(doc_id, (void*)&table);	
	return table.path;
}

bool Search_DocID::add_document( uint doc_id, std::string file_path, uint word_count )
{
	Table table;
	table.word_count=word_count;
	strcpy(table.path, file_path.c_str());
	m_docs_db->insert(doc_id, (void*)&table );

	m_docs_count++;
	return true;
}

uint Search_DocID::get_doc_total_word_count( uint doc_id )
{
	Table table;
	m_docs_db->search(doc_id, (void*)&table);	
	return table.word_count;
}
