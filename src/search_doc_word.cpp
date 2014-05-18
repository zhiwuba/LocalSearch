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

}

Search_DocID::~Search_DocID()
{

}


std::string Search_DocID::get_document( uint doc_id )
{
	return m_documents[doc_id];
}

bool Search_DocID::add_document( uint doc_id, std::string document )
{
	m_documents[doc_id]=document;
	return true;
}
