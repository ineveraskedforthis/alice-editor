
// copied from Project Alice
// Related GPL file can be found in ParserGenerator folder

#include <cstdint>
#include <memory>
#include <string>
#include <winnls.h>
#include <winnt.h>

#include "conversion.hpp"
namespace conversions {
char16_t win1250toUTF16(char in) {
	constexpr static char16_t converted[256] =
		//		0		1		2		3		4		5		6		7		8		9		A		B		C		D		E		F
		/*0*/{ u' ',		u'\u0001', u'\u0002', u'\u0003', u'\u0004', u' ', u' ', u' ', u' ', u'\t', u'\n', u' ', u' ', u' ', u' ', u' ',
		/*1*/	u' ',		u' ',		u' ',		u' ',		u' ',		u' ',		u' ',		u' ',		u' ',		u' ',		u' ',		u' ',		u' ',		u' ',		u' ',		u' ',
		/*2*/	u' ',		u'!',		u'\"',		u'#',		u'$',		u'%',	u'&',		u'\'',		u'(',		u')',		u'*',		u'+',		u',',		u'-',		u'.',		u'/',
		/*3*/ u'0', u'1', u'2', u'3', u'4', u'5', u'6', u'7', u'8', u'9', u':', u';', u'<', u'=', u'>', u'?',
		/*4*/ u'@', u'A', u'B', u'C', u'D', u'E', u'F', u'G', u'H', u'I', u'J', u'K', u'L', u'M', u'N', u'O',
		/*5*/ u'P', u'Q', u'R', u'S', u'T', u'U', u'V', u'W', u'X', u'Y', u'Z', u'[', u'\\', u']', u'^', u'_',
		/*6*/ u'`', u'a', u'b', u'c', u'd', u'e', u'f', u'g', u'h', u'i', u'j', u'k', u'l', u'm', u'n', u'o',
		/*7*/ u'p', u'q', u'r', u's', u't', u'u', u'v', u'w', u'x', u'y', u'z', u'{', u'|', u'}', u'~', u' ',
		/*8*/ u'\u20AC', u' ', u'\u201A', u' ', u'\u201E', u'\u2026', u'\u2020', u'\u2021', u' ', u'\u2030', u'\u0160', u'\u2039', u'\u015A', u'\u0164', u'\u017D', u'\u0179',
		/*9*/ u' ', u'\u2018', u'\u2019', u'\u201C', u'\u201D', u'\u2022', u'\u2013', u'\u2014', u' ', u'\u2122', u'\u0161',
				u'\u203A', u'\u015B', u'\u0165', u'\u017E', u'\u017A',
		/*A*/ u'\u00A0', u'\u02C7', u'\u02D8', u'\u00A2', u'\u00A3', u'\u0104', u'\u00A6', u'\u00A7', u'\u00A8', u'\u00A9',
				u'\u015E', u'\u00AB', u'\u00AC', u'-', u'\u00AE', u'\u017B',
		/*B*/ u'\u00B0', u'\u00B1', u'\u02DB', u'\u0142', u'\u00B4', u'\u00B5', u'\u00B6', u'\u00B7', u'\u00B8', u'\u0105',
				u'\u015F', u'\u00BB', u'\u013D', u'\u02DD', u'\u013E', u'\u017C',
		/*C*/ u'\u0154', u'\u00C1', u'\u00C2', u'\u0102', u'\u00C4', u'\u0139', u'\u0106', u'\u00C7', u'\u010C', u'\u00C9',
				u'\u0118', u'\u00CB', u'\u011A', u'\u00CD', u'\u00CE', u'\u010E',
		/*D*/ u'\u0110', u'\u0143', u'\u0147', u'\u00D3', u'\u00D4', u'\u0150', u'\u00D6', u'\u00D7', u'\u0158', u'\u016E',
				u'\u00DA', u'\u0170', u'\u00DC', u'\u00DD', u'\u0162', u'\u00DF',
		/*E*/ u'\u0115', u'\u00E1', u'\u00E2', u'\u0103', u'\u00E4', u'\u013A', u'\u0107', u'\u00E7', u'\u00E8', u'\u00E9',
				u'\u0119', u'\u00EB', u'\u011B', u'\u00ED', u'\u00EE', u'\u010F',
		/*F*/ u'\u0111', u'\u0144', u'\u0148', u'\u00F3', u'\u00F4', u'\u0151', u'\u00F6', u'\u00F7', u'\u0159', u'\u016F',
				u'\u00FA', u'\u0171', u'\u00FC', u'\u00FD', u'\u0163', u'\u02D9' };

	return converted[(uint8_t)in];
}

std::wstring win1250_to_native(std::string data_in) {
	std::wstring result;
	for(auto ch : data_in) {
		result += wchar_t(win1250toUTF16(ch));
	}
	return result;
}

std::string native_to_win1250(std::wstring data_in) {
	std::string result;
	for(auto ch: data_in) {
		for (size_t i = 0; i < 256; i++) {
			if (win1250toUTF16(i) == ch) {
				result += (char8_t)i;
			}
		}
	}
	return result;
}

std::wstring utf8_to_wstring(std::string_view str) {
	if(str.size() > 0) {
		auto buffer = std::unique_ptr<WCHAR[]>(new WCHAR[str.length() * 2]);
		auto chars_written = MultiByteToWideChar(CP_UTF8, 0, str.data(), int32_t(str.length()), buffer.get(), int32_t(str.length() * 2));
		return std::wstring(buffer.get(), size_t(chars_written));
	}
	return std::wstring(L"");
}

std::string wstring_to_utf8(std::wstring str) {
	if(str.size() > 0) {
		auto buffer = std::unique_ptr<char[]>(new char[str.length() * 4]);
		auto chars_written = WideCharToMultiByte(CP_UTF8, 0, str.data(), int32_t(str.length()), buffer.get(), int32_t(str.length() * 4), NULL, NULL);
		return std::string(buffer.get(), size_t(chars_written));
	}
	return std::string("");
}

}