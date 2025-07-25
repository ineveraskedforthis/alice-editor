#include "GL/glew.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>
#include <vector>

#include "../glm/fwd.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "../glm/ext/matrix_transform.hpp"
#include <string>

#include "GL/glew.h"

#include "../parsing/definitions.hpp"

#include "../map/unordered_dense.h"
#include "../misc.hpp"

#undef max
#undef min

namespace state {

glm::vec2 screen_to_texture(
    int x_in,
    int y_in,
    float width_texture,
    float height_texture,
    float width_screen,
    float height_screen,
    float zoom,
    glm::vec2 camera_shift
);

    // CORE PRINCIPLES:
    // CAN SAMPLE DATA FROM THE HIGHEST VISIBLE LAYER
    // LAYERS ABOVE CURRENT ARE INVISIBLE
    // CAN CHANGE DATA ONLY AT THE CURRENT LAYER ONLY IF THIS LAYER HAS THE REQUIRED DATA
    // OTHERWISE DATA EDITING SHOULD BE BLOCKED
    // USER CAN REQUEST TO COPY DATA TO THE ACTIVE LAYER FROM THE HIGHEST VISIBLE LAYER WHICH HAS THE REQUIRED DATA


// converts rgb to a single number
uint32_t inline rgb_to_uint(int r, int g, int b) {
    return (r << 16) + (g << 8) + b;
}


struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

color inline uint_to_r_g_b(uint32_t c) {
    auto r = c >> 16;
    c -= (r << 16);
    auto g = c >> 8;
    c -= (g << 8);
    auto b = c;

    return {(uint8_t)r, (uint8_t)g, (uint8_t)b};
}

void check_gl_error(std::string message);

struct province_texture {
    int size_x = 0;
    int size_y = 0;
    uint8_t* data = nullptr;
    GLuint id = 0;
    bool update_texture = false;
    bool update_texture_part = false;
    int update_texture_x_top = 0;
    int update_texture_y_top = 0;
    int update_texture_x_bottom = std::numeric_limits<int>::max();
    int update_texture_y_bottom = std::numeric_limits<int>::max();

    std::vector<int> v2id_to_size{};
    std::vector<uint8_t> v2id_exists{};
    std::vector<glm::vec2> v2id_to_mean{};

    province_texture(){
        v2id_to_size.resize(256 * 256);
        v2id_exists.resize(256 * 256);
        v2id_to_mean.resize(256 * 256);
    };
    province_texture& operator=(province_texture& source);
    int coord_to_pixel(glm::ivec2 coord);
    void load_province_texture_to_gpu();
    int coord_to_pixel(glm::vec2 coord);
    void commit_province_texture_changes_to_gpu();
};

struct province_map {
    int size_x = 0;
    int size_y = 0;
    uint8_t* provinces_image_data = nullptr;

    uint32_t available_color = 0;
    uint8_t available_r = 0;
    uint8_t available_g = 0;
    uint8_t available_b = 0;

    std::vector<uint8_t> color_present {};

    void clear();
    void update_available_colors();
    void recalculate_present_colors();
    void populate_adjacent_colors(uint32_t rgb, std::vector<uint32_t> & result);

    int inline coord_to_pixel(glm::ivec2 coord) {
        return coord.y * size_x + coord.x;
    }
    int inline coord_to_pixel(glm::vec2 coord) {
        return int(std::floor(coord.y))
            * size_x
            + int(std::floor(coord.x));
    }

    // constructor from dim
    explicit province_map(int x, int y) {
        size_x = x;
        size_y = y;
        provinces_image_data = new uint8_t[size_x * size_y * 4];
    }

    // constructor from dim and data
    explicit province_map(int x, int y, uint8_t* data) {
        size_x = x;
        size_y = y;
        provinces_image_data = data;
    }

    // destructor
    ~province_map()
    {
        delete[] provinces_image_data;
    }

    // copy constructor
    province_map(const province_map& source) {
        // delete old data
        delete[] provinces_image_data;

        size_x = source.size_x;
        size_y = source.size_y;

        // create new data and copy values there
        provinces_image_data = new uint8_t[size_x * size_y * 4];
        std::copy(source.provinces_image_data, source.provinces_image_data + size_x * size_y * 4, provinces_image_data);
        color_present = source.color_present;
        available_r = source.available_r;
        available_g = source.available_g;
        available_b = source.available_b;
    }

    // copy assignment
    province_map& operator=(province_map& source) {
        if (&source == this)
			return *this;

        delete[] provinces_image_data;

        size_x = source.size_x;
        size_y = source.size_y;

        provinces_image_data = new uint8_t[size_x * size_y * 4];
        std::copy(source.provinces_image_data, source.provinces_image_data + size_x * size_y * 4, provinces_image_data);
        color_present = source.color_present;
        available_r = source.available_r;
        available_g = source.available_g;
        available_b = source.available_b;

        return *this;
    }

    // move constructor
    province_map(province_map&& source) noexcept :
    size_x(std::move(source.size_x)),
    size_y(std::move(source.size_y)),
    available_r(std::move(source.available_r)),
    available_g(std::move(source.available_g)),
    available_b(std::move(source.available_b)),
    color_present(std::move(source.color_present)),
    provinces_image_data(source.provinces_image_data) {
        source.clear();
    }

    // move assignment
    province_map& operator=(province_map&& source) noexcept {
        if (&source == this)
			return *this;

        // delete whatever we had in the array
        delete[] provinces_image_data;
        // steal pointer
        provinces_image_data = std::move(source.provinces_image_data);
        size_x = std::move(source.size_x);
        size_y = std::move(source.size_y);
        available_r = std::move(source.available_r),
        available_g = std::move(source.available_g);
        available_b = std::move(source.available_b);
        color_present = std::move(source.color_present);
        source.clear();
        return *this;
    }

    int screen_to_pixel(glm::vec2 screen) {
        auto w = size_x;
        return int(std::floor(screen.y) * w + std::floor(screen.x));
    }
};

struct interface_dds_image {
    uint8_t* data = nullptr;
    int size_x;
    int size_y;
    int channels;

    GLuint texture_id = 0;

    interface_dds_image& operator=(interface_dds_image& source);

    bool valid();
    bool load(std::string path);
    void save(std::string path);
    // initial loading
    void upload_to_gpu();
    // update existing texture
    void commit_to_gpu();
    void expand_image_right(int amount);
};

// represent specific mod folder which overwrites previous definitions
struct layer {
    std::string path = "./base-game";

    std::vector<std::string> poptypes{};

    //is this layer visible
    bool visible = true;

    // provinces part of the layer
    std::optional<province_map> provinces_image {};

    // rivers part of the layer
    bool has_rivers_map = false;

    // adjacencies part of the layer
    std::vector<game_definition::adjacency> adjacencies{};
    bool has_adjacencies = false;

    bool has_continent_txt = false;

    uint8_t province_is_sea[256 * 256];
    GLuint sea_texture;
    bool has_default_map;
    void load_sea_texture_to_gpu();
    void commit_sea_texture_to_gpu();

    std::vector<game_definition::province> province_definitions {};
    ankerl::unordered_dense::map<uint32_t, uint32_t> v2id_to_vector_position {};
    std::array<bool, 256 * 256> is_used {};
    ankerl::unordered_dense::map<uint32_t, uint32_t> rgb_to_v2id {};
    uint32_t available_id = 1;
    bool has_province_definitions = false;

    // history/provinces/*.txt
    ankerl::unordered_dense::map<uint32_t, game_definition::province_history> province_history {};

    // history/pops/ DATE / *.txt
    std::vector<game_definition::pops_setups> province_population {};

    // region.txt

    std::vector<game_definition::state> states {};
    uint8_t province_state[256 * 256 * 2];
    GLuint state_texture;
    bool has_region_txt = false;
    void load_state_texture_to_gpu();
    void commit_state_texture_to_gpu();


    std::vector<game_definition::nation_definition> nations{};
    ankerl::unordered_dense::map<int32_t, uint32_t> tag_to_vector_position{};
    bool has_nations_list = false;

    ankerl::unordered_dense::map<std::string, game_definition::nation_common> filename_to_nation_common{};
    ankerl::unordered_dense::map<int32_t, game_definition::nation_history> tag_to_nation_history{};

    std::vector<game_definition::government> governments{};
    std::vector<std::string> detected_flags{};
    ankerl::unordered_dense::map<std::string, std::wstring> paths_to_new_flags{};
    bool has_governments_list = false;

    ankerl::unordered_dense::map<std::string, game_definition::technology> tech{};
    ankerl::unordered_dense::map<std::wstring, bool> has_tech{};
    ankerl::unordered_dense::map<std::string, game_definition::invention> inventions{};
    ankerl::unordered_dense::map<std::wstring, bool> has_invention{};
    ankerl::unordered_dense::map<std::string, game_definition::issue> issues{};
    bool has_issues = false;

    // goods
    ankerl::unordered_dense::map<std::string, game_definition::commodity> goods{};
    bool has_goods = false;

    // cultures
    ankerl::unordered_dense::map<std::string, game_definition::culture> culture_defs {};
    std::vector<std::string> cultures {};
    ankerl::unordered_dense::map<std::string, game_definition::culture_group> culture_group_defs {};
    std::vector<std::string> culture_groups {};
    ankerl::unordered_dense::map<std::string, std::string> culture_to_group {};
    bool has_cultures = false;

    // religions
    std::vector<std::string> religions {};
    bool has_religions = false;

    // interface description
    std::vector<game_definition::sprite> sprites{};
    std::vector<game_definition::sprite> text_sprites{};
    std::vector<game_definition::sprite> masked_shields{};
    std::vector<game_definition::sprite> cornered_sprites{};
    std::vector<std::string> lightTypes_text{};
    std::vector<std::string> objectTypes_text{};
    std::vector<std::string> bitmapfonts_text{};
    std::vector<std::string> bitmapfont_text{};
    std::vector<std::string> fonts_text{};
    bool has_core_gfx = false;

    // resources images
    interface_dds_image resources_small;
    interface_dds_image resources_medium;
    interface_dds_image resources_big;
};

struct layers_stack {
    std::vector<layer> data{};

    // this data is based on history files which could be spread across several layers,
    // so we have to store it outside of layers

    province_texture indices{};
    void generate_indices() {
        std::cout << "Generate indices texture \n";
        //obtain province map:
        layer* layer_with_province_map = nullptr;
        layer* layer_with_definitions = nullptr;

        indices.update_texture = true;

        for (int i = 0; i < 256 * 256; i++) {
            indices.v2id_exists[i] = false;
        }
        for (int i = 0; i < 256 * 256; i++) {
            indices.v2id_to_mean[i] = {0.f, 0.f};
        }
        for (int i = 0; i < 256 * 256; i++) {
            indices.v2id_to_size[i] = 0;
        }

        for (auto& l : data) {
            if (l.visible && l.has_province_definitions) {
                layer_with_definitions = &l;
            }
            if (l.visible && l.provinces_image != std::nullopt) {
                layer_with_province_map = &l;
            }
        }

        if (layer_with_definitions != nullptr && layer_with_province_map != nullptr) {
            indices.size_x = layer_with_province_map->provinces_image->size_x;
            indices.size_y = layer_with_province_map->provinces_image->size_y;

            delete[] indices.data;
            indices.data = new uint8_t[indices.size_x * indices.size_y * 4];

            for (auto i = 0; i < indices.size_x * indices.size_y; i++) {
                // std::cout << i << " ";
                auto r = layer_with_province_map->provinces_image->provinces_image_data[4 * i + 0];
                auto g = layer_with_province_map->provinces_image->provinces_image_data[4 * i + 1];
                auto b = layer_with_province_map->provinces_image->provinces_image_data[4 * i + 2];
                auto rgb = rgb_to_uint(r, g, b);
                auto find_result = layer_with_definitions->rgb_to_v2id.find(rgb);
                if (find_result != layer_with_definitions->rgb_to_v2id.end()) {
                    auto index = find_result->second;
                    indices.data[4 * i + 0] = index % 256;
                    indices.data[4 * i + 1] = index >> 8;
                    indices.v2id_exists[index] = true;
                    indices.v2id_to_size[index] ++;
                    indices.v2id_to_mean[index] += glm::vec2((float)((i) % indices.size_x), std::floor((i) / indices.size_x));
                }
            }
            for (int i = 0; i < 256 * 256; i++) {
                indices.v2id_to_mean[i] = indices.v2id_to_mean[i] / (float)indices.v2id_to_size[i];
            }
        }
    }

    GLuint owner_texture;
    uint8_t province_owner[256 * 256 * 3];
    void update_owner_texture(){
        for(int i = 0; i < 256 * 256; i++) {
            auto history = get_province_history(i);
            if (history != nullptr) {
                province_owner[i * 3 + 0] = history->owner_tag[0];
                province_owner[i * 3 + 1] = history->owner_tag[1];
                province_owner[i * 3 + 2] = history->owner_tag[2];
            }
        }
    }
    void inline load_owner_texture_to_gpu() {
        glGenTextures(1, &owner_texture);
        glBindTexture(GL_TEXTURE_2D, owner_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            256,
            256,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            province_owner
        );
        check_gl_error("State texture update");
    }
    void inline commit_owner_texture_to_gpu() {
        glBindTexture(GL_TEXTURE_2D, owner_texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            256,
            256,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            province_owner
        );
    }

    int current_layer_index = -1;

    int get_provinces_image_x() {
        int result;
        for (auto& l: data) {
            if (l.visible && l.provinces_image != std::nullopt) {
                result = l.provinces_image->size_x;
            }
        }
        return result;
    }
    int get_provinces_image_y() {
        int result;
        for (auto& l: data) {
            if (l.visible && l.provinces_image != std::nullopt) {
                result = l.provinces_image->size_y;
            }
        }
        return result;
    }

    void populate_adjacent_colors(uint32_t rgb, std::vector<uint32_t> & result) {
        layer * last_layer;
        for (auto& l: data) {
            if (l.visible && l.provinces_image != std::nullopt) {
                last_layer = &l;
            }
        }

        if (last_layer != nullptr) {
            last_layer->provinces_image->populate_adjacent_colors(rgb, result);
        }
    }

    int screen_to_pixel(glm::vec2 screen) {
        auto w = get_provinces_image_x();
        auto h = get_provinces_image_y();
        return std::clamp(int(std::floor(screen.y) * w + std::floor(screen.x)), 0, w * h - 1);
    }
    glm::vec2 sample_province_index_texture_coord(int pixel) {
        return {(float)(indices.data[pixel * 4])/ 256.f, (float)(indices.data[pixel * 4 + 1])/ 256.f};
    }
    glm::ivec2 sample_province_index_pair(int pixel) {
        return {(float)(indices.data[pixel * 4]), (float)(indices.data[pixel * 4 + 1])};
    }
    int sample_province_index(int pixel) {
        auto index_pair = sample_province_index_pair(pixel);
        return index_pair.x + index_pair.y * 256;
    }
    color sample_province_color(int pixel) {
        color result {0, 0, 0};
        layer* last_layer = nullptr;
        for (auto& l: data) {
            if (l.visible && l.provinces_image != std::nullopt) {
                last_layer = &l;
            }
        }

        result.r = last_layer->provinces_image->provinces_image_data[pixel * 4];
        result.g = last_layer->provinces_image->provinces_image_data[pixel * 4 + 1];
        result.b = last_layer->provinces_image->provinces_image_data[pixel * 4 + 2];
        return  result;
    }
    bool sample_province_is_sea(int province) {
        bool result = false;
        for (auto& l: data) {
            if (l.visible && l.has_default_map) {
                result = l.province_is_sea[province];
            }
        }
        return result;
    }

    bool can_edit_province_definition(int province_index) {
        if (current_layer_index == -1) {
            return false;
        }
        return data[current_layer_index].has_province_definitions;
    }

    game_definition::province* sample_province_definition(int pixel) {
        game_definition::province* result = nullptr;
        auto index = sample_province_index(pixel);
        for (auto& l: data)
            if (l.visible && l.has_province_definitions)
                if (l.v2id_to_vector_position.contains(index)) {
                    auto vector_position = l.v2id_to_vector_position[index];
                    result = &l.province_definitions[vector_position];
                }
        return result;
    }

    std::optional<int> rgb_to_v2id(uint32_t rgb) {
        std::optional<int> index = std::nullopt;
        for (auto& l: data) {
            if(l.visible && l.has_province_definitions) {
                if (l.rgb_to_v2id.contains(rgb)) {
                    index = l.rgb_to_v2id[rgb];
                }
            }
        }
        return index;
    }

    std::optional<int> rgb_to_v2id(uint8_t r, uint8_t g, uint8_t b) {
        auto rgb = rgb_to_uint(r, g, b);
        std::optional<int> index = std::nullopt;
        for (auto& l: data) {
            if(l.visible && l.has_province_definitions) {
                if (l.rgb_to_v2id.contains(rgb)) {
                    index = l.rgb_to_v2id[rgb];
                }
            }
        }
        return index;
    }

    bool can_set_pixel(int pixel) {
        if (current_layer_index == -1) return false;
        return data[current_layer_index].provinces_image != std::nullopt;
    }

    void set_pixel(int pixel, uint8_t r, uint8_t g, uint8_t b) {
        if (current_layer_index == -1) return;
        auto& active_layer = data[current_layer_index];
        if (active_layer.provinces_image == std::nullopt) return;

        auto x = active_layer.provinces_image->size_x;
        auto y = active_layer.provinces_image->size_y;

        auto v2id = rgb_to_v2id(r, g, b);
        auto old_v2id = sample_province_index(pixel);

        active_layer.provinces_image->provinces_image_data[pixel * 4] = r;
        active_layer.provinces_image->provinces_image_data[pixel * 4 + 1] = g;
        active_layer.provinces_image->provinces_image_data[pixel * 4 + 2] = b;
        indices.update_texture_part = true;
        int coord_y = pixel / x;
        int coord_x = pixel - (coord_y * x);
        indices.update_texture_x_bottom = std::min(coord_x, indices.update_texture_x_bottom);
        indices.update_texture_y_bottom = std::min(coord_y, indices.update_texture_y_bottom);
        indices.update_texture_x_top = std::max(coord_x, indices.update_texture_x_top);
        indices.update_texture_y_top = std::max(coord_y, indices.update_texture_y_top);

        if (v2id != std::nullopt) {
            indices.v2id_to_size[old_v2id] -= 1;
            indices.v2id_to_size[v2id.value()] += 1;
            indices.data[4 * pixel + 0] = v2id.value() % 256;
            indices.data[4 * pixel + 1] = v2id.value() / 256;
        } else {
            indices.v2id_to_size[old_v2id] -= 1;
            indices.data[4 * pixel + 0] = 0;
            indices.data[4 * pixel + 1] = 0;
        }
    }

    bool can_edit_states() {
        if (current_layer_index == -1) return false;
        return data[current_layer_index].has_region_txt;
    }
    bool can_edit_cultures() {
        if (current_layer_index == -1) return false;
        return data[current_layer_index].has_cultures;
    }

    void copy_state_data_to_active_layer() {
        if (current_layer_index == -1) return;
        auto& active_layer = data[current_layer_index];
        if (!active_layer.has_region_txt) {
            // find layer with region txt
            layer* source_layer = nullptr;
            for (auto& l: data) {
                if(l.visible && l.has_region_txt) {
                    source_layer = &l;
                }
            }
            if (source_layer != nullptr) {
                std::memcpy(active_layer.province_state, source_layer->province_state, 256 * 256 * 2);
                active_layer.states = source_layer->states;
                active_layer.has_region_txt = true;
                active_layer.load_state_texture_to_gpu();
            } else {
                return;
            }
        };
    }

    void new_state_at_province(int v2id, std::string name) {
        copy_state_data_to_active_layer();
        auto& active_layer = data[current_layer_index];

        auto new_state = game_definition::state {};
        if (name.size() == 0) {
            new_state.name = "EDITOR_" + std::to_string(active_layer.states.size());
        } else {
            new_state.name = name;
        }
        auto new_state_id = active_layer.states.size();
        auto state_x = new_state_id % 256;
        auto state_y = new_state_id / 256;
        active_layer.states.push_back(new_state);
        active_layer.province_state[2 * v2id] = state_x;
        active_layer.province_state[2 * v2id + 1] = state_y;
    }

    void copy_state_from_province_to_province(int source, int target) {
        copy_state_data_to_active_layer();
        auto& active_layer = data[current_layer_index];
        active_layer.province_state[2 * target] = active_layer.province_state[2 * source];
        active_layer.province_state[2 * target + 1] = active_layer.province_state[2 * source + 1];
    }

    game_definition::state* get_state(int v2id) {
        if (v2id == 0) {
            return nullptr;
        }
        layer* source_layer = nullptr;
        for (auto& l: data) {
            if(l.visible && l.has_region_txt) {
                source_layer = &l;
            }
        }
        if (source_layer != nullptr) {
            auto index_x = source_layer->province_state[2 * v2id];
            auto index_y = source_layer->province_state[2 * v2id + 1];
            return &source_layer->states[index_x + index_y * 256];
        }
        return nullptr;
    }

    bool can_edit_province_history(int province_index) {
        if (current_layer_index == -1) {
            return false;
        }
        return data[current_layer_index].province_history.contains(province_index);
    }


    bool can_edit_nation_history(int tag) {
        if (current_layer_index == -1) {
            return false;
        }
        return data[current_layer_index].tag_to_nation_history.contains(tag);
    }
    bool can_edit_nation_definition(int tag) {
        if (current_layer_index == -1) {
            return false;
        }
        return data[current_layer_index].tag_to_nation_history.contains(tag);
    }
    bool can_edit_nation_common(int tag) {
        auto def = get_nation_definition(tag);
        if (def == nullptr) {
            return false;
        }
        if (current_layer_index == -1) {
            return false;
        }
        return data[current_layer_index].filename_to_nation_common.contains(def->filename);
    }

    game_definition::province_history* get_province_history(int province_index) {
        // get latest history
        game_definition::province_history* result = nullptr;

        for (auto& l: data) {
            if(l.visible && l.province_history.contains(province_index)) {
                result = &l.province_history[province_index];
            }
        }

        return result;
    }

    bool has_tech_key(std::wstring folder, std::string key) {
        // CAUTION: this could fail
        layer* last = nullptr;
        for (auto& l: data) {
            if(l.visible && l.has_tech.contains(folder)) {
                last = &l;
            }
        }
        if (last==nullptr)
            return false;
        auto it = last->tech.find(key);
        if (it == last->tech.end()) return false;
        return true;
    }

    bool has_tech_key(std::string key) {
        bool result = false;

        ankerl::unordered_dense::map<std::wstring, bool> registered_folders;
        for (auto& l: data) {
            for (auto& [key, value] : l.has_tech) {
                registered_folders[key] = true;
            }
        }

        for (auto& [folder, value] : registered_folders) {
            result = result || has_tech_key(folder, key);
        }

        return result;
    }

    bool has_invention_key(std::wstring folder, std::string key) {
        // CAUTION: this could fail
        layer* last = nullptr;
        for (auto& l: data) {
            if(l.visible && l.has_invention.contains(folder)) {
                last = &l;
            }
        }
        if (last==nullptr)
            return false;
        auto it = last->inventions.find(key);
        if (it == last->inventions.end()) return false;
        return true;
    }
    bool has_invention_key(std::string key) {
        bool result = false;

        ankerl::unordered_dense::map<std::wstring, bool> registered_folders;
        for (auto& l: data) {
            for (auto& [key, value] : l.has_invention) {
                registered_folders[key] = true;
            }
        }

        for (auto& [folder, value] : registered_folders) {
            result = result || has_invention_key(folder, key);
        }
        return result;
    }

    bool has_issues_key(std::string key) {
        // CAUTION: this could fail
        layer* last = nullptr;
        for (auto& l: data) {
            if(l.visible && l.has_issues) {
                last = &l;
            }
        }
        if (last==nullptr)
            return false;
        return last->issues.find(key) != last->issues.end();
    }

    void retrieve_tech_folders(std::vector<std::wstring>& folders) {
        ankerl::unordered_dense::map<std::wstring, bool> registered_folders;
        for (auto& l: data) {
            for (auto& [key, value] : l.has_tech) {
                if (registered_folders.contains(key)) {

                } else {
                    registered_folders[key] = true;
                    folders.push_back(key);
                }
            }
        }
    }
    void retrieve_inventions_folders(std::vector<std::wstring>& folders) {
        ankerl::unordered_dense::map<std::wstring, bool> registered_folders;
        for (auto& l: data) {
            for (auto& [key, value] : l.has_invention) {
                if (registered_folders.contains(key)) {

                } else {
                    registered_folders[key] = true;
                    folders.push_back(key);
                }
            }
        }
    }

    void retrieve_techs(std::vector<std::string>& techs, std::wstring folder) {
        layer* last = nullptr;
        for (auto& l: data) {
            if(l.visible && l.has_tech.contains(folder)) {
                last = &l;
            }
        }
        if (last==nullptr)
            return;
        for (auto const & [key, value] : last->tech) {
            if (value.folder == folder) {
                techs.push_back(key);
            }
        }
    }

    std::vector<std::string> retrieve_cultures() {
        std::vector<std::string> cultures;
        layer* last = nullptr;
        for (auto& l: data) {
            if(l.visible && l.has_cultures) {
                last = &l;
            }
        }
        if (last!=nullptr) {
            for (auto c : last->cultures) {
                cultures.push_back(c);
            }
        }
        return cultures;
    }

    std::vector<std::string> retrieve_culture_groups() {
        std::vector<std::string> result;
        layer* last = nullptr;
        for (auto& l: data) {
            if(l.visible && l.has_cultures) {
                last = &l;
            }
        }
        if (last!=nullptr) {
            for (auto c : last->culture_groups) {
                result.push_back(c);
            }
        }
        return result;
    }

    std::vector<std::string> retrieve_religions() {
        std::vector<std::string> result;
        layer* last = nullptr;
        for (auto& l: data) {
            if(l.visible && l.has_religions) {
                last = &l;
            }
        }
        if (last!=nullptr) {
            for (auto c : last->religions) {
                result.push_back(c);
            }
        }
        return result;
    }

    std::vector<std::string> retrieve_poptypes() {
        std::vector<std::string> poptypes;
        for (auto& l: data) {
            if(l.visible) {
                for (auto& pt : l.poptypes) {
                    bool already_added = false;
                    for (auto& added_pt : poptypes) {
                        if (added_pt.compare(pt) == 0) {
                            already_added = true;
                            break;
                        }
                    }
                    if (!already_added) {
                        poptypes.push_back(pt);
                    }
                }
            }
        }
        return poptypes;
    }

    void retrieve_inventions(std::vector<std::string>& inventions, std::wstring folder) {
        layer* last = nullptr;
        for (auto& l: data) {
            if(l.visible && l.has_invention.contains(folder)) {
                last = &l;
            }
        }
        if (last==nullptr)
            return;
        for (auto const & [key, value] : last->inventions) {
            if (value.invention_file == folder) {
                inventions.push_back(key);
            }
        }
    }

    game_definition::issue* get_issue(std::string issue_name) {
        game_definition::issue* result = nullptr;
        for (auto& l: data) {
            if(l.visible && l.issues.contains(issue_name)) {
                result = &l.issues[issue_name];
            }
        }
        return result;
    }

    void retrieve_issues(std::vector<std::string>& issues) {
        layer* last = nullptr;
        for (auto& l: data) {
            if(l.visible && l.has_issues) {
                last = &l;
            }
        }
        if (last==nullptr)
            return;
        for (auto const & [key, value] : last->issues) {
            issues.push_back(key);
        }
    }

    game_definition::nation_history* get_nation_history(int tag) {
        // get latest history
        game_definition::nation_history* result = nullptr;
        for (auto& l: data) {
            if(l.visible && l.tag_to_nation_history.contains(tag)) {
                result = &l.tag_to_nation_history[tag];
            }
        }
        return result;
    }
    game_definition::nation_definition* get_nation_definition(int tag) {
        // get latest history
        game_definition::nation_definition* result = nullptr;
        for (auto& l: data) {
            if(l.visible && l.tag_to_vector_position.contains(tag)) {
                result = &l.nations[l.tag_to_vector_position[tag]];
            }
        }
        return result;
    }
    game_definition::nation_common* get_nation_common(int tag) {
        auto def = get_nation_definition(tag);
        if (def == nullptr) {
            return nullptr;
        }
        // get latest history
        game_definition::nation_common* result = nullptr;
        for (auto& l: data) {
            if(l.visible && l.filename_to_nation_common.contains(def->filename)) {
                result = &l.filename_to_nation_common[def->filename];
            }
        }
        return result;
    }

    game_definition::province* get_province_definition(int v2id) {
        // get latest history
        game_definition::province* result = nullptr;

        for (auto& l: data) {
            if(l.visible && l.has_province_definitions && l.v2id_to_vector_position.contains(v2id)) {
                auto position = l.v2id_to_vector_position[v2id];
                result = &l.province_definitions[position];
            }
        }

        return result;
    }

    game_definition::commodity* get_commodity_definition(std::string name) {
        game_definition::commodity* result = nullptr;
        for (auto& l: data) {
            if(l.visible && l.has_goods) {
                auto iterator = l.goods.find(name);
                if (iterator != l.goods.end())
                    result = &iterator->second;
            }
        }
        return result;
    }

    std::vector<int> get_available_dates() {
        std::vector<int> result;
        for (auto& l: data) {
            for (auto& folder: l.province_population) {
                bool already_defined = false;
                for (auto x: result) {
                    if (x == folder.date) {
                        already_defined = true;
                    }
                }
                if (!already_defined)
                    result.push_back(folder.date);
            }
        }
        return result;
    }

    bool can_edit_pops(uint32_t v2id, int date) {
        auto& active_layer = data[current_layer_index];
        for (auto& folder : active_layer.province_population) {
            if (folder.date != date) {
                continue;
            }

            for (auto& file : folder.data) {
                auto iterator = file.data.find(v2id);
                if (iterator != file.data.end())
                    return true;
            }
        }
        return false;
    }

    void copy_pops_data_to_current_layer(uint32_t v2id, int date) {
        if (can_edit_pops(v2id, date)) {
            return;
        }
        auto pops = get_pops(v2id, date);
        auto& active_layer = data[current_layer_index];
        bool date_exists = false;
        for (auto& folder : active_layer.province_population) {
            if (folder.date == date) {
                date_exists = true;
                bool editor_generated_population_exists = false;
                for (auto& file : folder.data) {
                    if (file.filename == "editor-pops.txt") {
                        editor_generated_population_exists = true;
                        file.data[v2id] = {};
                        if (pops != nullptr)
                            for (auto& pop : *pops) {
                                file.data[v2id].push_back(pop);
                            }
                    }
                }

                if (!editor_generated_population_exists) {
                    game_definition::pops_history_file file {
                        "editor-pops.txt", {}
                    };
                    if (pops != nullptr)
                        for (auto& pop : *pops) {
                            file.data[v2id].push_back(pop);
                        }
                    folder.data.push_back(file);
                }
            }
        }

        if (!date_exists) {
            game_definition::pops_setups folder {
                date, {}
            };
            game_definition::pops_history_file file {
                "editor-pops.txt", {}
            };
            if (pops != nullptr)
                for (auto& pop : *pops) {
                    file.data[v2id].push_back(pop);
                }
            folder.data.push_back(file);
            active_layer.province_population.push_back(folder);
        }
    }

    void create_new_population_list(uint32_t v2id, int date) {
        if (can_edit_pops(v2id, date)) {
            return;
        }
        auto& active_layer = data[current_layer_index];
        auto editor_pops_already_exist = false;
        // find existing editor-pops.txt:
        for (auto& folder : active_layer.province_population) {
            if (folder.date == date) {
                for (auto& file : folder.data) {
                    if (file.filename == "editor-pops.txt") {
                        editor_pops_already_exist = true;
                        file.data[v2id] = {};
                    }
                }
            }
        }
        if (!editor_pops_already_exist) {
            game_definition::pops_setups folder {
                date, {}
            };
            game_definition::pops_history_file file {
                "editor-pops.txt", {}
            };
            file.data[v2id] = {};
            folder.data.push_back(file);
            active_layer.province_population.push_back(folder);
        }
    }

    std::vector<game_definition::pop_history> * get_pops(uint32_t v2id, int date) {
        std::vector<game_definition::pop_history> * result = nullptr;
        for (auto& l : data) {
            for (auto& folder: l.province_population) {
                if (folder.date != date) {
                    continue;
                }
                for (auto& file : folder.data) {
                    auto iterator = file.data.find(v2id);
                    if (iterator != file.data.end())
                        result = &file.data[v2id];
                }
            }
        }
        return result;
    }

    std::vector<game_definition::province> * get_provinces() {
        std::vector<game_definition::province> * result = nullptr;
        for (auto& l : data) {
            if (l.has_province_definitions) {
                result = &l.province_definitions;
            }
        }
        return result;
    }

    int get_commodities_count() {
        int result = 1;
        for (auto& l: data) {
            if(l.visible && l.has_goods) {
                result = l.goods.size();
            }
        }
        return result;
    }

    game_definition::culture* get_culture(std::string culture) {
        layer* source = nullptr;
        for (auto& l: data) {
            if (l.visible && l.has_cultures) {
                source = &l;
            }
        }

        if (source == nullptr) {
            return nullptr;
        }

        auto find = source->culture_defs.find(culture);
        if (find == source->culture_defs.end()) {
            return nullptr;
        }

        return &source->culture_defs[culture];
    }

    game_definition::culture_group* get_culture_group_from_culture(std::string culture) {
        layer* source = nullptr;
        for (auto& l: data) {
            if (l.visible && l.has_cultures) {
                source = &l;
            }
        }

        if (source == nullptr) {
            return nullptr;
        }

        auto group_string_iterator = source->culture_to_group.find(culture);
        if (group_string_iterator == source->culture_to_group.end()) {
            return nullptr;
        }

        auto& group = group_string_iterator->second;

        auto find = source->culture_group_defs.find(group);
        if (find == source->culture_group_defs.end()) {
            return nullptr;
        }

        return &source->culture_group_defs[group];
    }

    void copy_province_history_to_current_layer(int v2id) {
        auto available_history = get_province_history(v2id);
        data[current_layer_index].province_history[v2id] = *available_history;
    }

    void set_province_history(int v2id) {
        data[current_layer_index].province_history[v2id] = {};
    }

    void copy_nation_history_to_current_layer(int tag) {
        auto available_history = get_nation_history(tag);
        data[current_layer_index].tag_to_nation_history[tag] = *available_history;
    }

    void copy_nation_common_to_current_layer(int tag) {
        auto def = get_nation_definition(tag);
        if (def == nullptr) {
            return;
        }
        auto common = get_nation_common(tag);
        data[current_layer_index].filename_to_nation_common[def->filename] = *common;
    }

    void copy_nations_list_to_current_layer() {
        layer* source = nullptr;
        for (auto& l: data) {
            if (l.visible && l.has_nations_list) {
                source = &l;
            }
        }

        auto& active_layer = data[current_layer_index];

        if (source != nullptr) {
            active_layer.has_nations_list = true;
            active_layer.nations = source->nations;
        }
    }

    void copy_resources_to_current_layer(){
        auto& active_layer = data[current_layer_index];
        if (!active_layer.resources_small.valid()) {
            layer* source = nullptr;
            for (auto& l: data) {
                if (l.visible && l.resources_small.valid()) {
                    source = &l;
                }
            }

            if (source != nullptr) {
                active_layer.resources_small = source->resources_small;
            }
        }
        if (!active_layer.resources_medium.valid()) {
            layer* source = nullptr;
            for (auto& l: data) {
                if (l.visible && l.resources_medium.valid()) {
                    source = &l;
                }
            }

            if (source != nullptr) {
                active_layer.resources_medium = source->resources_medium;
            }
        }
        if (!active_layer.resources_big.valid()) {
            layer* source = nullptr;
            for (auto& l: data) {
                if (l.visible && l.resources_big.valid()) {
                    source = &l;
                }
            }

            if (source != nullptr) {
                active_layer.resources_big = source->resources_big;
            }
        }
    }

    void copy_province_map_to_current_layer() {
        layer* result = nullptr;
        for (auto& l: data) {
            if (l.visible && l.provinces_image != std::nullopt) {
                result = &l;
            }
        }

        auto& active_layer = data[current_layer_index];

        if (result != nullptr) {
            active_layer.provinces_image = result->provinces_image.value();
        }
    }

    void copy_goods_to_current_layer() {
        layer* source = nullptr;
        for (auto& l: data) {
            if (l.visible && l.has_goods) {
                source = &l;
            }
        }

        auto& active_layer = data[current_layer_index];

        if (source != nullptr) {
            active_layer.has_goods = true;
            active_layer.goods = source->goods;
        }
    }

    void set_owner(int province, std::string owner_tag) {
        if (!can_edit_province_history(province)) {
            return;
        }
        data[current_layer_index].province_history[province].owner_tag = owner_tag;
        province_owner[3 * province + 0] = owner_tag[0];
        province_owner[3 * province + 1] = owner_tag[1];
        province_owner[3 * province + 2] = owner_tag[2];
    }

    int32_t get_owner_int(int v2id) {
        return game_definition::tag_to_int(
            {
                (char)province_owner[3 * v2id + 0],
                (char)province_owner[3 * v2id + 1],
                (char)province_owner[3 * v2id + 2]
            }
        );
    }

    int inline coord_to_pixel(glm::ivec2 coord) {
        int result = 0;
        for (auto& l: data) {
            if(l.visible && l.provinces_image != std::nullopt) {
                result = l.provinces_image->coord_to_pixel(coord);
            }
        }
        return result;
    }
    int inline coord_to_pixel(glm::vec2 coord) {
        int result = 0;
        for (auto& l: data) {
            if(l.visible && l.provinces_image != std::nullopt) {
                result = l.provinces_image->coord_to_pixel(coord);
            }
        }
        return result;
    }

    bool valid_tag(int32_t tag) {
        auto def = get_nation_definition(tag);
        if (def == nullptr) {
            return true;
        } else {
            return false;
        }
    }

    void copy_cultures_to_active_layer() {
        auto& active_layer = data[current_layer_index];
        if (active_layer.has_cultures) {
            return;
        }

        layer* source = nullptr;
        for (auto& l: data) {
            if (l.visible && l.has_nations_list) {
                source = &l;
            }
        }

        active_layer.culture_defs = source->culture_defs;
        active_layer.culture_group_defs = source->culture_group_defs;
        active_layer.culture_to_group = source->culture_to_group;
        active_layer.cultures = source->cultures;
        active_layer.culture_groups = source->culture_groups;
        active_layer.has_cultures = true;
    }

    void new_culture(std::string name, std::string culture_group, int r, int g, int b) {
        auto& active_layer = data[current_layer_index];
        copy_cultures_to_active_layer();

        active_layer.cultures.push_back(name);
        active_layer.culture_to_group[name] = culture_group;
        active_layer.culture_group_defs[culture_group].cultures.push_back(name);

        game_definition::culture result {};
        result.name = name;
        result.r = r;
        result.g = g;
        result.b = b;
        active_layer.culture_defs[name] = result;
    }

    void new_nation(int32_t source_tag, int32_t tag, std::string filename) {
        auto& active_layer = data[current_layer_index];

        if (!active_layer.has_nations_list) {
            copy_nations_list_to_current_layer();
        }

        game_definition::nation_definition def_template = {
            game_definition::int_to_tag(tag),
            filename,
            false
        };

        active_layer.nations.push_back(def_template);
        active_layer.tag_to_vector_position[tag] = active_layer.nations.size() - 1;

        auto& def = active_layer.nations.back();

        //generate history by cloning source if it exists
        auto source_history = get_nation_history(source_tag);
        game_definition::nation_history history = {};
        if (source_history != nullptr)
            history = *source_history;
        auto array_tag = game_definition::int_to_tag(tag);
        history.history_file_name = std::string{array_tag[0], array_tag[1], array_tag[2]} + " - " + filename;
        active_layer.tag_to_nation_history[tag] = history;

        // generate common by cloning souce if it exists
        auto source_common = get_nation_common(source_tag);
        game_definition::nation_common c = {};
        if (source_common != nullptr)
            c = *source_common;

        active_layer.filename_to_nation_common[filename] = c;
    };

    game_definition::province& new_province(uint32_t pixel, std::string name) {

        // when we create a new province, we have to:
        // 1) update the rgb province map and available rgb
        // 2) update available id and used ids
        // 3) create history definition if the province is not a sea province
        // 4) create province definition
        // 5) update the set of sea provinces
        // it requires collaboration of several layers, so we place this function to the layer stack

        auto& active_layer = data[current_layer_index];

        // copy provinces map to the top if it is missing
        if (active_layer.provinces_image == std::nullopt) {
            copy_province_map_to_current_layer();
        }

        // copy province definitions
        if (!active_layer.has_province_definitions) {
            layer* latest_layer_with_definitions = nullptr;
            for (auto& l: data) {
                if (l.visible && l.has_province_definitions) {
                    latest_layer_with_definitions = &l;
                }
            }
            if (latest_layer_with_definitions != nullptr) {
                active_layer.province_definitions = latest_layer_with_definitions->province_definitions;
                active_layer.v2id_to_vector_position = latest_layer_with_definitions->v2id_to_vector_position;
                active_layer.is_used = latest_layer_with_definitions->is_used;
                active_layer.rgb_to_v2id = latest_layer_with_definitions->rgb_to_v2id;
                active_layer.has_province_definitions = true;
            }
        }

        // copy default dot map
        if (!active_layer.has_default_map) {
            layer* latest_layer_with_default = nullptr;
            for (auto& l: data) {
                if (l.visible && l.has_default_map) {
                    latest_layer_with_default = &l;
                }
            }
            if (latest_layer_with_default != nullptr) {
                std::copy(
                    latest_layer_with_default->province_is_sea,
                    latest_layer_with_default->province_is_sea + sizeof(latest_layer_with_default->province_is_sea),
                    active_layer.province_is_sea
                );
                active_layer.has_default_map = true;
            }
        }

        // copy regions:
        if (!active_layer.has_region_txt) {
            layer* latest_layer_with_data = nullptr;
            for (auto& l: data) {
                if (l.visible && l.has_region_txt) {
                    latest_layer_with_data = &l;
                }
            }
            if (latest_layer_with_data != nullptr) {
                std::copy(
                    latest_layer_with_data->province_state,
                    latest_layer_with_data->province_state + sizeof(latest_layer_with_data->province_state),
                    active_layer.province_state
                );
                active_layer.has_region_txt = true;
            }
        }

        auto c = sample_province_color(pixel);
        auto old_rgb = rgb_to_uint(c.r, c.g, c.b);
        auto old_v2id = sample_province_index(pixel);

        // update available colors
        active_layer.provinces_image->update_available_colors();

        // update ids
        while (active_layer.is_used[active_layer.available_id])
            active_layer.available_id += 1;
        active_layer.is_used[active_layer.available_id] = true;

        // create new definition and upload it into arrays:

        if (name.size() == 0) {
            name = "UnknownProv" + std::to_string(active_layer.available_id);
        }

        game_definition::province def_template = {
            active_layer.available_id,
            name,
            active_layer.provinces_image->available_r,
            active_layer.provinces_image->available_g,
            active_layer.provinces_image->available_b
        };

        active_layer.v2id_to_vector_position[def_template.v2id] = active_layer.province_definitions.size();
        active_layer.province_definitions.push_back(def_template);

        auto& def = active_layer.province_definitions[active_layer.province_definitions.size() - 1];

        auto new_rgb = rgb_to_uint(def.r, def.g, def.b);
        active_layer.rgb_to_v2id[new_rgb] = def.v2id;

        // create province history

        game_definition::province_history p_new {};

        p_new.history_file_name = std::to_wstring(active_layer.available_id) + L" - " + conversions::utf8_to_wstring(def.name) + L".txt";

        auto p_old = get_province_history(old_v2id);
            // check if it is a sea
            if (p_old != nullptr) {


            p_new.life_rating = p_old->life_rating;
            p_new.owner_tag = p_old->owner_tag;
            p_new.controller_tag = p_old->controller_tag;
            p_new.main_trade_good = p_old->main_trade_good;
            p_new.colonial = p_old->colonial;
            p_new.historical_region = p_old->historical_region;

            for (auto& core: p_old->cores) {
                p_new.cores.push_back(core);
            }

            active_layer.province_history[def.v2id] = p_new;
        }


        //finally, update colors
        set_pixel(pixel, def.r, def.g, def.b);
        active_layer.provinces_image->recalculate_present_colors();
        indices.commit_province_texture_changes_to_gpu();

        // inherit seas, state and ownership arrays

        active_layer.province_is_sea[def.v2id] = active_layer.province_is_sea[old_v2id];
        active_layer.commit_sea_texture_to_gpu();

        active_layer.province_state[2 * def.v2id] = active_layer.province_state[2 * old_v2id];
        active_layer.province_state[2 * def.v2id + 1] = active_layer.province_state[2 * old_v2id + 1];
        active_layer.commit_state_texture_to_gpu();

        province_owner[3 * def.v2id] = province_owner[3 * old_v2id];
        province_owner[3 * def.v2id + 1] = province_owner[3 * old_v2id + 1];
        province_owner[3 * def.v2id + 2] = province_owner[3 * old_v2id + 2];
        commit_owner_texture_to_gpu();


        return def;
    }

    void save_population_texture();

    void update_adj_buffers(GLuint buffer, int& counter) {
        layer* adj_source = nullptr;
        for (auto& l: data) {
            if (l.visible && l.has_adjacencies) {
                adj_source = &l;
            }
        }
        if (adj_source == nullptr) {
            return;
        }

        layer* provinces_source = nullptr;
        for (auto& l: data) {
            if (l.visible && l.provinces_image != std::nullopt) {
                provinces_source = &l;
            }
        }
        if (provinces_source == nullptr) {
            return;
        }

        layer* definitions_source = nullptr;
        for (auto& l: data) {
            if (l.visible && l.has_province_definitions) {
                definitions_source = &l;
            }
        }
        if (definitions_source == nullptr) {
            return;
        }

        std::vector<float> vertices_for_adj;

        auto& v2id_to_vector_pos = definitions_source->v2id_to_vector_position;
        auto& definitions = definitions_source->province_definitions;

        auto size_x = provinces_source->provinces_image->size_x;
        auto size_y = provinces_source->provinces_image->size_y;

        for (auto& adj : adj_source->adjacencies) {

            if (adj.mark_for_delete) {
                continue;
            }

            auto pos_start = indices.v2id_to_mean[adj.from];
            auto pos_through = indices.v2id_to_mean[adj.through];
            auto pos_to = indices.v2id_to_mean[adj.to];

            if (adj.through != 0) {
                // triangle 1
                // vertex 1
                vertices_for_adj.push_back(pos_start.x / (float)size_x * 2.f - 1.f - 0.0001);
                vertices_for_adj.push_back(pos_start.y / (float)size_y * 2.f - 1.f - 0.0001);

                // vertex 2:
                vertices_for_adj.push_back(pos_start.x / (float)size_x * 2.f - 1.f + 0.0001);
                vertices_for_adj.push_back(pos_start.y / (float)size_y * 2.f - 1.f + 0.0001);

                // vertex 3:
                vertices_for_adj.push_back(pos_through.x / (float)size_x * 2.f - 1.f);
                vertices_for_adj.push_back(pos_through.y / (float)size_y * 2.f - 1.f);


                // triangle 2
                // vertex 1
                vertices_for_adj.push_back(pos_through.x / (float)size_x * 2.f - 1.f - 0.0001);
                vertices_for_adj.push_back(pos_through.y / (float)size_y * 2.f - 1.f - 0.0001);

                // vertex 2:
                vertices_for_adj.push_back(pos_through.x / (float)size_x * 2.f - 1.f + 0.0001);
                vertices_for_adj.push_back(pos_through.y / (float)size_y * 2.f - 1.f + 0.0001);

                // vertex 3:
                vertices_for_adj.push_back(pos_to.x / (float)size_x * 2.f - 1.f);
                vertices_for_adj.push_back(pos_to.y / (float)size_y * 2.f - 1.f);

                // triangle 3
                // vertex 1
                vertices_for_adj.push_back(pos_start.x / (float)size_x * 2.f - 1.f - 0.0001);
                vertices_for_adj.push_back(pos_start.y / (float)size_y * 2.f - 1.f + 0.0001);

                // vertex 2:
                vertices_for_adj.push_back(pos_start.x / (float)size_x * 2.f - 1.f + 0.0001);
                vertices_for_adj.push_back(pos_start.y / (float)size_y * 2.f - 1.f - 0.0001);

                // vertex 3:
                vertices_for_adj.push_back(pos_through.x / (float)size_x * 2.f - 1.f);
                vertices_for_adj.push_back(pos_through.y / (float)size_y * 2.f - 1.f);


                // triangle 4
                // vertex 1
                vertices_for_adj.push_back(pos_through.x / (float)size_x * 2.f - 1.f - 0.0001);
                vertices_for_adj.push_back(pos_through.y / (float)size_y * 2.f - 1.f + 0.0001);

                // vertex 2:
                vertices_for_adj.push_back(pos_through.x / (float)size_x * 2.f - 1.f + 0.0001);
                vertices_for_adj.push_back(pos_through.y / (float)size_y * 2.f - 1.f - 0.0001);

                // vertex 3:
                vertices_for_adj.push_back(pos_to.x / (float)size_x * 2.f - 1.f);
                vertices_for_adj.push_back(pos_to.y / (float)size_y * 2.f - 1.f);
            } else {
                if (adj.to != 0) {
                    // triangle 1
                    // vertex 1
                    vertices_for_adj.push_back(pos_start.x / (float)size_x * 2.f - 1.f - 0.0001);
                    vertices_for_adj.push_back(pos_start.y / (float)size_y * 2.f - 1.f - 0.0001);

                    // vertex 2:
                    vertices_for_adj.push_back(pos_start.x / (float)size_x * 2.f - 1.f + 0.0001);
                    vertices_for_adj.push_back(pos_start.y / (float)size_y * 2.f - 1.f + 0.0001);

                    // vertex 3:
                    vertices_for_adj.push_back(pos_to.x / (float)size_x * 2.f - 1.f);
                    vertices_for_adj.push_back(pos_to.y / (float)size_y * 2.f - 1.f);

                    // triangle 2
                    // vertex 1
                    vertices_for_adj.push_back(pos_start.x / (float)size_x * 2.f - 1.f - 0.0001);
                    vertices_for_adj.push_back(pos_start.y / (float)size_y * 2.f - 1.f + 0.0001);

                    // vertex 2:
                    vertices_for_adj.push_back(pos_start.x / (float)size_x * 2.f - 1.f + 0.0001);
                    vertices_for_adj.push_back(pos_start.y / (float)size_y * 2.f - 1.f - 0.0001);

                    // vertex 3:
                    vertices_for_adj.push_back(pos_to.x / (float)size_x * 2.f - 1.f);
                    vertices_for_adj.push_back(pos_to.y / (float)size_y * 2.f - 1.f);
                }
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, buffer);

        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(float) * vertices_for_adj.size(),
            vertices_for_adj.data(),
            GL_STATIC_DRAW
        );

        counter = vertices_for_adj.size();
    }

    std::vector<std::string>* get_flags(){
        std::vector<std::string>* result = nullptr;
        for (auto& l: data) {
            if (l.visible && l.has_governments_list) {
                result = &l.detected_flags;
            }
        }
        return result;
    }

    bool get_owners_texture(GLuint& result) {
        result = owner_texture;
        return true;
    }
    bool get_sea_texture(GLuint& result){
        bool found = false;
        for (auto& l: data) {
            if (l.visible && l.has_default_map) {
                result = l.sea_texture;
                found = true;
            }
        }
        return found;
    }
    bool get_resources_texture_small(GLuint& result, int& x, int& y){
        bool found = false;
        for (auto& l: data) {
            if (l.visible && l.resources_small.valid()) {
                result = l.resources_small.texture_id;
                x = l.resources_small.size_x;
                y = l.resources_small.size_y;
                found = true;
            }
        }
        return found;
    }
    bool get_resources_texture_big(GLuint& result, int& x, int& y){
        bool found = false;
        for (auto& l: data) {
            if (l.visible && l.resources_big.valid()) {
                result = l.resources_big.texture_id;
                x = l.resources_big.size_x;
                y = l.resources_big.size_y;
                found = true;
            }
        }
        return found;
    }
    bool get_resources_texture_medium(GLuint& result, int& x, int& y){
        bool found = false;
        for (auto& l: data) {
            if (l.visible && l.resources_medium.valid()) {
                result = l.resources_medium.texture_id;
                x = l.resources_medium.size_x;
                y = l.resources_medium.size_y;
                found = true;
            }
        }
        return found;
    }
    bool get_state_texture(GLuint& result){
        bool found = false;
        for (auto& l: data) {
            if (l.visible && l.has_region_txt) {
                result = l.state_texture;
                found = true;
            }
        }
        return found;
    }

    void commit_state_texture() {
        layer* layer = nullptr;
        for (auto& l: data) {
            if (l.visible && l.has_region_txt) {
                layer = &l;
            }
        }
        if (layer != nullptr) {
            layer->commit_state_texture_to_gpu();
        }
    }


};
}