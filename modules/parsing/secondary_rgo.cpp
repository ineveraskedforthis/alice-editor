#include <fstream>
#include <iostream>

#include "secondary_rgo.hpp"
#include "definitions.hpp"
#include "parser.hpp"

namespace parser {
void secondary_rgo_entry(game_definition::secondary_rgo& rgo, std::ifstream& file, char& c) {
    parser::word key;
    std::string trade_good;
    int max_employment;

    while (parser::until_open_bracket(c) && file.get(c));
    while (true) {
        while (parser::nothing(c) && file.get(c));
        if (c != '#') {
            key.reset();
            while (key.parse(c) && file.get(c));
            if (key.data == "trade_good") {
                parser::word_after_equality value;
                while (value.parse(c) && file.get(c));
                rgo.trade_good = value.data;
            } else if (key.data == "max_employment") {
                parser::word_after_equality value;
                while (value.parse(c) && file.get(c));
                rgo.size = std::stoi(value.data);
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
    return;
};


void secondary_rgo(std::vector<game_definition::secondary_rgo>& rgo_collection, std::ifstream& file, char& c) {
    parser::word key;
    while (true) {
        // start with parsing the key
        while (parser::nothing(c) && file.get(c));
        if (c != '#') {
            key.reset();
            while (key.parse(c)) {
                if (!file.get(c)) {
                    return;
                }
            }
            if (key.data == "entry") {
                game_definition::secondary_rgo rgo{};
                while (parser::equality(c) && file.get(c));
                secondary_rgo_entry(rgo, file, c);
                rgo_collection.push_back(rgo);
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
};
};