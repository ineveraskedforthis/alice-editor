#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <chrono>

#include <SDL.h>
#include <limits>
#include <string>
#include <thread>
#include <vector>

#include "GL/glew.h"

#include "SDL_events.h"
#include "SDL_keycode.h"


#include "modules/glm/fwd.hpp"
#include "modules/parsing/definitions.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "modules/glm/ext/matrix_transform.hpp"


#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

#include "modules/colormaps/viridis.hpp"
#include "modules/read_shader.hpp"
#include "modules/parsing/map.hpp"
#include "modules/widgets/selection_widget.hpp"
#include "modules/editor-state/editor-state.hpp"

#include "modules/assets-manager/assets.hpp"

#undef max
#undef min
#undef clamp

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

GLuint create_shader(GLenum type, const char *source) {
    GLuint result = glCreateShader(type);
    glShaderSource(result, 1, &source, nullptr);
    glCompileShader(result);
    GLint status;
    glGetShaderiv(result, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        GLint info_log_length;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &info_log_length);
        std::string info_log(info_log_length, '\0');
        glGetShaderInfoLog(result, info_log.size(), nullptr, info_log.data());
        std::cout << "Shader compilation failed: " + info_log;
        abort();
    }
    return result;
}

GLuint create_program(GLuint vertex_shader, GLuint fragment_shader) {
    GLuint result = glCreateProgram();
    glAttachShader(result, vertex_shader);
    glAttachShader(result, fragment_shader);
    glLinkProgram(result);

    GLint status;
    glGetProgramiv(result, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        GLint info_log_length;
        glGetProgramiv(result, GL_INFO_LOG_LENGTH, &info_log_length);
        std::string info_log(info_log_length, '\0');
        glGetProgramInfoLog(result, info_log.size(), nullptr, info_log.data());
        std::cout << "Program linkage failed: " + info_log;
        abort();
    }

    return result;
}

glm::vec2 screen_to_texture(
    int x_in,
    int y_in,
    float width_texture,
    float height_texture,
    float width_screen,
    float height_screen,
    float zoom,
    glm::vec2 camera_shift
) {
    float x_adjusted = ((float)x_in - (float)width_screen / 2.f);
    float y_adjusted = ((float)y_in - (float)height_screen / 2.f);
    glm::vec2 shift_true = camera_shift * glm::vec2((float)width_screen, (float)width_screen) / 2.f;
    float x = (x_adjusted) * zoom + (float)width_texture / 2.f - shift_true.x;
    float y = height_texture - ((y_adjusted) * zoom + (float)height_texture / 2.f) - shift_true.y ;

    return glm::vec2(x, y);
}

void update_adj_buffers(parsers::game_map& map_state, GLuint buffer, int& counter) {
    std::vector<float> vertices_for_adj;

    for (auto& adj : map_state.adjacencies ) {

        if (adj.mark_for_delete) {
            continue;
        }

        auto& prov_start = map_state.provinces[map_state.index_to_vector_position[adj.from]];
        auto& prov_through = map_state.provinces[map_state.index_to_vector_position[adj.through]];
        auto& prov_to = map_state.provinces[map_state.index_to_vector_position[adj.to]];


        if (adj.through != 0) {
            // triangle 1
            // vertex 1
            vertices_for_adj.push_back(prov_start.pos_x / (float)map_state.size_x * 2.f - 1.f - 0.0001);
            vertices_for_adj.push_back(prov_start.pos_y / (float)map_state.size_y * 2.f - 1.f - 0.0001);

            // vertex 2:
            vertices_for_adj.push_back(prov_start.pos_x / (float)map_state.size_x * 2.f - 1.f + 0.0001);
            vertices_for_adj.push_back(prov_start.pos_y / (float)map_state.size_y * 2.f - 1.f + 0.0001);

            // vertex 3:
            vertices_for_adj.push_back(prov_through.pos_x / (float)map_state.size_x * 2.f - 1.f);
            vertices_for_adj.push_back(prov_through.pos_y / (float)map_state.size_y * 2.f - 1.f);


            // triangle 2
            // vertex 1
            vertices_for_adj.push_back(prov_through.pos_x / (float)map_state.size_x * 2.f - 1.f - 0.0001);
            vertices_for_adj.push_back(prov_through.pos_y / (float)map_state.size_y * 2.f - 1.f - 0.0001);

            // vertex 2:
            vertices_for_adj.push_back(prov_through.pos_x / (float)map_state.size_x * 2.f - 1.f + 0.0001);
            vertices_for_adj.push_back(prov_through.pos_y / (float)map_state.size_y * 2.f - 1.f + 0.0001);

            // vertex 3:
            vertices_for_adj.push_back(prov_to.pos_x / (float)map_state.size_x * 2.f - 1.f);
            vertices_for_adj.push_back(prov_to.pos_y / (float)map_state.size_y * 2.f - 1.f);

            // triangle 3
            // vertex 1
            vertices_for_adj.push_back(prov_start.pos_x / (float)map_state.size_x * 2.f - 1.f - 0.0001);
            vertices_for_adj.push_back(prov_start.pos_y / (float)map_state.size_y * 2.f - 1.f + 0.0001);

            // vertex 2:
            vertices_for_adj.push_back(prov_start.pos_x / (float)map_state.size_x * 2.f - 1.f + 0.0001);
            vertices_for_adj.push_back(prov_start.pos_y / (float)map_state.size_y * 2.f - 1.f - 0.0001);

            // vertex 3:
            vertices_for_adj.push_back(prov_through.pos_x / (float)map_state.size_x * 2.f - 1.f);
            vertices_for_adj.push_back(prov_through.pos_y / (float)map_state.size_y * 2.f - 1.f);


            // triangle 4
            // vertex 1
            vertices_for_adj.push_back(prov_through.pos_x / (float)map_state.size_x * 2.f - 1.f - 0.0001);
            vertices_for_adj.push_back(prov_through.pos_y / (float)map_state.size_y * 2.f - 1.f + 0.0001);

            // vertex 2:
            vertices_for_adj.push_back(prov_through.pos_x / (float)map_state.size_x * 2.f - 1.f + 0.0001);
            vertices_for_adj.push_back(prov_through.pos_y / (float)map_state.size_y * 2.f - 1.f - 0.0001);

            // vertex 3:
            vertices_for_adj.push_back(prov_to.pos_x / (float)map_state.size_x * 2.f - 1.f);
            vertices_for_adj.push_back(prov_to.pos_y / (float)map_state.size_y * 2.f - 1.f);
        } else {
            if (adj.to != 0) {
                // triangle 1
                // vertex 1
                vertices_for_adj.push_back(prov_start.pos_x / (float)map_state.size_x * 2.f - 1.f - 0.0001);
                vertices_for_adj.push_back(prov_start.pos_y / (float)map_state.size_y * 2.f - 1.f - 0.0001);

                // vertex 2:
                vertices_for_adj.push_back(prov_start.pos_x / (float)map_state.size_x * 2.f - 1.f + 0.0001);
                vertices_for_adj.push_back(prov_start.pos_y / (float)map_state.size_y * 2.f - 1.f + 0.0001);

                // vertex 3:
                vertices_for_adj.push_back(prov_to.pos_x / (float)map_state.size_x * 2.f - 1.f);
                vertices_for_adj.push_back(prov_to.pos_y / (float)map_state.size_y * 2.f - 1.f);

                // triangle 2
                // vertex 1
                vertices_for_adj.push_back(prov_start.pos_x / (float)map_state.size_x * 2.f - 1.f - 0.0001);
                vertices_for_adj.push_back(prov_start.pos_y / (float)map_state.size_y * 2.f - 1.f + 0.0001);

                // vertex 2:
                vertices_for_adj.push_back(prov_start.pos_x / (float)map_state.size_x * 2.f - 1.f + 0.0001);
                vertices_for_adj.push_back(prov_start.pos_y / (float)map_state.size_y * 2.f - 1.f - 0.0001);

                // vertex 3:
                vertices_for_adj.push_back(prov_to.pos_x / (float)map_state.size_x * 2.f - 1.f);
                vertices_for_adj.push_back(prov_to.pos_y / (float)map_state.size_y * 2.f - 1.f);
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

struct window_wrapper {
    SDL_Window * window;
    int width, height;
    SDL_GLContext gl_context;

    float time;
    bool running;
    bool delayed;

    int mouse_x;
    int mouse_y;

    window_wrapper() {
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

    ~window_wrapper() {
        std::cout << "clear up sdl stuff";
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void update(parsers::game_map& map, state::control& control_state, ImGuiIO& io, glm::vec2& camera_shift, float & zoom) {
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
                    state::update_mouse_move(control_state, map, screen_to_texture(
                        mouse_x, mouse_y,
                        map.size_x, map.size_y,
                        width, height,
                        zoom, camera_shift
                    ));
                }
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (io.WantCaptureMouse) break;
                    control_state.mouse_map_coord = screen_to_texture(
                        event.button.x, event.button.y,
                        map.size_x, map.size_y,
                        width, height,
                        zoom, camera_shift
                    );

                    control_state.mouse_map_coord.y = std::clamp(control_state.mouse_map_coord.y, 0.f, (float)map.size_y - 1.f);
                    control_state.mouse_map_coord.x = std::clamp(control_state.mouse_map_coord.x, 0.f, (float)map.size_x - 1.f);

                    control_state.active = !control_state.active;
                    control_state.fill_center = glm::vec2{control_state.mouse_map_coord.x,control_state.mouse_map_coord.y};
                    control_state.delayed_map_coord = glm::ivec2{ control_state.mouse_map_coord };
                }
                else if (event.button.button == SDL_BUTTON_RIGHT)
                {
                    if (io.WantCaptureMouse) break;

                    update_context(control_state, map);

                    control_state.pixel_context = {event.button.x, event.button.y};
                    // delete_last_vertex(line);
                    // delete_control_point(curve);
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
                    // control_state.mode = CONTROL_MODE::PAINTING;
                } else if (event.key.keysym.sym == SDLK_c) {
                    control_state.mode = state::CONTROL_MODE::NONE;
                } else if (event.key.keysym.sym == SDLK_n) {
                    auto prov = map.new_province(pixel(control_state, map));
                    control_state.r = prov.r;
                    control_state.g = prov.g;
                    control_state.b = prov.b;
                    glBindTexture(GL_TEXTURE_2D, control_state.main_texture);
                    update_map_texture(control_state, map);
                } else if (event.key.keysym.sym == SDLK_f) {
                    control_state.fill_center = glm::vec2{control_state.mouse_map_coord.x, control_state.mouse_map_coord.y};
                    control_state.mode = state::CONTROL_MODE::FILL;
                    control_state.delayed_map_coord = glm::ivec2{control_state.mouse_map_coord};
                } else if (event.key.keysym.sym == SDLK_p) {
                    parsers::unload_data(map, "./editor-output");
                } else if (event.key.keysym.sym == SDLK_r) {
                    if (control_state.selected_pixel) {
                        auto prov_x = map.data[4 * control_state.selected_pixel];
                        auto prov_y = map.data[4 * control_state.selected_pixel + 1];
                        auto prov = (int)prov_x + (int)(prov_y) * 256;
                        auto state_r = map.province_state[prov * 2];
                        auto state_g = map.province_state[prov * 2 + 1];

                        if (state_r || state_g) {
                            control_state.mode = state::CONTROL_MODE::SET_STATE;
                        }
                    }
                }


                zoom = std::max(zoom, 1.f / 64.f);
                update_hover(control_state, map);

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
};

namespace SHADER_UNIFORMS {
    enum : size_t {
        PROVINCES_RGB = 0,
        PROVINCES_DATA = 1,
        MODEL = 2,
        VIEW = 3,
        PROJECTION = 4,
        ZOOM = 5,
        PIXEL_X = 6,
        PIXEL_Y = 7,
        SELECTED_PROVINCE = 8,
        HOVERED_PROVINCE = 9,
        STATES_DATA = 10,
        OWNER_DATA,
        SIZE,
        RIVERS,
        MODEL_LINE, VIEW_LINE,
        MODEL_RIVER, VIEW_RIVER
    };
}

std::array<GLuint, 256> TO_LOCATION {};

void update_rivers_mesh(state::control& state, parsers::game_map& map) {
    auto count = 0;
    for (int i = 0; i < map.size_x; i++) {
        for (int j = 0; j < map.size_y; j++) {
            auto pixel = state::coord_to_pixel(map, glm::ivec2{i, j});
            if (map.rivers_raw[pixel * 4] < 255) {
                auto width = (40.f + 255.f - float(map.rivers_raw[pixel * 4])) / 500.f;
                if (i + 1 < map.size_x){
                    auto next = state::coord_to_pixel(map, glm::ivec2{i + 1, j});
                    if (map.rivers_raw[next * 4] < 255) {
                        //std::cout << i << " " << j << " " << next << " " << pixel << " i + 1" << std::endl;
                        auto width_next = (40.f + 255.f - float(map.rivers_raw[next * 4])) / 500.f;
                        count++;
                        // triangle 1
                        state.rivers_mesh.push_back(i / (float)map.size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j - width) / (float)map.size_y * 2.f - 1.f);

                        state.rivers_mesh.push_back(i / (float)map.size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j + width) / (float)map.size_y * 2.f - 1.f);

                        state.rivers_mesh.push_back((i + 1) / (float)map.size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j + width_next) / (float)map.size_y * 2.f - 1.f);

                        // triangle 2
                        state.rivers_mesh.push_back(i / (float)map.size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j - width) / (float)map.size_y * 2.f - 1.f);

                        state.rivers_mesh.push_back((i + 1) / (float)map.size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j + width_next) / (float)map.size_y * 2.f - 1.f);

                        state.rivers_mesh.push_back((i + 1) / (float)map.size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j - width_next) / (float)map.size_y * 2.f - 1.f);
                    }
                }

                if (j + 1 < map.size_y){
                    auto next = state::coord_to_pixel(map, glm::ivec2{i, j + 1});
                    if (map.rivers_raw[next * 4] < 255) {
                        //std::cout << i << " " << j << " " << next << " " << pixel << " " << " j + 1" << std::endl;
                        auto width_next = (40.f + 255.f - float(map.rivers_raw[next * 4])) / 500.f;
                        count++;
                        // triangle 1
                        state.rivers_mesh.push_back((i - width) / (float)map.size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j) / (float)map.size_y * 2.f - 1.f);

                        state.rivers_mesh.push_back((i + width) / (float)map.size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j) / (float)map.size_y * 2.f - 1.f);

                        state.rivers_mesh.push_back((i + width_next) / (float)map.size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j + 1) / (float)map.size_y * 2.f - 1.f);

                        // triangle 2
                        state.rivers_mesh.push_back((i - width) / (float)map.size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back(j / (float)map.size_y * 2.f - 1.f );

                        state.rivers_mesh.push_back((i + width_next) / (float)map.size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j + 1) / (float)map.size_y * 2.f - 1.f );

                        state.rivers_mesh.push_back((i - width_next) / (float)map.size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j + 1) / (float)map.size_y * 2.f - 1.f);
                    }
                }
            }
        }
    }

    std::cout << "Edges: " << count << "\n";
    std::cout << "state.rivers_mesh: " << state.rivers_mesh.size() << "\n";
}


int main(int argc, char* argv[]) {
    std::cout << "Welcome\n";

    {
        window_wrapper window {};
        state::control control_state {};
        assets::storage storage {};

        float data_buffer[2000];
        uint8_t province_data[256 * 256 * 3];

        std::cout << "loading map\n";
        auto map_state = parsers::load_map("./editor-input");
        std::cout << "loading data\n";
        parsers::load(map_state, "./editor-input");

        for (auto i = 0; i <= 256 * 256 - 1; i++) {
            auto id = uint8_t(i % 256);
            province_data[i * 3 + 0] = colormaps::viridis[id][0] * 255;
            province_data[i * 3 + 1] = colormaps::viridis[id][1] * 255;
            province_data[i * 3 + 2] = colormaps::viridis[id][2] * 255;
        }

        std::cout << "loading shaders2\n";
        auto line_vertex_shader = create_shader(
            GL_VERTEX_SHADER,
            read_shader("./shaders/line.vs").c_str()
        );
        auto line_fragment_shader = create_shader(
            GL_FRAGMENT_SHADER,
            read_shader("./shaders/line.fs").c_str()
        );
        auto line_program = create_program(line_vertex_shader, line_fragment_shader);
        auto rivers_program = create_program(line_vertex_shader, line_fragment_shader);

        TO_LOCATION[SHADER_UNIFORMS::MODEL_RIVER] = glGetUniformLocation(rivers_program, "model");
        TO_LOCATION[SHADER_UNIFORMS::VIEW_RIVER] = glGetUniformLocation(rivers_program, "view");

        state::check_gl_error("Before loading textures");

        load_map_texture(control_state, map_state);

        update_rivers_mesh(control_state, map_state);

        std::cout << "Rivers mesh created. Amount of vertices: " << control_state.rivers_mesh.size();

        GLuint rivers_array;
        glGenVertexArrays(1, &rivers_array);
        GLuint rivers_buffer;
        glGenBuffers(1, &rivers_buffer);
        glBindVertexArray(rivers_array);
        glBindBuffer(GL_ARRAY_BUFFER, rivers_buffer);
        glVertexAttribPointer(
            0,
            2,
            GL_FLOAT,
            GL_FALSE,
            2 * sizeof(float),
            (void*)0
        );
        glEnableVertexAttribArray(0);
        glBindAttribLocation(rivers_program, 0, "vertex");

        glBindBuffer(GL_ARRAY_BUFFER, rivers_buffer);

        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof(float) * control_state.rivers_mesh.size(),
            control_state.rivers_mesh.data(),
            GL_STATIC_DRAW
        );

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        state::check_gl_error("rivers buffers");

        // loading textures for icons:



        state::check_gl_error("Before shaders");

        std::cout << "loading shaders\n";
        auto vertex_shader = create_shader(
            GL_VERTEX_SHADER,
            read_shader("./shaders/.vs").c_str()
        );
        auto fragment_shader = create_shader(
            GL_FRAGMENT_SHADER,
            read_shader("./shaders/.fs").c_str()
        );
        auto program = create_program(vertex_shader, fragment_shader);

        state::check_gl_error("Shaders");

        TO_LOCATION[SHADER_UNIFORMS::PROVINCES_RGB] = glGetUniformLocation(program, "province_rgb");
        TO_LOCATION[SHADER_UNIFORMS::PROVINCES_DATA] = glGetUniformLocation(program, "data");
        TO_LOCATION[SHADER_UNIFORMS::STATES_DATA] = glGetUniformLocation(program, "state_data");
        TO_LOCATION[SHADER_UNIFORMS::OWNER_DATA] = glGetUniformLocation(program, "owner_data");
        TO_LOCATION[SHADER_UNIFORMS::RIVERS] = glGetUniformLocation(program, "rivers");

        TO_LOCATION[SHADER_UNIFORMS::MODEL] = glGetUniformLocation(program, "model");
        TO_LOCATION[SHADER_UNIFORMS::VIEW] = glGetUniformLocation(program, "view");
        TO_LOCATION[SHADER_UNIFORMS::PROJECTION] = glGetUniformLocation(program, "projection");
        TO_LOCATION[SHADER_UNIFORMS::ZOOM] = glGetUniformLocation(program, "zoom");
        TO_LOCATION[SHADER_UNIFORMS::SIZE] = glGetUniformLocation(program, "size");

        TO_LOCATION[SHADER_UNIFORMS::PIXEL_X] = glGetUniformLocation(program, "pixel_x");
        TO_LOCATION[SHADER_UNIFORMS::PIXEL_Y] = glGetUniformLocation(program, "pixel_y");

        TO_LOCATION[SHADER_UNIFORMS::SELECTED_PROVINCE] = glGetUniformLocation(program, "selected_province");
        TO_LOCATION[SHADER_UNIFORMS::HOVERED_PROVINCE] = glGetUniformLocation(program, "hovered_province");

        std::cout << "prepare to run\n";
        float time = 0.f;
        auto last_frame_start = std::chrono::high_resolution_clock::now();

        GLuint fake_VAO;
        glGenVertexArrays(1, &fake_VAO);
        state::check_gl_error("Vertex array");

        glm::vec3 shift {0, 0, 0.5};
        glm::vec2 camera {0, 0};
        float zoom = 1.f;
        glm::vec3 scale {1.f, (float)map_state.size_y / (float)map_state.size_x, 1.f};

        std::cout << " " << map_state.size_x << " " << map_state.size_y <<"\n";

        std::cout << scale.x << " " << scale.y << " " << scale.z << "\n";

        float updates_delay = 1.f / 60.f;
        float update_texture_timer = 0.f;

        float framerate = 60.f;
        float frame_time = 1.f / framerate;
        float update_timer = 0.f;

        std::cout << "Prepare data";



        TO_LOCATION[SHADER_UNIFORMS::MODEL_LINE] = glGetUniformLocation(line_program, "model");
        TO_LOCATION[SHADER_UNIFORMS::VIEW_LINE] = glGetUniformLocation(line_program, "view");

        state::check_gl_error("line shaders");


        // prepare centers buffers:
        GLuint center_vertex_array;
        glGenVertexArrays(1, &center_vertex_array);
        GLuint centers_buffer;
        glGenBuffers(1, &centers_buffer);
        glBindVertexArray(center_vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, centers_buffer);
        glVertexAttribPointer(
            0,
            2,
            GL_FLOAT,
            GL_FALSE,
            2 * sizeof(float),
            (void*)0
        );
        glEnableVertexAttribArray(0);
        glBindAttribLocation(line_program, 0, "vertex");

        int adj_vertices_count = 0;
        update_adj_buffers(map_state, centers_buffer, adj_vertices_count);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);


        state::check_gl_error("line buffers");

        window.running = true;

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        control_state.reset_focus = true;

        std::cout << "Start the main loop";

        while (window.running) {
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame_start).count();
            last_frame_start = now;

            time += dt;
            update_timer += dt;
            update_texture_timer += dt;

            if (update_timer < frame_time) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            } else {
                update_timer -= frame_time;
            }

            window.update(map_state, control_state, io, camera, zoom);
            if (!window.running)
                break;
            if (window.delayed) {
                continue;
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            int status_bar_height = 25;

            {
                ImGui::SetNextWindowSize(ImVec2(50, window.height - status_bar_height));
                ImGui::SetNextWindowPos(ImVec2(0, 0));

                ImGui::Begin(
                    "Brushes",
                    NULL,
                    ImGuiWindowFlags_NoTitleBar
                    | ImGuiWindowFlags_NoResize
                    | ImGuiWindowFlags_NoScrollbar
                    | ImGuiWindowFlags_NoFocusOnAppearing
                );


                if (ImGui::Button("S", ImVec2(35, 35))) {
                    control_state.mode = state::CONTROL_MODE::SELECT;
                    control_state.active = false;
                };

                if (ImGui::Button("F", ImVec2(35, 35))) {
                    control_state.mode = state::CONTROL_MODE::FILL;
                    control_state.active = false;
                };

                if (ImGui::Button("P", ImVec2(35, 35))) {
                    control_state.mode = state::CONTROL_MODE::PICKING_COLOR;
                    control_state.active = false;
                };

                ImGui::End();
            }


            {
                ImGui::SetNextWindowSize(ImVec2(300, window.height - status_bar_height));
                ImGui::SetNextWindowPos(ImVec2(window.width - 300, 0));

                ImGui::Begin(
                    "Brush settings",
                    NULL,
                    ImGuiWindowFlags_NoTitleBar
                    | ImGuiWindowFlags_NoResize
                    | ImGuiWindowFlags_NoScrollbar
                    | ImGuiWindowFlags_NoFocusOnAppearing
                );

                if (control_state.mode == state::CONTROL_MODE::FILL) {
                    ImGui::Text("Fill mode");
                    if (ImGui::BeginCombo("dropdown fill", fill_mode_string(control_state.fill_mode).c_str())) {
                        for (int n = 0; n < 2; n++) {
                            const bool is_selected = (control_state.fill_mode == (state::FILL_MODE)n);
                            if (ImGui::Selectable(fill_mode_string((state::FILL_MODE)n).c_str(), is_selected))
                                control_state.fill_mode = (state::FILL_MODE)n;
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    if (control_state.fill_mode == state::FILL_MODE::OWNER_AND_CONTROLLER) {
                        ImGui::Text("Fill with:");
                        ImGui::Text("%s", control_state.fill_with_tag.c_str());
                    }
                }

                ImGui::End();
            }

            {
                ImGui::SetNextWindowSize(ImVec2(window.width, status_bar_height));
                ImGui::SetNextWindowPos(ImVec2(0, window.height - status_bar_height));

                ImGui::Begin(
                    "Status",
                    NULL,
                    ImGuiWindowFlags_NoTitleBar
                    | ImGuiWindowFlags_NoResize
                    | ImGuiWindowFlags_NoScrollbar
                    | ImGuiWindowFlags_NoFocusOnAppearing
                );

                ImGui::Text("Status: ");

                ImGui::SameLine();

                switch (control_state.mode) {

                case state::CONTROL_MODE::NONE:
                    ImGui::Text("None");
                    break;
                case state::CONTROL_MODE::PICKING_COLOR:
                    ImGui::Text("Pick color");
                    break;
                case state::CONTROL_MODE::PAINTING:
                    ImGui::Text("Paint");
                    break;
                case state::CONTROL_MODE::FILL:
                    ImGui::Text("Fill");
                    break;
                case state::CONTROL_MODE::SET_STATE:
                    ImGui::Text("Set state");
                    break;
                case state::CONTROL_MODE::SELECT:
                    ImGui::Text("Select");
                    break;
                break;
                }

                ImGui::SameLine();
                ImGui::Text("Provinces %d", (int)map_state.provinces.size());

                ImGui::SameLine();
                ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                ImGui::End();
            }

            widgets::selection(map_state, control_state, storage);

            if (control_state.context_province == 0) {
                ImGui::SetNextWindowSize(ImVec2(200, 100));
                ImGui::SetNextWindowPos(ImVec2(window.mouse_x + 25, window.mouse_y + 25));

                ImGui::Begin(
                    "tooltip",
                    NULL,
                    ImGuiWindowFlags_NoTitleBar
                    | ImGuiWindowFlags_NoResize
                    | ImGuiWindowFlags_NoScrollbar
                    | ImGuiWindowFlags_NoFocusOnAppearing
                );
                switch (control_state.mode) {
                case state::CONTROL_MODE::NONE:
                    ImGui::Text("None");
                    break;
                case state::CONTROL_MODE::PICKING_COLOR:
                    ImGui::Text("Pick color");
                    break;
                case state::CONTROL_MODE::PAINTING:
                    ImGui::Text("Paint");
                    break;
                case state::CONTROL_MODE::FILL:
                    ImGui::Text("Fill");
                    break;
                case state::CONTROL_MODE::SET_STATE:
                    ImGui::Text("Set state");
                case state::CONTROL_MODE::SELECT:
                    ImGui::Text("Select");
                break;
                }

                ImGui::Text("x %f y %f", control_state.mouse_map_coord.x, control_state.mouse_map_coord.y);

                ImGui::End();
            } else {
                ImGui::SetNextWindowSize(ImVec2(200, 100));
                ImGui::SetNextWindowPos(ImVec2(control_state.pixel_context.x, control_state.pixel_context.y));

                ImGui::Begin(
                    "tooltip",
                    NULL,
                    ImGuiWindowFlags_NoTitleBar
                    | ImGuiWindowFlags_NoResize
                    | ImGuiWindowFlags_NoScrollbar
                    | ImGuiWindowFlags_NoFocusOnAppearing
                );

                if (control_state.selected_adjacency) {
                    auto& adj = map_state.adjacencies[control_state.selected_adjacency];

                    if (ImGui::Button("Set from")) {
                        adj.from = control_state.context_province;
                        update_adj_buffers(map_state, centers_buffer, adj_vertices_count);
                    }

                    if (adj.type != game_definition::ADJACENCY_TYPE::IMPASSABLE) {
                        if (ImGui::Button("Set through")) {
                            adj.through = control_state.context_province;
                        }
                        update_adj_buffers(map_state, centers_buffer, adj_vertices_count);
                    }

                    if (ImGui::Button("Set to")) {
                        adj.to = control_state.context_province;
                        update_adj_buffers(map_state, centers_buffer, adj_vertices_count);
                    }
                }

                ImGui::End();
            }

            {
                ImGui::Begin(
                    "Adjacencies",
                    NULL,
                    ImGuiWindowFlags_NoFocusOnAppearing
                );

                if (ImGui::Button("Delete selected")) {
                    if (control_state.selected_adjacency != 0) {
                        auto& adj = map_state.adjacencies[control_state.selected_adjacency];
                        adj.mark_for_delete = true;
                        update_adj_buffers(map_state, centers_buffer, adj_vertices_count);
                    }
                }

                if (ImGui::BeginTable("adj", 5)) {

                    ImGui::TableSetupColumn("From");
                    ImGui::TableSetupColumn("Through");
                    ImGui::TableSetupColumn("To");
                    ImGui::TableSetupColumn("Type");
                    ImGui::TableSetupColumn("Comment");

                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                    for (int column = 0; column < 5; column++)
                    {
                        ImGui::TableSetColumnIndex(column);
                        const char* column_name = ImGui::TableGetColumnName(column);
                        ImGui::PushID(column);
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                        ImGui::PopStyleVar();
                        ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
                        ImGui::TableHeader(column_name);
                        ImGui::PopID();
                    }

                    std::vector<int> local_adjacencies;

                    for (int i = 0; i < map_state.adjacencies.size(); i++) {
                        auto & adj = map_state.adjacencies[i];

                        if (adj.mark_for_delete) {
                            continue;
                        }

                        if (
                            adj.from == control_state.selected_province_id
                            || adj.through == control_state.selected_province_id
                            || adj.to == control_state.selected_province_id
                        ) {
                            local_adjacencies.push_back(i);
                        }
                    }

                    for (int row = 0; row < local_adjacencies.size(); row++)
                    {
                        auto & adj = map_state.adjacencies[local_adjacencies[row]];
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::PushID(row);
                        ImGui::RadioButton("", &control_state.selected_adjacency, local_adjacencies[row]);
                        ImGui::PopID();
                        ImGui::SameLine();
                        ImGui::Text("%d", adj.from);
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", adj.through);
                        ImGui::TableNextColumn();
                        ImGui::Text("%d", adj.to);
                        ImGui::TableNextColumn();
                        switch (adj.type) {

                        case game_definition::ADJACENCY_TYPE::INVALID:
                        ImGui::Text("invalid");
                        break;
                        case game_definition::ADJACENCY_TYPE::STRAIT_CROSSING:
                        ImGui::Text("crossing");
                        break;
                        case game_definition::ADJACENCY_TYPE::CANAL:
                        ImGui::Text("canal");
                        break;
                        case game_definition::ADJACENCY_TYPE::IMPASSABLE:
                        ImGui::Text("impassable");
                        break;
                        }

                        ImGui::TableNextColumn();
                        ImGui::Text("%s", adj.comment.c_str());
                    }
                    ImGui::EndTable();
                }

                ImGui::End();
            }

            auto target = glm::ivec2(control_state.mouse_map_coord);

            if (control_state.active) {
                switch (control_state.mode) {
                    case state::CONTROL_MODE::PICKING_COLOR:
                        pick_color(control_state, map_state);
                        control_state.mode = state::CONTROL_MODE::NONE;
                        break;
                    case state::CONTROL_MODE::PAINTING:
                        paint(control_state, map_state);
                        update_map_texture(control_state, map_state);
                        break;
                    case state::CONTROL_MODE::FILL:
                        while(target.x != control_state.delayed_map_coord.x || target.y != control_state.delayed_map_coord.y) {
                            if (target.x > control_state.delayed_map_coord.x) {
                                control_state.delayed_map_coord.x++;
                            }
                            if (target.x < control_state.delayed_map_coord.x) {
                                control_state.delayed_map_coord.x--;
                            }
                            if (target.y > control_state.delayed_map_coord.y) {
                                control_state.delayed_map_coord.y++;
                            }
                            if (target.y < control_state.delayed_map_coord.y) {
                                control_state.delayed_map_coord.y--;
                            }
                            paint_line(control_state, map_state);
                        }
                        break;
                    case state::CONTROL_MODE::SET_STATE:
                        paint_state(control_state, map_state);
                        update_map_texture(control_state, map_state);
                        break;
                    case state::CONTROL_MODE::SELECT:
                        update_select(control_state, map_state);
                        break;
                    default: break;
                }
            }


            if ((control_state.update_texture || control_state.update_texture_part) && (update_texture_timer >= frame_time * 5.f)) {
                update_map_texture(control_state, map_state);
                update_texture_timer = 0.f;
                control_state.update_texture = false;
            }




            glClear(GL_COLOR_BUFFER_BIT);

            auto view = glm::identity<glm::mat4>();
            view = glm::scale(view, glm::vec3{1.f / zoom, (float)window.width / (float)window.height / zoom, 1.f});
            view = glm::translate(view, glm::vec3(camera, 0.f));

            glm::mat4 model = glm::identity<glm::mat4>();
            model = glm::translate(model, shift);
            model = glm::scale(model, scale / (float)window.width * (float)map_state.size_x);

            glUseProgram(program);

            glUniform1f(TO_LOCATION[SHADER_UNIFORMS::ZOOM], zoom);
            glUniform2f(TO_LOCATION[SHADER_UNIFORMS::SIZE], map_state.size_x, map_state.size_y);
            glUniform1f(TO_LOCATION[SHADER_UNIFORMS::PIXEL_X], control_state.mouse_map_coord.x);
            glUniform1f(TO_LOCATION[SHADER_UNIFORMS::PIXEL_Y], control_state.mouse_map_coord.y);
            glUniform2fv(
                TO_LOCATION[SHADER_UNIFORMS::SELECTED_PROVINCE],
                1, reinterpret_cast<float*>(&control_state.selected_province)
            );
            glUniform2fv(
                TO_LOCATION[SHADER_UNIFORMS::HOVERED_PROVINCE],
                1, reinterpret_cast<float*>(&control_state.hovered_province)
            );

            glUniformMatrix4fv(TO_LOCATION[SHADER_UNIFORMS::MODEL], 1, false, reinterpret_cast<float*>(&model));
            glUniformMatrix4fv(TO_LOCATION[SHADER_UNIFORMS::VIEW], 1, false, reinterpret_cast<float*>(&view));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, control_state.main_texture);
            glUniform1i(TO_LOCATION[SHADER_UNIFORMS::PROVINCES_RGB], 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, control_state.sea_texture);
            glUniform1i(TO_LOCATION[SHADER_UNIFORMS::PROVINCES_DATA], 1);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, control_state.state_texture);
            glUniform1i(TO_LOCATION[SHADER_UNIFORMS::STATES_DATA], 2);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, control_state.nation_texture);
            glUniform1i(TO_LOCATION[SHADER_UNIFORMS::OWNER_DATA], 3);

            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, control_state.rivers_texture);
            glUniform1i(TO_LOCATION[SHADER_UNIFORMS::RIVERS], 3);


            glBindVertexArray(fake_VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6 * 2 * 2);

            state::check_gl_error("After draw:");


            glUseProgram(line_program);

            glUniformMatrix4fv(TO_LOCATION[SHADER_UNIFORMS::MODEL_LINE], 1, false, reinterpret_cast<float*>(&model));
            glUniformMatrix4fv(TO_LOCATION[SHADER_UNIFORMS::VIEW_LINE], 1, false, reinterpret_cast<float*>(&view));

            glBindVertexArray(center_vertex_array);
            glDrawArrays(GL_TRIANGLES, 0, adj_vertices_count);

            state::check_gl_error("After draw line:");

            glUseProgram(rivers_program);

            glUniformMatrix4fv(TO_LOCATION[SHADER_UNIFORMS::MODEL_RIVER], 1, false, reinterpret_cast<float*>(&model));
            glUniformMatrix4fv(TO_LOCATION[SHADER_UNIFORMS::VIEW_RIVER], 1, false, reinterpret_cast<float*>(&view));

            glBindVertexArray(rivers_array);
            glDrawArrays(GL_TRIANGLES, 0, control_state.rivers_mesh.size());

            state::check_gl_error("After draw rivers:");

            if (control_state.reset_focus) {
                ImGui::SetWindowFocus(NULL);
                control_state.reset_focus = false;
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            control_state.selection_delay = false;

            SDL_GL_SwapWindow(window.window);
        }
    }
    return EXIT_SUCCESS;
}