#version 450 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out vec2 gTexCoord;

uniform mat4 model_matrix;

void main()
{
    gTexCoord = vTexCoord;
    gl_Position = model_matrix * vec4(vPos, 1.0);
}