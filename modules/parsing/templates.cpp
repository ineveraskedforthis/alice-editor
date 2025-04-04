#include <iostream>
#include <fstream>

#include "parser.hpp"
#include "templates.hpp"
#include "secondary_rgo.hpp"
#include "../editor-state/editor-state.hpp"

namespace parser {
    void secondary_rgo_template_file(state::editor& map, std::ifstream& file) {
        char c = ' ';

        parser::word template_name;
        while (true) {
            // start with parsing the key
            while (parser::nothing(c) && file.get(c));
            if (c != '#') {
                template_name.reset();
                while (template_name.parse(c) && file.get(c));
                map.secondary_rgo_templates[template_name.data] = {};
                std::vector<game_definition::secondary_rgo> template_data = {};
                secondary_rgo(template_data, file, c);
                for (auto item : template_data) {
                    map.secondary_rgo_templates[template_name.data][item.trade_good] = item.size;
                }
            }
            while (parser::until_end_of_the_line(c)) {
                if (!file.get(c)){
                    return;
                }
            };
            if (!parser::end_of_the_line(c)) {
                return;
            }
            while (parser::end_of_the_line(c)) {
                if (!file.get(c)){
                    return;
                }
            }
        }
    }
}