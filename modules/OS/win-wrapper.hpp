#pragma once
#include <string>
#include <windows.h>
#include <rpcdce.h>

namespace winapi {
extern UUID UUID_open_mod;
extern UUID UUID_open_flags;
extern UUID UUID_open_trade_goods_icon;
const std::wstring uuidformat = L"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
void load_uuids();
};