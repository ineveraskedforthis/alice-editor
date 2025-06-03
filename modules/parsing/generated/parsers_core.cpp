#include <string>
#include "parsers_core.hpp"
#include "../editor-state/content-state.hpp"
#include "parsers.hpp"

namespace parsers {
void register_issue_option(std::string_view name, token_generator& gen, error_handler& err, issue_context& context) {
    std::string actual_string {name};
    context.map.issues[context.issue_name].options.push_back(actual_string);
    gen.discard_group();
};
void register_invention(std::string_view name, token_generator& gen, error_handler& err, technology_context& context) {
    std::string actual_string {name};
    context.map.inventions[actual_string] = {actual_string, context.folder};
    gen.discard_group();
};
void register_technology(std::string_view name, token_generator& gen, error_handler& err, technology_context& context) {
    std::string actual_string {name};
    context.map.tech[actual_string] = {actual_string, context.folder};
    gen.discard_group();
};

void issue::next_step_only(association_type, bool value, error_handler& err, int32_t line, issue_context& context){

};
void issue::administrative(association_type, bool value, error_handler& err, int32_t line, issue_context& context){

};


void government_type::duration(association_type, int value, error_handler& err, int32_t line, government_type_context& context){
    context.current.duration = value;
};
void government_type::election(association_type, bool value, error_handler& err, int32_t line, government_type_context& context){
    context.current.election = value;
};
void government_type::flagtype(association_type, std::string_view value, error_handler& err, int32_t line, government_type_context& context){
    std::string actual_string {value};
    context.current.flagtype = actual_string;

    bool flag_type_found = false;
    for (auto& existing : context.map.detected_flags)
        if (existing == value)
            flag_type_found = true;

    if (!flag_type_found) {
        context.map.detected_flags.push_back(actual_string);
    }
};
void government_type::appoint_ruling_party(association_type, bool value, error_handler& err, int32_t line, government_type_context& context){
    context.current.appoint_ruling_party = value;
};
void government_type::any_value(std::string_view text, association_type, bool value, error_handler& err, int32_t line, government_type_context& context){
    std::string actual_string {text};
    context.current.allowed_parties[actual_string] = value;
};


void upper_house_handler::any_value(std::string_view label, association_type, float value, error_handler& err, int32_t line, nation_history_file& context){
    std::string actual_key = {label.begin(), label.end()};
    context.nation.upper_house[actual_key] = value;
};
void foreign_investment_handler::any_value(std::string_view label, association_type, float value, error_handler& err, int32_t line, nation_history_file& context){
    std::string actual_key = {label.begin(), label.end()};
    context.nation.foreign_investment[actual_key] = value;
};

void national_flag_handler::flag(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
    _government = value;
};
void national_flag_handler::government(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
    _flag = value;
};

void enter_country_file_dated_block(std::string_view label, token_generator& gen, error_handler& err, nation_history_file& context) {
    gen.discard_group();
};

void nation_handler::finish(nation_history_file&){

};
void nation_handler::govt_flag(national_flag_handler const& value, error_handler& err, int32_t line, nation_history_file& context){
    game_definition::scripted_flag flag {
        .government = value._government,
        .flag = value._flag
    };
    context.nation.govt_flag.push_back((flag));
};
void nation_handler::set_country_flag(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
    std::string actual_value = {value.begin(), value.end()};
    context.nation.set_country_flag.push_back(actual_value);
};
void nation_handler::set_global_flag(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
    std::string actual_value = {value.begin(), value.end()};
    context.nation.set_global_flag.push_back(actual_value);
};
void nation_handler::colonial_points(association_type, int32_t value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.colonial_points = value;
};
void nation_handler::capital(association_type, int32_t value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.capital = value;
};
void nation_handler::any_value(std::string_view label, association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
	std::string key(label);

    bool has_tech_key =
        context.state.has_tech_key(key)
        || (context.map.tech.find(key) != context.map.tech.end());

    bool has_invention_key =
        context.state.has_invention_key(key)
        || context.map.inventions.find(key) != context.map.inventions.end();

    bool has_issue_key =
        context.state.has_issues_key(key)
        || context.map.issues.find(key) != context.map.issues.end();

	if(has_tech_key) {
		auto v = parse_bool(value, line, err);
        context.nation.tech[key] = v;
	} else if(has_invention_key) {
		auto v = parse_bool(value, line, err);
		context.nation.inventions[key] = v;
    } else if(has_issue_key) {
		context.nation.issues[key] = value;
    } else {
		err.accumulated_errors +=
				"invalid key " + key + " encountered  (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
};
void nation_handler::primary_culture(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.primary_culture = value;
};
void nation_handler::culture(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
    std::string actual_value = {value.begin(), value.end()};
    context.nation.culture.push_back(actual_value);
};
void nation_handler::remove_culture(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){

};
void nation_handler::religion(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.religion = value;
};
void nation_handler::government(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.government = value;
};
void nation_handler::plurality(association_type, float value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.plurality = value;
};
void nation_handler::prestige(association_type, float value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.prestige = value;
};
void nation_handler::nationalvalue(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.nationalvalue = value;
};
void nation_handler::schools(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.schools = value;
};
void nation_handler::civilized(association_type, bool value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.civilized = value;
};
void nation_handler::is_releasable_vassal(association_type, bool value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.is_releasable_vassal = value;
};
void nation_handler::literacy(association_type, float value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.literacy = value;
};
void nation_handler::non_state_culture_literacy(association_type, float value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.non_state_culture_literacy = value;
};
void nation_handler::consciousness(association_type, float value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.consciousness = value;
};
void nation_handler::nonstate_consciousness(association_type, float value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.nonstate_consciousness = value;
};
void nation_handler::ruling_party(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
    context.nation.ruling_party = value;
};
void nation_handler::decision(association_type, std::string_view value, error_handler& err, int32_t line, nation_history_file& context){
    std::string actual_value = {value.begin(), value.end()};
    context.nation.decision.push_back(actual_value);
};

void sprite::name(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context){
    std::string actual_value = {value.begin(), value.end()};
    context.sprite.name = actual_value;
};
void sprite::effectfile(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context){
    std::string actual_value = {value.begin(), value.end()};
    context.sprite.effectfile = actual_value;
};
void sprite::clicksound(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context){
    std::string actual_value = {value.begin(), value.end()};
    context.sprite.clicksound = actual_value;
};
void sprite::loadtype(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context){
    std::string actual_value = {value.begin(), value.end()};
    context.sprite.loadtype = actual_value;
};
void sprite::noofframes(association_type, int value, error_handler& err, int32_t line, gfx_sprite_context& context){
    context.sprite.noofframes = value;
};
void sprite::norefcount(association_type, bool value, error_handler& err, int32_t line, gfx_sprite_context& context){
    context.sprite.norefcount = value;
};
void sprite::texturefile(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context){
    std::string actual_value = {value.begin(), value.end()};
    context.sprite.texturefile = actual_value;
};
void sprite::transparencecheck(association_type, bool value, error_handler& err, int32_t line, gfx_sprite_context& context){
    context.sprite.transparencecheck = value;
};
void sprite::allwaystransparent(association_type, bool value, error_handler& err, int32_t line, gfx_sprite_context& context){
    context.sprite.allwaystransparent = value;
};
}