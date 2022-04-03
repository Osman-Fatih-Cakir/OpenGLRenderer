#version 450 core

layout(location = 0) out vec4 OutColor;
layout(location = 1) out vec2 OutVelocity;

in vec2 fTexCoord;

uniform sampler2D image1;
uniform sampler2D image2;
uniform sampler2D velocity_map;

uniform bool horizontal;

void main()
{
    vec3 result = texture(image1, fTexCoord).rgb + texture(image2, fTexCoord).rgb;

    OutColor = vec4(result, 1.0);

    OutVelocity = texture(velocity_map, fTexCoord).rg;
}
