#version 330 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

in vec2 fTexCoord;
in vec3 fFragPos;
in vec3 fNormal;



void main()
{
	// Fragment position
	gPosition = fFragPos;
	// Fragment normal
	gNormal = fNormal;
	// Diffuse
	gAlbedoSpec.rgb = vec3(0.5, 0.5, 0.5);
	// Specular (float number)
	gAlbedoSpec.a = 0.5;
}
