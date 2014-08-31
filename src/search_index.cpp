#include <math.h>
#include <assert.h>
#include <algorithm>
#include "search_porting.h"
#include "search_index.h"


/*-----------------------------------------------------------
**  term_index_struct
**  ...|word_id|doc_count|start_offset|end_offset|... 
**----------------------------------------------------------
**  doc_index_struct
**  ...|size|doc_list|size|word_count_in_everydoc|size|offset_of_every_doc_in_position_file|...
**        l_____i         l___________i                            l____________________i                               
**-----------------------------------------------------------
** position_index_struct 
** ...|size|position_list |...
**      l________i
*-------------------------------------------------------------*/

void Search_Index_File::set_index_file_path( std::string path, std::string prefix )
{
	m_word_index_file_path=path+prefix+kWordIndexFile;
	create_file_if_nonexist(m_word_index_file_path.c_str());

	m_doc_index_file_path=path+prefix+kDocIndexFile;
	create_file_if_nonexist(m_doc_index_file_path.c_str());

	m_position_file_path=path+prefix+kPosIndexFile;
	create_file_if_nonexist(m_position_file_path.c_str());
}

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
			data.start_offset=atoi64(++p);
			while(*p!='#')++p;
			data.end_offset=atoi64(++p);

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
			int len=write_item_to_pos_index(pos_file, iter2->second->positions);
			uint   doc_id=iter2->first;
			doc_id_vec.push_back(doc_id);
			pos_offset_vec.push_back(cur_pos_p);
			doc_hits_vec.push_back(iter2->second->positions.size());
			cur_pos_p+=len;
		}

		offset+=write_item_to_doc_index(doc_file,doc_id_vec ,doc_hits_vec , pos_offset_vec );

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

	std::string temp_doc_path=get_core_path()+"middle_doc_file.dat";
	std::string temp_pos_path=get_core_path()+"middle_pos_file.dat";
	FILE* temp_doc_file=fopen(temp_doc_path.c_str(), "wb");
	FILE* temp_pos_file=fopen(temp_pos_path.c_str(), "wb");
	FILE* dest_doc_index_file=fopen(this->get_doc_index_file_path().c_str(), "rb");
	FILE* dest_pos_index_file=fopen(this->get_position_file_path().c_str(),"rb");
	FILE* src_doc_index_file=fopen(other_index->get_doc_index_file_path().c_str(),"rb");
	FILE* src_pos_index_file=fopen(other_index->get_position_file_path().c_str(),"rb");

	assert(dest_doc_index_file!=NULL&&dest_pos_index_file!=NULL&&
			src_doc_index_file!=NULL&&src_pos_index_file!=NULL&&
			temp_doc_file!=NULL&&temp_pos_file!=NULL);

	std::map<uint, TermIndexItem>  final_word_index;
	std::map<uint, TermIndexItem>::iterator dest_iter = this->m_word_pos.begin();
	std::map<uint, TermIndexItem>::iterator src_iter = other_index->m_word_pos.begin();


	uint64_t  doc_cur_pos=0;
	uint64_t  pos_cur_pos=0;
	//拉链  --|--|--|--|--|--
	while ( dest_iter!=this->m_word_pos.end() &&  src_iter!=other_index->m_word_pos.end()  )
	{
		TermIndexItem data;
		data.start_offset=doc_cur_pos;

		if ( dest_iter->first == src_iter->first )  //word_id 相等
		{
			data.doc_count=src_iter->second.doc_count+dest_iter->second.doc_count;
			/* 写文件*/
			int len=merge_pos_index(src_pos_index_file, temp_pos_file);
			int pos_cur_pos_s=pos_cur_pos; //保存
			pos_cur_pos+=len;
			len=merge_pos_index(dest_pos_index_file,temp_pos_file);
			int pos_cur_pos_d=pos_cur_pos; //保存
			pos_cur_pos+=len;

			len=merge_doc_index(src_doc_index_file, dest_doc_index_file, pos_cur_pos_s , pos_cur_pos_d, temp_doc_file);
			doc_cur_pos+=len;

			data.end_offset=doc_cur_pos;
			final_word_index[src_iter->first]=data;
			++src_iter;
			++dest_iter;
		}
		else if (  src_iter->first < dest_iter->first )
		{
			data.doc_count=src_iter->second.doc_count;
			/* 写文件 */
			int pos_len=merge_pos_index(src_pos_index_file,temp_pos_file);
			int doc_len=merge_doc_index(src_doc_index_file, pos_cur_pos, temp_doc_file );
			pos_cur_pos+=pos_len;
			doc_cur_pos+=doc_len;
			data.end_offset=doc_cur_pos;
			final_word_index[src_iter->first]=data;

			++src_iter;
		}
		else
		{
			data.doc_count=dest_iter->second.doc_count;
			/* 写文件 */
			int pos_len=merge_pos_index(dest_pos_index_file,temp_pos_file);
			int doc_len=merge_doc_index(dest_doc_index_file, pos_cur_pos, temp_doc_file );
			pos_cur_pos+=pos_len;
			doc_cur_pos+=doc_len;
			data.end_offset=doc_cur_pos;
			final_word_index[dest_iter->first]=data;
			
			++dest_iter;
		}
	}

	while ( dest_iter!=this->m_word_pos.end()  )
	{
		TermIndexItem data;
		data.start_offset=doc_cur_pos;
		data.doc_count=dest_iter->second.doc_count;
		/* 写文件 */

		int pos_len=merge_pos_index(dest_pos_index_file,temp_pos_file);
		int doc_len=merge_doc_index(dest_doc_index_file, pos_cur_pos, temp_doc_file );
		pos_cur_pos+=pos_len;
		doc_cur_pos+=doc_len;
		data.end_offset=doc_cur_pos;
		final_word_index[dest_iter->first]=data;

		++dest_iter;
	}

	while ( src_iter!=other_index->m_word_pos.end()  )
	{
		TermIndexItem data;
		data.start_offset=doc_cur_pos;
		data.doc_count=src_iter->second.doc_count;
		/* 写文件 */
		int pos_len=merge_pos_index(src_pos_index_file,temp_pos_file);
		int doc_len=merge_doc_index(src_doc_index_file, pos_cur_pos, temp_doc_file );
		pos_cur_pos+=pos_len;
		doc_cur_pos+=doc_len;
		data.end_offset=doc_cur_pos;
		final_word_index[src_iter->first]=data;

		++src_iter;
	}

	fclose(dest_doc_index_file);
	fclose(src_doc_index_file);
	fclose(dest_pos_index_file);
	fclose(src_pos_index_file);
	fclose(temp_doc_file);
	fclose(temp_pos_file);

	this->m_word_pos=final_word_index;
	this->save_word_index();

	//删除中间文件
	move_file(temp_doc_path.c_str(), this->get_doc_index_file_path().c_str() );
	move_file(temp_pos_path.c_str(), this->get_position_file_path().c_str());

	return 0;
}

/*
**  合并无冲突
**/
int Search_Index_File::merge_doc_index(FILE* src_file, int pos_offset, FILE* dest_file )
{   
	char* buffer1=NULL;
	int len1=file_read_buffer_by_head(src_file,&buffer1);

	char* buffer2=NULL;
	int len2=file_read_buffer_by_head(src_file,&buffer2);

	char* buffer3=NULL;
	int len3=file_read_buffer_by_head(src_file,&buffer3);

	std::vector<uint> pos_offset_vec;
	regular_byte_decode((uchar*)buffer3,len3, pos_offset_vec);

	int  begin_pos=pos_offset_vec[0];
	for ( int i=0; i<pos_offset_vec.size();++i )
	{
		pos_offset_vec[i]=pos_offset_vec[i]-begin_pos+pos_offset;
	}

	uint writen_len=0;
	writen_len=file_write_buffer_with_head(dest_file, buffer1, len1);
	writen_len+=file_write_buffer_with_head(dest_file,buffer2,len2);
	
	int len4=0;
	char* buffer4=new char[5*pos_offset_vec.size()];
	regular_byte_encode(pos_offset_vec,(uchar*)buffer4,&len4);
	writen_len+=file_write_buffer_with_head(dest_file,buffer4,len4);

	delete[] buffer1;
	delete[] buffer2;
	delete[] buffer3;
	delete[] buffer4;

	return writen_len;
}

/*
**  合并有冲突
**/
int Search_Index_File::merge_doc_index(FILE* src_file1, FILE* src_file2, int pos_offset1, int pos_offset2 , FILE* dest_file )
{  
	std::vector<uint> doc_id_vec1;
	std::vector<uint> doc_hits_vec1;
	std::vector<uint> pos_offset_vec1;
	int len1=read_item_from_doc_index(src_file1, (char)0x7, doc_id_vec1,  doc_hits_vec1, pos_offset_vec1 );

	std::vector<uint> doc_id_vec2;
	std::vector<uint> doc_hits_vec2;
	std::vector<uint> pos_offset_vec2;
	int len2=read_item_from_doc_index(src_file2, (char)0x7, doc_id_vec2,  doc_hits_vec2, pos_offset_vec2 );

	std::vector<uint> doc_id_vec3;
	std::vector<uint> doc_hits_vec3;
	std::vector<uint> pos_offset_vec3;

	int offset_begin1=pos_offset_vec1[0];
	int offset_begin2=pos_offset_vec2[0];
	int i=0, j=0;
	while ( i<doc_id_vec1.size()&&j<doc_id_vec2.size() )
	{
		if ( doc_id_vec1[i]<doc_id_vec2[j] )
		{
			doc_id_vec3.push_back(doc_id_vec1[i]);
			doc_hits_vec3.push_back(doc_hits_vec1[i]);
			pos_offset_vec3.push_back(pos_offset_vec1[i]-offset_begin1+pos_offset1);
			i++;
		}
		else
		{
			doc_id_vec3.push_back(doc_id_vec2[j]);
			doc_hits_vec3.push_back(doc_hits_vec2[j]);
			pos_offset_vec3.push_back(pos_offset_vec2[j]-offset_begin2+pos_offset2);
			j++;
		}
	}

	while ( i<doc_id_vec1.size() )
	{
		doc_id_vec3.push_back(doc_id_vec1[i]);
		doc_hits_vec3.push_back(doc_hits_vec1[i]);
		pos_offset_vec3.push_back(pos_offset_vec1[i]-offset_begin1+pos_offset1);
		i++;
	}

	while( j<doc_id_vec2.size() )
	{
		doc_id_vec3.push_back(doc_id_vec2[j]);
		doc_hits_vec3.push_back(doc_hits_vec2[j]);
		pos_offset_vec3.push_back(pos_offset_vec2[j]-offset_begin2+pos_offset2);
		j++;
	}

	int len=write_item_to_doc_index(dest_file,doc_id_vec3,doc_hits_vec3,pos_offset_vec3);

	return len;
}

int Search_Index_File::merge_pos_index(FILE* src_file , FILE* dest_file)
{
	char* buffer=NULL;
	int len=file_read_buffer_by_head(src_file, &buffer);
	if ( len<0 )
	{
		printf("merge_pos_index file_read_buffer_by_head error. \n");
		return -1;
	}

	int writen_len=0;
	writen_len+=file_write_buffer_with_head(dest_file,buffer,len);
	
	delete[] buffer;
	return writen_len;
}

///////////////
int Search_Index_File::write_item_to_doc_index( FILE* file, std::vector<uint>& doc_id_vec , std::vector<uint>& doc_hits_vec, std::vector<uint>& pos_offset_vec)
{
	int     writen_len=0;
	int     doc_count=doc_hits_vec.size();
	int     buffer_len=0;
	char* buffer=new char[doc_count*5];
	variable_byte_encode( doc_id_vec, (uchar*)buffer, &buffer_len);  //仅对doc_id 压缩存储
	int len=file_write_buffer_with_head(file,buffer, buffer_len);
	if ( len<0 )
	{
		return -1;
	}
	writen_len+=len;
	
	memset(buffer,0, doc_count*5);
	regular_byte_encode( doc_hits_vec, (uchar*)buffer, &buffer_len);
	len=file_write_buffer_with_head(file,buffer, buffer_len);
	if ( len<0 )
	{
		return -1;
	}
	writen_len+=len;

	memset(buffer,0,doc_count*5);
	regular_byte_encode(pos_offset_vec, (uchar*)buffer, &buffer_len );
	len=file_write_buffer_with_head(file,buffer, buffer_len);
	if ( len<0 )
	{
		return -1;
	}
	writen_len+=len;

	delete[] buffer;
	return writen_len;
}

int Search_Index_File::read_item_from_doc_index( FILE* file, char bitmap ,std::vector<uint>& doc_id_vec , std::vector<uint>& doc_hits_vec, std::vector<uint>& pos_offset_vec )
{
#define MOVE_FILE_POINTER() \
	int len=0;\
	int l=fread((char*)&len,1,4,file);\
	if ( l!=4 ){\
		printf("read_item_from_doc_index  header broken. \n");\
		return -1;\
	}fseek(file,len, SEEK_CUR);

	char* buffer=NULL;
	int buffer_len=0;
	if ( ((int)(bitmap&0x1))==1 )
	{	//读取doc_id_vec
		buffer_len=file_read_buffer_by_head(file, &buffer);
		if ( buffer_len<0)
		{
			printf("read_item_from_doc_index read doc_id_list error. \n");
			return -1;
		}
		variable_byte_decode((uchar*)buffer,buffer_len,doc_id_vec);
		delete[] buffer; buffer=NULL;
	}
	else if ( (int)(bitmap>>1)>0 )
	{
		MOVE_FILE_POINTER();
	}

	if ( (int)((bitmap>>1)&0x1)==1 )
	{	//读取doc_hits_vec
		buffer_len=file_read_buffer_by_head(file,&buffer);
		if ( buffer_len<0 )
		{
			printf("read_item_from_doc_index read doc_hits_list error. \n");
			return -1;
		}
		regular_byte_decode((uchar*)buffer, buffer_len,doc_hits_vec);
		delete[] buffer; buffer=NULL;
	}
	else if ( (int)(bitmap>>2)>0 )
	{
		MOVE_FILE_POINTER();
	}

	if ( (int)((bitmap>>2)&0x1)==1 )
	{	//读取pos_offset_vec
		buffer_len=file_read_buffer_by_head(file,&buffer);
		if ( buffer_len<0 )
		{
			printf("read_item_from_doc_index read pos_offset_vec .\n");
			return -1;
		}
		regular_byte_decode((uchar*)buffer,buffer_len,pos_offset_vec);
		delete[] buffer; buffer=NULL;
	}
	else if ( (int)(bitmap>>3)>0 )
	{
		MOVE_FILE_POINTER();
	}
	return 0;
}

/////////////
int Search_Index_File::write_item_to_pos_index(FILE* file, std::vector<uint> poss)
{
	int buffer_len=0;
	char* buffer=new char[poss.size()*5];
	variable_byte_encode( poss, (uchar*)buffer, &buffer_len);
	int len=file_write_buffer_with_head(file, buffer,buffer_len);
	delete[] buffer;
	return len;
}

int Search_Index_File::read_item_from_pos_index(FILE* file, std::vector<uint>& poss)
{
	int buffer_len=0;
	int ret=fread((char*)&buffer_len,1,4, file);
	if ( ret!=4 )
	{
		printf("read_item_from_pos_index header error. \n");
		return -1;
	}
	char* buffer=new char[buffer_len];
	ret=fread(buffer,1,buffer_len,file);
	if ( ret!=buffer_len )
	{
		printf("read_item_from_pos_index body error. \n");
		return -1;
	}

	variable_byte_decode((uchar*)buffer,buffer_len,poss);
	delete[] buffer;

	return 0;
}

//////////////////////
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

int Search_Index_File::file_quick_read_special_bits( FILE* file, int offset ,char* buffer, int length )
{
	assert(file!=NULL&&buffer!=NULL);
	fseek(file ,offset+4 ,SEEK_CUR); 
	
	int ret=fread(buffer,1, length, file);
	return ret;
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
	truncate_file(get_position_file_path().c_str());

	return 0;
}

////////////////////////////////////////////////////////

Search_Index_File_Manager::Search_Index_File_Manager()
{
	m_doc_count=0;
	m_main_index_file=new Search_Index_File();
	m_main_index_file->set_index_file_path(get_core_path() , "main_" );

	m_temp_index_file=new Search_Index_File();
	m_temp_index_file->set_index_file_path(get_core_path() , "temp_");
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

	if (m_doc_count%200==0 )
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
	fclose(m_pos_index_file);
	fclose(m_doc_index_file);
}

int Search_Index::initialize()
{
	set_index_file_path(get_core_path() ,"main_");

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

	m_pos_index_file=fopen(m_position_file_path.c_str(),"rb");
	if ( m_pos_index_file==NULL )
	{
		printf("Search_Index_File::initialize open pos_file_path failed.\n");
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
		uint64_t start_offset=iter->second.start_offset;
		uint64_t end_offset=iter->second.end_offset;
		fseek(m_doc_index_file, start_offset, SEEK_SET);
		
		std::vector<uint> doc_id_vec;
		std::vector<uint> null;
		int buffer_len=read_item_from_doc_index(m_doc_index_file, (char)0x1, doc_id_vec, null,null );
		for ( int i=0;i<doc_id_vec.size(); ++i )
		{
			doc_id_set.insert(doc_id_vec[i]);
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

// 获取在docs中 此word的数量
int Search_Index::get_doc_word_count(uint doc_id, uint word_id)
{
	std::map<uint, TermIndexItem>::iterator iter = m_word_pos.find(word_id);
	if ( iter!=m_word_pos.end() )
	{
		uint64_t offset=iter->second.start_offset;
		fseek(m_doc_index_file,offset, SEEK_SET);
		//本文档中此单词的个数

		std::vector<uint> doc_id_vec;
		std::vector<uint> null;
		int ret=read_item_from_doc_index(m_doc_index_file,(char)0x1, doc_id_vec, null, null);
		if ( ret!=0 )
		{
			printf("get_doc_word_count  read_item_from_doc_index error. \n");
			return -1;
		}

		//doc_ids 已经排序
		std::vector<uint>::iterator iter2=std::lower_bound(doc_id_vec.begin() , doc_id_vec.end(), word_id); /*二分搜索  限定范围减少搜索的时间*/
		int index = iter2 - doc_id_vec.begin();
		
		int count=0;
		ret=file_quick_read_special_bits(m_doc_index_file, index*4, (char*)&count, 4 );
		if (  ret!=4 )
		{
			printf("get_doc_word_count  file_quick_read_special_bits error. \n ");
			return -1;
		}

		return count;
	}
	
	return -1;
}

int Search_Index::get_doc_total_word_count(uint doc_id)
{
	return g_DocId.get_doc_total_word_count(doc_id);
}

