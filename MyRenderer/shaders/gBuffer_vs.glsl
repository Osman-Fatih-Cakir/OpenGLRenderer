#version 450 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;
layout(location = 3) in vec3 vTangent;
layout(location = 4) in vec3 vBitangent;

out vec3 fFragPos;
out vec2 fTexCoord;
out vec3 fNormal;
out mat3 TBN;

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
    // Calculate TBN matrix
    vec3 T = normalize(vec3(model_matrix * vec4(vTangent, 0.0)));
    vec3 B = normalize(vec3(model_matrix * vec4(vBitangent, 0.0)));
    vec3 N = normalize(vec3(model_matrix * vec4(vNormal, 0.0)));
    TBN = mat3(T, B, N);
    fNormal = normalize(mat3(normal_matrix) * vNormal); // Multiply the normal with normal matrix
}
