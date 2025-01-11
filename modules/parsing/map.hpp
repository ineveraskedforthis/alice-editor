#pragma once
#include <cstdint>
#include <map>
#include <vector>
#include <string>
#include "definitions.hpp"

namespace parsing{
    struct game_map {
        std::vector<game_definition::province> provinces;
        std::vector<game_definition::state> states;

        std::vector<game_definition::adjacency> adjacencies;

        int size_x;
        int size_y;
        uint8_t* data_raw;
        uint8_t* data;

        std::map<uint32_t, uint32_t> index_to_vector_position;
        std::map<uint32_t, uint32_t> rgb_to_index;
        std::map<uint32_t, uint32_t> prov_to_market;

        bool id_is_used[256 * 256];

        uint8_t province_is_sea[256 * 256];
        uint8_t province_state[256 * 256 * 2];
        uint8_t province_owner[256 * 256 * 3];

        uint32_t available_id = 1;
        uint8_t available_r;
        uint8_t available_g;
        uint8_t available_b;

        game_definition::province new_province(uint32_t pixel);
        void update_available_colors();
    };

    game_map load_map(std::string path);

    void load_provs(game_map& map_state, std::string path);
    void unload_data(game_map& map_state, std::string path);

    uint32_t rgb_to_uint(int r, int g, int b);
}