#include "imgui.h"
#include "ui_enums.hpp"
#include "pops_buffer_widget.hpp"
#include "../editor-state/editor-state.hpp"

namespace widgets {

void define_pop_table_columns() {
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
}

void define_provinces_table_columns() {
    ImGui::TableSetupColumn(
        "v2id",
        ImGuiTableColumnFlags_DefaultSort
        | ImGuiTableColumnFlags_WidthFixed,
        80.0f,
        province_split_target_v2id
    );
    ImGui::TableSetupColumn(
        "Name",
        ImGuiTableColumnFlags_DefaultSort
        | ImGuiTableColumnFlags_WidthFixed,
        80.0f,
        province_split_target_name
    );
    ImGui::TableSetupColumn(
        "Priority",
        ImGuiTableColumnFlags_DefaultSort
        | ImGuiTableColumnFlags_WidthFixed,
        250.0f,
        province_split_target_priority
    );
}

void sort_table_pops(state::control& control) {
    // Sort our data if sort specs have been changed!
    if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
        if (sort_specs->SpecsDirty) {
            std::sort(control.pop_buffer_indices.begin(), control.pop_buffer_indices.end(), [&](auto& a, auto& b) {
                auto& a_def = control.pop_buffer[a];
                auto& b_def = control.pop_buffer[b];

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
}

void sort_table_provinces(state::layers_stack& layers, state::control& control) {
    // Sort our data if sort specs have been changed!
    if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
        if (sort_specs->SpecsDirty) {
            std::sort(
                control.province_targets_for_pop_splitting_indices.begin(),
                control.province_targets_for_pop_splitting_indices.end(),
                [&](auto& a, auto& b) {
                auto& a_target = control.province_targets_for_pop_splitting[a];
                auto& b_target = control.province_targets_for_pop_splitting[b];

                auto a_def = layers.get_province_definition(a_target.v2id);
                std::string name_a = "Unknown";
                if (a_def != nullptr) {
                    name_a = a_def->name;
                }

                auto b_def = layers.get_province_definition(a_target.v2id);
                std::string name_b = "Unknown";
                if (b_def != nullptr) {
                    name_b = b_def->name;
                }

                for (int n = 0; n < sort_specs->SpecsCount; n++)
                {
                    const ImGuiTableColumnSortSpecs* sort_spec = &sort_specs->Specs[n];
                    int order = 0;
                    switch (sort_spec->ColumnUserID)
                    {
                    case province_split_target_v2id:
                        order = (int)(a_target.v2id) - (int)(b_target.v2id);
                        break;
                    case province_split_target_name:
                        order = name_a.compare(name_b);
                        break;
                    case province_split_target_priority:
                        order = a_target.priority - b_target.priority;
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

void display_pop_table_row(
    game_definition::pop_history& pop,
    std::vector<std::string>& poptypes,
    std::vector<std::string>& cultures,
    std::vector<std::string>& religions
) {
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

    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(60.f);
    ImGui::InputFloat("##militancy", &pop.militancy);

    ImGui::TableNextColumn();
    ImGui::Text("%s", pop.rebel_type.c_str());
}

void display_province_table_row(
    state::control& control,
    state::split_target_province& target,
    std::string& name,
    bool exists_non_zero_priority,
    int total_count
) {
    ImGui::TableNextColumn();
    ImGui::Text("%d", target.v2id);

    ImGui::TableNextColumn();
    ImGui::Text("%s", name.c_str());

    ImGui::TableNextColumn();
    auto ratio = target.priority;
    if (!exists_non_zero_priority) {
        ratio = 1.f / (float) total_count;
    }
    auto old_ratio = ratio;
    ImGui::SetNextItemWidth(250.f);
    ImGui::SliderFloat("##size_drag", &ratio, 0.f, 1.f);

    if (ratio != old_ratio) {
        auto remainder = 1.f - ratio;
        auto old_remainder = 1.f - old_ratio;
        auto scaler = (float)remainder / (float)old_remainder;
        target.priority = ratio;

        // scale everything else down:
        for (size_t index = 0; index < control.province_targets_for_pop_splitting.size(); index++) {
            auto& other_target = control.province_targets_for_pop_splitting[index];
            if (other_target.v2id == target.v2id) {
                continue;
            }
            if (!exists_non_zero_priority) {
                other_target.priority = 1.f / (float) total_count * scaler;
            } else {
                other_target.priority *= scaler;
            }
        }
    }
}

void pops_buffer_widget(state::layers_stack& layers, state::control& control) {

    if (ImGui::Button("Clear buffer")) {
        control.pop_buffer.clear();
        control.pop_buffer_indices.clear();
    }

    auto dates = layers.get_available_dates();

    if (ImGui::BeginTabBar("PopulationSplitterTabs", ImGuiTabBarFlags_None)) {
        for (auto date : dates) {
            auto year = date / (31*12);
            auto year_s = std::to_string(year);
            if (ImGui::BeginTabItem(year_s.c_str())) {
                bool all_zero = true;
                for (auto& item : control.province_targets_for_pop_splitting) {
                    if (item.priority != 0.f) {
                        all_zero = false;
                    }
                }
                if (ImGui::Button("Split pops among the target provinces")) {
                    for (auto& pop : control.pop_buffer) {
                        for (auto& target : control.province_targets_for_pop_splitting) {

                            auto priority = target.priority;
                            if (all_zero) {
                                priority = 1.f / control.province_targets_for_pop_splitting.size();
                            }

                            bool pop_already_exists = false;

                            if (!layers.can_edit_pops(target.v2id, date)) {
                                layers.copy_pops_data_to_current_layer(target.v2id, date);
                            }
                            auto pops = layers.get_pops(target.v2id, date);

                            for (auto & candidate : *pops) {
                                if (
                                    candidate.culture == pop.culture
                                    && candidate.poptype == pop.poptype
                                    && candidate.religion == pop.religion
                                    && pop.militancy == 0.f
                                    && pop.rebel_type.size() == 0
                                ) {
                                    pop_already_exists = true;
                                    candidate.size += (int) (pop.size * priority);
                                    break;
                                }
                            }

                            if (!pop_already_exists) {
                                auto split_pop = game_definition::pop_history {};
                                split_pop.culture = pop.culture;
                                split_pop.militancy = pop.militancy;
                                split_pop.poptype = pop.poptype;
                                split_pop.rebel_type = pop.rebel_type;
                                split_pop.religion = pop.religion;
                                split_pop.size = (int) (pop.size * priority);
                                if (split_pop.size == 0 && pop.size > 0) {
                                    split_pop.size = 1;
                                }
                                pops->push_back(split_pop);
                            }
                        }
                    }

                    control.pop_buffer.clear();
                    control.pop_buffer_indices.clear();
                    control.province_targets_for_pop_splitting.clear();
                    control.province_targets_for_pop_splitting_indices.clear();
                }

                auto poptypes = layers.retrieve_poptypes();
                auto cultures = layers.retrieve_cultures();
                auto religions = layers.retrieve_religions();

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

                ImGui::Text("These pops would be split");

                if (
                    ImGui::BeginTable(
                        "table_population",
                        6,
                        flags,
                        ImVec2(0.0f, 200),
                        0.0f
                    )
                ) {
                    define_pop_table_columns();
                    ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
                    ImGui::TableHeadersRow();
                    sort_table_pops(control);
                    ImGuiListClipper clipper;
                    clipper.Begin(control.pop_buffer_indices.size());
                    while (clipper.Step())
                        for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                            // Display a data item
                            auto pop_index = control.pop_buffer_indices[row_n];
                            auto& pop = control.pop_buffer[pop_index];
                            ImGui::PushID(row_n);
                            ImGui::TableNextRow();
                            display_pop_table_row(pop, poptypes, cultures, religions);
                            ImGui::PopID();
                        }
                    ImGui::EndTable();
                }

                if (ImGui::Button("Set priorities according to province size")) {
                    auto total_size = 0;
                    for (auto& target : control.province_targets_for_pop_splitting) {
                        auto size = layers.indices.v2id_to_size[target.v2id];
                        total_size += size;
                    }
                    for (auto& target : control.province_targets_for_pop_splitting) {
                        auto size = layers.indices.v2id_to_size[target.v2id];
                        if (total_size == 0) {
                            target.priority = 0.f;
                        } else {
                            target.priority = (float) size / (float) total_size;
                        }
                    }
                }

                ImGui::Text("Among these provinces, accoring to priorities in the table");

                if (
                    ImGui::BeginTable(
                        "table_split_targets",
                        3,
                        flags,
                        ImVec2(0.0f, 200),
                        0.0f
                    )
                ) {
                    define_provinces_table_columns();
                    ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
                    ImGui::TableHeadersRow();
                    sort_table_provinces(layers, control);
                    ImGuiListClipper clipper;
                    clipper.Begin(control.province_targets_for_pop_splitting_indices.size());
                    while (clipper.Step())
                        for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
                            // Display a data item
                            auto index = control.province_targets_for_pop_splitting_indices[row_n];
                            auto& target = control.province_targets_for_pop_splitting[index];

                            auto def = layers.get_province_definition(target.v2id);
                            std::string name = "Unknown";
                            if (def != nullptr) {
                                name = def->name;
                            }

                            ImGui::PushID(row_n);
                            ImGui::TableNextRow();
                            display_province_table_row(
                                control,
                                target,
                                name,
                                !all_zero,
                                control.province_targets_for_pop_splitting.size()
                            );
                            ImGui::PopID();
                        }
                    ImGui::EndTable();
                }
                ImGui::EndTabItem();
            }
        }
    }
    ImGui::EndTabBar();
}
}