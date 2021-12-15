#version 450 core

out vec4 OutColor;
in vec3 fPos;

uniform sampler2D equirectangular_map;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(fPos));
    vec3 color = texture(equirectangular_map, uv).rgb;

    OutColor = vec4(color, 1.0); // vec4(1.0, 0.0, 0.0, 1.0);
}
