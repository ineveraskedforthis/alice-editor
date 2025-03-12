#include <array>
#include <iostream>
#include <chrono>
#include <SDL.h>
#include "SDL_events.h"
#include "SDL_keycode.h"
#include <string>
#include <thread>
#include <vector>
#include "GL/glew.h"
#include "modules/glm/fwd.hpp"
#include "modules/parsing/generated/parsers_core.hpp"
#include "modules/parsing/map.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "modules/glm/ext/matrix_transform.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"
#include "modules/colormaps/viridis.hpp"
#include "modules/read_shader.hpp"
#include "modules/ui/editor.hpp"
#include "modules/ui/misc.hpp"
#include "modules/ui/window-wrapper.hpp"
#include "modules/editor-state/editor-state.hpp"
#include "modules/assets-manager/assets.hpp"

#undef max
#undef min
#undef clamp

std::string to_string(std::string_view str) {
    return std::string(str.begin(), str.end());
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

std::array<GLuint, 256> TO_LOCATION {};

void update_rivers_mesh(state::control& state, state::layers_stack layers, state::editor& map) {
    auto count = 0;
    auto size_x = layers.get_provinces_image_x();
    auto size_y = layers.get_provinces_image_y();
    for (int i = 0; i < size_x; i++) {
        for (int j = 0; j < size_y; j++) {
            auto pixel = layers.coord_to_pixel(glm::ivec2{i, j});
            if (map.rivers_raw[pixel * 4 + 1] < 255) {
                auto width = (40.f + 255.f - float(map.rivers_raw[pixel * 4])) / 500.f;
                if (i + 1 < size_x){
                    auto next = layers.coord_to_pixel(glm::ivec2{i + 1, j});
                    if (map.rivers_raw[next * 4] < 255) {
                        //std::cout << i << " " << j << " " << next << " " << pixel << " i + 1" << std::endl;
                        auto width_next = (40.f + 255.f - float(map.rivers_raw[next * 4])) / 500.f;
                        count++;
                        // triangle 1
                        state.rivers_mesh.push_back(i / (float)size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j - width) / (float)size_y * 2.f - 1.f);

                        state.rivers_mesh.push_back(i / (float)size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j + width) / (float)size_y * 2.f - 1.f);

                        state.rivers_mesh.push_back((i + 1) / (float)size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j + width_next) / (float)size_y * 2.f - 1.f);

                        // triangle 2
                        state.rivers_mesh.push_back(i / (float)size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j - width) / (float)size_y * 2.f - 1.f);

                        state.rivers_mesh.push_back((i + 1) / (float)size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j + width_next) / (float)size_y * 2.f - 1.f);

                        state.rivers_mesh.push_back((i + 1) / (float)size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j - width_next) / (float)size_y * 2.f - 1.f);
                    }
                }

                if (j + 1 < size_y){
                    auto next = layers.coord_to_pixel(glm::ivec2{i, j + 1});
                    if (map.rivers_raw[next * 4] < 255) {
                        //std::cout << i << " " << j << " " << next << " " << pixel << " " << " j + 1" << std::endl;
                        auto width_next = (40.f + 255.f - float(map.rivers_raw[next * 4])) / 500.f;
                        count++;
                        // triangle 1
                        state.rivers_mesh.push_back((i - width) / (float)size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j) / (float)size_y * 2.f - 1.f);

                        state.rivers_mesh.push_back((i + width) / (float)size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j) / (float)size_y * 2.f - 1.f);

                        state.rivers_mesh.push_back((i + width_next) / (float)size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j + 1) / (float)size_y * 2.f - 1.f);

                        // triangle 2
                        state.rivers_mesh.push_back((i - width) / (float)size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back(j / (float)size_y * 2.f - 1.f );

                        state.rivers_mesh.push_back((i + width_next) / (float)size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j + 1) / (float)size_y * 2.f - 1.f );

                        state.rivers_mesh.push_back((i - width_next) / (float)size_x * 2.f - 1.f);
                        state.rivers_mesh.push_back((j + 1) / (float)size_y * 2.f - 1.f);
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
        window::wrapper window {};
        state::control control_state {};
        assets::storage storage {};
        state::layers_stack layers {};
        layers.load_owner_texture_to_gpu();
        state::editor editor {};

        float data_buffer[2000];
        uint8_t province_data[256 * 256 * 3];

        std::string path_1;
        std::string path_2;
        std::string path_3;

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

        TO_LOCATION[SHADER_UNIFORMS::PROVINCE_INDICES] = glGetUniformLocation(program, "province_indices");
        TO_LOCATION[SHADER_UNIFORMS::PROVINCE_IS_SEA] = glGetUniformLocation(program, "is_sea_texture");
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
        // glm::vec3 scale {1.f, (float)map_state.size_y / (float)map_state.size_x, 1.f};

        // std::cout << " " << map_state.size_x << " " << map_state.size_y <<"\n";

        // std::cout << scale.x << " " << scale.y << " " << scale.z << "\n";

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

            window.update(layers, control_state, io, camera, zoom);
            if (!window.running)
                break;
            if (window.delayed) {
                continue;
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            if (layers.data.size()>0){
                widgets::main_scene(
                    window, io, layers, control_state, storage, editor,
                    program, line_program, rivers_program,
                    centers_buffer,
                    fake_VAO, rivers_buffer, centers_buffer,
                    TO_LOCATION, adj_vertices_count,
                    update_texture_timer, frame_time, camera, zoom, shift
                );
            } else {
                ImGui::SetNextWindowSize(ImVec2(400, 400));
                ImGui::SetNextWindowPos(ImVec2(int(window.width / 2 - 200), int(window.height / 2 - 200)));
                ImGui::Begin("Main menu");
                if(ImGui::Button("Load")){
                    parsers::load_templates(editor, "./editor-input");

                    {
                        state::layer l {};
                        l.path = "./base-game";
                        parsers::load_layer(l);
                        l.load_state_texture_to_gpu();
                        l.load_sea_texture_to_gpu();
                        layers.data.push_back(l);
                        layers.current_layer_index = 0;
                        layers.generate_indices();
                        layers.update_owner_texture();
                        layers.commit_owner_texture_to_gpu();
                        layers.indices.load_province_texture_to_gpu();
                    }

                    {
                        state::layer l {};
                        l.path = "./editor-input";
                        parsers::load_layer(l);
                        l.load_state_texture_to_gpu();
                        l.load_sea_texture_to_gpu();
                        layers.data.push_back(l);
                        layers.current_layer_index = 1;
                        layers.generate_indices();
                        layers.update_owner_texture();
                        layers.commit_owner_texture_to_gpu();
                        layers.indices.load_province_texture_to_gpu();
                    }
                }
                ImGui::End();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            control_state.selection_delay = false;

            SDL_GL_SwapWindow(window.window);
        }
    }
    return EXIT_SUCCESS;
}