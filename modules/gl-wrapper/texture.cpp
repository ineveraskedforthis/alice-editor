#include "texture.hpp"
#include <cmath>
#include "inline.hpp"
#include "../glm/ext/vector_float2.hpp"
#include "../glm/ext/vector_int2.hpp"

namespace ogl {
int data_texture::coord_to_pixel(glm::ivec2 coord) {
	return coord.y * size_x + coord.x;
}
int data_texture::coord_to_pixel(glm::vec2 coord) {
	return int(std::floor(coord.y))
		* size_x
		+ int(std::floor(coord.x));
}
void data_texture::upload_to_gpu() {
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
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

void data_texture::internal_commit_to_gpu_full() {
	glBindTexture(GL_TEXTURE_2D, texture_id);
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
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			data + pixel_index * 4
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
}
}