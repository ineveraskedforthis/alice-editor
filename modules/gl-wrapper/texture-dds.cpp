#include "SOIL2.h"
#include "texture-dds.hpp"
#include <iostream>
#include <filesystem>

namespace ogl {

bool interface_dds_image::valid() {
	return !(data == nullptr);
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

}