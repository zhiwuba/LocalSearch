#pragma once

#include <string>
#include <hash_map>
#include "search_define.h"


typedef std::hash_map<std::string, int>  StringIntHashMap;


bool is_alpha_char(char c);

void string_to_lower(char* str);

void print_binary(char c);

/*
**  游程编码-- 变长编码
**/
void variable_byte_encode(std::vector<uint>& arrays ,uchar** buffer ,int* length);

/*
** 游程解码-- 变长编码
**/
void variable_byte_decode(uchar* buffer, int len ,std::vector<uint>& arrays );


/*
**  获取程序当前路径
**/
std::string get_core_path();
