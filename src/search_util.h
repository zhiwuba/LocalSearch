#pragma once

#include <string>
#include <vector>
#include "search_porting.h"
#include "search_define.h"


bool is_alpha_char(char c);

void string_to_lower(char* str);

void print_binary(char c);

/*
**  游程编码-- 变长编码
**/
void variable_byte_encode(std::vector<uint>& arrays ,uchar* buffer ,int* length);

/*
** 游程解码-- 变长编码
**/
void variable_byte_decode(uchar* buffer, int len ,std::vector<uint>& arrays );

/*
**  数据编码-- 差分序列+变长
**/
void compress_data(std::vector<uint>& arrays, uchar* buffer, int* length);

/*
**  数据解码-- 差分序列+变长
**/
void decompress_data(uchar* buffer, int len, std::vector<uint>& arrays );


/*
**  数组编码  -- 非压缩
**/
void regular_byte_encode(std::vector<uint>& arrays, uchar* buffer, int* length);


/*
**  数组解码 --非压缩
**/
void regular_byte_decode(uchar* buffer, int len, std::vector<uint>& arrays);



/*
**  获取程序当前路径
**/
std::string get_core_path();


int move_file(const char* src_file, const char* dest_file);

int create_file_if_nonexist( const char* path);

int truncate_file( const char* path );
