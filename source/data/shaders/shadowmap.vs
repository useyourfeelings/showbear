#version 450

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;

out vec3 normal;
out vec3 position;
out vec4 position_lightspace;

uniform mat4 mv;
uniform mat3 normal_transform;
uniform mat4 mvp;
uniform mat4 mvp_light;

void main()
{
    position = (mv * vec4(vertex_position, 1.0)).xyz;
    normal = normalize( normal_transform * vertex_normal);
    position_lightspace = mvp_light * vec4(vertex_position, 1.0);
    gl_Position = mvp * vec4(vertex_position, 1.0);
}
