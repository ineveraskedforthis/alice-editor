#pragma once
namespace SHADER_UNIFORMS {
    enum : size_t {
        PROVINCE_INDICES = 0,
        PROVINCE_IS_SEA = 1,
        MODEL = 2,
        VIEW = 3,
        PROJECTION = 4,
        ZOOM = 5,
        PIXEL_X = 6,
        PIXEL_Y = 7,
        SELECTED_PROVINCE = 8,
        HOVERED_PROVINCE = 9,
        STATES_DATA = 10,
        OWNER_DATA,
        SIZE,
        RIVERS,
        MODEL_LINE, VIEW_LINE,
        MODEL_RIVER, VIEW_RIVER,

        TRIANGLE_POINT_0, TRIANGLE_POINT_1, TRIANGLE_POINT_2, TRIANGLE_MODEL, TRIANGLE_VIEW
    };
}