#pragma once
#include <cstdint>
#include "GL/glew.h"
#include <string>
#undef max
#undef min
#undef clamp

namespace ogl {
struct interface_dds_image {
	uint8_t* data = nullptr;
	int size_x;
	int size_y;
	int channels;

	GLuint texture_id = 0;

	interface_dds_image& operator=(interface_dds_image& source);

	bool valid();
	bool load(std::string path);
	void save(std::string path);
	// initial loading
	void upload_to_gpu();
	// update existing texture
	void commit_to_gpu();
	void expand_image_right(int amount);
	void erase_width(int start_from, int up_to);
	void insert_width(int start, int width);
	void replace_area(
		int x, int y, int w, int h,
		uint8_t* source,
		int source_x,
		int source_y,
		int source_channels
	);
};
};