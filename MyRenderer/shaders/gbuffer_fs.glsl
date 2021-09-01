#version 450 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;
layout(location = 3) out vec3 gPbr_materials;

in vec2 fTexCoord;
in vec3 fFragPos;
in vec3 fNormal;
in mat3 TBN;

uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D metallic_map;
uniform sampler2D roughness_map;
uniform sampler2D ao_map;

uniform bool has_normal_map;

void main()
{
	// Fragment position
	gPosition = fFragPos;
	// Fragment normal
	if (has_normal_map)
	{
		vec3 _gNormal = texture(normal_map, fTexCoord).xyz;
		_gNormal = _gNormal * 2.0 - 1.0;
		gNormal = normalize(TBN * _gNormal);
	}
	else
	{
		// TODO check here (Meshes without normal maps)
		/*
		vec3 _gNormal = fNormal * 2.0 - 1.0;
		gNormal = normalize(TBN * _gNormal);
		*/
		gNormal = normalize(fNormal);
	}
	
	// Albdedo
	gAlbedoSpec.rgb = texture(albedo_map, fTexCoord).rgb;

	// Roughness
	gPbr_materials.r = texture(roughness_map, fTexCoord).r;
	// Metallic
	gPbr_materials.g = texture(metallic_map, fTexCoord).r;
	// Ambient Occlusion
	gPbr_materials.b = texture(ao_map, fTexCoord).r;
}
