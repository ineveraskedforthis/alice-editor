#include <cassert>
#include <cstdint>
#include <numbers>
#include <string>
#include <string_view>
#include <vector>
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
void sprite::color(color_from_3f value, error_handler& err, int32_t line, gfx_sprite_context& context) {
    context.sprite.has_color = true;
    context.sprite.color.r = value.colors[0];
    context.sprite.color.g = value.colors[1];
    context.sprite.color.b = value.colors[2];
}
void sprite::colortwo(color_from_3f value, error_handler& err, int32_t line, gfx_sprite_context& context) {
    context.sprite.has_color_two = true;
    context.sprite.color_two.r = value.colors[0];
    context.sprite.color_two.g = value.colors[1];
    context.sprite.color_two.b = value.colors[2];
}
void sprite::mask(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context){
    std::string actual_value = {value.begin(), value.end()};
    context.sprite.mask = actual_value;
}
void sprite::font_size(association_type, int value, error_handler& err, int32_t line, gfx_sprite_context& context){
    context.sprite.font_size = value;
}
void sprite::scale(association_type, float value, error_handler& err, int32_t line, gfx_sprite_context& context){
    context.sprite.scale = value;
}

void sprite::offset2(color_from_3f value, error_handler& err, int32_t line, gfx_sprite_context& context){
    context.sprite.offset2 = value.colors;
}
void sprite::font(association_type, std::string_view value, error_handler& err, int32_t line, gfx_sprite_context& context){
    std::string actual_value = {value.begin(), value.end()};
    context.sprite.font = actual_value;
}
void handle_sprites_group(token_generator& gen, error_handler& err, gfx_file_context& context){
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

void save_light_types(token_generator& gen, error_handler& err, gfx_file_context& context) {
    std::string data;
    stupid_parsing(gen, data);
    context.gfx_file.lightTypes_text.push_back(data);
}
void save_bitmap_fonts(token_generator& gen, error_handler& err, gfx_file_context& context){
    std::string data;
    stupid_parsing(gen, data);
    context.gfx_file.bitmapfonts_text.push_back(data);
}
void save_bitmap_font(token_generator& gen, error_handler& err, gfx_file_context& context){
    std::string data;
    stupid_parsing(gen, data);
    context.gfx_file.bitmapfont_text.push_back(data);
}
void save_fonts(token_generator& gen, error_handler& err, gfx_file_context& context){
    std::string data;
    stupid_parsing(gen, data);
    context.gfx_file.fonts_text.push_back(data);
}

void make_sprite(token_generator& gen, error_handler& err, gfx_file_context& context) {
    game_definition::sprite result {};
    gfx_sprite_context ctx {
        .map = context.map,
        .sprite = result,
    };
    parse_sprite(gen, err, ctx);
    context.gfx_file.sprites.push_back(result);
}
void make_text_sprite(token_generator& gen, error_handler& err, gfx_file_context& context) {
    game_definition::sprite result {};
    gfx_sprite_context ctx {
        .map = context.map,
        .sprite = result,
    };
    parse_sprite(gen, err, ctx);
    context.gfx_file.text_sprites.push_back(result);
}
void make_masked_shield(token_generator& gen, error_handler& err, gfx_file_context& context) {
    game_definition::sprite result {};
    gfx_sprite_context ctx {
        .map = context.map,
        .sprite = result,
    };
    parse_sprite(gen, err, ctx);
    context.gfx_file.masked_shields.push_back(result);
}
void make_progressbartype(token_generator& gen, error_handler& err, gfx_file_context& context) {
    game_definition::sprite result {};
    gfx_sprite_context ctx {
        .map = context.map,
        .sprite = result,
    };
    parse_sprite(gen, err, ctx);
    context.gfx_file.progress_bars.push_back(result);
}
void make_progressbar3dtype(token_generator& gen, error_handler& err, gfx_file_context& context) {
    std::string res;
    stupid_parsing(gen, res);
    context.gfx_file.progress_bars3d.push_back(res);
}
void make_flagtype(token_generator& gen, error_handler& err, gfx_file_context& context) {
    std::string res;
    stupid_parsing(gen, res);
    context.gfx_file.flagtypes.push_back(res);
}
void make_cornered_sprite(token_generator& gen, error_handler& err, gfx_file_context& context) {
    game_definition::sprite result {};
    gfx_sprite_context ctx {
        .map = context.map,
        .sprite = result,
    };
    parse_sprite(gen, err, ctx);
    context.gfx_file.cornered_sprites.push_back(result);
}
void make_billboard(token_generator& gen, error_handler& err, gfx_file_context& context){
    game_definition::sprite result {};
    gfx_sprite_context ctx {
        .map = context.map,
        .sprite = result,
    };
    parse_sprite(gen, err, ctx);
    context.gfx_file.billboards.push_back(result);
}
void make_unitstatsbillboard(token_generator& gen, error_handler& err, gfx_file_context& context){
    game_definition::sprite result {};
    gfx_sprite_context ctx {
        .map = context.map,
        .sprite = result,
    };
    parse_sprite(gen, err, ctx);
    context.gfx_file.unitstats_billboards.push_back(result);
}

void make_projectiontype(token_generator& gen, error_handler& err, gfx_file_context& context){
    std::string res;
    stupid_parsing(gen, res);
    context.gfx_file.projections.push_back(res);
}

// void projection::finish(gfx_file_context& context) {
//     const game_definition::projection val {
//         size,
//         spin,
//         pulsating,
//         pulselowest,
//         pulsespeed,
//         expanding,
//         additative
//     };
//     context.gfx_file.projections.push_back(val);
// }

void make_meshtype(token_generator& gen, error_handler& err, gfx_file_context& context){
    std::string res;
    stupid_parsing(gen, res);
    context.gfx_file.mechtypes.push_back(res);
}
void make_maptexttype(token_generator& gen, error_handler& err, gfx_file_context& context){
    std::string res;
    stupid_parsing(gen, res);
    context.gfx_file.map_text_types.push_back(res);
}
void make_provincetype(token_generator& gen, error_handler& err, gfx_file_context& context){
    std::string res;
    stupid_parsing(gen, res);
    context.gfx_file.province_types.push_back(res);
}
void make_animatedmaptext(token_generator& gen, error_handler& err, gfx_file_context& context){
    std::string res;
    stupid_parsing(gen, res);
    context.gfx_file.animated_map_texts.push_back(res);
}
void make_watertype(token_generator& gen, error_handler& err, gfx_file_context& context){
    std::string res;
    stupid_parsing(gen, res);
    context.gfx_file.watertypes.push_back(res);
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

void make_continent_definition(std::string_view name, token_generator& gen, error_handler& err, generic_context& context) {
    std::string name_value {name};
    std::cout << "detect continent: " << name_value << "\n";

    auto parsed_continent = parse_continent_definition(gen, err, context);

    context.map.continent_modifiers[name_value] = parsed_continent.modifiers;

    for (auto item : parsed_continent.provinces.provinces_v2ids) {
        context.map.v2id_to_continent[item] = name_value;
    }
}

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





void
province_history_handler::life_rating
(
    association_type,
    uint32_t value,
    error_handler& err,
    int32_t line,
	province_history_context& context
) {
    context.history.life_rating = value;
}

void
province_history_handler::colony
(
    association_type,
    uint32_t value,
    error_handler& err,
    int32_t line,
    province_history_context& context
) {
    context.history.colonial = value;
}

void
province_history_handler::base_tax
(
    association_type,
    uint32_t value,
    error_handler& err,
    int32_t line,
    province_history_context& context
) {
    context.history.base_tax = value;
}

void
province_history_handler::base_production
(
    association_type,
    uint32_t value,
    error_handler& err,
    int32_t line,
    province_history_context& context
) {
    context.history.base_production = value;
}

void
province_history_handler::trade_goods
(
    association_type,
    std::string_view text,
    error_handler& err,
    int32_t line,
	province_history_context& context
) {
    context.history.main_trade_good = text;
}

void province_history_handler::rgo_distribution(province_rgo_ext const& value, error_handler& err, int32_t line, province_history_context& context) {
	return;
}

void province_history_handler::rgo_distribution_add(province_rgo_ext_2 const& value, error_handler& err, int32_t line, province_history_context& context) {
	return;
}

// void province_history_handler::factory_limit(province_factory_limit const& value, error_handler& err, int32_t line, province_history_context& context) {
// 	return;
// }

void province_history_handler::revolt(province_revolt const& rev, error_handler& err, int32_t line, province_history_context& context) {
    context.history.revolt = rev.rebel;
}

void
province_history_handler::owner(
    association_type,
    std::string_view value,
    error_handler& err,
    int32_t line,
	province_history_context& context
) {
    auto v = std::string{value};
    context.history.owner_tag = v;
}

void
province_history_handler::controller(
    association_type,
    std::string_view value,
    error_handler& err,
    int32_t line,
	province_history_context& context
) {
    auto v = std::string{value};
    context.history.controller_tag = v;
}

void
province_history_handler::terrain(
    association_type, std::string_view text, error_handler& err, int32_t line,
	province_history_context& context
) {
    context.history.terrain = text;
}

void
province_history_handler::culture(
    association_type, std::string_view text, error_handler& err, int32_t line,
	province_history_context& context
) {
    context.history.culture = text;
}

void
province_history_handler::religion(
    association_type, std::string_view text, error_handler& err, int32_t line,
	province_history_context& context
) {
    context.history.religion = text;
}

void province_history_handler::add_core(association_type, std::string_view value, error_handler& err, int32_t line,
	province_history_context& context
) {
    auto v = std::string{value};
    context.history.cores.push_back(v);
}

void province_history_handler::remove_core(
    association_type,
    std::string_view value,
    error_handler& err,
    int32_t line,
	province_history_context& context
) {
    // todo
    return;
}

void province_history_handler::party_loyalty(pv_party_loyalty const& value, error_handler& err, int32_t line,
	province_history_context& context
) {
    // todo
    return;
}

void province_history_handler::state_building(
    pv_state_building const& value,
    error_handler& err,
    int32_t line,
	province_history_context& context
) {
    context.history.buildings.push_back({value.level, value.building, value.upgrade});
}

void province_history_handler::is_slave(
    association_type,
    bool value,
    error_handler& err,
    int32_t line,
	province_history_context& context
) {
    context.history.is_slave = value;
}

void province_history_handler::any_value(std::string_view name, association_type, uint32_t value, error_handler& err, int32_t line,
			province_history_context& context
) {
    // todo: register buildings somewhere
    if (name == "railroad") {
        context.history.railroad = value;
    }
    if (name == "fort") {
        context.history.fort = value;
    }
    if (name == "naval_base") {
        context.history.naval_base = value;
    }
}

void province_rgo_ext_desc::max_employment(association_type, uint32_t value, error_handler& err, int32_t line, province_history_context& context) {
	max_employment_value = float(value);
}

void province_rgo_ext_desc::trade_good(association_type, std::string_view text, error_handler& err, int32_t line, province_history_context& context) {
	commodity = text;
}

void province_rgo_ext_desc::finish(province_history_context& context) {

};

void province_rgo_ext::entry(province_rgo_ext_desc const& value, error_handler& err, int32_t line, province_history_context& context) {
	context.history.secondary_rgo_size[value.commodity] = value.max_employment_value;
}

void province_rgo_ext_2_desc::max_employment(association_type, uint32_t value, error_handler& err, int32_t line, province_history_context& context) {
	max_employment_value = float(value);
}

void province_rgo_ext_2_desc::trade_good(
    association_type,
    std::string_view text,
    error_handler& err,
    int32_t line,
    province_history_context& context
) {
    commodity = text;
}

void province_rgo_ext_2_desc::finish(province_history_context& context) {

};

void province_rgo_ext_2::entry(province_rgo_ext_2_desc const& value, error_handler& err, int32_t line, province_history_context& context) {
	context.history.secondary_rgo_size_add[value.commodity] = value.max_employment_value;
}

/*
void province_factory_limit_desc::max_level(association_type, uint32_t value, error_handler& err, int32_t line, province_history_context& context) {
	max_level_value = int8_t(value);
}

void province_factory_limit_desc::trade_good(association_type, std::string_view text, error_handler& err, int32_t line, province_history_context& context) {
	if(auto it = context.outer_context.map_of_commodity_names.find(std::string(text));
			it != context.outer_context.map_of_commodity_names.end()) {
		trade_good_id = it->second;
	} else {
		err.accumulated_errors +=
			std::string(text) + " is not a valid commodity name (" + err.file_name + " line " + std::to_string(line) + ")\n";
	}
}

void province_factory_limit_desc::finish(province_history_context& context) {

};

void province_factory_limit::entry(province_factory_limit_desc const& value, error_handler& err, int32_t line, province_history_context& context) {
	if(value.trade_good_id) {
		auto p = context.id;
		context.outer_context.state.world.province_set_factory_max_size(p, value.trade_good_id, value.max_level_value * 10'000.f);
		context.outer_context.state.world.province_set_factory_limit_was_set_during_scenario_creation(p, true);
	}
}
*/

void province_revolt::type(parsers::association_type ,std::string_view text, error_handler& err, int32_t line, province_history_context& context) {
	rebel = text;
}

void enter_dated_block(std::string_view name, token_generator& gen, error_handler& err, province_history_context& context) {
    parse_dated_block(gen, err, context);
}
void enter_setter_meiou(token_generator& gen, error_handler& err, province_history_context& context) {
    parse_setter_meiou(gen, err, context);
}

void make_region_trigger_or(token_generator& gen, error_handler& err, region_trigger_context& context) {
    auto trigger = bundled_trigger { {}, trigger_environment::pdx_or, false };
    region_trigger_context new_context {
        context.map, trigger
    };
    parse_region_trigger(gen, err, new_context);
    context.inputs.push_back(trigger);
}
void make_region_trigger_and(token_generator& gen, error_handler& err, region_trigger_context& context) {
    auto trigger = bundled_trigger { {}, trigger_environment::pdx_and, false };
    region_trigger_context new_context {
        context.map, trigger, {}
    };
    parse_region_trigger(gen, err, new_context);
    context.inputs.push_back(trigger);
}
void make_region_trigger_not_and(token_generator& gen, error_handler& err, region_trigger_context& context) {
    auto trigger = bundled_trigger { {}, trigger_environment::pdx_and, true };
    region_trigger_context new_context {
        context.map, trigger, {}
    };
    parse_region_trigger(gen, err, new_context);
    context.inputs.push_back(trigger);
}
void make_region_trigger(std::string_view name, token_generator& gen, error_handler& err, generic_context& context) {
    auto actual_name = std::string {name};
    bundled_trigger data {{}, trigger_environment::pdx_and, false};
    region_trigger_context new_context {
        context.map, data, {}
    };
    auto trigger = parse_region_trigger(gen, err, new_context);
    context.map.loading_only_scripted_region_collection[actual_name] = data.included_provinces;
}
void region_trigger::province_id(association_type, uint32_t v2id, error_handler& err, int32_t line, region_trigger_context& context) {
    context.current.included_provinces[v2id] = true;
}
void region_trigger::area(association_type, std::string_view text, error_handler& err, int32_t line, region_trigger_context& context) {
    auto key =  std::string{text};
    for (auto& item: context.map.loading_only_area_to_collection[key]) {
        context.current.included_provinces[item] = true;
    }
}
void region_trigger::region(association_type, std::string_view text, error_handler& err, int32_t line, region_trigger_context& context) {
    auto key =  std::string{text};
    for (auto& eu4_area: context.map.loading_only_region_to_collection[key]) {
        for (auto& eu4_province : context.map.loading_only_area_to_collection[eu4_area]) {
            context.current.included_provinces[eu4_province] = true;
        }
    }
}
void region_trigger::finish(region_trigger_context& ctx) {
    // push negation:
    // we track negation to avoid expensive set negation
    // assume that non-negated sets are "small"
    // and negated are "big"

    bool negation_required = false;
    if (ctx.current.env == trigger_environment::pdx_or) {
        // we do not want to calculate unions of negated and non-negated sets
        // so if we see a union and a negation inside, we push it to the top
        // now we have an intersection of a "small" set and some negated sets which is just set difference
        for (auto& item: ctx.inputs) {
            if (item.negation) negation_required = true; // some set is big, ALARM
        }
    } else {
        // we don't want to intersect only "big" sets
        // we want to have at least one small set as a "base" to remove elements from if they are not included in other inputs
        // if all inputs are "big", then we can push negation to the top and obtain union of "small" sets
        for (auto& item: ctx.inputs) {
            if (!item.negation) negation_required = false; // some set is small, that's GOOD
        }
    }
    if (negation_required) {
        if (ctx.current.env == trigger_environment::pdx_or)
            ctx.current.env = trigger_environment::pdx_and;
        else
            ctx.current.env = trigger_environment::pdx_or;
        ctx.current.negation = !ctx.current.negation;
        for (auto& item: ctx.inputs) {
            item.negation = !item.negation;
        }
    }

    if (ctx.current.env == trigger_environment::pdx_or) {
        // now we are safe: we know that there are no "big" sets inside
        // lets build a union, simple and clear
        ctx.current.included_provinces = {};
        for (auto& item: ctx.inputs) {
            assert(!item.negation);
            for (auto& [prov, _] : item.included_provinces) {
                ctx.current.included_provinces[prov] = true;
            }
        }
    } else {
        // here we know that one of sets is small, lets find it:
        for (auto& item: ctx.inputs) {
            if (!item.negation) {
                for (auto& [prov, _] : item.included_provinces) {
                    ctx.current.included_provinces[prov] = true;
                }
                break;
            }
        }

        // now we can intersect it with everything else:
        std::vector<uint32_t> erase;
        for (auto& [key, value] : ctx.current.included_provinces) {
            bool remains = true;
            for (auto& item : ctx.inputs) {
                auto exists = item.included_provinces.contains(key);
                if (exists && item.negation) {
                    remains = false;
                    break;
                }
                if (!exists && !item.negation) {
                    remains = false;
                    break;
                }
            }
            if (!remains) {
                erase.push_back(key);
            }
        }
        for (auto& key : erase) {
            ctx.current.included_provinces.erase(key);
        }
    }
}

void eu4_region_content::finish(region_context& context) {
    context.map.loading_only_region_to_collection[context.current_region] = areas.values;
}

void make_eu4_region_definition(std::string_view name, token_generator& gen, error_handler& err, generic_context& context) {
    std::string name_value {name};
    std::cout << "detect eu4 region: " << name_value << "\n";

    context.map.loading_only_region_to_collection[name_value] = {};

    region_context new_context {
        context.map, name_value
    };

    auto parsed_continent = parse_eu4_region_content(gen, err, new_context);
}


void meiou_population_distribution_desc::finish(generic_context& ctx) {
    auto base_pop_weight = [&](float pop_level) {
        if (pop_level == 0.5f) return lvl_1_points * 0.75f;
        if (pop_level == 1.f) return lvl_1_points;
        if (pop_level == 1.5f) return lvl_2_points * 0.75f;
        if (pop_level == 2.f) return lvl_2_points;
        if (pop_level == 2.5f) return lvl_3_points * 0.75f;
        if (pop_level == 3.f) return lvl_3_points;
        if (pop_level == 3.5f) return lvl_4_points * 0.75f;
        if (pop_level == 4.f) return lvl_4_points;
        if (pop_level == 4.5f) return lvl_5_points * 0.75f;
        if (pop_level == 5.f) return lvl_5_points;
        if (pop_level == 5.5f) return lvl_6_points * 0.75f;
        if (pop_level == 6.f) return lvl_6_points;
        if (pop_level == 6.5f) return lvl_7_points * 0.75f;
        if (pop_level == 7.f) return lvl_7_points;
        if (pop_level == 8.f) return lvl_7_points * 1.5f;
        if (pop_level == 9.f) return lvl_7_points * 2.f;
        if (pop_level == 10.f) return lvl_7_points * 2.5f;
        if (pop_level == 11.f) return lvl_7_points * 3.f;
        if (pop_level == 12.f) return lvl_7_points * 3.5f;
        if (pop_level == 13.f) return lvl_7_points * 4.f;
        if (pop_level == 14.f) return lvl_7_points * 5.f;
        if (pop_level == 15.f) return lvl_7_points * 6.f;
        return 0.f;
    };

    ankerl::unordered_dense::map<uint32_t, float> province_sizes {};
    for (auto& [v2id, vector_index] : ctx.map.v2id_to_vector_position) {
        province_sizes[v2id] = 0.f;
    }

    float y_shrink_factor = 1.3;
    if (ctx.map.provinces_image->full_globe){
        y_shrink_factor = 1.f;
    }
    for (auto x = 0; x < ctx.map.provinces_image->size_x; x++) {
        for (auto y = 0; y < ctx.map.provinces_image->size_y; y++) {
            auto prov = ctx.map.provinces_image;
            auto pixel = ctx.map.provinces_image->coord_to_pixel(glm::ivec2{x, y});
            auto r = ctx.map.provinces_image->provinces_image_data[pixel * 4];
            auto g = ctx.map.provinces_image->provinces_image_data[pixel * 4 + 1];
            auto b = ctx.map.provinces_image->provinces_image_data[pixel * 4 + 2];
            auto rgb = datatypes::rgb_to_uint(r, g, b);
            auto ite = ctx.map.rgb_to_v2id.find(rgb);
            if (ite == ctx.map.rgb_to_v2id.end()) continue;
            float t = (((float)y + 0.5f) / (float) ctx.map.provinces_image->size_y - 0.5f) * std::numbers::pi_v<float>;
            auto area_form = cos(t / y_shrink_factor);
            auto pixel_size =
                area_form
                * (1.f / (float)ctx.map.provinces_image->size_y * std::numbers::pi_v<float>)
                * (1.f / (float)ctx.map.provinces_image->size_x * 2 * std::numbers::pi_v<float>);
            province_sizes[ite->second] += pixel_size;
        }
    }

    auto pop_weight = [&](uint32_t v2id) {
        auto& def = ctx.map.province_history[v2id];
        auto base  = base_pop_weight(def.pop_level);
        auto adjusted= base * province_sizes[v2id];
        return adjusted;
    };

    float total_weight = 0.f;
    for (auto& [item, _] : ctx.map.loading_only_scripted_region_collection[limit_trigger]) {
        total_weight += pop_weight(item);
    }

    if (total_weight > 0.f) {
        for (auto& [item, _] : ctx.map.loading_only_scripted_region_collection[limit_trigger]) {
            ctx.map.province_history[item].rural_population += pop_total * pop_weight(item) / total_weight;
        }
    }
}
}