#pragma once
#include <string>

namespace state {
struct layers_stack;
}

namespace widgets {
namespace auto_complete {
void culture(state::layers_stack& layers, std::string&);
void religion(state::layers_stack& layers, std::string&);
}
}
