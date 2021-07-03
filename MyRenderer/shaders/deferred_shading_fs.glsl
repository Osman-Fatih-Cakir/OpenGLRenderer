#version 330 core

out vec4 OutColor;

in vec2 fTexCoord;
in vec4 fPos_light_space;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light
{
	vec3 position;
	vec3 color;

	float radius;
	float linear;
	float quadratic;
};// TODO change names as "point lights"
const int NUMBER_OF_LIGHTS = 8; // TODO number of lights is hardcoded
uniform Light lights[NUMBER_OF_LIGHTS];

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

	// Get closest value from light perspective
	float closest_depth = texture(direct_lights[light_index].directional_shadow_map, proj_coord.xy).r;

	// Get depth of current fragment from lights perspective
	float current_depth = proj_coord.z;

	// Check if the fragment is in the shadow or not
	float shadow = 0.0;

	// TODO add pcf
	shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;

	if (proj_coord.z > 1.0) // Out of projection borders
	{
		shadow = 0.0;
	}

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

	
	// Point light calculations
	for (int i = 0; i < NUMBER_OF_LIGHTS; i++) // Calculate lighting for all lights
	{
		// Should be tested with a bigger scene
		//// If the fragment is not inside the light radious, no need to make calculation for that light
		//if (length(lights[i].position - frag_pos) > lights[i].radius)
		//	continue;
		

		// Diffuse
		vec3 light_dir = normalize(lights[i].position - frag_pos);
		vec3 Diffuse = max(dot(normal, light_dir), 0.0) * diffuse * lights[i].color;
		
		// Specular
		vec3 halfway = normalize(light_dir + view_dir);
		float specular = pow(max(dot(normal, halfway), 0), 4.0); // TODO shineness is hardcoded
		vec3 Specular = specular * lights[i].color * spec;

		// Attenuation
		float distance = length(lights[i].position - frag_pos);
		float attenuation = 1.0 / (1.0 + lights[i].linear * distance + lights[i].quadratic * distance * distance);
		Diffuse *= attenuation;
		Specular *= attenuation;

		lighting += Diffuse + Specular;
	}
	
	
	// Directional light calculations
	for (int i = 0; i < NUMBER_OF_DIRECT_LIGHTS; i++)
	{
		// Diffuse
		vec3 light_dir = normalize(-direct_lights[i].direction);
		vec3 Diffuse = max(dot(normal, light_dir), 0.0) * diffuse * direct_lights[i].color;

		// Specular
		vec3 halfway = normalize(light_dir + view_dir);
		float specular = pow(max(dot(normal, halfway), 0), 4.0); // TODO shineness is hardcoded
		vec3 Specular = specular * lights[i].color * spec;

		// Calculate shadow
		float shadow = 0.0;
		// TODO add point shadow
		vec4 fPos_light_space = direct_lights[i].light_space_matrix * vec4(frag_pos, 1.0);
		float dir_bias = 0.0;//max(0.0 * (1.0 - dot(normal, light_dir)), 0.0);
		shadow = directional_shadow_calculation(i, fPos_light_space, dir_bias);

		lighting += (Diffuse + Specular) * direct_lights[i].intensity * (1.0 - shadow);
	}


	lighting += Ambient; // Add ambient light at the end

	OutColor = vec4(lighting, 1.0);
}
