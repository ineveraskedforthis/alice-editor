#include <optional>
#include <string>
#include <iostream>
#include <algorithm>
#include "editor-state.hpp"

#undef max
#undef min
#undef clamp

namespace state {



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

int pixel(control& control_state, layers_stack& map) {
    return map.screen_to_pixel(control_state.mouse_map_coord);
}

void update_hover(control& control_state, layers_stack& map){
    auto pixel_index = pixel(control_state, map);
    control_state.hovered_province = map.sample_province_index_texture_coord(pixel_index);
}

void update_select(control& control_state, layers_stack& map){
    auto pixel_index = pixel(control_state, map);
    control_state.selected_pixel = pixel_index;
    control_state.selected_province = map.sample_province_index_texture_coord(pixel_index);
    control_state.selected_province_id = map.sample_province_index(pixel_index);
    control_state.selection_delay = true;
}

void update_context(control& control_state, layers_stack& map) {
    if (map.data.size() == 0) return;
    auto pixel_index = pixel(control_state, map);
    control_state.context_province = map.sample_province_index(pixel_index);
}

void pick_color(control& control_state, layers_stack& map) {
    auto pixel_index = pixel(control_state, map);
    auto color = map.sample_province_color(pixel_index);

    control_state.r = color.r;
    control_state.g = color.g;
    control_state.b = color.b;

    auto selected_province_id = map.sample_province_index(pixel_index);
    auto def = map.get_province_history(selected_province_id);

    if (def != nullptr) {
        control_state.fill_with_tag = def->owner_tag;
    }
}

void paint(control& control_state, layers_stack& map) {
    auto pixel_index = pixel(control_state, map);
    map.set_pixel(pixel_index, control_state.r, control_state.g, control_state.b);
}

void paint_state(control& control_state, layers_stack& map) {
    auto selected_prov = map.sample_province_index(control_state.selected_pixel);
    auto current_prov = map.sample_province_index(pixel(control_state, map));
    map.copy_state_from_province_to_province(selected_prov, current_prov);
}

void paint_safe(control& control_state, layers_stack& map, int pixel_index, uint32_t province_index) {
    if (control_state.fill_mode == FILL_MODE::PROVINCE) {
        auto color = map.sample_province_color(pixel_index);
        auto rgb_target = state::rgb_to_uint(color.r, color.g, color.b);
        auto index_target = map.rgb_to_index(color.r, color.g, color.b);
        if (index_target == std::nullopt) return;
        auto source_is_sea = map.sample_province_is_sea(province_index);
        auto target_is_sea = map.sample_province_is_sea(index_target.value());
        if (source_is_sea != target_is_sea) return;
        map.set_pixel(pixel_index, control_state.r, control_state.g, control_state.b);
    } else if (control_state.fill_mode == FILL_MODE::OWNER_AND_CONTROLLER) {
        auto color = map.sample_province_color(pixel_index);
        auto rgb_target = state::rgb_to_uint(color.r, color.g, color.b);
        auto index_target_optional = map.rgb_to_index(color.r, color.g, color.b);
        if (index_target_optional == std::nullopt) return;
        auto index_target = index_target_optional.value();
        auto target_is_sea = map.sample_province_is_sea(index_target);
        if (target_is_sea) return;
        auto selected_pixel = pixel_index;

        // if user attempts to paint province ownership,
        // he gives permission to copy history data to current layer

        if (!map.can_edit_province_history(index_target)) {
            map.copy_province_history_to_current_layer(index_target);
        }

        auto target = map.get_province_history(index_target);
        if (target == nullptr) return;

        if (target->owner_tag != control_state.fill_with_tag || target->controller_tag != control_state.fill_with_tag) {
            target->owner_tag = control_state.fill_with_tag;
            target->controller_tag = control_state.fill_with_tag;

            map.set_owner(index_target, target->owner_tag);
            map.commit_owner_texture_to_gpu();
        }
    }
}



void paint_line(control& control_state, layers_stack& map) {
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
            auto index = map.rgb_to_index(control_state.r, control_state.g, control_state.b);
            if (index == std::nullopt) continue;

            auto pixel_index = map.coord_to_pixel(glm::ivec2{x, y});
            paint_safe(control_state, map, pixel_index, index.value());
        }

        auto error = pairing(normal, {x - start.x, y - start.y});

        if (std::abs(error + error_change_x) < std::abs(error + error_change_y)) {
            x += shift_x;
        } else {
            y += shift_y;
        }
    }
}

void update_mouse_move(control& state, layers_stack& map, glm::vec2 new_position) {
    state.mouse_map_coord = new_position;
    state.mouse_map_coord.y = std::clamp(state.mouse_map_coord.y, 0.f, (float)map.get_provinces_image_x() - 1.f);
    state.mouse_map_coord.x = std::clamp(state.mouse_map_coord.x, 0.f, (float)map.get_provinces_image_x() - 1.f);
    update_hover(state, map);
}

};
