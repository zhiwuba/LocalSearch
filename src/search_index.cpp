#include <math.h>
#include <assert.h>

#include "search_index.h"

/*
**  词的索引表
**  WorldID  nDocs  Offset 
**
**  文档结合的索引表
**  DocId    NHits     HitList
**/

int Search_Index_File::load_word_index()
{
	m_word_pos.clear();
	std::string index_file_path=get_core_path()+"\\"+kWordIndexFileName;
	FILE* file=fopen(index_file_path.c_str(), "rb");
	if ( file!=NULL )
	{
		char buffer[512]={0};
		while ( NULL!=fgets(buffer,512, file) )
		{
			char* p=buffer;
			uint word_id=atoi(p);

			while (*p!='#')++p;
			WordData data;
			data.doc_count=atoi(++p);
			while (*p!='#')++p;
			data.start_offset=_atoi64(++p);

			m_word_pos[word_id]=data;
		}

		fclose(file);
	}
	return 0;
}

int Search_Index_File::save_word_index()
{
	FILE* file=fopen(m_word_index_file_path.c_str(), "rb");
	if ( file!=NULL )
	{
		std::map<uint, WordData>::iterator iter=m_word_pos.begin();
		for ( ; iter!=m_word_pos.end() ;++iter )
		{
			char buffer[1024];
			sprintf(buffer,"%s#%d#%llu\n", iter->first, iter->second.doc_count,  iter->second.start_offset );
			fwrite(buffer,1, strlen(buffer), file);
		}

		fclose(file);
	}

	return 0;
}


////////////////////////////////////////////////////////
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

/*
**   保存成临时索引文件
**/
int Search_Inverted_Index::save_index()
{
	uint  cur_pos=0;
	FILE* doc_file=fopen(m_doc_index_file_path.c_str(),"wb");
	if ( doc_file!=NULL )
	{
		std::map<uint, WordIndex*>::iterator iter = m_words.begin();
		for ( ; iter!=m_words.end() ; ++iter )
		{
			uint  offset=0;
			uint  word_id=iter->first;
			WordIndex* word_index=iter->second;
			std::map<uint, Word*>::iterator iter2=word_index->documents.begin();
			for ( ; iter2!= word_index->documents.end() ; ++iter2  )
			{
				uint doc_id=iter2->first;
				char buffer[10240];
				sprintf(buffer,"%u#%u#", doc_id, iter2->second->positions.size() );

				uchar* buffer2=NULL;
				int       buffer2_len=0;
				variable_byte_encode( iter2->second->positions, &buffer2, &buffer2_len);
				
				fwrite(buffer,1, strlen(buffer), doc_file);
				fwrite(buffer2, 1, buffer2_len, doc_file);
				fwrite("\r\n", 1, 2, doc_file);
				offset=strlen(buffer)+buffer2_len+2;

				delete[] buffer2;
			}

			WordData data;
			data.doc_count=word_index->documents.size();
			data.start_offset=cur_pos;
			cur_pos+=offset;
			data.end_offset=cur_pos;
			m_word_pos[word_id]=data;
		}
		fclose(doc_file);
	}

	save_word_index();
	return 0;
}
/////////////////////////////////////////////////////////////////

/*
**  拉链法合并索引文件
**/
int Search_Index_File::zipper_merge( Search_Index_File* dest_index, Search_Index_File* src_index )
{
	dest_index->load_word_index();
	src_index->load_word_index();

	std::string temp_file_path=get_core_path()+"\\"+"temp.dat";
	FILE* dest_doc_index_file=fopen(dest_index->get_doc_index_file_path().c_str(), "rb");
	FILE* src_doc_index_file=fopen(src_index->get_doc_index_file_path().c_str(),"rb");
	FILE* temp_file=fopen(temp_file_path.c_str(), "wb");

	std::map<uint,WordData>  final_word_index;
	std::map<uint, WordData>::iterator dest_iter = dest_index->m_word_pos.begin();
	std::map<uint, WordData>::iterator src_iter = src_index->m_word_pos.begin();

#define ADD_WORD_DATA(file,len,iter) \
	char* buffer=new char[len];\
	fread(buffer,1,len,file);\
	fwrite(buffer,1,len,temp_file);\
	cur_pos+=len;\
	data.end_offset=cur_pos;\
	final_word_index[iter->first]=data;\
	delete[] buffer;

	uint64_t  cur_pos=0;
	//拉链  --|--|--|--|--|--
	while ( dest_iter!=dest_index->m_word_pos.end() &&  src_iter!=src_index->m_word_pos.end()  )
	{
		WordData data;
		data.start_offset=cur_pos;

		if ( dest_iter->first == src_iter->first )
		{
			data.doc_count=src_iter->second.doc_count+dest_iter->second.doc_count;
			/* 写文件*/
			int i=0,j=0;
			while ( i<src_iter->second.doc_count&&j<=dest_iter->second.doc_count )
			{


			}
			int len=;


			cur_pos+=len;
			data.end_offset=cur_pos;
			final_word_index[src_iter->first]=data;
			++src_iter;
			++dest_iter;
		}
		else if (  src_iter->first < dest_iter->first )
		{
			data.doc_count=src_iter->second.doc_count;
			/* 写文件 */
			int len=src_iter->second.end_offset-src_iter->second.start_offset;
			ADD_WORD_DATA(src_doc_index_file,len,src_iter);
			++src_iter;
		}
		else
		{
			data.doc_count=dest_iter->second.doc_count;
			/* 写文件 */
			int len=dest_iter->second.end_offset - dest_iter->second.start_offset;
			ADD_WORD_DATA(dest_doc_index_file,len,dest_iter);
			++dest_iter;
		}
	}

	if ( dest_iter!=dest_index->m_word_pos.end()  )
	{
		
	}

	if ( src_iter!=src_index->m_word_pos.end()  )
	{
		
	}

	fclose(dest_doc_index_file);
	fclose(src_doc_index_file);
	fclose(temp_file);

	return 0;
}

////////////////////////////////////////////////////////////////

int Search_Index::initialize()
{
	set_doc_index_file_path(get_core_path()+"\\"+kDocFileName);
	set_word_index_file_path(get_core_path()+"\\"+kWordIndexFileName);
	if ( 0!=load_word_index() )
	{
		printf("Search_Index_File::initialize load_word_index failed.\n");
		return -1;
	}

	m_doc_index_file=fopen( m_doc_index_file_path.c_str(), "rb");
	if ( m_doc_index_file==NULL )
	{
		printf("Search_Index_File::initialize open doc_index_file failed.\n");
		return -1;
	}
	
	return 0;
}

int Search_Index::query_word( int word_id, std::set<uint>& doc_id_set )
{
	doc_id_set.clear();

	std::map<uint, WordData>::iterator iter = m_word_pos.find(word_id);
	if ( iter!=m_word_pos.end() )
	{
		uint doc_count=iter->second.doc_count;
		uint64_t offset=iter->second.start_offset;

		fseek(m_doc_index_file, offset, SEEK_SET);
		char buffer[1024];
		while ( NULL!=fgets(buffer,1024, m_doc_index_file) )
		{
			uint doc_id=atoi(buffer);
			doc_id_set.insert(doc_id);
		}
	}
	return 0;
}

float Search_Index::get_word_IDF( int word_id )
{
	float count=0;
	std::map<uint, WordData>::iterator iter = m_word_pos.find(word_id);
	if ( iter!=m_word_pos.end() )
	{
		count=iter->second.doc_count;
	}

	float result=log( g_DocId.get_doc_count()/count );

	return result;
}

int Search_Index::get_doc_word_count(uint doc_id, uint word_id)
{
	std::map<uint, WordData>::iterator iter = m_word_pos.find(word_id);
	if ( iter!=m_word_pos.end() )
	{
		uint64_t offset=iter->second.start_offset;
		fseek(m_doc_index_file,offset, SEEK_SET);
		//本文档中此单词的个数
		char buffer[1024];
		while ( NULL!=fgets(buffer,1024, m_doc_index_file) )
		{
			uint id=atoi(buffer);
			if ( doc_id==id )
			{
				char* p=buffer;
				while (*p!='#')++p;
				int count=atoi(++p);
				return count;
			}
		}
	}
	
	return 0;
}

int Search_Index::get_doc_total_word_count(uint doc_id)
{
	return g_DocId.get_doc_total_word_count(doc_id);
}

