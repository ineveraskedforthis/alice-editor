#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "parsers.hpp"
#include "../definitions.hpp"

namespace state {
	struct layer;
	struct layers_stack;
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

struct generic_context{
	state::layer& map;
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
			context.culture.first_names.push_back(s);
		}
	};
	void last_names(group_of_strings value, error_handler& err, int32_t line, culture_context& context) {
		for (auto& s: value.data) {
			context.culture.last_names.push_back(s);
		}
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
	void finish(generic_context&) { }
};

struct sprites_group {
	void finish(generic_context&) {}
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
			context.sprite.border_size_x = size.x;
			context.sprite.border_size_y = size.y;
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
};

void make_sprite(token_generator& gen, error_handler& err, generic_context& context);
void make_text_sprite(token_generator& gen, error_handler& err, generic_context& context);
void make_masked_shield(token_generator& gen, error_handler& err, generic_context& context);
void make_cornered_sprite(token_generator& gen, error_handler& err, generic_context& context);

void handle_sprites_group(token_generator& gen, error_handler& err, generic_context& context);
void save_light_types(token_generator& gen, error_handler& err, generic_context& context);
void save_object_types(token_generator& gen, error_handler& err, generic_context& context);
void save_bitmap_fonts(token_generator& gen, error_handler& err, generic_context& context);
void save_bitmap_font(token_generator& gen, error_handler& err, generic_context& context);
void save_fonts(token_generator& gen, error_handler& err, generic_context& context);

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

sprites_group parse_sprites_group(token_generator& gen, error_handler& err, generic_context&& context);

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
};