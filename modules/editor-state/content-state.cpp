#include "content-state.hpp"

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

province_texture& province_texture::operator=(province_texture& source) {
    size_x = source.size_x;
    size_y = source.size_y;
    delete[] data;
    data = new uint8_t[size_x * size_y * 4];
    std::copy(source.data, source.data + size_x * size_y * 4, data);
    load_province_texture_to_gpu();
    return *this;
}

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
    check_gl_error("Map texture update");
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
        auto width = update_texture_x_top - update_texture_x_bottom;
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
}