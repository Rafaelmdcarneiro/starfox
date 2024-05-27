#include "string_helpers.h"
#include <ctype.h>
#include <locale>
#include <algorithm>
#include <stdlib.h>
#include <stdarg.h>

using namespace std;

const int BUFF_SIZE = 4096;
static char g_buff[BUFF_SIZE];

std::string& str_tolower(std::string& str)
{
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str;
}

std::string& str_toupper(std::string& str)
{
	transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

std::string str_tolower(const std::string& str)
{	
	string strCopy = str; // Make copy
	return str_tolower( strCopy ); // Call non-const overload
}

std::string str_toupper(const std::string& str)
{
	string strCopy = str; // Make copy
	return str_toupper( strCopy ); // Call non-const overload
}

int str_compare_no_case(const std::string& str1, const std::string& str2)
{
	return str_tolower(str1).compare(str_tolower(str2));
}

std::string str_format(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(g_buff, format, args);
	return g_buff; // Will construct a std::string
}

std::string& str_format_into(std::string& str, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsprintf(g_buff, format, args);
	str = g_buff;
	return str;
}

void str_trim(std::string& str, const std::string& strWhiteSpace/*=" \t"*/)
{
  if ( str.empty() )
    return;

  std::string strTemp = str;
  size_t a = strTemp.find_first_not_of(strWhiteSpace);
  size_t b = strTemp.find_last_not_of(strWhiteSpace);

  str = strTemp.substr(a, b-a+1);
}
