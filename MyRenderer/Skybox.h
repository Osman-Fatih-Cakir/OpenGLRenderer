#pragma once

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <Camera.h>

class Skybox
{
public:
	// Constrctor takes path to .hdr file
	Skybox(const char* path);

	void get_uniform_location();
	void set_view_matrix(mat4 mat);
	void set_projection_matrix(mat4 mat);
	void render(Camera* camera);

	// Getters
	GLuint get_equirectangular_map();
	GLuint get_skybox_map();
	GLuint get_irradiance_map();
	GLuint get_width();
	GLuint get_height();
	

private:

	void generate_skybox_map();
	void generate_irradiance_map();
	void load_hdr_file(const char* path);
	void render_cube();
	void init_shader();
	void create_skybox_map();
	void create_irradiance_map();
	void create_framebuffer();
	void set_skybox_map();

	GLuint equirectangular_map;
	GLuint skybox_map;
	GLuint irradiance_map;
	GLuint program; // TODO rename program->skybox_program
	GLuint render_program;
	GLuint irradiance_program;
	GLuint captureFBO;
	GLuint captureRBO;

	GLuint loc_projection_matrix;
	GLuint loc_view_matrix;
	GLuint loc_skybox_map;

	GLuint width = 512;
	GLuint height = 512;
	GLuint irradiance_width = 32;
	GLuint irradiance_height = 32;
};
