#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libscws/scws.h"
#include "libscws/xdict.h"
#include "search_segment.h"

#define SCWS_PREFIX     "/usr/local/scws"

Search_Segment::Search_Segment()
{

}

Search_Segment::~Search_Segment()
{

}

int Search_Segment::test()
{
	scws_t s;
	scws_res_t res, cur;

	char buffer[10240]={0};
	FILE* file=fopen("D:\\aa.txt", "r");
	fread(buffer,1, 10240, file);

	char* text=buffer;
	//char *text = "Hello, 我名字叫李娜我是一个中国人, 我有时买Q币来玩, 我还听说过C#语言";

	if (!(s = scws_new()))
	{
		printf("ERROR: cann't init the scws!\n");
		exit(-1);
	}

	scws_set_charset(s, "utf8");
	scws_set_dict(s, "D:\\Workspace\\LocalSearch\\msvc\\etc\\dict.utf8.xdb", SCWS_XDICT_XDB);
	scws_set_rule(s, "D:\\Workspace\\LocalSearch\\msvc\\etc\\rules.utf8.ini");

	scws_send_text(s, text, strlen(text));
	while (res = cur = scws_get_result(s))
	{
		while (cur != NULL)
		{
			printf("WORD: %.*s/%s (IDF = %4.2f)\n", cur->len, text+cur->off, cur->attr, cur->idf);
			cur = cur->next;
		}
		scws_free_result(res);
	}
	scws_free(s);

	return 0;
}

