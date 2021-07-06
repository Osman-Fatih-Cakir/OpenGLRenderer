#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 light_space_matrix[6];

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
            EmitVertex();
        }
        EndPrimitive();
    }
}
