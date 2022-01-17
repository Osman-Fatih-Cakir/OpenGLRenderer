#version 450 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;
layout(location = 3) out vec3 gPbr_materials;
layout(location = 4) out vec3 gEmissive;

in vec2 fTexCoord;
in vec3 fFragPos;
in vec3 fNormal;
in mat3 TBN;

uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D metallic_roughness_map;
uniform sampler2D ao_map;
uniform sampler2D emissive_map;
uniform sampler2D opacity_map;

uniform bool has_normal_map;
uniform bool has_ao_map;
uniform bool has_emissive_map;
uniform bool has_opacity_map;

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
	gAlbedoSpec.rgba = texture(albedo_map, fTexCoord).rgba;
	// Opacity
	if (has_opacity_map)
	{
		gAlbedoSpec.a = texture(opacity_map, fTexCoord).a;
	}
	// Roughness
	gPbr_materials.r = texture(metallic_roughness_map, fTexCoord).g;
	// Metallic
	gPbr_materials.g = texture(metallic_roughness_map, fTexCoord).r;
	// Ambient Occlusion
	if (has_ao_map)
	{
		gPbr_materials.b = texture(ao_map, fTexCoord).r;
	}
	else
	{
		gPbr_materials.b = 1.0;
	}
	// Emissive
	if (has_emissive_map)
	{
		gEmissive.rgb = texture(emissive_map, fTexCoord).rgb;
	}
	else
	{
		gEmissive.rgb = vec3(0.0);
	}
}
