#include <iostream>

namespace state{
    struct layer;
}
namespace game_definition{
    struct nation_common;
}
namespace parser {
    void countries_list(state::layer& map, std::ifstream& file);
    void country_file_common(game_definition::nation_common& n, std::ifstream& file, std::string filename);
}