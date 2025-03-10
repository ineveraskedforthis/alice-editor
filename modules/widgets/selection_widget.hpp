#pragma once

#include "../editor-state/editor-state.hpp"
#include "../parsing/map.hpp"
#include "../assets-manager/assets.hpp"

namespace widgets {
    void selection(parsers::game_map& map, state::control& control, assets::storage& storage);
    void selection_province(parsers::game_map& map, state::control& control);
    void selection_nation(parsers::game_map& map, state::control& control, assets::storage& storage, int32_t tag);
}