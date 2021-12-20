#version 450 core

out vec4 OutColor;
in vec3 fPos;

uniform samplerCube skybox_map;

void main()
{
    vec3 color = texture(skybox_map, fPos).rgb;

    OutColor = vec4(color, 1.0);
}