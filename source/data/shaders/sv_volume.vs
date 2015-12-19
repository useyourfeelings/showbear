#version 450

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

out vec3 VPosition;

uniform mat4 mv;
uniform mat4 mvp;
uniform mat3 normal_transform;

void main()
{
    VPosition = (mv * vec4(vertex_position, 1.0)).xyz;
    gl_Position = mvp * vec4(vertex_position, 1.0);
}
