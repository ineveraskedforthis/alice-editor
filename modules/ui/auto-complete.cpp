#include "auto-complete.hpp"
#include "../editor-state/editor-state.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

namespace widgets {
namespace auto_complete {
void culture(state::layers_stack& layers, std::string& selected_culture) {
        auto cultures = layers.retrieve_cultures();
        if (ImGui::BeginCombo("##culture", selected_culture.c_str())) {
                static ImGuiTextFilter filter;
                ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
                filter.Draw("##Filter", -FLT_MIN);
                if (ImGui::IsWindowAppearing())
                        ImGui::SetKeyboardFocusHere(-1);
                for (int n = 0; n < cultures.size(); n++) {
                        const bool is_selected = (selected_culture == cultures[n]);
                        if (filter.PassFilter(cultures[n].c_str()))
                                if (ImGui::Selectable(cultures[n].c_str(), is_selected))
                                selected_culture = cultures[n];
                }
                ImGui::EndCombo();
        }
}
void religion(state::layers_stack& layers, std::string& selected_religion) {
        auto religions = layers.retrieve_religions();
        if (ImGui::BeginCombo("##religion", selected_religion.c_str())) {
                static ImGuiTextFilter filter;
                ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_F);
                filter.Draw("##Filter", -FLT_MIN);
                if (ImGui::IsWindowAppearing())
                        ImGui::SetKeyboardFocusHere(-1);
                for (int n = 0; n < religions.size(); n++) {
                        const bool is_selected = (selected_religion == religions[n]);
                        if (filter.PassFilter(religions[n].c_str()))
                                if (ImGui::Selectable(religions[n].c_str(), is_selected))
                                selected_religion = religions[n];
                }
                ImGui::EndCombo();
        }
}
}
}