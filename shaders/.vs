#version 330 core

const vec2 VERTICES[6] = vec2[6](
    vec2(-1.0, 1.0),
    vec2(-1.0, -1.0),
    vec2(1.0, -1.0),

    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0)
);

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 texcoord;
flat out vec2 chunk;

void main()
{
    vec2 base = VERTICES[gl_VertexID - int(floor(gl_VertexID / 6.f) * 6.f)];
    texcoord = (base.xy + 1.0) / 2.f;

    float chunks_size = 16.f;

    float square_index = floor(gl_VertexID / 6.f);
    float y = floor(square_index / chunks_size);
    float x = square_index - y * chunks_size;
    chunk = vec2(x / chunks_size, y / chunks_size);

    gl_Position = view * model * vec4((chunk + texcoord / chunks_size) * 2.f - 1.f, 0.f, 1.f);
}