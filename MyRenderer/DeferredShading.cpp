
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
	
}

void DeferredShading::start_program(gBuffer* _GBuffer)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GBuffer = _GBuffer;

	point_light_count = 0;
	direct_light_count = 0;

	glUseProgram(program);
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

void DeferredShading::set_point_light
	(
		vec3 position,
		vec3 color,
		float radius,
		float linear,
		float quadratic, 
		float _far,
		GLuint shadow_map,
		float intensity
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

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].linear";
	GLuint loc_light_l = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1f(loc_light_l, (GLfloat) linear);

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].quadratic";
	GLuint loc_light_q = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1f(loc_light_q, (GLfloat) quadratic);

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].far";
	GLuint loc_light_far = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1f(loc_light_far, (GLfloat) _far);

	light_array_str = "point_lights[" + std::to_string(point_light_count) + "].point_shadow_map";
	GLuint loc_light_cubemap = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1i(loc_light_cubemap, static_texture_uniform_count + point_light_count + direct_light_count);
	glActiveTexture(GL_TEXTURE0 + static_texture_uniform_count + point_light_count + direct_light_count);
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
	mat4 light_space_matrix
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

	light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].directional_shadow_map";
	GLuint loc_lights_sm = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniform1i(loc_lights_sm, static_texture_uniform_count + point_light_count + direct_light_count);
	glActiveTexture(GL_TEXTURE0 + static_texture_uniform_count + point_light_count + direct_light_count);
	glBindTexture(GL_TEXTURE_2D, shadow_map);

	light_array_str = "direct_lights[" + std::to_string(direct_light_count) + "].light_space_matrix";
	GLuint loc_lights_lsm = glGetUniformLocation(program, (GLchar*)light_array_str.c_str());
	glUniformMatrix4fv(loc_lights_lsm, 1, GL_FALSE, &light_space_matrix[0][0]);

	direct_light_count += 1;
}

// Renders the scene
void DeferredShading::render(Camera* camera)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	set_viewer_pos(camera->get_eye());
	// Set g-buffer color attachments
	set_gPosition(GBuffer->get_gPosition());
	set_gNormal(GBuffer->get_gNormal());
	set_gAlbedoSpec(GBuffer->get_gAlbedoSpec());
	set_gPbr_materials(GBuffer->get_gPbr_materials());
	
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
	loc_viewer_pos = glGetUniformLocation(program, "viewer_pos");
	loc_gPosition = glGetUniformLocation(program, "gPosition");
	loc_gNormal = glGetUniformLocation(program, "gNormal");
	loc_gAlbedoSpec = glGetUniformLocation(program, "gAlbedoSpec");
	loc_gPbr_materials = glGetUniformLocation(program, "gPbr_materials");
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
	GLuint quadVBO;
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
	GLuint err = glGetError(); if (err) fprintf(stderr, "%s\n", gluErrorString(err));
	glDrawArrays(GL_TRIANGLES, 0, 6);
	GLuint errr = glGetError(); if (errr) fprintf(stderr, "%s\n", gluErrorString(errr));
	glBindVertexArray(0);
}
