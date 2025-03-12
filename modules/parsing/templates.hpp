#pragma once
#include <fstream>

namespace state{
    struct editor;
}

namespace parser {
    void secondary_rgo_template_file(state::editor& map, std::ifstream& file);
}