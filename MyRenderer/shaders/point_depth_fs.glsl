#version 450

in vec4 fPos;

uniform vec3 light_position;
uniform float far;

out vec4 out_color;

void main()
{
    // Distance between light and the fragment
    float light_distance = length(fPos.xyz - light_position);

    // Map to [1:0] range
    light_distance = light_distance / far;

    // Override depth value
    gl_FragDepth = light_distance;
}