#version 450 core

out vec4 OutColor;

in vec2 fTexCoord;

uniform sampler2D image1;
uniform sampler2D image2;

uniform bool horizontal;

void main()
{
    vec3 result = texture(image1, fTexCoord).rgb + texture(image2, fTexCoord).rgb;

    OutColor = vec4(result, 1.0);
}