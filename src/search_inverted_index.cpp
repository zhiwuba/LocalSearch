#include "search_inverted_index.h"


Search_Inverted_Index::Search_Inverted_Index(void)
{

}


Search_Inverted_Index::~Search_Inverted_Index(void)
{

}


int Search_Inverted_Index::add_doc( DocumentIndex* doc )
{
	m_documents.push_back(doc);

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
	FILE* file=fopen("D:\\Workspace\\LocalSearch\\msvc\\Data\\search.index","wb");
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

/*std::list<DocumentIndex*>::iterator iter=m_documents.begin();
for ( ; iter!=m_documents.end() ; ++iter )
{
	std::map<uint,Word*>::iterator iter2=(*iter)->words.begin();
	for (; iter2!=(*iter)->words.end() ; ++iter2 )
	{
		char buffer[10240];
		uchar* buffer2=NULL;
		int  buffer2_len=0;
		variable_byte_encode( (iter2->second)->positions, &buffer2 ,&buffer2_len);
		sprintf(buffer, "%d#%u#", (*iter)->doc_id, iter2->first);
		fwrite(buffer,1, strlen(buffer), file);
		fwrite(buffer2, 1, buffer2_len, file);
		fwrite("\r\n", 1, 2, file);
		delete[] buffer2;
	}
}*/
