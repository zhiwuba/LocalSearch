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
	FILE* file=fopen(m_word_index_file_path.c_str(), "rb");
	if ( file!=NULL )
	{
		char buffer[512]={0};
		while ( NULL!=fgets(buffer,512, file) )
		{
			char* p=buffer;
			uint word_id=atoi(p);

			while (*p!='#')++p;
			TermIndexItem data;
			data.doc_count=atoi(++p);
			while (*p!='#')++p;
			data.start_offset=_atoi64(++p);
			while(*p!='#')++p;
			data.end_offset=_atoi64(++p);

			m_word_pos[word_id]=data;
		}

		fclose(file);
	}
	return 0;
}

int Search_Index_File::save_word_index()
{
	FILE* file=fopen(m_word_index_file_path.c_str(), "wb");
	if ( file!=NULL )
	{
		std::map<uint, TermIndexItem>::iterator iter=m_word_pos.begin();
		for ( ; iter!=m_word_pos.end() ;++iter )
		{
			char buffer[1024];
			sprintf(buffer,"%d#%d#%llu#%llu\n", iter->first, iter->second.doc_count,  iter->second.start_offset, iter->second.end_offset );
			fwrite(buffer,1, strlen(buffer), file);
		}

		fclose(file);
	}

	return 0;
}

void Search_Index_File::set_word_index_file_path( std::string& path )
{
	m_word_index_file_path=path;
	create_file_if_nonexist(path.c_str());
}

void Search_Index_File::set_doc_index_file_path( std::string& path )
{
	m_doc_index_file_path=path;
	create_file_if_nonexist(path.c_str());
}


/*
**  拉链法合并索引文件
**/
int Search_Index_File::zipper_merge( Search_Index_File* other_index )
{
	this->load_word_index();
	other_index->load_word_index();

	std::string temp_file_path=get_core_path()+"media_file.dat";
	FILE* dest_doc_index_file=fopen(this->get_doc_index_file_path().c_str(), "rb");
	FILE* src_doc_index_file=fopen(other_index->get_doc_index_file_path().c_str(),"rb");
	FILE* temp_file=fopen(temp_file_path.c_str(), "wb");
	assert(dest_doc_index_file!=NULL&&src_doc_index_file!=NULL&&temp_file!=NULL);

	std::map<uint, TermIndexItem>  final_word_index;
	std::map<uint, TermIndexItem>::iterator dest_iter = this->m_word_pos.begin();
	std::map<uint, TermIndexItem>::iterator src_iter = other_index->m_word_pos.begin();

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
	while ( dest_iter!=this->m_word_pos.end() &&  src_iter!=other_index->m_word_pos.end()  )
	{
		TermIndexItem data;
		data.start_offset=cur_pos;

		if ( dest_iter->first == src_iter->first )
		{
			data.doc_count=src_iter->second.doc_count+dest_iter->second.doc_count;
			/* 写文件*/
			uint i=0,j=0;
			char* src_buffer=NULL;
			char* dest_buffer=NULL;
			int src_buffer_len=0;
			int dest_buffer_len=0;
			bool read_src_flag=true, read_dest_flag=true;
			while ( i<src_iter->second.doc_count&&j<=dest_iter->second.doc_count )
			{
				if ( read_src_flag )
					src_buffer_len=file_read_buffer_by_head(src_doc_index_file , &src_buffer );
				if ( read_dest_flag )
					dest_buffer_len=file_read_buffer_by_head(dest_doc_index_file, &dest_buffer);
				
				uint src_doc_id=atoi(src_buffer);
				uint dest_doc_id=atoi(dest_buffer);
				if ( src_doc_id<dest_doc_id )
				{
					fwrite(src_buffer,1, src_buffer_len, temp_file);
					read_src_flag=true;
					read_dest_flag=false;
					cur_pos+=src_buffer_len;
					i++;
				}
				else
				{
					fwrite(dest_buffer,1, dest_buffer_len, temp_file);
					read_src_flag=false;
					read_dest_flag=true;
					cur_pos+=dest_buffer_len;
					j++;
				}
			}

			while ( i<=src_iter->second.doc_count )
			{
				src_buffer_len=file_read_buffer_by_head(src_doc_index_file , &src_buffer );
				fwrite(src_buffer,1, src_buffer_len, temp_file);
				cur_pos+=src_buffer_len;
				i++;
			}

			while ( j<=dest_iter->second.doc_count )
			{
				dest_buffer_len=file_read_buffer_by_head(dest_doc_index_file , &dest_buffer );
				fwrite(dest_buffer,1, dest_buffer_len, temp_file);
				cur_pos+=dest_buffer_len;
				j++;
			}

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

	while ( dest_iter!=this->m_word_pos.end()  )
	{
		TermIndexItem data;
		data.start_offset=cur_pos;
		data.doc_count=dest_iter->second.doc_count;
		/* 写文件 */
		int len=dest_iter->second.end_offset - dest_iter->second.start_offset;
		ADD_WORD_DATA(dest_doc_index_file,len,dest_iter);
		++dest_iter;
	}

	while ( src_iter!=other_index->m_word_pos.end()  )
	{
		TermIndexItem data;
		data.start_offset=cur_pos;
		data.doc_count=src_iter->second.doc_count;
		/* 写文件 */
		int len=src_iter->second.end_offset-src_iter->second.start_offset;
		ADD_WORD_DATA(src_doc_index_file,len,src_iter);
		++src_iter;
	}

	fclose(dest_doc_index_file);
	fclose(src_doc_index_file);
	fclose(temp_file);

	this->m_word_pos=final_word_index;
	this->save_word_index();

	//删除临时倒排索引
	remove(other_index->get_word_index_file_path().c_str());
	remove(other_index->get_doc_index_file_path().c_str());
	move_file(temp_file_path.c_str(), this->get_doc_index_file_path().c_str() );

	return 0;
}



int Search_Index_File::file_read_buffer_by_head( FILE* file, char** buffer )
{
	assert(file!=NULL&&buffer!=NULL);
	int ret=0;
	fread((char*)&ret,1,4, file);
	if ( ret<0 )
	{
		printf("file_get_buffer_by_head  index file is broken -f-.\n");
		return -2;
	}

	*buffer=new char[ret];
	int len=fread(*buffer,1 , ret, file);
	if ( len!=ret )
	{
		printf("file_get_buffer_by_head  index file is broken -s-. \n");
		return -1;
	}
	return len;
}

int Search_Index_File::file_write_buffer_with_head( FILE* file, char* buffer, int length )
{
	assert(file!=NULL&&buffer!=NULL);
	char* header=(char*)&length;
	int ret=fwrite(header,1,4,file);
	if ( ret!=4 )
	{
		printf("file_write_buffer_with_head write header error. \n");
		return -1;
	}
	ret=fwrite(buffer,1,length,file);
	if ( ret!=length )
	{
		printf("file_write_buffer_with_head write body error. \n");
		return -1;
	}

	return 4+length;
}

int Search_Index_File::write_item_to_doc_index( FILE* file, uint doc_id, std::vector<uint> positions )
{
	int     array_size=positions.size();
	int     buffer_len=0;
	char* buffer=new char[array_size*4+4096];
	sprintf(buffer,"%u#%u#", doc_id, array_size );
	int     len=strlen(buffer);

	variable_byte_encode( positions, (uchar*)buffer+len, &buffer_len);
	buffer_len+=len;

	int writen_len=file_write_buffer_with_head(file,buffer, buffer_len);
	delete[] buffer;
	return writen_len;
}

int Search_Index_File::read_item_from_doc_index( FILE* file, uint& doc_id, uint& hits )
{
	char* buffer=NULL;
	int length=file_read_buffer_by_head(file, &buffer);
	if ( length<=0 )
	{
		printf("read_item_from_doc_index error. \n");
		return -1;
	}
	char* p=buffer;
	doc_id=atoi(p);

	while(*p!='#')++p;
	hits=atoi(++p);

	delete[] buffer;
	return 0;
}

int Search_Index_File::read_item_from_doc_index( FILE* file, uint& doc_id, std::vector<uint>& positions )
{
	char* buffer=NULL;
	int length=file_read_buffer_by_head(file, &buffer);
	if ( length<=0 )
	{
		printf("read_item_from_doc_index error. \n");
		return -1;
	}
	char* p=buffer;
	doc_id=atoi(p);
	
	while(*p!='#')++p;
	int nhits=atoi(++p);
	while(*p!='#')++p;
	++p;
	variable_byte_decode((uchar*)p, length-(p-buffer), positions );

	delete[] buffer;
	return 0;
}

int Search_Index_File::add_term_item( uint word_id, TermIndexItem& item )
{
	m_word_pos[word_id]=item;
	return 0;
}

int Search_Index_File::clean()
{
	m_word_pos.clear();
	
	remove(get_word_index_file_path().c_str());
	remove(get_doc_index_file_path().c_str());

	return 0;
}



////////////////////////////////////////////////////////

Search_Index_File_Manager::Search_Index_File_Manager()
{
	m_document_count=0;
	m_main_index_file=new Search_Index_File();
	m_main_index_file->set_doc_index_file_path(get_core_path()+kMainDocIndexFileName);
	m_main_index_file->set_word_index_file_path(get_core_path()+kMainWordIndexFileName);

	m_temp_index_file=new Search_Index_File();
}

Search_Index_File_Manager::~Search_Index_File_Manager()
{
	delete m_main_index_file;
	delete m_temp_index_file;
}

int Search_Index_File_Manager::add_doc( DocumentIndex* doc )
{
	m_documents.push_back(doc);
	m_document_count++;

	build_index(doc);
	if ( m_documents.size()>50 )
	{
		m_temp_index_file->set_doc_index_file_path(get_core_path()+kTempDocIndexFileName);
		m_temp_index_file->set_word_index_file_path(get_core_path()+kTempWordIndexFileName);
		save_temp_index();
		for (std::map<uint, WordIndex*>::iterator iter=m_words.begin(); iter!=m_words.end() ;++iter )
		{
			delete iter->second;
		}
		m_words.clear();
		m_documents.clear();

		m_main_index_file->zipper_merge(m_temp_index_file);
		m_temp_index_file->clean();
	}

	return 0;
}

int Search_Index_File_Manager::build_index( DocumentIndex* doc )
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

int Search_Index_File_Manager::save_index()
{
	m_temp_index_file->set_doc_index_file_path(get_core_path()+kTempDocIndexFileName);
	m_temp_index_file->set_word_index_file_path(get_core_path()+kTempWordIndexFileName);

	/* 保存临时索引 */
	save_temp_index();
	
	for (std::map<uint, WordIndex*>::iterator iter=m_words.begin(); iter!=m_words.end() ;++iter )
	{
		delete iter->second;
	}
	m_words.clear();
	m_documents.clear();

	/* 归并到主索引文件 */
	m_main_index_file->zipper_merge(m_temp_index_file);
	m_temp_index_file->clean();
	
	return 0;
}


/*
**   保存成临时索引文件
**/
int Search_Index_File_Manager::save_temp_index()
{
	/* 保存doc索引 */
	uint  cur_pos=0;
	FILE* doc_file=fopen(m_temp_index_file->get_doc_index_file_path().c_str(),"wb");
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
				uint   doc_id=iter2->first;
				offset+=m_temp_index_file->write_item_to_doc_index(doc_file, doc_id , iter2->second->positions );
			}

			TermIndexItem data;
			data.doc_count=word_index->documents.size();
			data.start_offset=cur_pos;
			cur_pos+=offset;
			data.end_offset=cur_pos;
			m_temp_index_file->add_term_item(word_id, data);
		}
		fclose(doc_file);
	}
	/*   保存单词索引   */
	m_temp_index_file->save_word_index();

	return 0;
}

////////////////////////////////////////////////////////////////

int Search_Index::initialize()
{
	set_doc_index_file_path(get_core_path()+kMainDocIndexFileName);
	set_word_index_file_path(get_core_path()+kMainWordIndexFileName);
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

	std::map<uint, TermIndexItem>::iterator iter = m_word_pos.find(word_id);
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
	std::map<uint, TermIndexItem>::iterator iter = m_word_pos.find(word_id);
	if ( iter!=m_word_pos.end() )
	{
		count=iter->second.doc_count;
	}

	float result=log( g_DocId.get_doc_count()/count );

	return result;
}

int Search_Index::get_doc_word_count(uint doc_id, uint word_id)
{
	std::map<uint, TermIndexItem>::iterator iter = m_word_pos.find(word_id);
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

