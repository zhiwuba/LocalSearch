#include "search_util.h"

bool is_alpha_char( char c )
{
	if ( (c>='a'&&c<='z')||(c>='A'&&c<='Z') )
	{
		return true;
	}
	return false;
}

void string_to_lower( char* str )
{
	char* p=str;
	while ( *p!='\0' )
	{
		*p=tolower(*p);
		++p;
	}
}

void print_binary(char c)
{
	for (int i=0 ;i<8; i++ )
	{
		if ( ((int)c&0x80)==((int)0x80) )
		{
			printf("1");
		}
		else
		{
			printf("0");
		}
		c<<=1;
	}
	printf("\n");
}

void variable_byte_encode( std::vector<uint>& arrays, uchar** buffer ,int* length )
{
	*length=0;

	int array_size=arrays.size();
	*buffer=new uchar[arrays.size()*4];

	for ( int i=0; i<arrays.size(); i++ )
	{
		int len=0;
		uint t=arrays[i];
		while ( t!=0 )
		{
			t=t>>1;
			++len;
		}

		uchar bits[8]={0};
		t=arrays[i];
		len=len/7;
		for (int j=len ;j>=0 ; --j )
		{
			char c=0;
			if ( j>0 )
				c=(((t>>(7*j))<<1)|0x1);
			else
				c=((t>>(7*j))<<1);
			(*buffer)[*length]=c;
			(*length)++;
			//print_binary(c);
		}
	}
}

void variable_byte_decode( uchar* buffer, int len ,std::vector<uint>& arrays )
{
	const uchar* p=buffer;
	while ( *p!='\0' )
	{
		uint v=(uint)((*p)>>1);
		while ( (uint)((*p)&0x1)== 1  )
		{
			++p;
			v=((v<<7)|((*p)>>1));
		}
		++p;
		arrays.push_back(v);
	}
}

