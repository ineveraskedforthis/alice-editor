#pragma once

#include "GL/glew.h"
namespace state {
    struct layers_stack;
    struct control;
    struct editor;
}
namespace assets {
    struct storage;
}

namespace widgets {
    void explorer(state::layers_stack& map, state::control& control, state::editor& editor, assets::storage& storage);
}