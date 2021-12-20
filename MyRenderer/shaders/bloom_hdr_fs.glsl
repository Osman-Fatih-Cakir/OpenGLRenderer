#version 450 core

layout(location = 0) out vec4 OutColor;
layout(location = 1) out vec4 BrightColor;

in vec2 fTexCoord;

uniform sampler2D image;

void main()
{
    vec3 result = texture(image, fTexCoord).rgb;

    // Take bright values
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    OutColor = vec4(result, 1.0);
}