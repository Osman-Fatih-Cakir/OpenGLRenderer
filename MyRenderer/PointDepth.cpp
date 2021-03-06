
#include <PointDepth.h>

#include <gtc/matrix_transform.hpp>
#include <init_shaders.h>

// Constructor
PointDepth::PointDepth()
{
	// Compiles shaders and generate program
	init_shaders();

	// Get uniform locations from program
	get_uniform_locations();
}

// Destructor
PointDepth::~PointDepth() 
{
	
}

void PointDepth::start_program(PointLight* _light)
{
	light = _light;

	// Set viewport and buffer
	glViewport(0, 0, light->depth_map_width, light->depth_map_height); // Use shadow resolutions
	glBindFramebuffer(GL_FRAMEBUFFER, light->depth_cubemap_fbo);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);
}

GLuint PointDepth::get_shader_program()
{
	return program;
}

void PointDepth::set_space_matrices(mat4 mats[6])
{
	glUniformMatrix4fv(loc_space_matrices, 6, GL_FALSE, &mats[0][0][0]);
}

void PointDepth::set_far(float num)
{
	glUniform1f(loc_far, num);
}

void PointDepth::set_position(vec3 pos)
{
	glUniform3fv(loc_position, 1,&pos[0]);
}

// Draw model
void PointDepth::render_model(Model* model, vec3 cam_pos)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// Set space matrices
	set_space_matrices(light->space_matrices);
	// Set far
	set_far(light->shadow_projection_far);
	// Set position
	set_position(light->position);

	// Draw call
	model->draw(program, cam_pos);

	glDisable(GL_CULL_FACE);
}

// Draw scene
void PointDepth::render(Scene* scene)
{
	for (unsigned int i = 0; i < scene->point_lights.size(); i++)
	{
		// Check if the light casts shadow
		if (!scene->point_lights[i]->does_cast_shadow())
			continue;

		start_program(scene->point_lights[i]);

		// Draw scene
		for (unsigned int ii = 0; ii < scene->all_models.size(); ii++)
		{
			// Draw
			render_model(scene->all_models[ii], scene->camera->get_position());
		}
		for (unsigned int ii = 0; ii < scene->translucent_models.size(); ii++)
		{
			render_model(scene->translucent_models[ii], scene->camera->get_position());
		}
	}
}

// Compiles shaders and generates program
void PointDepth::init_shaders()
{
	GLuint vertex_shader = initshaders(GL_VERTEX_SHADER, "shaders/point_depth_vs.glsl");
	GLuint geometry_shader = initshaders(GL_GEOMETRY_SHADER, "shaders/point_depth_gs.glsl");
	GLuint fragment_shader = initshaders(GL_FRAGMENT_SHADER, "shaders/point_depth_fs.glsl");
	program = initprogram(vertex_shader, geometry_shader, fragment_shader);
}

// Get uniform locations from program
void PointDepth::get_uniform_locations()
{
	loc_space_matrices = glGetUniformLocation(program, "light_space_matrix");
	loc_position = glGetUniformLocation(program, "light_position");
	loc_far = glGetUniformLocation(program, "far");
	loc_model_matrix = glGetUniformLocation(program, "model");
}