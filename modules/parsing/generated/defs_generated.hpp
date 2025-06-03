#include "parsers.hpp"
#include "parsers_core.hpp"

namespace parsers {
template<typename C>
issue parse_issue(token_generator& gen, error_handler& err, C&& context) {
	issue cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					register_issue_option(cur.content, gen, err, context);
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				case 14:
					switch(0x20 | int32_t(cur.content[0])) {
					case 0x61:
						// administrative
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x727473696E696D64) && (*(uint32_t const*)(&cur.content[9]) | uint32_t(0x20202020) ) == uint32_t(0x76697461) && (cur.content[13] | 0x20 ) == 0x65)) {
							cobj.administrative(assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							err.unhandled_association_key(cur);
						}
						break;
					case 0x6E:
						// next_step_only
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x706574737F747865) && (*(uint32_t const*)(&cur.content[9]) | uint32_t(0x20202020) ) == uint32_t(0x6C6E6F7F) && (cur.content[13] | 0x20 ) == 0x79)) {
							cobj.next_step_only(assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							err.unhandled_association_key(cur);
						}
						break;
					default:
						err.unhandled_association_key(cur);
						break;
					}
					break;
				default:
					err.unhandled_association_key(cur);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
sprite parse_sprite(token_generator& gen, error_handler& err, C&& context) {
	sprite cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					err.unhandled_group_key(cur); gen.discard_group();
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				case 4:
					// name
					if((true && (*(uint32_t const*)(&cur.content[0]) | uint32_t(0x20202020) ) == uint32_t(0x656D616E))) {
						cobj.name(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						err.unhandled_association_key(cur);
					}
					break;
				case 8:
					// loadtype
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x6570797464616F6C))) {
						cobj.loadtype(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						err.unhandled_association_key(cur);
					}
					break;
				case 10:
					switch(0x20 | int32_t(cur.content[0])) {
					case 0x63:
						// clicksound
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x6E756F736B63696C) && (cur.content[9] | 0x20 ) == 0x64)) {
							cobj.clicksound(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							err.unhandled_association_key(cur);
						}
						break;
					case 0x65:
						// effectfile
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x6C69667463656666) && (cur.content[9] | 0x20 ) == 0x65)) {
							cobj.effectfile(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							err.unhandled_association_key(cur);
						}
						break;
					case 0x6E:
						// NO
						// running -  O
						if((true && (cur.content[1] | 0x20 ) == 0x6F)) {
							switch(0x20 | int32_t(cur.content[2])) {
							case 0x6F:
								// noofframes
								if((true && (*(uint32_t const*)(&cur.content[3]) | uint32_t(0x20202020) ) == uint32_t(0x61726666) && (*(uint16_t const*)(&cur.content[7]) | 0x2020 ) == 0x656D && (cur.content[9] | 0x20 ) == 0x73)) {
									cobj.noofframes(assoc_type, parse_int(rh_token.content, rh_token.line, err), err, cur.line, context);
								} else {
									err.unhandled_association_key(cur);
								}
								break;
							case 0x72:
								// norefcount
								if((true && (*(uint32_t const*)(&cur.content[3]) | uint32_t(0x20202020) ) == uint32_t(0x6F636665) && (*(uint16_t const*)(&cur.content[7]) | 0x2020 ) == 0x6E75 && (cur.content[9] | 0x20 ) == 0x74)) {
									cobj.norefcount(assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
								} else {
									err.unhandled_association_key(cur);
								}
								break;
							default:
								err.unhandled_association_key(cur);
								break;
							}
						} else {
							err.unhandled_association_key(cur);
							}
						break;
					default:
						err.unhandled_association_key(cur);
						break;
					}
					break;
				case 11:
					// texturefile
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x6665727574786574) && (*(uint16_t const*)(&cur.content[8]) | 0x2020 ) == 0x6C69 && (cur.content[10] | 0x20 ) == 0x65)) {
						cobj.texturefile(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						err.unhandled_association_key(cur);
					}
					break;
				case 17:
					// transparencecheck
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x726170736E617274) && (*(uint64_t const*)(&cur.content[8]) | uint64_t(0x2020202020202020) ) == uint64_t(0x6365686365636E65) && (cur.content[16] | 0x20 ) == 0x6B)) {
						cobj.transparencecheck(assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						err.unhandled_association_key(cur);
					}
					break;
				case 18:
					// allwaystransparent
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x74737961776C6C61) && (*(uint64_t const*)(&cur.content[8]) | uint64_t(0x2020202020202020) ) == uint64_t(0x65726170736E6172) && (*(uint16_t const*)(&cur.content[16]) | 0x2020 ) == 0x746E)) {
						cobj.allwaystransparent(assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						err.unhandled_association_key(cur);
					}
					break;
				default:
					err.unhandled_association_key(cur);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
sprites_group parse_sprites_group(token_generator& gen, error_handler& err, C&& context) {
	sprites_group cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				case 10:
					// spritetype
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x7974657469727073) && (*(uint16_t const*)(&cur.content[8]) | 0x2020 ) == 0x6570)) {
						make_sprite(gen, err, context);
					} else {
						err.unhandled_group_key(cur); gen.discard_group();
					}
					break;
				case 14:
					// textspritetype
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x6972707374786574) && (*(uint32_t const*)(&cur.content[8]) | uint32_t(0x20202020) ) == uint32_t(0x79746574) && (*(uint16_t const*)(&cur.content[12]) | 0x2020 ) == 0x6570)) {
						make_text_sprite(gen, err, context);
					} else {
						err.unhandled_group_key(cur); gen.discard_group();
					}
					break;
				case 16:
					// maskedshieldtype
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x687364656B73616D) && (*(uint64_t const*)(&cur.content[8]) | uint64_t(0x2020202020202020) ) == uint64_t(0x65707974646C6569))) {
						make_masked_shield(gen, err, context);
					} else {
						err.unhandled_group_key(cur); gen.discard_group();
					}
					break;
				case 22:
					// corneredtilespritetype
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x646572656E726F63) && (*(uint64_t const*)(&cur.content[8]) | uint64_t(0x2020202020202020) ) == uint64_t(0x69727073656C6974) && (*(uint32_t const*)(&cur.content[16]) | uint32_t(0x20202020) ) == uint32_t(0x79746574) && (*(uint16_t const*)(&cur.content[20]) | 0x2020 ) == 0x6570)) {
						make_cornered_sprite(gen, err, context);
					} else {
						err.unhandled_group_key(cur); gen.discard_group();
					}
					break;
				default:
					err.unhandled_group_key(cur); gen.discard_group();
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					err.unhandled_association_key(cur);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
core_gfx_file parse_core_gfx_file(token_generator& gen, error_handler& err, C&& context) {
	core_gfx_file cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				case 5:
					// fonts
					if((true && (*(uint32_t const*)(&cur.content[0]) | uint32_t(0x20202020) ) == uint32_t(0x746E6F66) && (cur.content[4] | 0x20 ) == 0x73)) {
						save_fonts(gen, err, context);
					} else {
						err.unhandled_group_key(cur); gen.discard_group();
					}
					break;
				case 10:
					switch(0x20 | int32_t(cur.content[0])) {
					case 0x62:
						// bitmapfont
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x6E6F6670616D7469) && (cur.content[9] | 0x20 ) == 0x74)) {
							save_bitmap_font(gen, err, context);
						} else {
							err.unhandled_group_key(cur); gen.discard_group();
						}
						break;
					case 0x6C:
						// lighttypes
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x6570797474686769) && (cur.content[9] | 0x20 ) == 0x73)) {
							save_light_types(gen, err, context);
						} else {
							err.unhandled_group_key(cur); gen.discard_group();
						}
						break;
					default:
						err.unhandled_group_key(cur); gen.discard_group();
						break;
					}
					break;
				case 11:
					switch(0x20 | int32_t(cur.content[0])) {
					case 0x62:
						// bitmapfonts
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x6E6F6670616D7469) && (*(uint16_t const*)(&cur.content[9]) | 0x2020 ) == 0x7374)) {
							save_bitmap_fonts(gen, err, context);
						} else {
							err.unhandled_group_key(cur); gen.discard_group();
						}
						break;
					case 0x6F:
						// objecttypes
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x7079747463656A62) && (*(uint16_t const*)(&cur.content[9]) | 0x2020 ) == 0x7365)) {
							save_object_types(gen, err, context);
						} else {
							err.unhandled_group_key(cur); gen.discard_group();
						}
						break;
					case 0x73:
						// spritetypes
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x7079746574697270) && (*(uint16_t const*)(&cur.content[9]) | 0x2020 ) == 0x7365)) {
							handle_sprites(gen, err, context);
						} else {
							err.unhandled_group_key(cur); gen.discard_group();
						}
						break;
					default:
						err.unhandled_group_key(cur); gen.discard_group();
						break;
					}
					break;
				default:
					err.unhandled_group_key(cur); gen.discard_group();
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					err.unhandled_association_key(cur);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
issues_group parse_issues_group(token_generator& gen, error_handler& err, C&& context) {
	issues_group cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					make_issue(cur.content, gen, err, context);
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					err.unhandled_association_key(cur);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
issues_file parse_issues_file(token_generator& gen, error_handler& err, C&& context) {
	issues_file cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					make_issues_group(cur.content, gen, err, context);
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					err.unhandled_association_key(cur);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
technology_sub_file parse_technology_sub_file(token_generator& gen, error_handler& err, C&& context) {
	technology_sub_file cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					register_technology(cur.content, gen, err, context);
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					err.unhandled_association_key(cur);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
inventions_file parse_inventions_file(token_generator& gen, error_handler& err, C&& context) {
	inventions_file cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					register_invention(cur.content, gen, err, context);
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					err.unhandled_association_key(cur);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
governments_file parse_governments_file(token_generator& gen, error_handler& err, C&& context) {
	governments_file cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					create_government_type(cur.content, gen, err, context);
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					err.unhandled_association_key(cur);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
government_type parse_government_type(token_generator& gen, error_handler& err, C&& context) {
	government_type cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					err.unhandled_group_key(cur); gen.discard_group();
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				case 8:
					switch(0x20 | int32_t(cur.content[0])) {
					case 0x64:
						// duration
						if((true && (*(uint32_t const*)(&cur.content[1]) | uint32_t(0x20202020) ) == uint32_t(0x74617275) && (*(uint16_t const*)(&cur.content[5]) | 0x2020 ) == 0x6F69 && (cur.content[7] | 0x20 ) == 0x6E)) {
							cobj.duration(assoc_type, parse_int(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					case 0x65:
						// election
						if((true && (*(uint32_t const*)(&cur.content[1]) | uint32_t(0x20202020) ) == uint32_t(0x7463656C) && (*(uint16_t const*)(&cur.content[5]) | 0x2020 ) == 0x6F69 && (cur.content[7] | 0x20 ) == 0x6E)) {
							cobj.election(assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					case 0x66:
						// flagtype
						if((true && (*(uint32_t const*)(&cur.content[1]) | uint32_t(0x20202020) ) == uint32_t(0x7467616C) && (*(uint16_t const*)(&cur.content[5]) | 0x2020 ) == 0x7079 && (cur.content[7] | 0x20 ) == 0x65)) {
							cobj.flagtype(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					default:
						cobj.any_value(cur.content, assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
						break;
					}
					break;
				case 20:
					// appoint_ruling_party
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x7F746E696F707061) && (*(uint64_t const*)(&cur.content[8]) | uint64_t(0x2020202020202020) ) == uint64_t(0x707F676E696C7572) && (*(uint32_t const*)(&cur.content[16]) | uint32_t(0x20202020) ) == uint32_t(0x79747261))) {
						cobj.appoint_ruling_party(assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						cobj.any_value(cur.content, assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
					}
					break;
				default:
					cobj.any_value(cur.content, assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
national_flag_handler parse_national_flag_handler(token_generator& gen, error_handler& err, C&& context) {
	national_flag_handler cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					err.unhandled_group_key(cur); gen.discard_group();
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				case 4:
					// flag
					if((true && (*(uint32_t const*)(&cur.content[0]) | uint32_t(0x20202020) ) == uint32_t(0x67616C66))) {
						cobj.flag(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						err.unhandled_association_key(cur);
					}
					break;
				case 10:
					// government
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x656D6E7265766F67) && (*(uint16_t const*)(&cur.content[8]) | 0x2020 ) == 0x746E)) {
						cobj.government(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						err.unhandled_association_key(cur);
					}
					break;
				default:
					err.unhandled_association_key(cur);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
upper_house_handler parse_upper_house_handler(token_generator& gen, error_handler& err, C&& context) {
	upper_house_handler cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					err.unhandled_group_key(cur); gen.discard_group();
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					cobj.any_value(cur.content, assoc_type, parse_float(rh_token.content, rh_token.line, err), err, cur.line, context);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
foreign_investment_handler parse_foreign_investment_handler(token_generator& gen, error_handler& err, C&& context) {
	foreign_investment_handler cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					err.unhandled_group_key(cur); gen.discard_group();
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				default:
					cobj.any_value(cur.content, assoc_type, parse_float(rh_token.content, rh_token.line, err), err, cur.line, context);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
template<typename C>
nation_handler parse_nation_handler(token_generator& gen, error_handler& err, C&& context) {
	nation_handler cobj;
	for(token_and_type cur = gen.get(); cur.type != token_type::unknown && cur.type != token_type::close_brace; cur = gen.get()) {
		if(cur.type == token_type::open_brace) { 
			err.unhandled_free_group(cur); gen.discard_group();
			continue;
		}
		auto peek_result = gen.next();
		if(peek_result.type == token_type::special_identifier) {
			auto peek2_result = gen.next_next();
			if(peek2_result.type == token_type::open_brace) {
				gen.get(); gen.get();
				switch(int32_t(cur.content.length())) {
				case 9:
					// govt_flag
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x616C667F74766F67) && (cur.content[8] | 0x20 ) == 0x67)) {
						cobj.govt_flag(parse_national_flag_handler(gen, err, context), err, cur.line, context);
					} else {
						enter_country_file_dated_block(cur.content, gen, err, context);
					}
					break;
				case 11:
					// upper_house
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x6F687F7265707075) && (*(uint16_t const*)(&cur.content[8]) | 0x2020 ) == 0x7375 && (cur.content[10] | 0x20 ) == 0x65)) {
						cobj.upper_house = parse_upper_house_handler(gen, err, context);
					} else {
						enter_country_file_dated_block(cur.content, gen, err, context);
					}
					break;
				case 18:
					// foreign_investment
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x7F6E676965726F66) && (*(uint64_t const*)(&cur.content[8]) | uint64_t(0x2020202020202020) ) == uint64_t(0x656D747365766E69) && (*(uint16_t const*)(&cur.content[16]) | 0x2020 ) == 0x746E)) {
						cobj.foreign_investment = parse_foreign_investment_handler(gen, err, context);
					} else {
						enter_country_file_dated_block(cur.content, gen, err, context);
					}
					break;
				default:
					enter_country_file_dated_block(cur.content, gen, err, context);
					break;
				}
			} else {
				auto const assoc_token = gen.get();
				auto const assoc_type = parse_association_type(assoc_token.content, assoc_token.line, err);
				auto const rh_token = gen.get();
				switch(int32_t(cur.content.length())) {
				case 3:
					// oob
					if((true && (*(uint16_t const*)(&cur.content[0]) | 0x2020 ) == 0x6F6F && (cur.content[2] | 0x20 ) == 0x62)) {
						
					} else {
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					}
					break;
				case 7:
					switch(0x20 | int32_t(cur.content[0])) {
					case 0x63:
						switch(0x20 | int32_t(cur.content[1])) {
						case 0x61:
							// capital
							if((true && (*(uint32_t const*)(&cur.content[2]) | uint32_t(0x20202020) ) == uint32_t(0x61746970) && (cur.content[6] | 0x20 ) == 0x6C)) {
								cobj.capital(assoc_type, parse_int(rh_token.content, rh_token.line, err), err, cur.line, context);
							} else {
								cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
							}
							break;
						case 0x75:
							// culture
							if((true && (*(uint32_t const*)(&cur.content[2]) | uint32_t(0x20202020) ) == uint32_t(0x7275746C) && (cur.content[6] | 0x20 ) == 0x65)) {
								cobj.culture(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
							} else {
								cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
							}
							break;
						default:
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
							break;
						}
						break;
					case 0x73:
						// schools
						if((true && (*(uint32_t const*)(&cur.content[1]) | uint32_t(0x20202020) ) == uint32_t(0x6F6F6863) && (*(uint16_t const*)(&cur.content[5]) | 0x2020 ) == 0x736C)) {
							cobj.schools(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					default:
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						break;
					}
					break;
				case 8:
					switch(0x20 | int32_t(cur.content[0])) {
					case 0x64:
						// decision
						if((true && (*(uint32_t const*)(&cur.content[1]) | uint32_t(0x20202020) ) == uint32_t(0x73696365) && (*(uint16_t const*)(&cur.content[5]) | 0x2020 ) == 0x6F69 && (cur.content[7] | 0x20 ) == 0x6E)) {
							cobj.decision(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					case 0x6C:
						// literacy
						if((true && (*(uint32_t const*)(&cur.content[1]) | uint32_t(0x20202020) ) == uint32_t(0x72657469) && (*(uint16_t const*)(&cur.content[5]) | 0x2020 ) == 0x6361 && (cur.content[7] | 0x20 ) == 0x79)) {
							cobj.literacy(assoc_type, parse_float(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					case 0x70:
						// prestige
						if((true && (*(uint32_t const*)(&cur.content[1]) | uint32_t(0x20202020) ) == uint32_t(0x74736572) && (*(uint16_t const*)(&cur.content[5]) | 0x2020 ) == 0x6769 && (cur.content[7] | 0x20 ) == 0x65)) {
							cobj.prestige(assoc_type, parse_float(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					case 0x72:
						// religion
						if((true && (*(uint32_t const*)(&cur.content[1]) | uint32_t(0x20202020) ) == uint32_t(0x67696C65) && (*(uint16_t const*)(&cur.content[5]) | 0x2020 ) == 0x6F69 && (cur.content[7] | 0x20 ) == 0x6E)) {
							cobj.religion(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					default:
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						break;
					}
					break;
				case 9:
					switch(0x20 | int32_t(cur.content[0])) {
					case 0x63:
						// civilized
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x64657A696C697669))) {
							cobj.civilized(assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					case 0x70:
						// plurality
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x7974696C6172756C))) {
							cobj.plurality(assoc_type, parse_float(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					default:
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						break;
					}
					break;
				case 10:
					// government
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x656D6E7265766F67) && (*(uint16_t const*)(&cur.content[8]) | 0x2020 ) == 0x746E)) {
						cobj.government(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					}
					break;
				case 12:
					// ruling_party
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x707F676E696C7572) && (*(uint32_t const*)(&cur.content[8]) | uint32_t(0x20202020) ) == uint32_t(0x79747261))) {
						cobj.ruling_party(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					}
					break;
				case 13:
					switch(0x20 | int32_t(cur.content[0])) {
					case 0x63:
						// consciousness
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x73756F6963736E6F) && (*(uint32_t const*)(&cur.content[9]) | uint32_t(0x20202020) ) == uint32_t(0x7373656E))) {
							cobj.consciousness(assoc_type, parse_float(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					case 0x6C:
						// last_election
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x63656C657F747361) && (*(uint32_t const*)(&cur.content[9]) | uint32_t(0x20202020) ) == uint32_t(0x6E6F6974))) {
							
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					case 0x6E:
						// nationalvalue
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x766C616E6F697461) && (*(uint32_t const*)(&cur.content[9]) | uint32_t(0x20202020) ) == uint32_t(0x65756C61))) {
							cobj.nationalvalue(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					default:
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						break;
					}
					break;
				case 14:
					// remove_culture
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x637F65766F6D6572) && (*(uint32_t const*)(&cur.content[8]) | uint32_t(0x20202020) ) == uint32_t(0x75746C75) && (*(uint16_t const*)(&cur.content[12]) | 0x2020 ) == 0x6572)) {
						cobj.remove_culture(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					}
					break;
				case 15:
					switch(0x20 | int32_t(cur.content[0])) {
					case 0x63:
						// colonial_points
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x7F6C61696E6F6C6F) && (*(uint32_t const*)(&cur.content[9]) | uint32_t(0x20202020) ) == uint32_t(0x6E696F70) && (*(uint16_t const*)(&cur.content[13]) | 0x2020 ) == 0x7374)) {
							cobj.colonial_points(assoc_type, parse_int(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					case 0x70:
						// primary_culture
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x637F7972616D6972) && (*(uint32_t const*)(&cur.content[9]) | uint32_t(0x20202020) ) == uint32_t(0x75746C75) && (*(uint16_t const*)(&cur.content[13]) | 0x2020 ) == 0x6572)) {
							cobj.primary_culture(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					case 0x73:
						// set_global_flag
						if((true && (*(uint64_t const*)(&cur.content[1]) | uint64_t(0x2020202020202020) ) == uint64_t(0x61626F6C677F7465) && (*(uint32_t const*)(&cur.content[9]) | uint32_t(0x20202020) ) == uint32_t(0x6C667F6C) && (*(uint16_t const*)(&cur.content[13]) | 0x2020 ) == 0x6761)) {
							cobj.set_global_flag(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						} else {
							cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						}
						break;
					default:
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
						break;
					}
					break;
				case 16:
					// set_country_flag
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x6E756F637F746573) && (*(uint64_t const*)(&cur.content[8]) | uint64_t(0x2020202020202020) ) == uint64_t(0x67616C667F797274))) {
						cobj.set_country_flag(assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					}
					break;
				case 20:
					// is_releasable_vassal
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x61656C65727F7369) && (*(uint64_t const*)(&cur.content[8]) | uint64_t(0x2020202020202020) ) == uint64_t(0x61767F656C626173) && (*(uint32_t const*)(&cur.content[16]) | uint32_t(0x20202020) ) == uint32_t(0x6C617373))) {
						cobj.is_releasable_vassal(assoc_type, parse_bool(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					}
					break;
				case 22:
					// nonstate_consciousness
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x65746174736E6F6E) && (*(uint64_t const*)(&cur.content[8]) | uint64_t(0x2020202020202020) ) == uint64_t(0x6F6963736E6F637F) && (*(uint32_t const*)(&cur.content[16]) | uint32_t(0x20202020) ) == uint32_t(0x656E7375) && (*(uint16_t const*)(&cur.content[20]) | 0x2020 ) == 0x7373)) {
						cobj.nonstate_consciousness(assoc_type, parse_float(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					}
					break;
				case 26:
					// non_state_culture_literacy
					if((true && (*(uint64_t const*)(&cur.content[0]) | uint64_t(0x2020202020202020) ) == uint64_t(0x746174737F6E6F6E) && (*(uint64_t const*)(&cur.content[8]) | uint64_t(0x2020202020202020) ) == uint64_t(0x7275746C75637F65) && (*(uint64_t const*)(&cur.content[16]) | uint64_t(0x2020202020202020) ) == uint64_t(0x61726574696C7F65) && (*(uint16_t const*)(&cur.content[24]) | 0x2020 ) == 0x7963)) {
						cobj.non_state_culture_literacy(assoc_type, parse_float(rh_token.content, rh_token.line, err), err, cur.line, context);
					} else {
						cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					}
					break;
				default:
					cobj.any_value(cur.content, assoc_type, parse_text(rh_token.content, rh_token.line, err), err, cur.line, context);
					break;
				}
			}
		} else {
			err.unhandled_free_value(cur);
		}
	}
	cobj.finish(context);
	return cobj;
}
}

