#version 450

uniform struct LightInfo
{
    vec4 position;
    vec3 intensity;
}light;

uniform struct MaterialInfo
{
    vec3 ka;
    vec3 kd;
    vec3 ks;
    float shininess;
}material;

uniform uint pcf;

in vec3 position;
in vec3 normal;
in vec4 position_lightspace;

layout (location = 0) out vec4 frag_color;
layout (binding = 0) uniform sampler2DShadow shadowmap;

// specular and diffuse light
vec3 DS()
{
    vec3 n = normal;
    if(!gl_FrontFacing)
        n = -n;
    vec3 s = normalize(vec3(light.position) - position);

    // diffuse
    float s_dot_n = max( dot(s,n), 0.0 );
    vec3 diffuse = light.intensity * material.kd * s_dot_n;

    vec3 v = normalize(-position.xyz);
    vec3 r = reflect(-s, n);

    // specular
    vec3 specular = vec3(0.0);
    if(s_dot_n > 0.0)
        specular = light.intensity * material.ks * pow(max(dot(r,v), 0.0), material.shininess);

    return diffuse + specular;
}

subroutine void RenderType();
subroutine uniform RenderType Render;

subroutine(RenderType)
void RenderWithShadow()
{
    float shadow = textureProj(shadowmap, position_lightspace);

    if(pcf == 1) // percentage-closer filtering
    {
        float sum = 0;
        sum += textureProjOffset(shadowmap, position_lightspace, ivec2(-1, -1));
        sum += textureProjOffset(shadowmap, position_lightspace, ivec2(-1, 1));
        sum += textureProjOffset(shadowmap, position_lightspace, ivec2(1, 1));
        sum += textureProjOffset(shadowmap, position_lightspace, ivec2(1, -1));

        sum += textureProjOffset(shadowmap, position_lightspace, ivec2(-1, 0));
        sum += textureProjOffset(shadowmap, position_lightspace, ivec2(0, 1));
        sum += textureProjOffset(shadowmap, position_lightspace, ivec2(1, 0));
        sum += textureProjOffset(shadowmap, position_lightspace, ivec2(0, -1));

        shadow = sum * 0.125;
    }

    vec3 ds = DS();

    //ambient light
    vec3 ambient = light.intensity * material.ka;

    frag_color = vec4(ds * shadow + ambient, 1.0);
}

subroutine(RenderType)
void RenderNothing()
{
}

void main()
{
    Render();
}
