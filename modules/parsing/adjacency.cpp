#include "adjacency.hpp"
#include <charconv>
#include <iostream>
#include "parser.hpp"

namespace parser {
int adj::parse(std::ifstream& file, std::vector<game_definition::adjacency>& storage) {
    int line = 1;
    char c;
    file.get(c);
    while(true) {
        current_word.clear();
        while(parser::nothing(c) && file.get(c));
        current_word += c;

        // handle comments in the body of csv
        if (c == '#' || current_word[0] == ';') {
            current_word.clear();
            while(parser::until_end_of_the_line(c)) {
                if(!file.get(c)) {
                    return 1;
                }
            };
            while(parser::end_of_the_line(c)) {
                if(!file.get(c)) {
                    return 1;
                }
            };

            continue;
        }

        while(file.get(c) && parser::until_semicolon(c)) {
            current_word += c;
        }

        // handle first line;
        if (current_word == "From") {
            std::cout<< "csv desc detected" << std::endl;

            while(parser::until_end_of_the_line(c)) {
                if(!file.get(c)) {
                    return 1;
                }
            };
            while(parser::end_of_the_line(c)) {
                if(!file.get(c)) {
                    return 1;
                }
            };

            continue;
        }

        // now we can add new adjacency

        game_definition::adjacency def;


        def.from = std::stoi(current_word);
        current_word.clear();

        while(file.get(c) && parser::until_semicolon(c)) {
            current_word += c;
        }
        if (current_word.size() > 0) {
            def.to = std::stoi(current_word);
            std::from_chars(current_word.c_str(), current_word.c_str() + current_word.size(), def.to);
            current_word.clear();
        } else {
            std::cout << "wrong adjacency at line " << line << "\n";
            return 0;
        }

        while(file.get(c) && parser::until_semicolon(c)) {
            current_word += c;
        }

        if (current_word == "sea") {
            def.type = game_definition::ADJACENCY_TYPE::STRAIT_CROSSING;
        } else if (current_word == "canal") {
            def.type = game_definition::ADJACENCY_TYPE::CANAL;
        } else if (current_word == "impassable") {
            def.type = game_definition::ADJACENCY_TYPE::IMPASSABLE;
        } else {
            def.type = game_definition::ADJACENCY_TYPE::INVALID;
            std::cout << "wrong adjacency at line " << line << "\n";
            return 0;
        }
        current_word.clear();

        while(file.get(c) && parser::until_semicolon(c)) {
            current_word += c;
        }
        def.through = std::stoi(current_word);
        current_word.clear();

        while(file.get(c) && parser::until_end_of_the_line(c) && parser::until_comment(c)) {
            current_word += c;
        }
        def.data = current_word;
        current_word.clear();

        while(parser::until_end_of_the_line(c) && file.get(c)) {
            current_word += c;
        }
        def.comment = current_word;
        current_word.clear();
        storage.push_back(def);

        while(parser::until_end_of_the_line(c)) {
            if(!file.get(c)) {
                return 1;
            }
        };
        while(parser::end_of_the_line(c)) {
            if(!file.get(c)) {
                return 1;
            }
        };
        line++;
    }
    return 1;
}
}