#version 450

uniform struct LightInfo
{
    vec4 position;
    vec3 intensity;
} light;

uniform struct MaterialInfo
{
    vec3 ka;
    vec3 kd;
    vec3 ks;
    float shininess;
} material;

in vec3 position; // vertex position in eye space
in vec3 normal;   // vertex normal in eye space

layout (location = 0) out vec4 frag_color;

// calculate Phong lighting in eye space
vec3 ADS()
{
    vec3 n = normal;
    if(!gl_FrontFacing) // light back face
        n = -n;
    vec3 s = normalize(vec3(light.position) - position); // light to vertex vector

    // diffuse
    float s_dot_n = max(dot(s,n), 0.0); // normal direction component of the light
    vec3 diffuse = light.intensity * material.kd * s_dot_n;

    vec3 v = normalize(-position.xyz);
    vec3 r = reflect(-s, n); // get reflection direction from the incident vector and surface normal

    // specular
    vec3 specular = vec3(0.0);
    if(s_dot_n > 0.0)
        specular = light.intensity * material.ks * pow(max(dot(r, v), 0.0), material.shininess); // eye direction component of the reflection light

    vec3 ambient = light.intensity * material.ka;

    return ambient + diffuse + specular; // add up the three components
}

void main()
{
    frag_color = vec4(ADS(), 1.0);
}
