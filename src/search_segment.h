#ifndef  __SEARCH_DEVIDE_WORD_H__
#define __SEARCH_DEVIDE_WORD_H__

#include "libscws/scws.h"
#include "libscws/xdict.h"

#include "search_define.h"

typedef  std::pair<std::string, int> SegValue;

#define g_Segment  Search_Segment::instance()
class Search_Segment
{
public:
	static Search_Segment& instance()
	{
		static Search_Segment _instance;
		return _instance;
	}

	~Search_Segment();

	int segment(const char* text, int length, std::vector<SegValue>& results);

private:
	Search_Segment();

	int init();

	scws_t m_scws;

};




#endif