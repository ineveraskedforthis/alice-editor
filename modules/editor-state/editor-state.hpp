#pragma once
#include <vector>
#undef max
#undef min
#undef clamp
#include <string>

#include "../glm/fwd.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "../glm/ext/matrix_transform.hpp"

#include "GL/glew.h"
#include "content-state.hpp"
#include "editor-enums.hpp"

namespace state {

void check_gl_error(std::string message);


enum class CONTROL_MODE {
    NONE, SELECT, PICKING_COLOR, PAINTING, FILL, FILL_UNSAFE
};


enum class FILL_MODE {
    PROVINCE, OWNER_AND_CONTROLLER
};

std::string fill_mode_string(FILL_MODE MODE);


struct control {
    FLAG_EXPORT_OPTIONS flags_export = FLAG_EXPORT_OPTIONS::TGA;
    uint32_t selected_pixel = 0;
    // glm::vec2 selected_province;
    uint32_t selected_province_id = 0;
    std::string selected_commodity {};
    std::string selected_tag {};
    bool selection_delay = false;
    glm::vec2 hovered_province {};
    glm::vec2 mouse_map_coord {};
    glm::ivec2 delayed_map_coord {};
    bool reset_focus = true;
    int selected_adjacency = 0;
    bool lmb_pressed = false;
    glm::ivec2 fill_center {};
    int32_t context_province = -1;
    uint32_t context_pixel = 0;
    std::string context_tag {};
    glm::vec2 context_window_origin {};
    CONTROL_MODE mode = CONTROL_MODE::SELECT;
    FILL_MODE fill_mode = FILL_MODE::PROVINCE;
    bool active = false;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    glm::vec2 local_adjacency_center{};
    std::vector<glm::vec2> local_adjacency{};

    std::vector<float> rivers_mesh{};
    std::string new_nation_tag{};
    std::string new_nation_filename{};
};


struct editor {
    uint8_t* rivers_raw = nullptr;
    ankerl::unordered_dense::map<std::string, ankerl::unordered_dense::map<std::string, int>> secondary_rgo_templates {};
    GLuint fill_tool_VertexArray = 0;
    GLuint fill_tool_ArrayBuffer = 0;
    GLuint map_fake_VAO = 0;
    GLuint map_program;
    GLuint triangle_program;
    GLuint line_program;
    GLuint rivers_program;
};

int pixel(control& control_state, layers_stack& map);
void update_hover(control& control_state, layers_stack& map);
void update_select(control& control_state, layers_stack& map);
void update_context(control& control_state, layers_stack& map);
void pick_color(control& control_state, layers_stack& map);
void paint(control& control_state, layers_stack& map);
void paint_state(control& control_state, layers_stack& map, uint32_t target_pixel, uint32_t source_pixel);
void paint_controler_and_owner_safe(control& control_state, layers_stack& map, int pixel_index);
void paint_safe(control& control_state, layers_stack& map, int pixel_index, uint32_t province_index);
void paint_line(control& control_state, layers_stack& map, bool respect_coasts);

void select_pixel(control& control_state, layers_stack& map, int pixel);
void pick_color_from_pixel(control& control_state, layers_stack& map, int pixel);

void update_mouse_move(control& state, layers_stack& map, glm::vec2 new_position);
};