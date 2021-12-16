#version 450 core

layout(location = 0) in vec3 vPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 fPos;

void main()
{
	fPos = vPos;

	mat4 rot_view = mat4(mat3(view));
	vec4 clip_pos = projection * rot_view * vec4(fPos, 1.0);

	gl_Position = clip_pos.xyww;
}