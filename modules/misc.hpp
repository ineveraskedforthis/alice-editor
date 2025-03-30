#pragma once

#include <string>
#include <memory>
#include <shobjidl_core.h>
#include <shobjidl.h>

namespace conversions {
    // copied from Project Alice
    // Related GPL file can be found in ParserGenerator folder
    inline std::wstring utf8_to_wstring(std::string_view str) {
        if(str.size() > 0) {
            auto buffer = std::unique_ptr<WCHAR[]>(new WCHAR[str.length() * 2]);
            auto chars_written = MultiByteToWideChar(CP_UTF8, 0, str.data(), int32_t(str.length()), buffer.get(), int32_t(str.length() * 2));
            return std::wstring(buffer.get(), size_t(chars_written));
        }
        return std::wstring(L"");
    }

    inline std::string wstring_to_utf8(std::wstring str) {
        if(str.size() > 0) {
            auto buffer = std::unique_ptr<char[]>(new char[str.length() * 4]);
            auto chars_written = WideCharToMultiByte(CP_UTF8, 0, str.data(), int32_t(str.length()), buffer.get(), int32_t(str.length() * 4), NULL, NULL);
            return std::string(buffer.get(), size_t(chars_written));
        }
        return std::string("");
    }
};