#version 450 core

layout(location = 0) out vec4 OutColor;

in vec2 fTexCoord;

uniform sampler2D image;

void main()
{
    vec3 result = texture(image, fTexCoord).rgb;

    OutColor = vec4(result, 1.0);
}