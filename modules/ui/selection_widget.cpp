#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
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

    std::wstring open_image_selection_dialog() {
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

    bool flag_widget(state::layers_stack& layers, assets::storage& storage, std::string flag_key, std::string& flag_path_from_layer) {
        state::layer& active_layer = layers.data[layers.current_layer_index];
        if (ImGui::Button("Choose a new flag")) {
            auto result = open_image_selection_dialog();
            active_layer.paths_to_new_flags[flag_key] = result;
        }
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

    void selection_province(state::layers_stack& map, state::control& control, state::editor& editor) {
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

            if (history == nullptr || def == nullptr) {
                ImGui::Text("Sea province or area without definition");
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
                selection_province(map, control, editor);
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
            ImGui::EndTabBar();
        }


        ImGui::End();
    }
}