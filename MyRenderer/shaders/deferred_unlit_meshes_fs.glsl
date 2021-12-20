#version 450 core

out vec4 OutColor;

uniform vec3 color;

void main()
{
    // Make the values over 1 so bloom will effect all
    // (Not sure if this is the correct way)
    vec3 col = color * vec3(5.0, 2.0, 20.0);

    OutColor = vec4(col, 1.0);
}
