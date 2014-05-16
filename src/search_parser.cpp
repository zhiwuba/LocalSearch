#include <assert.h>

#include "search_parser.h"
#include "search_md5.h"
#include "search_util.h"
#include "search_define.h"

////////////////////////////////////////////
int Search_English_Parser::Parse( const char* filepath )
{
	char md5[33]={0};
	uint doc_id=Search_MD5::get_file_md5_code(filepath,kMaxDocID);

	m_document=new DocumentIndex();
	m_document->doc_file_path=filepath;
	m_document->doc_id=doc_id;

	FILE* file=fopen(filepath,"r");
	if ( file!=NULL )
	{
		int  file_read_pos=0;
		while ( !feof(file) )
		{
			int  line_pos=0, buffer_len=0;
			char cword[100]={0};
			char buffer[1024]={0};
			if ( NULL!=fgets(buffer,1024,file))
			{
				buffer_len=strlen(buffer);
				while (line_pos<buffer_len)
				{
					_strset(cword,0);
					int word_len=0;
					int ptr_move_len=get_next_word(buffer+line_pos, cword ,&word_len);
					line_pos+=ptr_move_len;
					file_read_pos+=ptr_move_len;
					if ( word_len >3 )
					{
						string_to_lower(cword);
						uint word_id=Search_MD5::get_buffer_md5_code(cword, word_len, kMaxWordID);
						std::map<uint,Word*>::iterator iter2=m_document->words.find(word_id);
						if ( iter2!=m_document->words.end() )
						{
							if ( iter2->second->word.compare(cword)!=0 )
							{
								printf("hash fighting. %s -- %s \n", (iter2->second)->word.c_str(), cword);
							}
							iter2->second->positions.push_back(file_read_pos);
						}
						else
						{
							Word* word=new Word;
							word->word_id=word_id;
							word->word=cword;
							word->positions.push_back(file_read_pos);
							m_document->words[word_id]=word;
						}
					}
				}
			}
		}
		fclose(file);
	}

	return 0;
}

int Search_English_Parser::get_next_word( const char* line, char* word,  int* length )
{
	const char* p=line;
	while (!is_alpha_char(*p)&&*p!='\n')++p;
	const char* begin=p;

	while (is_alpha_char(*p)&&*p!='\n')++p;
	const char* end=p;

	*length=end-begin;
	if ( *length>0 )
	{
		strncpy(word, begin, *length);
	}

	return end-line+1;
}

/////////////////////////////////////////////////////////
int Search_UTF8_Parser::Parse( const char* filepath )
{
	return 0;
}

int Search_UTF8_Parser::get_next_word( const char* line, char* word, int* length )
{
	return 0;
}

////////////////////////////////////////////////////////
int Search_GBK_Parser::Parse( const char* filepath )
{
	return 0;
}

int Search_GBK_Parser::get_next_word( const char* line, char* word, int* length )
{
	return 0;
}
