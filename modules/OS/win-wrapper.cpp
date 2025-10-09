#include "win-wrapper.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <shobjidl_core.h>
#include <shtypes.h>

namespace winapi {

std::wstring open_image_selection_dialog(GUID& dialog_id) {
    IFileOpenDialog* DIALOG;
    auto DIALOG_RESULT = CoCreateInstance(
        CLSID_FileOpenDialog,
        NULL,
        CLSCTX_ALL,
        IID_IFileOpenDialog,
        reinterpret_cast<void**>(&DIALOG)
    );
    if(FAILED(DIALOG_RESULT)) {
        return L"";
    }

    auto hres = DIALOG->SetClientGuid(dialog_id);

    if (hres != S_OK) {
        MessageBoxW(
            NULL,
            L"Error during setting dialog guid.",
            L"Something is wrong???",
            MB_OK
        );
        return L"";
    }

    DIALOG->SetDefaultExtension(L"tga");

    _COMDLG_FILTERSPEC FILTER_JPEG;
    FILTER_JPEG.pszName = L"JPG (*.jpg,*.jpeg)";
    FILTER_JPEG.pszSpec = L"*.jpg;*.jpeg";

    _COMDLG_FILTERSPEC FILTER_PNG;
    FILTER_PNG.pszName = L"PNG (*.png)";
    FILTER_PNG.pszSpec = L"*.png";

    _COMDLG_FILTERSPEC FILTER_TGA;
    FILTER_TGA.pszName = L"TGA (*.tga)";
    FILTER_TGA.pszSpec = L"*.tga";

    _COMDLG_FILTERSPEC FILE_TYPES[3] {FILTER_PNG, FILTER_JPEG, FILTER_TGA};

    DIALOG->SetFileTypes(3, FILE_TYPES);
    DIALOG->SetOptions(FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM);

    DIALOG_RESULT = DIALOG->Show(NULL);
    if(FAILED(DIALOG_RESULT)) {
        DIALOG->Release();
        return L"";
    }

    IShellItem* ITEM;
    DIALOG_RESULT = DIALOG->GetResult(&ITEM);
    if(FAILED(DIALOG_RESULT)) {
        DIALOG->Release();
        return L"";
    }

    //  STORE AND CONVERT THE FILE NAME
    PWSTR RETRIEVED_PATH;
    DIALOG_RESULT = ITEM->GetDisplayName(SIGDN_FILESYSPATH, &RETRIEVED_PATH);
    if(FAILED(DIALOG_RESULT)) {
        ITEM->Release();
        DIALOG->Release();
        return L"";
    }

    std::wstring path(RETRIEVED_PATH);
    CoTaskMemFree(RETRIEVED_PATH);
    ITEM->Release();
    DIALOG->Release();
    return path;
}

UUID UUID_open_base_game;
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
                &UUID_open_base_game
            );
        } else {
            uuid_loaded = false;
        }

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
        UuidCreate(&UUID_open_base_game);
        UuidCreate(&UUID_open_mod);
        UuidCreate(&UUID_open_flags);
        UuidCreate(&UUID_open_trade_goods_icon);

        std::wofstream file("./editor-data.txt");

        {
            RPC_WSTR to_print;
            UuidToStringW(&UUID_open_base_game, &to_print);
            file << (LPCWSTR)to_print << L"\n";
            RpcStringFreeW(&to_print);
        }
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