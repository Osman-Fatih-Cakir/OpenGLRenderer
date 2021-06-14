#version 330 core

out vec4 OutColor;

in vec2 fTexCoord;

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
};
const int NUMBER_OF_LIGHTS = 8; // Max light count // TODO number of lights hardcoded
uniform Light lights[NUMBER_OF_LIGHTS];

uniform vec3 viewer_pos;

void main()
{
	// Sample the data from gBuffer
	vec3 frag_pos = texture(gPosition, fTexCoord).rgb;
	vec3 normal = texture(gNormal, fTexCoord).rgb;
	vec3 diffuse = texture(gAlbedoSpec, fTexCoord).rgb;
	float spec = texture(gAlbedoSpec, fTexCoord).a;

	// Calculate the lighting using Blinn-Phong
	vec3 Ambient = diffuse * 0.1;
	vec3 view_dir = normalize(viewer_pos - frag_pos);

	vec3 lighting = vec3(0.0);
	for (int i = 0; i < NUMBER_OF_LIGHTS; i++) // Calculate lighting for all lights
	{
		/* Should be tested with a big scene
		// If the fragment is not inside the light radious, no need to make calculation for that light
		if (length(lights[i].position - frag_pos) > lights[i].radius)
			continue;
		*/

		// Diffuse
		vec3 light_dir = normalize(lights[i].position - frag_pos);
		vec3 Diffuse = max(dot(normal, light_dir), 0.0) * diffuse * lights[i].color;
		
		// Specular
		vec3 halfway = normalize(light_dir + view_dir);
		float specular = pow(max(dot(normal, halfway), 0), 4.0); // TODO shineness hardcoded(16.0)
		vec3 Specular = specular * lights[i].color * spec;

		// Attenuation
		float distance = length(lights[i].position - frag_pos);
		float attenuation = 1.0 / (1.0 + lights[i].linear * distance + lights[i].quadratic * distance * distance);
		Diffuse *= attenuation;
		Specular *= attenuation;

		lighting += Diffuse + Specular + Ambient;
	}

	OutColor = vec4(lighting, 1.0);
}
