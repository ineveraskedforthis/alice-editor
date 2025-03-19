#include <iostream>
#include <SDL.h>
#include <string>
#include <vector>
#include "GL/glew.h"
#include "modules/glm/fwd.hpp"
#include "modules/parsing/generated/parsers_core.hpp"
#include "modules/parsing/map.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "modules/glm/ext/matrix_transform.hpp"

#include "modules/ui/editor.hpp"
#include "modules/ui/window-wrapper.hpp"
#include "modules/editor-state/editor-state.hpp"
#include "modules/assets-manager/assets.hpp"

#undef max
#undef min
#undef clamp

int main(int argc, char* argv[]) {
    std::cout << "Welcome\n";
    {
        window::wrapper window {};
        state::control control_state {};
        assets::storage storage {};
        state::layers_stack layers {};
        layers.load_owner_texture_to_gpu();
        state::editor editor {};
        {
            state::layer l {};
            l.path = "./base-game";
            parsers::load_layer(l);
            l.load_state_texture_to_gpu();
            l.load_sea_texture_to_gpu();
            layers.data.push_back(l);
            layers.current_layer_index = 0;
            layers.generate_indices();
            layers.update_owner_texture();
            layers.commit_owner_texture_to_gpu();
            layers.indices.load_province_texture_to_gpu();
        }
        {
            state::layer l {};
            l.path = "./editor-input";
            parsers::load_layer(l);
            l.load_state_texture_to_gpu();
            l.load_sea_texture_to_gpu();
            layers.data.push_back(l);
            layers.current_layer_index = 1;
            layers.generate_indices();
            layers.update_owner_texture();
            layers.commit_owner_texture_to_gpu();
            layers.indices.load_province_texture_to_gpu();
        }
    }
    return EXIT_SUCCESS;
}