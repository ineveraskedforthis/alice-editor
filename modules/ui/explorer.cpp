#include "explorer.hpp"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include "../editor-state/editor-state.hpp"
#include "../assets-manager/assets.hpp"
#include <array>
#include <cctype>
#include <cstddef>
#include <string>
#include "ui_enums.hpp"
#include "cultures-explore.hpp"

namespace widgets {

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
            | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV
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
                        control.selected_tag = history->owner_tag;
                    }

                    ImGui::PopID();
                }
            ImGui::EndTable();
        }
    }

    void explorer_nations(state::layers_stack& map, state::control& control) {
        // new nation creation:

        ImGui::Text("Create new nation");
        ImGui::InputText("TAG", &control.new_nation_tag);
        ImGui::InputText("Name", &control.new_nation_filename);

        std::array<int8_t, 3> tag_fixed_length = {
            control.new_nation_tag[0],
            control.new_nation_tag[1],
            control.new_nation_tag[2]
        };

        if (control.new_nation_tag.length() == 3 && map.valid_tag(game_definition::tag_to_int(tag_fixed_length)))
            if(ImGui::Button("Create nation")) {
                std::array<int8_t, 3> selected_tag_fixed_length = {
                    control.selected_tag[0],
                    control.selected_tag[1],
                    control.selected_tag[2]
                };

                map.new_nation(
                    game_definition::tag_to_int(selected_tag_fixed_length),
                    game_definition::tag_to_int(tag_fixed_length),
                    control.new_nation_filename + ".txt"
                );

                control.selected_tag = control.new_nation_tag;
            }

        static std::vector<int> list_of_tags {};

        auto& active_layer = map.data[map.current_layer_index];

        if (!active_layer.has_nations_list)
            list_of_tags.clear();

        if (active_layer.has_nations_list && list_of_tags.size() != active_layer.nations.size()) {
            list_of_tags.clear();
            for (int i = 0; i < active_layer.nations.size(); i++) {
                auto& def = active_layer.nations[i];
                list_of_tags.push_back(game_definition::tag_to_int(def.tag));
            }
        }

        // Options
        static ImGuiTableFlags flags =
            ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
            | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV
            | ImGuiTableFlags_ScrollY;
        ImGuiStyle& style = ImGui::GetStyle();

        if (ImGui::BeginTable("table_sorting", 4, flags, ImVec2(0.0f, 500), 0.0f))
        {
            // Declare columns
            // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
            // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
            // Demonstrate using a mixture of flags among available sort-related flags:
            // - ImGuiTableColumnFlags_DefaultSort
            // - ImGuiTableColumnFlags_NoSort / ImGuiTableColumnFlags_NoSortAscending / ImGuiTableColumnFlags_NoSortDescending
            // - ImGuiTableColumnFlags_PreferSortAscending / ImGuiTableColumnFlags_PreferSortDescending
            ImGui::TableSetupColumn(
                "TAG",
                ImGuiTableColumnFlags_DefaultSort
                | ImGuiTableColumnFlags_WidthFixed,
                80.0f,
                nation_tag
            );
            ImGui::TableSetupColumn(
                "Name",
                ImGuiTableColumnFlags_DefaultSort
                | ImGuiTableColumnFlags_WidthFixed,
                100.0f,
                nation_name
            );
            ImGui::TableSetupColumn(
                "Civilized",
                ImGuiTableColumnFlags_DefaultSort
                | ImGuiTableColumnFlags_WidthFixed,
                100.0f,
                nation_civilized
            );
            ImGui::TableSetupColumn(
                "Select",
                ImGuiTableColumnFlags_NoSort
                | ImGuiTableColumnFlags_WidthFixed,
                40.0f,
                nation_select
            );
            ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
            ImGui::TableHeadersRow();

            // Sort our data if sort specs have been changed!
            if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
                if (sort_specs->SpecsDirty) {
                    std::sort(list_of_tags.begin(), list_of_tags.end(), [&](int a, int b) {
                        auto a_def = map.get_nation_definition(a);
                        auto b_def = map.get_nation_definition(b);

                        auto a_history = map.get_nation_history(a);
                        auto b_history = map.get_nation_history(b);

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
                            case nation_tag:  order = (a - b); break;
                            case nation_name:  order = (a_def->filename.compare(b_def->filename)); break;
                            case nation_civilized:  order = ((int)(a_history->civilized) - (int)(b_history->civilized)); break;
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
            clipper.Begin(list_of_tags.size());
            while (clipper.Step())
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    // Display a data item
                    auto tag = list_of_tags[row_n];
                    auto def = map.get_nation_definition(tag);
                    auto tag_array = def->tag;
                    auto history = map.get_nation_history(tag);

                    ImGui::PushID(tag);
                    ImGui::TableNextRow();

                    // ID
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", std::string{tag_array[0], tag_array[1], tag_array[2]}.c_str());

                    // NAME
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", def->filename.c_str());

                    // CIV
                    ImGui::TableNextColumn();

                    if (history != nullptr) {
                        if (!map.can_edit_nation_history(tag)) {
                            ImGui::BeginDisabled();
                        }
                        ImGui::Checkbox("##", &(history->civilized));
                        if (!map.can_edit_nation_history(tag)) {
                            ImGui::EndDisabled();
                        }
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255));
                        ImGui::Text("No history");
                        ImGui::PopStyleColor();
                    }

                    // SELECT
                    ImGui::TableNextColumn();
                    if (ImGui::SmallButton(">>")) {
                        control.selected_province_id = history->capital;
                        control.selected_tag = std::string{tag_array[0], tag_array[1], tag_array[2]};
                    }

                    ImGui::PopID();
                }
            ImGui::EndTable();
        }
    }

    std::vector<std::string> trade_good_classes = {
        "military_goods", "raw_material_goods", "industrial_goods", "consumer_goods"
    };

    void explorer_goods(state::layers_stack& map, state::control& control) {
        static ImGuiTableFlags flags =
            ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
            | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV
            | ImGuiTableFlags_ScrollY;
        ImGuiStyle& style = ImGui::GetStyle();
        auto& active_layer = map.data[map.current_layer_index];

        ImGui::Text("Add new good");
        static std::string name_of_new_good;
        ImGui::InputText("Name", &name_of_new_good);

        // choose the type of trade good (hardcoded for now...)
        // ImGui::SameLine();

        static int selected_class = 0;

        if (ImGui::BeginCombo("Category", trade_good_classes[selected_class].c_str())) {
            for (int n = 0; n < trade_good_classes.size(); n++) {
                if (ImGui::Selectable(trade_good_classes[n].c_str(), selected_class == n)) {
                    selected_class = n;
                }
            }
            ImGui::EndCombo();
        }

        auto valid = true;
        if (name_of_new_good.size() == 0) {
            valid = false;
        } else {
            for(auto character : name_of_new_good) {
                if (!isprint(character)) {
                    valid = false;
                    break;
                }
            }
        }

        if (!valid) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Create")) {
            map.copy_goods_to_current_layer();
            map.copy_resources_to_current_layer();
            auto count = active_layer.goods.size();
            {
                auto& image = active_layer.resources_small;
                auto size = image.size_x / (count + 1);
                image.expand_image_right(size);
            }
            {
                auto& image = active_layer.resources_medium;
                auto size = image.size_x / (count + 1);
                image.expand_image_right(size);
            }
            {
                auto& image = active_layer.resources_big;
                auto size = image.size_x / (count + 1);
                image.expand_image_right(size);
            }

            game_definition::commodity new_good {
                .index = (int)active_layer.goods.size(),
                .name = name_of_new_good,
                .group = trade_good_classes[selected_class],

            };
            active_layer.goods[name_of_new_good] = new_good;
        }

        if (!valid) {
            ImGui::EndDisabled();
        }

        static std::vector<std::string> list_of_goods {};
        if (!active_layer.has_goods) {
            list_of_goods.clear();
            ImGui::Text("No goods are defined on the active layer.");
            ImGui::Text("Do you wish to copy goods to the active layer?");
            if (ImGui::Button("Copy goods to the active layer.")) {
                map.copy_goods_to_current_layer();
                list_of_goods.clear();
                for (auto& [key, value] : active_layer.goods) {
                    list_of_goods.push_back(key);
                }
            }
            return;
        }

        if (active_layer.has_goods && list_of_goods.size() != active_layer.goods.size()) {
            list_of_goods.clear();
            for (auto& [key, value] : active_layer.goods) {
                list_of_goods.push_back(key);
            }
        }

        if (ImGui::BeginTable("table_sorting", 2, flags, ImVec2(0.0f, 500), 0.0f)) {
            ImGui::TableSetupColumn(
                "Icon",
                ImGuiTableColumnFlags_NoSort
                | ImGuiTableColumnFlags_WidthFixed,
                40.0f,
                goods_icon
            );
            ImGui::TableSetupColumn(
                "Name",
                ImGuiTableColumnFlags_DefaultSort
                | ImGuiTableColumnFlags_WidthFixed,
                100.0f,
                goods_name
            );
            // ImGui::TableSetupColumn(
            //     "Select",
            //     ImGuiTableColumnFlags_NoSort
            //     | ImGuiTableColumnFlags_WidthFixed,
            //     40.0f,
            //     goods_select
            // );
            ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
            ImGui::TableHeadersRow();

            // Sort our data if sort specs have been changed!
            if (ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs())
                if (sort_specs->SpecsDirty) {
                    std::sort(list_of_goods.begin(), list_of_goods.end(), [&](std::string a, std::string b) {
                        auto a_def = active_layer.goods[a];
                        auto b_def = active_layer.goods[b];

                        for (int n = 0; n < sort_specs->SpecsCount; n++)
                        {
                            // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                            // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
                            const ImGuiTableColumnSortSpecs* sort_spec = &sort_specs->Specs[n];
                            int order = 0;
                            switch (sort_spec->ColumnUserID)
                            {
                            case goods_name:  order = a.compare(b); break;
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
            clipper.Begin(list_of_goods.size());
            while (clipper.Step())
                for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
                {
                    // Display a data item
                    auto name = list_of_goods[row_n];
                    auto def = active_layer.goods[name];

                    ImGui::PushID(name.c_str());
                    ImGui::TableNextRow();

                    // icon
                    ImGui::TableNextColumn();
                    GLuint texture_index;
                    int x;
                    int y;
                    if (map.get_resources_texture_latest(texture_index, x, y)) {
                        auto count = (float)active_layer.goods.size() + 1;
                        auto width = 1.f / count;
                        auto start = width * (def.index + 1);
                        if (ImGui::ImageButton(
                            name.c_str(),
                            (ImTextureID)(intptr_t)texture_index,
                            ImVec2(24, 24),
                            ImVec2(start, 0.0f),
                            ImVec2(start + width, 1.0f)
                        )) {
                            control.selected_commodity = name;
                        };
                    } else {
                        ImGui::Text("%d", def.index);
                    }


                    // NAME
                    ImGui::TableNextColumn();
                    ImGui::Text("%s", name.c_str());

                    // SELECT
                    // ImGui::TableNextColumn();
                    // if (ImGui::SmallButton(">>")) {
                    //     control.selected_commodity = name;
                    // }
                    ImGui::PopID();
                }
            ImGui::EndTable();
        }
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
            if (ImGui::BeginTabItem("Goods")) {
                explorer_goods(map, control);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Cultures")) {
                cultures_list(map, control);
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