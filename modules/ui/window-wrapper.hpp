#pragma once
#include <SDL.h>
#include <windef.h>
#include <winuser.h>
#include "imgui.h"
#include "../glm/fwd.hpp"

namespace state {
    struct layers_stack;
    struct control;
}

namespace window {
struct wrapper {
    SDL_Window * window;
    int width, height;
    SDL_GLContext gl_context;

    float time;
    bool running;
    bool delayed;

    int mouse_x;
    int mouse_y;

    HCURSOR cursor_default = 0;
    HCURSOR cursor_pick_color = 0;
    HCURSOR cursor_line_fill = 0;


    wrapper();
    ~wrapper();
    void update(state::layers_stack& layers, state::control& control_state, ImGuiIO& io, float & zoom);
};
}