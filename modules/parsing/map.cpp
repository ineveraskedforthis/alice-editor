#include "map.hpp"
#include "../editor-state/content-state.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include <filesystem>
#include "adjacency.hpp"
#include "definitions.hpp"
#include "generated/parsers.hpp"
#include "generated/parsers_core.hpp"
#include "generated/generated_parser.hpp"
#include "parser.hpp"
#include "secondary_rgo.hpp"
#include "state_building.hpp"
#include "templates.hpp"
#include "countries.hpp"

#include "../misc.hpp"
#include "../editor-state/editor-enums.hpp"

#include "SOIL2.h"

namespace parsers{
    enum class PARSER_TASK {
        READING_SPACE, READING_WORD
    };

    enum class PARSER_MODE_DEFAULT {
        NONE,
        SEA_STARTS,
        SEA_STARTS_BRACKET
    };

    struct parser_of_defaul_file {
        PARSER_MODE_DEFAULT mode;
        PARSER_TASK task;
        std::string last_word;
    };

    enum class REGIONS_PARSER_TASK {
        AWAIT_STATE_NAME,
        READING_STATE_NAME,
        AWAIT_PROVINCE_NAME,
        READING_PROVINCE_INDEX
    };


    struct parser_history_province2 {
        void parse(game_definition::province_history& p, std::ifstream& file) {
            char c = ' ';

            parser::word word;

            while (true) {
                // start with parsing the key
                while (parser::nothing(c) && file.get(c));
                if (c != '#') {
                    word.reset();
                    while (word.parse(c)) {
                        if (!file.get(c)) {
                            return;
                        }
                    }

                    if (word.data == "owner") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));
                        p.owner_tag = value.data;
                    } else if (word.data == "controller") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        p.controller_tag = value.data;
                    } else if (word.data == "state_building") {
                        while (parser::equality(c) && file.get(c));
                        parser::state_building building;
                        game_definition::state_building def;
                        building.parse(def, file, c);
                        p.buildings.push_back(def);
                    } else if (word.data == "rgo_distribution") {
                        std::vector<game_definition::secondary_rgo> rgos;
                        parser::secondary_rgo(rgos, file, c);
                        for (auto& item : rgos) {
                            if (p.secondary_rgo_size.contains((item.trade_good))) {
                                p.secondary_rgo_size[item.trade_good] += item.size;
                            } else {
                                p.secondary_rgo_size[item.trade_good] = item.size;
                            }
                        }
                    } else if (word.data == "add_core") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));
                        p.cores.push_back(value.data);
                    } else if (word.data == "terrain") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        p.terrain = value.data;
                    } else if (word.data == "trade_goods") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        p.main_trade_good = value.data;
                    } else if (word.data == "is_slave") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        p.is_slave = value.data;
                    } else if (word.data == "life_rating") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        p.life_rating = std::stoi(value.data);
                    } else if (word.data == "naval_base") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        p.naval_base = std::stoi(value.data);
                    } else if (word.data == "fort") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        p.fort = std::stoi(value.data);
                    } else if (word.data == "railroad") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        p.railroad = std::stoi(value.data);
                    } else if (word.data == "colonial") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        p.colonial = std::stoi(value.data);
                    } else if (word.data == "colony") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        p.colony = std::stoi(value.data);
                    } else {
                        if (word.data.find('.') != std::string::npos || word.data == "party_loyalty") {
                            // TODO: handle additional bookmarks
                            int counter = 0;
                            while (parser::until_close_bracket_balance(c, counter) && file.get(c));
                            file.get(c);
                        } else {
                            // std::cout << "unknown key: " << word.data << std::endl;
                        }
                    }
                }

                // std::cout << c;

                while (parser::until_end_of_the_line(c)) {
                    // std::cout << c;
                    if (!file.get(c)){
                        // std::cout << " end of file detected, return ";
                        return;
                    }
                };
                if (!parser::end_of_the_line(c)) {
                    // std::cout << " end of file detected, return ";
                    return;
                }
                while (parser::end_of_the_line(c)) {
                    // std::cout << c;
                    if (!file.get(c)){
                        return;
                    }
                }
            }
        }
    };

    struct parser_regions {
        uint32_t current_state_def;
        std::string current_word;
        REGIONS_PARSER_TASK task;

        bool reading_comment;

        void set_prov(state::layer& map) {
            if (current_word.empty()) {
                return;
            }

            auto index = std::stoi(current_word);
            current_word.clear();

            if (map.province_state[2 * index] == 0 && map.province_state[2 * index + 1] == 0) {
                map.province_state[2 * index] = current_state_def % 256;
                map.province_state[2 * index + 1] = current_state_def >> 8;
            } else {
                // todo: detect metaregions and move them elsewhere
            }
        }

        void parse_symbol(state::layer& map, char c) {
            if (c == '#') {
                reading_comment = true;
            }

            if (reading_comment) {
                if (c == '\n') {
                    reading_comment = false;
                }
                return;
            }

            switch (task) {
            case REGIONS_PARSER_TASK::AWAIT_STATE_NAME: {
                if (parser::nothing(c) || c == '=') {
                    break;
                } else {
                    current_word = c;
                    task = REGIONS_PARSER_TASK::READING_STATE_NAME;
                }
                break;
            }
            case REGIONS_PARSER_TASK::READING_STATE_NAME: {
                if (parser::nothing(c) || c == '=') {
                    game_definition::state def {current_word};
                    map.states.push_back(def);
                    current_state_def = map.states.size() - 1;
                    task = REGIONS_PARSER_TASK::AWAIT_PROVINCE_NAME;
                    current_word.clear();
                    break;
                } else {
                    current_word += c;
                }
                break;
            }
            case REGIONS_PARSER_TASK::AWAIT_PROVINCE_NAME: {
                if (parser::nothing(c) || c == '=' || c == '{') {
                    break;
                } else if (c == '}') {
                    task = REGIONS_PARSER_TASK::AWAIT_STATE_NAME;
                    set_prov(map);
                    current_word.clear();
                    break;
                } else {
                    current_word = c;
                    task = REGIONS_PARSER_TASK::READING_PROVINCE_INDEX;
                }
                break;
            }
            case REGIONS_PARSER_TASK::READING_PROVINCE_INDEX: {
                if (c == '{') {
                    current_word.clear();
                    break;
                }
                if (c == '}') {
                    task = REGIONS_PARSER_TASK::AWAIT_STATE_NAME;
                    set_prov(map);
                    current_word.clear();
                    break;
                }
                if (parser::nothing(c)) {
                    task = REGIONS_PARSER_TASK::AWAIT_PROVINCE_NAME;
                    set_prov(map);
                    current_word.clear();
                    break;
                }
                current_word += c;
                break;
            }
            }
        }
    };

    uint32_t rgb_to_uint(int r, int g, int b) {
        return (r << 16) + (g << 8) + b;
    }

    void load_provinces_map(state::layer &layer, std::string path) {
        std::cout << "loading provinces rgb map";
        int size_x;
        int size_y;
        int channels;

        auto has_alice_provinces = std::filesystem::exists(path + "/map/alice_provinces.png");
        auto has_v2_provinces = std::filesystem::exists(path + "/map/provinces.bmp");

        if (has_alice_provinces) {
            auto map = SOIL_load_image(
                (path + "/map/alice_provinces.png").c_str(),
                &size_x,
                &size_y,
                &channels,
                4
            );

            state::province_map result {
                size_x, size_y, map
            };
            result.recalculate_present_colors();
            result.update_available_colors();
            layer.provinces_image = std::move(result);
        } else if (has_v2_provinces) {
            auto map = SOIL_load_image(
                (path + "/map/provinces.bmp").c_str(),
                &size_x,
                &size_y,
                &channels,
                4
            );

            state::province_map result {
                size_x, size_y, map
            };
            result.recalculate_present_colors();
            result.update_available_colors();
            layer.provinces_image = std::move(result);
        }
        /*
        int rivers_size_x;
        int rivers_size_y;
        int rivers_channels;
        auto result_rivers = stbi_load(
            (path + "/map/alice_rivers.png").c_str(),
            &rivers_size_x,
            &rivers_size_y,
            &rivers_channels,
            4
        );

        return {
            {},
            {},
            {},
            {},
            {},
            {},
            size_x, size_y,
            result,
            (uint8_t *)calloc(size_x * size_y * 4, 1),
            result_rivers
        };
        */
    }

    void load_province_defs(state::layer &layer, std::string path) {
        std::cout << "reading /map/definition.csv\n";
        if(!std::filesystem::exists(path + "/map/definition.csv")){
            std::cout << "file was not found, skip\n";
            return;
        }

        std::ifstream file(path + "/map/definition.csv");
        std::string str;

        layer.has_province_definitions = true;

        std::getline(file, str);

        while (std::getline(file, str)) {
            auto end = str.find(";");
            auto index_str = str.substr(0, end);
            int index;

            try {
                index = std::stoi(str.substr(0, end));
                str.erase(0, end + 1);
            } catch (const std::invalid_argument e) {
                continue;
            }

            end = str.find(";");
            auto r = std::stoi(str.substr(0, end));
            str.erase(0, end + 1);

            end = str.find(";");
            auto g = std::stoi(str.substr(0, end));
            str.erase(0, end + 1);

            end = str.find(";");
            auto b = std::stoi(str.substr(0, end));
            str.erase(0, end + 1);

            end = str.find(";");
            std::string name = str.substr(0, end);

            if (index) {
                layer.is_used[index] = true;
                game_definition::province def {
                    (uint32_t)(index),
                    name,
                    (uint8_t)(r),
                    (uint8_t)(g),
                    (uint8_t)(b)
                };
                layer.province_definitions.push_back(def);
                layer.rgb_to_v2id.insert_or_assign(rgb_to_uint(r, g, b), (uint32_t)(index));
                layer.v2id_to_vector_position.insert_or_assign((uint32_t)(index), layer.province_definitions.size() - 1);
            }
        }
    }

    void load_default_dot_map(state::layer &layer, std::string path) {
        std::cout << "reading /map/default.map\n";
        if(!std::filesystem::exists(path + "/map/default.map")){
            std::cout << "file was not found, skip\n";
            return;
        }

        layer.has_default_map = true;

        std::ifstream file(path + "/map/default.map");
        parser_of_defaul_file parser {};
        char c;
        bool found = false;
        while (file.get(c) && !found) {
            if (c == '#') {
                while(parser::until_end_of_the_line(c) && file.get(c));
            }
            if (parser::nothing(c)) {
                if (parser.task == PARSER_TASK::READING_SPACE) continue;
                else if (parser.task == PARSER_TASK::READING_WORD) {
                    switch (parser.mode) {
                        case PARSER_MODE_DEFAULT::NONE: {
                            if (parser.last_word == "sea_starts") {
                                parser.mode = PARSER_MODE_DEFAULT::SEA_STARTS;
                            }
                            break;
                        }
                        case PARSER_MODE_DEFAULT::SEA_STARTS: {
                            if (parser.last_word == "{") {
                                parser.mode = PARSER_MODE_DEFAULT::SEA_STARTS_BRACKET;
                            }
                            break;
                        }
                        case PARSER_MODE_DEFAULT::SEA_STARTS_BRACKET: {
                            if (parser.last_word == "}") {
                                found = true;
                            } else {
                                auto index = std::stoi(parser.last_word);
                                layer.province_is_sea[index] = 255;
                            }
                            break;
                        }
                    }
                    parser.last_word.clear();
                    parser.task = PARSER_TASK::READING_SPACE;
                }
            } else  {
                if (parser.task == PARSER_TASK::READING_SPACE) {
                    parser.task = PARSER_TASK::READING_WORD;
                }
                if (parser.task == PARSER_TASK::READING_WORD) {
                    parser.last_word += c;
                }
            }
        }
    }

    void load_regions(state::layer& layer, std::string path) {
        std::cout << "Parse state membership \n";

        bool areas_file_exist = std::filesystem::exists(path + "/map/area.txt");
        bool region_file_exist = std::filesystem::exists(path + "/map/region.txt");

        std::ifstream file;

        if (areas_file_exist) {
            std::ifstream file(path + "/map/area.txt");
        } else if (region_file_exist) {
            file.open(path + "/map/region.txt");
        } else {
            std::cout << "State membership not found \n";
            return;
        }

        layer.has_region_txt = true;

        std::string str;
        char c;
        layer.states.push_back({"INVALID"});
        parser_regions parser {};
        std::cout << "parsing states\n";
        if (file) {
            while (file.get(c)) {
                parser.parse_symbol(layer, c);
            }
        } else {
            std::cout << "bad states/areas" << std::endl;
        }
    }

    void load_adjacencies(state::layer& layer, std::string path) {
        std::cout << "Parse adjacencies\n";

        if (!std::filesystem::exists(path + "/map/adjacencies.csv")) {
            std::cout << "Not found\n";
            return;
        }

        std::ifstream file(path + "/map/adjacencies.csv");
        std::string str;
        char c;
        parser::adj parser {};
        if (file) {
            layer.has_adjacencies = true;
            parser.parse(file, layer.adjacencies);
        } else
            std::cout << "bad /map/adjacencies.csv" << std::endl;
    }

    void load_templates(state::editor& editor_data, std::string path) {
        std::cout << "reading editor templates\n";
        std::ifstream file(path + "/editor-templates/secondary_rgo.txt");

        if (file) {
            std::cout << "parsing rgo templates\n";
            parser::secondary_rgo_template_file(editor_data, file);
        } else
            std::cout << "no rgo distribution templates found" << std::endl;
    }

    void unload_governments_list(state::layer &layer, std::string path) {
        if (!layer.has_governments_list) {
            return;
        }
        std::filesystem::create_directory(path + "/common");
        std::ofstream file(path + "/common/governments.txt");

        for(auto& g : layer.governments){
            file << g.name << " = {\n";

            for (auto& [key, value] : g.allowed_parties ) {
                if (value) {
                    file << "\t" << key << " = yes\n";
                }
            }

            if (g.appoint_ruling_party) {
                file << "\tappoint_ruling_party = yes\n";
            } else {
                file << "\tappoint_ruling_party = no\n";
            }
            if (g.election) {
                file << "\telection = yes\n";
                file << "\tduration = " << g.duration << "\n";
            } else {
                file << "\telection = no\n";
            }
            if (g.flagtype.length() > 0)
                file << "\tflagType = " << g.flagtype << "\n";
            file << "}\n";
        }
    }

    void load_governments_list(state::layer &layer, std::string path, parsers::error_handler& errors) {
        std::cout << "Parse government types\n";
        if (!std::filesystem::exists(path + "/common/governments.txt")) {
            std::cout << "Not found\n";
            return;
        }

        layer.has_governments_list = true;

        std::ifstream file(path + "/common/governments.txt");
        parsers::generic_context ctx_generic {
            layer
        };

        std::stringstream buffer;
        buffer << file.rdbuf();
        auto str = buffer.str();
        parsers::token_generator tk(str.c_str(), str.c_str() + buffer.str().length());
        parsers::parse_governments_file(tk, errors, ctx_generic);
    };

    void load_issues_list(state::layer &layer, std::string path, parsers::error_handler& errors) {
        std::cout << "Parse issues\n";
        if (!std::filesystem::exists(path + "/common/issues.txt")) {
            std::cout << "Not found\n";
            return;
        }

        layer.has_issues = true;

        std::ifstream file(path + "/common/issues.txt");
        parsers::generic_context ctx_generic {
            layer
        };

        std::stringstream buffer;
        buffer << file.rdbuf();
        auto str = buffer.str();
        parsers::token_generator tk(str.c_str(), str.c_str() + buffer.str().length());
        parsers::parse_issues_file(tk, errors, ctx_generic);
    };

    void load_technology_list(state::layer &layer, std::wstring path, parsers::error_handler& errors) {
        std::cout << "Parse technologies\n";

        if (!std::filesystem::exists(path + L"/technologies")) {
            std::cout << "Techs not found\n";
            return;
        }

        for (auto& entry : std::filesystem::directory_iterator  {path + L"/technologies"}) {
            if (!entry.is_directory() && entry.path().filename().string().ends_with(".txt")) {
                auto filename = entry.path().filename().wstring();
                std::wcout << filename << L" was found\n";
                std::ifstream file(entry.path());
                parsers::technology_context ctx {
                    layer, filename
                };
                layer.has_tech[filename] = true;
                std::stringstream buffer;
                buffer << file.rdbuf();
                auto str = buffer.str();
                parsers::token_generator tk(str.c_str(), str.c_str() + buffer.str().length());
                parsers::parse_technology_sub_file(tk, errors, ctx);
            }
        }
    };

    void load_inventions_list(state::layer &layer, std::wstring path, parsers::error_handler& errors) {
        std::cout << "Parse inventions\n";

        if (!std::filesystem::exists(path + L"/inventions")) {
            std::cout << "Inventions not found\n";
            return;
        }

        for (auto& entry : std::filesystem::directory_iterator  {path + L"/inventions"}) {
            if (!entry.is_directory() && entry.path().filename().string().ends_with(".txt")) {
                auto filename = entry.path().filename().wstring();
                std::wcout << filename << L" was found\n";
                std::ifstream file(entry.path());
                layer.has_invention[filename] = true;
                parsers::technology_context ctx {
                    layer, filename
                };
                std::stringstream buffer;
                buffer << file.rdbuf();
                auto str = buffer.str();
                parsers::token_generator tk(str.c_str(), str.c_str() + buffer.str().length());
                parsers::parse_inventions_file(tk, errors, ctx);
            }
        }
    };

    void unload_nations_list(state::layer &layer, std::string path) {
        if (!layer.has_nations_list) {
            return;
        }
        std::filesystem::create_directory(path + "/common");
        std::ofstream file(path + "/common/countries.txt");

        for(auto& g : layer.nations){
            if (g.dynamic) continue;
            file << g.tag[0] << g.tag[1] << g.tag[2] << "\t= " << "\"countries/" << g.filename << "\"\n";
        }
        file << "dynamic_tags = yes\n";
        for(auto& g : layer.nations){
            if (!g.dynamic) continue;
            file << g.tag[0] << g.tag[1] << g.tag[2] << "\t= " << "\"countries/" << g.filename << "\"\n";
        }
    }

    void load_nations_list(state::layer &layer, std::string path, parsers::error_handler& errors) {
        std::cout << "Parse nations list\n";
        if (!std::filesystem::exists(path + "/common/countries.txt")) {
            std::cout << "Not found\n";
            return;
        }

        layer.has_nations_list = true;

        std::ifstream file(path + "/common/countries.txt");
        parser::countries_list(layer, file);
    }

    void unload_nations_common(state::layer &layer, std::string path) {
        std::filesystem::create_directory(path + "/common/");
        std::filesystem::create_directory(path + "/common/countries");

        for (auto& [key, value] : layer.filename_to_nation_common) {
            std::ofstream file(path + "/common/countries/" + key);

            file << "color = { " << (int)value.R << " " << (int)value.G << " " << (int)value.B << " }\n";
            file << "graphical_culture = " << value.graphical_culture << "\n";
            for (auto& party : value.parties) {
                file << "party = {\n";
                file << "\tname = \"" << party.name << "\"\n";
                file << "\tstart_date = " << party.start << "\n";
                file << "\tend_date = " << party.end << "\n";
                file << "\tideology = " << party.ideology << "\n";
                file << "\teconomic_policy = " << party.economic_policy  << "\n";
                file << "\ttrade_policy = " << party.trade_policy  << "\n";
                file << "\treligious_policy = " << party.religious_policy  << "\n";
                file << "\tcitizenship_policy = " << party.citizenship_policy  << "\n";
                file << "\twar_policy = " << party.war_policy  << "\n";
                file << "}\n";
            }

            file << "unit_names = {";
            file << value.unit_names;
            file << "}";
        }
    }

    void load_nations_common(state::layer &layer, std::string path, parsers::error_handler& errors) {
        std::cout << "Parse nations common definitions\n";
        if (!std::filesystem::exists(path + "/common/countries")) {
            std::cout << path + "/common/countries" << " is missing." << "\n";
            return;
        }
        for (auto& entry : std::filesystem::directory_iterator  {path + "/common/countries"}) {
            if (!entry.is_directory() && entry.path().filename().string().ends_with(".txt")) {
                auto name = entry.path().filename().string();
                game_definition::nation_common n{};
                std::ifstream file(entry.path());
                parser::country_file_common(n, file, name);
                layer.filename_to_nation_common[name] = n;
            }
        }
    }

    void unload_nation_history(state::layer &layer, std::string path) {
        std::filesystem::create_directory(path + "/history/");
        std::filesystem::create_directory(path + "/history/countries");

        for (auto & [key, value] : layer.tag_to_nation_history) {
            auto tag = game_definition::int_to_tag(key);
            std::ofstream file(path + "/history/countries/" + value.history_file_name);

            file << "primary_culture = " << value.primary_culture << "\n";
            for (auto & culture : value.culture)
                if (culture.length() > 0)
                    file << "culture = " << culture << "\n";
            file << "religion = " << value.religion << "\n";
            file << "government = " << value.government << "\n";
            file << "plurality = " << value.plurality << "\n";
            file << "prestige = " << value.prestige << "\n";
            file << "nationalvalue = " << value.nationalvalue << "\n";
            file << "literacy = " << value.literacy << "\n";
            file << "non_state_culture_literacy = " << value.non_state_culture_literacy << "\n";
            if (value.civilized) {
                file << "civilized = yes\n";
            } else {
                file << "civilized = no\n";
            }
            if (value.is_releasable_vassal) {
                file << "is_releasable_vassal = yes\n";
            } else {
                file << "is_releasable_vassal = no\n";
            }
            for (auto & flag :value.govt_flag) {
                file << "govt_flag = {\n";
                file << "\tgovernment = " << flag.government << "\n";
                file << "\tflag = " << flag.flag << "\n";
                file << "}\n";
            }
            if (!value.foreign_investment.empty()) {
                file << "foreign_investment = {\n";
                for (auto & [key, value] : value.foreign_investment)
                    file << "\t" << key << " = " << value << "\n";
                file << "}\n";
            }
            if (!value.upper_house.empty()) {
                file << "upper_house = {\n";
                for (auto & [key, value] : value.upper_house)
                    file << "\t" << key << " = " << value << "\n";
                file << "}\n";
            }
            file << "ruling_party = " << value.ruling_party << "\n";
            if (!value.schools.empty())
                file << "schools = " << value.schools << "\n";
            file << "consciousness = " << value.consciousness << "\n";
            file << "nonstate_consciousness = " << value.nonstate_consciousness << "\n";
            if (!value.last_election.empty())
                file << "last_election = " << value.last_election << "\n";
            if (!value.oob.empty())
                file << "oob = " << value.oob << "\n";
            if (value.capital)
                file << "capital = " << value.capital << "\n";
            file << "colonial_points = " << value.colonial_points << "\n";
            for (auto & flag : value.set_country_flag)
                file << "set_country_flag = " << flag << "\n";
            for (auto & flag : value.set_global_flag)
                file << "set_global_flag = " << flag << "\n";
            for (auto & dec : value.decision)
                file << "decision = " << dec << "\n";

            for (auto & [key, value] : value.tech) {
                if (value)
                    file << key << " = yes\n";
            }
            for (auto & [key, value] : value.inventions) {
                if (value)
                    file << key << " = yes\n";
            }
            for (auto & [key, value] : value.issues) {
                if (value.size() > 0) {
                    file << key << " = " << value << "\n";
                }
            }
        }
    }

    void load_nation_history(state::layers_stack& state, state::layer &layer, std::string path, parsers::error_handler& errors) {
        std::cout << "Parse nations history\n";
        if (!std::filesystem::exists(path + "/history" + "/countries")) {
            return;
        }
        for (auto& entry : std::filesystem::directory_iterator  {path + "/history" + "/countries"}) {
            if (!entry.is_directory() && entry.path().filename().string().ends_with(".txt")) {
                errors.file_name = entry.path().filename().string();
                auto name = entry.path().filename().string();
                // std::cout << name << std::endl;
                auto first_space = name.find_first_of(' ');
                auto tag_string = name.substr(0, first_space);
                auto tag_int = game_definition::tag_to_int({
                    tag_string[0], tag_string[1], tag_string[2]
                });

                game_definition::nation_history n{};
                n.history_file_name = name;
                std::ifstream file(entry.path());

                parsers::nation_history_file context {
                    n,
                    state,
                    layer
                };

                std::stringstream buffer;
                buffer << file.rdbuf();
                auto str = buffer.str();
                parsers::token_generator tk(str.c_str(), str.c_str() + buffer.str().length());
                parsers::parse_nation_handler(tk, errors, context);

                layer.tag_to_nation_history[tag_int] = n;
            }
        }
    }

    void load_province_history(state::layer &layer, std::string path, parsers::error_handler& errors) {
        std::cout << "reading province history\n";
        if (!std::filesystem::exists(path + "/history" + "/provinces")) {
            return;
        }
        for (auto& entry : std::filesystem::directory_iterator  {path + "/history" + "/provinces"}) {
            if (!entry.is_directory() && entry.path().filename().string().ends_with(".txt")) {
                auto name = entry.path().filename().wstring();
                errors.file_name = conversions::wstring_to_utf8(name);
                // std::cout << name << std::endl;
                auto first_space = name.find_first_of(' ');
                auto id_string = name.substr(0, first_space);
                auto id = std::stoi(id_string);
                std::cout << id << " ";

                game_definition::province_history p{};
                p.history_file_name = name;
                p.historical_region = L"other";

                parser_history_province2 parser {};
                std::ifstream file(entry.path());
                parser.parse(p, file);

                layer.province_history[id] = p;

                continue;
            }

            for (auto& province_description : std::filesystem::directory_iterator(entry.path())) {
                auto name = province_description.path().filename().wstring();
                errors.file_name = conversions::wstring_to_utf8(name);
                // std::cout << name << std::endl;
                auto first_space = name.find_first_of(' ');
                auto id_string = name.substr(0, first_space);
                try {
                    auto id = std::stoi(id_string);

                    // std::cout << id << " ";

                    game_definition::province_history p{};
                    p.history_file_name = name;
                    p.historical_region = entry.path().filename().wstring();

                    parser_history_province2 parser {};
                    std::ifstream file(province_description.path());
                    parser.parse(p, file);

                    layer.province_history[id] = p;
                } catch (const std::invalid_argument & e) {
                    std::cout << e.what() << "\n";
                }
            }
        }
    }

    void load_province_population(state::layer &layer, std::string path, parsers::error_handler& errors) {
        std::cout << "reading population history\n";
        if (!std::filesystem::exists(path + "/history" + "/pops")) {
            return;
        }
        for (auto& entry : std::filesystem::directory_iterator  {path + "/history" + "/pops"}) {
            if (!entry.is_directory()) {
                continue;
            }

            auto date = entry.path().filename().string();
            auto first_dot = date.find_first_of('.');
            auto year_s = date.substr(0, first_dot);
            auto year = std::stoi(year_s);

            date = date.substr(first_dot + 1, date.size() - first_dot - 1);
            auto second_dot = date.find_first_of('.');
            auto month_s = date.substr(0, second_dot);
            auto month = std::stoi(month_s);

            date = date.substr(second_dot + 1, date.size() - second_dot - 1);
            auto day_s = date.substr(0, second_dot);
            auto day = std::stoi(month_s);

            auto date_i = year * 31*12 + month * 31 + day;

            game_definition::pops_setups new_pop_data {};
            new_pop_data.date = date_i;

            for (auto& pops_lump : std::filesystem::directory_iterator(entry.path())) {
                if (pops_lump.is_directory() || !pops_lump.path().filename().string().ends_with(".txt")) {
                    continue;
                }

                auto filename = pops_lump.path().filename().string();
                errors.file_name = pops_lump.path().filename().string();

                std::ifstream file(pops_lump.path());

                game_definition::pops_history_file file_content {};
                file_content.filename = filename;

                parsers::pop_history_file_context context {
                    layer,
                    file_content,
                    date_i,
                };

                std::stringstream buffer;
                buffer << file.rdbuf();
                auto str = buffer.str();
                parsers::token_generator tk(str.c_str(), str.c_str() + buffer.str().length());

                parse_pop_history_file(tk, errors, context);

                new_pop_data.data.push_back(file_content);
            }

            layer.province_population.push_back(new_pop_data);
        }
    }

    void register_pop_types(state::layer &layer, std::string path) {
        std::cout << "registration of poptypes\n";
        if (!std::filesystem::exists(path + "/poptypes")) {
            return;
        }
        for (auto& entry : std::filesystem::directory_iterator  {path + "/poptypes"}) {
            if (!entry.is_directory() && entry.path().filename().string().ends_with(".txt")) {
                auto name = entry.path().filename().string();
                layer.poptypes.push_back(name.substr(0, name.size() - 4));
            }
        }
    }

    void load_cultures(state::layer &layer, std::string path, parsers::error_handler& errors) {
        std::cout << "registration of cultures\n";
        if (!std::filesystem::exists(path + "/common/cultures.txt")) {
            std::cout << "no cultures found\n";
            return;
        }

        errors.file_name = "/common/cultures.txt";

        layer.has_cultures = true;

        std::ifstream file(path + "/common/cultures.txt");
        std::stringstream buffer;
        buffer << file.rdbuf();
        auto str = buffer.str();
        parsers::token_generator tk(str.c_str(), str.c_str() + buffer.str().length());

        culture_file_context ctx {
            layer
        };

        parsers::parse_culture_file(tk, errors, ctx);
    }

    void unload_cultures(state::layer &layer, std::string path) {
        if (!layer.has_cultures) return;
        std::filesystem::create_directory(path + "/common");
        std::ofstream file(path + "/common/cultures.txt");
        for (auto& group: layer.culture_groups) {
            auto& def = layer.culture_group_defs[group];
            file << group << " = {\n";
            if (def.leader.size() > 0) {
                file << "\tleader = " << def.leader << "\n";
            }
            if (def.unit.size() > 0) {
                file << "\tunit = " << def.unit << "\n";
            }
            if (def.union_tag.size() > 0) {
                file << "\tunion = " << def.union_tag << "\n";
            }
            if (!def.is_overseas) {
                file << "\tis_overseas = no\n";
            }
            for (auto& culture : def.cultures) {
                auto& culture_def = layer.culture_defs[culture];
                file << "\t" << culture << " = {\n";

                file << "\t\tcolor = { " << culture_def.r << " " << culture_def.g << " " << culture_def.b << " }\n";

                file << "\t\tfirst_names = {";
                for (int i = 0; i < culture_def.first_names.size(); i++) {
                    if (i % 5 == 0) {
                        file << "\n\t\t\t";
                    }
                    file << culture_def.first_names[i] << " ";
                }
                file << "\n";
                file << "\t\t}\n";

                file << "\t\tlast_names = {";
                for (int i = 0; i < culture_def.last_names.size(); i++) {
                    if (i % 5 == 0) {
                        file << "\n\t\t\t";
                    }
                    file << culture_def.last_names[i] << " ";
                }
                file << "\n";
                file << "\t\t}\n";

                file << "\t}\n";
            }
            file << "}\n";
        }
    }

    void register_religions(state::layer &layer, std::string path, parsers::error_handler& errors) {
        std::cout << "registration of religions\n";
        if (!std::filesystem::exists(path + "/common/religion.txt")) {
            std::cout << "no religions found\n";
            return;
        }

        errors.file_name = "/common/religion.txt";

        layer.has_religions = true;

        std::ifstream file(path + "/common/religion.txt");
        std::stringstream buffer;
        buffer << file.rdbuf();
        auto str = buffer.str();
        parsers::token_generator tk(str.c_str(), str.c_str() + buffer.str().length());

        generic_context ctx {
            layer
        };

        parsers::parse_religion_file(tk, errors, ctx);
    }

    void unload_province_defs(state::layer &layer, std::string path) {
        if (!layer.has_province_definitions) return;
        std::filesystem::create_directory(path + "/map");

        std::cout << "write definitions";
        std::ofstream file(path + "/map/definition.csv");
        file << "province;red;green;blue;x;x\n";
        for (auto i = 0; i < layer.province_definitions.size(); i++) {
            auto prov = layer.province_definitions[i];
            auto rgb = rgb_to_uint(prov.r, prov.g, prov.b);
            auto index = layer.rgb_to_v2id[rgb];

            file
                << prov.v2id << ";"
                << (int)prov.r << ";"
                << (int)prov.g << ";"
                << (int)prov.b << ";"
                << prov.name << ";"
                << "x\n";
        }
    };

    void unload_province_map(state::layer &layer, std::string path) {
        if(layer.provinces_image == std::nullopt) return;
        std::filesystem::create_directory(path + "/map");

        std::cout << "write provinces image";
        SOIL_save_image(
            (path + "/map/alice_provinces.png").c_str(),
            SOIL_SAVE_TYPE_PNG,
            layer.provinces_image->size_x,
            layer.provinces_image->size_y,
            4,
            layer.provinces_image->provinces_image_data
        );
    }

    void unload_default_dot_map(state::layer &layer, std::string path) {
        if(!layer.has_default_map) return;
        if(!layer.has_province_definitions) return;
        std::filesystem::create_directory(path + "/map");
        int available_id = 1;
        while (layer.is_used[available_id])
            available_id += 1;
        std::cout << "write default map file (list of sea provs)";
        std::ofstream file(path + "/map/default.map");
        file << "max_provinces = " << available_id << "\n";
        file << "sea_starts = {";
        int counter = 0;
        int v2id = 1;
        while (layer.is_used[v2id]) {
            if (layer.province_is_sea[v2id] > 0){
                if (counter % 10 == 0) {
                    file << "\n";
                }
                counter++;
                file << v2id << " ";
            }
            v2id++;
        }
        file << "\n}\n";
        file << R""""(
definitions = "definition.csv"
provinces = "provinces.bmp"
positions = "positions.txt"
terrain = "terrain.bmp"
rivers = "rivers.bmp"
terrain_definition = "terrain.txt"
tree_definition = "trees.txt"
continent = "continent.txt"
adjacencies = "adjacencies.csv"
region = "region.txt"
region_sea = "region_sea.txt"
province_flag_sprite = "province_flag_sprites"
border_heights = {
	500
	800
	1100
}
terrain_sheet_heights = {
	500 #150
}
tree = 350
border_cutoff = 1100.0
)"""";
    }

    void unload_regions(state::layer& layer, std::string path) {
        if (!layer.has_region_txt) return;

        std::filesystem::create_directory(path + "/map");
        std::ofstream file(path + "/map/region.txt");
        std::cout << "init collections of provs in a state\n";
        std::vector<std::vector<uint32_t>> states_to_provs;
        for (auto i = 0; i < layer.states.size(); i++) {
            states_to_provs.push_back({});
        }

        std::cout << "populate collections of provs in a state\n";
        for (int v2id = 1; v2id < 256 * 256; v2id++) {
            int state_x = layer.province_state[2 * v2id];
            int state_y = layer.province_state[2 * v2id + 1];
            auto state = state_x + state_y * 256;
            if (state) {
                states_to_provs[state].push_back(v2id);
            }
        }

        std::cout << "save collections of provs in a state\n";
        for (auto i = 1; i < layer.states.size(); i++) {
            auto state = layer.states[i];
            file << state.name << " = { ";
            for (auto j = 0; j < states_to_provs[i].size(); j++) {
                file << states_to_provs[i][j] << " ";
            }
            file << "}\n";
        }
    }

    void unload_adjacencies(state::layer& layer, std::string path) {
        if (!layer.has_adjacencies) return;
        std::filesystem::create_directory(path + "/map");
        std::ofstream file(path + "/map/adjacencies.csv");
        for (auto& adj: layer.adjacencies) {
            std::string type;

            switch (adj.type) {
            case game_definition::ADJACENCY_TYPE::INVALID:
            type = "invalid";
            break;
            case game_definition::ADJACENCY_TYPE::STRAIT_CROSSING:
            type = "sea";
            break;
            case game_definition::ADJACENCY_TYPE::CANAL:
            type = "canal";
            break;
            case game_definition::ADJACENCY_TYPE::IMPASSABLE:
            type = "impassable";
            break;
            }

            if (!adj.mark_for_delete) {
                file << adj.from << ";" << adj.to << ";" << type << ";" << adj.through << ";" << adj.data << "#" << adj.comment << "\n";
            }
        }
    }

    void unload_province_population(state::layer& layer, std::string path) {
        std::filesystem::create_directory(path + "/history");
        std::filesystem::create_directory(path + "/history/pops");
        for (auto & folder : layer.province_population) {
            auto date = folder.date;
            auto year = date / (31 * 12);
            date -= year * 31 * 12;
            auto month = date / 31;
            date -= month * 31;
            auto day = date;

            std::string folder_name =
                std::to_string(year) + "."
                + std::to_string(month) + "."
                + std::to_string(day);
            auto dated_path = path + "/history/pops/" + folder_name;

            std::filesystem::create_directory(dated_path);

            for (auto& pops_file : folder.data) {
                std::ofstream file(dated_path + "/" + pops_file.filename);
                for (auto& [v2id, pops] : pops_file.data) {
                    file << v2id << " = {\n";
                    for (auto& pop : pops) {
                        file << "\t" << pop.poptype << " = {\n";
                        file << "\t\tculture = " << pop.culture << "\n";
                        file << "\t\treligion = " << pop.religion << "\n";
                        if (pop.militancy != 0.f)
                            file << "\t\tmilitancy = " << pop.militancy << "\n";
                        if (pop.rebel_type.size() > 0)
                            file << "\t\trebel_type = " << pop.rebel_type << "\n";
                        file << "\t\tsize = " << pop.size << "\n";
                        file << "\t}\n";
                    }
                    file << "}\n";
                }
            }
        }
    }

    void unload_province_history(state::layer& layer, std::wstring path){
        if(layer.province_history.empty()) return;
        std::filesystem::create_directory(path + L"/history");
        std::filesystem::create_directory(path + L"/history/provinces");

        for (auto& [key, val] : layer.province_history) {
            auto folder_path = path + L"/history/provinces/" + val.historical_region;
            std::filesystem::create_directory(folder_path);
            std::ofstream file(folder_path + L"/" + val.history_file_name);

            if (val.owner_tag.length() > 0)
                file << "owner = " << val.owner_tag << std::endl;

            if (val.controller_tag.length() > 0)
                file << "controller = " << val.controller_tag << std::endl;

            for (auto& core : val.cores) {
                if (core.length() == 0) continue;
                file << "add_core = " << core << std::endl;
            }

            if (val.main_trade_good.length() > 0)
                file << "trade_goods = " << val.main_trade_good << std::endl;


            file << "colonial = " << val.colonial << std::endl;
            file << "colony = " << val.colony << std::endl;
            file << "life_rating = " << val.life_rating << std::endl;
            file << "naval_base = " << val.naval_base << std::endl;
            file << "railroad = " << val.railroad << std::endl;
            file << "fort = " << val.fort << std::endl;

            for (auto& building : val.buildings) {
                file << "state_building = {" << std::endl;
                file << "\tlevel = " << building.level << std::endl;
                file << "\tbuilding = " << building.building_type << std::endl;
                file << "\tupgrade = " << building.upgrade << std::endl;
                file << "}" << std::endl;
            }

            std::vector<std::string> local_rgo;
            for (auto const& [key, val] : val.secondary_rgo_size) {
                local_rgo.push_back(key);
            }

            if (local_rgo.size() > 0) {
                file << "rgo_distribution = {" << std::endl;

                for (auto key : local_rgo) {
                    file << "\tentry = {" << std::endl;
                    file << "\t\ttrade_good = " << key << std::endl;
                    file << "\t\tmax_employment = " << val.secondary_rgo_size[key] << std::endl;
                    file << "\t}" << std::endl;
                }

                file << "}" << std::endl;
            }

            if (val.terrain.length() > 0)
                file << "terrain = " << val.terrain << std::endl;
            if (val.is_slave.length() > 0)
                file << "is_slave = " << val.is_slave << std::endl;
        }
    };


    void unload_flags(state::layer& layer, std::string path, state::FLAG_EXPORT_OPTIONS export_option) {
        // keys: "/gfx/flags/" + string_tag + ".tga"
        std::filesystem::create_directory(path + "gfx");
        std::filesystem::create_directory(path + "gfx/flags");
        for (auto& [key, flag_path] : layer.paths_to_new_flags) {
            // load flag:
            if (!std::filesystem::exists(flag_path)) {
                continue;
            }
            int size_x;
            int size_y;
            int channels;
            auto flag_data = SOIL_load_image(
                conversions::wstring_to_utf8(flag_path).c_str(),
                &size_x,
                &size_y,
                &channels,
                3
            );
            if (export_option == state::FLAG_EXPORT_OPTIONS::TGA) {
                SOIL_save_image(
                    (path + "gfx/flags/" + key + ".tga").c_str(),
                    SOIL_SAVE_TYPE_TGA,
                    size_x,
                    size_y,
                    3,
                    flag_data
                );
            } else {
                SOIL_save_image(
                    (path + "gfx/flags/" + key + ".png").c_str(),
                    SOIL_SAVE_TYPE_PNG,
                    size_x,
                    size_y,
                    3,
                    flag_data
                );
            }
            delete []flag_data;
        }
    }

    void load_core_gfx(state::layer& layer, std::string path, parsers::error_handler& errors) {
        std::cout << "Parse core.gfx\n";
        std::cout << path + "/interface/core.gfx" << "\n";
        if (!std::filesystem::exists(path + "/interface/core.gfx")) {
            std::cout << "Not found\n";
            return;
        }

        layer.has_core_gfx = true;
        std::ifstream file(path + "/interface/core.gfx");
        parsers::generic_context ctx_generic {
            layer
        };

        errors.file_name = "/interface/core.gfx";

        std::stringstream buffer;
        buffer << file.rdbuf();
        auto str = buffer.str();
        parsers::token_generator tk(str.c_str(), str.c_str() + buffer.str().length());

        parsers::parse_core_gfx_file(tk, errors, ctx_generic);
    }

    void write_gfx(game_definition::sprite& item, std::ofstream& file, int amount_of_commodities) {
        file << "\t\tname = " << item.name << "\n";
        bool is_commodity = false;
        if (item.texturefile.size() > 0) {
            if (item.texturefile.ends_with("resources.dds")) {
                is_commodity = true;
            }
            if (item.texturefile.ends_with("resources_big.dds")) {
                is_commodity = true;
            }
            if (item.texturefile.ends_with("resources_small.dds")) {
                is_commodity = true;
            }
            file << "\t\ttexturefile = \"" << item.texturefile << "\"\n";
        }
        if (item.texturefile1.size() > 0) {
            file << "\t\ttextureFile1 = \"" << item.texturefile << "\"\n";
        }
        if (item.texturefile2.size() > 0) {
            file << "\t\ttextureFile2 = \"" << item.texturefile << "\"\n";
        }
        if (item.noofframes > 0) {
            if (!is_commodity)
                file << "\t\tnoOfFrames = " << item.noofframes << "\n";
            else {
                file << "\t\tnoOfFrames = " << amount_of_commodities + 1 << "\n";
            }
        }
        if (item.loadtype.size() > 0) {
            file << "\t\tloadType = \"" << item.loadtype << "\"\n";
        }
        if (item.effectfile.size() > 0) {
            file << "\t\teffectFile = \"" << item.effectfile << "\"\n";
        }
        if (item.clicksound.size() > 0) {
            file << "\t\tclicksound = " << item.clicksound << "\n";
        }
        if (item.transparencecheck) {
            file << "\t\ttransparencecheck = yes\n";
        }
        if (item.norefcount) {
            file << "\t\tnorefcount = yes\n";
        }
        if (item.allwaystransparent) {
            file << "\t\tallwaystransparent = yes\n";
        }
        if (item.has_size) {
            file << "\t\tsize = { x = " << item.size_x << " y = " << item.size_y <<  " }\n";
        }
        if (item.has_border_size) {
            file << "\t\tborderSize = { x = " << item.border_size_x << " y = " << item.border_size_y <<  " }\n";
        }
    }

    void unload_core_gfx(state::layer& layer, std::string path, int amount_of_commodities) {
        std::cout << "Write core.gfx\n";
        if (!layer.has_core_gfx) {
            return;
        }

        std::filesystem::create_directory(path + "/interface");
        std::ofstream file(path + "/interface/core.gfx");

        file << "spriteTypes = {\n";

        for (auto & item : layer.sprites) {
            file << "\tspriteType = {\n";
            write_gfx(item, file, amount_of_commodities);
            file << "\t}\n";
        }
        for (auto & item : layer.text_sprites) {
            file << "\ttextSpriteType = {\n";
            write_gfx(item, file, amount_of_commodities);
            file << "\t}\n";
        }
        for (auto & item : layer.masked_shields) {
            file << "\tmaskedShieldType = {\n";
            write_gfx(item, file, amount_of_commodities);
            file << "\t}\n";
        }
        for (auto & item : layer.cornered_sprites) {
            file << "\tcorneredTileSpriteType = {\n";
            write_gfx(item, file, amount_of_commodities);
            file << "\t}\n";
        }

        file << "}\n";

        for (auto & item : layer.lightTypes_text) {
            file << "lightTypes = " << item << "\n";
        }
        for (auto & item : layer.objectTypes_text) {
            file << "objectTypes = " << item << "\n";
        }
        for (auto & item : layer.bitmapfonts_text) {
            file << "bitmapfonts = " << item << "\n";
        }
        for (auto & item : layer.bitmapfont_text) {
            file << "bitmapfont = " << item << "\n";
        }
        for (auto & item : layer.fonts_text) {
            file << "fonts = " << item << "\n";
        }
    }

    void load_goods(state::layer& layer, std::string path, parsers::error_handler& errors) {
        std::cout << "Parse goods.txt\n";
        std::cout << path + "/common/goods.txt" << "\n";
        if (!std::filesystem::exists(path + "/common/goods.txt")) {
            std::cout << "Not found\n";
            return;
        }
        layer.has_goods = true;
        std::ifstream file(path + "/common/goods.txt");
        parsers::generic_context ctx_generic {
            layer
        };
        errors.file_name = path + "/common/goods.txt";
        std::stringstream buffer;
        buffer << file.rdbuf();
        auto str = buffer.str();
        parsers::token_generator tk(str.c_str(), str.c_str() + buffer.str().length());
        parsers::parse_goods_file(tk, errors, ctx_generic);
    }

    void unload_goods(state::layer& layer, std::string path) {
        std::cout << "Write goods\n";
        if (!layer.has_goods) {
            return;
        }

        std::filesystem::create_directory(path + "common");
        std::ofstream file(path + "common/goods.txt");

        ankerl::unordered_dense::map<std::string, std::vector<std::string>> group_to_goods {};

        for (auto& [key, commodity] : layer.goods) {
            auto iterator = group_to_goods.find(commodity.group);
            if (iterator == group_to_goods.end()) {
                group_to_goods[commodity.group] = {commodity.name};
            } else {
                iterator->second.push_back(commodity.name);
            }
        }

        std::string name{};
        std::string group{};
        float cost = 1.f;
        uint8_t r = 0;
        uint8_t g = 0;
        uint8_t b = 0;
        bool available_from_start = true;
        bool is_local = false;
        bool tradeable = true;
        bool overseas_penalty = false;
        bool money = false;
        bool uses_potentials = false;

        for (auto& [key, commodity_group] : group_to_goods) {
            file << key << " = {\n";

            for (auto& name : commodity_group) {
                file << "\t" << name << " = {\n";

                auto& commodity = layer.goods[name];

                file << "\t\tcost = " << commodity.cost << "\n";

                if (!commodity.available_from_start) {
                    file << "\t\tavailable_from_start = no\n";
                }
                if (commodity.is_local) {
                    file << "\t\tis_local = yes\n";
                }
                if (!commodity.tradeable) {
                    file << "\t\tis_local = no\n";
                }
                if (commodity.overseas_penalty) {
                    file << "\t\tis_local = yes\n";
                }
                if (commodity.money) {
                    file << "\t\tmoney = yes\n";
                }
                if (commodity.uses_potentials) {
                    file << "\t\tuses_potentials = yes\n";
                }

                file << "\t\tcolor = {" << (int)commodity.r << (int)commodity.g << (int)commodity.b << "}\n";

                file << "\t}\n";
            }

            file << "}\n";
        }
    }

    void load_resources_image(state::layer &layer, std::string path) {
        std::cout << "loading resources images\n";
        layer.resources_big.load(path + "/gfx/interface/resources_big.dds");
        layer.resources_medium.load(path + "/gfx/interface/resources.dds");
        layer.resources_small.load(path + "/gfx/interface/resources_small.dds");
    }

    void unload_resources_image(state::layer& layer, std::string path) {
        std::cout << "saving resources images";
        std::filesystem::create_directory(path + "/gfx/interface/");
        layer.resources_big.save(path + "/gfx/interface/resources_big.dds");
        layer.resources_medium.save(path + "/gfx/interface/resources.dds");
        layer.resources_small.save(path + "/gfx/interface/resources_small.dds");
    }

    void load_layer(state::layers_stack& state, state::layer &layer) {
        parsers::error_handler errors("parsing_errors.txt");

        register_pop_types(layer, layer.path);
        load_cultures(layer, layer.path, errors);
        register_religions(layer, layer.path, errors);

        load_province_defs(layer, layer.path);
        load_default_dot_map(layer, layer.path);
        load_provinces_map(layer, layer.path);
        load_regions(layer, layer.path);
        load_adjacencies(layer, layer.path);
        load_governments_list(layer, layer.path, errors);
        load_technology_list(layer, conversions::utf8_to_wstring(layer.path), errors);
        load_inventions_list(layer, conversions::utf8_to_wstring(layer.path), errors);
        load_issues_list(layer, layer.path, errors);
        load_nations_list(layer, layer.path, errors);
        load_nations_common(layer, layer.path, errors);
        load_nation_history(state, layer, layer.path, errors);
        load_province_history(layer, layer.path, errors);
        load_province_population(layer, layer.path, errors);
        load_core_gfx(layer, layer.path, errors);
        load_goods(layer, layer.path, errors);
        load_resources_image(layer, layer.path);

        std::cout << errors.accumulated_errors;
    }

    void unload_data(state::layer& layer, std::string path, state::FLAG_EXPORT_OPTIONS flag_option, int amount_of_commodities) {
        std::cout << "Create directory: " << path << "\n";
        std::filesystem::create_directory(path);

        unload_cultures(layer, path);

        unload_province_defs(layer, path);
        unload_default_dot_map(layer, path);
        unload_province_map(layer, path);
        unload_regions(layer, path);
        unload_adjacencies(layer, path);
        unload_governments_list(layer, path);
        unload_nations_list(layer, path);
        unload_nations_common(layer, path);
        unload_nation_history(layer, path);
        unload_province_history(layer, conversions::utf8_to_wstring(path));
        unload_province_population(layer, path);
        unload_flags(layer, path, flag_option);
        unload_core_gfx(layer, path, amount_of_commodities);
        unload_goods(layer, path);
        unload_resources_image(layer, path);
    }
}