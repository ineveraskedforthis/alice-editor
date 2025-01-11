#include "state_building.hpp"
#include "parser.hpp"
#include <fstream>
#include <iostream>

namespace parser {

void state_building::parse(game_definition::state_building& def, std::ifstream& file, char& c) {
    parser::word word;
    while (parser::until_open_bracket(c) && file.get(c));
    while (true) {
        while (parser::nothing(c) && file.get(c));
        if (c != '#') {
            word.reset();
            while (word.parse(c) && file.get(c));
            if (word.data == "level") {
                parser::word value;
                while (parser::equality(c) && file.get(c));
                while (value.parse(c) && file.get(c));

                def.level = std::stoi(value.data);
            } else if (word.data == "building") {
                parser::word value;
                while (parser::equality(c) && file.get(c));
                while (value.parse(c) && file.get(c));

                def.building_type = value.data;
            } else if (word.data == "upgrade") {
                parser::word value;
                while (parser::equality(c) && file.get(c));
                while (value.parse(c) && file.get(c));

                def.upgrade = value.data;
            }
        }

        while (parser::end_of_the_line(c) && file.get(c));
        if (parser::end_of_the_line(c)) {
            if (!file.get(c)){
                break;
            }
        }

        if (c == '}') {
            file.get(c);
            return;
        }
    }
}

}