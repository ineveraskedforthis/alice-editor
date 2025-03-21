#pragma once

#include <string>

#include "../glm/fwd.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "../glm/ext/matrix_transform.hpp"

#include "GL/glew.h"
#include "content-state.hpp"

namespace state {
void check_gl_error(std::string message);


enum class CONTROL_MODE {
    NONE, SELECT, PICKING_COLOR, PAINTING, FILL, SET_STATE
};

enum class SELECTION_MODE {
    PROVINCE, NATION
};

enum class FILL_MODE {
    PROVINCE, OWNER_AND_CONTROLLER
};

std::string selection_mode_string(SELECTION_MODE MODE);
std::string fill_mode_string(FILL_MODE MODE);

struct control {
    uint32_t selected_pixel;
    glm::vec2 selected_province;
    uint32_t selected_province_id;
    std::string fill_with_tag;
    bool selection_delay;
    glm::vec2 hovered_province;
    glm::vec2 mouse_map_coord;
    glm::ivec2 delayed_map_coord;
    bool reset_focus;
    int selected_adjacency;
    glm::ivec2 fill_center;
    uint32_t context_province;
    glm::ivec2 pixel_context;
    CONTROL_MODE mode;
    SELECTION_MODE selection_mode;
    FILL_MODE fill_mode;
    bool active;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    std::vector<float> rivers_mesh = {};
};

int pixel(control& control_state, layers_stack& map);
void update_hover(control& control_state, layers_stack& map);
void update_select(control& control_state, layers_stack& map);
void update_context(control& control_state, layers_stack& map);
void pick_color(control& control_state, layers_stack& map);
void paint(control& control_state, layers_stack& map);
void paint_state(control& control_state, layers_stack& map);
void paint_safe(control& control_state, layers_stack& map, int pixel_index, uint32_t province_index);
void paint_line(control& control_state, layers_stack& map);

void update_mouse_move(control& state, layers_stack& map, glm::vec2 new_position);
};