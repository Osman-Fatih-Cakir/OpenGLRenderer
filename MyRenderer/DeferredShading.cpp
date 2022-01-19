
#include <DeferredShading.h>

#include <init_shaders.h>

// Constructor
DeferredShading::DeferredShading()
{
	// Initialize and compile shaders
	init_shaders();

	// Get uniform locations from program
	get_uniform_locations();

	// Create a quad for rendering as texture
	init_quad();
}

// Destructor
DeferredShading::~DeferredShading()
{
	// Deallocate quad buffers
	glDeleteVertexArrays(1, &quad_VAO);
	glDeleteBuffers(1, &quadVBO);
}

void DeferredShading::start_program(gBuffer* _GBuffer, MainFramebuffer* fb)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fb->get_FBO());
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

	GBuffer = _GBuffer;
	framebuffer = fb;

	point_light_count = 0;
	direct_light_count = 0;

	glUseProgram(program);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredShading::change_viewport_resolution(unsigned int w, unsigned int h)
{
	width = w;
	height = h;
}

// Sets viewer position
void DeferredShading::set_viewer_pos(vec3 vec)
{
	glUniform3fv(loc_viewer_pos, 1, &vec[0]);
}

// Sets g-buffer position color attachment
void DeferredShading::set_gPosition(GLuint id)
{
	glUniform1i(loc_gPosition, texture_uniform_starting_point);
	glActiveTexture(GL_TEXTURE0 + texture_uniform_starting_point);
	glBindTexture(GL_TEXTURE_2D, id);
}

// Sets g-buffer normal color attachment
void DeferredShading::set_gNormal(GLuint id)
{
	glUniform1i(loc_gNormal, texture_uniform_starting_point+1);
	glActiveTexture(GL_TEXTURE0 + texture_uniform_starting_point + 1);
	glBindTexture(GL_TEXTURE_2D, id);
}

// Sets g-buffer Albdeo Specs color attachment
void DeferredShading::set_gAlbedoSpec(GLuint id)
{
	glUniform1i(loc_gAlbedoSpec, texture_uniform_starting_point + 2);
	glActiveTexture(GL_TEXTURE0 + texture_uniform_starting_point + 2);
	glBindTexture(GL_TEXTURE_2D, id);
}

// Sets g-buffer roughness color attachment
void DeferredShading::set_gPbr_materials(GLuint id)
{
	glUniform1i(loc_gPbr_materials, texture_uniform_starting_point + 3);
	glActiveTexture(GL_TEXTURE0 + texture_uniform_starting_point + 3);
	glBindTexture(GL_TEXTURE_2D, id);
}

// Set irradiance map to the uniform
void DeferredShading::set_irradiance_map(GLuint id)
{
	glUniform1i(loc_irradiance_map, texture_uniform_starting_point + 4);
	glActiveTexture(GL_TEXTURE0 + texture_uniform_starting_point + 4);
	if (!glIsTexture(id) || id == -1)
		return;
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

// Set prefiltered map to the uniform
void DeferredShading::set_prefiltered_map(GLuint id)
{
	glUniform1i(loc_prefiltered_map, texture_uniform_starting_point + 5);
	glActiveTexture(GL_TEXTURE0 + texture_uniform_starting_point + 5);
	if (!glIsTexture(id) || id == -1)
		return;
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

// Set brdf lut to the uniform
void DeferredShading::set_brdf_lut(GLuint id)
{
	glUniform1i(loc_brdf_lut, texture_uniform_starting_point + 6);
	glActiveTexture(GL_TEXTURE0 + texture_uniform_starting_point + 6);
	if (!glIsTexture(id) || id == -1)
		return;
	glBindTexture(GL_TEXTURE_2D, id);
}
// Set emissive to the uniform
void DeferredShading::set_emissive(GLuint id)
{
	glUniform1i(loc_emissive, texture_uniform_starting_point + 7);
	glActiveTexture(GL_TEXTURE0 + texture_uniform_starting_point + 7);
	if (!glIsTexture(id) || id == -1)
		return;
	glBindTexture(GL_TEXTURE_2D, id);
}

// Set maximum lod of reflections
void DeferredShading::set_max_reflection_lod(float val)
{
	glUniform1f(loc_max_reflection_lod, val);
}

// Set true if the IBL is active
void DeferredShading::set_is_ibl_active(bool val)
{
	glUniform1i(loc_is_ibl_active, (int)val);
}

void DeferredShading::set_point_light
	(
		vec3 position,
		vec3 color,
		float radius,
		float cutoff,
		float half_radius,
		float linear,
		float quadratic, 
		float _far,
		GLuint shadow_map,
		float intensity,
		bool cast_shadow
	)
{
	std::string light_array_str = "point_lights[" + std::to_string(point_light_count) + "].position";
	GLuint loc_lights_pos = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform3fv(loc_lights_pos, 1, &position[0]);

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].color";
	GLuint loc_lights_col = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform3fv(loc_lights_col, 1, &color[0]);

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].radius";
	GLuint loc_light_r = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1f(loc_light_r, (GLfloat) radius);

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].cutoff";
	GLuint loc_cutoff = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1f(loc_cutoff, (GLfloat)cutoff);

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].half_radius";
	GLuint loc_light_hr = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1f(loc_light_hr, (GLfloat)half_radius);

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].linear";
	GLuint loc_light_l = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1f(loc_light_l, (GLfloat) linear);

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].quadratic";
	GLuint loc_light_q = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1f(loc_light_q, (GLfloat) quadratic);

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].far";
	GLuint loc_light_far = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1f(loc_light_far, (GLfloat) _far);

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].cast_shadow";
	GLuint loc_cast_shadow = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1i(loc_cast_shadow, (GLint)cast_shadow);
	
	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].point_shadow_map";
	GLuint loc_light_cubemap = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1i(loc_light_cubemap, static_texture_uniform_count + point_light_count + direct_light_count);
	glActiveTexture(GL_TEXTURE0 + static_texture_uniform_count + point_light_count + direct_light_count );
	if (cast_shadow) // If there is a proper shadow map, assign the uniform
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadow_map);

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].intensity";
	GLuint loc_light_int = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1f(loc_light_int, (GLfloat) intensity);

	point_light_count++; // Count lights
}

void DeferredShading::set_direct_light
(
	vec3 color,
	vec3 direction,
	float intensity,
	GLuint shadow_map,
	mat4 light_space_matrix,
	bool cast_shadow
)
{
	std::string light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].color";
	GLuint loc_lights_pos = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform3fv(loc_lights_pos, 1, &color[0]);

	light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].direction";
	GLuint loc_lights_dir = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform3fv(loc_lights_dir, 1, &direction[0]);

	light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].intensity";
	GLuint loc_lights_in = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1f(loc_lights_in, (GLfloat) intensity);

	light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].cast_shadow";
	GLuint loc_cast_shadow = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1i(loc_cast_shadow, (GLint)cast_shadow);

	light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].directional_shadow_map";
	GLuint loc_lights_sm = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1i(loc_lights_sm, static_texture_uniform_count + point_light_count + direct_light_count);
	glActiveTexture(GL_TEXTURE0 + static_texture_uniform_count + point_light_count + direct_light_count);
	if (cast_shadow) // If there is a proper shadow map, assign the uniform
		glBindTexture(GL_TEXTURE_2D, shadow_map);

	light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].light_space_matrix";
	GLuint loc_lights_lsm = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniformMatrix4fv(loc_lights_lsm, 1, GL_FALSE, &light_space_matrix[0][0]);

	direct_light_count += 1;
}

// Renders the scene
void DeferredShading::render(Camera* camera, Skybox* skybox)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->get_FBO());

	set_viewer_pos(camera->get_position());
	// Set g-buffer color attachments
	set_gPosition(GBuffer->get_gPosition());
	set_gNormal(GBuffer->get_gNormal());
	set_gAlbedoSpec(GBuffer->get_gAlbedoSpec());
	set_gPbr_materials(GBuffer->get_gPbr_materials());
	set_emissive(GBuffer->get_emissive());
	
	// Set IBL textures
	if (skybox != nullptr)
	{
		set_irradiance_map(skybox->get_irradiance_map());
		set_prefiltered_map(skybox->get_prefiltered_map());
		set_brdf_lut(skybox->get_brdf_lut());
		set_max_reflection_lod((float)skybox->get_max_mip_level());
		set_is_ibl_active(skybox->is_ibl_active());
	}
	else
	{
		set_irradiance_map(-1);
		set_prefiltered_map(-1);
		set_brdf_lut(-1);
		set_max_reflection_lod(-1);
		set_is_ibl_active(false);
	}

	// Draw call
	draw_quad(program);
}

// Compiles the shaders and generates the shader program
void DeferredShading::init_shaders()
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/deferred_shading_vs.glsl");
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/deferred_shading_fs.glsl");
	program = initprogram(vertex_shader, fragment_shader);
}

// Get uniforms locations
void DeferredShading::get_uniform_locations()
{
	// Uniform variables (non textures)
	loc_viewer_pos = glGetUniformLocation(program, "viewer_pos");
	loc_max_reflection_lod = glGetUniformLocation(program, "MAX_REFLECTION_LOD");
	// Uniform textures
	loc_gPosition = glGetUniformLocation(program, "gPosition");
	loc_gNormal = glGetUniformLocation(program, "gNormal");
	loc_gAlbedoSpec = glGetUniformLocation(program, "gAlbedoSpec");
	loc_gPbr_materials = glGetUniformLocation(program, "gPbr_materials");
	loc_emissive = glGetUniformLocation(program, "gEmissive");
	loc_irradiance_map = glGetUniformLocation(program, "irradiance_map");
	loc_prefiltered_map = glGetUniformLocation(program, "prefiltered_map");
	loc_brdf_lut = glGetUniformLocation(program, "brdf_lut");
	loc_is_ibl_active = glGetUniformLocation(program, "is_ibl_active");

	static_texture_uniform_count = 5 + texture_uniform_starting_point;
}

// Initialize a quad
void DeferredShading::init_quad()
{
	float quadVertices[] = {

		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 2
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // 4
		1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // 3

		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // 2
		1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // 3
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f // 1
	};

	glGenVertexArrays(1, &quad_VAO);

	// Setup quad VAO
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quad_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindVertexArray(0);
}

// Draw everything on a quad as a texture
void DeferredShading::draw_quad(GLuint shader_program)
{
	glBindVertexArray(quad_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}
