#include <string>
#include "parsers_core.hpp"
#include "generated_parser.hpp"
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
void sprite::texturefile1(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context){
    std::string actual_value = {value.begin(), value.end()};
    context.sprite.texturefile1 = actual_value;
};
void sprite::texturefile2(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context){
    std::string actual_value = {value.begin(), value.end()};
    context.sprite.texturefile2 = actual_value;
};
void sprite::transparencecheck(association_type, bool value, error_handler& err, int32_t line, gfx_sprite_context& context){
    context.sprite.transparencecheck = value;
};
void sprite::allwaystransparent(association_type, bool value, error_handler& err, int32_t line, gfx_sprite_context& context){
    context.sprite.allwaystransparent = value;
};


void handle_sprites_group(token_generator& gen, error_handler& err, generic_context& context){
    parse_sprites_group(gen, err, context);
}

void stupid_parsing(token_generator& gen, std::string& result) {
    auto balance = 1;
    result += " { ";
    for(token_and_type cur = gen.get(); cur.type != token_type::unknown && balance > 0; cur = gen.get()) {
        if (cur.type == token_type::quoted_string) {
            result += '"';
            result += cur.content;
            result += '"';
        } else {
            result += cur.content;
        }
        if (cur.type == token_type::open_brace) {
            balance++;
        }
        if (cur.type == token_type::close_brace) {
            balance--;
        }

        if (balance <= 0) {
            break;
        }

        result += " ";
    }
}

void save_light_types(token_generator& gen, error_handler& err, generic_context& context) {
    std::string data;
    stupid_parsing(gen, data);
    context.map.lightTypes_text.push_back(data);
}
void save_object_types(token_generator& gen, error_handler& err, generic_context& context){
    std::string data;
    stupid_parsing(gen, data);
    context.map.objectTypes_text.push_back(data);
}
void save_bitmap_fonts(token_generator& gen, error_handler& err, generic_context& context){
    std::string data;
    stupid_parsing(gen, data);
    context.map.bitmapfonts_text.push_back(data);
}
void save_bitmap_font(token_generator& gen, error_handler& err, generic_context& context){
    std::string data;
    stupid_parsing(gen, data);
    context.map.bitmapfont_text.push_back(data);
}
void save_fonts(token_generator& gen, error_handler& err, generic_context& context){
    std::string data;
    stupid_parsing(gen, data);
    context.map.fonts_text.push_back(data);
}

void make_sprite(token_generator& gen, error_handler& err, generic_context& context) {
    game_definition::sprite result {};
    gfx_sprite_context ctx {
        .map = context.map,
        .sprite = result,
    };
    parse_sprite(gen, err, ctx);
    context.map.sprites.push_back(result);
}
void make_text_sprite(token_generator& gen, error_handler& err, generic_context& context) {
    game_definition::sprite result {};
    gfx_sprite_context ctx {
        .map = context.map,
        .sprite = result,
    };
    parse_sprite(gen, err, ctx);
    context.map.text_sprites.push_back(result);
}
void make_masked_shield(token_generator& gen, error_handler& err, generic_context& context) {
    game_definition::sprite result {};
    gfx_sprite_context ctx {
        .map = context.map,
        .sprite = result,
    };
    parse_sprite(gen, err, ctx);
    context.map.masked_shields.push_back(result);
}
void make_cornered_sprite(token_generator& gen, error_handler& err, generic_context& context) {
    game_definition::sprite result {};
    gfx_sprite_context ctx {
        .map = context.map,
        .sprite = result,
    };
    parse_sprite(gen, err, ctx);
    context.map.cornered_sprites.push_back(result);
}

void make_pop_province_list(
    std::string_view name, token_generator& gen, error_handler& err, pop_history_file_context& context
) {
    std::string v2id_str {name};
    auto v2id = std::stoul(v2id_str);

    pop_history_context new_context {
        .map = context.map,
        .file = context.file,
        .date = context.date,
        .v2id = v2id,
    };

    parse_pop_province_list(gen, err, new_context);
}

void make_issue(std::string_view name, token_generator& gen, error_handler& err, issue_group_context& context) {
    std::string actual_string {name};
    issue_context new_context(context.map, actual_string);
    game_definition::issue issue{actual_string};
    context.map.issues[actual_string] = issue;
    std::cout << "detect issue: " << actual_string << "\n";
    parse_issue(gen, err, new_context);
};

void make_issues_group(std::string_view name, token_generator& gen, error_handler& err, generic_context& context) {
    std::string actual_string {name};
    issue_group_context new_context(context.map, actual_string);
    parse_issues_group(gen, err, new_context);
};

void create_government_type(std::string_view name, parsers::token_generator &gen, parsers::error_handler &err, parsers::generic_context &context) {
    context.map.governments.emplace_back();
    context.map.governments.back().name = name;
    parsers::government_type_context ctx {
        context.map, context.map.governments.back()
    };
    parse_government_type(gen, err, ctx);
}

void good::cost(association_type, float value, error_handler& err, int32_t line, commodity_context& context){
    context.target.cost = value;
}
void good::color(color_from_3i value, error_handler& err, int32_t line, commodity_context& context){
    context.target.r = value.colors[0];
    context.target.g = value.colors[1];
    context.target.b = value.colors[2];
}
void good::available_from_start(association_type, bool value, error_handler& err, int32_t line, commodity_context& context){
    context.target.available_from_start = value;
}
void good::is_local(association_type, bool value, error_handler& err, int32_t line, commodity_context& context){
    context.target.is_local = value;
}
void good::tradeable(association_type, bool value, error_handler& err, int32_t line, commodity_context& context){
    context.target.tradeable = value;
}
void good::overseas_penalty(association_type, bool value, error_handler& err, int32_t line, commodity_context& context){
    context.target.overseas_penalty = value;
}
void good::money(association_type, bool value, error_handler& err, int32_t line, commodity_context& context){
    context.target.money = value;
}
void good::uses_potentials(association_type, bool value, error_handler& err, int32_t line, commodity_context& context){
    context.target.uses_potentials = value;
}

void make_good(std::string_view name, token_generator& gen, error_handler& err, commodity_group_context& context) {
    std::string actual_string {name};
    game_definition::commodity new_good {
        .index = (int)context.map.goods.size(),
        .name = actual_string,
        .group = context.group
    };
    commodity_context new_context {
        context.map,
        new_good
    };
    parse_good(gen, err, new_context);
    context.map.goods[actual_string] = new_good;
}

void make_goods_group(std::string_view name, token_generator& gen, error_handler& err, generic_context& context) {
    std::string actual_string {name};
    commodity_group_context new_context {
        context.map, actual_string
    };
    parse_goods_group(gen, err, new_context);
}

void make_culture_group(std::string_view name, token_generator& gen, error_handler& err, culture_file_context& context) {
    std::string actual_string {name};
    game_definition::culture_group new_group {};
    new_group.name = name;

    context.map.culture_group_defs[actual_string] = new_group;
    context.map.culture_groups.push_back(actual_string);

    culture_group_context next_context {
        context.map, context.map.culture_group_defs[actual_string]
    };
    parse_culture_group(gen, err, next_context);
}

void make_culture(std::string_view name, token_generator& gen, error_handler& err, culture_group_context& context) {
    std::string actual_string {name};
    game_definition::culture new_culture {};
    new_culture.name = actual_string;

    context.map.culture_defs[actual_string] = new_culture;
    context.map.culture_to_group[actual_string] = context.culture_group.name;
    context.map.cultures.push_back(actual_string);
    context.culture_group.cultures.push_back(actual_string);

    culture_context next_context {
        context.map, context.map.culture_defs[actual_string]
    };

    std::cout << "culture " << actual_string << " was detected\n";
    parse_culture(gen, err, next_context);
}

void make_religion_group(std::string_view name, token_generator& gen, error_handler& err, generic_context& context) {
    std::string actual_string {name};
    religion_group_context next_context {
        context.map, actual_string
    };
    parse_religion_group(gen, err, next_context);
}

void make_religion(std::string_view name, token_generator& gen, error_handler& err, religion_group_context& context) {
    std::string actual_string {name};
    context.map.religions.push_back(actual_string);
    std::cout << "religion " << actual_string << " was detected\n";
    gen.discard_group();
}

void pop_province_list::finish(pop_history_context&){

}

void pop_province_list::any_group(
    std::string_view key,
    pop_history_definition value,
    error_handler& err,
    int32_t line,
    pop_history_context context
) {
    std::string poptype {key};
    game_definition::pop_history result {
        .poptype = poptype,
        .culture = value._culture,
        .religion = value._religion,
        .rebel_type = value._rebel_type,
        .size = value.size,
        .militancy = value.militancy
    };

    context.file.data[context.v2id].push_back(result);
}

void pop_history_definition::culture(
    association_type, std::string_view value, error_handler& err, int32_t line, pop_history_context& context
) {
    std::string value_reserved {value};
    _culture = value_reserved;
}
void pop_history_definition::religion(
    association_type, std::string_view value, error_handler& err, int32_t line, pop_history_context& context
) {
    std::string value_reserved {value};
    _religion = value_reserved;
}
void pop_history_definition::rebel_type(
    association_type, std::string_view value, error_handler& err, int32_t line, pop_history_context& context
) {
    std::string value_reserved {value};
    _rebel_type = value_reserved;
}

}