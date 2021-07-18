#version 450

out vec4 OutColor;

uniform vec3 color;

void main()
{
    OutColor = vec4(color, 1.0);
}