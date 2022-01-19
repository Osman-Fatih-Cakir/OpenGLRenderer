#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

in vec2 gTexCoord[];

uniform mat4 light_space_matrix[6];

out vec2 fTexCoord;
out vec4 fPos;

void main()
{
    for (int face = 0; face < 6; face++)
    {
        gl_Layer = face; // gl_Layer is a built-in variable that specifies to which face we render
        for (int i = 0; i < 3; i++)
        {
            fPos = gl_in[i].gl_Position;
            gl_Position = light_space_matrix[face] * fPos;
            fTexCoord = gTexCoord[i];
            EmitVertex();
        }
        EndPrimitive();
    }
}
