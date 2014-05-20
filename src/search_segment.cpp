#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "search_util.h"
#include "search_segment.h"

#define SCWS_PREFIX     "/usr/local/scws"

Search_Segment::Search_Segment()
{
	init();
}

Search_Segment::~Search_Segment()
{
	if ( m_scws!=NULL )
	{
		scws_free(m_scws);
	}
}


int Search_Segment::init()
{
	m_scws=scws_new();
	if ( m_scws==NULL )
	{
		printf("scws_new error. \n");
		return -1;
	}

	std::string core_path=get_core_path();
	//std::string dict_path=core_path+"etc\\dict.utf8.xdb";
	//std::string rule_path=core_path+"etc\\rules.utf8.ini";
	//scws_set_charset(m_scws, "utf8");

	std::string dict_path=core_path+"etc\\dict.xdb";
	std::string rule_path=core_path+"etc\\rules.ini";
	scws_set_charset(m_scws, "gbk");

	int ret=scws_set_dict(m_scws, dict_path.c_str(), SCWS_XDICT_XDB);
	if ( ret!=0 )
	{
		printf("scws_set_dict error. \n");
		return -1;
	}
	scws_set_rule(m_scws, rule_path.c_str());

	return 0;
}

int Search_Segment::segment(const char* text, int length, std::vector<SegValue>& results)
{
	scws_res_t res, cur;
	scws_send_text(m_scws, text, length);
	while (res = cur = scws_get_result(m_scws))
	{
		while (cur != NULL)
		{
			//char word[kMaxWordLen]={0};
			//strncpy(word, text+cur->off, cur->len);
			std::string word;
			word.assign(text+cur->off, cur->len);
			results.push_back( std::make_pair(word, cur->off) );
			printf("WORD: %.*s/%s (IDF = %4.2f)\n", cur->len, text+cur->off, cur->attr, cur->idf);
			cur = cur->next;
		}
		scws_free_result(res);
	}
	return 0;
}




