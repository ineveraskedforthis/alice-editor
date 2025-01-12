#pragma once
#include <fstream>

namespace parsing{
    struct game_map;
}

namespace parser {
    void secondary_rgo_template_file(parsing::game_map& map, std::ifstream& file);
}