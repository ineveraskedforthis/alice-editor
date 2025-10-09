#include <optional>
#include <string>
#include <algorithm>
#include "editor-state.hpp"

#undef max
#undef min
#undef clamp

namespace state {



int inline pairing(glm::ivec2 a, glm::ivec2 b) {
    return a.x * b.x + a.y * b.y;
}

int pixel(control& control_state, layers_stack& map) {
    return map.screen_to_pixel(control_state.mouse_map_coord);
}

void update_hover(control& control_state, layers_stack& map){
    auto pixel_index = pixel(control_state, map);
    control_state.hovered_province = map.sample_province_index_texture_coord(pixel_index);
}

void select_pixel(control& control_state, layers_stack& map, int pixel) {
    control_state.selected_pixel = pixel;
    // control_state.selected_province = map.sample_province_index_texture_coord(pixel_index);
    auto selected_province_id = map.sample_province_index(pixel);
    control_state.selected_province_id = map.sample_province_index(pixel);
    auto def = map.get_province_history(selected_province_id);
    if (def != nullptr) {
        auto owner = def->owner_tag;
        control_state.selected_tag = owner;
    }
    state::pick_color(control_state, map);
    control_state.selection_delay = true;
}

void update_select(control& control_state, layers_stack& map){
    auto pixel_index = pixel(control_state, map);
    select_pixel(control_state, map, pixel_index);
}

void update_context(control& control_state, layers_stack& map) {
    if (map.data.size() == 0) return;
    auto pixel_index = pixel(control_state, map);
    control_state.context_province = map.sample_province_index(pixel_index);
    control_state.context_pixel = pixel_index;
    auto def = map.sample_province_definition(pixel_index);
    if (def == nullptr) {
        control_state.context_tag = "";
    } else {
        auto history = map.get_province_history(def->v2id);
        if (history == nullptr) {
            control_state.context_tag = "";
        } else {
            control_state.context_tag = history->owner_tag;
        }
    }
}

void pick_color_from_pixel(control& control_state, layers_stack& map, int pixel) {
    auto color = map.sample_province_color(pixel);
    control_state.r = color.r;
    control_state.g = color.g;
    control_state.b = color.b;

    auto selected_province_id = map.sample_province_index(pixel);
    auto def = map.get_province_history(selected_province_id);

    if (def != nullptr) {
        control_state.selected_tag = def->owner_tag;
    }
}

void pick_color(control& control_state, layers_stack& map) {
    auto pixel_index = pixel(control_state, map);
    pick_color_from_pixel(control_state, map, pixel_index);
}

void paint(control& control_state, layers_stack& map) {
    auto pixel_index = pixel(control_state, map);
    map.set_pixel(pixel_index, control_state.r, control_state.g, control_state.b);
}

void paint_state(control& control_state, layers_stack& map, uint32_t target_pixel, uint32_t source_pixel) {
    auto target_prov = map.sample_province_index(target_pixel);
    auto source_prov = map.sample_province_index(source_pixel);
    map.copy_state_from_province_to_province(source_prov, target_prov);
}

void paint_province_safe(control& control_state, layers_stack& map, int pixel_index, uint32_t province_index) {
    auto color = map.sample_province_color(pixel_index);
    auto rgb_target = datatypes::rgb_to_uint(color.r, color.g, color.b);
    auto index_target = map.rgb_to_v2id(color.r, color.g, color.b);
    if (index_target == std::nullopt) return;
    auto source_is_sea = map.sample_province_is_sea(province_index);
    auto target_is_sea = map.sample_province_is_sea(index_target.value());
    if (source_is_sea != target_is_sea) return;
    map.set_pixel(pixel_index, control_state.r, control_state.g, control_state.b);
}

void paint_province_unsafe(control& control_state, layers_stack& map, int pixel_index) {
    map.set_pixel(pixel_index, control_state.r, control_state.g, control_state.b);
}

void paint_controler_and_owner_safe(control& control_state, layers_stack& map, int pixel_index) {
    auto color = map.sample_province_color(pixel_index);
    auto rgb_target = datatypes::rgb_to_uint(color.r, color.g, color.b);
    auto index_target_optional = map.rgb_to_v2id(color.r, color.g, color.b);
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

    if (target->owner_tag != control_state.selected_tag || target->controller_tag != control_state.selected_tag) {
        target->owner_tag = control_state.selected_tag;
        target->controller_tag = control_state.selected_tag;

        map.set_owner(index_target, target->owner_tag);
        map.commit_province_colors_texture_to_gpu();
    }
}

void paint_safe(control& control_state, layers_stack& map, int pixel_index, uint32_t province_index) {
    if (control_state.fill_mode == FILL_MODE::PROVINCE) {
        paint_province_safe(control_state, map, pixel_index, province_index);
    } else if (control_state.fill_mode == FILL_MODE::OWNER_AND_CONTROLLER) {
        paint_controler_and_owner_safe(control_state, map, pixel_index);
    }
}



void paint_line(control& control_state, layers_stack& map, bool respect_coasts) {
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

    auto max_steps = 100000;
    auto step = 0;

    while ((x != end.x || y != end.y) && step < (max_steps)) {
        step++;

        {
            auto pixel_index = map.coord_to_pixel(glm::ivec2{x, y});
            if (respect_coasts) {
                auto index = map.rgb_to_v2id(control_state.r, control_state.g, control_state.b);
                if (index == std::nullopt) continue;
                paint_safe(control_state, map, pixel_index, index.value());
            } else {
                paint_province_unsafe(control_state, map, pixel_index);
            }
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
    state.mouse_map_coord.y = std::clamp(state.mouse_map_coord.y, 0.f, (float)map.get_provinces_image_y() - 1.f);
    state.mouse_map_coord.x = std::clamp(state.mouse_map_coord.x, 0.f, (float)map.get_provinces_image_x() - 1.f);
    update_hover(state, map);
}

};
