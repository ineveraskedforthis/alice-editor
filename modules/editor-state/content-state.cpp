#include "SOIL2.h"
#include "content-state.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include "../gl-wrapper/inline.hpp"

namespace state {
glm::vec2 screen_to_texture(
    int x_in,
    int y_in,
    float width_texture,
    float height_texture,
    float width_screen,
    float height_screen,
    float zoom,
    glm::vec2 camera_shift
) {
    float x_adjusted = ((float)x_in - (float)width_screen / 2.f);
    float y_adjusted = ((float)y_in - (float)height_screen / 2.f);
    glm::vec2 shift_true = camera_shift * glm::vec2((float)width_screen, (float)width_screen) / 2.f;
    float x = (x_adjusted) * zoom + (float)width_texture / 2.f - shift_true.x;
    float y = height_texture - ((y_adjusted) * zoom + (float)height_texture / 2.f) - shift_true.y ;

    return glm::vec2(x, y);
}

void layer::load_sea_texture_to_gpu() {
    glGenTextures(1, &sea_texture);
    glBindTexture(GL_TEXTURE_2D, sea_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        256,
        256,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        province_is_sea
    );
    ogl::check_gl_error("Sea texture loading to gpu");
}
void layer::commit_sea_texture_to_gpu() {
    glBindTexture(GL_TEXTURE_2D, sea_texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        256,
        256,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        province_is_sea
    );
}

void layer::load_state_texture_to_gpu() {
    glGenTextures(1, &state_texture);
    glBindTexture(GL_TEXTURE_2D, state_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RG,
        256,
        256,
        0,
        GL_RG,
        GL_UNSIGNED_BYTE,
        province_state
    );
    ogl::check_gl_error("State texture update");
}
void layer::commit_state_texture_to_gpu() {
    glBindTexture(GL_TEXTURE_2D, state_texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RG,
        256,
        256,
        0,
        GL_RG,
        GL_UNSIGNED_BYTE,
        province_state
    );
}

province_texture& province_texture::operator=(province_texture& source) {
    size_x = source.size_x;
    size_y = source.size_y;
    v2id_to_size = source.v2id_to_size;
    v2id_to_mean = source.v2id_to_mean;
    v2id_exists = source.v2id_exists;
    delete[] data;
    data = new uint8_t[size_x * size_y * 4];
    std::copy(source.data, source.data + size_x * size_y * 4, data);
    load_province_texture_to_gpu();
    return *this;
}


province_texture::province_texture(){
    v2id_to_size.resize(256 * 256);
    v2id_exists.resize(256 * 256);
    v2id_to_mean.resize(256 * 256);
};

int province_texture::coord_to_pixel(glm::ivec2 coord) {
    return coord.y * size_x + coord.x;
}
int province_texture::coord_to_pixel(glm::vec2 coord) {
    return int(std::floor(coord.y))
        * size_x
        + int(std::floor(coord.x));
}
void province_texture::load_province_texture_to_gpu() {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        size_x,
        size_y,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data
    );
    ogl::check_gl_error("Map texture update");
}
void province_texture::commit_province_texture_changes_to_gpu() {
    if (update_texture) {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            size_x,
            size_y,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data
        );
        update_texture = false;
        update_texture_part = false;
    }

    if (update_texture_part) {
        glBindTexture(GL_TEXTURE_2D, id);
        auto width = update_texture_x_top - update_texture_x_bottom + 1;
        for (int y = update_texture_y_bottom; y <= update_texture_y_top; y++) {
            auto pixel_index = coord_to_pixel(glm::ivec2{update_texture_x_bottom, y});
            glTexSubImage2D(
                GL_TEXTURE_2D,
                0,
                update_texture_x_bottom,
                y,
                width,
                1,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                data + pixel_index * 4
            );
        }

        update_texture_x_top = 0;
        update_texture_y_top = 0;
        update_texture_x_bottom = std::numeric_limits<int>::max();
        update_texture_y_bottom = std::numeric_limits<int>::max();
        update_texture_part = false;
    }
}



// TODO
void layers_stack::save_population_texture() {
    auto size_x = get_provinces_image_x();
    auto size_y = get_provinces_image_y();
    auto image_data = new uint8_t[size_x * size_y];

    auto density = std::vector<float> {};
    density.resize(256 * 256);

    // get latest history
    layer * result = &data[0];

    for (size_t i = 0; i < data.size(); i++) {
        if(data[i].visible && data[i].has_province_definitions) {
            result = &data[i];
        }
    }

    for (auto date : get_available_dates()){
        for (auto prov: result->province_definitions) {
            auto v2id = prov.v2id;
            auto province_size = indices.v2id_to_size[v2id];
            auto population = get_pops(v2id, date);
            if (population == nullptr) {
                continue;
            }
            auto total = 0;
            for (auto& pop: *population) {
                if (pop.culture == "north_german" && pop.religion == "protestant" && pop.poptype == "artisans") {
                    total += pop.size;
                }
            }

            density[v2id] = std::clamp((float)total / (float)province_size, 0.f, 255.f);
        }

        for (int i = 0; i < size_x; i++) {
            for(int j = 0; j < size_y; j++) {
                auto v2id = sample_province_index(i + j * size_x);
                auto local_density = (uint8_t) density[v2id];
                image_data[i + j * size_x] = local_density;
            }
        }
        break;
    }

    auto image_png_name = "protestant-north_german-artisans.png";
    auto image_jxl_name = "protestant-north_german-artisans.jxl";

    SOIL_save_image(
        "./editor-output/1/protestant-north_german-artisans.png",
        SOIL_SAVE_TYPE_PNG,
        size_x,
        size_y,
        1,
        image_data
    );

    // std::string command = "cjxl.exe";
    // std::string path = "./editor-output/1/";
    // command += " " + path + image_png_name;
    // command += " " + path + image_jxl_name;
    // command += " --effort=10";

    // system(command.c_str());
}

}