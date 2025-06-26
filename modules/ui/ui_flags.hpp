#include "imgui.h"

namespace widgets {
    static ImGuiTableFlags sortable_table_flags =
        ImGuiTableFlags_Reorderable
        | ImGuiTableFlags_Hideable
        | ImGuiTableFlags_Sortable
        | ImGuiTableFlags_SortMulti
        | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_BordersOuter
        | ImGuiTableFlags_BordersV
        | ImGuiTableFlags_BordersH
        | ImGuiTableFlags_ScrollY;

}