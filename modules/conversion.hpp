#pragma once
// copied from Project Alice
// Related GPL file can be found in ParserGenerator folder
#include <string>
#include <winnls.h>
#include <winnt.h>
namespace conversions {

char16_t win1250toUTF16(char in);
std::u16string win1250_to_u16(std::string data_in);
std::string u16_to_win1250(std::u16string data_in);

char16_t win1252toUTF16(char in);
std::u16string win1252_to_u16(std::string data_in);
std::string u16_to_win1252(std::u16string data_in);

std::u16string u8_to_u16(std::string str);
std::string u16_to_u8(std::u16string str);

std::wstring u8_to_w(std::string str);
std::string w_to_u8(std::wstring str);
}