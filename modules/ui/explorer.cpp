#include "explorer.hpp"

#include "imgui.h"

#include "../editor-state/editor-state.hpp"
#include "../assets-manager/assets.hpp"
#include <cstddef>

namespace widgets {

    enum explorer_columns
    {
        province_v2id,
        province_name,
        province_size,
        province_owner,
        province_select
    };

    void explorer_provinces(state::layers_stack& map, state::control& control) {

        static std::vector<int> list_of_v2id {};

        auto& active_layer = map.data[map.current_layer_index];

        if (active_layer.has_province_definitions && list_of_v2id.size() == 0) {
            for (int i = 0; i < active_layer.province_definitions.size(); i++) {
                auto history = map.get_province_history(active_layer.province_definitions[i].v2id);
                if (history != nullptr)
                    list_of_v2id.push_back(active_layer.province_definitions[i].v2id);
            }
        }

        // Options
        static ImGuiTableFlags flags =
            ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
            | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody
            | ImGuiTableFlags_ScrollY;
        ImGuiStyle& style = ImGui::GetStyle();

        if (ImGui::BeginTable("table_sorting", 5, flags, ImVec2(0.0f, 500), 0.0f))
        {
            // Declare columns
            // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
            // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
            // Demonstrate using a mixture of flags among available sort-related flags:
            // - ImGuiTableColumnFlags_DefaultSort
            // - ImGuiTableColumnFlags_NoSort / ImGuiTableColumnFlags_NoSortAscending / ImGuiTableColumnFlags_NoSortDescending
            // - ImGuiTableColumnFlags_PreferSortAscending / ImGuiTableColumnFlags_PreferSortDescending
            ImGui::TableSetupColumn(
                "ID",
                ImGuiTableColumnFlags_DefaultSort
                | ImGuiTableColumnFlags_WidthFixed,
                80.0f,
                province_v2id
            );
            ImGui::TableSetupColumn(
                "Name",
                ImGuiTableColumnFlags_DefaultSort
                | ImGuiTableColumnFlags_WidthFixed,
                100.0f,
                province_name
            );
            ImGui::TableSetupColumn(
                "Size",
                ImGuiTableColumnFlags_DefaultSort
                | ImGuiTableColumnFlags_WidthFixed,
                80.0f,
                province_size
            );
            ImGui::TableSetupColumn(
                "Owner",
                ImGuiTableColumnFlags_DefaultSort
                | ImGuiTableColumnFlags_WidthFixed,
                80.0f,
                province_owner
            );
            ImGui::TableSetupColumn(
                "Select",
                ImGuiTableColumnFlags_NoSort
                | ImGuiTableColumnFlags_WidthFixed,
                40.0f,
                province_select
            );
            ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
            ImGui::TableHeadersRow();

            // Sort our data if sort specs have been changed!
            if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
                if (sort_specs->SpecsDirty) {
                    std::sort(list_of_v2id.begin(), list_of_v2id.end(), [&](int a, int b) {
                        auto a_def = map.get_province_definition(a);
                        auto b_def = map.get_province_definition(b);

                        auto a_history = map.get_province_history(a);
                        auto b_history = map.get_province_history(b);

                        auto a_size = map.indices.v2id_to_size[a];
                        auto b_size = map.indices.v2id_to_size[b];

                        if (
                            a_def == nullptr
                            || b_def == nullptr
                            || a_history == nullptr
                            || b_history == nullptr
                        ) {
                            return a > b;
                        }

                        for (int n = 0; n < sort_specs->SpecsCount; n++)
                        {
                            // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                            // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
                            const ImGuiTableColumnSortSpecs* sort_spec = &sort_specs->Specs[n];
                            int order = 0;
                            switch (sort_spec->ColumnUserID)
                            {
                            case province_v2id:  order = (a - b); break;
                            case province_name:  order = (a_def->name.compare(b_def->name)); break;
                            case province_size:  order = (a_size - b_size); break;
                            case province_owner: order = (a_history->owner_tag.compare(b_history->owner_tag)); break;
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

            // Demonstrate using clipper for large vertical lists
            ImGuiListClipper clipper;
            clipper.Begin(list_of_v2id.size());
            while (clipper.Step())
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    // Display a data item
                    auto v2id = list_of_v2id[row_n];
                    auto def = map.get_province_definition(v2id);
                    auto history = map.get_province_history(v2id);
                    auto size = map.indices.v2id_to_size[v2id];

                    ImGui::PushID(v2id);
                    ImGui::TableNextRow();

                    // ID
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", v2id);

                    // NAME
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", def->name.c_str());

                    // SIZE
                    ImGui::TableNextColumn();
                    ImGui::Text("%d", size);

                    // OWNER
                    if (history == nullptr) {
                        ImGui::TableNextColumn();
                        ImGui::Text("???");
                    } else {
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", history->owner_tag.c_str());
                    }


                    // SELECT
                    ImGui::TableNextColumn();
                    if (ImGui::SmallButton(">>")) {
                        control.selected_province_id = v2id;
                    }

                    ImGui::PopID();
                }
            ImGui::EndTable();
        }
    }

    void explorer_nations(state::layers_stack& map, state::control& control) {

    }

    void explorer_adjacencies(state::layers_stack& map, state::control& control) {

    }


    void explorer(state::layers_stack& map, state::control& control, state::editor& editor, assets::storage& storage) {
        ImGui::Begin(
            "Explorer",
            NULL,
            ImGuiWindowFlags_NoFocusOnAppearing
        );

        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("ExplorerTabs", tab_bar_flags)) {
            if (ImGui::BeginTabItem("Provinces")) {
                explorer_provinces(map, control);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Nations")) {
                explorer_nations(map, control);
                ImGui::EndTabItem();
            }
            // if (ImGui::BeginTabItem("Adjacencies")) {
            //     explorer_adjacencies(map, control);
            //     ImGui::EndTabItem();
            // }
            ImGui::EndTabBar();
        }


        ImGui::End();
    }
}