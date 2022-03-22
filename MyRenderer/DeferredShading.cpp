
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
	glUniform1i(loc_gPosition, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, id);
}

// Sets g-buffer normal color attachment
void DeferredShading::set_gNormal(GLuint id)
{
	glUniform1i(loc_gNormal, 1);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, id);
}

// Sets g-buffer Albdeo Specs color attachment
void DeferredShading::set_gAlbedoSpec(GLuint id)
{
	glUniform1i(loc_gAlbedoSpec, 2);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, id);
}

// Sets g-buffer roughness color attachment
void DeferredShading::set_gPbr_materials(GLuint id)
{
	glUniform1i(loc_gPbr_materials, 3);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, id);
}

// Set irradiance map to the uniform
void DeferredShading::set_irradiance_map(GLuint id)
{
	glUniform1i(loc_irradiance_map, 4);
	glActiveTexture(GL_TEXTURE0 + 4);
	if (!glIsTexture(id) || id == -1)
		return;
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

// Set prefiltered map to the uniform
void DeferredShading::set_prefiltered_map(GLuint id)
{
	glUniform1i(loc_prefiltered_map, 5);
	glActiveTexture(GL_TEXTURE0 + 5);
	if (!glIsTexture(id) || id == -1)
		return;
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

// Set brdf lut to the uniform
void DeferredShading::set_brdf_lut(GLuint id)
{
	glUniform1i(loc_brdf_lut, 6);
	glActiveTexture(GL_TEXTURE0 + 6);
	if (!glIsTexture(id) || id == -1)
		return;
	glBindTexture(GL_TEXTURE_2D, id);
}

// Set emissive to the uniform
void DeferredShading::set_emissive(GLuint id)
{
	glUniform1i(loc_emissive, 7);
	glActiveTexture(GL_TEXTURE0 + 7);
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

// Renders the scene
void DeferredShading::render(gBuffer* GBuffer, MainFramebuffer* fb, Scene* scene)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fb->get_FBO());
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	point_light_count = 0;
	direct_light_count = 0;

	glUseProgram(program);

	// Set camera attributes
	set_uniforms(scene, GBuffer);
	// Draw call
	draw_quad(program);
}

// Set uniforms in shader
void DeferredShading::set_uniforms(Scene* scene, gBuffer* GBuffer)
{
	set_viewer_pos(scene->camera->get_position());
	// Set g-buffer color attachments
	set_gPosition(GBuffer->get_gPosition());
	set_gNormal(GBuffer->get_gNormal());
	set_gAlbedoSpec(GBuffer->get_gAlbedoSpec());
	set_gPbr_materials(GBuffer->get_gPbr_materials());
	set_emissive(GBuffer->get_emissive());

	// Set IBL textures
	Skybox* skybox = scene->get_render_skybox();
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

	set_light_uniforms(scene);

	glUniform1i(loc_point_light_count, point_light_count);
	glUniform1i(loc_direct_light_count, direct_light_count);
}

// Sets light uniforms
void DeferredShading::set_light_uniforms(Scene* scene)
{
	GLuint last_valid_cubemap_id = -1;
	std::vector<int> unassigned_cubemap_ids;
	// Set point lights
	glUniform1i(loc_point_light_count, (GLint)scene->point_lights.size());
	std::vector<PointLight*>::iterator itr;
	for (itr = scene->point_lights.begin(); itr != scene->point_lights.end(); ++itr)
	{
		std::string light_array_str = "point_lights[" + std::to_string(point_light_count) + "].position";
		GLuint loc_lights_pos = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform3fv(loc_lights_pos, 1, &((*itr)->position)[0]);

		light_array_str = "point_lights[" + std::to_string(point_light_count) + "].color";
		GLuint loc_lights_col = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform3fv(loc_lights_col, 1, &((*itr)->color)[0]);

		light_array_str = "point_lights[" + std::to_string(point_light_count) + "].radius";
		GLuint loc_light_r = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_r, (GLfloat)(*itr)->radius);

		light_array_str = "point_lights[" + std::to_string(point_light_count) + "].cutoff";
		GLuint loc_cutoff = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_cutoff, (GLfloat)(*itr)->cutoff);

		light_array_str = "point_lights[" + std::to_string(point_light_count) + "].half_radius";
		GLuint loc_light_hr = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_hr, (GLfloat)(*itr)->half_radius);

		light_array_str = "point_lights[" + std::to_string(point_light_count) + "].linear";
		GLuint loc_light_l = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_l, (GLfloat)(*itr)->linear);

		light_array_str = "point_lights[" + std::to_string(point_light_count) + "].quadratic";
		GLuint loc_light_q = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_q, (GLfloat)(*itr)->quadratic);

		light_array_str = "point_lights[" + std::to_string(point_light_count) + "].far";
		GLuint loc_light_far = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_far, (GLfloat)(*itr)->shadow_projection_far);

		light_array_str = "point_lights[" + std::to_string(point_light_count) + "].cast_shadow";
		GLuint loc_cast_shadow = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform1i(loc_cast_shadow, (GLint)(*itr)->does_cast_shadow());

		if ((*itr)->does_cast_shadow()) // If there is a proper shadow map, assign the uniform
		{
			light_array_str = "point_lights[" + std::to_string(point_light_count) + "].point_shadow_map";
			GLuint loc_light_cubemap = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
			glUniform1i(loc_light_cubemap, static_texture_uniform_count + point_light_count + direct_light_count);
			glActiveTexture(GL_TEXTURE0 + static_texture_uniform_count + point_light_count + direct_light_count);
			glBindTexture(GL_TEXTURE_CUBE_MAP, (*itr)->depth_cubemap);
			last_valid_cubemap_id = (*itr)->depth_cubemap;
		}
		else
		{
			unassigned_cubemap_ids.push_back(point_light_count);
		}

		light_array_str = "point_lights[" + std::to_string(point_light_count) + "].intensity";
		GLuint loc_light_int = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_light_int, (GLfloat)(*itr)->intensity);
		point_light_count++;
	}
	
	// Set the samplerCubemaps to a valid value to suppress the error
	if (last_valid_cubemap_id != -1)
	{
		for (int i = point_light_count; i < max_plight_per_call; i++)
		{
			std::string str = "point_lights[" + std::to_string(i) + "].point_shadow_map";
			GLuint loc = glGetUniformLocation(program, (GLchar*)str.c_str());
			glUniform1i(loc, static_texture_uniform_count + i + direct_light_count);
			glActiveTexture(GL_TEXTURE0 + static_texture_uniform_count + i + direct_light_count);
			glBindTexture(GL_TEXTURE_CUBE_MAP, last_valid_cubemap_id);
		}
		
		for (int i = 0; i < unassigned_cubemap_ids.size(); i++)
		{
			std::string str = "point_lights[" + std::to_string(unassigned_cubemap_ids[i]) + "].point_shadow_map";
			GLuint loc = glGetUniformLocation(program, (GLchar*)str.c_str());
			glUniform1i(loc, static_texture_uniform_count
				+ unassigned_cubemap_ids[i] + direct_light_count);
			glActiveTexture(GL_TEXTURE0 + static_texture_uniform_count
				+ unassigned_cubemap_ids[i] + direct_light_count);
			glBindTexture(GL_TEXTURE_CUBE_MAP, last_valid_cubemap_id);
		}
	}

	// Set directional lights
	glUniform1i(loc_direct_light_count, (GLint)scene->direct_lights.size());
	std::vector<DirectionalLight*>::iterator itrr;
	for (itrr = scene->direct_lights.begin(); itrr != scene->direct_lights.end(); itrr++)
	{
		std::string light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].color";
		GLuint loc_lights_pos = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform3fv(loc_lights_pos, 1, &((*itrr)->get_color())[0]);

		light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].direction";
		GLuint loc_lights_dir = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform3fv(loc_lights_dir, 1, &((*itrr)->get_direction())[0]);

		light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].intensity";
		GLuint loc_lights_in = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform1f(loc_lights_in, (GLfloat)(*itrr)->intensity);

		light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].cast_shadow";
		GLuint loc_cast_shadow = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniform1i(loc_cast_shadow, (GLint)(*itrr)->does_cast_shadow());

		if ((*itrr)->does_cast_shadow()) // If there is a proper shadow map, assign the uniform
		{
			light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].directional_shadow_map";
			GLuint loc_lights_sm = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
			glUniform1i(loc_lights_sm, static_texture_uniform_count
				+ max_plight_per_call + direct_light_count);
			glActiveTexture(GL_TEXTURE0 + static_texture_uniform_count
				+ max_plight_per_call + direct_light_count);
			glBindTexture(GL_TEXTURE_2D, (*itrr)->get_depth_map());
		}
		
		light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].light_space_matrix";
		GLuint loc_lights_lsm = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
		glUniformMatrix4fv(loc_lights_lsm, 1, GL_FALSE, &((*itrr)->get_space_matrix())[0][0]);

		direct_light_count++;
	}
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
	loc_point_light_count = glGetUniformLocation(program, "NUMBER_OF_POINT_LIGHTS");
	loc_direct_light_count = glGetUniformLocation(program, "NUMBER_OF_DIRECT_LIGHTS");

	static_texture_uniform_count = 8 + texture_uniform_starting_point;
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
