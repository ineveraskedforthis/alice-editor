#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "editor-state.hpp"

#undef max
#undef min
#undef clamp

namespace state {

int coord_to_pixel(parsing::game_map& map, glm::ivec2 coord) {
    return coord.y * map.size_x + coord.x;
}

int coord_to_pixel(parsing::game_map& map, glm::vec2 coord) {
    return int(std::floor(coord.y))
        * map.size_x
        + int(std::floor(coord.x));
}

int inline pairing(glm::ivec2 a, glm::ivec2 b) {
    return a.x * b.x + a.y * b.y;
}

void check_gl_error(std::string message) {
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "Error at ";
        std::cout << message << "\n";
        std::cout << "Gl Error " << error << ": ";
        std::cout << glewGetErrorString(error) << "\n";
    }
}

std::string selection_mode_string(SELECTION_MODE MODE) {
    switch (MODE) {
    case SELECTION_MODE::PROVINCE:
        return "Province";
        break;
    case SELECTION_MODE::NATION:
        return "Nation";
        break;
    }
}

std::string fill_mode_string(FILL_MODE MODE) {
    switch (MODE) {
    case FILL_MODE::PROVINCE:
        return "Province";
        break;
    case FILL_MODE::OWNER_AND_CONTROLLER:
        return "Owner&Control";
        break;
    }
}

void load_map_texture(control& control, parsing::game_map& map_state) {
    glGenTextures(1, &control.main_texture);
    glBindTexture(GL_TEXTURE_2D, control.main_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        map_state.size_x,
        map_state.size_y,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        map_state.data
    );
    check_gl_error("Map texture update");

    glGenTextures(1, &control.rivers_texture);
    glBindTexture(GL_TEXTURE_2D, control.rivers_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        map_state.size_x,
        map_state.size_y,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        map_state.rivers_raw
    );
    check_gl_error("Rivers texture update");

    glGenTextures(1, &control.sea_texture);
    glBindTexture(GL_TEXTURE_2D, control.sea_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        256,
        256,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        map_state.province_is_sea
    );
    check_gl_error("Province texture update");

    glGenTextures(1, &control.state_texture);
    glBindTexture(GL_TEXTURE_2D, control.state_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RG,
        256,
        256,
        0,
        GL_RG,
        GL_UNSIGNED_BYTE,
        map_state.province_state
    );
    check_gl_error("Province texture update");

    glGenTextures(1, &control.nation_texture);
    glBindTexture(GL_TEXTURE_2D, control.nation_texture);
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
        map_state.province_owner
    );
    check_gl_error("Province texture update");
}

void update_map_texture(control& control, parsing::game_map& map_state) {
    if (control.update_texture) {
        glBindTexture(GL_TEXTURE_2D, control.main_texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            map_state.size_x,
            map_state.size_y,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            map_state.data
        );
        control.update_texture = false;
        control.update_texture_part = false;
    }

    if (control.update_texture_part) {
        glBindTexture(GL_TEXTURE_2D, control.main_texture);
        auto width = control.update_texture_x_top - control.update_texture_x_bottom;
        for (int y = control.update_texture_y_bottom; y <= control.update_texture_y_top; y++) {
            auto pixel_index = coord_to_pixel(map_state, glm::ivec2{control.update_texture_x_bottom, y});
            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                control.update_texture_x_bottom,
                y,
                width,
                1,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                map_state.data + pixel_index * 4
            );
        }

        control.update_texture_x_top = 0;
        control.update_texture_y_top = 0;
        control.update_texture_x_bottom = std::numeric_limits<int>::max();
        control.update_texture_y_bottom = std::numeric_limits<int>::max();
    }

    glBindTexture(GL_TEXTURE_2D, control.sea_texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        256,
        256,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        map_state.province_is_sea
    );

    glBindTexture(GL_TEXTURE_2D, control.state_texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RG,
        256,
        256,
        0,
        GL_RG,
        GL_UNSIGNED_BYTE,
        map_state.province_state
    );

    glBindTexture(GL_TEXTURE_2D, control.nation_texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        256,
        256,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        map_state.province_owner
    );
}

int pixel(control& control_state, parsing::game_map& map) {
    return int(std::floor(control_state.mouse_map_coord.y)
        * map.size_x
        + std::floor(control_state.mouse_map_coord.x));
}

void update_hover(control& control_state, parsing::game_map& map){
    auto pixel_index = pixel(control_state, map);
    control_state.hovered_province = glm::vec2((float)map.data[pixel_index * 4] / 256.f, (float)map.data[pixel_index * 4 + 1] / 256.f);
}

void update_select(control& control_state, parsing::game_map& map){
    auto pixel_index = pixel(control_state, map);
    control_state.selected_pixel = pixel_index;
    control_state.selected_province = glm::vec2((float)map.data[pixel_index * 4] / 256.f, (float)map.data[pixel_index * 4 + 1] / 256.f);
    control_state.selected_province_id = (int)map.data[pixel_index * 4] + (int)map.data[pixel_index * 4 + 1] * 256;
    control_state.selection_delay = true;
}

void update_context(control& control_state, parsing::game_map& map) {
    auto pixel_index = pixel(control_state, map);
    control_state.context_province = (int)map.data[pixel_index * 4] + (int)map.data[pixel_index * 4 + 1] * 256;
}

void pick_color(control& control_state, parsing::game_map& map) {
    auto pixel_index = pixel(control_state, map);
    control_state.r = map.data_raw[pixel_index * 4];
    control_state.g = map.data_raw[pixel_index * 4 + 1];
    control_state.b = map.data_raw[pixel_index * 4 + 2];

    auto selected_pixel = pixel_index;
    auto selected_province = glm::vec2((float)map.data[pixel_index * 4] / 256.f, (float)map.data[pixel_index * 4 + 1] / 256.f);
    auto selected_province_id = (int)map.data[pixel_index * 4] + (int)map.data[pixel_index * 4 + 1] * 256;

    auto& def = map.provinces[map.index_to_vector_position[selected_province_id]];

    control_state.fill_with_tag = def.owner_tag;
}

void paint(control& control_state, parsing::game_map& map) {
    auto pixel_index = pixel(control_state, map);

    map.data_raw[pixel_index * 4] = control_state.r;
    map.data_raw[pixel_index * 4 + 1] = control_state.g;
    map.data_raw[pixel_index * 4 + 2] = control_state.b;

    auto rgb = parsing::rgb_to_uint(control_state.r, control_state.g, control_state.b);

    auto index = map.rgb_to_index[rgb];
    map.data[4 * pixel_index + 0] = index % 256;
    map.data[4 * pixel_index + 1] = index / 256;
}

void paint_state(control& control_state, parsing::game_map& map) {
    auto prov_x = map.data[4 * control_state.selected_pixel];
    auto prov_y = map.data[4 * control_state.selected_pixel + 1];
    auto prov = (int)prov_x + (int)(prov_y) * 256;

    auto pixel_index = pixel(control_state, map);
    auto target_prov_x = map.data[4 * pixel_index];
    auto target_prov_y = map.data[4 * pixel_index + 1];
    auto target_prov = (int)target_prov_x + (int)(target_prov_y) * 256;
    map.province_state[2 * target_prov] = map.province_state[2 * prov];
    map.province_state[2 * target_prov+1] = map.province_state[2 * prov+1];
}

void paint_safe(control& control_state, parsing::game_map& map, int pixel_index, uint32_t province_index) {
    if (control_state.fill_mode == FILL_MODE::PROVINCE) {
        auto target_r = map.data_raw[pixel_index * 4];
        auto target_g = map.data_raw[pixel_index * 4 + 1];
        auto target_b = map.data_raw[pixel_index * 4 + 2];
        auto rgb_target = parsing::rgb_to_uint(target_r, target_g, target_b);
        auto index_target = map.rgb_to_index[rgb_target];

        if (map.province_is_sea[province_index] != map.province_is_sea[index_target]) {
            return;
        }

        map.data_raw[pixel_index * 4] = control_state.r;
        map.data_raw[pixel_index * 4 + 1] = control_state.g;
        map.data_raw[pixel_index * 4 + 2] = control_state.b;


        map.data[4 * pixel_index + 0] = province_index % 256;
        map.data[4 * pixel_index + 1] = province_index / 256;
        control_state.update_texture_part = true;
    } else if (control_state.fill_mode == FILL_MODE::OWNER_AND_CONTROLLER) {
        auto target_r = map.data_raw[pixel_index * 4];
        auto target_g = map.data_raw[pixel_index * 4 + 1];
        auto target_b = map.data_raw[pixel_index * 4 + 2];
        auto rgb_target = parsing::rgb_to_uint(target_r, target_g, target_b);
        auto index_target = map.rgb_to_index[rgb_target];
        if (map.province_is_sea[index_target]) {
            return;
        }

        auto selected_pixel = pixel_index;
        auto selected_province = glm::vec2((float)map.data[pixel_index * 4] / 256.f, (float)map.data[pixel_index * 4 + 1] / 256.f);
        auto selected_province_id = (int)map.data[pixel_index * 4] + (int)map.data[pixel_index * 4 + 1] * 256;

        auto& def = map.provinces[map.index_to_vector_position[selected_province_id]];

        if (def.owner_tag != control_state.fill_with_tag || def.controller_tag != control_state.fill_with_tag) {
            def.owner_tag = control_state.fill_with_tag;
            def.controller_tag = control_state.fill_with_tag;
            map.province_owner[3 * def.v2id + 0] = def.owner_tag[0];
            map.province_owner[3 * def.v2id + 1] = def.owner_tag[1];
            map.province_owner[3 * def.v2id + 2] = def.owner_tag[2];

            control_state.update_texture_part = true;
        }
    }
}



void paint_line(control& control_state, parsing::game_map& map) {
    auto start = control_state.fill_center;
    auto end = glm::ivec2(control_state.delayed_map_coord);

    auto direction = end - start;
    auto normal = glm::ivec2(direction.y, -direction.x);

    auto shift_x = 1;
    if (direction.x < 0) {
        shift_x = -1;
    }
    auto shift_y = 1;
    if (direction.y < 0) {
        shift_y = -1;
    }

    auto error_change_x = shift_x * normal.x;
    auto error_change_y = shift_y * normal.y;

    auto x = start.x;
    auto y = start.y;

    while (x != end.x || y != end.y) {
        {
            auto rgb = parsing::rgb_to_uint(control_state.r, control_state.g, control_state.b);
            auto index = map.rgb_to_index[rgb];
            auto pixel_index = coord_to_pixel(map, glm::ivec2{x, y});
            paint_safe(control_state, map, pixel_index, index);
            control_state.update_texture_x_bottom = std::min(x, control_state.update_texture_x_bottom);
            control_state.update_texture_y_bottom = std::min(y, control_state.update_texture_y_bottom);
            control_state.update_texture_x_top = std::max(x, control_state.update_texture_x_top);
            control_state.update_texture_y_top = std::max(y, control_state.update_texture_y_top);
        }

        auto error = pairing(normal, {x - start.x, y - start.y});

        if (std::abs(error + error_change_x) < std::abs(error + error_change_y)) {
            x += shift_x;
        } else {
            y += shift_y;
        }
    }
}

void update_mouse_move(control& state, parsing::game_map& map, glm::vec2 new_position) {
    state.mouse_map_coord = new_position;
    state.mouse_map_coord.y = std::clamp(state.mouse_map_coord.y, 0.f, (float)map.size_y - 1.f);
    state.mouse_map_coord.x = std::clamp(state.mouse_map_coord.x, 0.f, (float)map.size_x - 1.f);
    update_hover(state, map);
}

};
