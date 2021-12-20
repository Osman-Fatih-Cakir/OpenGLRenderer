#version 450 core

out vec4 OutColor;

uniform vec3 color;

void main()
{
    // HDR tonemap and gamma correct
    vec3 col = color / (color + vec3(1.0));
    col = pow(col, vec3(1.0 / 2.2));

    OutColor = vec4(col, 1.0);
}
