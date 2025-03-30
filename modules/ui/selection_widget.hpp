#pragma once

#include "GL/glew.h"
namespace state {
    struct layers_stack;
    struct control;
    struct editor;
}
namespace assets {
    struct storage;
}

namespace widgets {
    void selection(state::layers_stack& map, state::control& control, state::editor& editor, assets::storage& storage);
    void map_context(state::layers_stack& layers, state::control& control, GLuint adj_buffer, int& adj_vertices_count);
}