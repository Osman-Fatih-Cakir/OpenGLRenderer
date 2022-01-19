#version 450 core

in vec2 fTexCoord;

uniform sampler2D albedo_map;

void main()
{
    float alpha = texture(albedo_map, fTexCoord).a;
    if (alpha < 0.25)
        discard;
    // gl_FragDepth = gl_FragCoord.z;
}