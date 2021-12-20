#version 450 core

out vec4 OutColor;

in vec2 fTexCoord;

uniform sampler2D image1;
uniform sampler2D image2;

uniform bool horizontal;
uniform float weight[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{
    vec3 result = texture(image1, fTexCoord).rgb + texture(image2, fTexCoord).rgb;

    OutColor = vec4(result, 1.0);
}