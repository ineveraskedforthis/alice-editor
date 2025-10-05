#include "imgui.h"
#include "ui_enums.hpp"
#include "cultures-explore.hpp"
#include "../editor-state/editor-state.hpp"
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>
#include "ui_flags.hpp"
#include "object-creation-modal.hpp"

namespace widgets {
void define_culture_table_columns() {
    ImGui::TableSetupColumn(
        "Culture",
        ImGuiTableColumnFlags_DefaultSort
        | ImGuiTableColumnFlags_WidthFixed,
        200.0f,
        culture_name
    );
    ImGui::TableSetupColumn(
        "Culture group",
        ImGuiTableColumnFlags_DefaultSort
        | ImGuiTableColumnFlags_WidthFixed,
        200.0f,
        culture_group
    );
    ImGui::TableSetupColumn(
        "Union tag",
        ImGuiTableColumnFlags_DefaultSort
        | ImGuiTableColumnFlags_WidthFixed,
        40.0f,
        culture_group
    );
}

void sort_culture_table(state::layers_stack& layers, std::vector<size_t> & indices, std::vector<std::string> & cultures) {
    // Sort our data if sort specs have been changed!
    if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
        if (sort_specs->SpecsDirty) {
            std::stable_sort(indices.begin(), indices.end(), [&](auto& a, auto& b) {
                auto a_group = layers.get_culture_group_from_culture(cultures[a]);
                auto b_group = layers.get_culture_group_from_culture(cultures[b]);

                for (int n = 0; n < sort_specs->SpecsCount; n++)
                {
                    const ImGuiTableColumnSortSpecs* sort_spec = &sort_specs->Specs[n];
                    int order = 0;
                    switch (sort_spec->ColumnUserID)
                    {
                    case culture_name:
                        order = cultures[a].compare(cultures[b]);
                        break;
                    case culture_group:
                        order = a_group->name.compare(b_group->name);
                        break;
                    case culture_union_tag:
                        if (a_group->union_tag.size() != b_group->union_tag.size()) {
                            order = (int)(a_group->union_tag.size()) - (int)(b_group->union_tag.size());
                        } else {
                            order = a_group->union_tag.compare(b_group->union_tag);
                        }
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
}

void display_culture_row(
    state::layers_stack& layers,
    state::control& control,
    std::string culture
) {
    auto group = layers.get_culture_group_from_culture(culture);

    ImGui::TableNextColumn();
    if (ImGui::Button(culture.c_str(), ImVec2(190, 0))) {
        control.selected_culture = culture;
        layers.request_map_update = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", group->name.c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", group->union_tag.c_str());
}

void cultures_list(state::layers_stack& layers, state::control& control) {
    static auto cultures = layers.retrieve_cultures();
    static std::vector<size_t> indices {};
    bool update_required = false;
    if (indices.size() != cultures.size()){
        update_required = true;
    }
    if (ImGui::Button("Update")) {
        update_required = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Create new culture"))
        ImGui::OpenPopup("Create new culture");

    modal::create_culture(layers, update_required);

    if (update_required) {
        cultures.clear();
        indices.clear();
        cultures = layers.retrieve_cultures();
        for (auto i = 0; i < cultures.size(); i++) {
            indices.push_back(i);
        }
    }

    if (
        ImGui::BeginTable(
            "table_culture",
            3,
            sortable_table_flags,
            ImVec2(0.0f, 400),
            0.0f
        )
    ) {
        define_culture_table_columns();
        ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
        ImGui::TableHeadersRow();
        sort_culture_table(layers, indices, cultures);
        ImGuiListClipper clipper;
        clipper.Begin(indices.size());
        while (clipper.Step())
            for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                // Display a data item
                auto culture_index = indices[row_n];
                auto& culture = cultures[culture_index];
                ImGui::PushID(row_n);
                ImGui::TableNextRow();
                display_culture_row(layers, control, culture);
                ImGui::PopID();
            }
        ImGui::EndTable();
    }
}


}