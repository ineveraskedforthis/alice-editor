#include <cstdint>
#include <map>
#include <vector>
#include <string>

namespace parsing{

    struct state_building_definition {
        int level;
        std::string building_type;
        std::string upgrade;
    };

    struct province_definition {
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

        int life_rating;
        int railroad;
        int naval_base;
        int fort;
        int colonial;
        int colony;

        std::vector<state_building_definition> buildings;

        std::string historical_region;

        std::string terrain;
        std::string is_slave;
    };

    struct state_definition {
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

        bool mark_for_delete;
    };

    struct game_map {
        std::vector<province_definition> provinces;
        std::vector<state_definition> states;

        std::vector<adjacency> adjacencies;

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

        province_definition new_province(uint32_t pixel);
        void update_available_colors();
    };

    game_map load_map(std::string path);

    void load_provs(game_map& map_state, std::string path);
    void unload_data(game_map& map_state, std::string path);

    uint32_t rgb_to_uint(int r, int g, int b);
}