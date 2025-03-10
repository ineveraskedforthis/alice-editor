#include "map.hpp"
#include <corecrt_terminate.h>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "adjacency.hpp"
#include "definitions.hpp"
#include "generated/parsers.hpp"
#include "generated/parsers_core.hpp"
#include "generated/defs_generated.hpp"
#include "parser.hpp"
#include "secondary_rgo.hpp"
#include "state_building.hpp"
#include "templates.hpp"
#include "countries.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../stbimage/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stbimage/stb_image_write.h"


namespace parsers{

    void create_government_type(std::string_view name, parsers::token_generator &gen, parsers::error_handler &err, parsers::generic_context &context) {
        context.map.governments.emplace_back();
        context.map.governments.back().name = name;
        parsers::government_type_context ctx {
            context.map, context.map.governments.back()
        };
        parse_government_type(gen, err, ctx);
    }

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
        void parse(game_map& map, game_definition::province& prov, std::ifstream& file) {
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

                        prov.owner_tag = value.data;
                        map.province_owner[3 * prov.v2id + 0] = value.data[0];
                        map.province_owner[3 * prov.v2id + 1] = value.data[1];
                        map.province_owner[3 * prov.v2id + 2] = value.data[2];
                    } else if (word.data == "controller") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        prov.controller_tag = value.data;
                    } else if (word.data == "state_building") {
                        while (parser::equality(c) && file.get(c));
                        parser::state_building building;
                        game_definition::state_building def;
                        building.parse(def, file, c);
                        prov.buildings.push_back(def);
                    } else if (word.data == "rgo_distribution") {
                        std::vector<game_definition::secondary_rgo> rgos;
                        parser::secondary_rgo(rgos, file, c);
                        for (auto& item : rgos) {
                            if (prov.secondary_rgo_size.contains((item.trade_good))) {
                                prov.secondary_rgo_size[item.trade_good] += item.size;
                            } else {
                                prov.secondary_rgo_size[item.trade_good] = item.size;
                            }
                        }
                    } else if (word.data == "add_core") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));
                        prov.cores.push_back(value.data);
                    } else if (word.data == "terrain") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        prov.terrain = value.data;
                    } else if (word.data == "trade_goods") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        prov.main_trade_good = value.data;
                    } else if (word.data == "is_slave") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        prov.is_slave = value.data;
                    } else if (word.data == "life_rating") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        prov.life_rating = std::stoi(value.data);
                    } else if (word.data == "naval_base") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        prov.naval_base = std::stoi(value.data);
                    } else if (word.data == "fort") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        prov.fort = std::stoi(value.data);
                    } else if (word.data == "railroad") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        prov.railroad = std::stoi(value.data);
                    } else if (word.data == "colonial") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        prov.colonial = std::stoi(value.data);
                    } else if (word.data == "colony") {
                        parser::word value;
                        while (parser::equality(c) && file.get(c));
                        while (value.parse(c) && file.get(c));

                        prov.colony = std::stoi(value.data);
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

        void set_prov(game_map& map) {
            if (current_word.empty()) {
                return;
            }

            auto index = std::stoi(current_word);
            current_word.clear();

            // std::cout << "!" << index << " ";

            // std::cout << "prov detected\n";

            map.province_state[2 * index] = current_state_def % 256;
            map.province_state[2 * index + 1] = current_state_def >> 8;
        }

        void parse_symbol(game_map& map, char c) {
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
                if (c == ' ' || c == '\n' || c == '=') {
                    break;
                } else {
                    current_word = c;
                    task = REGIONS_PARSER_TASK::READING_STATE_NAME;
                }
                break;
            }
            case REGIONS_PARSER_TASK::READING_STATE_NAME: {
                if (c == ' ' || c == '\n' || c == '=') {
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
                if (c == ' ' || c == '\n' || c == '=' || c == '{') {
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
                if (c == ' ') {
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

    game_map load_map(std::string path) {
        int size_x;
        int size_y;
        int channels;

        auto result = stbi_load(
            (path + "/map/alice_provinces.png").c_str(),
            &size_x,
            &size_y,
            &channels,
            4
        );

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
    }

    void load(game_map &map_state, std::string path) {
        {
            std::ifstream file(path + "/map/definition.csv");
            std::string str;

            std::getline(file, str);
            std::cout << "reading /map/definition.csv\n";

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
                    map_state.id_is_used[index] = true;
                    game_definition::province def {
                        (uint32_t)(index),
                        name,
                        (uint8_t)(r),
                        (uint8_t)(g),
                        (uint8_t)(b)
                    };
                    map_state.provinces.push_back(def);
                    map_state.rgb_to_index.insert_or_assign(rgb_to_uint(r, g, b), (uint32_t)(index));
                    map_state.index_to_vector_position.insert_or_assign((uint32_t)(index), map_state.provinces.size() - 1);
                }
            }
        }

        {
            std::ifstream file(path + "/map/default.map");

            parser_of_defaul_file parser {};
            std::cout << "reading /map/default.map\n";

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
                                    map_state.province_is_sea[index] = 255;
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

        std::cout << "\nupdating colors\n";

        for (auto i = 0; i < map_state.size_x * map_state.size_y; i++) {
            // std::cout << i << " ";
            auto r = map_state.data_raw[4 * i + 0];
            auto g = map_state.data_raw[4 * i + 1];
            auto b = map_state.data_raw[4 * i + 2];
            auto a = map_state.data_raw[4 * i + 3];
            auto rgb = rgb_to_uint(r, g, b);
            auto index = map_state.rgb_to_index[rgb];

            auto& prov = map_state.provinces[map_state.index_to_vector_position[index]];

            prov.pixels ++;
            prov.pos_x += (float)((i) % map_state.size_x);
            prov.pos_y += std::floor((i) / map_state.size_x);

            // std::cout << index << " " << index % 256 << " " << (index >> 8) << "\n";
            map_state.data[4 * i + 0] = index % 256;
            map_state.data[4 * i + 1] = index >> 8;
        }

        for (auto& prov : map_state.provinces) {
            prov.pos_x /= prov.pixels;
            prov.pos_y /= prov.pixels;

            // std::cout << "prov center " << prov.pos_x << " " << prov.pos_y << "\n";
        }

        std::cout << "\nupdating available colors\n";

        map_state.update_available_colors();

        {
            std::cout << "reading states\n";
            std::ifstream file(path + "/map/area.txt");
            if (!file.good()) {
                file.open(path + "/map/region.txt");
            }
            std::string str;
            char c;
            map_state.states.push_back({"INVALID"});
            parser_regions parser {};
            std::cout << "parsing states\n";
            if (file) {
                while (file.get(c)) {
                    parser.parse_symbol(map_state, c);
                }
            } else {
                std::cout << "bad states/areas" << std::endl;
            }
        }

        {
            std::cout << "reading adjacencies\n";
            std::cout << path + "/map/adjacencies.csv\n";
            std::ifstream file(path + "/map/adjacencies.csv");
            std::string str;
            char c;
            parser::adj parser {};
            if (file) {
                std::cout << "parsing adjacencies\n";
                parser.parse(file, map_state.adjacencies);
            } else
                std::cout << "bad /map/adjacencies.csv" << std::endl;
        }

        {
            std::cout << "reading editor templates\n";
            std::ifstream file(path + "/editor-templates/secondary_rgo.txt");

            if (file) {
                std::cout << "parsing rgo templates\n";
                parser::secondary_rgo_template_file(map_state, file);
            } else
                std::cout << "no rgo distribution templates found" << std::endl;
        }

        parsers::error_handler errors("parsing_errors.txt");

        parsers::generic_context ctx_generic {
            map_state
        };

        {
            std::cout << "reading governments\n";
            std::ifstream file(path + "/common/governments.txt");

            std::stringstream buffer;
            buffer << file.rdbuf();
            auto str = buffer.str();
            parsers::token_generator tk(str.c_str(), str.c_str() + buffer.str().length());
            parsers::parse_governments_file(tk, errors, ctx_generic);
        }

        {
            std::cout << "reading nations list\n";
            std::ifstream file(path + "/common/countries.txt");
            parser::countries_list(map_state, file);

            for (size_t i = 0; i < map_state.nations.size(); i++) {
                {
                    auto& n = map_state.nations[i];
                    std::ifstream file(path + "/common/countries/" + n.filename + ".txt");
                    if (file) {
                        parser::country_file_common(n, file);
                        n.defined_in_common = true;
                    } else {
                        std::cout << "Common file for " << n.filename << " was not provided.\n";
                    }
                }
            }



            for (auto& entry : std::filesystem::directory_iterator  {path + "/history" + "/countries"}) {
                if (!entry.is_directory() && entry.path().filename().string().ends_with(".txt")) {
                    auto name = entry.path().filename().string();
                    // std::cout << name << std::endl;
                    auto first_space = name.find_first_of(' ');
                    auto tag_string = name.substr(0, first_space);
                    auto tag_int = game_definition::tag_to_int({
                        tag_string[0], tag_string[1], tag_string[2]
                    });
                    auto index = map_state.tag_to_vector_position[tag_int];
                    auto& def = map_state.nations[index];
                    def.history_file_name = name;
                    std::ifstream file(entry.path());

                    parsers::nation_history_file context {
                        def,
                        map_state
                    };

                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    auto str = buffer.str();
                    parsers::token_generator tk(str.c_str(), str.c_str() + buffer.str().length());
                    parsers::parse_nation_handler(tk, errors, context);
                }
            }
        }

        {
            std::cout << "reading province history\n";

            for (auto& entry : std::filesystem::directory_iterator  {path + "/history" + "/provinces"}) {
                if (!entry.is_directory() && entry.path().filename().string().ends_with(".txt")) {
                    auto name = entry.path().filename().string();
                    // std::cout << name << std::endl;
                    auto first_space = name.find_first_of(' ');
                    auto id_string = name.substr(0, first_space);
                    auto id = std::stoi(id_string);
                    std::cout << id << " ";
                    auto& def = map_state.provinces[map_state.index_to_vector_position[id]];
                    def.history_file_name = name;
                    def.historical_region = "other";
                    parser_history_province2 parser {};
                    std::ifstream file(entry.path());
                    parser.parse(map_state, def, file);
                    // std::cout << "parsing completed" << std::endl;

                    continue;
                }

                for (auto& province_description : std::filesystem::directory_iterator(entry.path())) {
                    auto name = province_description.path().filename().string();
                    // std::cout << name << std::endl;
                    auto first_space = name.find_first_of(' ');
                    auto id_string = name.substr(0, first_space);
                    auto id = std::stoi(id_string);
                    // std::cout << id << " ";
                    auto& def = map_state.provinces[map_state.index_to_vector_position[id]];
                    def.history_file_name = name;
                    def.historical_region = entry.path().filename().string();
                    parser_history_province2 parser {};
                    std::ifstream file(province_description.path());
                    parser.parse(map_state, def, file);
                }
            }
        }

        std::cout << errors.accumulated_errors;
    }

    void unload_data(game_map& map_state, std::string path) {
        std::cout << "create directory";

        std::filesystem::create_directory(path + "/map");

        {
            std::cout << "write definitions";
            std::ofstream file(path + "/map/definition.csv");
            file << "province;red;green;blue;x;x\n";
            for (auto i = 0; i < map_state.provinces.size(); i++) {
                auto prov = map_state.provinces[i];
                auto rgb = rgb_to_uint(prov.r, prov.g, prov.b);
                auto index = map_state.rgb_to_index[rgb];

                file
                    << prov.v2id << ";"
                    << (int)prov.r << ";"
                    << (int)prov.g << ";"
                    << (int)prov.b << ";"
                    << prov.name << ";"
                    << "x\n";
            }
        }

        std::cout << "write provinces image";

        stbi_write_png(
            (path + "/map/alice_provinces.png").c_str(),
            map_state.size_x,
            map_state.size_y,
            4,
            map_state.data_raw,
            0
        );

        int available_id = 1;
        while (map_state.id_is_used[available_id])
            available_id += 1;

        {
            std::cout << "write default map file (list of sea provs)";

            std::ofstream file(path + "/map/default.map");

            file << "max_provinces = " << available_id << "\n";

            file << "sea_starts = {";

            int counter = 0;
            for (auto i = 0; i < map_state.provinces.size(); i++) {
                auto prov = map_state.provinces[i];
                if (map_state.province_is_sea[prov.v2id] > 0){
                    if (counter % 10 == 0) {
                        file << "\n";
                    }
                    counter++;
                    file << prov.v2id << " ";
                }
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

        {
            std::ofstream file(path + "/map/region.txt");

            std::cout << "init collections of provs in a state\n";
            std::vector<std::vector<uint32_t>> states_to_provs;
            for (auto i = 0; i < map_state.states.size(); i++) {
                states_to_provs.push_back({});
            }

            std::cout << "populate collections of provs in a state\n";
            for (auto i = 0; i < map_state.provinces.size(); i++) {
                auto prov = map_state.provinces[i];
                int state_x = map_state.province_state[2 * prov.v2id];
                int state_y = map_state.province_state[2 * prov.v2id + 1];
                auto state = state_x + state_y * 256;

                if (state) {
                    states_to_provs[state].push_back(prov.v2id);
                }
            }

            std::cout << "save collections of provs in a state\n";
            for (auto i = 1; i < map_state.states.size(); i++) {
                auto state = map_state.states[i];
                file << state.name << " = { ";
                for (auto j = 0; j < states_to_provs[i].size(); j++) {
                    file << states_to_provs[i][j] << " ";
                }
                file << "}\n";
            }
        }

        {
            std::ofstream file(path + "/map/adjacencies.csv");

            for (auto& adj: map_state.adjacencies) {
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
                    file << adj.from << ";" << adj.to << ";" << type << ";" << adj.through << ";" << adj.data << ";" << adj.comment << "\n";
                }
            }
        }

        {
            std::filesystem::create_directory(path + "/history");
            std::filesystem::create_directory(path + "/history/provinces");
            for (auto& def : map_state.provinces) {
                if (map_state.province_is_sea[def.v2id]) continue;

                auto folder_path = path + "/history/provinces/" + def.historical_region;
                std::filesystem::create_directory(folder_path);
                std::ofstream file(folder_path + "/" + def.history_file_name);

                if (def.owner_tag.length() > 0)
                    file << "owner = " << def.owner_tag << std::endl;

                if (def.controller_tag.length() > 0)
                    file << "controller = " << def.controller_tag << std::endl;

                for (auto& core : def.cores) {
                    if (core.length() == 0) continue;
                    file << "add_core = " << core << std::endl;
                }

                if (def.main_trade_good.length() > 0)
                    file << "trade_goods = " << def.main_trade_good << std::endl;


                file << "colonial = " << def.colonial << std::endl;
                file << "colony = " << def.colony << std::endl;
                file << "life_rating = " << def.life_rating << std::endl;
                file << "naval_base = " << def.naval_base << std::endl;
                file << "railroad = " << def.railroad << std::endl;
                file << "fort = " << def.fort << std::endl;

                for (auto& building : def.buildings) {
                    file << "state_building = {" << std::endl;
                    file << "\tlevel = " << building.level << std::endl;
                    file << "\tbuilding = " << building.building_type << std::endl;
                    file << "\tupgrade = " << building.upgrade << std::endl;
                    file << "}" << std::endl;
                }

                std::vector<std::string> local_rgo;
                for (auto const& [key, val] : def.secondary_rgo_size) {
                    local_rgo.push_back(key);
                }

                if (local_rgo.size() > 0) {
                    file << "rgo_distribution = {" << std::endl;

                    for (auto key : local_rgo) {
                        file << "\tentry = {" << std::endl;
                        file << "\t\ttrade_good = " << key << std::endl;
                        file << "\t\tmax_employment = " << def.secondary_rgo_size[key] << std::endl;
                        file << "\t}" << std::endl;
                    }

                    file << "}" << std::endl;
                }

                if (def.terrain.length() > 0)
                    file << "terrain = " << def.terrain << std::endl;
                if (def.is_slave.length() > 0)
                    file << "is_slave = " << def.is_slave << std::endl;
            }
        }
    }

    void game_map::update_available_colors() {
        for (int _r = 0; _r < 256; _r++)
            for (int _g = 0; _g < 256; _g++)
                for (int _b = 0; _b < 256; _b++) {
                    auto rgb = rgb_to_uint(_r, _g, _b);
                    if (!rgb_to_index.contains(rgb)) {
                        available_r = _r;
                        available_g = _g;
                        available_b = _b;
                        return;
                    }
                }
    }

    game_definition::province game_map::new_province(uint32_t pixel) {

        auto old_r = data_raw[4 * pixel];
        auto old_g = data_raw[4 * pixel + 1];
        auto old_b = data_raw[4 * pixel + 2];

        auto old_rgb = rgb_to_uint(old_r, old_g, old_b);
        auto old_index = rgb_to_index[old_rgb];

        while (id_is_used[available_id])
            available_id += 1;

        id_is_used[available_id] = true;

        update_available_colors();

        game_definition::province result = {
            available_id,
            "UnknownProv" + std::to_string(available_id),
            available_r,
            available_g,
            available_b
        };

        result.history_file_name = std::to_string(available_id) + " - " + result.name + ".txt";

        auto& old_province = provinces[index_to_vector_position[old_index]];

        result.life_rating = old_province.life_rating;
        result.owner_tag = old_province.owner_tag;
        result.controller_tag = old_province.controller_tag;
        result.main_trade_good = old_province.main_trade_good;
        result.colonial = old_province.colonial;
        result.historical_region = old_province.historical_region;

        for (auto& core: old_province.cores) {
            result.cores.push_back(core);
        }

        provinces.push_back(result);
        auto rgb = rgb_to_uint(result.r, result.g, result.b);
        rgb_to_index.insert_or_assign(rgb, result.v2id);
        index_to_vector_position.insert_or_assign((uint32_t)(result.v2id), provinces.size() - 1);

        data_raw[pixel * 4] = result.r;
        data_raw[pixel * 4 + 1] = result.g;
        data_raw[pixel * 4 + 2] = result.b;

        data[4 * pixel + 0] = result.v2id % 256;
        data[4 * pixel + 1] = result.v2id >> 8;

        province_is_sea[result.v2id] = province_is_sea[old_index];

        province_state[2 * result.v2id] = province_state[2 * old_index];
        province_state[2 * result.v2id + 1] = province_state[2 * old_index + 1];

        province_owner[3 * result.v2id] = province_owner[3 * old_index];
        province_owner[3 * result.v2id + 1] = province_owner[3 * old_index + 1];
        province_owner[3 * result.v2id + 2] = province_owner[3 * old_index + 2];

        return result;
    }
}