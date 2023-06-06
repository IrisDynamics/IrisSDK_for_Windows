#pragma once

#include <string>
#include <locale>
#include <codecvt>


/**
 *	This class was created and is updated as the need arises. It is by no means exhaustive or 
 *	context agnostic. It is designed for converting UTF-8 strings to UTF-16 strings with strings
 *  only containing 7 bit ASCII characters
 */ 
class StringConverter {
public:
	std::wstring to_wstr_from_str(std::string str) {
		return str_wstr_converter.from_bytes(str);
	}

private:
	//string converter
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> str_wstr_converter;
};