
#include <ForwardLitRender.h>
#include <init_shaders.h>

// Constructor
ForwardLitRender::ForwardLitRender()
{
	// Create program
	init_program();

	// Initialize uniforms
	init_uniforms();
}
// Destructor
ForwardLitRender::~ForwardLitRender()
{

}

// Renders the scene
void ForwardLitRender::render(gBuffer* GBuffer, MainFramebuffer* main_fb, Scene* scene)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	blit_depth_buffer(GBuffer, main_fb);

	glUseProgram(program);

	glViewport(0, 0, width, height);

	// Set camera attributes
	set_uniforms(scene);
	
	// Draw call for each object (forward rendering)
	std::vector<Model*>::iterator itr;
	for (itr = scene->translucent_models.begin(); itr != scene->translucent_models.end(); itr++)
	{
		(*itr)->draw(program, scene->camera->get_position());
	}
	glDisable(GL_BLEND);
}

// Change viewport resolution
void ForwardLitRender::change_viewport_resolution(unsigned int x, unsigned int y)
{
	width = x;
	height = y;
}

void ForwardLitRender::set_projection_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, &mat[0][0]);
}

void ForwardLitRender::set_view_matrix(mat4 mat)
{
	glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, &mat[0][0]);
}

// Getters and setters
GLuint ForwardLitRender::get_shader_program()
{
	return program;
}

// Compile shaders and create programs
void ForwardLitRender::init_program()
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/forward_lit_render_vs.glsl");
	GLint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/forward_lit_render_fs.glsl");
	program = initprogram(vertex_shader, fragment_shader);
}

// Initialize uniforms
void ForwardLitRender::init_uniforms()
{
	// Get uniform locaitons
	loc_projection_matrix = glGetUniformLocation(program, "projection_matrix");
	loc_view_matrix = glGetUniformLocation(program, "view_matrix");

	loc_viewer_pos = glGetUniformLocation(program, "viewer_pos");
	loc_irradiance_map = glGetUniformLocation(program, "irradiance_map");
	loc_prefiltered_map = glGetUniformLocation(program, "prefiltered_map");
	loc_brdf_lut = glGetUniformLocation(program, "brdf_lut");
	loc_is_ibl_active = glGetUniformLocation(program, "is_ibl_active");
	loc_max_reflection_lod = glGetUniformLocation(program, "MAX_REFLECTION_LOD");
	loc_point_light_count = glGetUniformLocation(program, "NUMBER_OF_POINT_LIGHTS");
	loc_direct_light_count = glGetUniformLocation(program, "NUMBER_OF_DIRECT_LIGHTS");

	static_texture_uniform_count = 3 + texture_uniform_starting_point;
}

// Blit depth buffer of gBuffer
void ForwardLitRender::blit_depth_buffer(gBuffer* GBuffer, MainFramebuffer* fb)
{
	// Attach depth buffer to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, GBuffer->get_fbo());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb->get_FBO()); // Write to default framebuffer
	glBlitFramebuffer(0, 0, GBuffer->get_width(), GBuffer->get_height(), 0, 0,
		GBuffer->get_width(), GBuffer->get_height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

// Set uniforms to shader
void ForwardLitRender::set_uniforms(Scene* scene)
{
	// Set matrices
	set_projection_matrix(scene->camera->get_projection_matrix());
	set_view_matrix(scene->camera->get_view_matrix());
	set_viewer_position(scene->camera->get_position());

	// Set IBL uniforms
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

	point_light_count = 0;
	direct_light_count = 0;

	set_light_uniforms(scene);
}

// Set light uniforms to shader
void ForwardLitRender::set_light_uniforms(Scene* scene)
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

void ForwardLitRender::set_irradiance_map(GLuint id)
{
	glUniform1i(loc_irradiance_map, texture_uniform_starting_point + 0);
	glActiveTexture(GL_TEXTURE0 + texture_uniform_starting_point + 0);
	if (!glIsTexture(id) || id == -1)
		return;
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}
void ForwardLitRender::set_prefiltered_map(GLuint id)
{
	glUniform1i(loc_prefiltered_map, texture_uniform_starting_point + 1);
	glActiveTexture(GL_TEXTURE0 + texture_uniform_starting_point + 1);
	if (!glIsTexture(id) || id == -1)
		return;
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}
void ForwardLitRender::set_brdf_lut(GLuint id)
{
	glUniform1i(loc_brdf_lut, texture_uniform_starting_point + 2);
	glActiveTexture(GL_TEXTURE0 + texture_uniform_starting_point + 2);
	if (!glIsTexture(id) || id == -1)
		return;
	glBindTexture(GL_TEXTURE_2D, id);
}
void ForwardLitRender::set_max_reflection_lod(float id)
{
	glUniform1f(loc_max_reflection_lod, id);
}
void ForwardLitRender::set_is_ibl_active(bool id)
{
	glUniform1i(loc_is_ibl_active, (int)id);
}
void ForwardLitRender::set_viewer_position(vec3 vec)
{
	glUniform3fv(loc_viewer_pos, 1, &vec[0]);
}
