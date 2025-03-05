#pragma once

#include "../editor-state/editor-state.hpp"
#include "../parsing/map.hpp"

namespace widgets {
    void selection_province(parsing::game_map& map, state::control& control);
    void selection_nation(parsing::game_map& map, state::control& control, int32_t tag);
}