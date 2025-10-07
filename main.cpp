#include "imgui_internal.h"
#include "modules/OS/win-wrapper.hpp"
#include "modules/misc.hpp"
#include <filesystem>
#include <shobjidl_core.h>
#undef max
#undef min
#undef clamp
#include <windef.h>

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
#include "modules/read_shader.hpp"
#include "modules/ui/editor.hpp"
#include "modules/ui/misc.hpp"
#include "modules/ui/window-wrapper.hpp"
#include "modules/editor-state/editor-state.hpp"
#include "modules/assets-manager/assets.hpp"

#include "win-wrapper.hpp"

#include "objbase.h"

#undef max
#undef min
#undef clamp

std::string to_string(std::string_view str) {
    return std::string(str.begin(), str.end());
}


std::wstring open_path_selection_dialog(bool base_game) {
    IFileOpenDialog* DIALOG;
    auto DIALOG_RESULT = CoCreateInstance(
        CLSID_FileOpenDialog,
        NULL,
        CLSCTX_ALL,
        IID_IFileOpenDialog,
        reinterpret_cast<void**>(&DIALOG)
    );
    if(FAILED(DIALOG_RESULT)) {
        return L"";
    }

    if (base_game) {
        DIALOG->SetClientGuid(winapi::UUID_open_base_game);
    } else {
        DIALOG->SetClientGuid(winapi::UUID_open_mod);
    }

    DIALOG->SetOptions(FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM | FOS_PICKFOLDERS);

    DIALOG_RESULT = DIALOG->Show(NULL);
    if(FAILED(DIALOG_RESULT)) {
        DIALOG->Release();
        return L"";
    }

    IShellItem* ITEM;
    DIALOG_RESULT = DIALOG->GetResult(&ITEM);
    if(FAILED(DIALOG_RESULT)) {
        DIALOG->Release();
        return L"";
    }

    //  STORE AND CONVERT THE FILE NAME
    PWSTR RETRIEVED_PATH;
    DIALOG_RESULT = ITEM->GetDisplayName(SIGDN_FILESYSPATH, &RETRIEVED_PATH);
    if(FAILED(DIALOG_RESULT)) {
        ITEM->Release();
        DIALOG->Release();
        return L"";
    }

    std::wstring path(RETRIEVED_PATH);
    CoTaskMemFree(RETRIEVED_PATH);
    ITEM->Release();
    DIALOG->Release();
    return path;
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
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    std::cout << "Welcome\n";

    {
        winapi::load_uuids();

        window::wrapper window {};

        state::control control_state {};
        assets::storage storage {};
        state::layers_stack layers {};
        layers.load_province_colors_texture_to_gpu();
        state::editor editor {};

        float data_buffer[2000];
        uint8_t province_data[256 * 256 * 3];

        std::string path_1;
        std::string path_2;
        std::string path_3;

        std::cout << "loading shaders2\n";
        auto line_vertex_shader = create_shader(
            GL_VERTEX_SHADER,
            read_shader("./shaders/line.vs").c_str()
        );
        auto line_fragment_shader = create_shader(
            GL_FRAGMENT_SHADER,
            read_shader("./shaders/line.fs").c_str()
        );
        editor.line_program = create_program(line_vertex_shader, line_fragment_shader);
        editor.rivers_program = create_program(line_vertex_shader, line_fragment_shader);

        TO_LOCATION[SHADER_UNIFORMS::MODEL_RIVER] = glGetUniformLocation(editor.rivers_program, "model");
        TO_LOCATION[SHADER_UNIFORMS::VIEW_RIVER] = glGetUniformLocation(editor.rivers_program, "view");

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
        glBindAttribLocation(editor.rivers_program, 0, "vertex");

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
        editor.map_program = create_program(vertex_shader, fragment_shader);

        {
            auto vertex_shader = create_shader(
                GL_VERTEX_SHADER,
                read_shader("./shaders/triangle.vs").c_str()
            );
            auto fragment_shader = create_shader(
                GL_FRAGMENT_SHADER,
                read_shader("./shaders/triangle.fs").c_str()
            );
            editor.triangle_program = create_program(vertex_shader, fragment_shader);

            TO_LOCATION[SHADER_UNIFORMS::TRIANGLE_POINT_0] = glGetUniformLocation(editor.triangle_program, "point0");
            TO_LOCATION[SHADER_UNIFORMS::TRIANGLE_POINT_1] = glGetUniformLocation(editor.triangle_program, "point1");
            TO_LOCATION[SHADER_UNIFORMS::TRIANGLE_POINT_2] = glGetUniformLocation(editor.triangle_program, "point2");
            TO_LOCATION[SHADER_UNIFORMS::TRIANGLE_MODEL] = glGetUniformLocation(editor.triangle_program, "model");
            TO_LOCATION[SHADER_UNIFORMS::TRIANGLE_VIEW] = glGetUniformLocation(editor.triangle_program, "view");
        }

        state::check_gl_error("Shaders");

        TO_LOCATION[SHADER_UNIFORMS::PROVINCE_INDICES] = glGetUniformLocation(editor.map_program, "province_indices");
        TO_LOCATION[SHADER_UNIFORMS::PROVINCE_IS_SEA] = glGetUniformLocation(editor.map_program, "is_sea_texture");
        TO_LOCATION[SHADER_UNIFORMS::STATES_DATA] = glGetUniformLocation(editor.map_program, "state_data");
        TO_LOCATION[SHADER_UNIFORMS::OWNER_DATA] = glGetUniformLocation(editor.map_program, "owner_data");
        TO_LOCATION[SHADER_UNIFORMS::RIVERS] = glGetUniformLocation(editor.map_program, "rivers");

        TO_LOCATION[SHADER_UNIFORMS::HAVE_BORDERS] = glGetUniformLocation(editor.map_program, "have_color_border");

        TO_LOCATION[SHADER_UNIFORMS::MODEL] = glGetUniformLocation(editor.map_program, "model");
        TO_LOCATION[SHADER_UNIFORMS::VIEW] = glGetUniformLocation(editor.map_program, "view");
        TO_LOCATION[SHADER_UNIFORMS::PROJECTION] = glGetUniformLocation(editor.map_program, "projection");
        TO_LOCATION[SHADER_UNIFORMS::ZOOM] = glGetUniformLocation(editor.map_program, "zoom");
        TO_LOCATION[SHADER_UNIFORMS::SIZE] = glGetUniformLocation(editor.map_program, "size");

        TO_LOCATION[SHADER_UNIFORMS::PIXEL_X] = glGetUniformLocation(editor.map_program, "pixel_x");
        TO_LOCATION[SHADER_UNIFORMS::PIXEL_Y] = glGetUniformLocation(editor.map_program, "pixel_y");

        TO_LOCATION[SHADER_UNIFORMS::SELECTED_PROVINCE] = glGetUniformLocation(editor.map_program, "selected_province");
        TO_LOCATION[SHADER_UNIFORMS::HOVERED_PROVINCE] = glGetUniformLocation(editor.map_program, "hovered_province");

        std::cout << "prepare to run\n";
        float time = 0.f;
        auto last_frame_start = std::chrono::high_resolution_clock::now();

        glGenVertexArrays(1, &editor.map_fake_VAO);
        state::check_gl_error("Vertex array");

        glm::vec3 shift {0, 0, 0.5};
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



        TO_LOCATION[SHADER_UNIFORMS::MODEL_LINE] = glGetUniformLocation(editor.line_program, "model");
        TO_LOCATION[SHADER_UNIFORMS::VIEW_LINE] = glGetUniformLocation(editor.line_program, "view");

        state::check_gl_error("line shaders");

        // prepare opengl stuff for fill tool display
        glGenVertexArrays(1, &editor.fill_tool_VertexArray);

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
        glBindAttribLocation(editor.line_program, 0, "vertex");

        int adj_vertices_count = 0;
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);


        state::check_gl_error("line buffers");

        window.running = true;

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImFont* font = io.Fonts->AddFontFromFileTTF(
            "./assets/Montserrat/static/Montserrat-Regular.ttf"
        );
        ImFontConfig config;
        config.MergeMode = true;
        io.Fonts->AddFontFromFileTTF(
            "./assets/Noto_Sans_SC/static/NotoSansSC-Regular.ttf", 0.f, &config
        );

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 6.f;
        style.ChildRounding = 6.f;
        style.FrameRounding = 6.f;
        style.PopupRounding = 6.f;
        style.ScrollbarRounding = 6.f;
        style.GrabRounding = 2.f;
        style.TabRounding = 6.f;
        style.SeparatorTextBorderSize = 5.f;
        style.FrameBorderSize = 1.f;
        style.TabBorderSize = 1.f;

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

            window.update(layers, control_state, io, zoom);
            if (!window.running)
                break;
            if (window.delayed) {
                continue;
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();


            ImGui::PushFont(font);
            ImGui::StyleColorsLight();

            // ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.f, 0.f));

            if (layers.data.size()>0){
                widgets::main_scene(
                    window, io, layers, control_state, storage, editor,
                    centers_buffer, rivers_buffer, center_vertex_array,
                    TO_LOCATION, adj_vertices_count,
                    update_texture_timer, frame_time, zoom, shift
                );
            } else {
                ImGui::SetNextWindowSize(ImVec2(400, 400));
                ImGui::SetNextWindowPos(ImVec2(int(window.width / 2 - 200), int(window.height / 2 - 200)));
                ImGui::Begin("Main menu");

                static std::wstring path_basegame = L"./base-game";
                static std::wstring path_mod = L"./editor-input";

                ImGui::Text("Current path to base game");
                ImGui::Text("%ls", path_basegame.c_str());
                if (ImGui::Button("Select path to base game")) {
                    path_basegame = open_path_selection_dialog(true);
                }

                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 10, 10, 255));
                if (!std::filesystem::exists(path_basegame)) {
                    ImGui::Text("BASE GAME FOLDER DOES NOT EXIST!");
                } else if (std::filesystem::is_empty(path_basegame)) {
                    ImGui::Text("BASE GAME FOLDER IS EMPTY!");
                }
                ImGui::PopStyleColor();

                ImGui::Text("Current path to mod");
                ImGui::Text("%ls", path_mod.c_str());
                if (ImGui::Button("Select path to mod folder")) {
                    path_mod = open_path_selection_dialog(false);
                }

                if(ImGui::Button("Load")){
                    parsers::load_templates(editor, "./editor-input");

                    std::cout << "Loading started\n";

                    auto t_start = std::chrono::high_resolution_clock::now();

                    {
                        state::layer l {};
                        l.path = conversions::w_to_u8(path_basegame);
                        parsers::load_layer(layers, l);
                        l.load_state_texture_to_gpu();
                        l.load_sea_texture_to_gpu();
                        layers.data.push_back(l);
                        layers.current_layer_index = 0;
                        layers.generate_indices();
                        layers.request_map_update = true;
                        layers.indices.load_province_texture_to_gpu();
                    }

                    {
                        state::layer l {};
                        l.path = conversions::w_to_u8(path_mod);
                        parsers::load_layer(layers, l);
                        l.load_state_texture_to_gpu();
                        l.load_sea_texture_to_gpu();
                        layers.data.push_back(l);
                        layers.current_layer_index = 1;
                        layers.generate_indices();
                        layers.request_map_update = true;
                        layers.indices.load_province_texture_to_gpu();
                    }

                    auto dates = layers.get_available_dates();
                    control_state.map_date = dates[0];

                    auto t_end = std::chrono::high_resolution_clock::now();

                    // https://stackoverflow.com/a/22387757/10281950
                    /* Getting number of milliseconds as an integer. */
                    auto ms_int = duration_cast<std::chrono::milliseconds>(t_end - t_start);
                    /* Getting number of milliseconds as a double. */
                    std::chrono::duration<double, std::milli> ms_double = t_end - t_start;
                    std::cout << ms_int.count() << "ms\n";
                    std::cout << ms_double.count() << "ms\n";
                }
                ImGui::End();
            }


            ImGui::PopFont();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            control_state.selection_delay = false;

            SDL_GL_SwapWindow(window.window);
        }
    }
    CoUninitialize();
    return EXIT_SUCCESS;
}