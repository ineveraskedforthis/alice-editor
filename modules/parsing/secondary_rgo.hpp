#pragma once
#include "definitions.hpp"

namespace parser {
    void secondary_rgo_entry(game_definition::secondary_rgo& rgo, std::ifstream& file, char& c);
    void secondary_rgo(std::vector<game_definition::secondary_rgo>& prov, std::ifstream& file, char& c);
}