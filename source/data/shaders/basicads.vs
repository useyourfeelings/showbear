#version 450

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;

out vec3 normal;
out vec3 position;

uniform mat4 mv;
uniform mat3 normal_transform;
uniform mat4 mvp;

void main()
{
    position = (mv * vec4(vertex_position, 1.0)).xyz;      // eye space position
    normal = normalize( normal_transform * vertex_normal); // get the normal in eye space
    gl_Position = mvp * vec4(vertex_position, 1.0);        // eye space position with projection
}
