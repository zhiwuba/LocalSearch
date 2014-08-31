#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


#ifdef WIN32
#include <io.h>
#endif // WIN32

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

void variable_byte_encode( std::vector<uint>& arrays, uchar* buffer ,int* length )
{
	*length=0;
	for ( uint i=0; i<arrays.size(); i++ )
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
			buffer[*length]=c;
			(*length)++;
			//print_binary(c);
		}
	}
}

void variable_byte_decode( uchar* buffer, int len ,std::vector<uint>& arrays )
{
	int i=0;
	const uchar* p=buffer;
	while ( i<len )
	{
		uint v=(uint)((*p)>>1);
		while ( (uint)((*p)&0x1)== 1  )
		{
			++p;
			++i;
			v=((v<<7)|((*p)>>1));
		}
		++p;
		++i;
		arrays.push_back(v);
	}
}

void compress_data( std::vector<uint>& arrays, uchar* buffer, int* length )
{
	for ( int i=arrays.size()-1; i>=1 ; --i )
	{
		arrays[i]-=arrays[i-1];
	}
	variable_byte_encode(arrays, buffer, length);
}

void decompress_data( uchar* buffer, int len, std::vector<uint>& arrays )
{
	variable_byte_decode(buffer,len, arrays);

	for ( uint i=1; i<arrays.size() ;i++ )
	{
		arrays[i]+=arrays[i-1];
	}
}

void regular_byte_encode( std::vector<uint>& arrays, uchar* buffer, int* length )
{
	int  cur=0;
	for ( int i=0; i<arrays.size(); ++i )
	{
		memcpy(buffer+cur,(char*)&arrays[i], 4 );
		cur+=4;
	}
	*length=cur;
}

void regular_byte_decode( uchar* buffer, int len, std::vector<uint>& arrays )
{
	for (int i=0; i<len ; i+=4 )
	{
		int value;
		memcpy((char*)&value,buffer+i,4);
		arrays.push_back(value);
	}
}



int move_file( const char* src_file, const char* dest_file )
{
	assert(dest_file!=NULL&&src_file!=NULL);
	if (0==access(dest_file, 0))
	{  //先删除 目的文件
		remove(dest_file);
	}
	rename(src_file, dest_file);
	return 0;
}

int create_file_if_nonexist( const char* path )
{
	assert(path!=NULL);
	if ( -1==access(path,0) )
	{
		FILE* file=fopen(path,"w");
		fclose(file);
	}
	return 0;
}

int truncate_file( const char* path )
{
	assert(path!=NULL);
	if ( 0==access(path,0) )
	{  //destory file
		FILE* file=fopen(path,"w");
		fclose(file);
	}
	return 0;
}

