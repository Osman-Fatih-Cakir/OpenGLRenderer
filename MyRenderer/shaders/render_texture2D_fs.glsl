#version 450 core

layout(location = 0) out vec4 OutColor;

in vec2 fTexCoord;

uniform sampler2D image;

void main()
{
    vec3 result = texture(image, fTexCoord).rgb;

    // Tone mapping
    result = vec3(1.0) - exp(-result * 0.1);
    // Gamma correct while we're at it       
    result = pow(result, vec3(1.0 / 2.2));

    OutColor = vec4(result, 1.0);
}