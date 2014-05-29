#include "search_doc_word.h"


Search_WordId::Search_WordId()
{
	std::string  db_path=get_core_path()+"word_info.db";
	//m_word_db=new Search_DB( db_path.c_str() ,false);
}

Search_WordId::~Search_WordId()
{
	//delete m_word_db;
}

std::string Search_WordId::get_word( uint word_id )
{
	//m_word_db->search();

	return m_words[word_id];
}

bool Search_WordId::add_word( uint word_id, std::string word )
{
	//m_word_db->insert(word_id);
	
	m_words[word_id]=word;
	return true;
}

//////////////////////////////////////////

Search_DocID::Search_DocID()
{
	m_docs_count=0;
	std::string db_path=get_core_path()+"docs_info.db";
	//m_docs_db=new Search_DB(db_path.c_str(), false);
}

Search_DocID::~Search_DocID()
{
	//delete m_docs_db;
}

std::string Search_DocID::get_doc_path( uint doc_id )
{
	//m_docs_db->search();
	
	std::map<uint, DocInfo*>::iterator iter= m_documents.find(doc_id);
	if ( iter!=m_documents.end() )
	{
		return iter->second->file_path;
	}
	return "";
}

bool Search_DocID::add_document( uint doc_id, std::string document, uint word_count )
{
	//m_docs_db->insert( );

	DocInfo* doc_info=new DocInfo();
	doc_info->file_path=document;
	doc_info->word_count=word_count;
	m_documents[doc_id]=doc_info;
	m_docs_count++;
	return true;
}

uint Search_DocID::get_doc_total_word_count( uint doc_id )
{
	//m_docs_db->insert();

 	std::map<uint, DocInfo*>::iterator iter= m_documents.find(doc_id);
	if ( iter!=m_documents.end() )
	{
		return iter->second->word_count;
	}
	return 0;
}
