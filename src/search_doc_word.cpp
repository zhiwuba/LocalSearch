#include "search_doc_word.h"


Search_WordId::Search_WordId()
{

}

Search_WordId::~Search_WordId()
{

}

std::string Search_WordId::get_word( uint word_id )
{
	return m_words[word_id];
}

bool Search_WordId::add_word( uint word_id, std::string word )
{
	m_words[word_id]=word;
	return true;
}

//////////////////////////////////////////

Search_DocID::Search_DocID()
{
	m_docs_count=0;
}

Search_DocID::~Search_DocID()
{

}

std::string Search_DocID::get_doc_path( uint doc_id )
{
	std::map<uint, DocInfo*>::iterator iter= m_documents.find(doc_id);
	if ( iter!=m_documents.end() )
	{
		return iter->second->file_path;
	}
	return "";
}

bool Search_DocID::add_document( uint doc_id, std::string document, uint word_count )
{
	DocInfo* doc_info=new DocInfo();
	doc_info->file_path=document;
	doc_info->word_count=word_count;
	m_documents[doc_id]=doc_info;
	m_docs_count++;
	return true;
}

uint Search_DocID::get_doc_total_word_count( uint doc_id )
{
 	std::map<uint, DocInfo*>::iterator iter= m_documents.find(doc_id);
	if ( iter!=m_documents.end() )
	{
		return iter->second->word_count;
	}
	return 0;
}
