#pragma once
#include "GL/glew.h"
#include <string>
#include "../map/unordered_dense.h"

namespace assets {
    struct asset {
        GLuint texture;
        int w; int h;
    };
    struct storage {
        ankerl::unordered_dense::map<std::string, asset> filename_to_texture_asset {};
    };
};