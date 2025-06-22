#pragma once

namespace state {
    struct layers_stack;
    struct control;
    struct editor;
}
namespace assets {
    struct storage;
}

namespace widgets {
void pops_buffer_widget(state::layers_stack& layers, state::control& control);
}