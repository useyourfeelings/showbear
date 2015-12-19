#version 450

layout (location = 0) out vec4 frag_color;

void main()
{
    frag_color = vec4(0.4, 0.5, 0.6, 1); // just for volume rendering
}
