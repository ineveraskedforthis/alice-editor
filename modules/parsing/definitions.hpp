#pragma once
#include <array>
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

    struct party {
        std::string name = "default_party";
        std::string start = "1800.1.1";
        std::string end = "2000.1.1";
        std::string ideology = "conservative";
        std::string economic_policy = "interventionism";
        std::string trade_policy = "free_trade";
        std::string religious_policy = "moralism";
        std::string citizenship_policy = "limited_citizenship";
        std::string war_policy = "pro_military";
    };

    struct nation {
        std::array<int8_t, 3> tag= {};
        std::string filename="";
        bool dynamic=false;

        bool defined_in_common=false;
        bool defined_in_history=false;

        uint8_t R = 0;
        uint8_t G = 0;
        uint8_t B = 0;

        std::string graphical_culture="";
        std::vector<party> parties={};

        std::string unit_names={};
    };

    inline void define_nation_common(nation& n) {
        n.defined_in_common = true;
        n.R = 0;
        n.G = 0;
        n.B = 0;

        n.graphical_culture = "BritishGC";

        party conservative = {
            .name = "default_conservative",
            .start = "1800.1.1",
            .end = "2000.1.1",
            .ideology = "conservative",
            .economic_policy = "interventionism",
            .trade_policy = "free_trade",
            .religious_policy = "moralism",
            .citizenship_policy = "limited_citizenship",
            .war_policy = "pro_military"
        };
        party liberal = {
            .name = "default_liberal",
            .start = "1800.1.1",
            .end = "2000.1.1",
            .ideology = "liberal",
            .economic_policy = "interventionism",
            .trade_policy = "free_trade",
            .religious_policy = "moralism",
            .citizenship_policy = "limited_citizenship",
            .war_policy = "pro_military"
        };
        party socialist = {
            .name = "default_socialist",
            .start = "1800.1.1",
            .end = "2000.1.1",
            .ideology = "socialist",
            .economic_policy = "interventionism",
            .trade_policy = "free_trade",
            .religious_policy = "moralism",
            .citizenship_policy = "limited_citizenship",
            .war_policy = "pro_military"
        };
        party reactionary = {
            .name = "default_reactionary",
            .start = "1800.1.1",
            .end = "2000.1.1",
            .ideology = "reactionary",
            .economic_policy = "interventionism",
            .trade_policy = "free_trade",
            .religious_policy = "moralism",
            .citizenship_policy = "limited_citizenship",
            .war_policy = "pro_military"
        };
        party anarcho_liberal = {
            .name = "default_anarcho_liberal",
            .start = "1800.1.1",
            .end = "2000.1.1",
            .ideology = "anarcho_liberal",
            .economic_policy = "interventionism",
            .trade_policy = "free_trade",
            .religious_policy = "moralism",
            .citizenship_policy = "limited_citizenship",
            .war_policy = "pro_military"
        };
        party communist = {
            .name = "default_communist",
            .start = "1800.1.1",
            .end = "2000.1.1",
            .ideology = "communist",
            .economic_policy = "planned_economy",
            .trade_policy = "free_trade",
            .religious_policy = "moralism",
            .citizenship_policy = "limited_citizenship",
            .war_policy = "pro_military"
        };
        party fascist = {
            .name = "default_fascist",
            .start = "1800.1.1",
            .end = "2000.1.1",
            .ideology = "fascist",
            .economic_policy = "state_capitalism",
            .trade_policy = "free_trade",
            .religious_policy = "moralism",
            .citizenship_policy = "limited_citizenship",
            .war_policy = "pro_military"
        };

        n.parties = {
            conservative, liberal, reactionary, socialist, anarcho_liberal, communist, fascist
        };
    }

    inline int32_t tag_to_int(std::array<int8_t, 3> tag) {
        return (int32_t)(tag[0]) + (int32_t)(tag[1]) * 256 + (int32_t)(tag[2]) * 256 * 256;
    }
    inline std::array<int8_t, 3> int_to_tag(int32_t tag) {
        int char2 = tag >> 16;
        tag -= char2 << 16;
        int char1 = tag >> 8;
        tag -= char1 << 8;
        int char0 = tag;

        return {
            (int8_t)char0, (int8_t)char1, (int8_t)char2
        };
    }

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