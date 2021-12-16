#version 450 core

out vec4 OutColor;
in vec3 fPos;

uniform samplerCube skybox_map;

void main()
{
    vec3 color = texture(skybox_map, fPos).rgb;

    // HDR tonemap and gamma correct
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    OutColor = vec4(color, 1.0);
}