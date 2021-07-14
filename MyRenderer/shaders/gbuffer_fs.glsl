#version 450

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

in vec2 fTexCoord;
in vec3 fFragPos;
in vec3 fNormal;

uniform sampler2D diffuse;

void main()
{
	// Fragment position
	gPosition = fFragPos;
	// Fragment normal
	gNormal = fNormal;
	// Diffuse
	gAlbedoSpec.rgb = texture(diffuse, fTexCoord).rgb;
	// Specular (float number)
	gAlbedoSpec.a = 0.5;
}
