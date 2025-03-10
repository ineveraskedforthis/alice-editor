#pragma once
#include "GL/glew.h"
#include <map>
#include <string>

namespace assets {
    struct asset {
        GLuint texture;
        int w; int h;
    };
    struct storage {
        std::map<std::string, asset> filename_to_texture_asset {};
    };
};