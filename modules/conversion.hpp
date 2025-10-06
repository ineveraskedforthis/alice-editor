#pragma once
// copied from Project Alice
// Related GPL file can be found in ParserGenerator folder
#include <string>
#include <winnls.h>
#include <winnt.h>
namespace conversions {
char16_t win1250toUTF16(char in);
std::wstring win1250_to_native(std::string data_in);
std::string native_to_win1250(std::wstring data_in);
std::wstring utf8_to_wstring(std::string_view str);
std::string wstring_to_utf8(std::wstring str);
}