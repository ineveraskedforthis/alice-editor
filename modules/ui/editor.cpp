#include "explorer.hpp"
#include "imgui.h"
#include "../editor-state/editor-state.hpp"
#include "window-wrapper.hpp"
#include "selection_widget.hpp"

#include "misc.hpp"
#include <string>
#include "editor.hpp"
#include "../parsing/map.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include "../glm/ext/matrix_transform.hpp"

namespace widgets {
    void brushes(state::control& control) {
        ImGui::Begin(
            "Brushes",
            NULL,
            ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoFocusOnAppearing
        );


        if (ImGui::Button("S", ImVec2(35, 35))) {
            control.mode = state::CONTROL_MODE::SELECT;
            control.active = false;
        };

        if (ImGui::Button("F", ImVec2(35, 35))) {
            control.mode = state::CONTROL_MODE::FILL;
            control.active = false;
        };

        if (ImGui::Button("P", ImVec2(35, 35))) {
            control.mode = state::CONTROL_MODE::PICKING_COLOR;
            control.active = false;
        };

        ImGui::End();
    }

    void settings(state::layers_stack& layers, state::control& control) {
        ImGui::Begin(
            "Brush settings",
            NULL,
            ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoFocusOnAppearing
        );

        if (ImGui::Button("SAVE")) {
            parsers::unload_data(
                layers.data[layers.current_layer_index],
                "./editor-output/" + std::to_string(layers.current_layer_index) + "/"
            );
        }

        ImGui::Text("Layers:");

        for (int i = 0; i < layers.data.size(); i++) {
            bool old = layers.data[i].visible;
            bool activate = layers.data[i].visible;
            ImGui::PushID(i);
            if (i == 0) ImGui::BeginDisabled();
            ImGui::Checkbox("##", &activate);
            if (i == 0) ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::Text("%s", layers.data[i].path.c_str());
            if (activate != old) {
                if (!activate) {
                    for (int j = i; j < layers.data.size(); j++) {
                        layers.data[i].visible = false;
                    }
                    layers.current_layer_index = i - 1;
                    layers.update_owner_texture();
                    layers.commit_owner_texture_to_gpu();
                    layers.generate_indices();
                    layers.indices.commit_province_texture_changes_to_gpu();
                } else {
                    for (int j = 0; j <= i; j++) {
                        layers.data[i].visible = true;
                    }
                    layers.current_layer_index = i;
                    layers.update_owner_texture();
                    layers.commit_owner_texture_to_gpu();
                    layers.generate_indices();
                    layers.indices.commit_province_texture_changes_to_gpu();
                }
            }
            ImGui::PopID();
        }

        if (control.mode == state::CONTROL_MODE::FILL) {
            ImGui::Text("Fill mode");
            if (ImGui::BeginCombo("dropdown fill", fill_mode_string(control.fill_mode).c_str())) {
                for (int n = 0; n < 2; n++) {
                    const bool is_selected = (control.fill_mode == (state::FILL_MODE)n);
                    if (ImGui::Selectable(fill_mode_string((state::FILL_MODE)n).c_str(), is_selected))
                        control.fill_mode = (state::FILL_MODE)n;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (control.fill_mode == state::FILL_MODE::OWNER_AND_CONTROLLER) {
                ImGui::Text("Fill with:");
                ImGui::Text("%s", control.fill_with_tag.c_str());
            }
        }

        ImGui::End();
    }

    void status(state::control& control, ImGuiIO& io) {
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

        switch (control.mode) {

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
        // ImGui::Text("Provinces %d", (int)map_state.provinces.size());
        ImGui::Text("Province count: TODO");

        ImGui::SameLine();
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
    }

    void map_tooltip(state::control& control) {
        ImGui::Begin(
            "tooltip",
            NULL,
            ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoFocusOnAppearing
        );
        switch (control.mode) {
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

        ImGui::Text("x %f y %f", control.mouse_map_coord.x, control.mouse_map_coord.y);

        ImGui::End();
    }

    void map_context(state::layers_stack& layers, state::control& control, GLuint adj_buffer, int& adj_vertices_count) {
        ImGui::Begin(
            "tooltip",
            NULL,
            ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoFocusOnAppearing
        );

        if (control.selected_adjacency) {
            // auto& adj = map_state.adjacencies[control.selected_adjacency];

            // if (ImGui::Button("Set from")) {
            //     adj.from = control.context_province;
            //     layers.update_adj_buffers(adj_buffer, adj_vertices_count);
            // }

            // if (adj.type != game_definition::ADJACENCY_TYPE::IMPASSABLE) {
            //     if (ImGui::Button("Set through")) {
            //         adj.through = control.context_province;
            //     }
            //     layers.update_adj_buffers(adj_buffer, adj_vertices_count);
            // }

            // if (ImGui::Button("Set to")) {
            //     adj.to = control.context_province;
            //     layers.update_adj_buffers(adj_buffer, adj_vertices_count);
            // }
        }

        ImGui::End();
    }

    void entities_table(state::control& control) {
        ImGui::Begin(
            "Adjacencies",
            NULL,
            ImGuiWindowFlags_NoFocusOnAppearing
        );

        /*
        if (ImGui::Button("Delete selected")) {
            if (control.selected_adjacency != 0) {
                auto& adj = map_state.adjacencies[control.selected_adjacency];
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
                    adj.from == control.selected_province_id
                    || adj.through == control.selected_province_id
                    || adj.to == control.selected_province_id
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
                ImGui::RadioButton("", &control.selected_adjacency, local_adjacencies[row]);
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
        */
        ImGui::End();
    }

    void main_scene(
        window::wrapper& window,
        ImGuiIO& io,
        state::layers_stack& layers,
        state::control& control,
        assets::storage& storage,
        state::editor& editor,
        GLuint map_program,
        GLuint line_program,
        GLuint rivers_program,
        GLuint centers_buffer,
        GLuint fake_VAO,
        GLuint rivers_VAO,
        GLuint adj_VAO,
        std::array<GLuint, 256>& uniform_locations,
        int& adj_vertices_count,
        float& update_texture_timer,
        float frame_time,
        glm::vec2 camera,
        float zoom,
        glm::vec3 shift
    ) {
        int status_bar_height = 25;

        {
            ImGui::SetNextWindowSize(ImVec2(50, window.height - status_bar_height));
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            widgets::brushes(control);
        }
        {
            ImGui::SetNextWindowSize(ImVec2(300, window.height - status_bar_height));
            ImGui::SetNextWindowPos(ImVec2(window.width - 300, 0));
            widgets::settings(layers, control);
        }
        {
            ImGui::SetNextWindowSize(ImVec2(window.width, status_bar_height));
            ImGui::SetNextWindowPos(ImVec2(0, window.height - status_bar_height));
            widgets::status(control, io);
        }

        widgets::explorer(layers, control, editor, storage);
        widgets::selection(layers, control, editor, storage);

        if (control.context_province == 0) {
            ImGui::SetNextWindowSize(ImVec2(200, 100));
            ImGui::SetNextWindowPos(ImVec2(window.mouse_x + 25, window.mouse_y + 25));
            widgets::map_tooltip(control);
        } else {
            ImGui::SetNextWindowSize(ImVec2(200, 100));
            ImGui::SetNextWindowPos(ImVec2(control.pixel_context.x, control.pixel_context.y));
            widgets::map_context(layers, control, centers_buffer, adj_vertices_count);
        }

        auto target = glm::ivec2(control.mouse_map_coord);

        if (control.active) {
            switch (control.mode) {
                case state::CONTROL_MODE::PICKING_COLOR:
                    pick_color(control, layers);
                    control.mode = state::CONTROL_MODE::NONE;
                    break;
                case state::CONTROL_MODE::PAINTING:
                    paint(control, layers);
                    layers.indices.commit_province_texture_changes_to_gpu();
                    break;
                case state::CONTROL_MODE::FILL:
                    while(target.x != control.delayed_map_coord.x || target.y != control.delayed_map_coord.y) {
                        if (target.x > control.delayed_map_coord.x) {
                            control.delayed_map_coord.x++;
                        }
                        if (target.x < control.delayed_map_coord.x) {
                            control.delayed_map_coord.x--;
                        }
                        if (target.y > control.delayed_map_coord.y) {
                            control.delayed_map_coord.y++;
                        }
                        if (target.y < control.delayed_map_coord.y) {
                            control.delayed_map_coord.y--;
                        }
                        paint_line(control, layers);
                    }
                    break;
                case state::CONTROL_MODE::SET_STATE:
                    paint_state(control, layers);
                    layers.commit_state_texture();
                    break;
                case state::CONTROL_MODE::SELECT:
                    update_select(control, layers);
                    break;
                default: break;
            }
        }


        if ((layers.indices.update_texture || layers.indices.update_texture_part) && (update_texture_timer >= frame_time * 5.f)) {
            layers.indices.commit_province_texture_changes_to_gpu();
            update_texture_timer = 0.f;
            layers.indices.update_texture = false;
        }




        glClear(GL_COLOR_BUFFER_BIT);

        auto view = glm::identity<glm::mat4>();
        view = glm::scale(view, glm::vec3{1.f / zoom, (float)window.width / (float)window.height / zoom, 1.f});
        view = glm::translate(view, glm::vec3(camera, 0.f));

        glm::mat4 model = glm::identity<glm::mat4>();
        model = glm::translate(model, shift);
        auto size_x = layers.get_provinces_image_x();
        auto size_y = layers.get_provinces_image_y();

        glm::vec3 scale {1.f, (float)layers.get_provinces_image_y()/ (float)layers.get_provinces_image_x(), 1.f};
        model = glm::scale(model, scale / (float)window.width * (float)layers.get_provinces_image_x());

        glUseProgram(map_program);

        glUniform1f(uniform_locations[SHADER_UNIFORMS::ZOOM], zoom);
        glUniform2f(uniform_locations[SHADER_UNIFORMS::SIZE], size_x, size_y);
        glUniform1f(uniform_locations[SHADER_UNIFORMS::PIXEL_X], control.mouse_map_coord.x);
        glUniform1f(uniform_locations[SHADER_UNIFORMS::PIXEL_Y], control.mouse_map_coord.y);
        glUniform2fv(
            uniform_locations[SHADER_UNIFORMS::SELECTED_PROVINCE],
            1, reinterpret_cast<float*>(&control.selected_province)
        );
        glUniform2fv(
            uniform_locations[SHADER_UNIFORMS::HOVERED_PROVINCE],
            1, reinterpret_cast<float*>(&control.hovered_province)
        );

        glUniformMatrix4fv(uniform_locations[SHADER_UNIFORMS::MODEL], 1, false, reinterpret_cast<float*>(&model));
        glUniformMatrix4fv(uniform_locations[SHADER_UNIFORMS::VIEW], 1, false, reinterpret_cast<float*>(&view));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, layers.indices.id);
        glUniform1i(uniform_locations[SHADER_UNIFORMS::PROVINCE_INDICES], 0);

        glActiveTexture(GL_TEXTURE1);
        GLuint sea_texture;
        layers.get_sea_texture(sea_texture);
        glBindTexture(GL_TEXTURE_2D, sea_texture);
        glUniform1i(uniform_locations[SHADER_UNIFORMS::PROVINCE_IS_SEA], 1);

        glActiveTexture(GL_TEXTURE2);
        GLuint state_texture;
        layers.get_state_texture(state_texture);
        glBindTexture(GL_TEXTURE_2D, state_texture);
        glUniform1i(uniform_locations[SHADER_UNIFORMS::STATES_DATA], 2);

        glActiveTexture(GL_TEXTURE3);
        GLuint owners_texture;
        layers.get_owners_texture(owners_texture);
        glBindTexture(GL_TEXTURE_2D, owners_texture);
        glUniform1i(uniform_locations[SHADER_UNIFORMS::OWNER_DATA], 3);


        glBindVertexArray(fake_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6 * 2 * 2);

        state::check_gl_error("After draw:");


        glUseProgram(line_program);

        glUniformMatrix4fv(uniform_locations[SHADER_UNIFORMS::MODEL_LINE], 1, false, reinterpret_cast<float*>(&model));
        glUniformMatrix4fv(uniform_locations[SHADER_UNIFORMS::VIEW_LINE], 1, false, reinterpret_cast<float*>(&view));

        glBindVertexArray(adj_VAO);
        glDrawArrays(GL_TRIANGLES, 0, adj_vertices_count);

        state::check_gl_error("After draw line:");

        glUseProgram(rivers_program);

        glUniformMatrix4fv(uniform_locations[SHADER_UNIFORMS::MODEL_RIVER], 1, false, reinterpret_cast<float*>(&model));
        glUniformMatrix4fv(uniform_locations[SHADER_UNIFORMS::VIEW_RIVER], 1, false, reinterpret_cast<float*>(&view));

        glBindVertexArray(rivers_VAO);
        glDrawArrays(GL_TRIANGLES, 0, control.rivers_mesh.size());

        state::check_gl_error("After draw rivers:");

        if (control.reset_focus) {
            ImGui::SetWindowFocus(NULL);
            control.reset_focus = false;
        }
    }
}