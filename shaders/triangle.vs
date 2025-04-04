#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 point0;
uniform vec2 point1;
uniform vec2 point2;

void main()
{
    vec2 base = vec2(0.f, 0.f);

    if(gl_VertexID == 0) {
        base = point0;
    }

    if(gl_VertexID == 1) {
        base = point1;
    }

    if(gl_VertexID == 2) {
        base = point2;
    }

    gl_Position = view * model * vec4((base.xy * 2.f - 1.f), 0.f, 1.f);
    gl_Position.z = 0.1f;
}