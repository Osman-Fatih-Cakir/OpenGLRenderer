#version 450 core

out vec4 OutColor;

uniform vec3 color;

void main()
{

    // TODO test col * intensity
    // If works, take color as color*intensity*vec3() for optimize

    // Make the values over 1 so bloom will effect all
    // (Not sure if this is the correct way)
    vec3 col = color * vec3(50.0, 50.0, 50.0);

    OutColor = vec4(col, 1.0);
}
