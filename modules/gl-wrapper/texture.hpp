#pragma once
#include <cstdint>
#include <limits>
#include "GL/glew.h"
#include "../glm/fwd.hpp"
#include <limits>
#undef max
#undef min
#undef clamp

namespace ogl {
struct data_texture {
	int size_x = 0;
	int size_y = 0;
	int channels = 1;

	uint8_t* data = nullptr;

	GLuint texture_id = 0;
	GLint internal_format;
	GLint format;

	bool update_texture = false;
	bool update_texture_part = false;
	int update_texture_x_top = 0;
	int update_texture_y_top = 0;
	int update_texture_x_bottom = std::numeric_limits<int>::max();
	int update_texture_y_bottom = std::numeric_limits<int>::max();
	void reset_flags_full();
	void reset_flags_part();

	// data_texture& operator=(data_texture& source);

	int coord_to_pixel(glm::ivec2 coord);
	int coord_to_pixel(int x, int y);
	int coord_to_pixel(glm::vec2 coord);
	glm::ivec2 pixel_to_coord(int pixel);
	void upload_to_gpu();
	void internal_commit_to_gpu_full();
	void internal_commit_to_gpu_part();
	void commit_to_gpu();
	void clear(int x, int y, int channels);

	void add_r(int x, int y, uint8_t value);
	void add_r(int pixel, uint8_t value);
};
}