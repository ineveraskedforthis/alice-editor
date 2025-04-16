#pragma once
#include <cstdint>
#include <string>

namespace state{
    struct layer;
    struct layers_stack;
    struct editor;
    enum class FLAG_EXPORT_OPTIONS;
}

namespace parsers{
    void load_templates(state::editor& editor_data, std::string path);
    void load_layer(state::layers_stack& state, state::layer& map_state);
    void unload_data(state::layer& layer, std::string path, state::FLAG_EXPORT_OPTIONS flag_option);
    uint32_t rgb_to_uint(int r, int g, int b);
}