#include <fstream>
#include <iostream>
#include <string>
#include <array>

#include "countries.hpp"
#include "definitions.hpp"
#include "parser.hpp"

namespace parser {

    void country_file_common(game_definition::nation& n, std::ifstream& file) {
        std::cout << "Parse common for " << n.filename << "\n";

        parser::word key;
        parser::word value;
        char c = ' ';
        // parse file
        while (true) {
            while (parser::nothing(c) && file.get(c));
            if (c != '#') {
                key.reset();
                while (key.parse(c) && file.get(c));

                if (key.data == "color") {
                    std::cout << "color detected\n";

                    // parse color:

                    while (parser::until_open_bracket(c) && file.get(c));

                    file.get(c);

                    // read three values:

                    value.reset();
                    while(parser::nothing(c) && file.get(c));
                    while(value.parse(c) && file.get(c));
                    std::cout << "R:" << value.data;
                    n.R = std::stoi(value.data);

                    value.reset();
                    while(parser::nothing(c) && file.get(c));
                    while(value.parse(c) && file.get(c));
                    std::cout << "G:" << value.data;
                    n.G = std::stoi(value.data);

                    value.reset();
                    while(parser::nothing(c) && file.get(c));
                    while(value.parse(c) && file.get(c));
                    std::cout << "B:" << value.data;
                    n.B = std::stoi(value.data);

                    std::cout << "\n";


                    while (parser::until_close_bracket(c) && file.get(c));
                } else if (key.data == "graphical_culture") {
                    parser::word_after_equality word;
                    while (word.parse(c) && file.get(c));
                    n.graphical_culture = word.data;
                } else if (key.data == "party") {
                    // parse party
                    parser::word party_key;
                    game_definition::party party;
                    while (parser::until_open_bracket(c) && file.get(c));
                    while (parser::until_close_bracket(c)) {
                        party_key.reset();
                        while (parser::nothing(c) && file.get(c));
                        if (c != '#') {
                            while (party_key.parse(c) && file.get(c));
                            if (party_key.data == "name") {
                                parser::string_after_equality name;
                                while (name.parse(c) && file.get(c));
                                party.name = name.data;
                            } else if (party_key.data == "start_date") {
                                parser::word_after_equality word;
                                while (word.parse(c) && file.get(c));
                                party.start = word.data;
                            } else if (party_key.data == "end_date") {
                                parser::word_after_equality word;
                                while (word.parse(c) && file.get(c));
                                party.end = word.data;
                            } else if (party_key.data == "ideology") {
                                parser::word_after_equality word;
                                while (word.parse(c) && file.get(c));
                                party.ideology = word.data;
                            } else if (party_key.data == "economic_policy") {
                                parser::word_after_equality word;
                                while (word.parse(c) && file.get(c));
                                party.economic_policy = word.data;
                            } else if (party_key.data == "trade_policy") {
                                parser::word_after_equality word;
                                while (word.parse(c) && file.get(c));
                                party.trade_policy = word.data;
                            } else if (party_key.data == "religious_policy") {
                                parser::word_after_equality word;
                                while (word.parse(c) && file.get(c));
                                party.religious_policy = word.data;
                            } else if (party_key.data == "citizenship_policy") {
                                parser::word_after_equality word;
                                while (word.parse(c) && file.get(c));
                                party.citizenship_policy = word.data;
                            } else if (party_key.data == "war_policy") {
                                parser::word_after_equality word;
                                while (word.parse(c) && file.get(c));
                                party.war_policy = word.data;
                            }
                        }
                        while (!parser::strict_end_of_the_line(c) && file.get(c));
                        while (parser::nothing(c) && file.get(c));
                    }
                    n.parties.push_back(party);
                } else if (key.data == "unit_names") {
                    while (parser::until_open_bracket(c) && file.get(c));
                    while (parser::nothing(c) && file.get(c));
                    while (parser::until_close_bracket(c) && file.get(c))
                        n.unit_names += c;
                }
            }
            while (!parser::strict_end_of_the_line(c)) {
                if (!file.get(c)) {
                    return;
                }
            };
            while (parser::end_of_the_line(c) && file.get(c));
            if (parser::end_of_the_line(c)) {
                if (!file.get(c)){
                    return;
                }
            }
        }
    }

    void countries_list(parsing::game_map& map, std::ifstream& file) {
        parser::word key;
        std::string country_string;
        bool dynamic_tags;
        int max_employment;

        char c = ' ';

        // parse file
        while (true) {
            // parse line:
            while (true) {

                while (parser::nothing(c) && file.get(c));

                if (c != '#') {
                    key.reset();
                    while (key.parse(c) && file.get(c));
                    if (key.data == "dynamic_tags") {
                        dynamic_tags = true;
                    } else {
                        parser::string_after_equality value;
                        while (value.parse(c) && file.get(c));
                        country_string = value.data;

                        std::cout << "tag: " << key.data << "\n";
                        std::cout << "file: " << country_string << "\n";
                        std::array<int8_t, 3> tag = {
                            (key.data[0]),
                            (key.data[1]),
                            (key.data[2])
                        };
                        std::string filename = country_string.substr(10, country_string.length() - 10 - 4);
                        auto tag_id = game_definition::tag_to_int(tag);
                        game_definition::nation n {
                            tag, filename, "", dynamic_tags
                        };
                        map.nations.emplace_back(n);
                        map.tag_to_vector_position[tag_id] = map.nations.size() - 1;
                    }
                }
                while (!parser::strict_end_of_the_line(c) && file.get(c));
                while (parser::end_of_the_line(c) && file.get(c));
                if (parser::end_of_the_line(c)) {
                    if (!file.get(c)){
                        break;
                    }
                }
            }
            if (parser::end_of_the_line(c)) {
                if (!file.get(c)){
                    break;
                }
            }
        }
        return;
    }
}