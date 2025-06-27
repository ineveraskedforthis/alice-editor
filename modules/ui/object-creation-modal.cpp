#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "../editor-state/editor-state.hpp"

namespace widgets {
    namespace modal {
        void create_culture(state::layers_stack& layers, bool& update_required) {


            if (ImGui::BeginPopupModal("Create new culture", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("This operation cannot be undone.");
                ImGui::Separator();

                static std::string name;
                ImGui::InputText("Culture name", &name);

                static float colors[3] = {0.f, 0.f, 0.f};
                ImGui::PushItemWidth(128);
                ImGui::ColorPicker3("Color", colors);

                static int selected_culture_group = 0;
                auto culture_groups = layers.retrieve_culture_groups();

                if (selected_culture_group >= culture_groups.size())
                    selected_culture_group = 0;

                if (ImGui::BeginCombo("Culture group", culture_groups[selected_culture_group].c_str())) {
                    static ImGuiTextFilter filter;
                    ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
                    filter.Draw("##Filter", -FLT_MIN);
                    if (ImGui::IsWindowAppearing())
                        ImGui::SetKeyboardFocusHere(-1);

                    for (int n = 0; n < culture_groups.size(); n++) {
                        const bool is_selected = (n == selected_culture_group);
                        if (filter.PassFilter(culture_groups[n].c_str()))
                            if (ImGui::Selectable(culture_groups[n].c_str(), is_selected))
                                selected_culture_group = n;
                    }
                    ImGui::EndCombo();
                }

                if (ImGui::Button("OK", ImVec2(120, 0))) {
                    auto r = static_cast<int>(colors[0] * 255.f);
                    auto g = static_cast<int>(colors[1] * 255.f);
                    auto b = static_cast<int>(colors[2] * 255.f);
                    layers.new_culture(name, culture_groups[selected_culture_group], r, g, b);
                    update_required = true;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();

                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    };
}