#include "win-wrapper.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace winapi {
UUID UUID_open_mod;
UUID UUID_open_flags;
UUID UUID_open_trade_goods_icon;
void load_uuids() {
    auto file_exists = std::filesystem::exists("./editor-data.txt");
    bool uuid_loaded = true;
    if (file_exists) {
        RPC_STATUS parsing_result;

        std::wifstream file("./editor-data.txt");
        std::wstring line;

        if (std::getline(file, line) && line.size() >= uuidformat.size()) {
            parsing_result = UuidFromStringW(
                reinterpret_cast<RPC_WSTR>(
                    const_cast<wchar_t*>(
                        line.substr(0, uuidformat.size()).c_str()
                    )
                ),
                &UUID_open_mod
            );
        } else {
            uuid_loaded = false;
        }

        if (parsing_result == RPC_S_OK && std::getline(file, line) && line.size() >= uuidformat.size()) {
            parsing_result = UuidFromStringW(
                reinterpret_cast<RPC_WSTR>(
                    const_cast<wchar_t*>(
                        line.substr(0, uuidformat.size()).c_str()
                    )
                ),
                &UUID_open_flags
            );
        } else {
            uuid_loaded = false;
        }

        if (parsing_result == RPC_S_OK && std::getline(file, line) && line.size() >= uuidformat.size()) {
            parsing_result = UuidFromStringW(
                reinterpret_cast<RPC_WSTR>(
                    const_cast<wchar_t*>(
                        line.substr(0, uuidformat.size()).c_str()
                    )
                ),
                &UUID_open_trade_goods_icon
            );
        } else {
            uuid_loaded = false;
        }

        if (parsing_result != RPC_S_OK) {
            uuid_loaded = false;
        }
    } else {
        uuid_loaded = false;
    }

    if (!uuid_loaded) {
        UuidCreate(&UUID_open_mod);
        UuidCreate(&UUID_open_flags);
        UuidCreate(&UUID_open_trade_goods_icon);

        std::wofstream file("./editor-data.txt");

        {
            RPC_WSTR to_print;
            UuidToStringW(&UUID_open_mod, &to_print);
            file << (LPCWSTR)to_print << L"\n";
            RpcStringFreeW(&to_print);
        }
        {
            RPC_WSTR to_print;
            UuidToStringW(&UUID_open_flags, &to_print);
            file << (LPCWSTR)to_print << L"\n";
            RpcStringFreeW(&to_print);
        }
        {
            RPC_WSTR to_print;
            UuidToStringW(&UUID_open_trade_goods_icon, &to_print);
            file << (LPCWSTR)to_print << L"\n";
            RpcStringFreeW(&to_print);
        }
    }

}
}