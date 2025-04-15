#pragma once
#include <string>
#include <vector>
#include <fstream>

#include "definitions.hpp"

namespace parser {
struct adj {
    std::string current_word;
    bool ignore_line = false;
    bool reading_comment;
    int parse(std::ifstream& file, std::vector<game_definition::adjacency>& storage);
};
}