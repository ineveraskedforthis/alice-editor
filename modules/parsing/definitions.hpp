#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "../map/unordered_dense.h"

namespace game_definition {

    struct issue {
        std::string name;
        std::vector<std::string> options{};
    };

    struct commodity {
        int index = 0;
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
    };

    // enum class tech_folder {
    //     army, navy, commerce, culture, industry
    // };

    struct invention {
        std::string name;
        std::wstring invention_file;
    };

    struct technology {
        std::string name;
        std::wstring folder;
    };

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

    struct scripted_flag {
        std::string government{};
        std::string flag{};
    };

    struct government {
        std::string name{};
        std::string flagtype{};
        bool election = false;
        int duration = -1;
        bool appoint_ruling_party = false;
        ankerl::unordered_dense::map<std::string, bool> allowed_parties {};
    };

    struct nation_definition {
        std::array<int8_t, 3> tag {};
        std::string filename {};
        bool dynamic=false;
    };

    struct nation_common {
        uint8_t R = 0;
        uint8_t G = 0;
        uint8_t B = 0;
        std::string graphical_culture{};
        std::vector<party> parties{};
        std::string unit_names{};
    };

    struct sprite {
        std::string key{};
        std::string name{};
        std::string texturefile{};
        std::string texturefile1{};
        std::string texturefile2{};
        std::string effectfile{};
        std::string loadtype{};
        std::string clicksound{};

        int noofframes=0;

        bool transparencecheck = false;
        bool allwaystransparent = false;
        bool norefcount = false;
    };

    struct nation_history {
        std::string history_file_name{};
        std::string primary_culture{};
        std::vector<std::string> culture{};
        // remove_culture
        std::string religion{};
        std::string government{};
        float plurality = 0.f;
        float prestige = 0.f;
        std::string nationalvalue{};
        float literacy = 0.f;
        float non_state_culture_literacy = 0.f;
        bool civilized = false;
        bool is_releasable_vassal = true;
        std::vector<scripted_flag> govt_flag{};
        ankerl::unordered_dense::map<std::string, float> foreign_investment{};
        ankerl::unordered_dense::map<std::string, float> upper_house{};
        std::string ruling_party{};
        std::string schools{};
        float consciousness = 0.f;
        float nonstate_consciousness = 0.f;
        std::string last_election{};
        std::string oob{};
        int capital = 0;
        int colonial_points = 0;
        std::vector<std::string> set_country_flag{};
        std::vector<std::string> set_global_flag{};
        std::vector<std::string> decision{};

        ankerl::unordered_dense::map<std::string, bool> tech{};
        ankerl::unordered_dense::map<std::string, bool> inventions{};
        ankerl::unordered_dense::map<std::string, std::string> issues{};
    };

    inline void define_nation_common(nation_common& n) {
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

    inline int32_t string_to_int(std::string tag) {
        return (int32_t)(tag[0]) + (int32_t)(tag[1]) * 256 + (int32_t)(tag[2]) * 256 * 256;
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

    struct province_history {
        std::wstring history_file_name{};
        std::string owner_tag{};
        std::string controller_tag{};
        std::vector<std::string> cores{};
        std::string main_trade_good{};
        ankerl::unordered_dense::map<std::string, int> secondary_rgo_size{};
        int life_rating;
        int railroad;
        int naval_base;
        int fort;
        int colonial;
        int colony;
        std::vector<state_building> buildings{};
        std::wstring historical_region = L"unknown";
        std::string terrain{};
        std::string is_slave{};
    };

    struct province {
        uint32_t v2id;
        std::string name;
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    struct state {
        std::string name;
    };

    enum class ADJACENCY_TYPE {
        INVALID, STRAIT_CROSSING, CANAL, IMPASSABLE
    };

    struct adjacency {
        uint32_t from = 0;
        uint32_t through = 0;
        uint32_t to = 0;
        ADJACENCY_TYPE type = ADJACENCY_TYPE::INVALID;
        std::string data {};
        std::string comment {};

        bool mark_for_delete = false;
    };
}