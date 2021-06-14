#version 450

out vec4 OutColor;

uniform vec3 light_color;

void main()
{
    OutColor = vec4(light_color, 1.0);
}