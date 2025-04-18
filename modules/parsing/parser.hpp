#pragma once
#include "string"
#include <string>
#include <vector>

namespace parser {
    // idea of parsing:
    // in paradox syntax there keys and expressions
    // most of the things are written as
    // key = expression
    // expressions could have different forms which requires either detection
    // during run time or telling parser types of expression in a given context at compile time

    // overall paradox language is a shallow tree of embedded expressions,
    // so it usually should be handled with a recursive stack of parsers
    // parsers return true when they are still working
    // predicates check whatever their name says

    bool inline nothing(char c) {
        if (c == ' ')
            return true;
        if (c == '\t')
            return true;
        if (c == '\r')
            return true;
        if (c == '\n')
            return true;
        return false;
    };

    bool inline until_semicolon(char c) {
        if (c == ';')
            return false;
        return true;
    };

    struct word {
        std::string data;
        bool parse(char c) {
            if (c == ' ')
                return false;
            if (c == '\t')
                return false;
            if (c == '\r')
                return false;
            if (c == '\n')
                return false;
            if (c == '#')
                return false;
            if (c == '}')
                return false;

            data += c;
            return true;
        }

        void reset() {
            data.clear();
        }
    };

    struct quoted_string {

        int quotes_count = 0;
        std::string data;
        bool parse(char c) {
            if (c == '"')
                quotes_count++;
            else
                data += c;
            if (quotes_count == 2)
                return false;
            return true;
        }

        void reset() {
            data.clear();
        }
    };


    inline bool equality(char c) {
        if (c == ' ')
            return true;
        if (c == '\t')
            return true;
        if (c == '\r')
            return true;
        if (c == '\n')
            return true;
        if (c == '=')
            return true;

        return false;
    }

    struct word_after_equality : word {
        bool equality_gone = false;
        bool parse(char c) {
            if (equality_gone) {
                return word::parse(c);
            } else {
                if (!equality(c)) {
                    word::parse(c);
                    equality_gone = true;
                }
            }
            return true;
        }
    };

    struct string_after_equality : quoted_string {
        bool equality_gone = false;
        bool parse(char c) {
            if (equality_gone) {
                return quoted_string::parse(c);
            } else {
                if (!equality(c)) {
                    quoted_string::parse(c);
                    equality_gone = true;
                }
            }
            return true;
        }
    };


    inline bool until_open_bracket(char c) {
        if (c == '{')
            return false;
        return true;
    }
    inline bool until_close_bracket(char c) {
        if (c == '}')
            return false;
        return true;
    }

    inline bool until_brackets_balance(char c, int& balance) {
        if (c == '}')
            balance--;
        if (c == '{')
            balance--;

        if (balance == 0) {
            return false;
        }
        return true;
    }

    inline bool until_close_bracket_balance(char c, int& counter) {
        if (c == '}') {
            counter--;

            if (counter == 0)
                return false;
        }
        if (c == '{')
            counter++;

        return true;
    }

    inline bool until_end_of_the_line(char c) {
        if (c == '\r')
            return false;
        if (c == '\n')
            return false;

        return true;
    }

    inline bool until_comment(char c) {
        if (c == '#')
            return false;

        return true;
    }

    inline bool strict_end_of_the_line(char c) {
        if (c == '\r')
            return true;
        if (c == '\n')
            return true;
        return false;
    }

    inline bool end_of_the_line(char c) {
        if (c == '\r')
            return true;
        if (c == '\n')
            return true;
        if (c == ' ')
            return true;
        if (c == '\t')
            return true;

        return false;
    }

    // a group of words
    struct expression_group {
        std::vector<std::string> data;
        word current_parser;

        bool parse(char c) {
            // the only way to stop descent into madness is to find a closing bracket
            if (c == '}') {
                return false;
            }

            if (c == ' ')
                return true;
            if (c == '\t')
                return true;
            if (c == '\r')
                return true;
            if (c == '\n')
                return true;

            if (current_parser.parse(c)) {
                data.push_back(current_parser.data);
                current_parser.reset();
                return true;
            }

            return true;
        }
    };
};