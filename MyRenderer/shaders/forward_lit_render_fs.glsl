#version 450 core

in vec3 fFragPos;
in vec2 fTexCoord;
in vec3 fNormal;
in mat3 TBN;

out vec4 OutColor;

uniform sampler2D albedo_map;
uniform sampler2D normal_map;
uniform sampler2D metallic_roughness_map;
uniform sampler2D ao_map;
uniform sampler2D emissive_map;
uniform sampler2D opacity_map;
uniform samplerCube irradiance_map;
uniform samplerCube prefiltered_map;
uniform sampler2D brdf_lut;
uniform float MAX_REFLECTION_LOD;
uniform int is_ibl_active;

uniform bool has_normal_map;
uniform bool has_ao_map;
uniform bool has_emissive_map;
uniform bool has_opacity_map;

struct Point_Light
{
	vec3 position;
	vec3 color;

	int cast_shadow;
	samplerCube point_shadow_map;
	float far;

	float cutoff;
	float half_radius;
	float linear;
	float quadratic;
	float intensity;
};
uniform int NUMBER_OF_POINT_LIGHTS;
uniform Point_Light point_lights[32]; // Max number of point lights is 32

struct Direct_Light
{
	vec3 direction;
	vec3 color;

	int cast_shadow;
	mat4 light_space_matrix;
	sampler2D directional_shadow_map;

	float intensity;
};
uniform int NUMBER_OF_DIRECT_LIGHTS; 
uniform Direct_Light direct_lights[4]; // Max number of directional lights is 4

uniform vec3 viewer_pos;

const float PI = 3.14159265359;

// Returns shadow value for directional light, (1.0: shadow, 0.0: non-shadow) 
float directional_shadow_calculation(int light_index, vec3 _fPos, float bias)
{
	// Perspective divide
	vec4 frag_pos_for_light = direct_lights[light_index].light_space_matrix * vec4(_fPos, 1.0);
	vec3 proj_coord = frag_pos_for_light.xyz / frag_pos_for_light.w;

	// Transform to [1,0] range
	proj_coord = proj_coord * 0.5 + 0.5;

	// Get depth of current fragment from lights perspective
	float current_depth = proj_coord.z;

	float shadow = 0.0;

	// Soft shadows (PCF)
	vec2 texel_size = 1.0 / textureSize(direct_lights[light_index].directional_shadow_map, 0);
	for (int i = -4; i <= 4; i++)
	{
		for (int j = -4; j <= 4; j++)
		{
			float pcf_depth = texture(direct_lights[light_index].directional_shadow_map, proj_coord.xy + vec2(i, j) * texel_size).r;
			shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
		}
	}
	shadow /= 81.0;//9.0;

	if (proj_coord.z > 1.0) // Out of projection borders
	{
		shadow = 0.0;
	}

	return shadow;
}

// Returns shadow value for point light, (1.0: shadow, 0.0: non-shadow)
float point_shadow_calculation(int light_index, vec3 _fPos, float bias)
{
	vec3 light_to_frag = _fPos - point_lights[light_index].position;

	float current_depth = length(light_to_frag);

	float shadow = 0.0;

	// Soft shadows (PCF)
	vec3 sample_offset_directions[20] = vec3[] // The directions
	(
		vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
		vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
		vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
		vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
		vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
		);

	float radius = 0.03;

	for (int i = 0; i < 20; i++)
	{
		float pcf_depth = texture(point_lights[light_index].point_shadow_map,
		  light_to_frag + sample_offset_directions[i] * radius).r;
		pcf_depth *= point_lights[light_index].far;
		if (current_depth - bias > pcf_depth)
			shadow += 1.0;
	}
	shadow /= 20.f;

	return shadow;
}

// Calculate the ratio of surface reflection with Fresnel equation
vec3 fresnel_schlick(float cos_theta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(max(1.0 - cos_theta, 0.0), 5.0);
}

// Calculate normal distribution property
float distribution_GGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

// Calculates geometry-schlick equation
float geometry_schlick_GGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

// Calculate self-shadowing property using Schlick-GGX with Smith method
float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = geometry_schlick_GGX(NdotV, roughness);
	float ggx1 = geometry_schlick_GGX(NdotL, roughness);

	return ggx1 * ggx2;
}

// Calculate irradiance with injecting a roughness term in the Fresnel-Schlick equation
vec3 fresnel_schlick_roughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Calculate the IBL diffuse part
vec3 IBL(vec3 normal, vec3 view_dir, float metallic, float roughness, vec3 albedo, float ao)
{
	// IBL diffuse
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);
	vec3 F = fresnel_schlick_roughness(max(dot(normal, view_dir), 0.0), F0, roughness);
	vec3 kS = F;
	vec3 kD = 1.0 - kS;
	kD *= 1.0 - metallic;
	vec3 irradiance = texture(irradiance_map, normal).rgb;
	vec3 diffuse = irradiance * albedo;

	// IBL specular
	vec3 R = reflect(-view_dir, normal);
	// Sample both the pre-filter map and the BRDF lut and combine them 
	// together as per the Split-Sum approximation to get the IBL specular part
	vec3 prefiltered_color = textureLod(prefiltered_map, R, roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf = texture(brdf_lut, vec2(max(dot(normal, view_dir), 0.0), roughness)).rg;
	vec3 specular = prefiltered_color * (F * brdf.x + brdf.y);

	// Ambient
	vec3 ambient = ((kD * diffuse + specular) * ao);// * vec3(0.4); // TODO check if this is okay
	return ambient;
}

// Calculate point lighting
vec3 calculate_point_light(vec3 frag_pos, vec3 normal, vec3 albedo, float roughness, float metallic)
{
	vec3 col = vec3(0.0);

	// Point light calculations
	for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++) // Calculate lighting for all lights
	{
		vec3 Lo = vec3(0.0);

		// If the fragment is not inside the light radious, no need to make calculation for that light
		if (length(point_lights[i].position - frag_pos) > point_lights[i].half_radius * 2.0)
			continue;

		// View direction
		vec3 view_dir = normalize(viewer_pos - frag_pos);
		// Light direction
		vec3 light_dir = normalize(point_lights[i].position - frag_pos);
		// Halfway vector
		vec3 halfway = normalize(view_dir + light_dir);

		float shadow = 0.0;
		// Calculate shadow
		if (point_lights[i].cast_shadow != 0)
		{
			float max_bias = 0.5;
			float min_bias = 0.1;
			float bias = max(max_bias * (1.0 - dot(normal, light_dir)), min_bias);
			shadow = point_shadow_calculation(i, frag_pos, bias);
		}

		// If the fragment is in the shadow, there is no need for lighting calculations
		if (shadow >= 1.0)
			continue;

		// Surface reflection at zero incidence (F0)
		vec3 F0 = vec3(0.04);
		F0 = mix(F0, albedo, metallic);

		// Cook-Torrance specular BRDF variables
		float NDF = distribution_GGX(normal, halfway, roughness);
		float G = geometry_smith(normal, view_dir, light_dir, roughness);
		vec3 F = fresnel_schlick(max(dot(halfway, view_dir), 0.0), F0);

		// Calculate refracted light (kD)
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		// Calculate Cook-Torrance specular BRDF
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(normal, view_dir), 0.0) * max(dot(normal, light_dir), 0.0);
		vec3 specular = numerator / max(denominator, 0.001);

		// Attenuation
		float distance = length(point_lights[i].position - frag_pos);
		float attenuation = 1.0 / (1.0 + point_lights[i].linear * distance
			+ point_lights[i].quadratic * distance * distance);

		// Decrase attenuation heavily near radius
		attenuation *= clamp((point_lights[i].cutoff * distance) + point_lights[i].half_radius - distance,
			0.0, 1.0);

		// Calculate Lo
		float angle = max(dot(normal, light_dir), 0.0);
		Lo += (kD * albedo / PI + specular) * attenuation * angle;
		Lo *= point_lights[i].intensity * (1.0 - shadow) * point_lights[i].color;

		col += Lo;
	}

	return col;
}

// Calculate directional lighting
vec3 calculate_direct_light(vec3 frag_pos, vec3 normal, vec3 albedo, float roughness, float metallic)
{
	vec3 col = vec3(0.0);

	// Directional light calculations
	for (int i = 0; i < NUMBER_OF_DIRECT_LIGHTS; i++)
	{
		vec3 Lo = vec3(0.0);

		// View direction
		vec3 view_dir = normalize(viewer_pos - frag_pos);
		// Light direction
		vec3 light_dir = normalize(-direct_lights[i].direction);
		// Halfway vector
		vec3 halfway = normalize(view_dir + light_dir);

		float shadow = 0.0;
		if (direct_lights[i].cast_shadow != 0.0)
		{
			// Calculate shadow
			float max_bias = 0.002;
			float min_bias = 0.00001;
			float bias = max(max_bias * (1.0 - dot(normal, light_dir)), min_bias);
			shadow = directional_shadow_calculation(i, frag_pos, bias);
		}

		// If the fragment is in the shadow, there is no need for lighting calculations
		if (shadow >= 1.0)
			continue;

		// Surface reflection at zero incidence (F0)
		vec3 F0 = vec3(0.04);
		F0 = mix(F0, albedo, metallic);

		// Cook-Torrance specular BRDF variables
		float NDF = distribution_GGX(normal, halfway, roughness);
		float G = geometry_smith(normal, view_dir, light_dir, roughness);
		vec3 F = fresnel_schlick(max(dot(halfway, view_dir), 0.0), F0);

		// Calculate refracted light (kD)
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;

		// Calculate Cook-Torrance specular BRDF
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(normal, view_dir), 0.0) * max(dot(normal, light_dir), 0.0);
		vec3 specular = numerator / max(denominator, 0.001);

		// Calculate Lo
		float angle = max(dot(normal, light_dir), 0.0);
		Lo += (kD * albedo / PI + specular) * angle;
		Lo *= direct_lights[i].intensity * (1.0 - shadow) * direct_lights[i].color;

		col += Lo;
	}
	return col;
}

void main()
{
	// Albdedo
	vec4 albedo = texture(albedo_map, fTexCoord).rgba;
	// For easy render of fully transparency just discard very translucent fragments :)
	if (albedo.a < 0.25)
		discard;
	
	// Normal
	vec3 normal;
	if (has_normal_map)
	{
		vec3 _gNormal = texture(normal_map, fTexCoord).xyz;
		_gNormal = _gNormal * 2.0 - 1.0;
		normal = normalize(TBN * _gNormal);
	}
	else
	{
		// TODO check here (Meshes without normal maps)
		/*
		vec3 _gNormal = fNormal * 2.0 - 1.0;
		normal = normalize(TBN * _gNormal);
		*/
		normal = normalize(fNormal);
	}
	// Opacity map (if there is any)
	if (has_opacity_map)
	{
		albedo.a = texture(opacity_map, fTexCoord).a;
	}
	// Roughness
	float roughness = texture(metallic_roughness_map, fTexCoord).g;
	// Metallic
	float metallic = texture(metallic_roughness_map, fTexCoord).r;
	// Ambient Occlusion
	float ao = 1.0;
	if (has_ao_map)
	{
		ao = texture(ao_map, fTexCoord).r;
	}
	// Emissive
	vec3 emissive = vec3(0.0);
	if (has_emissive_map)
	{
		emissive = texture(emissive_map, fTexCoord).rgb;
	}

	vec3 view_dir = normalize(viewer_pos - fFragPos);

	// Outgoing light
	vec3 Lo = vec3(0.0);

	// Point light calculation
	Lo += calculate_point_light(fFragPos, normal, albedo.xyz, roughness, metallic);

	// Direct light calculation
	Lo += calculate_direct_light(fFragPos, normal, albedo.xyz, roughness, metallic);

	if (is_ibl_active != 0)
	{
		// IBL
		Lo += IBL(normal, view_dir, metallic, roughness, albedo.xyz, ao);
	}
	else
	{
		Lo += vec3(0.01) * albedo.xyz * ao;
	}

	// Emissive
	Lo += emissive;

	// Tone mapping
	Lo = vec3(1.0) - exp(-Lo * 1.0);
	// Gamma correct while we're at it       
	Lo = pow(Lo, vec3(1.0 / 2.2));

	OutColor = vec4(Lo, albedo.a);
}
