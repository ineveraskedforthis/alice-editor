#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace game_definition {

    struct secondary_rgo {
        int size;
        std::string trade_good;
    };

    struct state_building {
        int level;
        std::string building_type;
        std::string upgrade;
    };

    struct province {
        uint32_t v2id;
        std::string name;

        uint8_t r;
        uint8_t g;
        uint8_t b;

        float pos_x;
        float pos_y;
        float pixels;

        std::string history_file_name;
        std::string owner_tag;
        std::string controller_tag;
        std::vector<std::string> cores;
        std::string main_trade_good;

        std::map<std::string, int> secondary_rgo_size;

        int life_rating;
        int railroad;
        int naval_base;
        int fort;
        int colonial;
        int colony;

        std::vector<state_building> buildings;

        std::string historical_region = "unknown";

        std::string terrain;
        std::string is_slave;
    };

    struct state {
        std::string name;
    };

    enum class ADJACENCY_TYPE {
        INVALID, STRAIT_CROSSING, CANAL, IMPASSABLE
    };

    struct adjacency {
        uint32_t from;
        uint32_t through;
        uint32_t to;
        ADJACENCY_TYPE type;
        std::string data;
        std::string comment;

        bool mark_for_delete = false;
    };
}