#pragma once

#include "../editor-state/editor-state.hpp"
#include "../assets-manager/assets.hpp"

namespace widgets {
    void selection(state::layers_stack& map, state::control& control, state::editor& editor, assets::storage& storage);
    void map_context(state::layers_stack& layers, state::control& control, GLuint adj_buffer, int& adj_vertices_count);
}