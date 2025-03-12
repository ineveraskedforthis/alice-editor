#version 400 core

#define PI 3.1415926538

in vec2 texcoord;
flat in vec2 chunk;

uniform float zoom;
uniform float pixel_x;
uniform float pixel_y;
uniform vec2 hovered_province;
uniform vec2 selected_province;
uniform vec2 size;

uniform sampler2D province_indices;
uniform sampler2D is_sea_texture;
uniform sampler2D state_data;
uniform sampler2D owner_data;

layout (location = 0) out vec4 out_color;

void main()
{
    vec2 dtex = texcoord;
    vec2 dchunk = chunk;

    float chunks_size = 2.f;

    vec2 true_textcoord = dchunk + dtex / chunks_size;

    float u = true_textcoord.x;
    float v = true_textcoord.y;

    // vec2 modif = vec2(u * 10 - 5, v * 10 - 5);

    // out_color = vec4(texture(data, texcoord).r, 0.f, 0.f, 1.f);
    // return;

    vec2 base_shift = 1.f / size;

    vec2 base_shift_y = vec2(0.f, 1/size.y);
    vec2 base_shift_x = vec2(1/size.x, 0.f);

    vec2 coord = vec2(u, v);

    vec2 pixel = floor(coord * size);
    vec2 local_coord = fract(coord * size + 1.0/512.0);
    vec2 half_of_province_pixel = vec2(0.f, 0.f); //vec2(1.f, 1.f) / 1024.f / 4.f;

    float border_inner;

    if (zoom > 1.f) {
        border_inner = 1.f;
    } else if (zoom < 1.f / 16.f) {
        border_inner = 0.1f;
    } else {
        float t = (zoom - 1.f/16.f) / 1.f;
        border_inner = (1.f - t) * 0.1f + t * 1.f;
    }

    bool border_top = false;
    bool border_bottom = false;
    bool border_left = false;
    bool border_right = false;

    bool is_state_border = false;
    bool is_nation_border = false;

    vec2 prov_local = texture(province_indices, vec2(coord)).xy / 256.f * 255.f;
    vec2 state_local = texture(state_data, prov_local + half_of_province_pixel).xy;
    vec2 owner_local = texture(owner_data, prov_local + half_of_province_pixel).xy;

    vec2 prov_top = texture(province_indices, vec2(coord + base_shift_y*border_inner)).xy / 256.f * 255.f;
    vec2 prov_bottom = texture(province_indices, vec2(coord - base_shift_y*border_inner)).xy / 256.f * 255.f;
    vec2 prov_left = texture(province_indices, vec2(coord - base_shift_x*border_inner)).xy / 256.f * 255.f;
    vec2 prov_right = texture(province_indices, vec2(coord + base_shift_x*border_inner)).xy / 256.f * 255.f;

    vec2 state_top = texture(state_data, prov_top + half_of_province_pixel).xy;
    vec2 state_bottom = texture(state_data, prov_bottom + half_of_province_pixel).xy;
    vec2 state_left = texture(state_data, prov_left + half_of_province_pixel).xy;
    vec2 state_right = texture(state_data, prov_right + half_of_province_pixel).xy;

    vec2 owner_top = texture(owner_data, prov_top + half_of_province_pixel).xy;
    vec2 owner_bottom = texture(owner_data, prov_bottom + half_of_province_pixel).xy;
    vec2 owner_left = texture(owner_data, prov_left + half_of_province_pixel).xy;
    vec2 owner_right = texture(owner_data, prov_right + half_of_province_pixel).xy;


    if (distance(prov_top, prov_local)> 0.00001f) {
        border_top = true;
        if (distance(state_top, state_local)> 0.00001f) {
            is_state_border = true;
        }
        if (distance(owner_top, owner_local)> 0.00001f) {
            is_nation_border = true;
        }
    }
    if (distance(prov_bottom, prov_local)> 0.00001f) {
        border_bottom = true;
        if (distance(state_bottom, state_local)> 0.00001f) {
            is_state_border = true;
        }
        if (distance(owner_bottom, owner_local)> 0.00001f) {
            is_nation_border = true;
        }
    }
    if (distance(prov_left, prov_local)> 0.00001f) {
        border_left = true;
        if (distance(state_left, state_local)> 0.00001f) {
            is_state_border = true;
        }
        if (distance(owner_left, owner_local)> 0.00001f) {
            is_nation_border = true;
        }
    }
    if (distance(prov_right, prov_local)> 0.00001f) {
        border_right = true;
        if (distance(state_right, state_local)> 0.00001f) {
            is_state_border = true;
        }
        if (distance(owner_right, owner_local)> 0.00001f) {
            is_nation_border = true;
        }
    }

    vec4 border_color = vec4(0.f, 0.f, 0.f, 1.f);
    if (is_state_border)
        border_color = vec4(0.5f, 0.0f, 0.0f, 1.f);
    if (is_nation_border)
        border_color = vec4(0.0f, 1.0f, 0.0f, 1.f);

    bool is_border = true;

    if (border_top) {
    } else if (border_bottom) {
    } else if (border_left) {
    } else if (border_right) {
    } else {
        border_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        is_border = false;
    }

    if (is_border) {
        out_color = border_color;
        return;
    }


    vec4 highlight_pixel = vec4(0,0,0,0);

    if (abs(pixel.x - floor(pixel_x)) < 1.f * zoom - 0.01f || abs(pixel.y - floor(pixel_y)) < 1.f * zoom - 0.01f) {
        highlight_pixel = vec4(0.1, 0.1, 0.1, 0);
    }

    vec2 selected_state = texture(state_data, selected_province + half_of_province_pixel).xy;
    vec2 selected_nation = texture(owner_data, selected_province + half_of_province_pixel).xy;

    vec4 highlight_province = vec4(0, 0, 0, 0);
    if (distance(selected_province, prov_local) < 0.00001f) {
        highlight_province = vec4(0.2, 0.5, 0.2, 0);
    }
    if (distance(selected_state, state_local) < 0.00001f) {
        highlight_province += vec4(0.1, 0.2, 0.1, 0);
    }

    vec4 hover_province = vec4(0, 0, 0, 0);
    if (distance(hovered_province, prov_local) < 0.00001f) {
        hover_province = vec4(0.05, 0.05, 0.2, 0);
    }

    float is_sea = texture(is_sea_texture, prov_local + half_of_province_pixel).r;

    vec3 owner = (texture(owner_data, prov_local + half_of_province_pixel).rgb + 1.f) * 0.5f;

    vec3 neutral_vector = vec3(1.f, 1.f, 1.f);
    neutral_vector /= length(neutral_vector);
    vec3 chroma = owner - dot(owner, neutral_vector) * neutral_vector;
    owner += chroma * 0.6f / (length(chroma) + 0.1f);

    vec4 sea_color = vec4(owner, 1.f);
    if (is_sea > 0.8f) {
        sea_color = vec4(0.3f, 0.4f, 0.8f, 1.f);
    }

    out_color = sea_color + highlight_pixel + hover_province + highlight_province;
}