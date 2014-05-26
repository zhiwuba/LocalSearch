#include <math.h>
#include <assert.h>

#include "search_index.h"

/*-----------------------------------------------------------
**  term_index_struct
**  ...|word_id|doc_count|start_offset|end_offset|... 
**----------------------------------------------------------
**  doc_index_struct
**  ...|doc_count|size|doc_list|size|word_count_in_everydoc|size|offset_of_every_doc_in_position_file|...
**                        l_____i           l___________i                            l____________________i                               
**-----------------------------------------------------------
** position_index_struct 
** ...|size|position_list |...
**      l________i
*-------------------------------------------------------------*/

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

void Search_Index_File::set_index_file_path( std::string path, std::string prefix )
{
	m_word_index_file_path=path+prefix+kWordIndexFile;
	create_file_if_nonexist(m_word_index_file_path.c_str());

	m_doc_index_file_path=path+prefix+kDocIndexFile;
	create_file_if_nonexist(m_doc_index_file_path.c_str());

	m_position_file_path=path+prefix+kPosIndexFile;
	create_file_if_nonexist(m_position_file_path.c_str());
}


int Search_Index_File::add_doc( DocumentIndex* doc )
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
**   保存索引文件
**/
int Search_Index_File::save_index()
{
	uint  cur_pos_d=0; //doc
	uint  cur_pos_p=0; //pos
	FILE* doc_file=fopen(get_doc_index_file_path().c_str(),"wb");
	FILE* pos_file=fopen(get_position_file_path().c_str(),"wb");
	if ( doc_file==NULL || pos_file==NULL )
	{
		printf("open index file error!.");
		return -1;
	}

	/* 保存 doc_index */
	std::map<uint, WordIndex*>::iterator iter = m_words.begin();
	for ( ; iter!=m_words.end() ; ++iter )
	{
		uint  offset=0;
		uint  word_id=iter->first;
		std::vector<uint> doc_hits_vec;
		std::vector<uint> pos_offset_vec;
		std::vector<uint> doc_id_vec;
		WordIndex* word_index=iter->second;
		std::map<uint, Word*>::iterator iter2=word_index->documents.begin(); /*doc_id---Word*/
		for ( ; iter2!= word_index->documents.end() ; ++iter2  )
		{
			uint   doc_id=iter2->first;
			doc_id_vec.push_back(doc_id);
			int len=write_item_to_pos_index(pos_file, iter2->second->positions);
			pos_offset_vec.push_back(cur_pos_p);
			doc_hits_vec.push_back(iter2->second->positions.size());
			cur_pos_p+=len;
		}

		offset+=write_item_to_doc_index(doc_file, doc_id , iter2->second->positions );

		TermIndexItem data;
		data.doc_count=word_index->documents.size();
		data.start_offset=cur_pos_d;
		cur_pos_d+=offset;
		data.end_offset=cur_pos_d;
		m_word_pos[word_id]=data;
	}

	fclose(doc_file);
	fclose(pos_file);

	/*   保存单词索引   */
	save_word_index();

	return 0;
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
	file_write_buffer_with_head(temp_file,buffer,len);\
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

		if ( dest_iter->first == src_iter->first )  //word_id 相等
		{
			data.doc_count=src_iter->second.doc_count+dest_iter->second.doc_count;
			/* 写文件*/
			uint i=0,j=0;
			char* src_buffer=NULL;
			char* dest_buffer=NULL;
			int src_buffer_len=0;
			int dest_buffer_len=0;
			int len=0;
			fwrite((char*)&len,1,4, temp_file);
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
					int l=file_write_buffer_with_head(temp_file,src_buffer,src_buffer_len);
					read_src_flag=true;
					read_dest_flag=false;
					cur_pos+=l;
					i++;
				}
				else
				{
					int l=file_write_buffer_with_head(temp_file,dest_buffer,dest_buffer_len);
					read_src_flag=false;
					read_dest_flag=true;
					cur_pos+=l;
					j++;
				}
			}

			while ( i<=src_iter->second.doc_count )
			{
				src_buffer_len=file_read_buffer_by_head(src_doc_index_file , &src_buffer );
				int l=file_write_buffer_with_head(temp_file,src_buffer,src_buffer_len);
				cur_pos+=l;
				i++;
			}

			while ( j<=dest_iter->second.doc_count )
			{
				dest_buffer_len=file_read_buffer_by_head(dest_doc_index_file , &dest_buffer );
				int l=file_write_buffer_with_head(temp_file,dest_buffer,dest_buffer_len);
				cur_pos+=dest_buffer_len;
				j++;
			}

			data.end_offset=cur_pos;
			fseek(temp_file,data.start_offset,SEEK_SET);
			len=data.end_offset-data.start_offset;
			fwrite((char*)&len,1,4,temp_file);  //写入buffer长度
			fseek(temp_file,data.end_offset,SEEK_SET);

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

	//删除中间文件
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

int Search_Index_File::write_item_to_doc_index( FILE* file, std::vector<uint>& doc_id_vec , std::vector<uint>& doc_hits_vec, std::vector<uint>& pos_offset_vec)
{
	int     writen_len=0;
	int     doc_count=doc_hits_vec.size();
	int     buffer_len=0;
	char* buffer=new char[doc_count*4];
	variable_byte_encode( doc_id_vec, (uchar*)buffer, &buffer_len);  //仅对doc_id 压缩存储
	int len=file_write_buffer_with_head(file,buffer, buffer_len);
	if ( len<0 )
	{
		return -1;
	}
	writen_len+=len;
	
	memset(buffer,0, doc_count*4);
	variable_byte_encode( doc_hits_vec, (uchar*)buffer, &buffer_len);
	len=file_write_buffer_with_head(file,buffer, buffer_len);
	if ( len<0 )
	{
		return -1;
	}
	writen_len+=len;

	memset(buffer,0,doc_count*4);
	variable_byte_encode(pos_offset_vec, (uchar*)buffer, &buffer_len );
	len=file_write_buffer_with_head(file,buffer, buffer_len);
	if ( len<0 )
	{
		return -1;
	}
	writen_len+=len;

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

int Search_Index_File::write_item_to_pos_index(FILE* file, std::vector<uint> poss)
{
	int buffer_len=0;
	char* buffer=new char[poss.size()*4];
	variable_byte_encode( poss, (uchar*)buffer, &buffer_len);
	int len=file_write_buffer_with_head(file, buffer,buffer_len);
	delete[] buffer;
	return len;
}

int Search_Index_File::read_item_from_pos_index(FILE* file, std::vector<uint>& poss)
{
	return 0;
}

int Search_Index_File::clean()
{
	for (std::map<uint, WordIndex*>::iterator iter=m_words.begin(); iter!=m_words.end() ;++iter )
	{
		delete iter->second;
	}
	m_word_pos.clear();
	m_words.clear();

	truncate_file(get_word_index_file_path().c_str());
	truncate_file(get_doc_index_file_path().c_str());

	return 0;
}

////////////////////////////////////////////////////////

Search_Index_File_Manager::Search_Index_File_Manager()
{
	m_doc_count=0;
	m_main_index_file=new Search_Index_File();
	m_main_index_file->set_index_file_path(get_core_path()+kMainWordIndex , get_core_path()+kMainDocIndex);

	m_temp_index_file=new Search_Index_File();
	m_temp_index_file->set_index_file_path(get_core_path()+kTempWordIndex , get_core_path()+kTempDocIndex);
}

Search_Index_File_Manager::~Search_Index_File_Manager()
{
	delete m_main_index_file;
	delete m_temp_index_file;
}

int Search_Index_File_Manager::add_doc( DocumentIndex* doc )
{
	m_temp_index_file->add_doc(doc);
	m_doc_count++;

	if ( m_doc_count%50==0 )
	{
		save_index();
	}
	return 0;
}


int Search_Index_File_Manager::save_index()
{
	/* 保存临时索引 */
	m_temp_index_file->save_index();
	
	/* 归并到主索引文件 */
	m_main_index_file->zipper_merge(m_temp_index_file);
	m_temp_index_file->clean();
	
	return 0;
}


////////////////////////////////////////////////////////////////

Search_Index::Search_Index()
{
}

Search_Index::~Search_Index()
{
	fclose(m_doc_index_file);
}

int Search_Index::initialize()
{
	set_index_file_path(get_core_path()+kMainWordIndex ,get_core_path()+kMainDocIndex);

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

// 获取在doc_id 中 word_id的数量
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

