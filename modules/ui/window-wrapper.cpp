#include <windef.h>
#include <winuser.h>
#include <SDL.h>
#include "SDL_events.h"
#include "SDL_keycode.h"
#include "../editor-state/editor-state.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

#include "window-wrapper.hpp"

namespace window {
std::string to_string(std::string_view str) {
    return std::string(str.begin(), str.end());
}

void sdl2_fail(std::string_view message) {
    std::cout << (to_string(message) + SDL_GetError());
    abort();
}

void glew_fail(std::string_view message, GLenum error) {
    std::cout << (to_string(message) + reinterpret_cast<const char *>(glewGetErrorString(error)));
    abort();
}

wrapper::wrapper() {
    std::cout << "init video\n";
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        sdl2_fail("SDL_Init: ");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window = SDL_CreateWindow("Project Alice Map Editor",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_OPENGL
        | SDL_WINDOW_RESIZABLE
        | SDL_WINDOW_MAXIMIZED
    );

    if (!window)
        sdl2_fail("SDL_CreateWindow: ");

    SDL_GetWindowSize(window, &width, &height);

    {
        LPCTSTR image = IDC_CROSS;
        cursor_default = LoadCursor(NULL, image);
        SetCursor(cursor_default);
    }
    {
        LPCTSTR image = IDC_PIN;
        cursor_pick_color = LoadCursor(NULL, image);
    }
    {
        cursor_line_fill = LoadCursorFromFile("./assets/fill_cursor.ico");
    }

    gl_context = SDL_GL_CreateContext(window);

    if (!gl_context)
        sdl2_fail("SDL_GL_CreateContext: ");

    if (auto result = glewInit(); result != GLEW_NO_ERROR)
        glew_fail("glewInit: ", result);

    if (!GLEW_VERSION_3_3)
        throw std::runtime_error("OpenGL 3.3 is not supported");

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    glClearColor(0.1f, 0.1f, 0.2f, 0.f);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init();
}

wrapper::~wrapper() {
    std::cout << "clear up sdl stuff";
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void wrapper::update(state::layers_stack& layers, state::control& control_state, ImGuiIO& io, glm::vec2& camera_shift, float & zoom) {
    for (SDL_Event event; SDL_PollEvent(&event);) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_WINDOWEVENT: switch (event.window.event)
            {
            case SDL_WINDOWEVENT_RESIZED:
                width = event.window.data1;
                height = event.window.data2;
                glViewport(0, 0, width, height);
                break;
            }
            break;
        case SDL_MOUSEMOTION:
            {
                if (io.WantCaptureMouse) break;
                mouse_x = event.motion.x;
                mouse_y = event.motion.y;
                if(layers.data.size() > 0) {
                    state::update_mouse_move(control_state, layers, state::screen_to_texture(
                        mouse_x, mouse_y,
                        layers.get_provinces_image_x(), layers.get_provinces_image_y(),
                        width, height,
                        zoom, camera_shift
                    ));
                }
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                control_state.lmb_pressed = false;
                if (io.WantCaptureMouse) break;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                if (io.WantCaptureMouse) break;
                if(layers.data.size() > 0) {
                    control_state.mouse_map_coord = state::screen_to_texture(
                        event.button.x, event.button.y,
                        layers.get_provinces_image_x(), layers.get_provinces_image_y(),
                        width, height,
                        zoom, camera_shift
                    );

                    control_state.mouse_map_coord.y = std::clamp(control_state.mouse_map_coord.y, 0.f, (float)layers.get_provinces_image_y() - 1.f);
                    control_state.mouse_map_coord.x = std::clamp(control_state.mouse_map_coord.x, 0.f, (float)layers.get_provinces_image_x() - 1.f);

                    control_state.active = !control_state.active;
                    control_state.fill_center = glm::vec2{control_state.mouse_map_coord.x,control_state.mouse_map_coord.y};
                    control_state.delayed_map_coord = glm::ivec2{ control_state.mouse_map_coord };
                    control_state.lmb_pressed = true;
                }
            }
            else if (event.button.button == SDL_BUTTON_RIGHT)
            {
                if (io.WantCaptureMouse) break;

                update_context(control_state, layers);
                control_state.context_window_origin = {event.button.x, event.button.y};
            }
            break;
        case SDL_KEYDOWN:
            if (io.WantCaptureKeyboard) {
                break;
            }

            control_state.context_province = 0;
            if (event.key.keysym.sym == SDLK_a)
            {
                camera_shift.x += 0.1f * zoom;
            }
            else if (event.key.keysym.sym == SDLK_d)
            {
                camera_shift.x -= 0.1f * zoom;
            } else if (event.key.keysym.sym == SDLK_w)
            {
                camera_shift.y -= 0.1f * zoom;
            } else if (event.key.keysym.sym == SDLK_s)
            {
                camera_shift.y += 0.1f * zoom;
            } else if (event.key.keysym.sym == SDLK_q)
            {
                zoom *= 0.5;
            } else if (event.key.keysym.sym == SDLK_e)
            {
                zoom *= 2.0;
            } else if (event.key.keysym.sym == SDLK_z) {
                control_state.mode = state::CONTROL_MODE::PICKING_COLOR;
            } else if (event.key.keysym.sym == SDLK_x) {
                control_state.mode = state::CONTROL_MODE::FILL;
            } else if (event.key.keysym.sym == SDLK_c) {
                control_state.mode = state::CONTROL_MODE::NONE;
            }

            zoom = std::max(zoom, 1.f / 64.f);
            state::update_hover(control_state, layers);

            break;
        }
    }

    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
        delayed = true;
        SDL_Delay(10);
        return;
    } else {
        delayed = false;
    }
}
}
