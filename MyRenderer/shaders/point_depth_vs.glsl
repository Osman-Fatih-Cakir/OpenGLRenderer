#version 450 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;


uniform mat4 model;

void main()
{
    gl_Position = model * vec4(vPos, 1.0);
}