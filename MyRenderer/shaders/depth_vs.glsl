#version 450

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

uniform mat4 space_matrix;
uniform mat4 model_matrix;

void main()
{
    gl_Position = space_matrix * model_matrix * vec4(vPos, 1.0);
}
