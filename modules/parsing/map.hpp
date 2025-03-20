#pragma once
#include <cstdint>
#include <string>

namespace state{
    struct layer;
    struct layers_stack;
    struct editor;
}

namespace parsers{
    void load_templates(state::editor& editor_data, std::string path);
    void load_layer(state::layers_stack& state, state::layer& map_state);
    void unload_layer(state::layer& map_state, std::string path);
    uint32_t rgb_to_uint(int r, int g, int b);
}