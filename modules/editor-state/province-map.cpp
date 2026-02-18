#include "province-map.hpp"
#include <iostream>
#include "inline.hpp"
#include <cstdint>
#include "../map/unordered_dense.h"
#include "../glm/ext/vector_float2.hpp"
#include "../glm/ext/vector_int2.hpp"

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

	auto converted_color = datatypes::uint_to_r_g_b(available_color);
	available_r = converted_color.r;
	available_g = converted_color.g;
	available_b = converted_color.b;
}

void province_map::recalculate_present_colors() {
	color_present.clear();
	color_present.resize(256 * 256 * 256);

	for (auto i = 0; i < size_x * size_y; i++) {
		// std::cout << i << " ";
		auto r = provinces_image_data[4 * i + 0];
		auto g = provinces_image_data[4 * i + 1];
		auto b = provinces_image_data[4 * i + 2];
		auto rgb = datatypes::rgb_to_uint(r, g, b);
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
			local_rgb = datatypes::rgb_to_uint(r, g, b);
		}

		uint32_t bottom_rgb;
		{
			auto r = provinces_image_data[4 * bottom + 0];
			auto g = provinces_image_data[4 * bottom + 1];
			auto b = provinces_image_data[4 * bottom + 2];
			bottom_rgb = datatypes::rgb_to_uint(r, g, b);
		}

		uint32_t right_rgb;
		{
			auto r = provinces_image_data[4 * right + 0];
			auto g = provinces_image_data[4 * right + 1];
			auto b = provinces_image_data[4 * right + 2];
			right_rgb = datatypes::rgb_to_uint(r, g, b);
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

int province_map::coord_to_pixel(glm::ivec2 coord) {
	return coord.y * size_x + coord.x;
}
int province_map::coord_to_pixel(glm::vec2 coord) {
	return int(std::floor(coord.y))
	* size_x
	+ int(std::floor(coord.x));
}

// constructor from dim
province_map::province_map(int x, int y, bool full_globe) {
	size_x = x;
	size_y = y;
	provinces_image_data = new uint8_t[size_x * size_y * 4];
	full_globe = full_globe;
}

// constructor from dim and data
province_map::province_map(int x, int y, uint8_t* data, bool full_globe) {
	size_x = x;
	size_y = y;
	provinces_image_data = data;
	full_globe = full_globe;
}

// destructor
province_map::~province_map()
{
	delete[] provinces_image_data;
}

// copy constructor
province_map::province_map(const province_map& source) {
	// delete old data
	delete[] provinces_image_data;

	size_x = source.size_x;
	size_y = source.size_y;

	// create new data and copy values there
	provinces_image_data = new uint8_t[size_x * size_y * 4];
	std::copy(source.provinces_image_data, source.provinces_image_data + size_x * size_y * 4, provinces_image_data);
	color_present = source.color_present;
	available_r = source.available_r;
	available_g = source.available_g;
	available_b = source.available_b;
	full_globe = source.full_globe;
}

// copy assignment
province_map& province_map::operator=(province_map& source) {
	if (&source == this)
			return *this;

	delete[] provinces_image_data;

	size_x = source.size_x;
	size_y = source.size_y;

	provinces_image_data = new uint8_t[size_x * size_y * 4];
	std::copy(source.provinces_image_data, source.provinces_image_data + size_x * size_y * 4, provinces_image_data);
	color_present = source.color_present;
	available_r = source.available_r;
	available_g = source.available_g;
	available_b = source.available_b;
	full_globe = source.full_globe;

	return *this;
}

// move constructor
province_map::province_map
	(province_map&& source)
noexcept :
	full_globe(std::move(source.full_globe)),
	size_x(std::move(source.size_x)),
	size_y(std::move(source.size_y)),
	available_r(std::move(source.available_r)),
	available_g(std::move(source.available_g)),
	available_b(std::move(source.available_b)),
	color_present(std::move(source.color_present)),
	provinces_image_data(source.provinces_image_data)
{
	source.clear();
}

// move assignment
province_map& province_map::operator=(province_map&& source) noexcept {
	if (&source == this)
			return *this;

	// delete whatever we had in the array
	delete[] provinces_image_data;
	// steal pointer
	provinces_image_data = std::move(source.provinces_image_data);
	size_x = std::move(source.size_x);
	size_y = std::move(source.size_y);
	available_r = std::move(source.available_r),
	available_g = std::move(source.available_g);
	available_b = std::move(source.available_b);
	color_present = std::move(source.color_present);
	source.clear();
	return *this;
}

int province_map::screen_to_pixel(glm::vec2 screen) {
	auto w = size_x;
	return int(std::floor(screen.y) * w + std::floor(screen.x));
}
}