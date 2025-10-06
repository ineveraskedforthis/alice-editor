
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

char16_t win1252toUTF16(char in) {
	constexpr static char16_t convert[256] = {
0x0000,
0x0001, //START OF HEADING
0x0002, //START OF TEXT
0x0003, //END OF TEXT
0x0004, //END OF TRANSMISSION
0x0005, //ENQUIRY
0x0006, //ACKNOWLEDGE
0x0007, //BELL
0x0008, //BACKSPACE
0x0009, //HORIZONTAL TABULATION
0x000A, //LINE FEED
0x000B, //VERTICAL TABULATION
0x000C, //FORM FEED
0x000D, //CARRIAGE RETURN
0x000E, //SHIFT OUT
0x000F, //SHIFT IN
0x0010, //DATA LINK ESCAPE
0x0011, //DEVICE CONTROL ONE
0x0012, //DEVICE CONTROL TWO
0x0013, //DEVICE CONTROL THREE
0x0014, //DEVICE CONTROL FOUR
0x0015, //NEGATIVE ACKNOWLEDGE
0x0016, //SYNCHRONOUS IDLE
0x0017, //END OF TRANSMISSION BLOCK
0x0018, //CANCEL
0x0019, //END OF MEDIUM
0x001A, //SUBSTITUTE
0x001B, //ESCAPE
0x001C, //FILE SEPARATOR
0x001D, //GROUP SEPARATOR
0x001E, //RECORD SEPARATOR
0x001F, //UNIT SEPARATOR
0x0020, //SPACE
0x0021, //EXCLAMATION MARK
0x0022, //QUOTATION MARK
0x0023, //NUMBER SIGN
0x0024, //DOLLAR SIGN
0x0025, //PERCENT SIGN
0x0026, //AMPERSAND
0x0027, //APOSTROPHE
0x0028, //LEFT PARENTHESIS
0x0029, //RIGHT PARENTHESIS
0x002A, //ASTERISK
0x002B, //PLUS SIGN
0x002C, //COMMA
0x002D, //HYPHEN-MINUS
0x002E, //FULL STOP
0x002F, //SOLIDUS
0x0030, //DIGIT ZERO
0x0031, //DIGIT ONE
0x0032, //DIGIT TWO
0x0033, //DIGIT THREE
0x0034, //DIGIT FOUR
0x0035, //DIGIT FIVE
0x0036, //DIGIT SIX
0x0037, //DIGIT SEVEN
0x0038, //DIGIT EIGHT
0x0039, //DIGIT NINE
0x003A, //COLON
0x003B, //SEMICOLON
0x003C, //LESS-THAN SIGN
0x003D, //EQUALS SIGN
0x003E, //GREATER-THAN SIGN
0x003F, //QUESTION MARK
0x0040, //COMMERCIAL AT
0x0041, //LATIN CAPITAL LETTER A
0x0042, //LATIN CAPITAL LETTER B
0x0043, //LATIN CAPITAL LETTER C
0x0044, //LATIN CAPITAL LETTER D
0x0045, //LATIN CAPITAL LETTER E
0x0046, //LATIN CAPITAL LETTER F
0x0047, //LATIN CAPITAL LETTER G
0x0048, //LATIN CAPITAL LETTER H
0x0049, //LATIN CAPITAL LETTER I
0x004A, //LATIN CAPITAL LETTER J
0x004B, //LATIN CAPITAL LETTER K
0x004C, //LATIN CAPITAL LETTER L
0x004D, //LATIN CAPITAL LETTER M
0x004E, //LATIN CAPITAL LETTER N
0x004F, //LATIN CAPITAL LETTER O
0x0050, //LATIN CAPITAL LETTER P
0x0051, //LATIN CAPITAL LETTER Q
0x0052, //LATIN CAPITAL LETTER R
0x0053, //LATIN CAPITAL LETTER S
0x0054, //LATIN CAPITAL LETTER T
0x0055, //LATIN CAPITAL LETTER U
0x0056, //LATIN CAPITAL LETTER V
0x0057, //LATIN CAPITAL LETTER W
0x0058, //LATIN CAPITAL LETTER X
0x0059, //LATIN CAPITAL LETTER Y
0x005A, //LATIN CAPITAL LETTER Z
0x005B, //LEFT SQUARE BRACKET
0x005C, //REVERSE SOLIDUS
0x005D, //RIGHT SQUARE BRACKET
0x005E, //CIRCUMFLEX ACCENT
0x005F, //LOW LINE
0x0060, //GRAVE ACCENT
0x0061, //LATIN SMALL LETTER A
0x0062, //LATIN SMALL LETTER B
0x0063, //LATIN SMALL LETTER C
0x0064, //LATIN SMALL LETTER D
0x0065, //LATIN SMALL LETTER E
0x0066, //LATIN SMALL LETTER F
0x0067, //LATIN SMALL LETTER G
0x0068, //LATIN SMALL LETTER H
0x0069, //LATIN SMALL LETTER I
0x006A, //LATIN SMALL LETTER J
0x006B, //LATIN SMALL LETTER K
0x006C, //LATIN SMALL LETTER L
0x006D, //LATIN SMALL LETTER M
0x006E, //LATIN SMALL LETTER N
0x006F, //LATIN SMALL LETTER O
0x0070, //LATIN SMALL LETTER P
0x0071, //LATIN SMALL LETTER Q
0x0072, //LATIN SMALL LETTER R
0x0073, //LATIN SMALL LETTER S
0x0074, //LATIN SMALL LETTER T
0x0075, //LATIN SMALL LETTER U
0x0076, //LATIN SMALL LETTER V
0x0077, //LATIN SMALL LETTER W
0x0078, //LATIN SMALL LETTER X
0x0079, //LATIN SMALL LETTER Y
0x007A, //LATIN SMALL LETTER Z
0x007B, //LEFT CURLY BRACKET
0x007C, //VERTICAL LINE
0x007D, //RIGHT CURLY BRACKET
0x007E, //TILDE
0x007F, //DELETE
0x20AC, //EURO SIGN
0,       //UNDEFINED
0x201A, //SINGLE LOW-9 QUOTATION MARK
0x0192, //LATIN SMALL LETTER F WITH HOOK
0x201E, //DOUBLE LOW-9 QUOTATION MARK
0x2026, //HORIZONTAL ELLIPSIS
0x2020, //DAGGER
0x2021, //DOUBLE DAGGER
0x02C6, //MODIFIER LETTER CIRCUMFLEX ACCENT
0x2030, //PER MILLE SIGN
0x0160, //LATIN CAPITAL LETTER S WITH CARON
0x2039, //SINGLE LEFT-POINTING ANGLE QUOTATION MARK
0x0152, //LATIN CAPITAL LIGATURE OE
0,       //UNDEFINED
0x017D, //LATIN CAPITAL LETTER Z WITH CARON
0,       //UNDEFINED
0,       //UNDEFINED
0x2018, //LEFT SINGLE QUOTATION MARK
0x2019, //RIGHT SINGLE QUOTATION MARK
0x201C, //LEFT DOUBLE QUOTATION MARK
0x201D, //RIGHT DOUBLE QUOTATION MARK
0x2022, //BULLET
0x2013, //EN DASH
0x2014, //EM DASH
0x02DC, //SMALL TILDE
0x2122, //TRADE MARK SIGN
0x0161, //LATIN SMALL LETTER S WITH CARON
0x203A, //SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
0x0153, //LATIN SMALL LIGATURE OE
0,      //UNDEFINED
0x017E, //LATIN SMALL LETTER Z WITH CARON
0x0178, //LATIN CAPITAL LETTER Y WITH DIAERESIS
0x00A0, //NO-BREAK SPACE
0x00A1, //INVERTED EXCLAMATION MARK
0x00A2, //CENT SIGN
0x00A3, //POUND SIGN
0x00A4, //CURRENCY SIGN
0x00A5, //YEN SIGN
0x00A6, //BROKEN BAR
0x00A7, //SECTION SIGN
0x00A8, //DIAERESIS
0x00A9, //COPYRIGHT SIGN
0x00AA, //FEMININE ORDINAL INDICATOR
0x00AB, //LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
0x00AC, //NOT SIGN
0x00AD, //SOFT HYPHEN
0x00AE, //REGISTERED SIGN
0x00AF, //MACRON
0x00B0, //DEGREE SIGN
0x00B1, //PLUS-MINUS SIGN
0x00B2, //SUPERSCRIPT TWO
0x00B3, //SUPERSCRIPT THREE
0x00B4, //ACUTE ACCENT
0x00B5, //MICRO SIGN
0x00B6, //PILCROW SIGN
0x00B7, //MIDDLE DOT
0x00B8, //CEDILLA
0x00B9, //SUPERSCRIPT ONE
0x00BA, //MASCULINE ORDINAL INDICATOR
0x00BB, //RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
0x00BC, //VULGAR FRACTION ONE QUARTER
0x00BD, //VULGAR FRACTION ONE HALF
0x00BE, //VULGAR FRACTION THREE QUARTERS
0x00BF, //INVERTED QUESTION MARK
0x00C0, //LATIN CAPITAL LETTER A WITH GRAVE
0x00C1, //LATIN CAPITAL LETTER A WITH ACUTE
0x00C2, //LATIN CAPITAL LETTER A WITH CIRCUMFLEX
0x00C3, //LATIN CAPITAL LETTER A WITH TILDE
0x00C4, //LATIN CAPITAL LETTER A WITH DIAERESIS
0x00C5, //LATIN CAPITAL LETTER A WITH RING ABOVE
0x00C6, //LATIN CAPITAL LETTER AE
0x00C7, //LATIN CAPITAL LETTER C WITH CEDILLA
0x00C8, //LATIN CAPITAL LETTER E WITH GRAVE
0x00C9, //LATIN CAPITAL LETTER E WITH ACUTE
0x00CA, //LATIN CAPITAL LETTER E WITH CIRCUMFLEX
0x00CB, //LATIN CAPITAL LETTER E WITH DIAERESIS
0x00CC, //LATIN CAPITAL LETTER I WITH GRAVE
0x00CD, //LATIN CAPITAL LETTER I WITH ACUTE
0x00CE, //LATIN CAPITAL LETTER I WITH CIRCUMFLEX
0x00CF, //LATIN CAPITAL LETTER I WITH DIAERESIS
0x00D0, //LATIN CAPITAL LETTER ETH
0x00D1, //LATIN CAPITAL LETTER N WITH TILDE
0x00D2, //LATIN CAPITAL LETTER O WITH GRAVE
0x00D3, //LATIN CAPITAL LETTER O WITH ACUTE
0x00D4, //LATIN CAPITAL LETTER O WITH CIRCUMFLEX
0x00D5, //LATIN CAPITAL LETTER O WITH TILDE
0x00D6, //LATIN CAPITAL LETTER O WITH DIAERESIS
0x00D7, //MULTIPLICATION SIGN
0x00D8, //LATIN CAPITAL LETTER O WITH STROKE
0x00D9, //LATIN CAPITAL LETTER U WITH GRAVE
0x00DA, //LATIN CAPITAL LETTER U WITH ACUTE
0x00DB, //LATIN CAPITAL LETTER U WITH CIRCUMFLEX
0x00DC, //LATIN CAPITAL LETTER U WITH DIAERESIS
0x00DD, //LATIN CAPITAL LETTER Y WITH ACUTE
0x00DE, //LATIN CAPITAL LETTER THORN
0x00DF, //LATIN SMALL LETTER SHARP S
0x00E0, //LATIN SMALL LETTER A WITH GRAVE
0x00E1, //LATIN SMALL LETTER A WITH ACUTE
0x00E2, //LATIN SMALL LETTER A WITH CIRCUMFLEX
0x00E3, //LATIN SMALL LETTER A WITH TILDE
0x00E4, //LATIN SMALL LETTER A WITH DIAERESIS
0x00E5, //LATIN SMALL LETTER A WITH RING ABOVE
0x00E6, //LATIN SMALL LETTER AE
0x00E7, //LATIN SMALL LETTER C WITH CEDILLA
0x00E8, //LATIN SMALL LETTER E WITH GRAVE
0x00E9, //LATIN SMALL LETTER E WITH ACUTE
0x00EA, //LATIN SMALL LETTER E WITH CIRCUMFLEX
0x00EB, //LATIN SMALL LETTER E WITH DIAERESIS
0x00EC, //LATIN SMALL LETTER I WITH GRAVE
0x00ED, //LATIN SMALL LETTER I WITH ACUTE
0x00EE, //LATIN SMALL LETTER I WITH CIRCUMFLEX
0x00EF, //LATIN SMALL LETTER I WITH DIAERESIS
0x00F0, //LATIN SMALL LETTER ETH
0x00F1, //LATIN SMALL LETTER N WITH TILDE
0x00F2, //LATIN SMALL LETTER O WITH GRAVE
0x00F3, //LATIN SMALL LETTER O WITH ACUTE
0x00F4, //LATIN SMALL LETTER O WITH CIRCUMFLEX
0x00F5, //LATIN SMALL LETTER O WITH TILDE
0x00F6, //LATIN SMALL LETTER O WITH DIAERESIS
0x00F7, //DIVISION SIGN
0x00F8, //LATIN SMALL LETTER O WITH STROKE
0x00F9, //LATIN SMALL LETTER U WITH GRAVE
0x00FA, //LATIN SMALL LETTER U WITH ACUTE
0x00FB, //LATIN SMALL LETTER U WITH CIRCUMFLEX
0x00FC, //LATIN SMALL LETTER U WITH DIAERESIS
0x00FD, //LATIN SMALL LETTER Y WITH ACUTE
0x00FE, //LATIN SMALL LETTER THORN
0x00FF, //LATIN SMALL LETTER Y WITH DIAERESIS
	};

	return convert[in];
}

std::u16string win1250_to_u16(std::string data_in) {
	std::u16string result;
	for(auto ch : data_in) {
		result += wchar_t(win1250toUTF16(ch));
	}
	return result;
}

std::string u16_to_win1250(std::u16string data_in) {
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

std::u16string win1252_to_u16(std::string data_in) {
	std::u16string result;
	for(auto ch : data_in) {
		result += wchar_t(win1252toUTF16(ch));
	}
	return result;
}

std::string u16_to_win1252(std::u16string data_in) {
	std::string result;
	for(auto ch: data_in) {
		for (size_t i = 0; i < 256; i++) {
			if (win1252toUTF16(i) == ch) {
				result += (char8_t)i;
			}
		}
	}
	return result;
}

std::u16string u8_to_u16(std::string str) {
	if(str.size() > 0) {
		auto buffer = std::unique_ptr<WCHAR[]>(new WCHAR[str.length() * 2]);
		auto chars_written = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(str.data()), int32_t(str.length()), buffer.get(), int32_t(str.length() * 2));
		auto wstring = std::wstring(buffer.get(), size_t(chars_written));
		return std::u16string(wstring.begin(), wstring.end());
	}
	return std::u16string(u"");
}

std::string u16_to_u8(std::u16string str) {
	if(str.size() > 0) {
		auto buffer = std::unique_ptr<char[]>(new char[str.length() * 4]);
		auto chars_written = WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(str.data()), int32_t(str.length()), buffer.get(), int32_t(str.length() * 4), NULL, NULL);
		return std::string(buffer.get(), size_t(chars_written));
	}
	return std::string("");
}

std::wstring u8_to_w(std::string str) {
	if(str.size() > 0) {
		auto buffer = std::unique_ptr<WCHAR[]>(new WCHAR[str.length() * 2]);
		auto chars_written = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(str.data()), int32_t(str.length()), buffer.get(), int32_t(str.length() * 2));
		auto wstring = std::wstring(buffer.get(), size_t(chars_written));
		return wstring;
	}
	return std::wstring(L"");
}

std::string w_to_u8(std::wstring str) {
	if(str.size() > 0) {
		auto buffer = std::unique_ptr<char[]>(new char[str.length() * 4]);
		auto chars_written = WideCharToMultiByte(CP_UTF8, 0, reinterpret_cast<const wchar_t*>(str.data()), int32_t(str.length()), buffer.get(), int32_t(str.length() * 4), NULL, NULL);
		return std::string(buffer.get(), size_t(chars_written));
	}
	return std::string("");
}
}