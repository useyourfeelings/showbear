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

in vec3 position;
in vec3 normal;

layout (location = 0) out vec4 ambient_color;
layout (location = 1) out vec4 diffuse_specular_color;

// specular and diffuse light
vec3 DS()
{
    vec3 n = normal;
    if(!gl_FrontFacing)
        n = -n;
    vec3 s = normalize(light.position.xyz - position);

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

void main()
{
    ambient_color = vec4(light.intensity * material.ka, 1.0f);
    diffuse_specular_color = vec4(DS(), 1.0f);
}
