#pragma once

#include <cstdint>

namespace datatypes {
struct color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};
color inline uint_to_r_g_b(uint32_t c) {
	auto r = c >> 16;
	c -= (r << 16);
	auto g = c >> 8;
	c -= (g << 8);
	auto b = c;
	return {(uint8_t)r, (uint8_t)g, (uint8_t)b};
}
// converts rgb to a single number
uint32_t inline rgb_to_uint(int r, int g, int b) {
	return (r << 16) + (g << 8) + b;
}
}