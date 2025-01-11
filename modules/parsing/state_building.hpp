#pragma once
#include <string>
#include "definitions.hpp"

namespace parser {
    struct state_building {
        int level;
        std::string building_type;
        std::string upgrade;
        void parse(game_definition::state_building& def, std::ifstream& file, char& c);
    };
}
