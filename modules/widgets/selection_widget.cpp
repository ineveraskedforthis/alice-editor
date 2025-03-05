#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "selection_widget.hpp"


namespace widgets {
    void selection_province(parsing::game_map& map, state::control& control) {
        ImGui::Begin(
            "Selection",
            NULL,
            ImGuiWindowFlags_NoFocusOnAppearing
        );


        auto & def = map.provinces[map.index_to_vector_position[control.selected_province_id]];

        ImGui::Text("%s", (def.name + " (" + def.history_file_name + ") " + std::to_string(def.v2id)).c_str());

        if (ImGui::InputText("Owner", &def.owner_tag)) {
            map.province_owner[3 * def.v2id + 0] = def.owner_tag[0];
            map.province_owner[3 * def.v2id + 1] = def.owner_tag[1];
            map.province_owner[3 * def.v2id + 2] = def.owner_tag[2];
            state::update_map_texture(control, map);
        }

        ImGui::SameLine();
        if (ImGui::Button("Clear owner")) {
            def.owner_tag = "";
            map.province_owner[3 * def.v2id + 0] = 0;
            map.province_owner[3 * def.v2id + 1] = 0;
            map.province_owner[3 * def.v2id + 2] = 0;
            state::update_map_texture(control, map);
        }

        ImGui::InputText("Controller", &def.controller_tag);
        ImGui::SameLine();
        if (ImGui::Button("Clear control")) {
            def.controller_tag = "";
        }

        ImGui::InputText("Main RGO: ", &def.main_trade_good);

        if (ImGui::TreeNode("Secondary RGO")) {

            std::vector<std::string> template_names = {};
            for (auto const& [key, val] : map.secondary_rgo_templates) {
                template_names.push_back(key);
            }

            ImGui::Text("Apply template");

            ImGui::SameLine();

            if (ImGui::BeginCombo("Select", "")) {
                for (int n = 0; n < template_names.size(); n++) {
                    if (ImGui::Selectable(template_names[n].c_str(), false)) {
                        def.secondary_rgo_size.clear();
                        for (auto const& [key, val] : map.secondary_rgo_templates[template_names[n]]) {
                            def.secondary_rgo_size[key] = val;
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

                for (auto const& [key, val] : def.secondary_rgo_size) {
                    local_rgo.push_back(key);
                }

                for (int row = 0; row < local_rgo.size(); row++)
                {
                    auto size = def.secondary_rgo_size[local_rgo[row]];
                    auto new_size = size;
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::PushID(row);
                    ImGui::Text("%s", local_rgo[row].c_str());
                    ImGui::TableNextColumn();
                    ImGui::InputInt("", &new_size);
                    if (new_size != size) {
                        def.secondary_rgo_size[local_rgo[row]] = new_size;
                    }
                    ImGui::TableNextColumn();
                    if (ImGui::Button("Delete")) {
                        def.secondary_rgo_size.erase(local_rgo[row]);
                    }
                    ImGui::PopID();
                }

                ImGui::TableNextRow();
                static std::string new_secondary_rgo_entry;
                ImGui::TableNextColumn();
                ImGui::InputText("Commodity", &new_secondary_rgo_entry);
                ImGui::TableNextColumn();
                if (!def.secondary_rgo_size.contains(new_secondary_rgo_entry) && new_secondary_rgo_entry.length() > 0) {
                    if (ImGui::Button("Insert")) {
                        def.secondary_rgo_size[new_secondary_rgo_entry] = 0;
                    }
                }
                ImGui::TableNextColumn();

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }

        ImGui::InputInt("Railroad: ", &def.railroad);
        ImGui::InputInt("Naval base: ", &def.naval_base);
        ImGui::InputInt("Fort: ", &def.fort);

        bool remove_flag = false;
        int remove_index = 0;
        for (int i = 0; i < def.buildings.size(); i++) {
            ImGui::PushID(i);
            if (ImGui::TreeNode("Building")) {
                ImGui::InputInt("Level", &def.buildings[i].level);
                ImGui::InputText("Type", &def.buildings[i].building_type);
                ImGui::InputText("Upgrade", &def.buildings[i].upgrade);
                if (ImGui::Button("Remove")) {
                    remove_flag = true;
                    remove_index = i;
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (remove_flag) {
            def.buildings.erase(def.buildings.begin() + remove_index);
        }
        if (ImGui::Button("Add building")) {
            game_definition::state_building bdef {
                .level = 1, .building_type = "?", .upgrade = "yes"
            };
            def.buildings.push_back(bdef);
        }

        for (int i = 0; i < def.cores.size(); i++) {
            ImGui::PushID(i);
            ImGui::InputText("Core", &(def.cores[i]));
            ImGui::SameLine();
            if (ImGui::Button("Clear core")) {
                def.cores[i] = "";
            }
            ImGui::PopID();
        }

        if (ImGui::Button("Add core")) {
            def.cores.push_back("");
        }

        ImGui::End();
    }

    void selection_nation(parsing::game_map& map, state::control& control, int32_t tag) {
        ImGui::Begin(
            "Selection",
            NULL,
            ImGuiWindowFlags_NoFocusOnAppearing
        );

        if (tag == 0) {
            ImGui::Text("No nation");
            ImGui::End();
            return;
        }

        auto & def = map.nations[map.tag_to_vector_position[tag]];
        std::string text_tag {
            (def.tag[0]),
            (def.tag[1]),
            (def.tag[2])
        };
        ImGui::Text("%s", (text_tag + " (" + def.filename + ") ").c_str());



        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Color"))
            {
                if (def.defined_in_common) {
                    float colour[3] = {
                    float(def.R) / 255.f,
                    float(def.G) / 255.f,
                    float(def.B) / 255.f
                    };
                    ImGui::ColorPicker3("National Colour", colour);
                    def.R = uint8_t(colour[0] * 255.f);
                    def.G = uint8_t(colour[1] * 255.f);
                    def.B = uint8_t(colour[2] * 255.f);
                } else {
                    ImGui::Text("%s", "This nation has no /common/countries file, do you want to generate it?");
                    if (ImGui::Button("Generate commons")) {
                        game_definition::define_nation_common(def);
                    }
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Parties"))
            {
                if (def.defined_in_common) {
                    bool remove_flag = false;
                    int remove_index = 0;
                    for (int i = 0; i < def.parties.size(); i++) {
                        ImGui::PushID(i);
                        std::string label = "Party(" + def.parties[i].ideology + ")";
                        if (ImGui::TreeNode(label.c_str())) {
                            ImGui::InputText("Name", &def.parties[i].name);
                            ImGui::InputText("Start date", &def.parties[i].start);
                            ImGui::InputText("End date", &def.parties[i].end);
                            ImGui::InputText("Ideology", &def.parties[i].ideology);
                            ImGui::InputText("Economic Policy", &def.parties[i].economic_policy);
                            ImGui::InputText("Trade Policy", &def.parties[i].trade_policy);
                            ImGui::InputText("Religious Policy", &def.parties[i].religious_policy);
                            ImGui::InputText("Citizenship Policy", &def.parties[i].citizenship_policy);
                            ImGui::InputText("War policy", &def.parties[i].war_policy);

                            if (ImGui::Button("Remove")) {
                                remove_flag = true;
                                remove_index = i;
                            }
                            ImGui::TreePop();
                        }
                        ImGui::PopID();
                    }
                    if (remove_flag) {
                        def.parties.erase(def.parties.begin() + remove_index);
                    }
                    if (ImGui::Button("Add party")) {
                        game_definition::party bdef {};
                        def.parties.push_back(bdef);
                    }
                } else {
                    ImGui::Text("%s", "This nation has no /common/countries file, do you want to generate it?");
                    if (ImGui::Button("Generate commons")) {
                        game_definition::define_nation_common(def);
                    }
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Misc.commons"))
            {
                ImGui::InputText("Name", &def.graphical_culture);
                ImGui::Text("Edit unit names manually, sorry.");
                ImGui::TextUnformatted("%s", def.unit_names.c_str());
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }
}