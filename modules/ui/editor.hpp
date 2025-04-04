#pragma once
#include "imgui.h"
#include <array>
#include "../glm/fwd.hpp"
#include "GL/glew.h"

namespace window {
    struct wrapper;
};

namespace state {
    struct control;
    struct layers_stack;
    struct editor;
};

namespace assets {
    struct storage;
};

namespace widgets {
    void brushes(state::control& control);
    void settings(state::control& control);
    void status(state::control& control, ImGuiIO& io);
    void map_tooltip(state::control& control);
    void main_scene(
        window::wrapper& window,
        ImGuiIO& io,
        state::layers_stack& layers,
        state::control& control,
        assets::storage& storage,
        state::editor& editor,
        GLuint centers_buffer,
        GLuint rivers_VAO,
        GLuint adj_VAO,
        std::array<GLuint, 256>& uniform_locations,
        int& adj_vertices_count,
        float& update_texture_timer,
        float frame_time,
        glm::vec2 camera,
        float zoom,
        glm::vec3 shift
    );
}