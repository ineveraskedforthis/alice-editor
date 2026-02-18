#pragma once
#include <array>
#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "parsers.hpp"
#include "../definitions.hpp"
#include "../conversion.hpp"

namespace state {
	struct layer;
	struct layers_stack;
	struct gfx_file;
};

namespace parsers {

struct color_from_3i {
	int index = 0;
	std::array<uint8_t, 3> colors;
	template<typename C>
	void finish(C& context) {
		if (index == 1) {
			colors[1] = colors[0];
			colors[2] = colors[0];
		}
	}
	template<typename C>
	void free_value(int32_t v, error_handler& err, int32_t line, C& context) {
		colors[index] = (uint8_t)v;
		index++;
	}
};

struct color_from_3f {
	int index = 0;
	std::array<float, 3> colors;
	template<typename C>
	void finish(C& context) {
		if (index == 1) {
			colors[1] = colors[0];
			colors[2] = colors[0];
		}
	}
	template<typename C>
	void free_value(float v, error_handler& err, int32_t line, C& context) {
		colors[index] = (uint8_t)v;
		index++;
	}
};

struct generic_context{
	state::layer& map;
};

struct gfx_file_context{
	state::layer& map;
	state::gfx_file& gfx_file;
};

struct commodity_group_context {
	state::layer& map;
	std::string group;
};

struct culture_file_context{
	state::layer& map;
};

struct culture_file {
	void finish(culture_file_context&) {}
};
struct religion_file {
	void finish(generic_context&) {}
};
struct culture_group_context {
	state::layer& map;
	game_definition::culture_group& culture_group;
};
struct religion_group_context {
	state::layer& map;
	std::string name;
};

struct culture_context {
	state::layer& map;
	game_definition::culture& culture;
};

struct group_of_strings {
	std::vector<std::string> data;

	template<typename C>
	void finish(C&) {}

	template<typename C>
	void free_value(std::string_view value, error_handler& err, int32_t line, C& context) {
		std::string actual_value {value};
		data.push_back(actual_value);
	}
};

struct culture {
	void finish(culture_context&) {}
	void color(color_from_3i value, error_handler& err, int32_t line, culture_context& context) {
		context.culture.r = value.colors[0];
		context.culture.g = value.colors[1];
		context.culture.b = value.colors[2];
	};
	void first_names(group_of_strings value, error_handler& err, int32_t line, culture_context& context) {
		for (auto& s: value.data) {
			context.culture.first_names.push_back(conversions::u16_to_u8(conversions::win1252_to_u16(s)));
		}
	};
	void last_names(group_of_strings value, error_handler& err, int32_t line, culture_context& context) {
		for (auto& s: value.data) {
			context.culture.last_names.push_back(conversions::u16_to_u8(conversions::win1252_to_u16(s)));
		}
	};
	void male_names(group_of_strings value, error_handler& err, int32_t line, culture_context& context) {
		for (auto& s: value.data) {
			context.culture.first_names.push_back(conversions::u16_to_u8(conversions::win1252_to_u16(s)));
		}
	};
	void female_names(group_of_strings value, error_handler& err, int32_t line, culture_context& context) {
		for (auto& s: value.data) {
			context.culture.first_names.push_back(conversions::u16_to_u8(conversions::win1252_to_u16(s)));
		}
	};
	void dynasty_names(group_of_strings value, error_handler& err, int32_t line, culture_context& context) {
		for (auto& s: value.data) {
			context.culture.last_names.push_back(conversions::u16_to_u8(conversions::win1252_to_u16(s)));
		}
	};
	void primary(association_type, std::string_view value, error_handler& err, int32_t line, culture_context& context) {
		std::string actual_value {value};
		context.culture.primary = actual_value;
	};
	void radicalism(association_type, int value, error_handler& err, int32_t line, culture_context& context) {
		context.culture.radicalism = value;
	};
};

struct culture_group {
	void finish(culture_group_context&) {}
	void union_tag(association_type, std::string_view value, error_handler& err, int32_t line, culture_group_context& context) {
		std::string actual_value {value};
		context.culture_group.union_tag = actual_value;
	};
	void leader(association_type, std::string_view value, error_handler& err, int32_t line, culture_group_context& context) {
		std::string actual_value {value};
		context.culture_group.leader = actual_value;
	};
	void unit(association_type, std::string_view value, error_handler& err, int32_t line, culture_group_context& context) {
		std::string actual_value {value};
		context.culture_group.unit = actual_value;
	};
	void is_overseas(association_type, bool value, error_handler& err, int32_t line, culture_group_context& context) {
		context.culture_group.is_overseas = value;
	};
	void male_names(group_of_strings value, error_handler& err, int32_t line, culture_group_context& context) {
		for (auto& s: value.data) {
			context.culture_group.first_names.push_back(conversions::u16_to_u8(conversions::win1252_to_u16(s)));
		}
	};
	void female_names(group_of_strings value, error_handler& err, int32_t line, culture_group_context& context) {
		for (auto& s: value.data) {
			context.culture_group.first_names.push_back(conversions::u16_to_u8(conversions::win1252_to_u16(s)));
		}
	};
	void dynasty_names(group_of_strings value, error_handler& err, int32_t line, culture_group_context& context) {
		for (auto& s: value.data) {
			context.culture_group.last_names.push_back(conversions::u16_to_u8(conversions::win1252_to_u16(s)));
		}
	};
};
struct province_history_context {
	game_definition::province_history& history;
	// game_definition::pops_history_file& extrapolated_population;
	std::optional<ankerl::unordered_dense::map<uint32_t, bool>> data;
};

struct finite_predicate {
	// none if total
	std::optional<ankerl::unordered_dense::map<uint32_t, bool>> data;
};
inline void multiply(finite_predicate& a, const finite_predicate& b) {
	if (b.data && a.data) {
		for (auto& [key, value] : b.data.value()) {
			auto a_it = a.data->find(key);
			if (a_it != a.data->end())
				a.data.value()[key] = true;
		}
		std::vector<uint32_t> to_erase;
		for (auto& [key, value] : a.data.value()) {
			auto b_it = b.data->find(key);
			if (b_it == b.data->end())
				to_erase.push_back(key);
		}
		for (auto& key : to_erase) {
			a.data->erase(key);
		}
	} else if (b.data) {
		a.data = b.data;
	}
}
inline void unite(finite_predicate& a, const finite_predicate& b) {
	if (a.data && b.data) {
		for (auto& [key, value] : b.data.value()) {
			a.data.value()[key] = true;
		}
	}
}
inline void exclude(finite_predicate& a, const finite_predicate& b) {
	assert(a.data.has_value());
	assert(b.data.has_value());
	for (auto& [key, value] : b.data.value()) {
		if (a.data->contains(key)){
			a.data->erase(key);
		}
	}
}

enum class trigger_environment {
	pdx_or, pdx_and
};
struct bundled_trigger {
	ankerl::unordered_dense::map<uint32_t, bool> included_provinces {};
	trigger_environment env;
	bool negation;
};
struct region_trigger_context {
	state::layer& map;
	bundled_trigger& current;
	std::vector<bundled_trigger> inputs;
};

struct region_triggers_file {
	void finish(generic_context& ctx) {}
};

struct region_trigger {
	void finish(region_trigger_context& ctx);

	void province_id(association_type, uint32_t v2id, error_handler& err, int32_t line, region_trigger_context& context);
	void area(association_type, std::string_view text, error_handler& err, int32_t line, region_trigger_context& context);
	void region(association_type, std::string_view text, error_handler& err, int32_t line, region_trigger_context& context);
};

void make_region_trigger_or(token_generator& gen, error_handler& err, region_trigger_context& context);
void make_region_trigger_and(token_generator& gen, error_handler& err, region_trigger_context& context);
void make_region_trigger_not_and(token_generator& gen, error_handler& err, region_trigger_context& context);
void make_region_trigger(std::string_view name, token_generator& gen, error_handler& err, generic_context& context);


struct region_context {
	state::layer& map;
	std::string current_region;
};


struct eu4_region_areas {
	std::vector<std::string> values;
	template<typename C>
	void free_value(std::string_view v, error_handler& err, int32_t line, C& context) {
		auto vr = std::string{v};
		values.push_back(vr);
	}

	template<typename C>
	void finish(C& context) {};
};

struct eu4_region_content {
	eu4_region_areas areas;
	void finish(region_context& context);
};

struct eu4_regions_file {
	void finish(generic_context& context) {};
};

struct setter_meiou {
	std::string lhs;
	float value;
	void finish(province_history_context& context) {
		if (lhs == "starting_rural_pop_1350") {
			context.history.rural_population = value * 1000.f;
		}
		if (lhs == "starting_urban_pop_1350") {
			context.history.urban_population += value * 1000.f;
		}
		if (lhs == "starting_urban_pop") {
			context.history.urban_population += value * 1000.f;
		}
		if (lhs == "pop_level") {
			context.history.pop_level = value;
		}
	}
};

struct dated_block {
	void finish(province_history_context&) { }
};

void enter_dated_block(std::string_view name, token_generator& gen, error_handler& err, province_history_context& context);
void enter_setter_meiou(token_generator& gen, error_handler& err, province_history_context& context);

struct pv_party_loyalty {
	int32_t loyalty_value = 0;
	std::string party;
	void ideology(association_type, std::string_view text, error_handler& err, int32_t line, province_history_context& context) {
		party = text;
	}
	void finish(province_history_context&) { }
};
struct pv_state_building {
	int32_t level = 1;
	std::string building;
	std::string upgrade;
	void finish(province_history_context&) { }
};

struct province_rgo_ext_desc {
	std::string commodity;
	float max_employment_value;
	void max_employment(association_type, uint32_t value, error_handler& err, int32_t line, province_history_context& context);
	void trade_good(association_type, std::string_view text, error_handler& err, int32_t line, province_history_context& context);
	void finish(province_history_context&);
};

struct province_rgo_ext {
	void entry(province_rgo_ext_desc const& value, error_handler& err, int32_t line, province_history_context& context);
	void finish(province_history_context&) { }
};

// void enter_dated_block(std::string_view name, token_generator& gen, error_handler& err, province_history_context& context);

struct province_rgo_ext_2_desc {
	std::string commodity;
	float max_employment_value;
	void max_employment(association_type, uint32_t value, error_handler& err, int32_t line, province_history_context& context);
	void trade_good(association_type, std::string_view text, error_handler& err, int32_t line, province_history_context& context);
	void finish(province_history_context&);
};

struct province_rgo_ext_2 {
	void entry(province_rgo_ext_2_desc const& value, error_handler& err, int32_t line, province_history_context& context);
	void finish(province_history_context&) { }
};

struct province_revolt {
	std::string rebel;
	void type(parsers::association_type, std::string_view text, error_handler& err, int32_t line, province_history_context& context);
	void finish(province_history_context&) {
	}
};

struct province_history_handler {
	void life_rating(association_type, uint32_t value, error_handler& err, int32_t line, province_history_context& context);
	void colony(association_type, uint32_t value, error_handler& err, int32_t line, province_history_context& context);
	void base_tax(association_type, uint32_t value, error_handler& err, int32_t line, province_history_context& context);
	void base_production(association_type, uint32_t value, error_handler& err, int32_t line, province_history_context& context);
	void trade_goods(association_type, std::string_view text, error_handler& err, int32_t line, province_history_context& context);
	void owner(association_type, std::string_view value , error_handler& err, int32_t line, province_history_context& context);
	void controller(association_type, std::string_view value , error_handler& err, int32_t line, province_history_context& context);
	void terrain(association_type, std::string_view text, error_handler& err, int32_t line, province_history_context& context);
	void culture(association_type, std::string_view text, error_handler& err, int32_t line, province_history_context& context);
	void religion(association_type, std::string_view text, error_handler& err, int32_t line, province_history_context& context);
	void add_core(association_type, std::string_view value , error_handler& err, int32_t line, province_history_context& context);
	void remove_core(association_type, std::string_view value , error_handler& err, int32_t line, province_history_context& context);
	void party_loyalty(pv_party_loyalty const& value, error_handler& err, int32_t line, province_history_context& context);
	void state_building(pv_state_building const& value, error_handler& err, int32_t line, province_history_context& context);
	void is_slave(association_type, bool value, error_handler& err, int32_t line, province_history_context& context);
	void rgo_distribution(province_rgo_ext const& value, error_handler& err, int32_t line, province_history_context& context);
	void rgo_distribution_add(province_rgo_ext_2 const& value, error_handler& err, int32_t line, province_history_context& context);
	// void factory_limit(province_factory_limit const& value, error_handler& err, int32_t line, province_history_context& context);
	void revolt(province_revolt const& rev, error_handler& err, int32_t line, province_history_context& context);
	void any_value(std::string_view name, association_type, uint32_t value, error_handler& err, int32_t line, province_history_context& context);
	void finish(province_history_context&) { }
};

struct religion_group {
	void finish(religion_group_context&) {}
};

struct commodity_context {
	state::layer& map;
	game_definition::commodity& target;
};

struct gfx_sprite_context{
	state::layer& map;
	game_definition::sprite& sprite;
};

struct gfx_projection_context{
	state::layer& map;
	game_definition::projection& sprite;
};

struct generic_global_context {
	state::layers_stack& state;
	state::layer& map;
};

struct issue_context{
	state::layer& map;
	std::string issue_name;
};

struct issue_group_context{
	state::layer& map;
	std::string group;
};

struct technology_context{
	state::layer& map;
	std::wstring folder;
};

struct context_with_file_label{
	state::layer& map;
	std::string file;
};

struct pair_of_x_and_y {
	bool exists = true;
	int x;
	int y;
	template<typename C>
	void finish(C& context) {}
};


struct continent_provinces {
	std::vector<int> provinces_v2ids;

	template<typename C>
	void free_value(int32_t v, error_handler& err, int32_t line, C& context) {
		provinces_v2ids.push_back(v);
	}
	template<typename C>
	void finish(C&) { };
};


struct continent_definition {
	continent_provinces provinces;
	std::vector<game_definition::modifier> modifiers;

	template<typename C>
	void any_value(std::string_view name, association_type, float value, error_handler& err, int32_t line, C& context) {
		std::string name_value {name};
		modifiers.push_back({name_value, value});
	};

	template<typename C>
	void finish(C&) { };
};

struct continent_file {
	void finish(generic_context&) { };
};

void make_continent_definition(std::string_view name, token_generator& gen, error_handler& err, generic_context& context);

struct pop_history_context {
	state::layer& map;
	game_definition::pops_history_file& file;
	int date;
	uint32_t v2id;
};

struct pop_history_file_context {
	state::layer& map;
	game_definition::pops_history_file& file;
	int date;
};

struct issues_group {
	void finish(issue_group_context&) { }
};

struct issue {
	void finish(issue_context&) { }
	void next_step_only(association_type, bool value, error_handler& err, int32_t line, issue_context& context);
	void administrative(association_type, bool value, error_handler& err, int32_t line, issue_context& context);
};

struct issues_file {
	void finish(generic_context&) { }
};

struct inventions_file {
	void finish(technology_context&) { }
};

struct technology_sub_file {
	void finish(technology_context&) { }
};

struct core_gfx_file {
	void finish(gfx_file_context&) { }
};

struct sprites_group {
	void finish(gfx_file_context&) {}
};

struct goods_file {
	void finish(generic_context&) {}
};

struct goods_group {
	void finish(commodity_group_context&) {}
};

struct good {
	void finish(commodity_context&) {}
	void cost(association_type, float value, error_handler& err, int32_t line, commodity_context& context);
	void color(color_from_3i value, error_handler& err, int32_t line, commodity_context& context);
	void available_from_start(association_type, bool value, error_handler& err, int32_t line, commodity_context& context);
	void is_local(association_type, bool value, error_handler& err, int32_t line, commodity_context& context);
	void tradeable(association_type, bool value, error_handler& err, int32_t line, commodity_context& context);
	void overseas_penalty(association_type, bool value, error_handler& err, int32_t line, commodity_context& context);
	void money(association_type, bool value, error_handler& err, int32_t line, commodity_context& context);
	void uses_potentials(association_type, bool value, error_handler& err, int32_t line, commodity_context& context);
};

void make_good(std::string_view name, token_generator& gen, error_handler& err, commodity_group_context& context);
void make_goods_group(std::string_view name, token_generator& gen, error_handler& err, generic_context& context);

void make_culture_group(std::string_view name, token_generator& gen, error_handler& err, culture_file_context& context);
void make_culture(std::string_view name, token_generator& gen, error_handler& err, culture_group_context& context);

void make_religion_group(std::string_view name, token_generator& gen, error_handler& err, generic_context& context);
void make_religion(std::string_view name, token_generator& gen, error_handler& err, religion_group_context& context);

struct sprite {
	pair_of_x_and_y size {false};
	pair_of_x_and_y bordersize {false};
	void finish(gfx_sprite_context& context) {
		if (size.exists) {
			context.sprite.size_x = size.x;
			context.sprite.size_y = size.y;
			context.sprite.has_size = true;
		}
		if (bordersize.exists) {
			context.sprite.border_size_x = bordersize.x;
			context.sprite.border_size_y = bordersize.y;
			context.sprite.has_border_size = true;
		}
	}
	void name(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void effectfile(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void clicksound(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void loadtype(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void noofframes(association_type, int value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void norefcount(association_type, bool value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void texturefile(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void texturefile1(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void texturefile2(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void transparencecheck(association_type, bool value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void allwaystransparent(association_type, bool value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void color(color_from_3f value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void colortwo(color_from_3f value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void mask(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void font_size(association_type, int value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void scale(association_type, float value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void offset2(color_from_3f value, error_handler& err, int32_t line, gfx_sprite_context& context);
	void font(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context);
};

// struct projection {
// 	void finish(gfx_file_context& context);
// 	int size;
// 	float spin;
// 	bool pulsating;
// 	float pulselowest;
// 	int pulsespeed;
// 	float expanding;
// 	bool additative;
// };

void make_sprite(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_text_sprite(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_masked_shield(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_cornered_sprite(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_progressbartype(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_progressbar3dtype(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_flagtype(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_billboard(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_unitstatsbillboard(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_projectiontype(token_generator& gen, error_handler& err, gfx_file_context& context);

void make_meshtype(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_maptexttype(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_provincetype(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_animatedmaptext(token_generator& gen, error_handler& err, gfx_file_context& context);
void make_watertype(token_generator& gen, error_handler& err, gfx_file_context& context);

void handle_sprites_group(token_generator& gen, error_handler& err, gfx_file_context& context);
void save_light_types(token_generator& gen, error_handler& err, gfx_file_context& context);
void save_bitmap_fonts(token_generator& gen, error_handler& err, gfx_file_context& context);
void save_bitmap_font(token_generator& gen, error_handler& err, gfx_file_context& context);
void save_fonts(token_generator& gen, error_handler& err, gfx_file_context& context);

void make_issue(std::string_view name, token_generator& gen, error_handler& err, issue_group_context& context);
void make_issues_group(std::string_view name, token_generator& gen, error_handler& err, generic_context& context);

void register_issue_option(std::string_view name, token_generator& gen, error_handler& err, issue_context& context);
void register_invention(std::string_view name, token_generator& gen, error_handler& err, technology_context& context);
void register_technology(std::string_view name, token_generator& gen, error_handler& err, technology_context& context);

void create_government_type(std::string_view name, token_generator& gen, error_handler& err, generic_context& context);

struct governments_file {
	void finish(generic_context&) { }
};

struct government_type_context{
	state::layer& map;
	game_definition::government& current;
};

struct government_type {
	void duration(association_type, int value, error_handler& err, int32_t line, government_type_context& context);
	void election(association_type, bool value, error_handler& err, int32_t line, government_type_context& context);
	void flagtype(association_type, std::string_view value, error_handler& err, int32_t line, government_type_context& context);
	void appoint_ruling_party(association_type, bool value, error_handler& err, int32_t line, government_type_context& context);
	void any_value(std::string_view text, association_type, bool value, error_handler& err, int32_t line, government_type_context& context);
	void finish(government_type_context&) { }
};

struct pop_history_definition {
	int size = 0;
	float militancy = 0;
	std::string _culture {};
	std::string _religion {};
	std::string _rebel_type {};
	void finish(pop_history_context&) {};
	void culture(association_type, std::string_view value, error_handler& err, int32_t line, pop_history_context& context);
	void religion(association_type, std::string_view value, error_handler& err, int32_t line, pop_history_context& context);
	void rebel_type(association_type, std::string_view value, error_handler& err, int32_t line, pop_history_context& context);
};

struct pop_province_list {
	void finish(pop_history_context&);
	void any_group(std::string_view key, pop_history_definition value, error_handler& err, int32_t line, pop_history_context context);
};

struct pop_history_file {
	void finish(pop_history_file_context&){};
};

void make_pop_province_list(std::string_view name, token_generator& gen, error_handler& err, pop_history_file_context& context);

//forward declaration for the correspording cpp file

template<typename C>
government_type parse_government_type(token_generator& gen, error_handler& err, C&& context);

sprites_group parse_sprites_group(token_generator& gen, error_handler& err, gfx_file_context& context);

struct nation_history_file{
	game_definition::nation_history& nation;
	state::layers_stack& state;
	state::layer& map;
};

struct national_flag_handler {
	std::string _government;
	std::string _flag;
	void flag(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void government(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void finish(nation_history_file&) { }
};

struct upper_house_handler {
	void any_value(std::string_view label, association_type, float value, error_handler& err, int32_t line, nation_history_file& context);
	void finish(nation_history_file&) { }
};

struct foreign_investment_handler {
	void any_value(std::string_view label, association_type, float value, error_handler& err, int32_t line, nation_history_file& context);
	void finish(nation_history_file&) { }
};

void enter_country_file_dated_block(std::string_view label, token_generator& gen, error_handler& err, nation_history_file& context);
void make_eu4_region_definition(std::string_view name, token_generator& gen, error_handler& err, generic_context& context) ;

struct nation_handler {
	void finish(nation_history_file&);
	void govt_flag(national_flag_handler const& value, error_handler& err, int32_t line, nation_history_file& context);
	foreign_investment_handler foreign_investment;
	upper_house_handler upper_house;
	void set_country_flag(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void set_global_flag(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void colonial_points(association_type, int32_t value, error_handler& err, int32_t line, nation_history_file& context);
	void capital(association_type, int32_t value, error_handler& err, int32_t line, nation_history_file& context);
	void any_value(std::string_view label, association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void primary_culture(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void culture(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void remove_culture(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void religion(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void government(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void plurality(association_type, float value, error_handler& err, int32_t line, nation_history_file& context);
	void prestige(association_type, float value, error_handler& err, int32_t line, nation_history_file& context);
	void nationalvalue(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void schools(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void civilized(association_type, bool value, error_handler& err, int32_t line, nation_history_file& context);
	void is_releasable_vassal(association_type, bool value, error_handler& err, int32_t line, nation_history_file& context);
	void literacy(association_type, float value, error_handler& err, int32_t line, nation_history_file& context);
	void non_state_culture_literacy(association_type, float value, error_handler& err, int32_t line, nation_history_file& context);
	void consciousness(association_type, float value, error_handler& err, int32_t line, nation_history_file& context);
	void nonstate_consciousness(association_type, float value, error_handler& err, int32_t line, nation_history_file& context);
	void ruling_party(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
	void decision(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context);
};


struct meiou_population_distribution_desc {
	std::string limit_trigger;
	float pop_total;
	float lvl_1_points;
	float lvl_2_points;
	float lvl_3_points;
	float lvl_4_points;
	float lvl_5_points;
	float lvl_6_points;
	float lvl_7_points;
	void finish(generic_context& ctx);
};

struct meiou_population_distribution_list {
	void finish(generic_context& ctx) {};
};

struct meiou_population_distribution_file {
	void finish(generic_context& ctx) {};
};




};