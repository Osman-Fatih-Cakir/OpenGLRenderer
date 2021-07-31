#version 450 core

out vec4 OutColor;

in vec2 fTexCoord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Point_Light
{
	vec3 position;
	vec3 color;

	samplerCube point_shadow_map;
	float far;

	float radius;
	float linear;
	float quadratic;
	float intensity;
};
const int NUMBER_OF_POINT_LIGHTS = 1; // TODO number of lights is hardcoded
uniform Point_Light point_lights[NUMBER_OF_POINT_LIGHTS];

struct Direct_Light
{
	vec3 direction;
	vec3 color;

	mat4 light_space_matrix;
	sampler2D directional_shadow_map;

	float intensity;
};
const int NUMBER_OF_DIRECT_LIGHTS = 1; // TODO hardcoded
uniform Direct_Light direct_lights[NUMBER_OF_DIRECT_LIGHTS];

uniform vec3 viewer_pos;

// Returns shadow value for directional light, (1.0: shadow, 0.0: non-shadow) 
float directional_shadow_calculation(int light_index, vec4 _fPos_light_space, float bias)
{
	// Perspective divide
	vec3 proj_coord = _fPos_light_space.xyz / _fPos_light_space.w;

	// Transform to [1,0] range
	proj_coord = proj_coord * 0.5 + 0.5;

	// Get depth of current fragment from lights perspective
	float current_depth = proj_coord.z;

	float shadow = 0.0;

	// Soft shadows (PCF)
	vec2 texel_size = 1.0 / textureSize(direct_lights[light_index].directional_shadow_map, 0);
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			float pcf_depth = texture(direct_lights[light_index].directional_shadow_map, proj_coord.xy + vec2(i, j) * texel_size).r;
			shadow += current_depth - bias > pcf_depth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

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

	float radius = 0.08;

	for (int i = 0; i < 20; i++)
	{
		float pcf_depth = texture(point_lights[light_index].point_shadow_map, light_to_frag + sample_offset_directions[i] * radius).r;
		pcf_depth *= point_lights[light_index].far;
		if (current_depth - bias > pcf_depth)
			shadow += 1.0;
	}

	shadow /= 20.f;

	return shadow;
}

void main()
{
	// Sample the data from gBuffer
	vec3 frag_pos = texture(gPosition, fTexCoord).rgb;
	vec3 normal = texture(gNormal, fTexCoord).rgb;
	vec3 diffuse = texture(gAlbedoSpec, fTexCoord).rgb;
	float spec = texture(gAlbedoSpec, fTexCoord).a;

	vec3 Ambient = diffuse * 0.005;
	// Calculate the lighting using Blinn-Phong
	vec3 view_dir = normalize(viewer_pos - frag_pos);

	vec3 lighting = vec3(0.0);
	
	// Directional light calculations
	for (int i = 0; i < NUMBER_OF_DIRECT_LIGHTS; i++)
	{
		// Diffuse
		vec3 light_dir = normalize(-direct_lights[i].direction);
		vec3 Diffuse = max(dot(normal, light_dir), 0.0) * diffuse * direct_lights[i].color;

		// Specular
		vec3 halfway = normalize(light_dir + view_dir);
		float specular = pow(max(dot(normal, halfway), 0), 1.0); // TODO shineness is hardcoded
		vec3 Specular = specular * point_lights[i].color * spec;

		// Calculate shadow
		vec4 fPos_light_space = direct_lights[i].light_space_matrix * vec4(frag_pos, 1.0);
		float bias = max(0.001 * (1.0 - dot(normal, light_dir)), 0.001);
		float shadow = directional_shadow_calculation(i, fPos_light_space, bias);

		lighting += (Diffuse + Specular) * direct_lights[i].intensity * (1.0 - shadow);
	}

	// Point light calculations
	for (int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++) // Calculate lighting for all lights
	{
		// Point light radius
		// If the fragment is not inside the light radious, no need to make calculation for that light
		if (length(point_lights[i].position - frag_pos) > point_lights[i].radius)
			continue;

		// Diffuse
		vec3 light_dir = normalize(point_lights[i].position - frag_pos);
		vec3 Diffuse = max(dot(normal, light_dir), 0.0) * diffuse * point_lights[i].color;
		
		// Specular
		vec3 halfway = normalize(light_dir + view_dir);
		float specular = pow(max(dot(normal, halfway), 0), 4.0); // TODO shineness is hardcoded
		vec3 Specular = specular * point_lights[i].color * spec;

		// Attenuation
		float distance = length(point_lights[i].position - frag_pos);
		float attenuation = 1.0 / (1.0 + point_lights[i].linear * distance + point_lights[i].quadratic * distance * distance);
		Diffuse *= attenuation;
		Specular *= attenuation;
		
		float bias = max(0.0001 * (1.0 - dot(normal, light_dir)), 0.0001);
		float shadow = point_shadow_calculation(i, frag_pos, bias);
		lighting += (Diffuse + Specular) * (1.0 - shadow) * point_lights[i].intensity;
	}

	lighting += Ambient; // Add ambient light at the end

	OutColor = vec4(lighting, 1.0);
}
