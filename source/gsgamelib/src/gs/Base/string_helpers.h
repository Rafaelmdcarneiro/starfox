#ifndef _STRING_HELPERS_H_
#define _STRING_HELPERS_H_

#include <string>

// Contains std::string helper functions

// Converts to lower-case
std::string& str_tolower(std::string& str);			// Returns input string
std::string str_tolower(const std::string& str);	// Returns string copy

// Converts to upper-case
std::string& str_toupper(std::string& str);			// Returns input string
std::string str_toupper(const std::string& str);	// Returns string copy

// Compares input strings, ignoring case
int str_compare_no_case(const std::string& str1, const std::string& str2);

// Builds string copy using printf() style formatting
std::string str_format(const char* format, ...);							// Returns string copy
std::string& str_format_into(std::string& str, const char* format, ...);	// Returns input string

// Trims surrounding whitespace of input string
void str_trim(std::string& str, const std::string& strWhiteSpace=" \t");

#endif
