namespace state {
    struct layers_stack;
    struct control;
    struct editor;
}
namespace assets {
    struct storage;
}

namespace widgets {
    namespace modal {
        void create_culture(state::layers_stack& layers, bool& update_required);
    };
}