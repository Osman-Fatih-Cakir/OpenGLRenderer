#version 450 core

out vec4 OutColor;
in vec3 fPos;

uniform sampler2D equirectangular_map;
uniform float exposure;

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

    // Convert colors to LDR because images might have very high rgb values that are higher
    // than the float size, causing inf values
    
    // Tone mapping
    color = vec3(1.0) - exp(-color * exposure);
    // Gamma correct while we're at it       
    color = pow(color, vec3(1.0 / 2.2));

    OutColor = vec4(color, 1.0);
}
