#include <assert.h>
#include <io.h>
#ifdef WIN32
#include <windows.h>
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


std::string get_core_path()
{
	std::string core_path;
#ifdef WIN32
	CHAR path[MAX_PATH+1] = {0};
	GetModuleFileName(NULL, path, MAX_PATH);
	//WideCharToMultiByte(CP_ACP,0,path,MAX_PATH,cstr_path, MAX_PATH, NULL, 0);
	core_path = path;
	core_path=core_path.substr(0, core_path.find_last_of('\\')+1);
#else
#endif
	return core_path;
}

int move_file( const char* src_file, const char* dest_file )
{
	assert(dest_file!=NULL&&src_file!=NULL);
	if (0==_access(dest_file, 0))
	{  //先删除 目的文件
		remove(dest_file);
	}
	rename(src_file, dest_file);
	return 0;
}

int create_file_if_nonexist( const char* path )
{
	assert(path!=NULL);
	if ( -1==_access(path,0) )
	{
		FILE* file=fopen(path,"w");
		fclose(file);
	}
	return 0;
}
