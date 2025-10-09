#pragma once
#include <cstdint>
#include <vector>
#include <../glm/fwd.hpp>

namespace state {

struct province_map {
	int size_x = 0;
	int size_y = 0;
	uint8_t* provinces_image_data = nullptr;

	uint32_t available_color = 0;
	uint8_t available_r = 0;
	uint8_t available_g = 0;
	uint8_t available_b = 0;

	std::vector<uint8_t> color_present {};

	void clear();
	void update_available_colors();
	void recalculate_present_colors();
	void populate_adjacent_colors(uint32_t rgb, std::vector<uint32_t> & result);

	int coord_to_pixel(glm::ivec2 coord);
	int coord_to_pixel(glm::vec2 coord);

	// constructor from dim
	province_map(int x, int y);

	// constructor from dim and data
	province_map(int x, int y, uint8_t* data);

	// destructor
	~province_map();

	// copy constructor
	province_map(const province_map& source);

	// copy assignment
	province_map& operator=(province_map& source);

	// move constructor
	province_map(province_map&& source) noexcept;

	// move assignment
	province_map& operator=(province_map&& source) noexcept;

	int screen_to_pixel(glm::vec2 screen);
};

}