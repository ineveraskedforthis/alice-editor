#include "texture.hpp"
#include <cmath>
#include <cstdint>
#include "inline.hpp"
#include "../glm/ext/vector_float2.hpp"
#include "../glm/ext/vector_int2.hpp"

namespace ogl {

glm::ivec2 data_texture::pixel_to_coord(int pixel) {
	int coord_y = pixel / size_x;
        int coord_x = pixel - (coord_y * size_x);
	return {coord_x, coord_y};
};

int data_texture::coord_to_pixel(glm::ivec2 coord) {
	return coord.y * size_x + coord.x;
}

int data_texture::coord_to_pixel(int x, int y) {
	return y * size_x + x;
}

int data_texture::coord_to_pixel(glm::vec2 coord) {
	return int(std::floor(coord.y))
		* size_x
		+ int(std::floor(coord.x));
}
void data_texture::clear(int new_x, int new_y, int new_channels) {
	delete[] data;
	size_x = new_x;
	size_y = new_y;
	channels = new_channels;
	if (channels == 1) {
		internal_format = GL_RGBA;
	} else {
		internal_format = GL_RGBA;
	}
	if (channels == 1) {
		format = GL_RED;
	} else if (channels == 2) {
		format = GL_RG;
	} else if (channels == 3) {
		format = GL_RGB;
	} else if (channels == 4) {
		format = GL_RGBA;
	}
	data = new uint8_t[size_x * size_y * channels];
}
void data_texture::upload_to_gpu() {
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		internal_format,
		size_x,
		size_y,
		0,
		format,
		GL_UNSIGNED_BYTE,
		data
	);
	check_gl_error("Map texture update");
}

void data_texture::internal_commit_to_gpu_full() {
	glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexImage2D(
		GL_TEXTURE_2D,
		0,
		internal_format,
		size_x,
		size_y,
		0,
		format,
		GL_UNSIGNED_BYTE,
		data
	);
}

void data_texture::internal_commit_to_gpu_part() {
	glBindTexture(GL_TEXTURE_2D, texture_id);
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
			format,
			GL_UNSIGNED_BYTE,
			data + pixel_index * channels
		);
	}
}

void data_texture::reset_flags_full() {
	reset_flags_part();
	update_texture = false;
}

void data_texture::reset_flags_part() {
	update_texture_x_top = 0;
	update_texture_y_top = 0;
	update_texture_x_bottom = std::numeric_limits<int>::max();
	update_texture_y_bottom = std::numeric_limits<int>::max();
	update_texture_part = false;
}

void data_texture::commit_to_gpu() {

	if (update_texture) {
		internal_commit_to_gpu_full();
		reset_flags_full();
	}

	if (update_texture_part) {
		internal_commit_to_gpu_part();
		reset_flags_part();
	}

	ogl::check_gl_error("Commit to gpu");
}

void data_texture::add_r(int x, int y, uint8_t value) {
	if (x < 0) {
		return;
	}
	if (y < 0) {
		return;
	}
	if (x >= size_x) {
		return;
	}
	if (y >= size_y) {
		return;
	}
	auto pixel = coord_to_pixel(x, y);
	uint8_t old_value = data[pixel * channels];
	uint8_t new_value = old_value + value;
	if (new_value < value) {
		new_value = 255;
	}
	data[pixel * channels] = new_value;
	update_texture_part = true;
	int coord_y = pixel / x;
	int coord_x = pixel - (coord_y * x);
	update_texture_x_bottom = std::min(x, update_texture_x_bottom);
	update_texture_y_bottom = std::min(y, update_texture_y_bottom);
	update_texture_x_top = std::max(x, update_texture_x_top);
	update_texture_y_top = std::max(y, update_texture_y_top);
}

void data_texture::add_r(int pixel_index, uint8_t value) {
	auto coords = pixel_to_coord(pixel_index);
	add_r(coords.x, coords.y, value);
}

}