#version 450

layout(triangles_adjacency) in;
layout(triangle_strip, max_vertices = 12) out;

in vec3 VPosition[];

uniform vec4 light_position;
uniform mat4 projection_matrix;

bool FaceLight(vec4 light_pos, vec3 a, vec3 b, vec3 c)
{
    vec3 ab = b - a;
    vec3 ac = c - a;

    return dot(cross(ab, ac), light_pos.xyz - a) > 0;
}

void EmitEdgeQuad(vec4 light_pos, vec3 a, vec3 b)
{
    gl_Position = projection_matrix * vec4(a, 1);
    EmitVertex();

    gl_Position = projection_matrix * vec4(a - light_pos.xyz, 0);
    EmitVertex();

    gl_Position = projection_matrix * vec4(b, 1);
    EmitVertex();

    gl_Position = projection_matrix * vec4(b - light_pos.xyz, 0);
    EmitVertex();

    EndPrimitive();
}

void main()
{
    if(FaceLight(light_position, VPosition[0], VPosition[2], VPosition[4]))
    {
        if(!FaceLight(light_position, VPosition[0], VPosition[1], VPosition[2]))
            EmitEdgeQuad(light_position, VPosition[0], VPosition[2]);
        if(!FaceLight(light_position, VPosition[2], VPosition[3], VPosition[4]))
            EmitEdgeQuad(light_position, VPosition[2], VPosition[4]);
        if(!FaceLight(light_position, VPosition[4], VPosition[5], VPosition[0]))
            EmitEdgeQuad(light_position, VPosition[4], VPosition[0]);
    }
}
