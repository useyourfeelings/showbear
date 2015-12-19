#version 450

layout(binding = 0) uniform sampler2D diffuse_specular_texture;
layout(location = 0) out vec4 frag_color;

void main()
{
    vec4 final_color = texelFetch(diffuse_specular_texture, ivec2(gl_FragCoord), 0);
    frag_color = vec4(final_color.xyz, 1);
}
