#include "map.hpp"

namespace parser {
    void countries_list(parsers::game_map& map, std::ifstream& file);
    void country_file_common(game_definition::nation& n, std::ifstream& file);
}