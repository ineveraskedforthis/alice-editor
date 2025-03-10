#pragma once
#include <fstream>

namespace parsers{
    struct game_map;
}

namespace parser {
    void secondary_rgo_template_file(parsers::game_map& map, std::ifstream& file);
}