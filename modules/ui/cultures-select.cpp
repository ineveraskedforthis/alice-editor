#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "cultures-explore.hpp"
#include "../editor-state/editor-state.hpp"
#include <string>
#include <vector>
#include "ui_flags.hpp"

namespace widgets {
void culture_select(state::layers_stack& layers, state::control& control) {


    bool can_edit = layers.can_edit_cultures();

    if (!can_edit) {
        ImGui::Text("Can't edit cultures: the selected layer doesn't have culture files");
        if (ImGui::Button("Copy cultures data to current layer"))
            layers.copy_cultures_to_active_layer();

        ImGui::BeginDisabled();
    }

    ImGui::SeparatorText("Culture group");
    auto group_def = layers.get_culture_group_from_culture(control.selected_culture);
    ImGui::Text("%s", group_def->name.c_str());
    ImGui::Checkbox("Boost overseas assimilation", &group_def->is_overseas);
    ImGui::InputText("Union tag", &group_def->union_tag);
    ImGui::InputText("Leaders graphics", &group_def->leader);
    ImGui::InputText("Units graphics", &group_def->unit);

    ImGui::SeparatorText("Culture");
    auto culture_def = layers.get_culture(control.selected_culture);
    ImGui::Text("%s", control.selected_culture.c_str());

    ImGui::InputText("Primary tag", &culture_def->primary);
    ImGui::InputInt("Radical.", &culture_def->radicalism);

    float fr = (float)(culture_def->r) / 255.f;
    float fg = (float)(culture_def->g) / 255.f;
    float fb = (float)(culture_def->b) / 255.f;
    float colors[3] = {fr, fg, fb};
    ImGui::PushItemWidth(128);
    ImGui::ColorPicker3("Color", colors);
    if (colors[0] != fr || colors[1] != fg || colors[2] != fb) {
        culture_def->r = static_cast<uint8_t>(colors[0] * 255.f);
        culture_def->g = static_cast<uint8_t>(colors[1] * 255.f);
        culture_def->b = static_cast<uint8_t>(colors[2] * 255.f);
    }
    ImGui::PopItemWidth();

    if (!can_edit) {
        ImGui::EndDisabled();
    }

    ImGui::SeparatorText("Cultural names (DISABLED)");
    ImGui::Text("Disabled until integration of a unicode conversions library");


    ImGui::BeginDisabled();
    ImGui::Text("First names:");
    for (int i = 0; i < culture_def->first_names.size(); i++) {
        ImGui::PushID(i);
        ImGui::InputText("##first_name", &culture_def->first_names[i]);
        ImGui::PopID();
    }

    if (ImGui::Button("Add first name")) {
        culture_def->first_names.push_back({});
    }

    ImGui::Text("Last names:");
    for (int i = 0; i < culture_def->last_names.size(); i++) {
        ImGui::PushID(i);
        ImGui::InputText("##last_name", &culture_def->last_names[i]);
        ImGui::PopID();
    }
    if (ImGui::Button("Add second name")) {
        culture_def->last_names.push_back({});
    }
    ImGui::EndDisabled();
}
}