#include <math.h>
#include <assert.h>

#include "search_index.h"


Search_Inverted_Index::Search_Inverted_Index(void)
{
	m_docs_count=0;
}


Search_Inverted_Index::~Search_Inverted_Index(void)
{

}


int Search_Inverted_Index::add_doc( DocumentIndex* doc )
{
	m_documents.push_back(doc);

	m_docs_count++;

	build_index(doc);

	return 0;
}

int Search_Inverted_Index::build_index( DocumentIndex* doc )
{
	uint doc_id=doc->doc_id;
	std::map<uint,Word*>::iterator iter=doc->words.begin();
	for ( ; iter!=doc->words.end() ; ++iter)
	{
		int word_id=iter->first;
		std::map<uint, WordIndex*>::iterator iter2 = m_words.find(word_id);
		if ( iter2 !=m_words.end() )
		{
			iter2->second->documents[doc_id]=iter->second;
		}
		else
		{
			WordIndex* index=new WordIndex;
			index->documents[doc_id]=iter->second;
			m_words[word_id]=index;
		}
	}

	return 0;
}


int Search_Inverted_Index::save_index()
{
	std::string index_save_path=get_core_path()+"\\search.index";
	FILE* file=fopen( index_save_path.c_str(),"wb");
	if ( file!=NULL )
	{
		std::map<uint, WordIndex*>::iterator iter = m_words.begin();
		for ( ; iter!=m_words.end() ; ++iter )
		{
			uint word_id=iter->first;

			WordIndex* word_index=iter->second;
			std::map<uint, Word*>::iterator iter2=word_index->documents.begin();
			for ( ; iter2!= word_index->documents.end() ; ++iter2  )
			{
				uint doc_id=iter2->first;
				char buffer[10240];
				uchar* buffer2=NULL;
				int  buffer2_len=0;
				variable_byte_encode( iter2->second->positions, &buffer2, &buffer2_len);
				sprintf(buffer,"%u#%u#", word_id, doc_id );

				fwrite(buffer,1, strlen(buffer), file);
				fwrite(buffer2, 1, buffer2_len, file);
				fwrite("\r\n", 1, 2, file);
				
				delete[] buffer2;
			}
		}

		fclose(file);
	}

	return 0;
}

int Search_Inverted_Index::query_word( int word_id, std::set<uint>& doc_id_set )
{
	doc_id_set.clear();
	std::map<uint, WordIndex*>::iterator iter=m_words.find(word_id);
	if ( iter!=m_words.end() )
	{
		WordIndex* word_index=iter->second;
		std::map<uint, Word*>::iterator iter2=word_index->documents.begin();
		for ( ; iter2!=word_index->documents.end() ; ++iter2 )
		{
			doc_id_set.insert(iter2->first);
		}
	}
	return 0;
}

float Search_Inverted_Index::get_word_IDF( int word_id )
{
	float count=0;
	std::map<uint, WordIndex*>::iterator iter=m_words.find(word_id);
	if ( iter!=m_words.end() )
	{
		WordIndex* word_index=iter->second;
		count=word_index->documents.size();
	}

	float result=log( m_docs_count/(count) );
	
	return result;
}

int Search_Inverted_Index::get_doc_word_count(uint doc_id, uint word_id)
{
	std::map<uint, WordIndex*>::iterator iter=m_words.find(word_id);
	if ( iter!=m_words.end() )
	{
		WordIndex* word_index=iter->second;
		std::map<uint, Word*>::iterator iter2=word_index->documents.find(doc_id);
		if ( iter2!=word_index->documents.end() )
		{
			return iter2->second->positions.size(); //本文档中此单词的个数
		}
	}
	return 0;
}

int Search_Inverted_Index::get_doc_total_word_count(uint doc_id)
{
	return g_DocId.get_doc_word_count(doc_id);
}



////////////////////////////////////////////////////////////////
/*
**  词的索引表
**  WorldID  nDocs  Offset 
**
**  文档结合的索引表
**  DocId    NHits     HitList
**/


int Search_Index_File::load_word_index()
{

}

int Search_Index_File::save_word_index()
{

}

int Search_Index_File::zipper_merge( const char* dest_index, const char* source_index )
{
	std::string temp_file_path=get_core_path()+"\\temp.dat";
	assert(dest_index!=NULL&&source_index!=NULL);
	FILE* dest_file=fopen(dest_index,"rb");
	FILE* source_file=fopen(source_index,"rb");
	FILE* temp_file=fopen(temp_file_path.c_str(), "wb");
	assert(dest_file!=NULL&&source_file!=NULL&&temp_file!=NULL);







	return 0;
}

