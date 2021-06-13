#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out vec3 fFragPos;
out vec2 fTexCoord;
out vec3 fNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(vPos, 1.0);

    // I use world coordinates for light calculations
    fFragPos = (model * vec4(vPos, 1.0)).xyz;
    fTexCoord = vTexCoord;
    // TODO calculate normal matrix in C++ and send it here
    fNormal = transpose(inverse(mat3(model))) * vNormal; // Multiply the normal with normal matrix
}