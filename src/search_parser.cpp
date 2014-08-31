#include <assert.h>

#include "search_parser.h"
#include "search_md5.h"
#include "search_util.h"
#include "search_segment.h"

////////////////////////////////////////////
int Search_Parser::parse_file( const char* filepath )
{
	char md5[33]={0};
	uint doc_id=Search_MD5::get_file_md5_code(filepath,kMaxDocID);

	FILE* file=fopen(filepath,"r");
	if ( file!=NULL )
	{
		m_document=new DocumentIndex();
		m_document->doc_path=filepath;
		m_document->doc_id=doc_id;

		uint word_count=0;
		int  file_read_pos=0;
		char* buffer=new char[1024*1024];
		while ( !feof(file) )
		{
			int line_pos=0;
			int length=fread(buffer,1,1024*1024,file);
			if ( length>0 )
			{
				std::vector<SegValue> words;
				g_Segment.segment(buffer, length, words);
				
				for ( int i=0; i<words.size(); i++ )
				{
					word_count++;
					std::string cur_word=words[i].first;
					uint word_id=Search_MD5::get_buffer_md5_code(cur_word.c_str(), cur_word.size(), kMaxWordID);
					std::map<uint,Word*>::iterator iter2=m_document->words.find(word_id);
					if ( iter2!=m_document->words.end() )
					{
						if ( iter2->second->word.compare(cur_word)!=0 )
						{
							printf("hash fighting. %s -- %s \n", (iter2->second)->word.c_str(), cur_word.c_str());
						}
						iter2->second->positions.push_back(file_read_pos+words[i].second);
					}
					else
					{
						Word* word=new Word;
						word->word_id=word_id;
						word->word=cur_word;
						word->positions.push_back(file_read_pos);
						m_document->words[word_id]=word;
						g_WordId.add_word(word_id, cur_word);
					}
				}
			}

			file_read_pos+=length;
		}
		fclose(file);
		delete[] buffer;

		g_DocId.add_document(doc_id , filepath, word_count);
	}

	return 0;
}

int Search_Parser::parse_text(const char* identity , const char* body)
{
	uint doc_id=Search_MD5::get_buffer_md5_code(identity, strlen(identity), kMaxDocID);
	
	m_document=new DocumentIndex();
	m_document->doc_path=identity;
	m_document->doc_id=doc_id;
	uint word_count=0;

	std::vector<SegValue> words;
	g_Segment.segment(body, strlen(body), words);

	for ( int i=0; i<words.size(); i++ )
	{
		word_count++;
		std::string cur_word=words[i].first;
		uint word_id=Search_MD5::get_buffer_md5_code(cur_word.c_str(), cur_word.size(), kMaxWordID);
		std::map<uint,Word*>::iterator iter2=m_document->words.find(word_id);
		if ( iter2!=m_document->words.end() )
		{
			if ( iter2->second->word.compare(cur_word)!=0 )
			{
				printf("hash fighting. %s -- %s \n", (iter2->second)->word.c_str(), cur_word.c_str());
			}
			iter2->second->positions.push_back(words[i].second);
		}
		else
		{
			Word* word=new Word;
			word->word_id=word_id;
			word->word=cur_word;
			word->positions.push_back(words[i].second);
			m_document->words[word_id]=word;
			g_WordId.add_word(word_id, cur_word);
		}
	}
	g_DocId.add_document(doc_id , identity , word_count);
	return 0;
}


int Search_Parser::get_next_word( const char* line, char* word,  int* length )
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


