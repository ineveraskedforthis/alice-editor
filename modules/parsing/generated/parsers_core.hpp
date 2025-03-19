#pragma once
#include <string>
#include "parsers.hpp"
#include "../definitions.hpp"

namespace state {
	struct layer;
};

namespace parsers {
struct generic_context{
	state::layer& map;
};

struct context_with_file_label{
	state::layer& map;
	std::string file;
};

struct inventions_file {
	void finish(generic_context&) { }
};

struct technology_sub_file {
	void finish(generic_context&) { }
};

void register_invention(std::string_view name, token_generator& gen, error_handler& err, context_with_file_label&& context);
void register_technology(std::string_view name, token_generator& gen, error_handler& err, context_with_file_label&& context);

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

//forward declaration for according cpp file

template<typename C>
government_type parse_government_type(token_generator& gen, error_handler& err, C&& context);

struct nation_history_file{
    game_definition::nation_history& nation;
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