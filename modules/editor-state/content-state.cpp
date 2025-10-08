#include "SOIL2.h"
#include "content-state.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

namespace state {

void province_map::clear() {
    size_x = 0;
    size_y = 0;
    provinces_image_data = nullptr;
    available_color = 0;
    available_r = 0;
    available_g = 0;
    available_b = 0;
    color_present.clear();
}

void province_map::update_available_colors() {
    std::cout << "Update available colors\n";
    auto starting_color = available_color;

    while (color_present[available_color]) {
        available_color += 97;
        available_color = available_color % (256 * 256 * 256);
        if (starting_color == available_color) {
            break;
        }
    }

    auto converted_color = uint_to_r_g_b(available_color);
    available_r = converted_color.r;
    available_g = converted_color.g;
    available_b = converted_color.b;

    // for (int _r = 0; _r < 256; _r++)
    //     for (int _g = 0; _g < 256; _g++)
    //         for (int _b = 0; _b < 256; _b++) {
    //             auto rgb = rgb_to_uint(_r, _g, _b);
    //             if (!color_present[rgb]) {
    //                 available_r = _r;
    //                 available_g = _g;
    //                 available_b = _b;
    //                 return;
    //             }
    //         }
}

void province_map::recalculate_present_colors() {
    color_present.clear();
    color_present.resize(256 * 256 * 256);

    for (auto i = 0; i < size_x * size_y; i++) {
        // std::cout << i << " ";
        auto r = provinces_image_data[4 * i + 0];
        auto g = provinces_image_data[4 * i + 1];
        auto b = provinces_image_data[4 * i + 2];
        auto rgb = rgb_to_uint(r, g, b);
        color_present[rgb] = true;
    }
}

void province_map::populate_adjacent_colors(uint32_t rgb, std::vector<uint32_t> & result) {
    ankerl::unordered_dense::map<uint32_t, bool> temp_result {};
    for (auto x = 0; x < size_x - 1; x++) {
        for (auto y = 0; y < size_y - 1; y++) {
            auto local = y * size_x + x;
            auto bottom = (y + 1) * size_x + (x);
            auto right = (y) * size_x + (x + 1);

            uint32_t local_rgb;
            {
                auto r = provinces_image_data[4 * local + 0];
                auto g = provinces_image_data[4 * local + 1];
                auto b = provinces_image_data[4 * local + 2];
                local_rgb = rgb_to_uint(r, g, b);
            }

            uint32_t bottom_rgb;
            {
                auto r = provinces_image_data[4 * bottom + 0];
                auto g = provinces_image_data[4 * bottom + 1];
                auto b = provinces_image_data[4 * bottom + 2];
                bottom_rgb = rgb_to_uint(r, g, b);
            }

            uint32_t right_rgb;
            {
                auto r = provinces_image_data[4 * right + 0];
                auto g = provinces_image_data[4 * right + 1];
                auto b = provinces_image_data[4 * right + 2];
                right_rgb = rgb_to_uint(r, g, b);
            }


            if (local_rgb == rgb) {
                temp_result[bottom_rgb] = true;
                temp_result[right_rgb] = true;
            }
            if (bottom_rgb == rgb) {
                temp_result[local_rgb] = true;
            }
            if (right_rgb == rgb) {
                temp_result[local_rgb] = true;
            }
        }
    }
    for (auto [key, value] : temp_result) {
        result.push_back(key);
    }
}

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
    check_gl_error("Sea texture loading to gpu");
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
    check_gl_error("State texture update");
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

interface_dds_image& interface_dds_image::operator=(interface_dds_image& source) {
    size_x = source.size_x;
    size_y = source.size_y;
    channels = source.channels;

    delete[] data;
    data = new uint8_t[size_x * size_y * channels];
    std::copy(source.data, source.data + size_x * size_y * channels, data);
    upload_to_gpu();
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



bool interface_dds_image::valid() {
    return !(data == nullptr);
}

bool interface_dds_image::load(std::string path) {
    std::cout << "loading image:" << path << "\n";
    auto image_exists = std::filesystem::exists(path);
    if (!image_exists) {
        return false;
    }
    data = SOIL_load_image(
        path.c_str(),
        &size_x,
        &size_y,
        &channels,
        SOIL_LOAD_AUTO
    );
    upload_to_gpu();

    return true;
}

void interface_dds_image::save(std::string path) {
    if (valid()) {
        SOIL_save_image(
            path.c_str(),
            SOIL_SAVE_TYPE_DDS,
            size_x,
            size_y,
            channels,
            data
        );
    }
}

void interface_dds_image::expand_image_right(int amount) {
    if (!valid()) {
        return;
    }
    if (amount < 0) {
        return;
    }

    auto new_size_x = size_x + amount;

    uint8_t* temp = new uint8_t[new_size_x * size_y * channels];

    // copy data
    // TODO: copy entire lines instead pixel by pixel
    for (int i = 0; i < size_x; i++) {
        for (int j = 0; j < size_y; j++) {
            auto temp_pixel = i + j * new_size_x;
            auto old_pixel =  i + j * size_x;
            temp[temp_pixel * 4 + 0] = data[old_pixel * 4 + 0];
            temp[temp_pixel * 4 + 1] = data[old_pixel * 4 + 1];
            temp[temp_pixel * 4 + 2] = data[old_pixel * 4 + 2];
            if (channels > 3) {
                temp[temp_pixel * 4 + 3] = data[old_pixel * 4 + 3];
            }
        }
    }

    size_x = new_size_x;

    delete [] data;
    data = temp;

    commit_to_gpu();
}

void interface_dds_image::replace_area(
    int x, int y, int w, int h,
    uint8_t* source_data, int source_x, int source_y, int data_channels
) {
    float width_per_pixel = (float)source_x / float(w);
    float height_per_pixel = (float)source_y / float(h);

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            float total_count = 0.f;
            float total_r = 0.f;
            float total_g = 0.f;
            float total_b = 0.f;
            float total_a = 0.f;

            for (int shift_i = 0; shift_i < width_per_pixel; shift_i++) {
                for (int shift_j = 0; shift_j < height_per_pixel; shift_j++) {
                    float width = 1.f;
                    if (width_per_pixel - (float)shift_i < 1.f) {
                        width = width_per_pixel - (float)shift_i;
                    }
                    float height = 1.f;
                    if (height_per_pixel - (float)shift_j < 1.f) {
                        height = height_per_pixel - (float)shift_j;
                    }

                    float area = width * height;
                    total_count += area;

                    int final_i = (int)(i * width_per_pixel + shift_i);
                    int final_j = (int)(j * height_per_pixel + shift_j);
                    int pixel = final_i + final_j * source_x;

                    total_r += source_data[pixel * data_channels + 0];
                    total_g += source_data[pixel * data_channels + 1];
                    total_b += source_data[pixel * data_channels + 2];
                    if (data_channels > 3) {
                        total_a += source_data[pixel * data_channels + 3];
                    } else {
                        total_a += area * 255.f;
                    }
                }
            }

            int strip_i = x + i;
            int strip_j = y + j;
            int pixel = strip_i + strip_j * size_x;

            data[pixel * channels + 0] = static_cast<uint8_t>(std::clamp(total_r / total_count, 0.f, 255.f));
            data[pixel * channels + 1] = static_cast<uint8_t>(std::clamp(total_g / total_count, 0.f, 255.f));
            data[pixel * channels + 2] = static_cast<uint8_t>(std::clamp(total_b / total_count, 0.f, 255.f));
            if (channels > 3) {
                data[pixel * channels + 3] = static_cast<uint8_t>(std::clamp(total_a / total_count, 0.f, 255.f));
            }
        }
    }

    commit_to_gpu();
}

void interface_dds_image::erase_width(int start, int end) {
    if (!valid()) {
        return;
    }
    if (start >= end) {
        return;
    }

    auto erase_width = end - start;

    auto new_size_x = size_x - erase_width;

    uint8_t* temp = new uint8_t[new_size_x * size_y * channels];

    // copy data
    // TODO: copy entire lines instead pixel by pixel
    for (int i = 0; i < start; i++) {
        for (int j = 0; j < size_y; j++) {
            auto temp_pixel = i + j * new_size_x;
            auto old_pixel =  i + j * size_x;
            temp[temp_pixel * 4 + 0] = data[old_pixel * 4 + 0];
            temp[temp_pixel * 4 + 1] = data[old_pixel * 4 + 1];
            temp[temp_pixel * 4 + 2] = data[old_pixel * 4 + 2];
            if (channels > 3) {
                temp[temp_pixel * 4 + 3] = data[old_pixel * 4 + 3];
            }
        }
    }

    for (int i = end; i < size_x; i++) {
        for (int j = 0; j < size_y; j++) {
            auto temp_pixel = i - end + start + j * new_size_x;
            auto old_pixel =  i + j * size_x;
            temp[temp_pixel * 4 + 0] = data[old_pixel * 4 + 0];
            temp[temp_pixel * 4 + 1] = data[old_pixel * 4 + 1];
            temp[temp_pixel * 4 + 2] = data[old_pixel * 4 + 2];
            if (channels > 3) {
                temp[temp_pixel * 4 + 3] = data[old_pixel * 4 + 3];
            }
        }
    }

    size_x = new_size_x;

    delete [] data;
    data = temp;

    commit_to_gpu();
}

void interface_dds_image::insert_width(int start, int width) {
    if (!valid()) {
        return;
    }
    if (width < 0) {
        return;
    }

    auto new_size_x = size_x + width;

    uint8_t* temp = new uint8_t[new_size_x * size_y * channels];

    // copy data
    // TODO: copy entire lines instead pixel by pixel
    for (int i = 0; i < start; i++) {
        for (int j = 0; j < size_y; j++) {
            auto temp_pixel = i + j * new_size_x;
            auto old_pixel =  i + j * size_x;
            temp[temp_pixel * 4 + 0] = data[old_pixel * 4 + 0];
            temp[temp_pixel * 4 + 1] = data[old_pixel * 4 + 1];
            temp[temp_pixel * 4 + 2] = data[old_pixel * 4 + 2];
            if (channels > 3) {
                temp[temp_pixel * 4 + 3] = data[old_pixel * 4 + 3];
            }
        }
    }

    for (int i = start; i < size_x; i++) {
        for (int j = 0; j < size_y; j++) {
            auto temp_pixel = i + width + j * new_size_x;
            auto old_pixel =  i + j * size_x;
            temp[temp_pixel * 4 + 0] = data[old_pixel * 4 + 0];
            temp[temp_pixel * 4 + 1] = data[old_pixel * 4 + 1];
            temp[temp_pixel * 4 + 2] = data[old_pixel * 4 + 2];
            if (channels > 3) {
                temp[temp_pixel * 4 + 3] = data[old_pixel * 4 + 3];
            }
        }
    }

    size_x = new_size_x;

    delete [] data;
    data = temp;

    commit_to_gpu();
}

void interface_dds_image::upload_to_gpu() {
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    auto format = GL_RGBA;
    if (channels == 3) {
        format = GL_RGB;
    }
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        size_x,
        size_y,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );
}

void interface_dds_image::commit_to_gpu() {
    glBindTexture(GL_TEXTURE_2D, texture_id);
    auto format = GL_RGBA;
    if (channels == 3) {
        format = GL_RGB;
    }
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        size_x,
        size_y,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );
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