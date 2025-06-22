#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>
#include "selection_widget.hpp"

#include <shobjidl_core.h>
#include <shobjidl.h>
#include <vector>
#include <winnt.h>
#include "win-wrapper.hpp"

#include "ui_enums.hpp"
#include "pops_buffer_widget.hpp"


//we need only declarations here
#include "SOIL2.h"
// #include "stb_image_write.h"

#include "../editor-state/editor-state.hpp"
#include "../assets-manager/assets.hpp"
#include "../misc.hpp"

// copied from Project Alice
// Related GPL file can be found in ParserGenerator folder
std::wstring utf8_to_wstring(std::string_view str) {
	if(str.size() > 0) {
		auto buffer = std::unique_ptr<WCHAR[]>(new WCHAR[str.length() * 2]);
		auto chars_written = MultiByteToWideChar(CP_UTF8, 0, str.data(), int32_t(str.length()), buffer.get(), int32_t(str.length() * 2));
		return std::wstring(buffer.get(), size_t(chars_written));
	}
	return std::wstring(L"");
}

std::string wstring_to_utf8(std::wstring str) {
	if(str.size() > 0) {
		auto buffer = std::unique_ptr<char[]>(new char[str.length() * 4]);
		auto chars_written = WideCharToMultiByte(CP_UTF8, 0, str.data(), int32_t(str.length()), buffer.get(), int32_t(str.length() * 4), NULL, NULL);
		return std::string(buffer.get(), size_t(chars_written));
	}
	return std::string("");
}

namespace widgets {

    void technology_folder(state::layers_stack& map, game_definition::nation_history* history, bool can_edit, std::wstring folder) {
        if (history == nullptr) {
            return;
        }
        if (!can_edit)
            ImGui::BeginDisabled();
        std::vector<std::string> technologies{};
        map.retrieve_techs(technologies, folder);
        for (int row = 0; row < technologies.size(); row++) {
            auto name = technologies[row];
            bool present = false;
            if (history->tech.contains(name)) {
                present = history->tech[name];
            }
            bool old = present;

            ImGui::PushID(row);
            ImGui::Checkbox("##", &present);
            ImGui::SameLine();
            ImGui::Text("%s", name.c_str());

            if (old != present)
                history->tech[name] = present;

            ImGui::PopID();
        }
        if (!can_edit)
            ImGui::EndDisabled();
    }

    void inventions_folder(state::layers_stack& map, game_definition::nation_history* history, bool can_edit, std::wstring folder) {
        if (history == nullptr) {
            return;
        }
        if (!can_edit)
            ImGui::BeginDisabled();
        std::vector<std::string> inventions{};
        map.retrieve_inventions(inventions, folder);
        for (int row = 0; row < inventions.size(); row++) {
            auto name = inventions[row];
            bool present = false;
            if (history->inventions.contains(name)) {
                present = history->inventions[name];
            }
            bool old = present;

            ImGui::PushID(row);
            ImGui::Checkbox("##", &present);
            ImGui::SameLine();
            ImGui::Text("%s", name.c_str());

            if (old != present)
                history->inventions[name] = present;

            ImGui::PopID();
        }
        if (!can_edit)
            ImGui::EndDisabled();
    }

    void issues_selection(state::layers_stack& map, game_definition::nation_history* history, bool can_edit) {
        if (history == nullptr) {
            return;
        }
        if (!can_edit)
            ImGui::BeginDisabled();
        std::vector<std::string> issues{};
        map.retrieve_issues(issues);
        for (int row = 0; row < issues.size(); row++) {
            auto name = issues[row];
            auto issue = map.get_issue(name);
            ImGui::PushID(row);
            ImGui::Text("%s", name.c_str());
            if (issue == nullptr) {
                ImGui::Text("Invalid issue");
            } else {
                if (ImGui::BeginCombo("Select", history->issues[name].c_str())) {
                    for (int n = 0; n < issue->options.size(); n++) {
                        if (ImGui::Selectable(issue->options[n].c_str(), history->issues[name] == issue->options[n])) {
                            history->issues[name] = issue->options[n];
                        }
                    }
                    ImGui::EndCombo();
                }
            }
            ImGui::PopID();
        }
        if (!can_edit)
            ImGui::EndDisabled();
    }

    std::wstring open_image_selection_dialog(GUID& dialog_id) {
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

        auto hres = DIALOG->SetClientGuid(dialog_id);

        if (hres != S_OK) {
            MessageBoxW(
                NULL,
                L"Error during setting dialog guid.",
                L"Something is wrong???",
                MB_OK
            );
            return L"";
        }

        DIALOG->SetDefaultExtension(L"tga");

        _COMDLG_FILTERSPEC FILTER_JPEG;
        FILTER_JPEG.pszName = L"JPG (*.jpg,*.jpeg)";
        FILTER_JPEG.pszSpec = L"*.jpg;*.jpeg";

        _COMDLG_FILTERSPEC FILTER_PNG;
        FILTER_PNG.pszName = L"PNG (*.png)";
        FILTER_PNG.pszSpec = L"*.png";

        _COMDLG_FILTERSPEC FILTER_TGA;
        FILTER_TGA.pszName = L"TGA (*.tga)";
        FILTER_TGA.pszSpec = L"*.tga";

        std::array<_COMDLG_FILTERSPEC, 3> FILE_TYPES {FILTER_JPEG, FILTER_PNG, FILTER_TGA};

        DIALOG->SetFileTypes(3, FILE_TYPES.data());
        DIALOG->SetOptions(FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_NOCHANGEDIR | FOS_FORCEFILESYSTEM);

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

    void commodity_widget_definition(state::layers_stack& layers, std::string commodity_name) {
        state::layer& active_layer = layers.data[layers.current_layer_index];
        auto commodity = layers.get_commodity_definition(commodity_name);

        if (commodity == nullptr) {
            ImGui::Text("INVALID TRADE GOOD");
        }

        ImGui::Text("%s", commodity_name.c_str());

        auto blocked_definition = false;
        if (!active_layer.has_goods) {
            blocked_definition = true;
            ImGui::Text("No goods are defined on the active layer.");
            ImGui::Text("Do you wish to copy goods to the active layer to edit them?");
            if (ImGui::Button("Copy goods to the active layer.")) {
                layers.copy_goods_to_current_layer();
                return;
            }
        }

        if (blocked_definition) {
            ImGui::BeginDisabled();
        }

        ImGui::InputFloat("Starting price", &commodity->cost);
        ImGui::Checkbox("Is money", &commodity->money);
        ImGui::Checkbox("Is local", &commodity->is_local);
        ImGui::Checkbox("Is tradeable", &commodity->tradeable);
        ImGui::Checkbox("Overseas penalty", &commodity->overseas_penalty);
        ImGui::Checkbox("Available from start", &commodity->available_from_start);
        ImGui::Checkbox("Uses potentials", &commodity->uses_potentials);

        float fr = (float)(commodity->r) / 255.f;
        float fg = (float)(commodity->g) / 255.f;
        float fb = (float)(commodity->b) / 255.f;

        float colors[3] = {fr, fg, fb};

        ImGui::PushItemWidth(128);

        ImGui::ColorPicker3("Color", colors);

        if (colors[0] != fr || colors[1] != fg || colors[2] != fb) {
            commodity->r = static_cast<uint8_t>(colors[0] * 255.f);
            commodity->g = static_cast<uint8_t>(colors[1] * 255.f);
            commodity->b = static_cast<uint8_t>(colors[2] * 255.f);
        }

        ImGui::PopItemWidth();

        if (blocked_definition) {
            ImGui::EndDisabled();
        }
    }

    void compress_image_to_commodity_strip(
        state::interface_dds_image& target,
        int count,
        int target_index,
        uint8_t* new_image,
        int size_x,
        int size_y,
        int channels
    ) {
        auto compressed_x = target.size_x / (count + 1);
        auto compressed_y = target.size_y;

        float area_x = float(size_x) / (float)compressed_x;
        float area_y = float(size_y) / (float)compressed_y;

        for (int i = 0; i < compressed_x; i++) {
            for (int j = 0; j < compressed_y; j++) {
                float total_count = 0.f;
                float total_r = 0.f;
                float total_g = 0.f;
                float total_b = 0.f;
                float total_a = 0.f;

                for (int shift_i = 0; shift_i < area_x; shift_i++) {
                    for (int shift_j = 0; shift_j < area_y; shift_j++) {
                        float width = 1.f;
                        if (area_x - (float)shift_i < 1.f) {
                            width = area_x - (float)shift_i;
                        }
                        float height = 1.f;
                        if (area_y - (float)shift_j < 1.f) {
                            height = area_y - (float)shift_j;
                        }

                        float area = width * height;
                        total_count += area;

                        int final_i = (int)(i * area_x + shift_i);
                        int final_j = (int)(j * area_y + shift_j);
                        int pixel = final_i + final_j * size_x;

                        total_r += new_image[pixel * channels + 0];
                        total_g += new_image[pixel * channels + 1];
                        total_b += new_image[pixel * channels + 2];
                        if (channels > 3) {
                            total_a += new_image[pixel * channels + 3];
                        } else {
                            total_a += area * 255.f;
                        }
                    }
                }

                int strip_i = compressed_x * (target_index + 1) + i;
                int strip_j = j;
                int pixel = strip_i + strip_j * target.size_x;

                target.data[pixel * target.channels + 0] = static_cast<uint8_t>(std::clamp(total_r / total_count, 0.f, 255.f));
                target.data[pixel * target.channels + 1] = static_cast<uint8_t>(std::clamp(total_g / total_count, 0.f, 255.f));
                target.data[pixel * target.channels + 2] = static_cast<uint8_t>(std::clamp(total_b / total_count, 0.f, 255.f));
                if (target.channels > 3) {
                    target.data[pixel * target.channels + 3] = static_cast<uint8_t>(std::clamp(total_a / total_count, 0.f, 255.f));
                }
            }
        }

        target.commit_to_gpu();
    }

    void commodity_widget_icon(state::layers_stack& layers, std::string commodity_name) {
        state::layer& active_layer = layers.data[layers.current_layer_index];
        auto commodity = layers.get_commodity_definition(commodity_name);
        if (commodity == nullptr) {
            ImGui::Text("INVALID TRADE GOOD");
        }

        ImGui::Text("%s", commodity_name.c_str());

        auto count = layers.get_commodities_count();
        auto width = 1.f / (float)(count + 1);
        auto start = width * (commodity->index + 1);
        auto end = width + start;

        {
            GLuint texture;
            int size_x;
            int size_y;
            ImGui::Text("resources_big.dds");
            if (layers.get_resources_texture_big(texture, size_x, size_y)) {
                ImGui::Image(
                    texture,
                    ImVec2(size_x * width, size_y),
                    ImVec2(start, 0.f),
                    ImVec2(end, 1.f)
                );
            }
        }

        {
            GLuint texture;
            int size_x;
            int size_y;
            ImGui::Text("resources.dds");
            if (layers.get_resources_texture_medium(texture, size_x, size_y)) {
                ImGui::Image(
                    texture,
                    ImVec2(size_x * width, size_y),
                    ImVec2(start, 0.f),
                    ImVec2(end, 1.f)
                );
            }
        }

        {
            GLuint texture;
            int size_x;
            int size_y;
            ImGui::Text("resources_small.dds");
            if (layers.get_resources_texture_small(texture, size_x, size_y)) {
                ImGui::Image(
                    texture,
                    ImVec2(size_x * width, size_y),
                    ImVec2(start, 0.f),
                    ImVec2(end, 1.f)
                );
            }
        }

        if (ImGui::Button("Choose a new image")) {

            if (
                !active_layer.resources_big.valid()
                || !active_layer.resources_medium.valid()
                || !active_layer.resources_small.valid()
            ) {
                layers.copy_resources_to_current_layer();
            }

            auto result = open_image_selection_dialog(winapi::UUID_open_trade_goods_icon);

            if (result.empty()) {
                return;
            }

            int size_x;
            int size_y;
            int channels;

            auto new_image = SOIL_load_image(
                (wstring_to_utf8(result)).c_str(),
                &size_x,
                &size_y,
                &channels,
                SOIL_LOAD_AUTO
            );

            if (size_x < width) {
                return;
            }
            if (size_y < active_layer.resources_big.size_y) {
                return;
            }
            compress_image_to_commodity_strip(
                active_layer.resources_big, count, commodity->index,
                new_image, size_x, size_y, channels
            );
            compress_image_to_commodity_strip(
                active_layer.resources_medium, count, commodity->index,
                new_image, size_x, size_y, channels
            );
            compress_image_to_commodity_strip(
                active_layer.resources_small, count, commodity->index,
                new_image, size_x, size_y, channels
            );
        }
    }

    void commodity_widget(state::layers_stack& layers, std::string commodity_name) {
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("GoodsTabs", tab_bar_flags)) {
            if (ImGui::BeginTabItem("Definition")) {
                commodity_widget_definition(layers, commodity_name);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Icons")) {
                commodity_widget_icon(layers, commodity_name);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

    bool flag_widget(state::layers_stack& layers, assets::storage& storage, std::string flag_key, std::string& flag_path_from_layer) {
        state::layer& active_layer = layers.data[layers.current_layer_index];

        bool flag_found = false;
        for (int i = layers.current_layer_index; i >= 0; i--) {
            auto& layer = layers.data[i];
            auto flag_path = layer.path + flag_path_from_layer;
            if (layer.paths_to_new_flags.contains(flag_key)) {
                flag_path = wstring_to_utf8(layer.paths_to_new_flags[flag_key]);
            }
            if (storage.filename_to_texture_asset.contains(flag_path)) {
                if (layer.paths_to_new_flags.contains(flag_key)) {
                    auto path = layer.paths_to_new_flags[flag_key];
                    ImGui::Text("%s", wstring_to_utf8(path).c_str());
                } else {
                    ImGui::Text("%s", (layer.path + flag_path_from_layer).c_str());
                }

                auto asset = storage.filename_to_texture_asset[flag_path];
                ImGui::Image((ImTextureID)(intptr_t)asset.texture, ImVec2(asset.w, asset.h));
                return true;
            } else {
                //check if path really exists:
                if (!std::filesystem::exists(flag_path)) {
                    continue;
                }
                flag_found = true;

                int size_x;
                int size_y;
                int channels;

                auto flag_data = SOIL_load_image(
                    (flag_path).c_str(),
                    &size_x,
                    &size_y,
                    &channels,
                    4
                );

                std::cout << "load flag " << size_x << " " << size_y << "\n";

                // Create a OpenGL texture identifier
                GLuint image_texture;
                glGenTextures(1, &image_texture);
                glBindTexture(GL_TEXTURE_2D, image_texture);

                // Setup filtering parameters for display
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                // Upload pixels into texture
                glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size_x, size_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, flag_data);

                assets::asset flag {
                    .texture = image_texture,
                    .w = size_x, .h = size_y
                };

                storage.filename_to_texture_asset[flag_path] = flag;
            }
        }
        return flag_found;
    }

    void selection_province_history(state::layers_stack& map, state::control& control, state::editor& editor) {
        auto can_edit = map.can_edit_province_history(control.selected_province_id);

        if (control.selected_province_id > 0) {
            auto history = map.get_province_history(control.selected_province_id);
            auto def = map.get_province_definition(control.selected_province_id);
            auto v2id = control.selected_province_id;

            if (def != nullptr) {
                if (ImGui::Button("Recalculate local adjacencies")) {
                    control.local_adjacency_center = map.indices.v2id_to_mean[v2id];
                    control.local_adjacency.clear();

                    std::vector<uint32_t> adj_colors{};
                    map.populate_adjacent_colors(state::rgb_to_uint(def->r, def->g, def->b), adj_colors);

                    for (auto & rgb : adj_colors) {
                        auto adj_v2id = map.rgb_to_v2id(rgb);

                        if(adj_v2id.has_value()) {
                            auto mean = map.indices.v2id_to_mean[adj_v2id.value()];
                            control.local_adjacency.push_back(mean);
                        }
                    }
                }
            }

            if (def == nullptr) {
                ImGui::Text("Province without definition");
                return;
            }

            if (history == nullptr) {
                ImGui::Text("Province without history");
                if (ImGui::Button("Set up history")) {
                    map.copy_province_history_to_current_layer(v2id);
                }
                return;
            }

            if (!can_edit) {

                if (ImGui::Button("Copy province data to active mod layer")) {
                    map.copy_province_history_to_current_layer(v2id);
                }

                ImGui::BeginDisabled();
            }

            ImGui::Text("%s", ((def->name) + conversions::wstring_to_utf8(L" (" + history->history_file_name + L") " + std::to_wstring(def->v2id))).c_str());

            if (ImGui::InputText("Owner", &history->owner_tag)) {
                map.province_owner[3 * v2id + 0] = history->owner_tag[0];
                map.province_owner[3 * v2id + 1] = history->owner_tag[1];
                map.province_owner[3 * v2id + 2] = history->owner_tag[2];
                map.commit_owner_texture_to_gpu();
            }

            ImGui::SameLine();
            if (ImGui::Button("Clear owner")) {
                history->owner_tag = "";
                map.province_owner[3 * v2id + 0] = 0;
                map.province_owner[3 * v2id + 1] = 0;
                map.province_owner[3 * v2id + 2] = 0;
                map.commit_owner_texture_to_gpu();
            }

            ImGui::InputText("Controller", &history->controller_tag);
            ImGui::SameLine();
            if (ImGui::Button("Clear control")) {
                history->controller_tag = "";
            }

            ImGui::InputText("Main RGO: ", &history->main_trade_good);

            if (ImGui::TreeNode("Secondary RGO")) {

                std::vector<std::string> template_names = {};
                for (auto const& [key, val] : editor.secondary_rgo_templates) {
                    template_names.push_back(key);
                }

                ImGui::Text("Apply template");

                ImGui::SameLine();

                if (ImGui::BeginCombo("Select", "")) {
                    for (int n = 0; n < template_names.size(); n++) {
                        if (ImGui::Selectable(template_names[n].c_str(), false)) {
                            history->secondary_rgo_size.clear();
                            for (auto const& [key, val] : editor.secondary_rgo_templates[template_names[n]]) {
                                history->secondary_rgo_size[key] = val;
                            }
                        }
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::BeginTable("adj", 3)) {

                    ImGui::TableSetupColumn("Commodity");
                    ImGui::TableSetupColumn("Max employment");
                    ImGui::TableSetupColumn("Delete rgo");

                    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
                    for (int column = 0; column < 3; column++)
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

                    std::vector<std::string> local_rgo;

                    for (auto const& [key, val] : history->secondary_rgo_size) {
                        local_rgo.push_back(key);
                    }

                    for (int row = 0; row < local_rgo.size(); row++)
                    {
                        auto size = history->secondary_rgo_size[local_rgo[row]];
                        auto new_size = size;
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::PushID(row);
                        ImGui::Text("%s", local_rgo[row].c_str());
                        ImGui::TableNextColumn();
                        ImGui::InputInt("", &new_size);
                        if (new_size != size) {
                            history->secondary_rgo_size[local_rgo[row]] = new_size;
                        }
                        ImGui::TableNextColumn();
                        if (ImGui::Button("Delete")) {
                            history->secondary_rgo_size.erase(local_rgo[row]);
                        }
                        ImGui::PopID();
                    }

                    ImGui::TableNextRow();
                    static std::string new_secondary_rgo_entry;
                    ImGui::TableNextColumn();
                    ImGui::InputText("Commodity", &new_secondary_rgo_entry);
                    ImGui::TableNextColumn();
                    if (!history->secondary_rgo_size.contains(new_secondary_rgo_entry) && new_secondary_rgo_entry.length() > 0) {
                        if (ImGui::Button("Insert")) {
                            history->secondary_rgo_size[new_secondary_rgo_entry] = 0;
                        }
                    }
                    ImGui::TableNextColumn();

                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }

            ImGui::InputInt("Railroad: ", &history->railroad);
            ImGui::InputInt("Naval base: ", &history->naval_base);
            ImGui::InputInt("Fort: ", &history->fort);

            ImGui::InputInt("Colonial: ", &history->colonial);
            ImGui::InputInt("Colony: ", &history->colony);

            ImGui::InputText("Terrain", &history->terrain);

            bool remove_flag = false;
            int remove_index = 0;
            for (int i = 0; i < history->buildings.size(); i++) {
                ImGui::PushID(i);
                if (ImGui::TreeNode("Building")) {
                    ImGui::InputInt("Level", &history->buildings[i].level);
                    ImGui::InputText("Type", &history->buildings[i].building_type);
                    ImGui::InputText("Upgrade", &history->buildings[i].upgrade);
                    if (ImGui::Button("Remove")) {
                        remove_flag = true;
                        remove_index = i;
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            if (remove_flag) {
                history->buildings.erase(history->buildings.begin() + remove_index);
            }
            if (ImGui::Button("Add building")) {
                game_definition::state_building bdef {
                    .level = 1, .building_type = "?", .upgrade = "yes"
                };
                history->buildings.push_back(bdef);
            }

            for (int i = 0; i < history->cores.size(); i++) {
                ImGui::PushID(i);
                ImGui::InputText("Core", &(history->cores[i]));
                ImGui::SameLine();
                if (ImGui::Button("Clear core")) {
                    history->cores[i] = "";
                }
                ImGui::PopID();
            }

            if (ImGui::Button("Add core")) {
                history->cores.push_back("");
            }

            if (!can_edit) {
                ImGui::EndDisabled();
            }
        }
    }

    void pops_buffer_widget(state::layers_stack& layers, state::control& control);

    void province_population_widget(state::layers_stack& layers, state::control& control, uint32_t v2id) {
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        auto dates = layers.get_available_dates();
        auto poptypes = layers.retrieve_poptypes();
        auto cultures = layers.retrieve_cultures();
        auto religions = layers.retrieve_religions();

        if (ImGui::Button("Add province to split buffer")) {
            // check if it is already there:
            bool already_there = false;
            for (auto& target : control.province_targets_for_pop_splitting) {
                if (target.v2id == v2id) {
                    already_there = true;
                }
            }
            if (!already_there) {
                state::split_target_province target {
                    v2id, 0.f
                };
                control.province_targets_for_pop_splitting_indices.push_back(
                    control.province_targets_for_pop_splitting.size()
                );
                control.province_targets_for_pop_splitting.push_back(target);
            }
        }

        if (ImGui::BeginTabBar("ProvincePopulationTabs", tab_bar_flags)) {
            for (auto d : dates) {
                auto year = d / (31*12);
                auto year_s = std::to_string(year);
                if (ImGui::BeginTabItem(year_s.c_str())) {
                    auto pops = layers.get_pops(v2id, d);


                    if (pops == nullptr) {
                        ImGui::Text("No pops defined for this province");

                        if (ImGui::Button("Create pops table for the province")) {
                            layers.create_new_population_list(v2id, d);
                        }

                        ImGui::EndTabItem();
                        continue;
                    }

                    int total_pop = 0;
                    for (auto& pop : *pops) {
                        total_pop += pop.size;
                    }

                    bool can_edit = layers.can_edit_pops(v2id, d);

                    if (!can_edit) {
                        ImGui::Text("Pops are defined on a layer below, so you can't edit them");
                        if (ImGui::Button("Create empty pops table for the province")) {
                            layers.create_new_population_list(v2id, d);
                        }
                        if (ImGui::Button("Copy pops table from the layer below")) {
                            layers.copy_pops_data_to_current_layer(v2id, d);
                        }
                        ImGui::BeginDisabled();
                    }

                    static std::vector<size_t> pops_indices;
                    static uint32_t selected_province = 0;

                    if (ImGui::Button("Add empty pop")) {
                        game_definition::pop_history item {};
                        pops->push_back(item);
                        selected_province = 0;
                    }

                    ImGui::Text("%zu pops are selected", control.selected_pops.size());

                    if (ImGui::Button("Deselect all pops")) {
                        control.selected_pops.clear();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Copy selected to the buffer")) {
                        control.pop_buffer.clear();
                        for (size_t i = 0; i < control.selected_pops.size(); i++) {
                            auto& selection = control.selected_pops[i];
                            auto pops = layers.get_pops(selection.v2id, d);
                            if (pops == nullptr) {
                                continue;
                            }
                            if (pops->size() <= selection.index) {
                                continue;
                            }
                            auto& pop = pops->data()[selection.index];
                            control.pop_buffer_indices.push_back(control.pop_buffer.size());
                            control.pop_buffer.push_back(pop);
                        }
                    }

                    // ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(125, 0, 125, 255));
                    if (ImGui::Button("Move selected to the buffer (!CAUTION!)")) {
                        control.pop_buffer.clear();
                        for (size_t i = 0; i < control.selected_pops.size(); i++) {
                            auto& selection = control.selected_pops[i];
                            auto pops = layers.get_pops(selection.v2id, d);
                            if (pops == nullptr) {
                                continue;
                            }
                            if (pops->size() <= selection.index) {
                                continue;
                            }
                            auto& pop = pops->data()[selection.index];
                            control.pop_buffer_indices.push_back(control.pop_buffer.size());
                            control.pop_buffer.push_back(pop);
                        }

                        std::stable_sort(control.selected_pops.begin(), control.selected_pops.end(), [&](auto& a, auto& b) {
                            return a.index < b.index;
                        });
                        for (int i = control.selected_pops.size() - 1; i >= 0; i--) {
                            auto& selection = control.selected_pops[i];
                            auto pops = layers.get_pops(selection.v2id, d);
                            if (pops == nullptr) {
                                continue;
                            }
                            if (pops->size() <= selection.index) {
                                continue;
                            }
                            pops->erase(pops->begin() + selection.index);
                        }
                        control.selected_pops.clear();
                        selected_province = 0;
                    }
                    ImGui::PopStyleColor();

                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(125, 0, 0, 255));
                    if (ImGui::Button("Delete selected (!CAUTION!)")) {
                        control.pop_buffer.clear();
                        std::stable_sort(control.selected_pops.begin(), control.selected_pops.end(), [&](auto& a, auto& b) {
                            return a.index < b.index;
                        });
                        for (int i = control.selected_pops.size() - 1; i >= 0; i--) {
                            auto& selection = control.selected_pops[i];
                            auto pops = layers.get_pops(selection.v2id, d);
                            if (pops == nullptr) {
                                continue;
                            }
                            if (pops->size() <= selection.index) {
                                continue;
                            }
                            pops->erase(pops->begin() + selection.index);
                        }
                        control.selected_pops.clear();
                        selected_province = 0;
                    }
                    ImGui::PopStyleColor();

                    if (ImGui::Button("Paste copied pops")) {
                        for (size_t i = 0; i < control.pop_buffer.size(); i++) {
                            pops->push_back(control.pop_buffer[i]);
                        }
                        selected_province = 0;
                    }

                    if (selected_province != v2id) {
                        pops_indices.clear();
                        for (auto i = 0; i < pops->size(); i++) {
                            pops_indices.push_back(i);
                        }
                        selected_province = v2id;
                    }

                    static ImGuiTableFlags flags =
                        ImGuiTableFlags_Reorderable
                        | ImGuiTableFlags_Hideable
                        | ImGuiTableFlags_Sortable
                        | ImGuiTableFlags_SortMulti
                        | ImGuiTableFlags_RowBg
                        | ImGuiTableFlags_BordersOuter
                        | ImGuiTableFlags_BordersV
                        | ImGuiTableFlags_NoBordersInBody
                        | ImGuiTableFlags_ScrollY;

                    ImGuiStyle& style = ImGui::GetStyle();

                    if (
                        ImGui::BeginTable(
                            "table_population",
                            7,
                            flags,
                            ImVec2(0.0f, 500),
                            0.0f
                        )
                    ) {
                        // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));

                        ImGui::TableSetupColumn(
                            "Religion",
                            ImGuiTableColumnFlags_DefaultSort
                            | ImGuiTableColumnFlags_WidthFixed,
                            80.0f,
                            province_population_religion
                        );
                        ImGui::TableSetupColumn(
                            "Culture",
                            ImGuiTableColumnFlags_DefaultSort
                            | ImGuiTableColumnFlags_WidthFixed,
                            80.0f,
                            province_population_culture
                        );
                        ImGui::TableSetupColumn(
                            "Poptype",
                            ImGuiTableColumnFlags_DefaultSort
                            | ImGuiTableColumnFlags_WidthFixed,
                            80.0f,
                            province_population_poptype
                        );
                        ImGui::TableSetupColumn(
                            "Size",
                            ImGuiTableColumnFlags_DefaultSort
                            | ImGuiTableColumnFlags_WidthFixed,
                            200.0f,
                            province_population_size
                        );
                        ImGui::TableSetupColumn(
                            "Militancy",
                            ImGuiTableColumnFlags_DefaultSort
                            | ImGuiTableColumnFlags_WidthFixed,
                            60.0f,
                            province_population_militancy
                        );
                        ImGui::TableSetupColumn(
                            "Rebel",
                            ImGuiTableColumnFlags_DefaultSort
                            | ImGuiTableColumnFlags_WidthFixed,
                            50.0f,
                            province_population_rebel
                        );
                        ImGui::TableSetupColumn(
                            "Select##province_selection_header",
                            ImGuiTableColumnFlags_NoSort
                            | ImGuiTableColumnFlags_WidthFixed,
                            50.0f,
                            province_population_copy
                        );

                        ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
                        ImGui::TableHeadersRow();

                        // Sort our data if sort specs have been changed!
                        if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
                            if (sort_specs->SpecsDirty) {
                                std::sort(pops_indices.begin(), pops_indices.end(), [&](size_t a, size_t b) {
                                    auto& a_def = pops->data()[a];
                                    auto& b_def = pops->data()[b];

                                    for (int n = 0; n < sort_specs->SpecsCount; n++)
                                    {
                                        const ImGuiTableColumnSortSpecs* sort_spec = &sort_specs->Specs[n];
                                        int order = 0;
                                        switch (sort_spec->ColumnUserID)
                                        {
                                        case province_population_culture:
                                            order = a_def.culture.compare(b_def.culture);
                                            break;
                                        case province_population_religion:
                                            order = a_def.religion.compare(b_def.religion);
                                            break;
                                        case province_population_poptype:
                                            order = a_def.poptype.compare(b_def.poptype);
                                            break;
                                        case province_population_rebel:
                                            order = a_def.poptype.compare(b_def.poptype);
                                            break;
                                        case province_population_size:
                                            order = a_def.size - b_def.size;
                                            break;
                                        case province_population_militancy:
                                            order = a_def.militancy - b_def.militancy;
                                            break;
                                        default: IM_ASSERT(0); break;
                                        }
                                        if (order > 0)
                                            return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? true : false;
                                        if (order < 0)
                                            return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? false : true;
                                    }
                                    return a > b;
                                });
                                sort_specs->SpecsDirty = false;
                            }

                        ImGuiListClipper clipper;
                        clipper.Begin(pops_indices.size());
                        while (clipper.Step())
                            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                                // Display a data item
                                auto pop_index = pops_indices[row_n];
                                auto& pop = pops->data()[pop_index];



                                ImGui::PushID(row_n);
                                ImGui::TableNextRow();

                                ImGui::TableNextColumn();
                                ImGui::SetNextItemWidth(80.f);
                                if (ImGui::BeginCombo("##religion", pop.religion.c_str())) {
                                    static ImGuiTextFilter filter;
                                    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
                                    filter.Draw("##Filter", -FLT_MIN);
                                    if (ImGui::IsWindowAppearing())
                                        ImGui::SetKeyboardFocusHere(-1);
                                    for (int n = 0; n < religions.size(); n++)
                                    {
                                        const bool is_selected = (pop.religion == religions[n]);
                                        if (filter.PassFilter(religions[n].c_str()))
                                            if (ImGui::Selectable(religions[n].c_str(), is_selected))
                                                pop.religion = religions[n];
                                    }
                                    ImGui::EndCombo();
                                }

                                ImGui::TableNextColumn();
                                ImGui::SetNextItemWidth(80.f);

                                if (ImGui::BeginCombo("##culture", pop.culture.c_str())) {
                                    static ImGuiTextFilter filter;
                                    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
                                    filter.Draw("##Filter", -FLT_MIN);
                                    if (ImGui::IsWindowAppearing())
                                        ImGui::SetKeyboardFocusHere(-1);

                                    for (int n = 0; n < cultures.size(); n++)
                                    {
                                        const bool is_selected = (pop.culture == cultures[n]);
                                        if (filter.PassFilter(cultures[n].c_str()))
                                            if (ImGui::Selectable(cultures[n].c_str(), is_selected))
                                                pop.culture = cultures[n];
                                    }
                                    ImGui::EndCombo();
                                }

                                ImGui::TableNextColumn();
                                ImGui::SetNextItemWidth(80.f);
                                if (ImGui::BeginCombo("", pop.poptype.c_str())) {
                                    for (int n = 0; n < poptypes.size(); n++) {
                                        if (ImGui::Selectable(poptypes[n].c_str(), poptypes[n] == pop.poptype)) {
                                            pop.poptype = poptypes[n];
                                        }
                                    }
                                    ImGui::EndCombo();
                                }

                                ImGui::TableNextColumn();
                                ImGui::SetNextItemWidth(100.f);
                                ImGui::InputInt("##size", &pop.size, 0);

                                ImGui::SameLine();
                                auto ratio = (float) pop.size / (float) total_pop;
                                if (total_pop == 0) {
                                    ratio = 0.f;
                                }
                                auto old_ratio = ratio;
                                ImGui::SetNextItemWidth(100.f);
                                ImGui::SliderFloat("##size_drag", &ratio, 0.f, 1.f);

                                if (ratio != old_ratio) {
                                    auto new_size = (int) (total_pop * ratio);
                                    auto remainder = total_pop - new_size;
                                    auto old_remainder = total_pop - pop.size;
                                    auto scaler = (float)remainder / (float)old_remainder;

                                    pop.size = new_size;

                                    // scale everything else down:
                                    for (size_t index = 0; index < pops->size(); index++) {
                                        if (index == pops_indices[row_n]) {

                                            continue;
                                        }
                                        auto& target_pop = pops->data()[index];
                                        target_pop.size *= scaler;
                                    }
                                }

                                ImGui::TableNextColumn();
                                ImGui::SetNextItemWidth(60.f);
                                ImGui::InputFloat("##militancy", &pop.militancy);

                                ImGui::TableNextColumn();
                                ImGui::Text("%s", pop.rebel_type.c_str());

                                ImGui::TableNextColumn();

                                bool selected_pop = false;
                                size_t selection_index = 0;

                                for (auto i = 0; i < control.selected_pops.size(); i++) {
                                    auto& candidate = control.selected_pops[i];
                                    if (
                                        candidate.v2id == selected_province
                                        && candidate.index == pops_indices[row_n]
                                    ) {
                                        selected_pop = true;
                                        selection_index = i;
                                        break;
                                    }
                                }
                                if (ImGui::RadioButton("##selected_pop", selected_pop)) {
                                    if (!selected_pop) {
                                        state::selected_pop new_selection {
                                            selected_province, pops_indices[row_n]
                                        };
                                        control.selected_pops.push_back(new_selection);
                                    } else {
                                        control.selected_pops.erase(control.selected_pops.begin() + selection_index);
                                    }
                                }
                                ImGui::PopID();
                            }

                        // ImGui::PopStyleVar();
                        ImGui::EndTable();
                    }

                    if (!can_edit) {
                        ImGui::EndDisabled();
                    }

                    ImGui::EndTabItem();
                }
            }
        }
        ImGui::EndTabBar();
    }

    void province_widget(state::layers_stack& layers, state::control& control, state::editor& editor) {
        ImGui::Text("v2id: %s", std::to_string(control.selected_province_id).c_str());
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("ProvinceTabs", tab_bar_flags)) {
            if (ImGui::BeginTabItem("Definition")) {
                selection_province_history(layers, control, editor);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Population")) {
                province_population_widget(layers, control, control.selected_province_id);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

    void selection_nation(state::layers_stack& map, state::control& control, assets::storage& storage, int32_t tag) {
        if (tag == 0) {
            ImGui::Text("No nation");
            return;
        }

        const auto & def = map.get_nation_definition(tag);
        std::string text_tag {
            (def->tag[0]),
            (def->tag[1]),
            (def->tag[2])
        };
        ImGui::Text("%s", (text_tag).c_str());

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("NationTabs", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Color"))
            {
                bool can_edit_common = map.can_edit_nation_common(tag);
                if (can_edit_common) {
                    auto common = map.get_nation_common(tag);
                    float colour[3] = {
                    float(common->R) / 255.f,
                    float(common->G) / 255.f,
                    float(common->B) / 255.f
                    };
                    ImGui::ColorPicker3("National Colour", colour);
                    common->R = uint8_t(colour[0] * 255.f);
                    common->G = uint8_t(colour[1] * 255.f);
                    common->B = uint8_t(colour[2] * 255.f);
                } else {
                    ImGui::Text("%s", "This nation has no /common/countries file in mod folders.");
                    if (ImGui::Button("Copy data from the base game")) {
                        map.copy_nation_common_to_current_layer(tag);
                    }
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Parties"))
            {
                bool can_edit_common = map.can_edit_nation_common(tag);
                if (can_edit_common) {
                    auto common = map.get_nation_common(tag);
                    bool remove_flag = false;
                    int remove_index = 0;
                    for (int i = 0; i < common->parties.size(); i++) {
                        ImGui::PushID(i);
                        std::string label = "Party(" + common->parties[i].ideology + ")";
                        if (ImGui::TreeNode(label.c_str())) {
                            ImGui::InputText("Name", &common->parties[i].name);
                            ImGui::InputText("Start date", &common->parties[i].start);
                            ImGui::InputText("End date", &common->parties[i].end);
                            ImGui::InputText("Ideology", &common->parties[i].ideology);
                            ImGui::InputText("Economic Policy", &common->parties[i].economic_policy);
                            ImGui::InputText("Trade Policy", &common->parties[i].trade_policy);
                            ImGui::InputText("Religious Policy", &common->parties[i].religious_policy);
                            ImGui::InputText("Citizenship Policy", &common->parties[i].citizenship_policy);
                            ImGui::InputText("War policy", &common->parties[i].war_policy);

                            if (ImGui::Button("Remove")) {
                                remove_flag = true;
                                remove_index = i;
                            }
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                    if (remove_flag) {
                        common->parties.erase(common->parties.begin() + remove_index);
                    }
                    if (ImGui::Button("Add party")) {
                        game_definition::party bdef {};
                        common->parties.push_back(bdef);
                    }
                } else {
                    ImGui::Text("%s", "This nation has no /common/countries file in mod folders.");
                    if (ImGui::Button("Copy data from the base game")) {
                        map.copy_nation_common_to_current_layer(tag);
                    }
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Misc.commons"))
            {
                bool can_edit_common = map.can_edit_nation_common(tag);
                if(can_edit_common) {
                    auto common = map.get_nation_common(tag);
                    ImGui::InputText("Graphical culture", &common->graphical_culture);
                    ImGui::Text("Edit unit names manually, sorry.");
                    //ImGui::TextUnformatted("%s", common->unit_names.c_str());
                } else {
                    ImGui::Text("%s", "This nation has no /common/countries file in mod folders.");
                    if (ImGui::Button("Copy data from the base game")) {
                        map.copy_nation_common_to_current_layer(tag);
                    }
                }

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("History"))
            {
                bool can_edit_history = map.can_edit_nation_history(tag);
                auto history = map.get_nation_history(tag);

                ImGuiTabBarFlags history_tab_bar_flags = ImGuiTabBarFlags_None;
                if (ImGui::BeginTabBar("NationHistoryTabs", tab_bar_flags))
                {
                    if (ImGui::BeginTabItem("Main"))
                    {
                        if (!can_edit_history) {
                            if (ImGui::Button("Copy history from the base game.")) {
                                map.copy_nation_history_to_current_layer(tag);
                            }
                            ImGui::BeginDisabled();
                        }
                        ImGui::InputInt("Capital", &history->capital);
                        if (history->capital == -1) {
                            ImGui::Text("Undefined");
                        } else {
                            auto prov = map.get_province_definition(history->capital);
                            if (prov != nullptr) {
                                ImGui::Text("%s", (prov->name).c_str());
                            } else {
                                ImGui::Text("Invalid id");
                            }
                        }
                        ImGui::InputText("Primary culture", &history->primary_culture);
                        for (int i = 0; i < history->culture.size(); i++) {
                            ImGui::PushID(i);
                            ImGui::InputText("Culture", &(history->culture[i]));
                            ImGui::SameLine();
                            if (ImGui::Button("Clear culture")) {
                                history->culture[i] = "";
                            }
                            ImGui::PopID();
                        }
                        if (ImGui::Button("Add culture")) {
                            history->culture.push_back("");
                        }
                        ImGui::InputText("Religion", &history->religion);
                        ImGui::Checkbox("Civilized", &history->civilized);
                        ImGui::Checkbox("Releasable", &history->is_releasable_vassal);
                        ImGui::InputText("Government", &history->government);
                        ImGui::InputText("National value", &history->nationalvalue);
                        ImGui::InputFloat("Plurality", &history->plurality);
                        ImGui::InputFloat("Prestige", &history->prestige);
                        ImGui::InputFloat("Literacy", &history->literacy);
                        ImGui::InputFloat("Literacy(non-state)", &history->non_state_culture_literacy);
                        ImGui::InputFloat("Consciousness", &history->consciousness);
                        ImGui::InputFloat("Consciousness(non-state)", &history->nonstate_consciousness);

                        if (!can_edit_history)
                            ImGui::EndDisabled();
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Technologies"))
                    {
                        if (!can_edit_history) {
                            if (ImGui::Button("Copy history from the base game.")) {
                                map.copy_nation_history_to_current_layer(tag);
                            }
                        }
                        ImGuiTabBarFlags technology_history_tab_bar_flags = ImGuiTabBarFlags_None;
                        std::vector<std::wstring> folders{};
                        map.retrieve_tech_folders(folders);
                        if (ImGui::BeginTabBar("TechnologyNationHistoryTabs", tab_bar_flags)) {
                            for (auto& folder : folders) {
                                if (ImGui::BeginTabItem(conversions::wstring_to_utf8(folder).c_str())) {
                                    technology_folder(
                                        map,
                                        history,
                                        can_edit_history,
                                        folder
                                    );
                                    ImGui::EndTabItem();
                                }
                            }
                            ImGui::EndTabBar();
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Inventions"))
                    {
                        if (!can_edit_history) {
                            if (ImGui::Button("Copy history from the base game.")) {
                                map.copy_nation_history_to_current_layer(tag);
                            }
                        }
                        std::vector<std::wstring> folders{};
                        map.retrieve_inventions_folders(folders);
                        ImGuiTabBarFlags inventions_history_tab_bar_flags = ImGuiTabBarFlags_None;
                        if (ImGui::BeginTabBar("InventionsNationHistoryTabs", tab_bar_flags)) {
                            for (auto& folder : folders) {
                                if (ImGui::BeginTabItem(conversions::wstring_to_utf8(folder).c_str())) {
                                    inventions_folder(
                                        map,
                                        history,
                                        can_edit_history,
                                        folder
                                    );
                                    ImGui::EndTabItem();
                                }
                            }
                            ImGui::EndTabBar();
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Issues"))
                    {
                        if (!can_edit_history) {
                            if (ImGui::Button("Copy history from the base game.")) {
                                map.copy_nation_history_to_current_layer(tag);
                            }
                        }
                        if (!can_edit_history)
                            ImGui::BeginDisabled();

                        issues_selection(map, history, can_edit_history);

                        if (!can_edit_history)
                            ImGui::EndDisabled();
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Flags"))
            {

                ImGui::Text("Flags");
                ImGui::Text("Default flag");
                std::string string_tag {(char)def->tag[0], (char)def->tag[1], (char)def->tag[2]};
                std::string default_flag_path = "/gfx/flags/" + string_tag + ".tga";
                std::string default_flag_path_png = "/gfx/flags/" + string_tag + ".png";

                auto& active_layer = map.data[map.current_layer_index];

                if (active_layer.paths_to_new_flags.contains(string_tag)) {
                    if (ImGui::Button("Copy default flag to other positions")) {
                        auto flags = map.get_flags();
                        if (flags != nullptr){
                            for(auto& flagtype : *flags) {
                                std::string flag_key = string_tag + + "_" + flagtype;
                                active_layer.paths_to_new_flags[flag_key] = active_layer.paths_to_new_flags[string_tag];
                            }
                        }
                    }
                }

                ImGui::PushID(0);
                if (ImGui::Button("Choose a new flag")) {
                    auto result = open_image_selection_dialog(winapi::UUID_open_flags);
                    active_layer.paths_to_new_flags[string_tag] = result;
                }
                if (!flag_widget(map, storage, string_tag, default_flag_path_png)) {
                    flag_widget(map, storage, string_tag, default_flag_path);
                }
                ImGui::PopID();
                auto flags = map.get_flags();
                if (flags != nullptr){
                    int counter = 1;
                    for(auto& flagtype : *flags) {
                        ImGui::PushID(counter);
                        ImGui::Text("%s", flagtype.c_str());
                        std::string flag_key = string_tag + + "_" + flagtype;
                        std::string flag_path = "/gfx/flags/" + flag_key + ".tga";
                        std::string flag_path_png = "/gfx/flags/" + flag_key + ".png";
                        if (ImGui::Button("Choose a new flag")) {
                            auto result = open_image_selection_dialog(winapi::UUID_open_flags);
                            active_layer.paths_to_new_flags[string_tag] = result;
                        }
                        if (!flag_widget(map, storage, flag_key, flag_path_png)) {
                            flag_widget(map, storage, flag_key, flag_path);
                        }
                        counter++;
                        ImGui::PopID();
                    }
                }

                ImGui::Text("Overrides");

                auto history = map.get_nation_history(tag);
                if (history != nullptr){
                    bool can_edit = map.can_edit_nation_history(tag);
                    if (!can_edit)
                        ImGui::BeginDisabled();

                    auto remove_flag = false;
                    auto remove_index = -1;
                    for (int i = 0; i < history->govt_flag.size(); i++) {
                        ImGui::PushID(i);
                        std::string label = "Flag(" + history->govt_flag[i].government + ")";
                        if (ImGui::TreeNode(label.c_str())) {
                            ImGui::InputText("Government", &history->govt_flag[i].government);
                            ImGui::InputText("Flag", &history->govt_flag[i].flag);

                            if (ImGui::Button("Remove")) {
                                remove_flag = true;
                                remove_index = i;
                            }
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                    if (remove_flag) {
                        history->govt_flag.erase(history->govt_flag.begin() + remove_index);
                    }
                    if (ImGui::Button("Add flag override")) {
                        history->govt_flag.emplace_back();
                    }


                    if (!can_edit)
                        ImGui::EndDisabled();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

    void selection(state::layers_stack& map, state::control& control, state::editor& editor, assets::storage& storage) {
        ImGui::Begin(
            "Selection",
            NULL,
            ImGuiWindowFlags_NoFocusOnAppearing
        );

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("SelectionTabs", tab_bar_flags)) {
            if (ImGui::BeginTabItem("Province")) {
                province_widget(map, control, editor);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Pops splitter")) {
                pops_buffer_widget(map, control);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Nation")) {
                if (control.selected_tag.length() == 0) {
                    widgets::selection_nation(map, control, storage, 0);
                } else {
                    auto id = game_definition::tag_to_int({
                        control.selected_tag[0],
                        control.selected_tag[1],
                        control.selected_tag[2]
                    });
                    widgets::selection_nation(map, control, storage, id);
                }
                ImGui::EndTabItem();
            }
            if (control.selected_commodity.size() != 0) {
                if (ImGui::BeginTabItem("Commodity")) {
                    widgets::commodity_widget(map, control.selected_commodity);
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }


        ImGui::End();
    }
}