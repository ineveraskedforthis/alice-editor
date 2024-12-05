#version 330 core

layout (location = 0) in vec2 vertex;

uniform mat4 model;
uniform mat4 view;

void main()
{
    gl_Position = view * model * vec4(vertex, 0.1, 1.0);
}