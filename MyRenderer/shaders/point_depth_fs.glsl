#version 450 core

in vec4 fPos;
in vec2 fTexCoord;

uniform sampler2D albedo_map;
uniform vec3 light_position;
uniform float far;

out vec4 out_color;

void main()
{
    float alpha = texture(albedo_map, fTexCoord).a;
    if (alpha < 0.1)
        discard;

    // Distance between light and the fragment
    float light_distance = length(fPos.xyz - light_position);

    // Map to [1:0] range
    light_distance = light_distance / far;

    // Override depth value
    gl_FragDepth = light_distance;
}