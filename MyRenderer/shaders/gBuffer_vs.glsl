#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out vec3 fFragPos;
out vec2 fTexCoord;
out vec3 fNormal;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat4 normal_matrix;

void main()
{
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(vPos, 1.0);

    // I use world coordinates for light calculations
    fFragPos = (model_matrix * vec4(vPos, 1.0)).xyz;
    fTexCoord = vTexCoord;
    fNormal = normalize(mat3(normal_matrix) * vNormal); // Multiply the normal with normal matrix
}